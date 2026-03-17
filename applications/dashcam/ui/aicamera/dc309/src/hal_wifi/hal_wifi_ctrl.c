// #define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "hal_wifi_ctrl.h"
#include "mlog.h"

#define CONNECT_TIMEOUT_SEC 20
#define WLAN_CTRL_PATH "/var/run/wpa_supplicant/wlan0"

struct wpa_ctrl* wpa_ctrl_handle = NULL;

/* WiFi扫描状态管理 */
static wifi_scan_state_e g_wifi_scan_state = WIFI_SCAN_STATE_IDLE;
static wifi_info_t g_cached_scan_results[WIFI_INFO_MAX];
static int32_t g_cached_scan_count = 0;
static time_t g_scan_start_time = 0;
static bool g_scan_results_valid = false;

typedef struct list_network {
    int32_t network_id; // 网络 ID
    char ssid[128]; // 也就是 wifi 名称
    char bssid[32]; // 基站 MAC 地址
    char flags[64];
} list_network_t;

static int32_t hex_char_to_val(char c)
{
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if(c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static void decode_wpa_hex_string(const char *src, char *dst, size_t dst_size)
{
    if(dst_size == 0) return;

    size_t di = 0;
    for(size_t si = 0; src[si] != '\0' && di < dst_size - 1;) {
        if(src[si] == '\\' && src[si + 1] == 'x' && src[si + 2] != '\0' && src[si + 3] != '\0') {
            int32_t hi = hex_char_to_val(src[si + 2]);
            int32_t lo = hex_char_to_val(src[si + 3]);
            if(hi >= 0 && lo >= 0) {
                dst[di++] = (char)((hi << 4) | lo);
                si += 4;
                continue;
            }
        }
        dst[di++] = src[si++];
    }
    dst[di] = '\0';
}

static int32_t hal_wpa_open(struct wpa_ctrl **ctrl, char *ctrl_path)
{
    *ctrl = wpa_ctrl_open(ctrl_path);
    if (*ctrl == NULL) {
        MLOG_ERR("wpa_ctrl_open failed for path: %s\n", ctrl_path);
        MLOG_ERR("Please check if wpa_supplicant is running and %s exists\n", ctrl_path);
        return -1;
    }
    return 0;
}

static int32_t wpa_ctrl_handle_init()
{
    // 如果已经初始化，直接返回成功
    if (wpa_ctrl_handle != NULL) {
        MLOG_DBG("wpa_ctrl_handle already initialized\n");
        return 0;
    }

    int32_t ret = hal_wpa_open(&wpa_ctrl_handle, WLAN_CTRL_PATH);
    if (ret != 0) {
        MLOG_ERR("Failed to open wpa_ctrl, ret=%d\n", ret);
        MLOG_ERR("WLAN_CTRL_PATH=%s\n", WLAN_CTRL_PATH);
        wpa_ctrl_handle = NULL;
        return -1;
    }

    return 0;
}

// 比较函数，用于根据 signal_level 对 wifi_info_t 数组进行排序
static int32_t compare_wifi_signal_level(const void *a, const void *b)
{
    const wifi_info_t *info1 = (const wifi_info_t *)a;
    const wifi_info_t *info2 = (const wifi_info_t *)b;

    return info2->signal_level - info1->signal_level; // 降序排序
}

// 排序 wifi_info_t 数组
static void sort_wifi_info_by_signal_level(wifi_info_t info[], int32_t size)
{
    qsort(info, size, sizeof(wifi_info_t), compare_wifi_signal_level);
}

// 比较函数：先按 ssid 升序，再按 signal_level 降序
static int32_t compare_wifi_info(const void *a, const void *b)
{
    const wifi_info_t *wa = (const wifi_info_t *)a;
    const wifi_info_t *wb = (const wifi_info_t *)b;

    int32_t ssid_cmp = strcmp(wa->ssid, wb->ssid);
    if(ssid_cmp == 0) {
        // ssid 相同，按 signal_level 降序排序
        return wb->signal_level - wa->signal_level;
    }
    return ssid_cmp;
}

/*
 * @brief - 先排序（按 SSID + 信号强度）- 再遍历进行去重过滤。
 * @param info: wifi_info_t 数组
 * @param size: 数组的大小
 * @return: 返回去重后的实际大小
 */
static int32_t remove_duplicate_wifi_info(wifi_info_t *info, int32_t size)
{
    if(size <= 1) return size;

    // 第一步：排序，按照 ssid + signal_level
    qsort(info, size, sizeof(wifi_info_t), compare_wifi_info);

    // 第二步：去重（原地保留）
    int32_t write_index = 0;
    for(int32_t i = 0; i < size; ++i) {
        if(write_index == 0 || strncmp(info[i].ssid, info[write_index - 1].ssid, sizeof(info[i].ssid)) != 0) {
            info[write_index++] = info[i]; // 拷贝非重复项
        }
        // 否则跳过重复 ssid（只保留第一个，即信号最强）
    }

    return write_index;
}

/**
 * 分割一行文本，按指定分隔符，允许空字段
 * 返回字段个数，字段内容存在 fields[] 中（指向 line 的内部位置）
 */
static int32_t split_line(char *line, const char *delim, char *fields[], int32_t max_fields)
{
    int32_t count = 0;
    char *start = line;
    char *p;

    while(count < max_fields) {
        p = strstr(start, delim);
        if(p) {
            *p              = '\0';              // 替换为字符串结尾符
            fields[count++] = start;             // 保存字段起始位置
            start           = p + strlen(delim); // 移动到下一个字段起始位置
        } else {
            fields[count++] = start;
            break;
        }
    }

    return count;
}

// 解析单行 WiFi 结果
/*
 * 解析单行 WiFi 结果
 * 存在异常情况：
 * 1. 无ssid: 40:fe:95:02:3b:7e        2472    -20     [WPA2-PSK-CCMP][ESS]
 * 2. 异常ssid: ec:6c:9f:c0:3b:60        2412    -65     [WPA2-PSK-CCMP+TKIP][ESS]
 * \x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00
 * 3. 重复的ssid:
 *    a0:93:51:8d:18:a2        2462    -68     [WPA2-PSK-CCMP][ESS] Bitmaincorp
 *    00:a7:42:fa:b7:42        2412    -70     [WPA2-PSK-CCMP][ESS] Bitmaincorp
 */
static int32_t parse_scan_line(const char *line_in, wifi_info_t *res)
{
    char *fields[5] = {0};
    char temp[512];
    MLOG_DBG("Parsing scan line: %s\n", line_in);

    strncpy(temp, line_in, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    // 按 \t 分割字符串
    int32_t count = split_line(temp, "\t", fields, 5);

    if(count < 4) return -1; // 至少要有 bssid、freq、signal、ssid 四个字段

    if(strlen(fields[4]) == 0) return -1; // ssid 字段为空

    memset(res, 0, sizeof(*res));
    for(int32_t i = 0; i < count; i++) {
        switch(i) {
            case 0: strncpy(res->bssid, fields[i], sizeof(res->bssid) - 1); break;
            case 1: res->frequency = atoi(fields[i]); break;
            case 2: res->signal_level = atoi(fields[i]); break;
            case 3: strncpy(res->flags, fields[i], sizeof(res->flags) - 1); break;
            case 4: decode_wpa_hex_string(fields[i], res->ssid, sizeof(res->ssid)); break;
        }
    }

    return 0;
}

/*
 * 解析 WiFi 扫描结果字符串
 * @param srcbuf: 包含 WiFi 扫描结果的字符串
 * @param info: 用于存储解析结果的 wifi_info_t 数组
 * @param info_size: 输入为数组的最大大小，输出为实际解析的数量
 * @return: 0 成功，-1 失败
 */
static void parse_scan_results(char *srcbuf, wifi_info_t *info, int32_t *info_size)
{
    if(srcbuf == NULL || info == NULL || info_size == NULL) {
        MLOG_ERR("wifi info input or output NULL\n");
        return;
    }

    int32_t cnt = 0; // 记录当前的有效数据数量
    char* saveptr;
    char* line = strtok_r(srcbuf, "\n", &saveptr);

    MLOG_DBG("First line (header): %s\n", line ? line : "NULL");

    // 跳过标题行
    if (line) {
        line = strtok_r(NULL, "\n", &saveptr);
        MLOG_DBG("First data line: %s\n", line ? line : "NULL");
    }

    MLOG_DBG("Starting to parse scan lines...\n");
    // 解析剩余的每一行
    int32_t line_num = 0;
    while (line && cnt < *info_size) {
        line_num++;
        MLOG_DBG("Parsing line %d: %s\n", line_num, line);
        if (parse_scan_line(line, &info[cnt]) == 0) {
            cnt++;
        } else {
            MLOG_DBG("  Failed to parse line %d\n", line_num);
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    // 去除重复的数据
    cnt = remove_duplicate_wifi_info(info, cnt);

    // 按照WiFi信号顺序排序
    sort_wifi_info_by_signal_level(info, cnt);
    *info_size = cnt;

    // 打印WiFi INFO
    MLOG_DBG("-----------------------------------------------------------\n");
    for (int32_t i = 0; i < cnt; i++) {
        MLOG_DBG("Network %d:\n", i);
        MLOG_DBG("  BSSID: %s\n", info[i].bssid);
        MLOG_DBG("  Frequency: %d\n", info[i].frequency);
        MLOG_DBG("  Signal Level: %d\n", info[i].signal_level);
        MLOG_DBG("  Flags: %s\n", info[i].flags);
        MLOG_DBG("  SSID: %s\n", info[i].ssid);
        MLOG_DBG("--------------------------------------\n");
    }
    MLOG_DBG("[DEBUG] Final cnt = %d\n", cnt);
    MLOG_DBG("-----------------------------------------------------------\n");
}

/*
 * @brief 解析单行 network 结果, tab 分隔
 * @param line_in: 输入的行字符串
 * @param res: 输出的 list_network_t 结构体
 * @return: 0 成功，-1 失败
 * @note 格式为：
 * network id / ssid / bssid / flags
 * 0       SSID    any
 * 1       HONOR-440I77    any
 * 2       H3C_3B7C        any     [CURRENT]
 */
static int32_t parse_network_line(const char *line_in, list_network_t *res)
{
    char *fields[4] = {0};
    char temp[512];
    strncpy(temp, line_in, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    // 按 \t 分割字符串
    int32_t count = split_line(temp, "\t", fields, 4);

    if(count < 3) return -1; // 至少要有 network_id、ssid、bssid 三个字段

    MLOG_DBG("Parsing network line: %s\n", line_in);
    memset(res, 0, sizeof(*res));
    for(int32_t i = 0; i < count; i++) {
        switch(i) {
            case 0: res->network_id = atoi(fields[i]); break;
            case 1: decode_wpa_hex_string(fields[i], res->ssid, sizeof(res->ssid)); break;
            case 2: strncpy(res->bssid, fields[i], sizeof(res->bssid) - 1); break;
            case 3: strncpy(res->flags, fields[i], sizeof(res->flags) - 1); break;
        }
    }

    return 0;
}

/*
 * @brief 解析网络列表
 * @param srcbuf: 输入的网络列表字符串
 * @param list: 输出的 list_network_t 结构体
 * @param list_size: 输入为数组的最大大小，输出为实际解析的数量
 * @return: 0 成功，-1 失败
 */
static void parse_network_list(char *srcbuf, list_network_t *list, int32_t *list_size)
{
    if(srcbuf == NULL || list == NULL || list_size == NULL) {
        MLOG_ERR("wifi info input or output NULL\n");
        return;
    }

    int32_t cnt = 0; // 记录当前的有效数据数量
    char *saveptr;
    char *line = strtok_r(srcbuf, "\n", &saveptr);

    // 跳过标题行
    if(line) line = strtok_r(NULL, "\n", &saveptr);

    MLOG_DBG("Parsing network list: %s\n", srcbuf);
    // 解析剩余的每一行
    while(line && cnt < *list_size) {
        if(parse_network_line(line, &list[cnt]) == 0) {
            cnt++;
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    *list_size = cnt;
}

/*
 * @brief 获取网络列表（本地保存的网络列表）
 * @param ctrl: wpa_ctrl 结构体
 * @param list: 输出的 list_network_t 结构体
 * @param list_size: 输入为数组的最大大小，输出为实际解析的数量
 * @return: 0 成功，-1 失败
 */
static int32_t get_network_lists(struct wpa_ctrl *ctrl, list_network_t *list, int32_t *list_size)
{
    int32_t ret = 0;
    char buf[BUF_SIZE_MAX];
    size_t tmp_len = BUF_SIZE_MAX - 1;

    memset(buf, 0, sizeof(buf));
    MLOG_DBG("Sending LIST_NETWORKS command...\n");
    ret = wpa_ctrl_request(ctrl, "LIST_NETWORKS", strlen("LIST_NETWORKS"), buf, &tmp_len, NULL);
    if (ret != 0) {
        MLOG_ERR("wpa_ctrl_request LIST_NETWORKS failed, ret=%d\n", ret);
        return ret;
    }

    parse_network_list(buf, list, list_size);

    return ret;
}

/*
 * @brief 比较函数，用于根据保存状态和信号强度排序
 * @param a: 第一个 wifi_info_t 结构体
 * @param b: 第二个 wifi_info_t 结构体
 * @return: 返回比较结果
 */
static int32_t compare_wifi_info_with_saved(const void *a, const void *b)
{
    const wifi_info_t *wa = (const wifi_info_t *)a;
    const wifi_info_t *wb = (const wifi_info_t *)b;

    // 首先按是否保存过排序
    if(wa->save_flag != wb->save_flag) {
        return wb->save_flag - wa->save_flag; // 保存过的排在前面
    }

    // 其次按信号强度排序
    return wb->signal_level - wa->signal_level;
}

/*
 * @brief select_network id; 然后检查是否连接成功
          如果连接失败是因为密码错误，删除该网络。
          连接成功则保存网络配置。
 * @param ctrl: wpa_ctrl 结构体
 * @param network_id: 网络 ID
 * @return: 0 成功，-1 失败
*/
static int32_t enable_network(struct wpa_ctrl *ctrl, int32_t network_id)
{
    char buf[256], result[256];
    size_t result_len = sizeof(result) - 1;
    int32_t ret;
    bool connected = false;
    char event_buf[2048];
    size_t event_len;
    time_t start_time = time(NULL);

    memset(buf, 0x0, sizeof(buf));
    sprintf(buf, "SELECT_NETWORK %d", network_id);
    ret = wpa_ctrl_request(ctrl, buf, strlen(buf), result, &result_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Select network failed: %d\n", ret);
        return ret;
    }

    // 一般是网络不存在，才会 FAIL
    if(strstr(result, "FAIL")) {
        MLOG_ERR("SELECT_NETWORK %d FAIL, perhaps network not exist\n", network_id);
        return -1;
    }

    // 等待事件，判断是连接成功还是失败
    if(wpa_ctrl_attach(ctrl) != 0) {
        MLOG_ERR("Attach event failed\n");
        return -1;
    }

    while (!connected) {
        fd_set rfds;
        struct timeval tv;
        int32_t retval;

        FD_ZERO(&rfds);
        FD_SET(wpa_ctrl_get_fd(ctrl), &rfds);

        tv.tv_sec  = 2; // 2秒超时
        tv.tv_usec = 0;

        retval = select(wpa_ctrl_get_fd(ctrl) + 1, &rfds, NULL, NULL, &tv);
        if(retval == -1) {
            perror("select()");
            break;
        } else if(retval == 0) {
            MLOG_DBG("Timeout waiting for event.\n");
        } else {
            event_len = sizeof(event_buf) - 1;
            if(wpa_ctrl_recv(ctrl, event_buf, &event_len) == 0) {
                event_buf[event_len] = '\0';
                MLOG_DBG("Received event: %s\n", event_buf);

                // 检查连接成功的事件
                if(strstr(event_buf, "CTRL-EVENT-CONNECTED")) {
                    MLOG_DBG("Successfully connected!\n");
                    connected = true;
                }
                // 检查密码错误的事件
                else if(strstr(event_buf, "CTRL-EVENT-SSID-TEMP-DISABLED") || strstr(event_buf, "WRONG_KEY")) {
                    MLOG_ERR("Connection failed: Incorrect password.\n");
                    break;
                }
                // 检查其他认证失败事件
                else if(strstr(event_buf, "CTRL-EVENT-ASSOC-REJECT")) {
                    MLOG_ERR("Connection failed: Association rejected.\n");
                    break;
                }
            }
        }

        // 超时退出
        if(time(NULL) - start_time > CONNECT_TIMEOUT_SEC) {
            MLOG_ERR("连接超时\n");
            break;
        }
    }
    // 断开事件监听
    wpa_ctrl_detach(ctrl);

    if(!connected) {
        MLOG_ERR("连接失败，删除网络 ID %d\n", network_id);
        // 删除网络
        snprintf(buf, sizeof(buf), "REMOVE_NETWORK %d", network_id);
        result_len = sizeof(result) - 1;
        memset(result, 0, sizeof(result));
        ret = wpa_ctrl_request(ctrl, buf, strlen(buf), result, &result_len, NULL);
        if(ret != 0 || strstr(result, "FAIL")) {
            MLOG_ERR("Disable network failed: %s\n", result);
        }
    }

    // 保存网络配置
    result_len = sizeof(result) - 1;
    memset(result, 0, sizeof(result));
    snprintf(buf, sizeof(buf), "SAVE_CONFIG");
    ret = wpa_ctrl_request(ctrl, buf, strlen(buf), result, &result_len, NULL);
    if(ret != 0) {
        MLOG_ERR("RUN command %s failed: %d\n", buf, ret);
    }

    return connected ? 0 : -1; // 返回连接状态
}

/*
 * flags 会包含以下关键字：
 * [WPA] / [WPA2] / [WPA3] / [SAE] → 表示有密码（WPA/WPA2/WPA3）
 * [WEP] → 表示有密码（WEP 加密）
 * [ESS] → 表示基础服务集（普通 WiFi 网络）
 * 没有加密关键字，仅 [ESS] → 表示开放网络（无密码
 */
static int32_t is_wifi_open(const char *flags)
{
    if(strstr(flags, "WPA") || strstr(flags, "WEP") || strstr(flags, "SAE")) {
        return 0; // 有密码
    }
    return 1; // 开放网络
}

void Hal_Wpa_Up(void)
{
    system("ifconfig wlan0 up");
}

void Hal_Wpa_Down(void)
{
    system("ifconfig wlan0 down");
}

int32_t Hal_Wpa_GetScanResult(wifi_info_t* info, int32_t* info_size)
{
    int32_t ret;

    if (info == NULL || info_size == NULL || *info_size <= 0) {
        MLOG_ERR("Invalid parameters\n");
        return -1;
    }

    // 启动异步扫描
    ret = Hal_Wpa_Scan();
    if (ret != 0) {
        MLOG_ERR("Failed to start scan, ret=%d\n", ret);
        return -1;
    }

    // 等待扫描完成（使用轮询而非sleep）
    int32_t max_wait_ms = 5000; // 最多等待5秒
    int32_t poll_interval_ms = 200; // 每200ms检查一次
    int32_t elapsed_ms = 0;

    while (elapsed_ms < max_wait_ms) {
        // 尝试获取扫描结果
        ret = Hal_Wpa_GetScanResultAsync(info, info_size);
        if (ret == 0) {
            MLOG_DBG("Scan completed in %d ms, found %d networks\n", elapsed_ms, *info_size);
            return 0;
        }

        // 如果扫描超时或失败
        wifi_scan_state_e state = Hal_Wpa_GetScanState();
        MLOG_DBG("Poll %dms: state=%d, elapsed=%dms\n", elapsed_ms, state, elapsed_ms);
        if (state == WIFI_SCAN_STATE_IDLE && !g_scan_results_valid) {
            MLOG_ERR("Scan failed or timed out after %d ms\n", elapsed_ms);
            return -1;
        }

        // 等待100ms后重试
        usleep(poll_interval_ms * 1000);
        elapsed_ms += poll_interval_ms;
    }

    MLOG_ERR("Scan timeout after %d ms (max_wait_ms=%d)\n", elapsed_ms, max_wait_ms);
    return -1;
}

/*
 * @brief: 启动WiFi扫描（非阻塞方式）
 *         此函数会立即返回，不会等待扫描完成。
 * @return: 0 成功启动扫描，-1 失败
 */
int32_t Hal_Wpa_Scan(void)
{
    char tmpbuf[64];
    size_t tmp_len = sizeof(tmpbuf) - 1;
    int32_t ret;

    MLOG_DBG("Hal_Wpa_Scan called, current state=%d\n", g_wifi_scan_state);

    if (wpa_ctrl_handle_init() != 0) {
        MLOG_ERR("Hal_Wpa_Scan: wpa_ctrl_handle_init failed\n");
        return -1;
    }

    // 如果扫描正在进行，不重复启动
    if (g_wifi_scan_state == WIFI_SCAN_STATE_IN_PROGRESS) {
        MLOG_DBG("Scan already in progress, skipping new scan request\n");
        return 0;
    }

    // 发送扫描命令
    ret = wpa_ctrl_request(wpa_ctrl_handle, "SCAN", strlen("SCAN"), tmpbuf, &tmp_len, NULL);
    if (ret != 0) {
        MLOG_ERR("SCAN command failed: %d\n", ret);
        MLOG_ERR("tmpbuf response: %s\n", tmpbuf);
        return -1;
    }

    // 更新扫描状态
    g_wifi_scan_state = WIFI_SCAN_STATE_IN_PROGRESS;
    g_scan_start_time = time(NULL);
    g_scan_results_valid = false;
    MLOG_DBG("WiFi scan started, state set to IN_PROGRESS, start_time=%ld\n", g_scan_start_time);

    // 验证时间戳是否有效
    if (g_scan_start_time <= 0) {
        MLOG_ERR("WARNING: Invalid scan start time: %ld\n", g_scan_start_time);
        g_scan_start_time = time(NULL);
        MLOG_DBG("Re-initialized scan start time: %ld\n", g_scan_start_time);
    }

    return 0;
}

/*
 * @brief: 获取当前扫描状态
 * @return: 返回扫描状态（wifi_scan_state_e）
 */
wifi_scan_state_e Hal_Wpa_GetScanState(void)
{
    // 检查扫描是否超时（超过5秒则认为失败）
    if (g_wifi_scan_state == WIFI_SCAN_STATE_IN_PROGRESS) {
        time_t current_time = time(NULL);
        double elapsed = difftime(current_time, g_scan_start_time);
        MLOG_DBG("Hal_Wpa_GetScanState: scan in progress, elapsed=%.2fs\n", elapsed);

        if (elapsed > 5.0) {
            MLOG_DBG("Scan timeout (elapsed=%.2fs > 5.0s), resetting to IDLE\n", elapsed);
            MLOG_DBG("Scan started at: %ld, current time: %ld\n", g_scan_start_time, current_time);
            g_wifi_scan_state = WIFI_SCAN_STATE_IDLE;
            g_scan_results_valid = false;
        }
    } else {
        MLOG_DBG("Hal_Wpa_GetScanState: current state=%d\n", g_wifi_scan_state);
    }

    return g_wifi_scan_state;
}

/*
 * @brief: 获取WiFi扫描结果（异步版本）
 *         此函数不会阻塞等待，会立即返回当前可用的扫描结果。
 * @param info: 存储扫描结果的数组
 * @param info_size: 数组的最大大小，返回有效数据的大小。
 * @return: 0 成功，-1 失败或结果不可用
 */
int32_t Hal_Wpa_GetScanResultAsync(wifi_info_t* info, int32_t* info_size)
{
    if (info == NULL || info_size == NULL || *info_size <= 0) {
        MLOG_ERR("Invalid parameters: info=%p, info_size=%p, *info_size=%d\n", info, info_size, info_size ? *info_size : -1);
        return -1;
    }

    // 如果结果缓存有效，直接返回缓存的结果
    if (g_scan_results_valid && g_cached_scan_count > 0) {
        int32_t copy_count = (g_cached_scan_count < *info_size) ? g_cached_scan_count : *info_size;
        memcpy(info, g_cached_scan_results, sizeof(wifi_info_t) * copy_count);
        *info_size = copy_count;
        MLOG_DBG("Returning cached scan results: %d networks\n", copy_count);
        return 0;
    }

    // 如果扫描已完成但结果未缓存，则获取结果
    if (g_wifi_scan_state == WIFI_SCAN_STATE_COMPLETE && !g_scan_results_valid) {
        char buf[BUF_SIZE_MAX];
        size_t len = BUF_SIZE_MAX - 1;
        list_network_t net_list[NETWORK_LIST_MAX];
        int32_t net_count = NETWORK_LIST_MAX;
        int32_t ret;

        // 获取扫描结果
        memset(buf, 0, sizeof(buf));
        len = BUF_SIZE_MAX - 1;
        ret = wpa_ctrl_request(wpa_ctrl_handle, "SCAN_RESULTS", strlen("SCAN_RESULTS"), buf, &len, NULL);
        if (ret != 0) {
            MLOG_ERR("Get scan results failed: %d\n", ret);
            MLOG_ERR("Response buffer: %s\n", buf);
            g_wifi_scan_state = WIFI_SCAN_STATE_IDLE;
            return -1;
        }

        // 获取已保存的网络列表
        memset(net_list, 0, sizeof(net_list));
        ret = get_network_lists(wpa_ctrl_handle, net_list, &net_count);
        if (ret != 0) {
            MLOG_ERR("Get network list failed, ret=%d\n", ret);
            net_count = 0;
        } else {
            MLOG_DBG("Found %d saved networks\n", net_count);
        }

        // 解析扫描结果到临时缓冲区
        wifi_info_t temp_results[WIFI_INFO_MAX];
        int32_t temp_count = WIFI_INFO_MAX;
        parse_scan_results(buf, temp_results, &temp_count);

        // 标记已保存的网络
        for (int32_t i = 0; i < temp_count; i++) {
            temp_results[i].save_flag = false;
            temp_results[i].network_id = -1;

            for (int32_t j = 0; j < net_count; j++) {
                if (strcmp(temp_results[i].ssid, net_list[j].ssid) == 0) {
                    temp_results[i].save_flag = true;
                    temp_results[i].network_id = net_list[j].network_id;
                    if (strstr(net_list[j].flags, "[CURRENT]")) {
                        temp_results[i].connect_flag = true;
                    }
                    break;
                }
            }
        }

        // 按已保存状态和信号强度排序
        qsort(temp_results, temp_count, sizeof(wifi_info_t), compare_wifi_info_with_saved);

        // 缓存结果
        int32_t copy_count = (temp_count < WIFI_INFO_MAX) ? temp_count : WIFI_INFO_MAX;
        memcpy(g_cached_scan_results, temp_results, sizeof(wifi_info_t) * copy_count);
        g_cached_scan_count = copy_count;
        g_scan_results_valid = true;

        // 返回结果
        int32_t return_count = (copy_count < *info_size) ? copy_count : *info_size;
        memcpy(info, g_cached_scan_results, sizeof(wifi_info_t) * return_count);
        *info_size = return_count;

        MLOG_DBG("Scan completed: %d networks found\n", return_count);
        return 0;
    }

    // 如果扫描正在进行中，尝试获取结果（如果扫描已完成但状态未更新）
    if (g_wifi_scan_state == WIFI_SCAN_STATE_IN_PROGRESS && !g_scan_results_valid) {
        // 尝试直接获取扫描结果
        char buf[BUF_SIZE_MAX];
        size_t len = BUF_SIZE_MAX - 1;
        list_network_t net_list[NETWORK_LIST_MAX];
        int32_t net_count = NETWORK_LIST_MAX;
        int32_t ret;

        // 获取扫描结果
        memset(buf, 0, sizeof(buf));
        len = BUF_SIZE_MAX - 1;
        ret = wpa_ctrl_request(wpa_ctrl_handle, "SCAN_RESULTS", strlen("SCAN_RESULTS"), buf, &len, NULL);
        if (ret == 0) {
            MLOG_DBG("SCAN_RESULTS succeeded, scan may have completed\n");
            MLOG_DBG("Response: %s\n", buf);

            // 检查响应是否包含有效数据（不只是标题行）
            if (len > 50 && strstr(buf, "bssid")) { // 有实际数据
                MLOG_DBG("Scan results appear valid, processing...\n");
                // 如果成功获取结果，说明扫描已完成，更新状态
                g_wifi_scan_state = WIFI_SCAN_STATE_COMPLETE;

                // 获取已保存的网络列表
                memset(net_list, 0, sizeof(net_list));
                ret = get_network_lists(wpa_ctrl_handle, net_list, &net_count);
                if (ret != 0) {
                    MLOG_ERR("Get network list failed\n");
                    net_count = 0;
                }

                MLOG_DBG("Early detection: Scan completed while in progress state\n");
                // 解析扫描结果到临时缓冲区
                wifi_info_t temp_results[WIFI_INFO_MAX];
                int32_t temp_count = WIFI_INFO_MAX;
                parse_scan_results(buf, temp_results, &temp_count);

                // 标记已保存的网络
                for (int32_t i = 0; i < temp_count; i++) {
                    temp_results[i].save_flag = false;
                    temp_results[i].network_id = -1;

                    for (int32_t j = 0; j < net_count; j++) {
                        if (strcmp(temp_results[i].ssid, net_list[j].ssid) == 0) {
                            temp_results[i].save_flag = true;
                            temp_results[i].network_id = net_list[j].network_id;
                            if (strstr(net_list[j].flags, "[CURRENT]")) {
                                temp_results[i].connect_flag = true;
                            }
                            break;
                        }
                    }
                }

                // 按已保存状态和信号强度排序
                qsort(temp_results, temp_count, sizeof(wifi_info_t), compare_wifi_info_with_saved);

                // 缓存结果
                int32_t copy_count = (temp_count < WIFI_INFO_MAX) ? temp_count : WIFI_INFO_MAX;
                memcpy(g_cached_scan_results, temp_results, sizeof(wifi_info_t) * copy_count);
                g_cached_scan_count = copy_count;
                g_scan_results_valid = true;

                // 返回结果
                int32_t return_count = (copy_count < *info_size) ? copy_count : *info_size;
                memcpy(info, g_cached_scan_results, sizeof(wifi_info_t) * return_count);
                *info_size = return_count;

                MLOG_DBG("Scan completed (early detection): %d networks found\n", return_count);
                return 0;
            } else {
                MLOG_DBG("SCAN_RESULTS response too short or invalid, waiting...\n");
            }
        } else {
            MLOG_DBG("SCAN_RESULTS not ready yet, ret=%d\n", ret);
        }
    }

    // 如果扫描未完成或没有结果
    *info_size = 0;
    MLOG_DBG("Scan results not available yet, state: %d, g_scan_results_valid: %d\n", g_wifi_scan_state, g_scan_results_valid);
    return -1;
}

int32_t Hal_Wpa_Connect(wifi_info_t *info, const char *passwd)
{
    int32_t ret = 0;
    char cmd_buf[256]; // 增加缓冲区大小
    char result_buf[32];
    size_t result_len = sizeof(result_buf) - 1;
    int32_t i = 0, add_net_id = 0;
    list_network_t network_lists[NETWORK_LIST_MAX];
    int32_t list_count = NETWORK_LIST_MAX;

    MLOG_DBG("Hal_Wpa_Connect called\n");
    if (info == NULL) {
        MLOG_ERR("info is NULL\n");
        return -1;
    }

    MLOG_DBG("Connecting to SSID: %s\n", info->ssid);
    MLOG_DBG("Password: %s\n", passwd ? "***provided***" : "NULL");
    MLOG_DBG("Flags: %s\n", info->flags);

    if (wpa_ctrl_handle_init() != 0) {
        MLOG_ERR("wpa_ctrl_handle_init failed\n");
        return -1;
    }

    MLOG_DBG("Getting saved network lists...\n");
    // 获取网络列表
    ret = get_network_lists(wpa_ctrl_handle, network_lists, &list_count);
    if(ret != 0) {
        MLOG_ERR("Get network list failed, ret=%d\n", ret);
        return ret;
    }

    MLOG_DBG("Found %d saved networks\n", list_count);
    // 对比list_network 中是否有一样的ssid，如果有则直接select_network id，
    // 没有则add_network, set_network，再select_netwrok
    for(i = 0; i < list_count; i++) {
        MLOG_DBG("Checking saved network %d: %s (id=%d, flags=%s)\n", i, network_lists[i].ssid, network_lists[i].network_id, network_lists[i].flags);
        if(!strcmp(info->ssid, network_lists[i].ssid)) {
            MLOG_DBG("SSID %s already saved with network_id=%d, enabling it\n", info->ssid, network_lists[i].network_id);
            return enable_network(wpa_ctrl_handle, network_lists[i].network_id);
        }
    }

    MLOG_DBG("SSID %s not found in saved networks, adding new network\n", info->ssid);

    // add_network 返回网络 ID
    MLOG_DBG("Sending ADD_NETWORK command...\n");
    result_len = sizeof(result_buf);
    ret        = wpa_ctrl_request(wpa_ctrl_handle, "ADD_NETWORK", strlen("ADD_NETWORK"), result_buf, &result_len, NULL);
    if(ret == 0) {
        add_net_id = atoi(result_buf);
        MLOG_DBG("ADD_NETWORK succeeded, network_id=%d\n", add_net_id);
    } else {
        MLOG_ERR("add_network failed: %s\n", result_buf);
        return -1;
    }

    // 设置网络 SSID
    snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d ssid \"%s\"", add_net_id, info->ssid);
    MLOG_DBG("Setting SSID: %s\n", cmd_buf);
    result_len = sizeof(result_buf);
    ret        = wpa_ctrl_request(wpa_ctrl_handle, cmd_buf, strlen(cmd_buf), result_buf, &result_len, NULL);
    MLOG_DBG("wpa_ctrl_request cmd:%s, ret:%d, buf:%s\n", cmd_buf, ret, result_buf);
    if (ret != 0) {
        MLOG_ERR("Failed to set SSID\n");
        return -1;
    }

    // 设置网络 密码
    if(is_wifi_open(info->flags)) {
        // 如果是开放网络或WEP网络，则不设置密码
        MLOG_DBG("Open network detected, setting key_mgmt NONE\n");
        snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d key_mgmt NONE", add_net_id);
    } else {
        MLOG_DBG("Protected network detected, setting password\n");
        snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d psk \"%s\"", add_net_id, passwd);
    }
    MLOG_DBG("Setting password/key_mgmt: %s\n", cmd_buf);
    result_len = sizeof(result_buf);
    ret        = wpa_ctrl_request(wpa_ctrl_handle, cmd_buf, strlen(cmd_buf), result_buf, &result_len, NULL);
    MLOG_DBG("wpa_ctrl_request cmd:%s, ret:%d, buf:%s\n", cmd_buf, ret, result_buf);
    if (ret != 0) {
        MLOG_ERR("Failed to set password/key_mgmt\n");
        return -1;
    }

    MLOG_DBG("Enabling network...\n");
    ret = enable_network(wpa_ctrl_handle, add_net_id);
    MLOG_DBG("enable_network returned: %d\n", ret);

    return ret;
}

int32_t Hal_Wpa_EnableNetwork(int32_t network_id)
{
    int32_t ret = 0;
    char tmpbuf[128];
    char resp_buf[32];
    size_t tmp_len = sizeof(resp_buf) - 1;

    if(wpa_ctrl_handle_init() != 0) {
        return -1;
    }

    memset(tmpbuf, 0, sizeof(tmpbuf));
    sprintf(tmpbuf, "ENABLE_NETWORK %d", network_id);
    MLOG_DBG("Enable network command: %s\n", tmpbuf);

    ret = wpa_ctrl_request(wpa_ctrl_handle, tmpbuf, strlen(tmpbuf), resp_buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Enable network failed: %d\n", ret);
        return ret;
    }

    if(strstr(resp_buf, "OK")) {
        return 0;
    }

    return -1;
}

int32_t Hal_Wpa_DeleteNetwork(int32_t network_id)
{
    int32_t ret = 0;
    char buf[128], resp_buf[128];
    size_t tmp_len = sizeof(resp_buf) - 1;

    if(wpa_ctrl_handle_init() != 0) {
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "REMOVE_NETWORK %d", network_id);
    MLOG_DBG("Remove network command: %s\n", buf);

    ret = wpa_ctrl_request(wpa_ctrl_handle, buf, strlen(buf), resp_buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Run command %s failed: %d\n", buf, ret);
        return ret;
    }

    sprintf(buf, "SAVE_CONFIG");
    ret = wpa_ctrl_request(wpa_ctrl_handle, buf, strlen(buf), resp_buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Run command %s failed: %d\n", buf, ret);
        return ret;
    }

    return 0;
}

int32_t Hal_Wpa_GetConnectSignal()
{
    int32_t ret = 0;
    char buf[256], bss_buf[1024];
    size_t len     = sizeof(buf) - 1;
    size_t bss_len = sizeof(bss_buf) - 1;

    if(wpa_ctrl_handle_init() != 0) {
        return -1;
    }

    // 1. 先获取当前状态，确认是否连接
    ret = wpa_ctrl_request(wpa_ctrl_handle, "STATUS", strlen("STATUS"), buf, &len, NULL);
    if(ret != 0 || !strstr(buf, "wpa_state=COMPLETED")) {
        return -1; // 未连接
    }

    // 2. 从STATUS中提取BSSID
    char *bssid_line = strstr(buf, "bssid=");
    if(!bssid_line) {
        return -1;
    }

    char bssid[32];
    sscanf(bssid_line, "bssid=%s", bssid);

    // 3. 查询该BSS的详细信息（包含信号强度）
    snprintf(buf, sizeof(buf), "BSS %s", bssid);
    ret = wpa_ctrl_request(wpa_ctrl_handle, buf, strlen(buf), bss_buf, &bss_len, NULL);
    if(ret != 0) {
        return -1;
    }

    // 4. 解析信号强度（level参数）
    char *level_line = strstr(bss_buf, "level=");
    if(level_line) {
        return atoi(level_line + 6); // "level=" 长度为6
    }

    return -1;
}

/*
    @brief: 检查WiFi是否已开启
    @return: 0 表示 up，1 表示 down，2 表示出错（如无法读取标志位）
*/
int Hal_Wifi_Is_Up(void)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 2;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    // 接口名固定为 wlan0
    snprintf(ifr.ifr_name, IFNAMSIZ, "wlan0");

    if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
        perror("ioctl(SIOCGIFFLAGS)");
        close(sock);
        return 2;
    }

    close(sock);

    if (ifr.ifr_flags & IFF_UP) {
        puts("up");
        return 0;
    } else {
        puts("down");
        return 1;
    }
}

/*
 * @brief: 禁用所有保存的网络（禁止自动连接）
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_DisableAllNetworks(void)
{
    char buf[64], result[64];
    size_t result_len = sizeof(result) - 1;
    int32_t ret;

    if (wpa_ctrl_handle_init() != 0) {
        MLOG_ERR("wpa_ctrl_handle_init failed\n");
        return -1;
    }

    snprintf(buf, sizeof(buf), "DISABLE_NETWORK all");
    ret = wpa_ctrl_request(wpa_ctrl_handle, buf, strlen(buf), result, &result_len, NULL);
    if (ret != 0) {
        MLOG_ERR("DISABLE_NETWORK all failed: %d\n", ret);
        return -1;
    }

    MLOG_DBG("DISABLE_NETWORK all success\n");
    return 0;
}

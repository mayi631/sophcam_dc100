// #define DEBUG
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hal_wifi_ctrl.h"
#include "mlog.h"

typedef struct list_network
{
    int network_id; // 网络 ID
    char ssid[128]; // 也就是 wifi 名称
    char bssid[32]; // 基站 MAC 地址
    char flags[64];
} list_network_t;

// 比较函数，用于根据 signal_level 对 wifi_info_t 数组进行排序
int compare_wifi_signal_level(const void *a, const void *b)
{
    const wifi_info_t *info1 = (const wifi_info_t *)a;
    const wifi_info_t *info2 = (const wifi_info_t *)b;

    return info2->signal_level - info1->signal_level; // 降序排序
}

// 排序 wifi_info_t 数组
void sort_wifi_info_by_signal_level(wifi_info_t info[], int size)
{
    qsort(info, size, sizeof(wifi_info_t), compare_wifi_signal_level);
}

// 比较函数：先按 ssid 升序，再按 signal_level 降序
static int compare_wifi_info(const void *a, const void *b)
{
    const wifi_info_t *wa = (const wifi_info_t *)a;
    const wifi_info_t *wb = (const wifi_info_t *)b;

    int ssid_cmp = strcmp(wa->ssid, wb->ssid);
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
static int remove_duplicate_wifi_info(wifi_info_t *info, int size)
{
    if(size <= 1)
        return size;

    // 第一步：排序，按照 ssid + signal_level
    qsort(info, size, sizeof(wifi_info_t), compare_wifi_info);

    // 第二步：去重（原地保留）
    int write_index = 0;
    for(int i = 0; i < size; ++i) {
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
static int split_line(char *line, const char *delim, char *fields[], int max_fields)
{
    int count   = 0;
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
static int parse_scan_line(const char *line_in, wifi_info_t *res)
{
    char *fields[5] = {0};
    char temp[512];
    MLOG_DBG("Parsing scan line: %s\n", line_in);

    strncpy(temp, line_in, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    // 按 \t 分割字符串
    int count = split_line(temp, "\t", fields, 5);

    if(count < 4)
        return -1; // 至少要有 bssid、freq、signal、ssid 四个字段

    if(strlen(fields[4]) == 0)
        return -1; // ssid 字段为空

    memset(res, 0, sizeof(*res));
    for(int i = 0; i < count; i++) {
        switch(i) {
            case 0: strncpy(res->bssid, fields[i], sizeof(res->bssid) - 1); break;
            case 1: res->frequency = atoi(fields[i]); break;
            case 2: res->signal_level = atoi(fields[i]); break;
            case 3: strncpy(res->flags, fields[i], sizeof(res->flags) - 1); break;
            case 4: strncpy(res->ssid, fields[i], sizeof(res->ssid) - 1); break;
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

    int cnt = 0; // 记录当前的有效数据数量
    char *saveptr;
    char *line = strtok_r(srcbuf, "\n", &saveptr);

    // 跳过标题行
    if(line)
        line = strtok_r(NULL, "\n", &saveptr);

    // 解析剩余的每一行
    while(line && cnt < *info_size) {
        if(parse_scan_line(line, &info[cnt]) == 0) {
            cnt++;
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
    for(int i = 0; i < cnt; i++) {
        MLOG_DBG("BSSID: %s\n", info[i].bssid);
        MLOG_DBG(" Frequency: %d\n", info[i].frequency);
        MLOG_DBG(" Signal Level: %d\n", info[i].signal_level);
        MLOG_DBG(" Flags: %s\n", info[i].flags);
        MLOG_DBG(" SSID: %s\n", info[i].ssid);
        MLOG_DBG("--------------------------------------\n");
    }
    MLOG_DBG("[DEBUG] cnt = %d\n", cnt);
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
static int parse_network_line(const char *line_in, list_network_t *res)
{
    char *fields[4] = {0};
    char temp[512];
    strncpy(temp, line_in, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    // 按 \t 分割字符串
    int count = split_line(temp, "\t", fields, 4);

    if(count < 3)
        return -1; // 至少要有 network_id、ssid、bssid 三个字段

    MLOG_DBG("Parsing network line: %s\n", line_in);
    memset(res, 0, sizeof(*res));
    for(int i = 0; i < count; i++) {
        switch(i) {
            case 0: res->network_id = atoi(fields[i]); break;
            case 1: strncpy(res->ssid, fields[i], sizeof(res->ssid) - 1); break;
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

    int cnt = 0; // 记录当前的有效数据数量
    char *saveptr;
    char *line = strtok_r(srcbuf, "\n", &saveptr);

    // 跳过标题行
    if(line)
        line = strtok_r(NULL, "\n", &saveptr);

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

    ret = wpa_ctrl_request(ctrl, "LIST_NETWORKS", strlen("LIST_NETWORKS"), buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("wpa_ctrl_request failed\n");
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
static int compare_wifi_info_with_saved(const void *a, const void *b)
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
    int ret;

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

    while(1) {
        result_len = sizeof(result) - 1;
        if(wpa_ctrl_recv(ctrl, result, &result_len) == 0) {
            result[result_len] = '\0';
            MLOG_DBG("event: %s\n", result);

            if(strstr(result, "CTRL-EVENT-CONNECTED")) {
                MLOG_INFO("连接成功!\n");
                sprintf(buf, "SAVE_CONFIG"); // 保存网络配置
                break;
            } else if(strstr(result, "WRONG_KEY")) { // 某些版本可能报告这个
                MLOG_ERR("密码错误!\n");
                sprintf(buf, "REMOVE_NETWORK %d", network_id); // 删除网络配置
                break;
            } else if(strstr(result, "CTRL-EVENT-SSID-TEMP-DISABLED")) {
                MLOG_ERR("SSID 临时禁用，可能是密码错误!\n");
                sprintf(buf, "REMOVE_NETWORK %d", network_id); // 删除网络配置
                break;
            }
        }
    }

    // 断开事件监听
    wpa_ctrl_detach(ctrl);

    // 保存网络配置 或 删除网络配置
    ret = wpa_ctrl_request(ctrl, buf, strlen(buf), result, &result_len, NULL);
    if(ret != 0) {
        MLOG_ERR("RUN command %s failed: %d\n", buf, ret);
        return ret;
    }

    return 0;
}

int32_t Hal_Wpa_GetScanResult(struct wpa_ctrl *ctrl, wifi_info_t *info, int32_t *info_size)
{
    int32_t ret = 0;
    char buf[BUF_SIZE_MAX];
    size_t len = BUF_SIZE_MAX - 1;
    list_network_t net_list[NETWORK_LIST_MAX];
    int32_t net_count = NETWORK_LIST_MAX;

    if(ctrl == NULL || info == NULL || info_size == NULL) {
        return -1;
    }

    // 1. 先获取已保存的网络列表
    memset(net_list, 0, sizeof(net_list));
    ret = get_network_lists(ctrl, net_list, &net_count);
    if(ret != 0) {
        MLOG_ERR("Get network list failed\n");
        return ret;
    }

    // 2. 执行扫描
    char tmpbuf[64];
    size_t tmp_len = sizeof(tmpbuf) - 1;
    ret            = wpa_ctrl_request(ctrl, "SCAN", strlen("SCAN"), tmpbuf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("SCAN command failed: %d\n", ret);
        return ret;
    }

    // 等待扫描完成(建议等待2-3秒)
    sleep(2);

    // 3. 获取扫描结果
    memset(buf, 0, sizeof(buf));
    len = BUF_SIZE_MAX - 1;
    ret = wpa_ctrl_request(ctrl, "SCAN_RESULTS", strlen("SCAN_RESULTS"), buf, &len, NULL);
    if(ret != 0) {
        MLOG_ERR("Get scan results failed: %d\n", ret);
        return ret;
    }

    // 4. 解析扫描结果到info数组
    parse_scan_results(buf, info, info_size);

    // 5. 标记已保存的网络
    for(int i = 0; i < *info_size; i++) {
        info[i].save_flag  = false;
        info[i].network_id = -1;

        for(int j = 0; j < net_count; j++) {
            if(strcmp(info[i].ssid, net_list[j].ssid) == 0) {
                info[i].save_flag  = true;
                info[i].network_id = net_list[j].network_id;
                // 检查是否为当前连接的网络
                if(strstr(net_list[j].flags, "[CURRENT]")) {
                    info[i].connect_flag = true;
                }
                break;
            }
        }
    }

    // 6. 按已保存状态和信号强度排序
    qsort(info, *info_size, sizeof(wifi_info_t), compare_wifi_info_with_saved);

    return 0;
}

int32_t Hal_Wpa_Connect(struct wpa_ctrl *ctrl, wifi_info_t *info, const char *passwd)
{
    int32_t ret = 0;
    char sel_buf[32], add_net_buf[32];
    char tmpbuf[256]; // 增加缓冲区大小
    int32_t i = 0, add_net_id = 0;
    bool bfind_flag = false;
    size_t tmp_len  = sizeof(sel_buf) - 1;
    list_network_t network_lists[NETWORK_LIST_MAX];
    int32_t list_count = NETWORK_LIST_MAX;

    if(ctrl == NULL || info == NULL) {
        printf("ctrl or info is null\n");
        return -1;
    }

    // 获取网络列表
    ret = get_network_lists(ctrl, network_lists, &list_count);
    if(ret != 0) {
        MLOG_ERR("Get network list failed\n");
        return ret;
    }

    // 对比list_network 中是否有一样的ssid，如果有则直接select_network id，
    // 没有则add_network, set_network，再select_netwrok
    for(i = 0; i < list_count; i++) {
        if(!strcmp(info->ssid, network_lists[i].ssid)) {
            bfind_flag = true;
            break;
        }
    }

    if(bfind_flag) {
        // select_network
        memset(tmpbuf, 0x0, sizeof(tmpbuf));
        sprintf(tmpbuf, "SELECT_NETWORK %d", network_lists[i].network_id);
        MLOG_DBG("%s\n", tmpbuf);
        ret = wpa_ctrl_request(ctrl, tmpbuf, strlen(tmpbuf), sel_buf, &tmp_len, NULL);
        MLOG_DBG("%s\n", sel_buf);
        if(ret != 0)
            return ret;
    } else {
        // add_network
        ret = wpa_ctrl_request(ctrl, "ADD_NETWORK", strlen("ADD_NETWORK"), add_net_buf, &tmp_len, NULL);

        if(ret == 0) {
            add_net_id = atoi(add_net_buf);
        }
        memset(tmpbuf, 0x0, sizeof(tmpbuf));
        memset(add_net_buf, 0x0, sizeof(add_net_buf));
        snprintf(tmpbuf, sizeof(tmpbuf), "SET_NETWORK %d ssid \"%s\"", add_net_id, info->ssid);
        ret = wpa_ctrl_request(ctrl, tmpbuf, strlen(tmpbuf), add_net_buf, &tmp_len, NULL);
        if(ret == 0)
            printf("--%d--buf:%s--\n", __LINE__, add_net_buf);

        memset(tmpbuf, 0x0, sizeof(tmpbuf));
        memset(add_net_buf, 0x0, sizeof(add_net_buf));
        snprintf(tmpbuf, sizeof(tmpbuf), "SET_NETWORK %d psk \"%s\"", add_net_id, passwd);
        ret = wpa_ctrl_request(ctrl, tmpbuf, strlen(tmpbuf), add_net_buf, &tmp_len, NULL);
        if(ret == 0)
            printf("--%d--buf:%s--\n", __LINE__, add_net_buf);

        memset(tmpbuf, 0x0, sizeof(tmpbuf));
        memset(add_net_buf, 0x0, sizeof(add_net_buf));
        snprintf(tmpbuf, sizeof(tmpbuf), "SELECT_NETWORK %d", add_net_id);
        ret = wpa_ctrl_request(ctrl, tmpbuf, strlen(tmpbuf), add_net_buf, &tmp_len, NULL);
        if(ret == 0)
            printf("--%d--buf:%s--\n", __LINE__, add_net_buf);
    }

    return ret;
}

int32_t Hal_Wpa_EnableNetwork(struct wpa_ctrl *ctrl, int32_t network_id)
{
    int32_t ret = 0;
    char tmpbuf[128];
    char resp_buf[32];
    size_t tmp_len = sizeof(resp_buf) - 1;

    if(ctrl == NULL) {
        MLOG_ERR("ctrl is null\n");
        return -1;
    }

    memset(tmpbuf, 0, sizeof(tmpbuf));
    sprintf(tmpbuf, "ENABLE_NETWORK %d", network_id);
    MLOG_DBG("Enable network command: %s\n", tmpbuf);

    ret = wpa_ctrl_request(ctrl, tmpbuf, strlen(tmpbuf), resp_buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Enable network failed: %d\n", ret);
        return ret;
    }

    if(strstr(resp_buf, "OK")) {
        return 0;
    }

    return -1;
}

int32_t Hal_Wpa_DeleteNetwork(struct wpa_ctrl *ctrl, int32_t network_id)
{
    int32_t ret = 0;
    char buf[128], resp_buf[128];
    size_t tmp_len = sizeof(resp_buf) - 1;

    if(ctrl == NULL) {
        MLOG_ERR("ctrl is null\n");
        return -1;
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "REMOVE_NETWORK %d", network_id);
    MLOG_DBG("Remove network command: %s\n", buf);

    ret = wpa_ctrl_request(ctrl, buf, strlen(buf), resp_buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Run command %s failed: %d\n", buf, ret);
        return ret;
    }

    sprintf(buf, "SAVE_CONFIG");
    ret = wpa_ctrl_request(ctrl, buf, strlen(buf), resp_buf, &tmp_len, NULL);
    if(ret != 0) {
        MLOG_ERR("Run command %s failed: %d\n", buf, ret);
        return ret;
    }

    return 0;
}

int32_t Hal_Wpa_GetConnectSignal(struct wpa_ctrl *ctrl)
{
    int32_t ret = 0;
    char buf[256], bss_buf[1024];
    size_t len     = sizeof(buf) - 1;
    size_t bss_len = sizeof(bss_buf) - 1;

    if(ctrl == NULL) {
        return -1;
    }

    // 1. 先获取当前状态，确认是否连接
    ret = wpa_ctrl_request(ctrl, "STATUS", strlen("STATUS"), buf, &len, NULL);
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
    ret = wpa_ctrl_request(ctrl, buf, strlen(buf), bss_buf, &bss_len, NULL);
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

int32_t Hal_Wpa_Open(struct wpa_ctrl **ctrl, char *ctrl_path)
{
    *ctrl = wpa_ctrl_open(ctrl_path);
    if(*ctrl == NULL) {
        MLOG_ERR("wpa_ctrl_open failed\n");
        return -1;
    }
    return 0;
}

void Hal_Wpa_Close(struct wpa_ctrl *ctrl)
{
    wpa_ctrl_close(ctrl);
}

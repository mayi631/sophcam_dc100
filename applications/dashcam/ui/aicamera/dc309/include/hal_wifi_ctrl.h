#ifndef __HAL_WIFI_CTRL_H__
#define __HAL_WIFI_CTRL_H__

#include <stdbool.h>
#include <stdint.h>
#include "wpa_ctrl.h"

#define NETWORK_LIST_MAX 30 // 网络列表最大显示数量
#define WIFI_INFO_MAX 30    // wifi 信息最大数量
#define BUF_SIZE_MAX 4096

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct
{
    char bssid[32];       // mac 地址
    int32_t frequency;    // 频率
    int32_t signal_level; // 信号强度
    char flags[64];
    char ssid[128];

    /* network related */
    bool save_flag;     // 是否保存密码
    bool connect_flag;  // 是否连接成功
    int32_t network_id; // 网络 ID，用于直接连接或禁用网络
} wifi_info_t;

typedef enum {
    WPA_STATE_DISCONNECTED = 0, // 断开连接
    WPA_STATE_CONNECTING, // 正在连接
    WPA_STATE_CONNECTED, // 已连接
    WPA_STATE_FAILED, // 连接失败
} wpa_state_e;

typedef enum {
    WIFI_SCAN_STATE_IDLE = 0, // 空闲状态
    WIFI_SCAN_STATE_IN_PROGRESS, // 扫描进行中
    WIFI_SCAN_STATE_COMPLETE, // 扫描完成
} wifi_scan_state_e;

/*
 * @brief: ifconfig wlan0 up
 */
void Hal_Wpa_Up(void);

/*
 * @brief: ifconfig wlan0 down
 */
void Hal_Wpa_Down(void);

/*
 * @brief: 返回当前连接的信号强度
 *         先检查当前连接的信号强度，如果当前没有连接，则返回 -1。用于状态栏显示。
 * @param ctrl: wpa_ctrl 控制器
 * @return: 返回信号强度，单位 dBm（未连接返回 -1）
 */
int32_t Hal_Wpa_GetConnectSignal();

/*
 * @brief: 获取wifi的扫描结果，记录在 info 数组中，数组的大小由 Max_info 指定，
 *        返回实际填充的行数到 line_num。
 * @param ctrl: wpa_ctrl 控制器
 * @param info: 存储扫描结果的数组
 * @param info_size: 数组的最大大小，返回有效数据的大小。
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_GetScanResult(wifi_info_t* info, int32_t* info_size);

/*
 * @brief: 启动WiFi扫描（非阻塞方式）
 *         此函数会立即返回，不会等待扫描完成。
 *         使用 Hal_Wpa_GetScanState() 检查扫描状态，
 *         使用 Hal_Wpa_GetScanResultAsync() 获取扫描结果。
 * @return: 0 成功启动扫描，-1 失败
 */
int32_t Hal_Wpa_Scan(void);

/*
 * @brief: 获取当前扫描状态
 * @return: 返回扫描状态（wifi_scan_state_e）
 */
wifi_scan_state_e Hal_Wpa_GetScanState(void);

/*
 * @brief: 获取WiFi扫描结果（异步版本）
 *         此函数不会阻塞等待，会立即返回当前可用的扫描结果。
 *         需要先调用 Hal_Wpa_Scan() 启动扫描。
 * @param info: 存储扫描结果的数组
 * @param info_size: 数组的最大大小，返回有效数据的大小。
 * @return: 0 成功，-1 失败或结果不可用
 */
int32_t Hal_Wpa_GetScanResultAsync(wifi_info_t* info, int32_t* info_size);

/*
 * @brief: 连接到特定的 SSID，连接成功后会自动保存密码。
 * @param ctrl: wpa_ctrl 控制器
 * @param ssid: SSID 名称
 * @param passwd: 密码
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_Connect(wifi_info_t *info, const char *passwd);

/*
 * @brief: 删除指定的网络，网络是之前连接过的wifi，有保存密码等信息，可以直接连接。
 * @param ctrl: wpa_ctrl 控制器
 * @param network_id: 网络 ID
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_DeleteNetwork(int32_t network_id);

/*
    @brief: 检查WiFi是否已开启
    @return: 0 表示 up，1 表示 down，2 表示出错（如无法读取标志位）
*/
int Hal_Wifi_Is_Up(void);

/*
    @brief: 禁用所有保存的网络（禁止自动连接）
    @return: 0 成功，-1 失败
*/
int32_t Hal_Wpa_DisableAllNetworks(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #define __HAL_WIFI_CTRL_H__ */

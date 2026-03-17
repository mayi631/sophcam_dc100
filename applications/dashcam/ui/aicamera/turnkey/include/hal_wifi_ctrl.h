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

typedef enum
{
    WPA_STATE_DISCONNECTED = 0, // 断开连接
    WPA_STATE_CONNECTING,       // 正在连接
    WPA_STATE_CONNECTED,        // 已连接
    WPA_STATE_FAILED,           // 连接失败
} wpa_state_e;

/*
 * @brief: call wpa_ctrl_open
 * @param ctrl: 输出控制器
 * @param ctrl_path: 控制器路径
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_Open(struct wpa_ctrl **ctrl, char *ctrl_path);

/*
 * @brief: call wpa_ctrl_close
 * @param ctrl: 控制器
 * @return: 无返回值
 */
void Hal_Wpa_Close(struct wpa_ctrl *ctrl);

/*
 * @brief: 返回当前连接的信号强度
 *         先检查当前连接的信号强度，如果当前没有连接，则返回 -1。用于状态栏显示。
 * @param ctrl: wpa_ctrl 控制器
 * @return: 返回信号强度，单位 dBm（未连接返回 -1）
 */
int32_t Hal_Wpa_GetConnectSignal(struct wpa_ctrl *ctrl);

/*
 * @brief: 获取wifi的扫描结果，记录在 info 数组中，数组的大小由 Max_info 指定，
 *        返回实际填充的行数到 line_num。
 * @param ctrl: wpa_ctrl 控制器
 * @param info: 存储扫描结果的数组
 * @param info_size: 数组的最大大小，返回有效数据的大小。
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_GetScanResult(struct wpa_ctrl *ctrl, wifi_info_t *info, int32_t *info_size);

/*
 * @brief: 连接到特定的 SSID，连接成功后会自动保存密码。
 * @param ctrl: wpa_ctrl 控制器
 * @param ssid: SSID 名称
 * @param passwd: 密码
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_Connect(struct wpa_ctrl *ctrl, wifi_info_t *info, const char *passwd);

/*
 * @brief: 删除指定的网络，网络是之前连接过的wifi，有保存密码等信息，可以直接连接。
 * @param ctrl: wpa_ctrl 控制器
 * @param network_id: 网络 ID
 * @return: 0 成功，-1 失败
 */
int32_t Hal_Wpa_DeleteNetwork(struct wpa_ctrl *ctrl, int32_t network_id);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #define __HAL_WIFI_CTRL_H__ */

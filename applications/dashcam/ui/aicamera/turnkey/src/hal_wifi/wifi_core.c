/*
 * 不单单是wifi设置页面需要使用 wifi_ctrl_handle, 状态栏也需要使用。
 * 单独定义一个全局变量，避免在不同页面重复初始化。
 *
 * 另外就是 wifi 连接页面和 wifi 列表页面需要共享 wifi 连接状态。
 */
#include <stdio.h>
#include "mlog.h"
#include "hal_wifi_ctrl.h"

#define WLAN_CTRL_PATH "/var/run/wpa_supplicant/wlan0"

int wifi_scan_count              = 0;
struct wpa_ctrl *wpa_ctrl_handle = NULL;
wifi_info_t wifi_scan_results[WIFI_INFO_MAX];

// 初始化wpa_ctrl连接
int init_wpa_ctrl(void)
{
    // 如果已经初始化，直接返回成功
    if(wpa_ctrl_handle != NULL) {
        MLOG_WARN("WPA_CTRL already initialized\n");
        return 0;
    }

    // 创建新的连接
    int ret = Hal_Wpa_Open(&wpa_ctrl_handle, WLAN_CTRL_PATH);
    if(ret != 0) {
        MLOG_ERR("Failed to open wpa_ctrl\n");
        wpa_ctrl_handle = NULL;
        return -1;
    }

    return 0;
}

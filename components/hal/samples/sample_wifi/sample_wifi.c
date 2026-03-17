#include "hal_wifi.h"
#include <stdio.h>
#include <stdbool.h>

int main() {
    HAL_WIFI_CFG_S wifiCfg = {0};
    bool isStarted = false;

    // 配置 Wi-Fi 为 AP 模式
    wifiCfg.enMode = HAL_WIFI_MODE_AP;
    snprintf(wifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID, sizeof(wifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID), "MyWiFi");
    snprintf(wifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord, sizeof(wifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord), "password123");
    wifiCfg.unCfg.stApCfg.s32Channel = 6;

    // 初始化 Wi-Fi 模块
    if (HAL_WIFI_Init(HAL_WIFI_MODE_AP) != 0) {
        printf("Wi-Fi 初始化失败\n");
        return -1;
    }
    printf("Wi-Fi 初始化成功\n");

    // 验证配置合法性
    bool isValid = false;
    if (HAL_WIFI_CheckeCfgValid(&wifiCfg, &isValid) != 0 || !isValid) {
        printf("Wi-Fi 配置无效\n");
        HAL_WIFI_Deinit();
        return -1;
    }
    printf("Wi-Fi 配置合法\n");

    // 启动 Wi-Fi
    if (HAL_WIFI_Start(&wifiCfg) != 0) {
        printf("Wi-Fi 启动失败\n");
        HAL_WIFI_Deinit();
        return -1;
    }
    printf("Wi-Fi 已启动，SSID: %s, 密码: %s, 信道: %d\n",
           wifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID,
           wifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord,
           wifiCfg.unCfg.stApCfg.s32Channel);

    // 获取 Wi-Fi 启动状态
    if (HAL_WIFI_GetStartedStatus(&isStarted) == 0 && isStarted) {
        printf("Wi-Fi 当前状态: 已启动\n");
    } else {
        printf("Wi-Fi 当前状态: 未启动\n");
    }

    // 停止 Wi-Fi
    if (HAL_WIFI_Stop() != 0) {
        printf("Wi-Fi 停止失败\n");
        HAL_WIFI_Deinit();
        return -1;
    }
    printf("Wi-Fi 已停止\n");

    // 反初始化 Wi-Fi 模块
    if (HAL_WIFI_Deinit() != 0) {
        printf("Wi-Fi 反初始化失败\n");
        return -1;
    }
    printf("Wi-Fi 模块已反初始化\n");

    return 0;
}
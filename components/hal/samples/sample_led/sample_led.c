#include "hal_led.h"
#include <stdio.h>

int main() {
    HAL_LED_STATE_E ledState;

    // 初始化 LED 模块
    if (HAL_LED_Init() != 0) {
        printf("LED 初始化失败\n");
        return -1;
    }
    printf("LED 初始化成功\n");

    // 设置 LED0 为高电平（打开）
    if (HAL_LED_SetValue(HAL_GPIOA_28, 1) != 0) {
        printf("设置 LED0 状态失败\n");
        HAL_LED_Deinit();
        return -1;
    }
    printf("LED0 已打开\n");

    // 获取 LED0 的当前状态
    if (HAL_LED_GetState(HAL_LED_IDX_0, &ledState) == 0) {
        printf("LED0 当前状态: %s\n", (ledState == HAL_LED_STATE_H) ? "高电平（打开）" : "低电平（关闭）");
    } else {
        printf("获取 LED0 状态失败\n");
    }

    // 设置 LED1 为低电平（关闭）
    if (HAL_LED_SetValue(HAL_GPIOA_29, 0) != 0) {
        printf("设置 LED1 状态失败\n");
        HAL_LED_Deinit();
        return -1;
    }
    printf("LED1 已关闭\n");

    // 获取 LED1 的当前状态
    if (HAL_LED_GetState(HAL_LED_IDX_1, &ledState) == 0) {
        printf("LED1 当前状态: %s\n", (ledState == HAL_LED_STATE_H) ? "高电平（打开）" : "低电平（关闭）");
    } else {
        printf("获取 LED1 状态失败\n");
    }

    // 反初始化 LED 模块
    if (HAL_LED_Deinit() != 0) {
        printf("LED 反初始化失败\n");
        return -1;
    }
    printf("LED 模块已反初始化\n");

    return 0;
}

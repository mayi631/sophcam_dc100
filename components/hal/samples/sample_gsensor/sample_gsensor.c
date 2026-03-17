#include "hal_gsensor.h"
#include <stdio.h>

int main() {
    HAL_GSENSOR_CFG_S gsensorCfg = {0};
    HAL_GSENSOR_VALUE_S gsensorValue = {0};
    unsigned char collisionStatus = 0;

    // 配置 G-sensor 参数
    gsensorCfg.busnum = 1; // I2C 总线号

    // 初始化 G-sensor
    if (HAL_GSENSOR_Init(&gsensorCfg) != 0) {
        printf("G-sensor 初始化失败\n");
        return -1;
    }
    printf("G-sensor 初始化成功\n");

    // 设置灵敏度为中等
    if (HAL_GSENSOR_SetSensitity(HAL_GSENSOR_SENSITITY_MIDDLE) != 0) {
        printf("设置灵敏度失败\n");
        HAL_GSENSOR_DeInit();
        return -1;
    }
    printf("G-sensor 灵敏度已设置为中等\n");

    // 获取当前加速度值
    if (HAL_GSENSOR_GetCurValue(&gsensorValue) == 0) {
        printf("当前加速度值: X = %d, Y = %d, Z = %d\n",
               gsensorValue.s16XDirValue,
               gsensorValue.s16YDirValue,
               gsensorValue.s16ZDirValue);
    } else {
        printf("获取加速度值失败\n");
    }

    // 获取碰撞状态
    if (HAL_GSENSOR_GetCollisionStatus(&collisionStatus) == 0) {
        printf("碰撞状态: %s\n", collisionStatus ? "发生碰撞" : "未发生碰撞");
    } else {
        printf("获取碰撞状态失败\n");
    }

    // 打开中断
    if (HAL_GSENSOR_OpenInterrupt(1) != 0) {
        printf("打开中断失败\n");
    } else {
        printf("中断已打开\n");
    }

    // 读取中断状态
    int32_t interruptStatus = HAL_GSNESOR_ReadInterrupt();
    printf("中断状态: %d\n", interruptStatus);

    // 反初始化 G-sensor
    if (HAL_GSENSOR_DeInit() != 0) {
        printf("G-sensor 反初始化失败\n");
        return -1;
    }
    printf("G-sensor 已反初始化\n");

    return 0;
}

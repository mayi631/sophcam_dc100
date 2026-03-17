#include "hal_pwm.h"
#include <stdio.h>

int main() {
    HAL_PWM_S pwmAttr = {0};

    // 配置 PWM 参数
    pwmAttr.group = 0;          // PWM 组
    pwmAttr.channel = 1;        // PWM 通道
    pwmAttr.period = 1000000;   // 周期（单位：纳秒）
    pwmAttr.duty_cycle = 500000; // 占空比（单位：纳秒）

    // 初始化 PWM
    if (HAL_PWM_Init(pwmAttr) != 0) {
        printf("PWM 初始化失败\n");
        return -1;
    }
    printf("PWM 初始化成功\n");

    // 设置占空比为 75%
    if (HAL_PWM_Set_Percent(75) != 0) {
        printf("设置占空比失败\n");
        HAL_PWM_Deinit(pwmAttr);
        return -1;
    }
    printf("占空比已设置为 75%%\n");

    // 获取当前占空比
    int32_t percent = HAL_PWM_Get_Percent();
    if (percent < 0) {
        printf("获取占空比失败\n");
    } else {
        printf("当前占空比为 %d%%\n", percent);
    }

    // 禁用 PWM
    if (HAL_PWM_Deinit(pwmAttr) != 0) {
        printf("PWM 反初始化失败\n");
        return -1;
    }
    printf("PWM 已禁用并反初始化\n");

    return 0;
}

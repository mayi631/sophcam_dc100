#ifndef __USER_IIC0_H__
#define __USER_IIC0_H__

#ifdef __cplusplus
extern "C" {
#endif

extern int8_t brightness_level;

/**
 * 打开LED灯
 * 
 * 通过I2C控制LED灯，设置地址0x01为开启状态，地址0x02为关闭状态
 */
void led_on(void);

/**
 * 关闭LED灯
 * 
 * 通过I2C控制关闭LED灯，向地址0x01写入0x00值
 */
void led_off(void);
/**
 * 打开红外截止滤镜
 * 
 * 通过I2C通信向设备发送控制指令，激活红外截止滤镜功能
 * 用于在夜间或低光环境下切换摄像头的工作模式
 */
void ircut_on(void);

/**
 * 关闭红外截止滤镜
 * 
 * 通过I2C通信向设备发送控制指令，关闭红外截止滤镜功能
 * 用于在正常光照环境下切换摄像头的工作模式
 */
void ircut_off(void);


//设置红外灯LED灯亮度
void led_on_with_brightness(int levlel);

void led_cleanup(void);

/**
 * 根据电池电量自动调整红外灯亮度档位
 * 当电量下降时，自动降档到当前电量允许的最大档位
 */
void auto_adjust_redlight_by_battery(void);

int8_t get_max_red_light_level(void);

#ifdef __cplusplus
}
#endif
#endif
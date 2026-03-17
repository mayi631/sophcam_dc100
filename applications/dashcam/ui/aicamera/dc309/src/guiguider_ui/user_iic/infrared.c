#define DEBUG
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <linux/i2c.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "mlog.h"
#include "hal_pwm.h"  // 添加PWM头文件
#include "ui_common.h"
#include "page_all.h"

#define I2C_DEVICE  "/dev/i2c-%d"
#define TP_I2C_ADDR 0x30 //设备地址
int bus_id = 0;		// I2C bus id

// PWM相关定义
#define LED_PWM_GROUP 1      // PWM组号
#define LED_PWM_CHANNEL 2    // PWM8通道
#define LED_MAX_BRIGHTNESS 100  // 最大亮度值

int red_light_level[] = {0, 64, 74, 80, 84, 90, 96, 100};

// PWM属性结构体
static HAL_PWM_S led_pwm_attr = {0};
static bool pwm_initialized = false;
int8_t brightness_level = 0;
/**
 * @brief 初始化PWM用于LED亮度控制
 * 
 * @return int 成功返回0，失败返回-1
 */
static int init_led_pwm(void)
{
    if (pwm_initialized) {
        return 0;  // 已经初始化过
    }
    
    // 配置PWM参数
    led_pwm_attr.group = LED_PWM_GROUP;
    led_pwm_attr.channel = LED_PWM_CHANNEL;
    led_pwm_attr.period = 100;        // 周期（单位：纳秒）
    led_pwm_attr.duty_cycle = 0;      // 初始占空比为0（LED熄灭）
    
    // 初始化PWM
    if (HAL_PWM_Init(led_pwm_attr) != 0) {
        MLOG_ERR("LED PWM初始化失败\n");
        return -1;
    }
    
    pwm_initialized = true;
    MLOG_DBG("LED PWM初始化成功，使用PWM%d\n", LED_PWM_CHANNEL);
    return 0;
}

/**
 * @brief 设置LED亮度
 * 
 * @param brightness 亮度值 (0-100)，0为完全关闭，100为最亮
 * @return int 成功返回0，失败返回-1
 */
static int set_led_brightness(int brightness)
{
    // 确保亮度值在有效范围内
    if (brightness < 0) brightness = 0;
    if (brightness > LED_MAX_BRIGHTNESS) brightness = LED_MAX_BRIGHTNESS;
    
    // 如果PWM未初始化，先初始化
    if (!pwm_initialized) {
        if (init_led_pwm() != 0) {
            return -1;
        }
    }
    
    // 设置PWM占空比
    led_pwm_attr.duty_cycle = brightness;
    
    if (HAL_PWM_Set_Param(led_pwm_attr) != 0) {
        MLOG_ERR("设置LED亮度失败: %d%%\n", brightness);
        return -1;
    }
    
    MLOG_DBG("LED亮度设置为: %d%%\n", brightness);
    return 0;
}

/**
 * @brief 执行I2C设备测试函数
 * 
 * 打开指定的I2C总线，设置从设备地址，并向寄存器0x02写入值0x01。
 * 该函数主要用于测试I2C设备的通信功能，目前只实现写入操作。
 * 写入完成后会切换value的值用于下一次测试。
 * 
 * @return int 成功返回0，失败返回错误码：
 *         -ENODEV: 无法打开I2C设备或无法访问从设备
 *         -1: I2C写入操作失败
 */
int user_i2c0(uint8_t reg, uint8_t value)
{
    int i2c_file;
    char bus_path[64];
    MLOG_DBG("I2C测试\n");
    
    // 打开I2C设备
    snprintf(bus_path, sizeof(bus_path), I2C_DEVICE, bus_id);
    i2c_file = open(bus_path, O_RDWR);
    if(i2c_file < 0) {
        MLOG_ERR("Failed to open the I2C bus: %s\n", strerror(errno));
        return -ENODEV;
    }

    // 设置I2C从设备地址
    if(ioctl(i2c_file, I2C_SLAVE, TP_I2C_ADDR) < 0) {
        MLOG_ERR("Failed to acquire bus access and/or talk to slave: %s\n", strerror(errno));
        close(i2c_file);
        return -ENODEV;
    }

    // 写I2C
    uint8_t buf[2] = {reg, value};

    if(write(i2c_file, buf, sizeof(buf)) != sizeof(buf)) {
        perror("Write failed");
        close(i2c_file);
        return -1;
    }

    return 0;
}

uint8_t g_curreffectindex = 0;

/**
 * 打开LED灯（使用PWM控制亮度）
 * 
 * 通过PWM控制LED灯亮度，默认设置为最大亮度
 */
void led_on(void)
{
    // 如果PWM控制失败，可以回退到原来的I2C控制方式
    user_i2c0(0x01, 0x01);
    extern bool is_video_mode;
    //开启黑白特效
    g_curreffectindex = geteffect_index();    
    MESSAGE_S event = {0};
    event.topic     = EVENT_MODEMNG_SETTING;
    if(is_video_mode == false) {
        event.arg1 = PARAM_MENU_PHOTO_EFFECT;
    } else if(is_video_mode == true) {
        event.arg1 = PARAM_MENU_VIDEO_EFFECT;
    }
    event.arg2 = 1;
    MODEMNG_SendMessage(&event);
}

/**
 * 打开LED灯并设置指定亮度
 * 
 * @param levlel 亮度值 (0-7)
 */
void led_on_with_brightness(int levlel)
{
    if (levlel >= 7)
        levlel = 7;
    else if (levlel <= 0)
        levlel = 0;
    if (set_led_brightness(red_light_level[levlel]) == 0) {
        brightness_level = levlel;
        MLOG_DBG("LED灯已打开，自定义亮度等级: %d\n", levlel);
    } else {
        MLOG_ERR("LED灯打开失败，使用默认亮度\n");
        led_on();  // 回退到默认亮度
    }
}

/**
 * 关闭LED灯
 * 
 * 通过PWM控制关闭LED灯，将亮度设置为0
 */
void led_off(void)
{
    user_i2c0(0x01, 0x00);

    //恢复原有特效
    extern bool is_video_mode;
    MESSAGE_S event = {0};
    event.topic     = EVENT_MODEMNG_SETTING;
    if(is_video_mode == false) {
        event.arg1 = PARAM_MENU_PHOTO_EFFECT;
    } else if(is_video_mode == true) {
        event.arg1 = PARAM_MENU_VIDEO_EFFECT;
    }
    event.arg2 = g_curreffectindex;
    MODEMNG_SendMessage(&event);
}

/**
 * 清理PWM资源
 */
void led_cleanup(void)
{
    if (pwm_initialized) {
        HAL_PWM_Deinit(led_pwm_attr);
        pwm_initialized = false;
        MLOG_DBG("LED PWM资源已清理\n");
    }
}

/**
 * 打开红外截止滤镜
 * 
 * 通过I2C通信向设备发送控制指令，激活红外截止滤镜功能
 * 用于在夜间或低光环境下切换摄像头的工作模式
 */
void ircut_on(void)
{
    user_i2c0(0x02, 0x01);
}

/**
 * 关闭红外截止滤镜
 * 
 * 通过I2C通信向设备发送控制指令，关闭红外截止滤镜功能
 * 用于在正常光照环境下切换摄像头的工作模式
 */
void ircut_off(void)
{
    user_i2c0(0x01, 0x00);
}
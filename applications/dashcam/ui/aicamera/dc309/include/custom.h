/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
#include <string.h>

#include "config.h"
#include "lvgl/src/libs/freetype/lv_freetype.h"

void custom_init(lv_ui_t *ui);
void recreate_status_bar(lv_ui_t *ui);

typedef void (*mode_chage_completed_t)(void);

typedef struct
{
    lv_timer_t *signal_timer;
    lv_timer_t *wifi_timer;
    lv_timer_t *power_timer;
    lv_timer_t *date_timer;
    lv_timer_t *timelapse_timer;
} timer_manager_t;

// 定义字体颜色类型为 uint32_t 以支持任意 RGB 颜色、更加灵活
typedef uint32_t font_color_t;

extern lv_style_t ttf_font_12;
extern lv_style_t ttf_font_14;
extern lv_style_t ttf_font_16;
extern lv_style_t ttf_font_18;
extern lv_style_t ttf_font_20;
extern lv_style_t ttf_font_22;
extern lv_style_t ttf_font_24;
extern lv_style_t ttf_font_28;
extern lv_style_t ttf_font_30;
extern lv_style_t ttf_font_34;

typedef enum {
    FONT_DEFAULT, // 默认字体
    FONT_SC16, // 思源黑体16号
} font_id_t;

// 定义字体大小模式
typedef enum {
    FONT_SIZE_MODE_NATIVE, // 使用字体文件中的原始大小
    FONT_SIZE_MODE_CUSTOM  // 使用自定义字体大小缩放
} font_size_mode_t;

// 定义字体大小枚举
typedef enum {
    FONT_SIZE_16 = 16,
    FONT_SIZE_22 = 22,
    FONT_SIZE_24 = 24,
    FONT_SIZE_28 = 28,
    FONT_SIZE_30 = 30,
    FONT_SIZE_32 = 32,
    FONT_SIZE_34 = 34,
    FONT_SIZE_36 = 36,
    FONT_SIZE_40 = 40,
} font_size_t;

// 字体大小配置结构体
typedef struct
{
    font_size_mode_t mode; // 字体大小模式
    font_size_t size;      // 仅在 CUSTOM 模式下有效
} font_size_config_t;

void font_setting(lv_obj_t *obj, font_id_t font_id, font_size_config_t size_cfg, font_color_t color, int16_t x,
                  int16_t y);

// void set_chs_fonts(const char *font_path, int font_size, lv_style_t *style) ; // 改为指针参数
extern lv_font_t *font_chs;
void set_chs_fonts(const char *font_path, int font_size, lv_style_t *style);

//只获取字体，不设置风格（获取之后需lv_obj_set_style_text_font自定义设置字体）
lv_font_t *get_usr_fonts(const char *font_path, int font_size);

//设置控件的渐变风格
void lv_set_obj_grad_style(lv_obj_t *obj, lv_grad_dir_t value, lv_color_t star_color_value, lv_color_t end_color_value);

//初始化图片显示的文件系统
void init_image_filesystem(void);
//显示图片
void show_image(lv_obj_t *obj, const char *image);

// 图像描述符结构体
typedef struct
{
    lv_img_dsc_t dsc;
    uint8_t *data;    // 存储图像数据的指针
    size_t data_size; // 图像数据大小
} image_descriptor_t;

// 主要函数声明
void show_image(lv_obj_t *obj, const char *image_name);
void free_image_descriptors(void);

// 初始化字体风格
void init_fonts_style(void);

extern timer_manager_t timers;

void show_battery_warning(void);
bool get_lowbatter_tips_flag(void);
void set_lowbatter_tips_flag(bool ind);

void normalize_path(char *path);

void completed_execution_and_unregister_cb(void);

void completed_register_cb(mode_chage_completed_t callback);

bool get_exit_completed(void);

void set_exit_completed(bool sta);

void update_setting_from_param(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

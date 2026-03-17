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

typedef struct
{
    lv_timer_t *signal_timer;
    lv_timer_t *wifi_timer;
    lv_timer_t *power_timer;
    lv_timer_t *charge_timer;
    lv_timer_t *date_timer;
} timer_manager_t;

// 定义字体颜色类型为 uint32_t 以支持任意 RGB 颜色、更加灵活
typedef uint32_t font_color_t;

typedef enum {
    FONT_DEFAULT, // 默认字体
    FONT_SC16,    // 思源黑体16号
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

extern lv_font_t *font_chs;
void set_chs_fonts(const char *font_path, int font_size, lv_style_t *style);

void init_fonts_style(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

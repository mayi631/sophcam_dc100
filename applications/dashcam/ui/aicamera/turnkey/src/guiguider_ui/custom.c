/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include <stdlib.h>
#include <string.h>
#include "custom.h"
#include <time.h>

static timer_manager_t timers;

static bool power_charging = false;

lv_font_t *font_chs = NULL;

static void sys_time_update(lv_timer_t *timer)
{
    lv_ui_t *ui  = (lv_ui_t *)lv_timer_get_user_data(timer);
    time_t now   = time(NULL);
    struct tm *t = localtime(&now);

    // 更新日期
    lv_label_set_text_fmt(ui->date_text, "%04d/%02d/%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    // 更新时间
    lv_label_set_text_fmt(ui->digital_clock, "%02d:%02d", t->tm_hour, t->tm_min);
}
#if 0
static void digital_clock_1_timer(lv_timer_t *timer) {
    lv_ui_t *ui = (lv_ui_t *)lv_timer_get_user_data(timer);
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 格式化为"HH:MM"（示例输出："14:30"）
    char time_str[6];
    snprintf(time_str, sizeof(time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    // 更新标签文本
    if(lv_scr_act() == ui->screenHome1)
        lv_label_set_text(ui->screenHome1_digital_clock_1, time_str);
    else if(lv_scr_act() == ui->screenHome2)
        lv_label_set_text(ui->screenHome2_digital_clock_1, time_str);
    else ;
}
#endif
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */
// static void signal_update_cb(lv_timer_t *t) {
//     lv_ui_t *ui = (lv_ui_t *)lv_timer_get_user_data(t);
//     static uint8_t level = 0;
//     level = (level + 1) % 5;
//     lv_img_set_src(ui->screenHome1_img_4g,
//         level == 0 ? &_4G_none_RGB565A8_24x17 :
//         level == 1 ? &_4g_1_RGB565A8_24x17 :
//         level == 2 ? &_4g_2_RGB565A8_24x17 :
//         level == 3 ? &_4g_3_RGB565A8_24x17 : &_4g_4_RGB565A8_24x17);
// }

// 模拟信号强度检测函数（需根据实际硬件实现）
static int get_signal_strength()
{
    // 这里模拟返回0-4的信号强度值
    // 实际项目中应替换为真实的信号检测逻辑（如AT命令查询或ADC读取）
    return rand() % 5; // 随机模拟0-4的信号强度
}
static int get_wifi_strength()
{
    // 这里模拟返回0-4的信号强度值
    // 实际项目中应替换为真实的信号检测逻辑（如AT命令查询或ADC读取）
    return rand() % 5; // 随机模拟0-4的信号强度
}
static int get_power_strength()
{
    // 生成1-4的随机数
    return rand() % 4 + 1; // 等价于 rand() % (4 - 1 + 1) + 1
}
static void signal_update_cb(lv_timer_t *t)
{
    lv_ui_t *ui      = (lv_ui_t *)lv_timer_get_user_data(t);
    int signal_level = get_signal_strength();

    if(signal_level == 0) {
        lv_obj_add_flag(ui->img_4g, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui->img_4g, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(ui->img_4g, signal_level == 1   ? &_4g_1_RGB565A8_24x17
                                   : signal_level == 2 ? &_4g_2_RGB565A8_24x17
                                   : signal_level == 3 ? &_4g_3_RGB565A8_24x17
                                                       : &_4g_4_RGB565A8_24x17);
    }
}
static void wifi_update_cb(lv_timer_t *t)
{
    lv_ui_t *ui    = (lv_ui_t *)lv_timer_get_user_data(t);
    int wifi_level = get_wifi_strength();

    if(wifi_level == 0) {
        lv_obj_add_flag(ui->img_wifi, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui->img_wifi, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(ui->img_wifi, wifi_level == 1   ? &_wifi_1_RGB565A8_24x23
                                     : wifi_level == 2 ? &_wifi_2_RGB565A8_24x23
                                     : wifi_level == 3 ? &_wifi_3_RGB565A8_24x23
                                                       : &_wifi_4_RGB565A8_24x23);
    }
}

// 充电状态切换回调
static void charging_toggle_cb(lv_timer_t *timer)
{
    UNUSED(timer);
    power_charging = !power_charging;
}

static void power_update_cb(lv_timer_t *t)
{
    lv_ui_t *ui     = (lv_ui_t *)lv_timer_get_user_data(t);
    int power_level = get_power_strength();

    if(power_charging) {
        lv_obj_clear_flag(ui->img_power, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(ui->img_power, &_power_charging_RGB565A8_33x28);
    } else {
        if(power_level == 0) {
            lv_obj_add_flag(ui->img_power, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(ui->img_power, LV_OBJ_FLAG_HIDDEN);
            lv_img_set_src(ui->img_power, power_level == 1   ? &_power0_red_RGB565A8_33x28
                                          : power_level == 2 ? &_power_1_RGB565A8_33x28
                                          : power_level == 3 ? &_power_2_RGB565A8_33x28
                                                             : &_power_3_RGB565A8_33x28);
        }
    }
}

void custom_init(lv_ui_t *ui)
{
    /* Add your codes here */
    /* 初始化定时器管理器 */
    memset(&timers, 0, sizeof(timer_manager_t));

    // 创建共享的顶部状态栏对象
    ui->status_bar = lv_obj_create(lv_layer_top()); // 创建在顶层对象上
    lv_obj_set_size(ui->status_bar, LV_PCT(100), 40);
    lv_obj_set_pos(ui->status_bar, 0, 0);
    lv_obj_set_style_bg_color(ui->status_bar, lv_color_hex(0xFFD400), LV_PART_MAIN | LV_STATE_DEFAULT); // 明亮的黄色
    lv_obj_set_style_bg_opa(ui->status_bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT); // 完全不透明
    lv_obj_set_style_border_width(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置状态栏为浮动对象，这样它就会始终显示在最上层
    lv_obj_add_flag(ui->status_bar, LV_OBJ_FLAG_FLOATING);

    // 创建共享的4G图标
    ui->img_4g = lv_img_create(ui->status_bar);
    lv_obj_set_pos(ui->img_4g, 497, 11); // 调整y位置到11
    lv_obj_set_size(ui->img_4g, 24, 17);
    lv_obj_add_flag(ui->img_4g, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_pivot(ui->img_4g, 50, 50);
    lv_image_set_rotation(ui->img_4g, 0);
    lv_obj_set_style_image_recolor_opa(ui->img_4g, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->img_4g, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建共享的WiFi图标
    ui->img_wifi = lv_img_create(ui->status_bar);
    lv_obj_set_pos(ui->img_wifi, 535, 8); // 调整y位置到8
    lv_obj_set_size(ui->img_wifi, 24, 23);
    lv_obj_add_flag(ui->img_wifi, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_pivot(ui->img_wifi, 50, 50);
    lv_image_set_rotation(ui->img_wifi, 0);
    lv_obj_set_style_image_recolor_opa(ui->img_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->img_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建共享的电源图标
    ui->img_power = lv_img_create(ui->status_bar);
    lv_obj_set_pos(ui->img_power, 583, 6); // 调整y位置到6
    lv_obj_set_size(ui->img_power, 33, 28);
    lv_obj_add_flag(ui->img_power, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_pivot(ui->img_power, 50, 50);
    lv_image_set_rotation(ui->img_power, 0);
    lv_obj_set_style_image_recolor_opa(ui->img_power, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->img_power, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建共享的日期文本
    ui->date_text = lv_label_create(ui->status_bar);
    lv_obj_set_pos(ui->date_text, 20, 11); // 调整y位置到11
    lv_obj_set_style_text_color(ui->date_text, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 保持黑色
    lv_obj_set_style_text_font(ui->date_text, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建共享的数字时钟
    ui->digital_clock = lv_label_create(ui->status_bar);
    lv_obj_set_pos(ui->digital_clock, 140, 11); // 调整y位置到11
    lv_obj_set_size(ui->digital_clock, 60, 23); // 增加宽度以确保时间显示在一行
    lv_obj_set_style_text_color(ui->digital_clock, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 保持黑色
    lv_obj_set_style_text_font(ui->digital_clock, &lv_font_montserratMedium_17, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->digital_clock, LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN | LV_STATE_DEFAULT); // 确保文本左对齐

    // 创建定时器
    timers.signal_timer = lv_timer_create(signal_update_cb, 2000, ui);
    timers.wifi_timer   = lv_timer_create(wifi_update_cb, 2000, ui);
    timers.power_timer  = lv_timer_create(power_update_cb, 2000, ui);
    timers.charge_timer = lv_timer_create(charging_toggle_cb, 10000, NULL);
    timers.date_timer   = lv_timer_create(sys_time_update, 3000, ui);

    // 立即执行一次更新
    lv_timer_ready(timers.signal_timer);
    lv_timer_ready(timers.wifi_timer);
    lv_timer_ready(timers.charge_timer);
    lv_timer_ready(timers.power_timer);
    lv_timer_ready(timers.date_timer);

    setup_scr_screen_VolumeOverlay(ui);
}

void font_setting(lv_obj_t *obj, font_id_t font_id, font_size_config_t size_cfg, font_color_t color, int16_t x,
                  int16_t y)
{
    const lv_font_t *base_font = NULL;
    uint16_t base_line_height  = 32;

    switch(font_id) {
        case FONT_SC16:
            base_font        = &lv_font_SourceHanSerifSC_Regular_16;
            base_line_height = lv_font_SourceHanSerifSC_Regular_16.line_height;
            break;
        case FONT_DEFAULT:
        default:
            base_font        = LV_FONT_DEFAULT;
            base_line_height = base_font->line_height;
            break;
    }

    lv_color_t lv_color = lv_color_hex(color); // 转换颜色

    lv_obj_set_style_text_font(obj, base_font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color, LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(obj, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);

    // 缩放
    int zoom = 256;
    if(size_cfg.mode == FONT_SIZE_MODE_CUSTOM) {
        zoom = (int)(((float)size_cfg.size / (float)base_line_height) * 256.0f);
    }
    lv_obj_set_style_transform_zoom(obj, zoom, LV_STATE_DEFAULT);
}

void set_chs_fonts(const char *font_path, int font_size, lv_style_t *style) // 改为指针参数
{
    font_chs = lv_freetype_font_create(font_path,                           // 字体文件路径
                                       LV_FREETYPE_FONT_RENDER_MODE_BITMAP, // 渲染模式（位图或轮廓）
                                       font_size,                           // 字体大小（像素）
                                       LV_FREETYPE_FONT_STYLE_NORMAL        // 字体样式（普通/粗体/斜体）
    );

    if(!font_chs) {
        MLOG_DBG("create fzyt failed, fontpath: %s\n", font_path);
        return;
    }

    // 不要重新初始化样式，只设置字体相关属性
    lv_style_set_text_font(style, font_chs); // 设置字体
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
}

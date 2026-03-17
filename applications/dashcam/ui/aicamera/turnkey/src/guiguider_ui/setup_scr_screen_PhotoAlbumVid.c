/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"

static bool is_playing = true;
// 速度控制参数
typedef enum {
    SPEED_075X = 1333, // 0.75倍速（周期增大）
    SPEED_1X   = 1000, // 1倍速（每秒更新一次）
    SPEED_15X  = 667   // 1.5倍速（周期减小）
} playback_speed_t;

static lv_timer_t *sliderAlbumVid_timer = NULL;
static lv_timer_t *timeUpdate_timer     = NULL; // 新增时间更新定时器

static playback_speed_t current_speed = SPEED_1X;

// 视频时长（秒）
static int video_duration = 0;

// 时间标签
static lv_obj_t *current_time_label   = NULL; // 已播放时长
static lv_obj_t *total_time_label     = NULL; // 总时长
static lv_obj_t *remaining_time_label = NULL; // 剩余时长

// 当前播放时间（秒）
static int current_play_time = 0;

// 格式化时间函数
static void format_time(char *buf, int seconds)
{
    uint8_t minutes = seconds / 60;
    uint8_t secs    = seconds % 60;
    snprintf(buf, 10, "%d:%02d", minutes, secs);
}

// 更新所有时间标签
static void update_time_labels(int current_seconds)
{
    char time_str[10];

    // 更新已播放时长
    format_time(time_str, current_seconds);
    lv_label_set_text(current_time_label, time_str);

    // 更新总时长
    format_time(time_str, video_duration);
    lv_label_set_text(total_time_label, time_str);

    // 更新剩余时长
    format_time(time_str, video_duration - current_seconds);
    lv_label_set_text(remaining_time_label, time_str);
}

// 时间更新回调
static void time_update_cb(lv_timer_t *timer)
{
    UNUSED(timer);
    if(!is_playing) return;

    // 根据速度更新当前时间
    switch(current_speed) {
        case SPEED_075X:
            current_play_time += 1; // 每秒增加1秒
            break;
        case SPEED_1X:
            current_play_time += 1; // 每秒增加1秒
            break;
        case SPEED_15X:
            current_play_time += 1; // 每秒增加1秒
            break;
    }

    // 检查是否超过总时长
    if(current_play_time >= video_duration) {
        current_play_time = 0;
    }

    // 更新时间标签
    update_time_labels(current_play_time);

    // 更新进度条（直接使用当前时间作为进度值）
    lv_slider_set_value(g_ui.screen_PhotoAlbumVid_slider, current_play_time, LV_ANIM_ON);
}

static void screen_PhotoAlbumVid_btn_stop_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_flag(g_ui.screen_PhotoAlbumVid_btn_stop, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.screen_PhotoAlbumVid_btn_play, LV_OBJ_FLAG_HIDDEN);
            is_playing = false;
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_play_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_flag(g_ui.screen_PhotoAlbumVid_btn_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.screen_PhotoAlbumVid_btn_stop, LV_OBJ_FLAG_HIDDEN);
            is_playing = true;
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_slider_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_SCROLL: {
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_15x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_state(g_ui.screen_PhotoAlbumVid_btn_15x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_15x, lv_color_hex(0xFFD600),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.screen_PhotoAlbumVid_btn_075x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_075x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.screen_PhotoAlbumVid_btn_1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_1x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            current_speed = SPEED_15X;
            if(sliderAlbumVid_timer) {
                lv_timer_set_period(sliderAlbumVid_timer, current_speed);
            }
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_1x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_state(g_ui.screen_PhotoAlbumVid_btn_1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_1x, lv_color_hex(0xFFD600),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.screen_PhotoAlbumVid_btn_075x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_075x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.screen_PhotoAlbumVid_btn_15x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_15x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            current_speed = SPEED_1X;
            if(sliderAlbumVid_timer) {
                lv_timer_set_period(sliderAlbumVid_timer, current_speed);
            }
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_075x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_state(g_ui.screen_PhotoAlbumVid_btn_075x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_075x, lv_color_hex(0xFFD600),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.screen_PhotoAlbumVid_btn_1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_1x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.screen_PhotoAlbumVid_btn_15x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.screen_PhotoAlbumVid_btn_15x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            current_speed = SPEED_075X;
            if(sliderAlbumVid_timer) {
                lv_timer_set_period(sliderAlbumVid_timer, current_speed);
            }
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_photoalbum.scr, g_ui.screen_PhotoAlbum_del,
                                  &g_ui.screen_PhotoAlbumVid_del, setup_scr_screen_PhotoAlbum, LV_SCR_LOAD_ANIM_NONE,
                                  200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void events_init_screen_PhotoAlbumVid(lv_ui_t *ui)
{
    // lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_stop, screen_PhotoAlbumVid_btn_stop_event_handler, LV_EVENT_ALL,
    // ui); lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_play, screen_PhotoAlbumVid_btn_play_event_handler,
    // LV_EVENT_ALL, ui); lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_slider,
    // screen_PhotoAlbumVid_slider_event_handler, LV_EVENT_ALL, ui);
    // lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_15x, screen_PhotoAlbumVid_btn_15x_event_handler, LV_EVENT_ALL,
    // ui); lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_1x, screen_PhotoAlbumVid_btn_1x_event_handler,
    // LV_EVENT_ALL, ui); lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_075x,
    // screen_PhotoAlbumVid_btn_075x_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_back, screen_PhotoAlbumVid_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

lv_obj_t *time_label;

// 设置视频时长并更新UI
void set_video_duration(int duration)
{
    video_duration = duration;

    // 更新总时长标签
    char time_str[10];
    format_time(time_str, duration);
    lv_label_set_text(total_time_label, time_str);

    // 设置进度条范围（总步数等于视频时长）
    lv_slider_set_range(g_ui.screen_PhotoAlbumVid_slider, 0, duration);

    // 重置进度条和时间标签
    lv_slider_set_value(g_ui.screen_PhotoAlbumVid_slider, 0, LV_ANIM_OFF);
    update_time_labels(0);
    current_play_time = 0;
}

void setup_scr_screen_PhotoAlbumVid(lv_ui_t *ui)
{
    // Write codes screen_PhotoAlbumVid
    ui->screen_PhotoAlbumVid = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_PhotoAlbumVid, 640, 480);
    lv_obj_set_scrollbar_mode(ui->screen_PhotoAlbumVid, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_PhotoAlbumVid, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid, LV_OPA_0, LV_PART_MAIN);

    // Write codes screen_PhotoAlbumVid_cont_top (顶部栏)
    ui->screen_PhotoAlbumVid_cont_top = lv_obj_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_cont_top, 0, 0);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(ui->screen_PhotoAlbumVid_cont_top, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_PhotoAlbumVid_cont_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_cont_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_cont_top, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_cont_top, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_back (返回按钮)
    ui->screen_PhotoAlbumVid_btn_back = lv_button_create(ui->screen_PhotoAlbumVid_cont_top);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_back, 0, 4);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_back, 60, 50);
    ui->screen_PhotoAlbumVid_btn_back_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_back);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_back_label, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_back_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_btn_back, lv_color_hex(0xFFD600),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_back, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_back, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_back, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_delete (删除按钮)
    ui->screen_PhotoAlbumVid_btn_delete = lv_button_create(ui->screen_PhotoAlbumVid_cont_top);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_delete, 547, 12);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_delete, 64, 38);
    ui->screen_PhotoAlbumVid_btn_delete_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_delete);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_delete_label, " " LV_SYMBOL_TRASH " ");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_delete_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_delete_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_delete, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_delete_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_delete, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_delete, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_delete, lv_color_hex(0xFFD600),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_delete, &lv_font_SourceHanSerifSC_Regular_30,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_delete, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_delete, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_label_name (标题)
    ui->screen_PhotoAlbumVid_label_name = lv_label_create(ui->screen_PhotoAlbumVid_cont_top);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_label_name, 269, 23);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_label_name, 101, 18);
    lv_label_set_text(ui->screen_PhotoAlbumVid_label_name, "文件名");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_label_name, LV_LABEL_LONG_WRAP);

    // Write style for screen_PhotoAlbumVid_label_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_label_name, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_label_name, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_label_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_label_name, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_cont_bottom (底部栏)
    ui->screen_PhotoAlbumVid_cont_bottom = lv_obj_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_cont_bottom, 0, 420);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_cont_bottom, 640, 60);
    lv_obj_set_scrollbar_mode(ui->screen_PhotoAlbumVid_cont_bottom, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_PhotoAlbumVid_cont_bottom, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_cont_bottom, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_cont_bottom, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_cont_bottom, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_stop (暂停按钮)
    ui->screen_PhotoAlbumVid_btn_stop = lv_button_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_stop, 35, 377);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_stop, 34, 27);
    ui->screen_PhotoAlbumVid_btn_stop_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_stop);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_stop_label, "" LV_SYMBOL_PAUSE " ");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_stop_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_stop_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_stop, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_stop_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_stop, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_stop, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_btn_stop, lv_color_hex(0xFFD600),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_stop, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_stop, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_stop, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_stop, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_stop, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_stop, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_stop, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_play (播放按钮)
    ui->screen_PhotoAlbumVid_btn_play = lv_button_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_play, 35, 377);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_play, 34, 27);
    lv_obj_add_flag(ui->screen_PhotoAlbumVid_btn_play, LV_OBJ_FLAG_HIDDEN);
    ui->screen_PhotoAlbumVid_btn_play_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_play);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_play_label, "" LV_SYMBOL_PLAY " ");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_play_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_play_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_play, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_play_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_play, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_play, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_btn_play, lv_color_hex(0xFFD600),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_play, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_play, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_play, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_play, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_play, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_play, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_slider (进度条)
    ui->screen_PhotoAlbumVid_slider = lv_slider_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_slider, 131, 385);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_slider, 468, 12);
    // 初始范围设置为0-100，后续会根据视频时长调整
    lv_slider_set_range(ui->screen_PhotoAlbumVid_slider, 0, 100);
    lv_slider_set_mode(ui->screen_PhotoAlbumVid_slider, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_PhotoAlbumVid_slider, 0, LV_ANIM_OFF);

    // Write style for screen_PhotoAlbumVid_slider, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_slider, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_slider, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_slider, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_slider, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_PhotoAlbumVid_slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_slider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style for screen_PhotoAlbumVid_slider, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_slider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_slider, lv_color_hex(0xFFD600),
                              LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_slider, LV_GRAD_DIR_NONE,
                                 LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_slider, 8, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    // Write style for screen_PhotoAlbumVid_slider, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_slider, 255, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_slider, lv_color_hex(0xFFD600), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_slider, LV_GRAD_DIR_NONE, LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_slider, 8, LV_PART_KNOB | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_075x (0.75x速度按钮)
    ui->screen_PhotoAlbumVid_btn_075x = lv_button_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_075x, 188, 436);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_075x, 44, 34);
    ui->screen_PhotoAlbumVid_btn_075x_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_075x);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_075x_label, "0.75x");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_075x_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_075x_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_075x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_075x_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_075x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_075x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_btn_075x, lv_color_hex(0xCCCCCC),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_btn_075x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_075x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_075x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_075x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_075x, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_075x, &lv_font_montserratMedium_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_075x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_075x, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_1x (1x速度按钮)
    ui->screen_PhotoAlbumVid_btn_1x = lv_button_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_1x, 294, 436);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_1x, 44, 34);
    ui->screen_PhotoAlbumVid_btn_1x_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_1x);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_1x_label, "1x");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_1x_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_1x_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_1x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_1x_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_1x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_btn_1x, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_btn_1x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_1x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_1x, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_1x, &lv_font_montserratMedium_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_1x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumVid_btn_15x (1.5x速度按钮)
    ui->screen_PhotoAlbumVid_btn_15x = lv_button_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(ui->screen_PhotoAlbumVid_btn_15x, 406, 436);
    lv_obj_set_size(ui->screen_PhotoAlbumVid_btn_15x, 44, 34);
    ui->screen_PhotoAlbumVid_btn_15x_label = lv_label_create(ui->screen_PhotoAlbumVid_btn_15x);
    lv_label_set_text(ui->screen_PhotoAlbumVid_btn_15x_label, "1.5x");
    lv_label_set_long_mode(ui->screen_PhotoAlbumVid_btn_15x_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_PhotoAlbumVid_btn_15x_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_PhotoAlbumVid_btn_15x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_PhotoAlbumVid_btn_15x_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumVid_btn_15x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_PhotoAlbumVid_btn_15x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_PhotoAlbumVid_btn_15x, lv_color_hex(0xCCCCCC),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_PhotoAlbumVid_btn_15x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_PhotoAlbumVid_btn_15x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_PhotoAlbumVid_btn_15x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_PhotoAlbumVid_btn_15x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_PhotoAlbumVid_btn_15x, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_PhotoAlbumVid_btn_15x, &lv_font_montserratMedium_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_PhotoAlbumVid_btn_15x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_PhotoAlbumVid_btn_15x, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of screen_PhotoAlbumVid.
    //  设置按钮按压状态样式
    lv_obj_add_state(ui->screen_PhotoAlbumVid_btn_1x, LV_STATE_PRESSED);

    // 创建时间标签
    current_time_label = lv_label_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(current_time_label, 131, 365); // 进度条左上方
    lv_label_set_text(current_time_label, "0:00");
    lv_obj_set_style_text_color(current_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(current_time_label, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    total_time_label = lv_label_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(total_time_label, 320, 365); // 进度条中间
    lv_label_set_text(total_time_label, "0:00");
    lv_obj_set_style_text_color(total_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(total_time_label, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    remaining_time_label = lv_label_create(ui->screen_PhotoAlbumVid);
    lv_obj_set_pos(remaining_time_label, 550, 365); // 进度条右上方
    lv_label_set_text(remaining_time_label, "0:00");
    lv_obj_set_style_text_color(remaining_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(remaining_time_label, &lv_font_montserratMedium_12, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 初始化时间标签
    update_time_labels(0);

    // 创建定时器（固定每秒更新一次）
    timeUpdate_timer = lv_timer_create(time_update_cb, 1000, NULL);

    // 设置初始视频时长
    set_video_duration(20);

    // 注册事件处理函数
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_stop, screen_PhotoAlbumVid_btn_stop_event_handler, LV_EVENT_ALL,
                        NULL);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_play, screen_PhotoAlbumVid_btn_play_event_handler, LV_EVENT_ALL,
                        NULL);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_slider, screen_PhotoAlbumVid_slider_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_15x, screen_PhotoAlbumVid_btn_15x_event_handler, LV_EVENT_ALL,
                        NULL);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_1x, screen_PhotoAlbumVid_btn_1x_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_075x, screen_PhotoAlbumVid_btn_075x_event_handler, LV_EVENT_ALL,
                        NULL);
    lv_obj_add_event_cb(ui->screen_PhotoAlbumVid_btn_back, screen_PhotoAlbumVid_btn_back_event_handler, LV_EVENT_ALL,
                        NULL);

    // Update current screen layout.
    lv_obj_update_layout(ui->screen_PhotoAlbumVid);

    // Init events for screen.
    events_init_screen_PhotoAlbumVid(ui);
}

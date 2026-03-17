/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "config.h"
#include "custom.h"
#include "ui_common.h"
// 添加音效播放相关头文件
#include "voiceplay.h"
// 添加硬件按键相关头文件
#include <linux/input.h>
#include "indev.h"

static TakePhoto_t *TakePhoto;

extern char g_button_labelSel[32];

// 添加录像相关的全局变量
static bool is_video_mode = false;           // 是否处于录像模式
static bool is_recording = false;            // 是否正在录像
static int recording_seconds = 0;            // 录像计时（秒）
// static lv_timer_t *recording_timer = NULL;   // 录像计时器
// static lv_obj_t *video_circle_container = NULL; // 外部白色圆圈容器

// 添加全局变量用于记录定时拍照模式
static bool is_timer_photo_mode = false;
static int timer_photo_seconds  = 0;

// 录像计时器回调函数
static void recording_timer_cb(lv_timer_t *timer)
{
    UNUSED(timer);
    recording_seconds++;

    // 更新录像时间显示（格式：MM:SS）
    int minutes = recording_seconds / 60;
    int seconds = recording_seconds % 60;
    char recording_text[32];
    snprintf(recording_text, sizeof(recording_text), "%02d:%02d", minutes, seconds);
    lv_label_set_text(TakePhoto->label_video_timer, recording_text);
}
// 录像模式下设置红色拍照按钮和计时器标签的接口函数
static void setup_video_mode_style(void)
{
    MLOG_DBG("setup_video_mode_style\n");
    is_video_mode = true;
    is_recording = false;
    recording_seconds = 0;

    if(TakePhoto->btn_temp_photo == NULL) {
        TakePhoto->btn_temp_photo = lv_obj_create(TakePhoto->scr);
        lv_obj_set_size(TakePhoto->btn_temp_photo, 100, 100);
        lv_obj_set_pos(TakePhoto->btn_temp_photo, 520, 200);
        lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    // 将拍照按钮变为红色，不添加文字
    lv_obj_set_style_bg_color(TakePhoto->btn_temp_photo, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(TakePhoto->label_temp_photo, "");

    // 创建外部白色圆圈容器
    if(TakePhoto->video_circle_container != NULL) {
        // 设置容器可点击，与btn_temp_photo事件一致
        lv_obj_clear_flag(TakePhoto->video_circle_container, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(TakePhoto->video_circle_container, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(TakePhoto->video_circle_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(TakePhoto->video_circle_container, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // 显示计时器标签并移动到顶部栏中间位置
    lv_obj_clear_flag(TakePhoto->label_video_timer, LV_OBJ_FLAG_HIDDEN);
}

// 退出录像模式
static void exit_video_mode_style(void)
{
    MLOG_DBG("exit_video_mode_style\n");
    is_video_mode = false;
    is_recording = true;
    recording_seconds = 0;

    // 停止录像计时器
    if(TakePhoto->recording_timer != NULL) {
        lv_timer_del(TakePhoto->recording_timer);
        TakePhoto->recording_timer = NULL;
    }

    lv_obj_add_flag(TakePhoto->video_circle_container, LV_OBJ_FLAG_HIDDEN);

    // 恢复拍照按钮样式
    lv_obj_set_style_bg_color(TakePhoto->btn_temp_photo, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(TakePhoto->btn_temp_photo, 100, 100);
    lv_obj_set_pos(TakePhoto->btn_temp_photo, 520, 200);
    lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(TakePhoto->label_temp_photo, "拍照");

    // 隐藏计时器标签
    lv_obj_add_flag(TakePhoto->label_video_timer, LV_OBJ_FLAG_HIDDEN);
}

static void screen_TakePhoto_btn_size05x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size1x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        }
        default: break;
    }
}

static void screen_TakePhoto_btn_size1x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size05x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

            MESSAGE_S event;
            memset(&event, 0, sizeof(event));
            event.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
            // window 0
            event.arg1 = 0;
            event.aszPayload[0] = '1';
            MODEMNG_SendMessage(&event);
            break;
        }
        default: break;
    }
}

static void screen_TakePhoto_btn_size2x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size05x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size1x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

            MESSAGE_S event;
            memset(&event, 0, sizeof(event));
            event.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
            // window 0
            event.arg1 = 0;
            event.aszPayload[0] = '2';
            MODEMNG_SendMessage(&event);
            break;
            break;
        }
        default: break;
    }
}

static void screen_TakePhoto_btn_size3x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size05x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size1x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

            MESSAGE_S event;
            memset(&event, 0, sizeof(event));
            event.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
            // window 0
            event.arg1 = 0;
            event.aszPayload[0] = '3';
            MODEMNG_SendMessage(&event);
            break;
        }
        default: break;
    }
}

/* 下方 录像功能按钮 */
static void screen_TakePhoto_btn_video_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_loopvideo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_loopvideo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_photo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_conti_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_conti_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_timed_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_timed_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_time_lapse_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

            // 进入录像模式
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_MOVIE;
            MODEMNG_SendMessage(&Msg);
            setup_video_mode_style();

            break;
        }
        default: break;
    }
}

static void screen_TakePhoto_btn_loopvideo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_conti_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_conti_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_timed_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_timed_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_time_lapse_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

            // 进入录像模式
            setup_video_mode_style();

            break;
        }
        default: break;
    }
}

/* 下方 缩时录像功能按钮 */
static void screen_TakePhoto_btn_time_lapse_video_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_loopvideo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_loopvideo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_photo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_conti_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_conti_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_timed_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_timed_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

            // 进入录像模式
            setup_video_mode_style();

            break;
        }
        default: break;
    }
}

static void screen_TakePhoto_btn_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_loopvideo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_loopvideo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_conti_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_conti_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_timed_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_timed_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_time_lapse_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

            // 退出录像模式
            exit_video_mode_style();

            // 退出定时拍照模式
            if(is_timer_photo_mode) {
                is_timer_photo_mode = false;
            }

            // 进入拍照模式
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);

            break;
        }
        default: break;
    }
}

static void screen_TakePhoto_btn_conti_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_loopvideo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_loopvideo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_photo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_timed_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_timed_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_time_lapse_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

            // 退出录像模式
            exit_video_mode_style();

            // 退出定时拍照模式
            if(is_timer_photo_mode) {
                is_timer_photo_mode = false;
            }

            break;
        }
        default: break;
    }
}

// 倒计时更新回调
static void countdown_timer_cb(lv_timer_t *timer)
{
    MESSAGE_S Msg = {0};
    static int remaining_seconds = 0;
    static bool is_initialized = false;
    static int original_timer_seconds = 0; // 保存原始的倒计时时间

    if(!is_initialized) {
        // 第一次调用，初始化剩余时间
        remaining_seconds = timer_photo_seconds;
        original_timer_seconds = timer_photo_seconds; // 保存原始时间
        is_initialized = true;
    }

    // 更新倒计时显示
    char timer_text[32];
    snprintf(timer_text, sizeof(timer_text), "%ds", remaining_seconds);
    lv_label_set_text(TakePhoto->label_timer, timer_text);

    remaining_seconds--;

    if(remaining_seconds < 0) {
        // 倒计时结束，删除定时器
        lv_timer_del(timer);
        // 隐藏倒计时标签
        lv_obj_add_flag(TakePhoto->label_timer, LV_OBJ_FLAG_HIDDEN);
        // 执行拍照操作
        printf("拍照操作执行\n");
        // 恢复原始的倒计时时间，以便下次可以重新开始倒计时
        timer_photo_seconds = original_timer_seconds;
        // 重置静态变量
        remaining_seconds = 0;
        is_initialized = false;
        // 注意：不重置 is_timer_photo_mode，保持定时拍照模式
        Msg.topic = EVENT_MODEMNG_START_PIV;
        MODEMNG_SendMessage(&Msg);
    }
}

/* 下方 定时拍照按钮 */
static void screen_TakePhoto_btn_timed_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_PRESSED);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(TakePhoto->btn_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_loopvideo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_loopvideo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_photo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_conti_photo, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_conti_photo, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(TakePhoto->btn_time_lapse_video, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);

             // 退出录像模式
            exit_video_mode_style();

            // 设置定时拍照模式
            is_timer_photo_mode = true;
            // 根据g_button_labelSel设置延时
            if(strcmp(g_button_labelSel, "定时2s") == 0) {
                timer_photo_seconds = 2;
            } else if(strcmp(g_button_labelSel, "定时5s") == 0) {
                timer_photo_seconds = 5;
            } else if(strcmp(g_button_labelSel, "定时10s") == 0) {
                timer_photo_seconds = 10;
            }

            // 恢复拍照按钮样式
            lv_obj_set_style_bg_color(TakePhoto->btn_temp_photo, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_size(TakePhoto->btn_temp_photo, 100, 100);
            lv_obj_set_pos(TakePhoto->btn_temp_photo, 520, 200);
            lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(TakePhoto->label_temp_photo, "拍照");

            // 隐藏计时器标签
            lv_obj_add_flag(TakePhoto->label_timer, LV_OBJ_FLAG_HIDDEN);

            // 进入拍照模式
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);

            break;
        }
        default: break;
    }
}

/* 拍照按钮 */
static void screen_TakePhoto_btn_temp_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MESSAGE_S Msg = {0};

    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    if(code == LV_EVENT_CLICKED) {
        if(is_video_mode) {
            MLOG_DBG("is_video_mode\n");
            // 录像模式
            if(!is_recording) {
                MLOG_DBG("start recording\n");
                // 开始录像
                is_recording = true;
                recording_seconds = 0;

                // 创建录像计时器
                if(TakePhoto->recording_timer == NULL) {
                    TakePhoto->recording_timer = lv_timer_create(recording_timer_cb, 1000, NULL);
                }

                // 将按钮变为较小的红色圆角矩形
                lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 8, LV_PART_MAIN | LV_STATE_DEFAULT); // 添加圆角
                lv_obj_set_size(TakePhoto->btn_temp_photo, 40, 40); // 缩小按钮尺寸
                // 调整位置使矩形居中显示在白色圆圈内
                // 外部圆圈位置：(517, 197)，大小：106x106
                // 内部矩形大小：40x40
                // 居中位置：517 + (106-40)/2 = 517 + 33 = 550, 197 + (106-40)/2 = 197 + 33 = 230
                lv_obj_set_pos(TakePhoto->btn_temp_photo, 550, 230); // 调整位置保持居中
                // 移除按钮的白色边框
                lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

                // 通知底层 开始录像
                Msg.topic = EVENT_MODEMNG_START_REC;
                MODEMNG_SendMessage(&Msg);

                printf("开始录像\n");
            } else {
                MLOG_DBG("stop recording\n");
                // 停止录像
                is_recording = false;

                // 停止录像计时器
                if(TakePhoto->recording_timer != NULL) {
                    lv_timer_del(TakePhoto->recording_timer);
                    TakePhoto->recording_timer = NULL;
                }
                // 通知底层 停止录像
                Msg.topic = EVENT_MODEMNG_STOP_REC;
                MODEMNG_SendMessage(&Msg);

                // 恢复按钮样式
                lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_size(TakePhoto->btn_temp_photo, 100, 100); // 恢复原始尺寸
                lv_obj_set_pos(TakePhoto->btn_temp_photo, 520, 200); // 恢复原始位置
                // 移除按钮的白色边框
                lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

                // 外部白色圆圈容器保持106x106像素不变

                // 计时归零
                lv_label_set_text(TakePhoto->label_video_timer, "00:00");

                printf("停止录像，总时长: %02d:%02d\n", recording_seconds / 60, recording_seconds % 60);
            }
        } else if(is_timer_photo_mode && timer_photo_seconds > 0) {
            MLOG_DBG("is_timer_photo_mode\n");
            // 定时拍照模式
            // 显示定时器标签并移动到屏幕中间的偏下方
            lv_obj_clear_flag(TakePhoto->label_timer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(TakePhoto->label_timer, 270, 300); // 移动到屏幕中间的偏下方

            // 创建倒计时定时器（每秒更新一次）
            lv_timer_t *timer = lv_timer_create(countdown_timer_cb, 1000, NULL);

            // 立即执行一次更新，显示初始倒计时
            lv_timer_ready(timer);
        } else {
            MLOG_DBG("is_direct_photo_mode 直接拍照操作执行\n");
            Msg.topic = EVENT_MODEMNG_START_PIV;
            MODEMNG_SendMessage(&Msg);
            ui_common_wait_piv_end();
        }
    }
}

/* 右上角 设置按钮 */
static void screen_TakePhoto_btn_setting_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_TakePhoto_del, setup_scr_screen_TakePhotoSetting, LV_SCR_LOAD_ANIM_NONE,
                                  200, 200, false, true);
            break;
        }
        default: break;
    }
}

/* 左上角 返回按钮 */
static void screen_TakePhoto_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            if(is_video_mode) {
                exit_video_mode_style();
            }
            // 进入BOOT模式
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_BOOT;
            MODEMNG_SendMessage(&Msg);

            // 退出音效播放
            pthread_t cleanup_thread;
            pthread_create(&cleanup_thread, NULL, UI_VOICEPLAY_DeInit, NULL);
            pthread_detach(cleanup_thread); // 分离线程，自动清理
            // 设置当前页面按键处理回调为NULL
            set_current_page_handler(NULL);

            ui_load_scr_animation(&g_ui, &g_ui.page_home1.scr, g_ui.screenHome1_del, &g_ui.screen_TakePhoto_del,
                                  setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

// TakePhoto页面的统一按键处理回调函数
static void takephoto_key_handler(int key_code, int key_value)
{
    // 检测按键类型
    button_type_t button_type = BUTTON_TYPE_NONE;
    if (key_value == 1) {
        if (key_code == KEY_CAMERA_FOCUS) {
            button_type = BUTTON_TYPE_FOCUS; // 对焦
        } else if (key_code == KEY_CAMERA) {
            button_type = BUTTON_TYPE_PHOTO; // 拍照
        }
    }

    bool pressed = (button_type != BUTTON_TYPE_NONE);

    MESSAGE_S Msg = {0};

    // 检测按键按下
    if (pressed) {
        MLOG_DBG("Unified Hardware button pressed, type: %d\n", button_type);

        if (button_type == BUTTON_TYPE_FOCUS) {
            // 对焦按键 (KEY_CAMERA_FOCUS)
            MLOG_DBG("执行对焦操作\n");
        } else if (button_type == BUTTON_TYPE_PHOTO) {
            // 拍照按键 (KEY_CAMERA)
            MLOG_DBG("Camera button pressed\n");

            if(is_video_mode) {
                MLOG_DBG("is_video_mode\n");
                // 录像模式
                if(!is_recording) {
                    MLOG_DBG("start recording\n");
                    // 开始录像
                    is_recording = true;
                    recording_seconds = 0;

                    // 创建录像计时器
                    if(TakePhoto->recording_timer == NULL) {
                        TakePhoto->recording_timer = lv_timer_create(recording_timer_cb, 1000, NULL);
                    }

                    // 将按钮变为较小的红色圆角矩形
                    lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 8, LV_PART_MAIN | LV_STATE_DEFAULT); // 添加圆角
                    lv_obj_set_size(TakePhoto->btn_temp_photo, 40, 40); // 缩小按钮尺寸
                    // 调整位置使矩形居中显示在白色圆圈内
                    // 外部圆圈位置：(517, 197)，大小：106x106
                    // 内部矩形大小：40x40
                    // 居中位置：517 + (106-40)/2 = 517 + 33 = 550, 197 + (106-40)/2 = 197 + 33 = 230
                    lv_obj_set_pos(TakePhoto->btn_temp_photo, 550, 230); // 调整位置保持居中
                    // 移除按钮的白色边框
                    lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

                    // 通知底层 开始录像
                    Msg.topic = EVENT_MODEMNG_START_REC;
                    MODEMNG_SendMessage(&Msg);

                    printf("开始录像\n");
                } else {
                    MLOG_DBG("stop recording\n");
                    // 停止录像
                    is_recording = false;

                    // 停止录像计时器
                    if(TakePhoto->recording_timer != NULL) {
                        lv_timer_del(TakePhoto->recording_timer);
                        TakePhoto->recording_timer = NULL;
                    }
                    // 通知底层 停止录像
                    Msg.topic = EVENT_MODEMNG_STOP_REC;
                    MODEMNG_SendMessage(&Msg);

                    // 恢复按钮样式
                    lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_size(TakePhoto->btn_temp_photo, 100, 100); // 恢复原始尺寸
                    lv_obj_set_pos(TakePhoto->btn_temp_photo, 520, 200); // 恢复原始位置
                    // 移除按钮的白色边框
                    lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

                    // 外部白色圆圈容器保持106x106像素不变

                    // 计时归零
                    lv_label_set_text(TakePhoto->label_video_timer, "00:00");

                    printf("停止录像，总时长: %02d:%02d\n", recording_seconds / 60, recording_seconds % 60);
                }
            } else if(is_timer_photo_mode && timer_photo_seconds > 0) {
                MLOG_DBG("is_timer_photo_mode\n");
                // 定时拍照模式
                // 显示定时器标签并移动到屏幕中间的偏下方
                lv_obj_clear_flag(TakePhoto->label_timer, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_pos(TakePhoto->label_timer, 270, 300); // 移动到屏幕中间的偏下方

                // 创建倒计时定时器（每秒更新一次）
                lv_timer_t *timer = lv_timer_create(countdown_timer_cb, 1000, NULL);

                // 立即执行一次更新，显示初始倒计时
                lv_timer_ready(timer);
            } else {
                MLOG_DBG("is_direct_photo_mode 直接拍照操作执行\n");
                Msg.topic = EVENT_MODEMNG_START_PIV;
                MODEMNG_SendMessage(&Msg);
                ui_common_wait_piv_end();
            }
        }
    }
}

void events_init_screen_TakePhoto(lv_ui_t *ui)
{
    lv_obj_add_event_cb(TakePhoto->btn_size05x, screen_TakePhoto_btn_size05x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(TakePhoto->btn_size1x, screen_TakePhoto_btn_size1x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(TakePhoto->btn_size2x, screen_TakePhoto_btn_size2x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(TakePhoto->btn_size3x, screen_TakePhoto_btn_size3x_event_handler, LV_EVENT_CLICKED, ui);
    // 录像功能按钮
    lv_obj_add_event_cb(TakePhoto->btn_video, screen_TakePhoto_btn_video_event_handler, LV_EVENT_CLICKED, ui);
    // 循环录像功能按钮
    lv_obj_add_event_cb(TakePhoto->btn_loopvideo, screen_TakePhoto_btn_loopvideo_event_handler, LV_EVENT_CLICKED, ui);
    // 缩时录像功能按钮
    lv_obj_add_event_cb(TakePhoto->btn_time_lapse_video, screen_TakePhoto_btn_time_lapse_video_event_handler,
                        LV_EVENT_CLICKED, ui);
    // 拍照功能按钮
    lv_obj_add_event_cb(TakePhoto->btn_photo, screen_TakePhoto_btn_photo_event_handler, LV_EVENT_CLICKED, ui);
    // 连续拍照功能按钮
    lv_obj_add_event_cb(TakePhoto->btn_conti_photo, screen_TakePhoto_btn_conti_photo_event_handler, LV_EVENT_CLICKED,
                        ui);
    // 定时拍照功能按钮
    lv_obj_add_event_cb(TakePhoto->btn_timed_photo, screen_TakePhoto_btn_timed_photo_event_handler, LV_EVENT_CLICKED,
                        ui);
    // 设置按钮
    lv_obj_add_event_cb(TakePhoto->btn_setting, screen_TakePhoto_btn_setting_event_handler, LV_EVENT_CLICKED, ui);
    // 返回按钮
    lv_obj_add_event_cb(TakePhoto->btn_back, screen_TakePhoto_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    // 拍照按钮
    lv_obj_add_event_cb(TakePhoto->btn_temp_photo, screen_TakePhoto_btn_temp_photo_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_TakePhoto(lv_ui_t *ui)
{

    MLOG_DBG("loading pageTakePhoto...\n");

    TakePhoto      = &ui->page_takephoto;
    TakePhoto->del = true;

    // 创建主页面1 容器
    if(TakePhoto->scr != NULL) {
        if(lv_obj_is_valid(TakePhoto->scr)) {
            MLOG_DBG("page_TakePhoto->scr 仍然有效，删除旧对象\n");
            lv_obj_del(TakePhoto->scr);
        } else {
            MLOG_DBG("page_TakePhoto->scr 已被自动销毁，仅重置指针\n");
        }
        TakePhoto->scr = NULL;
    }

    // Write codes screen_TakePhoto
    TakePhoto->scr = lv_obj_create(NULL);
    lv_obj_set_size(TakePhoto->scr, 640, 480);
    lv_obj_set_scrollbar_mode(TakePhoto->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_TakePhoto, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(TakePhoto->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont
    TakePhoto->bottom_cont = lv_obj_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->bottom_cont, 0, 420);
    lv_obj_set_size(TakePhoto->bottom_cont, 640, 60);
    lv_obj_set_scrollbar_mode(TakePhoto->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(TakePhoto->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(TakePhoto->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(TakePhoto->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(TakePhoto->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size05x
    TakePhoto->btn_size05x = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_size05x, 195, 360);
    lv_obj_set_size(TakePhoto->btn_size05x, 50, 30);
    TakePhoto->label_size05x = lv_label_create(TakePhoto->btn_size05x);
    lv_label_set_text(TakePhoto->label_size05x, "0.5x");
    lv_label_set_long_mode(TakePhoto->label_size05x, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_size05x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_size05x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_size05x, LV_PCT(100));

    // Write style for btn_size05x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_size05x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_size05x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_size05x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_size05x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_size05x, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_size05x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_size05x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_size05x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_size05x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_size05x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size1x
    TakePhoto->btn_size1x = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_size1x, 265, 360);
    lv_obj_set_size(TakePhoto->btn_size1x, 50, 30);
    TakePhoto->label_size1x = lv_label_create(TakePhoto->btn_size1x);
    lv_label_set_text(TakePhoto->label_size1x, "1x");
    lv_label_set_long_mode(TakePhoto->label_size1x, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_size1x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_size1x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_size1x, LV_PCT(100));

    // Write style for btn_size1x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_size1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_size1x, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_size1x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_size1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_size1x, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_size1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_size1x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_size1x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_size1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_size1x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size2x
    TakePhoto->btn_size2x = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_size2x, 335, 360);
    lv_obj_set_size(TakePhoto->btn_size2x, 50, 30);
    TakePhoto->label_size2x = lv_label_create(TakePhoto->btn_size2x);
    lv_label_set_text(TakePhoto->label_size2x, "2x");
    lv_label_set_long_mode(TakePhoto->label_size2x, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_size2x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_size2x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_size2x, LV_PCT(100));

    // Write style for btn_size2x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_size2x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_size2x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_size2x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_size2x, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_size2x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_size2x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_size2x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_size2x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_size2x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size3x
    TakePhoto->btn_size3x = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_size3x, 405, 360);
    lv_obj_set_size(TakePhoto->btn_size3x, 50, 30);
    TakePhoto->label_size3x = lv_label_create(TakePhoto->btn_size3x);
    lv_label_set_text(TakePhoto->label_size3x, "3x");
    lv_label_set_long_mode(TakePhoto->label_size3x, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_size3x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_size3x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_size3x, LV_PCT(100));

    // Write style for btn_size3x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_size3x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_size3x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_size3x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_size3x, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_size3x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_size3x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_size3x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_size3x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_size3x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_video
    TakePhoto->btn_video = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_video, 260, 430);
    lv_obj_set_size(TakePhoto->btn_video, 60, 35);
    TakePhoto->label_video = lv_label_create(TakePhoto->btn_video);
    lv_label_set_text(TakePhoto->label_video, "录像");
    lv_label_set_long_mode(TakePhoto->label_video, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_video, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_video, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_video, LV_PCT(100));

    // Write style for btn_video, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_video, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_video, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_video, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_video, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_video, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_video, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_video, &lv_font_SourceHanSerifSC_Regular_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_video, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_video, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_loopvideo
    TakePhoto->btn_loopvideo = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_loopvideo, 180, 430);
    lv_obj_set_size(TakePhoto->btn_loopvideo, 70, 35);
    TakePhoto->label_loopvideo = lv_label_create(TakePhoto->btn_loopvideo);
    lv_label_set_text(TakePhoto->label_loopvideo, "循环录像");
    lv_label_set_long_mode(TakePhoto->label_loopvideo, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_loopvideo, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_loopvideo, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_loopvideo, LV_PCT(100));

    // Write style for btn_loopvideo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_loopvideo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_loopvideo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_loopvideo, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_loopvideo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_loopvideo, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_loopvideo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_loopvideo, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_loopvideo, &lv_font_SourceHanSerifSC_Regular_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_loopvideo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_loopvideo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_photo
    TakePhoto->btn_photo = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_photo, 330, 430);
    lv_obj_set_size(TakePhoto->btn_photo, 60, 35);
    TakePhoto->label_photo = lv_label_create(TakePhoto->btn_photo);
    lv_label_set_text(TakePhoto->label_photo, "拍照");
    lv_label_set_long_mode(TakePhoto->label_photo, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_photo, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_photo, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_photo, LV_PCT(100));

    // Write style for btn_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_photo, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_photo, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_photo, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_photo, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_photo, &lv_font_SourceHanSerifSC_Regular_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_photo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_conti_photo
    TakePhoto->btn_conti_photo = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_conti_photo, 400, 430);
    lv_obj_set_size(TakePhoto->btn_conti_photo, 70, 35);
    TakePhoto->label_conti_photo = lv_label_create(TakePhoto->btn_conti_photo);
    lv_label_set_text(TakePhoto->label_conti_photo, "连续拍照");
    lv_label_set_long_mode(TakePhoto->label_conti_photo, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_conti_photo, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_conti_photo, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_conti_photo, LV_PCT(100));

    // Write style for btn_conti_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_conti_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_conti_photo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_conti_photo, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_conti_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_conti_photo, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_conti_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_conti_photo, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_conti_photo, &lv_font_SourceHanSerifSC_Regular_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_conti_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_conti_photo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_timed_photo
    TakePhoto->btn_timed_photo = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_timed_photo, 480, 430);
    lv_obj_set_size(TakePhoto->btn_timed_photo, 70, 35);
    TakePhoto->label_timed_photo = lv_label_create(TakePhoto->btn_timed_photo);
    lv_label_set_text(TakePhoto->label_timed_photo, "定时拍照");
    lv_label_set_long_mode(TakePhoto->label_timed_photo, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_timed_photo, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_timed_photo, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_timed_photo, LV_PCT(100));

    // Write style for btn_timed_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_timed_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_timed_photo, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_timed_photo, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_timed_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_timed_photo, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_timed_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_timed_photo, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_timed_photo, &lv_font_SourceHanSerifSC_Regular_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_timed_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_timed_photo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_time_lapse_video
    TakePhoto->btn_time_lapse_video = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_time_lapse_video, 100, 430);
    lv_obj_set_size(TakePhoto->btn_time_lapse_video, 70, 35);
    TakePhoto->label_time_lapse_video = lv_label_create(TakePhoto->btn_time_lapse_video);
    lv_label_set_text(TakePhoto->label_time_lapse_video, "缩时录像");
    lv_label_set_long_mode(TakePhoto->label_time_lapse_video, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_time_lapse_video, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_time_lapse_video, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_time_lapse_video, LV_PCT(100));

    // Write style for btn_time_lapse_video, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_time_lapse_video, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_time_lapse_video, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_time_lapse_video, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_time_lapse_video, 18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_time_lapse_video, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_time_lapse_video, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_time_lapse_video, &lv_font_SourceHanSerifSC_Regular_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_time_lapse_video, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_time_lapse_video, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_top
    TakePhoto->top_cont = lv_obj_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->top_cont, 0, 0);
    lv_obj_set_size(TakePhoto->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(TakePhoto->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(TakePhoto->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_setting
    TakePhoto->btn_setting = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_setting, 520, 4);
    lv_obj_set_size(TakePhoto->btn_setting, 100, 52);
    TakePhoto->label_setting = lv_label_create(TakePhoto->btn_setting);
    lv_label_set_text(TakePhoto->label_setting, "" LV_SYMBOL_LIST " ");
    lv_label_set_long_mode(TakePhoto->label_setting, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_setting, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_setting, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_setting, LV_PCT(100));

    // Write style for btn_setting, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_setting, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_setting, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_setting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_setting, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_setting, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_setting, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_setting, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_setting, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_setting, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    TakePhoto->btn_back = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_back, 4, 4);
    lv_obj_set_size(TakePhoto->btn_back, 60, 52);
    TakePhoto->label_back = lv_label_create(TakePhoto->btn_back);
    lv_label_set_text(TakePhoto->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(TakePhoto->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_back, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes label_timer
    TakePhoto->label_timer = lv_label_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->label_timer, 470, 420);
    lv_obj_set_size(TakePhoto->label_timer, 64, 20);
    lv_obj_set_style_text_font(TakePhoto->label_timer, &lv_font_montserratMedium_16, 0);
    lv_obj_set_style_text_color(TakePhoto->label_timer, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(TakePhoto->label_timer, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(TakePhoto->label_timer, "");
    // 初始时隐藏定时器标签
    lv_obj_add_flag(TakePhoto->label_timer, LV_OBJ_FLAG_HIDDEN);

    TakePhoto->label_video_timer = lv_label_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->label_video_timer, 270, 20);
    lv_obj_set_size(TakePhoto->label_video_timer, 75, 25);
    lv_label_set_text(TakePhoto->label_video_timer, "00:00");
    lv_obj_set_style_text_align(TakePhoto->label_video_timer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(TakePhoto->label_video_timer, &lv_font_montserratMedium_16, 0);
    lv_obj_set_style_text_color(TakePhoto->label_video_timer, lv_color_hex(0xFFFFFF), 0);
    // 为录像计时器添加红色背景
    lv_obj_set_style_bg_color(TakePhoto->label_video_timer, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_bg_opa(TakePhoto->label_video_timer, 255, 0);
    lv_obj_set_style_radius(TakePhoto->label_video_timer, 10, 0); // 添加圆角
    lv_obj_set_style_pad_all(TakePhoto->label_video_timer, 5, 0); // 添加内边距
    // 初始时隐藏录像计时器标签
    lv_obj_add_flag(TakePhoto->label_video_timer, LV_OBJ_FLAG_HIDDEN);

    // Write codes btn_temp_photo
    TakePhoto->btn_temp_photo = lv_button_create(TakePhoto->scr);
    lv_obj_set_pos(TakePhoto->btn_temp_photo, 520, 200);
    lv_obj_set_size(TakePhoto->btn_temp_photo, 100, 100);
    TakePhoto->label_temp_photo = lv_label_create(TakePhoto->btn_temp_photo);
    lv_label_set_text(TakePhoto->label_temp_photo, "拍照");
    lv_label_set_long_mode(TakePhoto->label_temp_photo, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhoto->label_temp_photo, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhoto->btn_temp_photo, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhoto->label_temp_photo, LV_PCT(100));

    // Write style for btn_temp_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhoto->btn_temp_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhoto->btn_temp_photo, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhoto->btn_temp_photo, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->btn_temp_photo, 50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhoto->btn_temp_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhoto->btn_temp_photo, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhoto->btn_temp_photo, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhoto->btn_temp_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhoto->btn_temp_photo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 录像模式下拍照按钮的白色圆圈容器
    TakePhoto->video_circle_container = lv_obj_create(TakePhoto->scr);
    lv_obj_set_size(TakePhoto->video_circle_container, 106, 106); // 比按钮大6像素
    lv_obj_set_pos(TakePhoto->video_circle_container, 517, 197); // 居中位置
    lv_obj_set_style_bg_opa(TakePhoto->video_circle_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_border_width(TakePhoto->video_circle_container, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(TakePhoto->video_circle_container, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhoto->video_circle_container, 53, LV_PART_MAIN | LV_STATE_DEFAULT); // 圆形
    lv_obj_set_style_border_side(TakePhoto->video_circle_container, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(TakePhoto->video_circle_container, LV_OBJ_FLAG_HIDDEN);

    // The custom code of takePhoto->scr.
    //  设置按钮按压状态样式
    lv_obj_add_state(TakePhoto->btn_size1x, LV_STATE_PRESSED);
    lv_obj_add_state(TakePhoto->btn_photo, LV_STATE_PRESSED);

    // 初始化音效播放
    UI_VOICEPLAY_Init();

    /* 设置当前页面和按键处理回调 */
    set_current_page_handler(takephoto_key_handler);

    // Update current screen layout.
    lv_obj_update_layout(TakePhoto->scr);

    // Init events for screen.
    events_init_screen_TakePhoto(ui);
}

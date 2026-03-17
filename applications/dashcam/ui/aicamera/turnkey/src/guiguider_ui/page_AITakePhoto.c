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
#include "mlog.h"
#include "custom.h"
#include "ui_common.h"
// 添加音效播放相关头文件
#include "voiceplay.h"
// 添加硬件按键相关头文件
#include <linux/input.h>
#include "indev.h"

static AiTakePhoto_t *AiTakePhoto;

// 硬件拍照按键读取回调函数
static void aitakephoto_key_handler(int key_code, int key_value)
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

    // 检测按键按下
    if (pressed) {
        MLOG_DBG("Hardware button pressed, type: %d\n", button_type);
        if (button_type == BUTTON_TYPE_FOCUS) {
            // 对焦按键 (KEY_CAMERA_FOCUS)
            MLOG_DBG("执行对焦操作\n");
        } else if (button_type == BUTTON_TYPE_PHOTO) {
            MLOG_DBG("拍照按键 (KEY_CAMERA) 按下\n");
            // 直接执行拍照业务逻辑，与UI按钮功能一致
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_START_PIV;
            MODEMNG_SendMessage(&Msg);
            ui_common_wait_piv_end();
            // 设置当前页面按键处理回调为NULL
            set_current_page_handler(NULL);
            // 异步处理音频清理
            pthread_t cleanup_thread;
            pthread_create(&cleanup_thread, NULL, UI_VOICEPLAY_DeInit_Delay, NULL);
            pthread_detach(cleanup_thread); // 分离线程，自动清理
            // 加载新页面
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphoto.scr, g_ui.screen_AIPhoto_del, &g_ui.screen_AITakePhoto_del,
                                setup_scr_screen_AIPhoto, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            return;
        }
    }
}

static void screen_AITakePhoto_btn_simulate_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_START_PIV;
            MODEMNG_SendMessage(&Msg);
            ui_common_wait_piv_end();
            // 异步处理音频清理
            pthread_t cleanup_thread;
            pthread_create(&cleanup_thread, NULL, UI_VOICEPLAY_DeInit_Delay, NULL);
            pthread_detach(cleanup_thread); // 分离线程，自动清理

            ui_load_scr_animation(&g_ui, &g_ui.page_aiphoto.scr, g_ui.screen_AIPhoto_del, &g_ui.screen_AITakePhoto_del,
                                  setup_scr_screen_AIPhoto, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}


static void screen_AITakePhoto_btn_size05x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size1x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

            break;
        }
        default: break;
    }
}

static void screen_AITakePhoto_btn_size1x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(AiTakePhoto->btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size05x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

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

static void screen_AITakePhoto_btn_size2x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(AiTakePhoto->btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size05x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size1x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

            MESSAGE_S event;
            memset(&event, 0, sizeof(event));
            event.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
            // window 0
            event.arg1 = 0;
            event.aszPayload[0] = '2';
            MODEMNG_SendMessage(&event);
            break;
        }
        default: break;
    }
}

static void screen_AITakePhoto_btn_size3x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取按钮对象
            lv_obj_t *btn = lv_event_get_target(e);
            // 设置按钮按压状态样式
            lv_obj_add_state(btn, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);

            // 清除其他按钮的按压状态并设置为灰色
            lv_obj_clear_state(AiTakePhoto->btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size05x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size1x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(AiTakePhoto->btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(AiTakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);

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

static void screen_AITakePhoto_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
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

            ui_load_scr_animation(&g_ui, &g_ui.page_home1.scr, g_ui.screenHome1_del, &g_ui.screen_AITakePhoto_del,
                                  setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_AITakePhoto(lv_ui_t *ui)
{
    lv_obj_add_event_cb(AiTakePhoto->btn_size05x, screen_AITakePhoto_btn_size05x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(AiTakePhoto->btn_size1x, screen_AITakePhoto_btn_size1x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(AiTakePhoto->btn_size2x, screen_AITakePhoto_btn_size2x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(AiTakePhoto->btn_size3x, screen_AITakePhoto_btn_size3x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(AiTakePhoto->btn_back, screen_AITakePhoto_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_AITakePhoto(lv_ui_t *ui)
{
    MLOG_DBG("loading pageAiTakePhoto...\n");

    AiTakePhoto      = &ui->page_aitakephoto;
    AiTakePhoto->del = true;

    // 创建主页面1 容器
    if(AiTakePhoto->scr != NULL) {
        if(lv_obj_is_valid(AiTakePhoto->scr)) {
            MLOG_DBG("pageAiTakePhoto->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AiTakePhoto->scr);
        } else {
            MLOG_DBG("pageAiTakePhoto->scr 已被自动销毁，仅重置指针\n");
        }
        AiTakePhoto->scr = NULL;
    }

    // Write codes screen_AITakePhoto
    AiTakePhoto->scr = lv_obj_create(NULL);
    lv_obj_set_size(AiTakePhoto->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AiTakePhoto->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_AITakePhoto, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AiTakePhoto->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(AiTakePhoto->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont
    AiTakePhoto->bottom_cont = lv_obj_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->bottom_cont, 0, 420);
    lv_obj_set_size(AiTakePhoto->bottom_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AiTakePhoto->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(AiTakePhoto->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AiTakePhoto->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(AiTakePhoto->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AiTakePhoto->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AiTakePhoto->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_btn_size05x
    AiTakePhoto->btn_size05x = lv_button_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->btn_size05x, 190, 436);
    lv_obj_set_size(AiTakePhoto->btn_size05x, 44, 34);
    AiTakePhoto->label_size05x = lv_label_create(AiTakePhoto->btn_size05x);
    lv_label_set_text(AiTakePhoto->label_size05x, "0.5x");
    lv_label_set_long_mode(AiTakePhoto->label_size05x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AiTakePhoto->label_size05x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AiTakePhoto->btn_size05x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AiTakePhoto->label_size05x, LV_PCT(100));

    // Write style for btn_size05x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AiTakePhoto->btn_size05x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->btn_size05x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AiTakePhoto->btn_size05x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AiTakePhoto->btn_size05x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->btn_size05x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->btn_size05x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AiTakePhoto->btn_size05x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AiTakePhoto->btn_size05x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AiTakePhoto->btn_size05x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AiTakePhoto->btn_size05x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size1x
    AiTakePhoto->btn_size1x = lv_button_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->btn_size1x, 260, 436);
    lv_obj_set_size(AiTakePhoto->btn_size1x, 44, 34);
    AiTakePhoto->label_size1x = lv_label_create(AiTakePhoto->btn_size1x);
    lv_label_set_text(AiTakePhoto->label_size1x, "1x");
    lv_label_set_long_mode(AiTakePhoto->label_size1x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AiTakePhoto->label_size1x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AiTakePhoto->btn_size1x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AiTakePhoto->label_size1x, LV_PCT(100));

    // Write style for btn_size1x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AiTakePhoto->btn_size1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->btn_size1x, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AiTakePhoto->btn_size1x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AiTakePhoto->btn_size1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->btn_size1x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->btn_size1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AiTakePhoto->btn_size1x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AiTakePhoto->btn_size1x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AiTakePhoto->btn_size1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AiTakePhoto->btn_size1x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size2x
    AiTakePhoto->btn_size2x = lv_button_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->btn_size2x, 330, 436);
    lv_obj_set_size(AiTakePhoto->btn_size2x, 44, 34);
    AiTakePhoto->label_size2x = lv_label_create(AiTakePhoto->btn_size2x);
    lv_label_set_text(AiTakePhoto->label_size2x, "2x");
    lv_label_set_long_mode(AiTakePhoto->label_size2x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AiTakePhoto->label_size2x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AiTakePhoto->btn_size2x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AiTakePhoto->label_size2x, LV_PCT(100));

    // Write style for btn_size2x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AiTakePhoto->btn_size2x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AiTakePhoto->btn_size2x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AiTakePhoto->btn_size2x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->btn_size2x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->btn_size2x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AiTakePhoto->btn_size2x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AiTakePhoto->btn_size2x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AiTakePhoto->btn_size2x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AiTakePhoto->btn_size2x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size3x
    AiTakePhoto->btn_size3x = lv_button_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->btn_size3x, 400, 436);
    lv_obj_set_size(AiTakePhoto->btn_size3x, 44, 34);
    AiTakePhoto->label_size3x = lv_label_create(AiTakePhoto->btn_size3x);
    lv_label_set_text(AiTakePhoto->label_size3x, "3x");
    lv_label_set_long_mode(AiTakePhoto->label_size3x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AiTakePhoto->label_size3x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AiTakePhoto->btn_size3x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AiTakePhoto->label_size3x, LV_PCT(100));

    // Write style for btn_size3x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AiTakePhoto->btn_size3x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AiTakePhoto->btn_size3x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AiTakePhoto->btn_size3x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->btn_size3x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->btn_size3x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AiTakePhoto->btn_size3x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AiTakePhoto->btn_size3x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AiTakePhoto->btn_size3x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AiTakePhoto->btn_size3x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont
    AiTakePhoto->top_cont = lv_obj_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->top_cont, 0, 0);
    lv_obj_set_size(AiTakePhoto->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AiTakePhoto->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AiTakePhoto->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AiTakePhoto->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    AiTakePhoto->btn_back = lv_button_create(AiTakePhoto->scr);
    lv_obj_set_pos(AiTakePhoto->btn_back, 0, 4);
    lv_obj_set_size(AiTakePhoto->btn_back, 60, 50);
    AiTakePhoto->label_back = lv_label_create(AiTakePhoto->btn_back);
    lv_label_set_text(AiTakePhoto->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AiTakePhoto->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AiTakePhoto->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AiTakePhoto->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AiTakePhoto->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AiTakePhoto->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AiTakePhoto->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AiTakePhoto->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AiTakePhoto->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AiTakePhoto->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AiTakePhoto->btn_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AiTakePhoto->btn_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AiTakePhoto->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AiTakePhoto->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of screen_AITakePhoto.
    //  设置按钮按压状态样式
    lv_obj_add_state(AiTakePhoto->btn_size1x, LV_STATE_PRESSED);

    // Write codes screen_AITakePhoto_btn_simulate_photo
    lv_obj_t *btn_simulate_photo = lv_button_create(AiTakePhoto->scr);
    btn_simulate_photo = btn_simulate_photo;
    lv_obj_set_pos(btn_simulate_photo, 580, 300);
    lv_obj_set_size(btn_simulate_photo, 60, 50);
    lv_obj_t *btn_simulate_photo_label = lv_label_create(btn_simulate_photo);
    lv_label_set_text(btn_simulate_photo_label, "AI Photo");
    lv_label_set_long_mode(btn_simulate_photo_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(btn_simulate_photo_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_simulate_photo, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(btn_simulate_photo_label, LV_PCT(100));

    // Write style for screen_AITakePhoto_btn_simulate_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(btn_simulate_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_simulate_photo, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_simulate_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_simulate_photo, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_simulate_photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_simulate_photo, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(btn_simulate_photo, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_simulate_photo, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_simulate_photo, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_simulate_photo, screen_AITakePhoto_btn_simulate_photo_event_handler, LV_EVENT_CLICKED, ui);

    // 初始化音效播放
    UI_VOICEPLAY_Init();

    /* 设置当前页面和按键处理回调 */
    set_current_page_handler(aitakephoto_key_handler);

    // Update current screen layout.
    lv_obj_update_layout(AiTakePhoto->scr);

    // Init events for screen.
    events_init_screen_AITakePhoto(ui);
}

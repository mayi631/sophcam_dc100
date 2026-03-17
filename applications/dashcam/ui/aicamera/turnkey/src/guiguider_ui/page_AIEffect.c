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
#include "ui_common.h"

#include "custom.h"
#include "config.h"
// 添加系统消息相关头文件
#include "sysutils_eventhub.h"
#include "mode.h"
// 添加音效播放相关头文件
#include "voiceplay.h"
// 添加硬件按键相关头文件
#include <linux/input.h>
#include "indev.h"

static void screen_AIEffect_btn_size3x_event_handler(lv_event_t *e)
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
            lv_obj_clear_state(g_ui.page_aieffect.btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size05x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size1x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size2x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        }
        default: break;
    }
}

static void screen_AIEffect_btn_size2x_event_handler(lv_event_t *e)
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
            lv_obj_clear_state(g_ui.page_aieffect.btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size05x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size1x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size3x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        }
        default: break;
    }
}

static void screen_AIEffect_btn_size1x_event_handler(lv_event_t *e)
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
            lv_obj_clear_state(g_ui.page_aieffect.btn_size05x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size05x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size2x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size3x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        }
        default: break;
    }
}

static void screen_AIEffect_btn_size05x_event_handler(lv_event_t *e)
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
            lv_obj_clear_state(g_ui.page_aieffect.btn_size1x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size1x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size2x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size2x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_clear_state(g_ui.page_aieffect.btn_size3x, LV_STATE_PRESSED);
            lv_obj_set_style_bg_color(g_ui.page_aieffect.btn_size3x, lv_color_hex(0xCCCCCC),
                                      LV_PART_MAIN | LV_STATE_DEFAULT);
            break;
        }
        default: break;
    }
}

static void screen_AIEffect_btn_back_event_handler(lv_event_t *e)
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

            // 异步处理音频清理
            pthread_t cleanup_thread;
            pthread_create(&cleanup_thread, NULL, UI_VOICEPLAY_DeInit, NULL);
            pthread_detach(cleanup_thread); // 分离线程，自动清理
            // 设置当前页面按键处理回调为NULL
            set_current_page_handler(NULL);

            ui_load_scr_animation(&g_ui, &g_ui.page_home1.scr, g_ui.screenHome1_del, &g_ui.screen_AIEffect_del,
                                  setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screen_AIEffect_btn_takephoto_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 触发拍照事件
            MESSAGE_S stEvent;
            stEvent.topic = EVENT_MODEMNG_START_PIV;
            MODEMNG_SendMessage(&stEvent);
            ui_common_wait_piv_end();

            // 异步处理音频清理
            pthread_t cleanup_thread;
            pthread_create(&cleanup_thread, NULL, UI_VOICEPLAY_DeInit_Delay, NULL);
            pthread_detach(cleanup_thread); // 分离线程，自动清理

            // 设置当前页面按键处理回调为NULL
            set_current_page_handler(NULL);

            // 等待拍照完成后跳转到AI特效页面
            // 这里可以添加延时或回调机制
            ui_load_scr_animation(&g_ui, &g_ui.page_aieffectpic.scr, g_ui.screen_AIEffectPic_del,
                                  &g_ui.screen_AIEffect_del, setup_scr_screen_AIEffectPic, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                  false, true);
            break;
        }
        default: break;
    }
}

// 硬件按键读取回调函数
static void aieffect_key_handler(int key_code, int key_value)
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
            MESSAGE_S stEvent;
            stEvent.topic = EVENT_MODEMNG_START_PIV;
            MODEMNG_SendMessage(&stEvent);
            ui_common_wait_piv_end();
            // 设置当前页面按键处理回调为NULL
            set_current_page_handler(NULL);
            // 异步处理音频清理
            pthread_t cleanup_thread;
            pthread_create(&cleanup_thread, NULL, UI_VOICEPLAY_DeInit_Delay, NULL);
            pthread_detach(cleanup_thread); // 分离线程，自动清理

            ui_load_scr_animation(&g_ui, &g_ui.page_aieffectpic.scr, g_ui.screen_AIEffectPic_del,
                                    &g_ui.screen_AIEffect_del, setup_scr_screen_AIEffectPic, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                    false, true);
            return;
        }
    }
}

void events_init_screen_AIEffect(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_aieffect.btn_size3x, screen_AIEffect_btn_size3x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aieffect.btn_size2x, screen_AIEffect_btn_size2x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aieffect.btn_size1x, screen_AIEffect_btn_size1x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aieffect.btn_size05x, screen_AIEffect_btn_size05x_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aieffect.btn_back, screen_AIEffect_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aieffect.btn_takephoto, screen_AIEffect_btn_takephoto_event_handler, LV_EVENT_CLICKED,
                        ui);
}

void setup_scr_screen_AIEffect(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIEffect...\n");

    AIEffect_t *AIEffect = &ui->page_aieffect;
    AIEffect->del        = true;

    // 创建主页面1 容器
    if(AIEffect->scr != NULL) {
        if(lv_obj_is_valid(AIEffect->scr)) {
            MLOG_DBG("page_AIEffect->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIEffect->scr);
        } else {
            MLOG_DBG("page_AIEffect->scr 已被自动销毁，仅重置指针\n");
        }
        AIEffect->scr = NULL;
    }

    // Write codes screen_AIEffect
    AIEffect->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIEffect->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIEffect->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_AIEffect, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(AIEffect->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont
    AIEffect->bottom_cont = lv_obj_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->bottom_cont, 0, 420);
    lv_obj_set_size(AIEffect->bottom_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIEffect->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(AIEffect->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AIEffect->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(AIEffect->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIEffect->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size3x
    AIEffect->btn_size3x = lv_button_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->btn_size3x, 400, 436);
    lv_obj_set_size(AIEffect->btn_size3x, 44, 34);
    AIEffect->label_size3x = lv_label_create(AIEffect->btn_size3x);
    lv_label_set_text(AIEffect->label_size3x, "3x");
    lv_label_set_long_mode(AIEffect->label_size3x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffect->label_size3x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffect->btn_size3x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffect->label_size3x, LV_PCT(100));

    // Write style for btn_size3x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->btn_size3x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->btn_size3x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->btn_size3x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffect->btn_size3x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->btn_size3x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->btn_size3x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffect->btn_size3x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffect->btn_size3x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffect->btn_size3x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffect->btn_size3x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size2x
    AIEffect->btn_size2x = lv_button_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->btn_size2x, 330, 436);
    lv_obj_set_size(AIEffect->btn_size2x, 44, 34);
    AIEffect->label_size2x = lv_label_create(AIEffect->btn_size2x);
    lv_label_set_text(AIEffect->label_size2x, "2x");
    lv_label_set_long_mode(AIEffect->label_size2x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffect->label_size2x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffect->btn_size2x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffect->label_size2x, LV_PCT(100));

    // Write style for btn_size2x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->btn_size2x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->btn_size2x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->btn_size2x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffect->btn_size2x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->btn_size2x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->btn_size2x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffect->btn_size2x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffect->btn_size2x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffect->btn_size2x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffect->btn_size2x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size1x
    AIEffect->btn_size1x = lv_button_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->btn_size1x, 260, 436);
    lv_obj_set_size(AIEffect->btn_size1x, 44, 34);
    AIEffect->label_size1x = lv_label_create(AIEffect->btn_size1x);
    lv_label_set_text(AIEffect->label_size1x, "1x");
    lv_label_set_long_mode(AIEffect->label_size1x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffect->label_size1x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffect->btn_size1x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffect->label_size1x, LV_PCT(100));

    // Write style for btn_size1x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->btn_size1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->btn_size1x, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->btn_size1x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffect->btn_size1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->btn_size1x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->btn_size1x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffect->btn_size1x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffect->btn_size1x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffect->btn_size1x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffect->btn_size1x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_size05x
    AIEffect->btn_size05x = lv_button_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->btn_size05x, 190, 436);
    lv_obj_set_size(AIEffect->btn_size05x, 44, 34);
    AIEffect->label_size05x = lv_label_create(AIEffect->btn_size05x);
    lv_label_set_text(AIEffect->label_size05x, "0.5x");
    lv_label_set_long_mode(AIEffect->label_size05x, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffect->label_size05x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffect->btn_size05x, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffect->label_size05x, LV_PCT(100));

    // Write style for btn_size05x, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->btn_size05x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->btn_size05x, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->btn_size05x, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffect->btn_size05x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->btn_size05x, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->btn_size05x, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffect->btn_size05x, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffect->btn_size05x, &lv_font_montserratMedium_13, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffect->btn_size05x, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffect->btn_size05x, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont
    AIEffect->top_cont = lv_obj_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->top_cont, 0, 0);
    lv_obj_set_size(AIEffect->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIEffect->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIEffect->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    AIEffect->btn_back = lv_button_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->btn_back, 0, 4);
    lv_obj_set_size(AIEffect->btn_back, 60, 50);
    AIEffect->label_back = lv_label_create(AIEffect->btn_back);
    lv_label_set_text(AIEffect->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIEffect->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffect->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffect->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffect->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffect->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffect->btn_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffect->btn_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffect->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffect->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_AIEffect_btn_takephoto
    AIEffect->btn_takephoto = lv_button_create(AIEffect->scr);
    lv_obj_set_pos(AIEffect->btn_takephoto, 521, 215);
    lv_obj_set_size(AIEffect->btn_takephoto, 100, 50);
    AIEffect->label_takephoto = lv_label_create(AIEffect->btn_takephoto);
    lv_label_set_text(AIEffect->label_takephoto, "模拟拍照");
    lv_label_set_long_mode(AIEffect->label_takephoto, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffect->label_takephoto, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffect->btn_takephoto, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffect->label_takephoto, LV_PCT(100));

    // Write style for screen_AIEffect_btn_takephoto, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffect->btn_takephoto, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffect->btn_takephoto, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffect->btn_takephoto, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffect->btn_takephoto, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffect->btn_takephoto, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffect->btn_takephoto, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffect->btn_takephoto, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffect->btn_takephoto, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffect->btn_takephoto, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffect->btn_takephoto, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of screen_AIEffect.
    //  设置按钮按压状态样式
    lv_obj_add_state(AIEffect->btn_size1x, LV_STATE_PRESSED);

    // 初始化音效播放
    UI_VOICEPLAY_Init();

    /* 设置当前页面和按键处理回调 */
    set_current_page_handler(aieffect_key_handler);

    // Update current screen layout.
    lv_obj_update_layout(AIEffect->scr);

    // Init events for screen.
    events_init_screen_AIEffect(ui);
}

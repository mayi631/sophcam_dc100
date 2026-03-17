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
#include <stdlib.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"
#include "config.h"
#include "ui_common.h"

static void screenHome1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_GESTURE: {
            // 获取手势方向，需要 TP 驱动支持
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
            switch(dir) {
                case LV_DIR_LEFT: {
                    lv_indev_wait_release(lv_indev_active());
                    ui_load_scr_animation(&g_ui, &g_ui.page_home2.scr, g_ui.screenHome2_del, &g_ui.screenHome1_del,
                                          setup_scr_screenHome2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 200, 0, false, true);
                    break;
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

static void screenHome1_btn_ai_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aitakephoto.scr, g_ui.screen_AITakePhoto_del, &g_ui.screenHome1_del,
                                  setup_scr_screen_AITakePhoto, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            MESSAGE_S Msg = {0};
            Msg.topic     = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1      = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);
            break;
        }
        default: break;
    }
}

static void screenHome1_btn_photo_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephoto.scr, g_ui.screen_TakePhoto_del, &g_ui.screenHome1_del,
                                  setup_scr_screen_TakePhoto, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            MESSAGE_S Msg = {0};
            Msg.topic     = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1      = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);
            break;
        }
        default: break;
    }
}

static void screenHome1_btn_ai_dialog_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aidialog.scr, g_ui.screen_AIDialog_del, &g_ui.screenHome1_del,
                                  setup_scr_screen_AIDialog, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screenHome1_btn_ai_effect_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aieffect.scr, g_ui.screen_AIEffect_del, &g_ui.screenHome1_del,
                                  setup_scr_screen_AIEffect, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            MESSAGE_S Msg = {0};
            Msg.topic     = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1      = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);
            break;
        }
        default: break;
    }
}

static void events_init_screenHome1(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_home1.scr, screenHome1_event_handler, LV_EVENT_GESTURE, ui);
    lv_obj_add_event_cb(ui->page_home1.imgbtn_ai_photo, screenHome1_btn_ai_photo_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_home1.imgbtn_photo, screenHome1_btn_photo_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_home1.imgbtn_ai_dialog, screenHome1_btn_ai_dialog_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_home1.imgbtn_ai_effect, screenHome1_btn_ai_effect_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_home1(lv_ui_t *ui)
{
    MLOG_DBG("loading page_home1...\n");

    PageHome1_t *home1 = &ui->page_home1;
    home1->del         = true;

    // 创建主页面1 容器
    if(home1->scr != NULL) {
        if(lv_obj_is_valid(home1->scr)) {
            MLOG_DBG("page_home1->scr 仍然有效，删除旧对象\n");
            lv_obj_del(home1->scr);
        } else {
            MLOG_DBG("page_home1->scr 已被自动销毁，仅重置指针\n");
        }
        home1->scr = NULL;
    }
    home1->scr = lv_obj_create(NULL);
    lv_obj_set_size(home1->scr, H_RES, V_RES);

    MLOG_DBG("home1->scr lv_obj_get_scrollbar_mode: %d\n", lv_obj_get_scrollbar_mode(home1->scr));
    lv_obj_set_scrollbar_mode(home1->scr, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条，默认 LV_SCROLLBAR_MODE_AUTO
    lv_obj_set_style_bg_opa(home1->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明度 0~255，255 完全不透明
    lv_obj_set_style_bg_color(home1->scr, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 背景颜色
    lv_obj_set_style_bg_grad_dir(home1->scr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);    // 无渐变色

    // 主页面上增加按钮

    // 1. 增加AI拍照/拍万物
    home1->imgbtn_ai_photo = lv_imagebutton_create(home1->scr);
    lv_obj_set_pos(home1->imgbtn_ai_photo, 113, 79);
    lv_obj_set_size(home1->imgbtn_ai_photo, 167, 161);
    lv_imagebutton_set_src(home1->imgbtn_ai_photo, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_AIPhoto_2_RGB565A8_167x161,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_photo, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_AIPhoto_2_RGB565A8_167x161,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_photo, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_AIPhoto_2_RGB565A8_167x161, NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_photo, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_AIPhoto_2_RGB565A8_167x161, NULL);
    lv_obj_set_style_pad_all(home1->imgbtn_ai_photo, 0, LV_STATE_DEFAULT);

    // Write style for screenHome1_btn_imgbtn_ai_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home1->imgbtn_ai_photo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home1->imgbtn_ai_photo, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home1->imgbtn_ai_photo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 2. 增加拍照按钮（传统拍照）
    home1->imgbtn_photo = lv_imagebutton_create(home1->scr);
    lv_obj_set_pos(home1->imgbtn_photo, 360, 96);
    lv_obj_set_size(home1->imgbtn_photo, 141, 133);
    lv_imagebutton_set_src(home1->imgbtn_photo, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_TakePhoto_2_RGB565A8_141x133,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_photo, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_TakePhoto_2_RGB565A8_141x133,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_photo, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_TakePhoto_2_RGB565A8_141x133, NULL);
    lv_imagebutton_set_src(home1->imgbtn_photo, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_TakePhoto_2_RGB565A8_141x133, NULL);
    lv_obj_set_style_pad_all(home1->imgbtn_photo, 0, LV_STATE_DEFAULT);

    // Write style for screenHome1_btn_photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home1->imgbtn_photo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home1->imgbtn_photo, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home1->imgbtn_photo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 3. 增加AI对话
    home1->imgbtn_ai_dialog = lv_imagebutton_create(home1->scr);
    lv_obj_set_pos(home1->imgbtn_ai_dialog, 113, 269);
    lv_obj_set_size(home1->imgbtn_ai_dialog, 158, 149);
    lv_imagebutton_set_src(home1->imgbtn_ai_dialog, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_AItalk_2_RGB565A8_158x149,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_dialog, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_AItalk_2_RGB565A8_158x149,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_dialog, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_AItalk_2_RGB565A8_158x149, NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_dialog, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_AItalk_2_RGB565A8_158x149, NULL);
    lv_obj_set_style_pad_all(home1->imgbtn_ai_dialog, 0, LV_STATE_DEFAULT);

    // Write style for screenHome1_btn_ai_dialog, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home1->scr_btn_ai_dialog, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home1->scr_btn_ai_dialog, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home1->scr_btn_ai_dialog, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 4. 增加AI特效
    home1->imgbtn_ai_effect = lv_imagebutton_create(home1->scr);
    lv_obj_set_pos(home1->imgbtn_ai_effect, 350, 240);
    lv_obj_set_size(home1->imgbtn_ai_effect, 168, 167);
    lv_imagebutton_set_src(home1->imgbtn_ai_effect, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_AIeffect_2_RGB565A8_168x167,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_effect, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_AIeffect_2_RGB565A8_168x167,
                           NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_effect, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_AIeffect_2_RGB565A8_168x167, NULL);
    lv_imagebutton_set_src(home1->imgbtn_ai_effect, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_AIeffect_2_RGB565A8_168x167, NULL);
    lv_obj_set_style_pad_all(home1->imgbtn_ai_effect, 0, LV_STATE_DEFAULT);

    // Write style for screenHome1_btn_ai_effect, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home1->scr_btn_ai_effect, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home1->scr_btn_ai_effect, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home1->scr_btn_ai_effect, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建页面指示器容器
    home1->dots_cont = lv_obj_create(home1->scr);
    lv_obj_set_size(home1->dots_cont, 40, 20);
    lv_obj_set_pos(home1->dots_cont, 300, 450);
    lv_obj_set_style_bg_opa(home1->dots_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(home1->dots_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(home1->dots_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建第一个圆点（当前页面）
    home1->dot_home1 = lv_obj_create(home1->dots_cont);
    lv_obj_set_size(home1->dot_home1, 12, 12);
    lv_obj_set_pos(home1->dot_home1, 2, 4);
    lv_obj_set_style_radius(home1->dot_home1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(home1->dot_home1, lv_color_hex(0xFFD400), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(home1->dot_home1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建第二个圆点
    home1->dot_home2 = lv_obj_create(home1->dots_cont);
    lv_obj_set_size(home1->dot_home2, 12, 12);
    lv_obj_set_pos(home1->dot_home2, 22, 4);
    lv_obj_set_style_radius(home1->dot_home2, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(home1->dot_home2, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(home1->dot_home2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout. 不需要，LVGL 会自动更新。
    // lv_obj_update_layout(home1->scr);

    // Init events for screen.
    events_init_screenHome1(ui);
}

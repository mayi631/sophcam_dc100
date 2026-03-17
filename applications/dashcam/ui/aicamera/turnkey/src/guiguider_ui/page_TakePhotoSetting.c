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
#include "config.h"
#include "custom.h"
// extern char g_button_labelRes[32];
// extern char g_button_labelWhi[32];
// extern char g_button_labelPho[32];
// extern char g_button_labelExp[32];
// extern char g_button_labelPicMode[32];
// extern char g_button_labelSel[32];
// extern char g_button_labelSho[32];
// extern char g_button_labelPicQual[32];
// extern char g_button_labelSen[32];
// extern char g_button_labelAnti[32];
// extern char g_button_labelAuto[32];
// extern char g_button_labelFace[32];
// extern char g_button_labelSmile[32];
// extern char g_button_labelBeau1[32];
// extern char g_button_labelBeau2[32];

char g_button_labelRes[32]     = "8M";
char g_button_labelWhi[32]     = "自动";
char g_button_labelPho[32]     = "普通";
char g_button_labelExp[32]     = "0";
char g_button_labelPicMode[32] = "自动";
char g_button_labelSel[32]     = "定时2s";
char g_button_labelSho[32]     = "单张";
char g_button_labelPicQual[32] = "普通";
char g_button_labelSen[32]     = "自动";
char g_button_labelAnti[32]    = "开启";
char g_button_labelAuto[32]    = "触控对焦";
char g_button_labelFace[32]    = "开启";
char g_button_labelSmile[32]   = "开启";
char g_button_labelBeau1[32]   = "";
char g_button_labelBeau2[32]   = "美白";

static void screen_TakePhotoSetting_btn_3_event_handler(lv_event_t *e)
{
    MLOG_DBG("screen_TakePhotoSetting_btn_3_event_handler \n");
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingResolution, g_ui.screen_SettingResolution_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingResolution,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_4_event_handler(lv_event_t *e)
{
    MLOG_DBG("screen_TakePhotoSetting_btn_4_event_handler \n");
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingWhiteBalance, g_ui.screen_SettingWhiteBalance_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingWhiteBalance,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_5_event_handler(lv_event_t *e)
{
    MLOG_DBG("screen_TakePhotoSetting_btn_5_event_handler \n");
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingPhotoEffect, g_ui.screen_SettingPhotoEffect_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingPhotoEffect,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_6_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingExposure, g_ui.screen_SettingExposure_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingExposure,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_7_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingPictureMode, g_ui.screen_SettingPictureMode_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingPictureMode,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_8_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingSelfieTime, g_ui.screen_SettingSelfieTime_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingSelfieTime,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_9_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingShootingMode, g_ui.screen_SettingShootingMode_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingShootingMode,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_10_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingPictureQuality, g_ui.screen_SettingPictureQuality_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingPictureQuality,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_11_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingSensitivity, g_ui.screen_SettingSensitivity_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingSensitivity,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_12_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingAntiShake, g_ui.screen_SettingAntiShake_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingAntiShake,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_13_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingAutofocus, g_ui.screen_SettingAutofocus_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingAutofocus,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_14_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingFaceDectection, g_ui.screen_SettingFaceDectection_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingFaceDectection,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_15_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingSmileDectection, g_ui.screen_SettingSmileDectection_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingSmileDectection,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_16_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_SettingBeauty, g_ui.screen_SettingBeauty_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_SettingBeauty,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_TakePhotoSetting_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephoto.scr, g_ui.screen_TakePhoto_del,
                                  &g_ui.screen_TakePhotoSetting_del, setup_scr_screen_TakePhoto, LV_SCR_LOAD_ANIM_NONE,
                                  0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_TakePhotoSetting(lv_ui_t *ui)
{
    // 获取设置选项容器
    lv_obj_t *settings_cont = lv_obj_get_child(ui->page_takephotosetting.scr, 1); // 第二个子对象是设置选项容器

    // 为每个设置按钮添加事件处理
    for(int i = 0; i < 14; i++) {
        lv_obj_t *btn = lv_obj_get_child(settings_cont, i);
        switch(i) {
            case 0: // 分辨率
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_3_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 1: // 白平衡
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_4_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 2: // 摄影效果
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_5_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 3: // 曝光设置
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_6_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 4: // 画面模式
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_7_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 5: // 自拍时间
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_8_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 6: // 拍摄模式
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_9_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 7: // 画质
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_10_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 8: // 感光度
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_11_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 9: // 防抖
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_12_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 10: // 自动对焦功能
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_13_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 11: // 人脸侦测功能
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_14_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 12: // 笑脸侦测功能
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_15_event_handler, LV_EVENT_CLICKED, ui);
                break;
            case 13: // 美颜功能
                lv_obj_add_event_cb(btn, screen_TakePhotoSetting_btn_16_event_handler, LV_EVENT_CLICKED, ui);
                break;
        }
    }

    // 为返回按钮添加事件处理
    lv_obj_add_event_cb(ui->page_takephotosetting.btn_back, screen_TakePhotoSetting_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_TakePhotoSetting(lv_ui_t *ui)
{

    MLOG_DBG("loading page_takephotosetting...\n");

    TakePhotoSetting_t *TakePhotoSetting = &ui->page_takephotosetting;
    TakePhotoSetting->del                = true;

    // 创建主页面1 容器
    if(TakePhotoSetting->scr != NULL) {
        if(lv_obj_is_valid(TakePhotoSetting->scr)) {
            MLOG_DBG("page_TakePhotoSetting->scr 仍然有效，删除旧对象\n");
            lv_obj_del(TakePhotoSetting->scr);
        } else {
            MLOG_DBG("page_TakePhotoSetting->scr 已被自动销毁，仅重置指针\n");
        }
        TakePhotoSetting->scr = NULL;
    }

    // Write codes scr
    TakePhotoSetting->scr = lv_obj_create(NULL);
    lv_obj_set_size(TakePhotoSetting->scr, 640, 480);
    lv_obj_set_scrollbar_mode(TakePhotoSetting->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(TakePhotoSetting->scr, 0, 0);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhotoSetting->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(TakePhotoSetting->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes cont_top (顶部栏)
    TakePhotoSetting->cont_top = lv_obj_create(TakePhotoSetting->scr);
    lv_obj_set_pos(TakePhotoSetting->cont_top, 0, 0);
    lv_obj_set_size(TakePhotoSetting->cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(TakePhotoSetting->cont_top, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_TakePhoto_cont_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(TakePhotoSetting->cont_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhotoSetting->cont_top, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(TakePhotoSetting->cont_top, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhotoSetting->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    TakePhotoSetting->btn_back = lv_button_create(TakePhotoSetting->cont_top);
    lv_obj_set_pos(TakePhotoSetting->btn_back, 4, 4);
    lv_obj_set_size(TakePhotoSetting->btn_back, 60, 52);
    TakePhotoSetting->label_back = lv_label_create(TakePhotoSetting->btn_back);
    lv_label_set_text(TakePhotoSetting->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(TakePhotoSetting->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(TakePhotoSetting->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(TakePhotoSetting->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(TakePhotoSetting->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(TakePhotoSetting->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(TakePhotoSetting->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhotoSetting->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhotoSetting->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(TakePhotoSetting->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(TakePhotoSetting->btn_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(TakePhotoSetting->btn_back, &lv_font_montserratMedium_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(TakePhotoSetting->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(TakePhotoSetting->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建设置选项容器 - 使用flex布局
    lv_obj_t *settings_cont = lv_obj_create(TakePhotoSetting->scr);
    lv_obj_set_size(settings_cont, 600, 320);
    lv_obj_align(settings_cont, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_opa(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_border_width(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(settings_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(settings_cont, 10, 0);

    // 创建所有设置按钮
    const char *btn_labels[] = {"分辨率",       "白平衡",       "摄影效果",     "曝光设置", "画面模式",
                                "自拍时间",     "拍摄模式",     "画质",         "感光度",   "防抖",
                                "自动对焦功能", "人脸侦测功能", "笑脸侦测功能", "美颜功能"};

    for(int i = 0; i < 14; i++) {
        lv_obj_t *btn = lv_button_create(settings_cont);
        if(!btn) continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, 40);
        lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
        lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(btn, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *label = lv_label_create(btn);
        if(!label) continue; // 如果标签创建失败则跳过

        lv_label_set_text(label, btn_labels[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_set_style_text_font(label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 创建右侧值标签
        lv_obj_t *value_label = lv_label_create(btn);
        lv_obj_align(value_label, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(value_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 设置对应的g_button_label值
        switch(i) {
            case 0: // 分辨率
                lv_label_set_text(value_label, g_button_labelRes);
                break;
            case 1: // 白平衡
                lv_label_set_text(value_label, g_button_labelWhi);
                break;
            case 2: // 摄影效果
                lv_label_set_text(value_label, g_button_labelPho);
                break;
            case 3: // 曝光设置
                lv_label_set_text(value_label, g_button_labelExp);
                break;
            case 4: // 画面模式
                lv_label_set_text(value_label, g_button_labelPicMode);
                break;
            case 5: // 自拍时间
                lv_label_set_text(value_label, g_button_labelSel);
                break;
            case 6: // 拍摄模式
                lv_label_set_text(value_label, g_button_labelSho);
                break;
            case 7: // 画质
                lv_label_set_text(value_label, g_button_labelPicQual);
                break;
            case 8: // 感光度
                lv_label_set_text(value_label, g_button_labelSen);
                break;
            case 9: // 防抖
                lv_label_set_text(value_label, g_button_labelAnti);
                break;
            case 10: // 自动对焦功能
                lv_label_set_text(value_label, g_button_labelAuto);
                break;
            case 11: // 人脸侦测功能
                lv_label_set_text(value_label, g_button_labelFace);
                break;
            case 12: // 笑脸侦测功能
                lv_label_set_text(value_label, g_button_labelSmile);
                break;
            case 13: // 美颜功能
                if(strlen(g_button_labelBeau1) > 0) {
                    char combined[64];
                    snprintf(combined, sizeof(combined), "%s %s", g_button_labelBeau1, g_button_labelBeau2);
                    lv_label_set_text(value_label, combined);
                } else {
                    lv_label_set_text(value_label, g_button_labelBeau2);
                }
                break;
        }
    }

    // Write codes cont_bottom (底部栏)
    TakePhotoSetting->cont_bottom = lv_obj_create(TakePhotoSetting->scr);
    lv_obj_set_pos(TakePhotoSetting->cont_bottom, 0, 420);   // 调整位置避免重叠
    lv_obj_set_size(TakePhotoSetting->cont_bottom, 640, 60); // 调整高度
    lv_obj_set_scrollbar_mode(TakePhotoSetting->cont_bottom, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(TakePhotoSetting->cont_bottom, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(TakePhotoSetting->cont_bottom, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(TakePhotoSetting->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(TakePhotoSetting->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(TakePhotoSetting->scr);

    // Init events for screen.
    events_init_screen_TakePhotoSetting(ui);
}

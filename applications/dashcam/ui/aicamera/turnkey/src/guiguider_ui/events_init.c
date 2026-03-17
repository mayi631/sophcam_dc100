/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
#define DEBUG
#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"
#include "config.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

// static bool is_playing = true;
#include <string.h>
// char g_button_labelRes[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelRes[32] = "96M";
extern char g_button_labelRes[32];
// char g_button_labelWhi[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelWhi[32] = "自动";
extern char g_button_labelWhi[32];
// char g_button_labelPho[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelPho[32] = "普通";
extern char g_button_labelPho[32];
// #include <string.h>
// char g_button_labelExp[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelExp[32] = "0";
extern char g_button_labelExp[32];
// char g_button_labelPicMode[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelPicMode[32] = "自动";
extern char g_button_labelPicMode[32];
// char g_button_labelSel[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelSel[32] = "定时2s";
extern char g_button_labelSel[32];
// char g_button_labelSho[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelSho[32] = "单张";
extern char g_button_labelSho[32];
// char g_button_labelPicQual[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelPicQual[32] = "普通";
extern char g_button_labelPicQual[32];
// char g_button_labelSen[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelSen[32] = "自动";
extern char g_button_labelSen[32];
// char g_button_labelAnti[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelAnti[32] = "开启";
extern char g_button_labelAnti[32];
// char g_button_labelAuto[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelAuto[32] = "触控对焦";
extern char g_button_labelAuto[32];
// char g_button_labelFace[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelFace[32] = "关闭";
extern char g_button_labelFace[32];
// char g_button_labelSmile[32] = {0}; // 存储按钮标签的全局变量
// char g_button_labelSmile[32] = "关闭";
extern char g_button_labelSmile[32];
// char g_button_labelBeau2[32] = {0}; // 存储按钮标签的全局变量
extern char g_button_labelBeau2[32];
// char g_button_labelBeau1[32] = {0}; // 存储按钮标签的全局变量
extern char g_button_labelBeau1[32];

static void screen_PhotoAlbumVid_btn_stop_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_flag(g_ui.screen_PhotoAlbumVid_btn_stop, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.screen_PhotoAlbumVid_btn_play, LV_OBJ_FLAG_HIDDEN);
            // is_playing = false;
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_play_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_flag(g_ui.screen_PhotoAlbumVid_btn_play, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.screen_PhotoAlbumVid_btn_stop, LV_OBJ_FLAG_HIDDEN);
            // is_playing = true;
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_slider_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
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
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
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
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_1x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
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
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_075x_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
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
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumVid_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_photoalbum.scr, g_ui.screen_PhotoAlbum_del,
                                  &g_ui.screen_PhotoAlbumVid_del, setup_scr_screen_PhotoAlbum, LV_SCR_LOAD_ANIM_NONE, 0,
                                  0, false, true);

            break;
        }
        default: break;
    }
}

void events_init_screen_PhotoAlbumVid(lv_ui_t *ui)
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

static void screen_Photo_btn_1_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephoto.scr, g_ui.screen_TakePhoto_del, &g_ui.screen_Photo_del,
                                  setup_scr_screen_TakePhoto, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_Photo(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_Photo_btn_1, screen_Photo_btn_1_event_handler, LV_EVENT_CLICKED, ui);
}

static void screen_SettingResolution_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingResolution_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingResolution(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingResolution_btn_back, screen_SettingResolution_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingWhiteBalance_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingWhiteBalance_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingWhiteBalance(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingWhiteBalance_btn_back, screen_SettingWhiteBalance_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingPhotoEffect_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingPhotoEffect_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingPhotoEffect(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingPhotoEffect_btn_back, screen_SettingPhotoEffect_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingExposure_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingExposure_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingExposure(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingExposure_btn_back, screen_SettingExposure_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingPictureMode_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingPictureMode_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingPictureMode(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingPictureMode_btn_back, screen_SettingPictureMode_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingSelfieTime_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingSelfieTime_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingSelfieTime(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingSelfieTime_btn_back, screen_SettingSelfieTime_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingShootingMode_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingShootingMode_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingShootingMode(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingShootingMode_btn_back, screen_SettingShootingMode_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingPictureQuality_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingPictureQuality_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingPictureQuality(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingPictureQuality_btn_back, screen_SettingPictureQuality_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingSensitivity_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingSensitivity_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingSensitivity(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingSensitivity_btn_back, screen_SettingSensitivity_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingAntiShake_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingAntiShake_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingAntiShake(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingAntiShake_btn_back, screen_SettingAntiShake_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingAutofocus_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingAutofocus_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingAutofocus(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingAutofocus_btn_back, screen_SettingAutofocus_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingFaceDectection_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingFaceDectection_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingFaceDectection(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingFaceDectection_btn_back, screen_SettingFaceDectection_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_SettingSmileDectection_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingSmileDectection_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingSmileDectection(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingSmileDectection_btn_back,
                        screen_SettingSmileDectection_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

static void screen_SettingBeauty_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_takephotosetting.scr, g_ui.screen_TakePhotoSetting_del,
                                  &g_ui.screen_SettingBeauty_del, setup_scr_screen_TakePhotoSetting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingBeauty(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_SettingBeauty_btn_back, screen_SettingBeauty_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void screen_TakeChinese_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_home2.scr, g_ui.screenHome2_del, &g_ui.screen_TakeChinese_del,
                                  setup_scr_screenHome2, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_TakeChinese(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_TakeChinese_btn_back, screen_TakeChinese_btn_back_event_handler, LV_EVENT_CLICKED,
                        ui);
}

static void screen_TakeEnglish_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_home2.scr, g_ui.screenHome2_del, &g_ui.screen_TakeEnglish_del,
                                  setup_scr_screenHome2, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_TakeEnglish(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->screen_TakeEnglish_btn_back, screen_TakeEnglish_btn_back_event_handler, LV_EVENT_CLICKED,
                        ui);
}

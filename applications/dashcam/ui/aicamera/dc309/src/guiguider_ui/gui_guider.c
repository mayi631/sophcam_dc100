/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
// #define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "config.h"
#include "mlog.h"
#include "page_all.h"
#include "custom.h"
#include "param.h"
UIScreenManager_t ui_Screen_Manager_s;

static uint8_t homeMode_Index = 0;

void homeMode_Set(uint8_t mode)
{
    homeMode_Index = mode;
}

uint8_t homeMode_Get(void)
{
    return homeMode_Index;
}

lv_obj_t *ui_Get_PreScreen(void)
{
    return ui_Screen_Manager_s.prev_screen;
}

void ui_manager_init()
{
    ui_Screen_Manager_s.current_screen = NULL;
    ui_Screen_Manager_s.prev_screen    = NULL;
    ui_Screen_Manager_s.nav_bar        = NULL;
}

void ui_init_style(lv_style_t *style)
{
    if(style->prop_cnt > 1)
        lv_style_reset(style);
    else
        lv_style_init(style);
}

void ui_load_win_animation(lv_obj_t *new_scr, ui_setup_scr_t new_win, lv_screen_load_anim_t anim_type, uint32_t time,
                           uint32_t delay, bool auto_del)
{
    new_win(NULL);
    lv_screen_load_anim(new_scr, anim_type, time, delay, auto_del);
}

void ui_load_scr_animation(lv_ui_t *ui, lv_obj_t **new_scr, bool new_scr_del, bool *old_scr_del,
                           ui_setup_scr_t setup_scr, lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay,
                           bool is_clean, bool auto_del)
{
    lv_obj_t *act_scr                  = lv_screen_active();
    ui_Screen_Manager_s.current_screen = lv_screen_active();
    MLOG_DBG("ui_load_scr_animation\n");
#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "gg_external_data.h"
    if(auto_del) {

        gg_edata_task_clear(act_scr);
    }
#endif
    if(auto_del && is_clean) {

        lv_obj_clean(act_scr);
    }
    
    setup_scr(ui);

    // 保存当前屏幕作为上一级界面
    if(ui_Screen_Manager_s.current_screen != ui_Screen_Manager_s.prev_screen) {
        ui_Screen_Manager_s.prev_screen    = ui_Screen_Manager_s.current_screen;
        ui_Screen_Manager_s.current_screen = *new_scr;
    }
    // 在加载新屏幕之前，根据目标屏幕决定是否显示状态栏
    if(setup_scr == setup_scr_home1) 
    {
        lv_obj_clear_flag(ui->status_bar, LV_OBJ_FLAG_HIDDEN);
        lv_timer_resume(timers.wifi_timer);
        lv_timer_resume(timers.power_timer);
        // lv_timer_resume(timers.date_timer);
        MLOG_DBG("status_bar: clear_flag_hidden\n");
        if(setup_scr == setup_scr_home1) 
        {
            MLOG_DBG("home1\n");
        } 
    } 
    else
    {
        lv_obj_add_flag(ui->status_bar, LV_OBJ_FLAG_HIDDEN);
        lv_timer_pause(timers.wifi_timer);
        lv_timer_pause(timers.power_timer);
        // lv_timer_pause(timers.date_timer);
        MLOG_DBG("status_bar: add_flag_hidden\n");
    }


    // 设置新屏幕
    lv_screen_load_anim(*new_scr, anim_type, time, delay, auto_del);
    if(old_scr_del != NULL) {
        *old_scr_del = auto_del;
    }
}

void ui_animation(void *var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value,
                  lv_anim_path_cb_t path_cb, uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time,
                  uint32_t playback_delay, lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb,
                  lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb)
{
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, var);
    lv_anim_set_exec_cb(&anim, exec_cb);
    lv_anim_set_values(&anim, start_value, end_value);
    lv_anim_set_time(&anim, duration);
    lv_anim_set_delay(&anim, delay);
    lv_anim_set_path_cb(&anim, path_cb);
    lv_anim_set_repeat_count(&anim, repeat_cnt);
    lv_anim_set_repeat_delay(&anim, repeat_delay);
    lv_anim_set_playback_time(&anim, playback_time);
    lv_anim_set_playback_delay(&anim, playback_delay);
    if(start_cb) {
        lv_anim_set_start_cb(&anim, start_cb);
    }
    if(ready_cb) {
        lv_anim_set_completed_cb(&anim, ready_cb);
    }
    if(deleted_cb) {
        lv_anim_set_deleted_cb(&anim, deleted_cb);
    }
    lv_anim_start(&anim);
}

void init_scr_del_flag(lv_ui_t *ui)
{
    MLOG_DBG("init_scr_del_flag\n");

    ui->screen_VolumeOverlay_del          = true;
    ui->screenHome1_del                   = true;
    ui->screenHomePhoto_del               = true;
    ui->screenPhotoMenuSetting_del        = true;
    ui->screen_SettingResolution_del      = true;
    ui->screen_SettingWhiteBalance_del    = true;
    ui->screen_SettingPhotoEffect_del     = true;
    ui->screen_SettingExposure_del        = true;
    ui->screen_SettingAutofocus_del       = true;
    ui->screen_SettingPictureMode_del     = true;
    ui->screen_SettingSelfieTime_del      = true;
    ui->screen_SettingShootingMode_del    = true;
    ui->screen_SettingPictureQuality_del  = true;
    ui->screen_SettingSensitivity_del     = true;
    ui->screen_SettingAntiShake_del       = true;
    ui->screen_SettingFaceDectection_del  = true;
    ui->screen_SettingSmileDectection_del = true;
    ui->screen_SettingBeauty_del          = true;

    homeMode_Set(PHOTO_MODE);
}

void setup_bottom_layer(void)
{
    lv_theme_apply(lv_layer_bottom());
}

void setup_ui(lv_ui_t *ui)
{
    memset(ui, 0, sizeof(lv_ui_t)); // 清空 ui 结构体，避免指针默认状态异常。
    setup_bottom_layer();
    init_scr_del_flag(ui);
    ui_manager_init();
    // 初始化图片显示文件系统驱动
    init_image_filesystem();
    led_off();// 关闭IR_CUT
    
    // 根据保存的状态灯参数初始化呼吸灯
    {
        PARAM_CONTEXT_S* pstParamCtx = PARAM_GetCtx();
        int32_t stlight_val = pstParamCtx->pstCfg->Menu.StatusLight.Current;
        stlight_init_by_param(stlight_val);
    }

    // 根据保存的亮度参数设置初始亮度
    {
        PARAM_CONTEXT_S* pstParamCtx = PARAM_GetCtx();
        int32_t brightness_val = pstParamCtx->pstCfg->Menu.Brightness.Current;
        brightness_set_level(brightness_val + 1); // param 0-6, level 1-7
    }
    setup_scr_home1(ui);
    lv_screen_load(obj_home_s);
}

void init_keyboard(lv_ui_t *ui)
{}

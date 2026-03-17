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
#include "config.h"

void ui_init_style(lv_style_t *style)
{
    if(style->prop_cnt > 1)
        lv_style_reset(style);
    else
        lv_style_init(style);
}

void ui_load_scr_animation(lv_ui_t *ui, lv_obj_t **new_scr, bool new_scr_del, bool *old_scr_del,
                           ui_setup_scr_t setup_scr, lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay,
                           bool is_clean, bool auto_del)
{
    lv_obj_t *act_scr = lv_screen_active();

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

    if(new_scr_del) {
        setup_scr(ui);
    }

    // 在加载新屏幕之前，根据目标屏幕决定是否显示状态栏
    if(setup_scr == setup_scr_home1 || setup_scr == setup_scr_screenHome2) {
        lv_obj_clear_flag(ui->status_bar, LV_OBJ_FLAG_HIDDEN);
        MLOG_DBG("status_bar: clear_flag_hidden\n");
        if(setup_scr == setup_scr_home1) {
            MLOG_DBG("home1\n");
        } else if(setup_scr == setup_scr_screenHome2) {
            MLOG_DBG("home2\n");
        }
    } else {
        lv_obj_add_flag(ui->status_bar, LV_OBJ_FLAG_HIDDEN);
        MLOG_DBG("status_bar: add_flag_hidden\n");
    }

    lv_screen_load_anim(*new_scr, anim_type, time, delay, auto_del);
    *old_scr_del = auto_del;
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
    ui->screenHome2_del                   = true;
    ui->screen_AITakePhoto_del            = true;
    ui->screen_TakePhoto_del              = true;
    ui->screen_AIDialog_del               = true;
    ui->screen_AIDialogDr_del             = true;
    ui->screen_AIDialogNZ_del             = true;
    ui->screen_AIDialogDS_del             = true;
    ui->screen_AIEffect_del               = true;
    ui->screen_AIEffectPic_del            = true;
    ui->screen_Settings_del               = true;
    ui->screen_SettingsSys_del            = true;
    ui->screen_SettingsSysWifi_del        = true;
    ui->screen_SettingsSysWifiCode_del    = true;
    ui->screen_SettingsSys4G_del          = true;
    ui->screen_SettingsSysVolume_del      = true;
    ui->screen_SettingsSysLuma_del        = true;
    ui->screen_SettingsSysBl_del          = true;
    ui->screen_SettingsSysAbout_del       = true;
    ui->screen_SettingsSysUpdate_del      = true;
    ui->screen_SettingsSysInfo_del        = true;
    ui->screen_SettingsSysLog_del         = true;
    ui->screen_SettingsSysService_del     = true;
    ui->screen_SettingsSysPrivacy_del     = true;
    ui->screen_SettingsAI_del             = true;
    ui->screen_PhotoAlbum_del             = true;
    ui->screen_PhotoAlbumPic_del          = true;
    ui->screen_PhotoAlbumVid_del          = true;
    ui->screen_Photo_del                  = true;
    ui->screen_AIPhoto_del                = true;
    ui->screen_AIPhotoResult_del          = true;
    ui->screen_AIPhotoResult1_del         = true;
    ui->screen_AIPhotoResult2_del         = true;
    ui->screen_AIPhotoResult3_del         = true;
    ui->screen_AIPhotoResult4_del         = true;
    ui->screen_TakePhotoSetting_del       = true;
    ui->screen_SettingResolution_del      = true;
    ui->screen_SettingWhiteBalance_del    = true;
    ui->screen_SettingPhotoEffect_del     = true;
    ui->screen_SettingExposure_del        = true;
    ui->screen_SettingPictureMode_del     = true;
    ui->screen_SettingSelfieTime_del      = true;
    ui->screen_SettingShootingMode_del    = true;
    ui->screen_SettingPictureQuality_del  = true;
    ui->screen_SettingSensitivity_del     = true;
    ui->screen_SettingAntiShake_del       = true;
    ui->screen_SettingAutofocus_del       = true;
    ui->screen_SettingFaceDectection_del  = true;
    ui->screen_SettingSmileDectection_del = true;
    ui->screen_SettingBeauty_del          = true;
    ui->screen_TakeChinese_del            = true;
    ui->screen_TakeEnglish_del            = true;
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
    setup_scr_home1(ui);
    lv_screen_load(ui->page_home1.scr);
}

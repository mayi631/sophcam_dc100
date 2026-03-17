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
#include "config.h"
// Add a timer pointer for the volume overlay hide timer
static lv_timer_t *settings_sys_volume_overlay_hide_timer = NULL;

// Add a static variable for simulated volume level
// static int simulated_volume_level = 50; // Initial volume level
extern int volume_level;

static void screen_SettingsSys_btn_wifi_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_syswifi.scr, g_ui.screen_SettingsSysWifi_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_SettingsSysWifi, LV_SCR_LOAD_ANIM_NONE,
                                  200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_btn_volumeg_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_sys4g.scr, g_ui.screen_SettingsSys4G_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_SettingsSys4G, LV_SCR_LOAD_ANIM_NONE,
                                  200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_btn_volume_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_sysvolume.scr, g_ui.screen_SettingsSysVolume_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_SettingsSysVolume,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_btn_luma_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_sysluma.scr, g_ui.screen_SettingsSysLuma_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_SettingsSysLuma, LV_SCR_LOAD_ANIM_NONE,
                                  200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_btn_backlight_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_sysbltim.scr, g_ui.screen_SettingsSysBl_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_SettingsSysBl, LV_SCR_LOAD_ANIM_NONE,
                                  200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_btn_about_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_sysabout.scr, g_ui.screen_SettingsSysAbout_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_SettingsSysAbout,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_settings.scr, g_ui.screen_Settings_del,
                                  &g_ui.screen_SettingsSys_del, setup_scr_screen_Settings, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                  false, true);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_volume_down_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // lv_obj_clear_flag(g_ui.screen_VolumeOverlay, LV_OBJ_FLAG_HIDDEN);
            // lv_obj_move_foreground(g_ui.screen_VolumeOverlay);
            break;
        }
        default: break;
    }
}

static void screen_SettingsSys_volume_up_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // lv_obj_clear_flag(g_ui.screen_VolumeOverlay, LV_OBJ_FLAG_HIDDEN);
            // lv_obj_move_foreground(g_ui.screen_VolumeOverlay);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingsSys(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_settingssys.btn_wifi, screen_SettingsSys_btn_wifi_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_volumeg, screen_SettingsSys_btn_volumeg_event_handler,
                        LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_volume, screen_SettingsSys_btn_volume_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_luma, screen_SettingsSys_btn_luma_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_backlight, screen_SettingsSys_btn_backlight_event_handler,
                        LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_about, screen_SettingsSys_btn_about_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_back, screen_SettingsSys_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_volume_down, screen_SettingsSys_volume_down_btn_event_handler,
                        LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settingssys.btn_volume_up, screen_SettingsSys_volume_up_btn_event_handler,
                        LV_EVENT_CLICKED, ui);
}

// Timer callback function to hide the volume overlay
static void settings_sys_hide_volume_overlay_timer_cb(lv_timer_t *timer)
{
    lv_ui_t *ui = timer->user_data;
    if(ui && ui->screen_VolumeOverlay) {
        lv_obj_add_flag(ui->screen_VolumeOverlay, LV_OBJ_FLAG_HIDDEN);
    }
}

// Function to show and update the volume overlay
static void show_and_update_volume_overlay(lv_ui_t *ui)
{
    if(ui && ui->screen_VolumeOverlay) {
        lv_obj_clear_flag(ui->screen_VolumeOverlay, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(ui->screen_VolumeOverlay);
        if(ui->screen_VolumeOverlay_bar) {
            // lv_bar_set_value(ui->screen_VolumeOverlay_bar, simulated_volume_level, LV_ANIM_OFF);
            lv_bar_set_value(ui->screen_VolumeOverlay_bar, volume_level, LV_ANIM_OFF);
        }
        if(ui->screen_VolumeOverlay_label) {
            // lv_label_set_text_fmt(ui->screen_VolumeOverlay_label, "音量: %d%%", simulated_volume_level);
            lv_label_set_text_fmt(ui->screen_VolumeOverlay_label, "音量: %d%%", volume_level);
        }

        // Reset or create the hide timer
        if(settings_sys_volume_overlay_hide_timer) {
            lv_timer_reset(settings_sys_volume_overlay_hide_timer);
        } else {
            settings_sys_volume_overlay_hide_timer =
                lv_timer_create(settings_sys_hide_volume_overlay_timer_cb, 3000, ui); // Hide after 3000 ms (3 seconds)
        }
    }
}

// Volume Up button event callback
static void volume_up_btn_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // if (simulated_volume_level < 100) {
    //     simulated_volume_level += 10;
    // }
    if(volume_level < 100) {
        volume_level += 10;
    }
    show_and_update_volume_overlay(ui);
}

// Volume Down button event callback
static void volume_down_btn_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // if (simulated_volume_level > 0) {
    //     simulated_volume_level -= 10;
    // }
    if(volume_level > 0) {
        volume_level -= 10;
    }
    show_and_update_volume_overlay(ui);
}

void setup_scr_screen_SettingsSys(lv_ui_t *ui)
{

    MLOG_DBG("loading page_settingssys...\n");

    SettingsSys_t *SettingSys = &ui->page_settingssys;
    SettingSys->del           = true;

    // 创建主页面1 容器
    if(SettingSys->scr != NULL) {
        if(lv_obj_is_valid(SettingSys->scr)) {
            MLOG_DBG("page_SettingSys->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SettingSys->scr);
        } else {
            MLOG_DBG("page_SettingSys->scr 已被自动销毁，仅重置指针\n");
        }
        SettingSys->scr = NULL;
    }

    // Write codes screen_SettingsSys
    SettingSys->scr = lv_obj_create(NULL);
    lv_obj_set_size(SettingSys->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SettingSys->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_main
    SettingSys->cont_main = lv_obj_create(SettingSys->scr);
    lv_obj_set_pos(SettingSys->cont_main, 0, 0);
    lv_obj_set_size(SettingSys->cont_main, 640, 480);
    lv_obj_set_scrollbar_mode(SettingSys->cont_main, LV_SCROLLBAR_MODE_OFF);

    // Write style for cont_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(SettingSys->cont_main, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(SettingSys->cont_main, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(SettingSys->cont_main, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(SettingSys->cont_main, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->cont_main, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_wifi
    SettingSys->btn_wifi = lv_button_create(SettingSys->cont_main);
    lv_obj_set_pos(SettingSys->btn_wifi, 46, 93);
    lv_obj_set_size(SettingSys->btn_wifi, 137, 142);
    SettingSys->label_wifi = lv_label_create(SettingSys->btn_wifi);
    lv_label_set_text(SettingSys->label_wifi, "Wi-Fi");
    lv_label_set_long_mode(SettingSys->label_wifi, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_wifi, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_wifi, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_wifi, LV_PCT(100));

    // Write style for btn_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_wifi, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_wifi, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_wifi, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_wifi, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_wifi, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_wifi, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_volumeg
    SettingSys->btn_volumeg = lv_button_create(SettingSys->cont_main);
    lv_obj_set_pos(SettingSys->btn_volumeg, 251, 93);
    lv_obj_set_size(SettingSys->btn_volumeg, 137, 142);
    SettingSys->label_volumeg = lv_label_create(SettingSys->btn_volumeg);
    lv_label_set_text(SettingSys->label_volumeg, "移动网络");
    lv_label_set_long_mode(SettingSys->label_volumeg, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_volumeg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_volumeg, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_volumeg, LV_PCT(100));

    // Write style for btn_volumeg, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_volumeg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_volumeg, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_volumeg, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_volumeg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_volumeg, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_volumeg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_volumeg, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_volumeg, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_volumeg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_volumeg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_volume
    SettingSys->btn_volume = lv_button_create(SettingSys->cont_main);
    lv_obj_set_pos(SettingSys->btn_volume, 448, 93);
    lv_obj_set_size(SettingSys->btn_volume, 137, 142);
    SettingSys->label_volume = lv_label_create(SettingSys->btn_volume);
    lv_label_set_text(SettingSys->label_volume, "音量");
    lv_label_set_long_mode(SettingSys->label_volume, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_volume, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_volume, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_volume, LV_PCT(100));

    // Write style for btn_volume, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_volume, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_volume, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_volume, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_volume, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_volume, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_volume, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_volume, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_volume, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_volume, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_volume, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_luma
    SettingSys->btn_luma = lv_button_create(SettingSys->cont_main);
    lv_obj_set_pos(SettingSys->btn_luma, 46, 271);
    lv_obj_set_size(SettingSys->btn_luma, 137, 142);
    SettingSys->label_luma = lv_label_create(SettingSys->btn_luma);
    lv_label_set_text(SettingSys->label_luma, "亮度");
    lv_label_set_long_mode(SettingSys->label_luma, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_luma, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_luma, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_luma, LV_PCT(100));

    // Write style for btn_luma, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_luma, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_luma, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_luma, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_luma, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_luma, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_luma, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_luma, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_luma, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_luma, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_luma, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_backlight
    SettingSys->btn_backlight = lv_button_create(SettingSys->cont_main);
    lv_obj_set_pos(SettingSys->btn_backlight, 251, 272);
    lv_obj_set_size(SettingSys->btn_backlight, 137, 142);
    SettingSys->label_backlight = lv_label_create(SettingSys->btn_backlight);
    lv_label_set_text(SettingSys->label_backlight, "亮屏时间");
    lv_label_set_long_mode(SettingSys->label_backlight, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_backlight, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_backlight, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_backlight, LV_PCT(100));

    // Write style for btn_backlight, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_backlight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_backlight, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_backlight, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_backlight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_backlight, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_backlight, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_backlight, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_backlight, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_backlight, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_backlight, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_about
    SettingSys->btn_about = lv_button_create(SettingSys->cont_main);
    lv_obj_set_pos(SettingSys->btn_about, 448, 272);
    lv_obj_set_size(SettingSys->btn_about, 137, 142);
    SettingSys->label_about = lv_label_create(SettingSys->btn_about);
    lv_label_set_text(SettingSys->label_about, "关于");
    lv_label_set_long_mode(SettingSys->label_about, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_about, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_about, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_about, LV_PCT(100));

    // Write style for btn_about, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_about, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_about, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_about, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_about, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_about, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_about, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_about, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_about, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_about, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_about, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    SettingSys->btn_back = lv_button_create(SettingSys->scr);
    lv_obj_set_pos(SettingSys->btn_back, 0, 0);
    lv_obj_set_size(SettingSys->btn_back, 77, 56);
    SettingSys->label_back = lv_label_create(SettingSys->btn_back);
    lv_label_set_text(SettingSys->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(SettingSys->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_back, lv_color_hex(0xfff290), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_back, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_back, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_back, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes title
    SettingSys->title = lv_label_create(SettingSys->scr);
    lv_obj_set_pos(SettingSys->title, 247, 15);
    lv_obj_set_size(SettingSys->title, 100, 32);
    lv_label_set_text(SettingSys->title, "Label");
    lv_label_set_long_mode(SettingSys->title, LV_LABEL_LONG_WRAP);

    // Write style for title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->title, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->title, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_volume_down
    SettingSys->btn_volume_down = lv_button_create(SettingSys->scr);
    lv_obj_set_pos(SettingSys->btn_volume_down, 43, 429);
    lv_obj_set_size(SettingSys->btn_volume_down, 83, 29);
    SettingSys->label_volume_down = lv_label_create(SettingSys->btn_volume_down);
    lv_label_set_text(SettingSys->label_volume_down, "");
    lv_label_set_long_mode(SettingSys->label_volume_down, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_volume_down, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_volume_down, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_volume_down, LV_PCT(100));

    // Write style for btn_volume_down, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_volume_down, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_volume_down, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_volume_down, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_volume_down, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_volume_down, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_volume_down, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_volume_down, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_volume_down, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_volume_down, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_volume_down, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_volume_up
    SettingSys->btn_volume_up = lv_button_create(SettingSys->scr);
    lv_obj_set_pos(SettingSys->btn_volume_up, 496, 430);
    lv_obj_set_size(SettingSys->btn_volume_up, 95, 27);
    SettingSys->label_volume_up = lv_label_create(SettingSys->btn_volume_up);
    lv_label_set_text(SettingSys->label_volume_up, "");
    lv_label_set_long_mode(SettingSys->label_volume_up, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingSys->label_volume_up, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingSys->btn_volume_up, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingSys->label_volume_up, LV_PCT(100));

    // Write style for btn_volume_up, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingSys->btn_volume_up, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_volume_up, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingSys->btn_volume_up, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys->btn_volume_up, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingSys->btn_volume_up, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingSys->btn_volume_up, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_volume_up, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->btn_volume_up, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingSys->btn_volume_up, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->btn_volume_up, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of scr.
    //  setup_scr_screen_VolumeOverlay(SettingSys);
    // lv_screen_load(SettingSys->screen_VolumeOverlay);

    // Set background color to dark gray
    lv_obj_set_style_bg_opa(SettingSys->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 顶部返回按钮（黄色背景） - Similar to WiFi/4G
    // SettingSys->btn_back = lv_btn_create(SettingSys->scr);
    lv_obj_set_pos(SettingSys->btn_back, 16, 16);
    lv_obj_set_size(SettingSys->btn_back, 80, 40);
    lv_obj_set_style_radius(SettingSys->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_add_event_cb(SettingSys->btn_back, sys_back_cb, LV_EVENT_CLICKED, ui); // Add placeholder
    // callback 返回按钮图标（作为按钮的子控件） SettingSys->label_back =
    // lv_label_create(SettingSys->btn_back);
    lv_label_set_text(SettingSys->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SettingSys->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SettingSys->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"系统设置"
    // SettingSys->title = lv_label_create(SettingSys->scr);
    lv_label_set_text(SettingSys->title, "系统设置");
    lv_obj_set_style_text_color(SettingSys->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys->title, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SettingSys->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SettingSys->title, LV_ALIGN_TOP_MID, 0, 24); // Align below the back button area

    lv_obj_set_style_bg_color(SettingSys->btn_wifi, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_wifi, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_volumeg, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_volumeg, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_volume, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_volume, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_luma, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_luma, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_backlight, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_backlight, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys->btn_about, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingSys->btn_about, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add Volume Up and Down buttons (temporary for testing)
    // SettingSys->btn_volume_down = lv_btn_create(SettingSys->scr);
    lv_obj_set_size(SettingSys->btn_volume_down, 80, 40);
    lv_obj_align(SettingSys->btn_volume_down, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_event_cb(SettingSys->btn_volume_down, volume_down_btn_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *down_label = lv_label_create(SettingSys->btn_volume_down);
    lv_label_set_text(down_label, "音量-");
    lv_obj_center(down_label);
    lv_obj_set_style_text_color(down_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(down_label, &lv_font_SourceHanSerifSC_Regular_16, LV_STATE_DEFAULT);

    // SettingSys->btn_volume_up = lv_btn_create(SettingSys->scr);
    lv_obj_set_size(SettingSys->btn_volume_up, 80, 40);
    lv_obj_align(SettingSys->btn_volume_up, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_event_cb(SettingSys->btn_volume_up, volume_up_btn_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *up_label = lv_label_create(SettingSys->btn_volume_up);
    lv_label_set_text(up_label, "音量+");
    lv_obj_center(up_label);
    lv_obj_set_style_text_color(up_label, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(up_label, &lv_font_SourceHanSerifSC_Regular_16, LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(SettingSys->scr);

    // Init events for screen.
    events_init_screen_SettingsSys(ui);
}

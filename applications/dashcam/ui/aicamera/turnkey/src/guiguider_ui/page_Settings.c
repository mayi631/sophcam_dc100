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

static void screen_Settings_btn_ai_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_settingsai.scr, g_ui.screen_SettingsAI_del,
                                  &g_ui.screen_Settings_del, setup_scr_screen_SettingsAI, LV_SCR_LOAD_ANIM_NONE, 200,
                                  200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_Settings_btn_sys_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_settingssys.scr, g_ui.screen_SettingsSys_del,
                                  &g_ui.screen_Settings_del, setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200,
                                  200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_Settings_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_home2.scr, g_ui.screenHome2_del, &g_ui.screen_Settings_del,
                                  setup_scr_screenHome2, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_Settings(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_settings.btn_ai, screen_Settings_btn_ai_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settings.btn_sys, screen_Settings_btn_sys_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_settings.btn_back, screen_Settings_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_Settings(lv_ui_t *ui)
{
    MLOG_DBG("loading page_Settings...\n");

    Settings_t *Settings = &ui->page_settings;
    Settings->del        = true;

    // 创建主页面1 容器
    if(Settings->scr != NULL) {
        if(lv_obj_is_valid(Settings->scr)) {
            MLOG_DBG("page_Settings->scr 仍然有效，删除旧对象\n");
            lv_obj_del(Settings->scr);
        } else {
            MLOG_DBG("page_Settings->scr 已被自动销毁，仅重置指针\n");
        }
        Settings->scr = NULL;
    }

    // Write codes scr
    Settings->scr = lv_obj_create(NULL);
    lv_obj_set_size(Settings->scr, 640, 480);
    lv_obj_set_scrollbar_mode(Settings->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(Settings->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(Settings->scr, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(Settings->scr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes scr_btn_ai
    Settings->btn_ai = lv_button_create(Settings->scr);
    lv_obj_set_pos(Settings->btn_ai, 380, 130);
    lv_obj_set_size(Settings->btn_ai, 136, 193);
    Settings->label_ai = lv_label_create(Settings->btn_ai);
    lv_label_set_text(Settings->label_ai, "AI设置");
    lv_label_set_long_mode(Settings->label_ai, LV_LABEL_LONG_WRAP);
    lv_obj_align(Settings->label_ai, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(Settings->btn_ai, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(Settings->label_ai, LV_PCT(100));

    // Write style for btn_ai, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(Settings->btn_ai, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(Settings->btn_ai, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(Settings->btn_ai, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(Settings->btn_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(Settings->btn_ai, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(Settings->btn_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(Settings->btn_ai, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(Settings->btn_ai, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(Settings->btn_ai, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(Settings->btn_ai, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_sys
    Settings->btn_sys = lv_button_create(Settings->scr);
    lv_obj_set_pos(Settings->btn_sys, 128, 130);
    lv_obj_set_size(Settings->btn_sys, 136, 193);
    Settings->label_sys = lv_label_create(Settings->btn_sys);
    lv_label_set_text(Settings->label_sys, "系统设置");
    lv_label_set_long_mode(Settings->label_sys, LV_LABEL_LONG_WRAP);
    lv_obj_align(Settings->label_sys, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(Settings->btn_sys, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(Settings->label_sys, LV_PCT(100));

    // Write style for btn_sys, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(Settings->btn_sys, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(Settings->btn_sys, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(Settings->btn_sys, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(Settings->btn_sys, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(Settings->btn_sys, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(Settings->btn_sys, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(Settings->btn_sys, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(Settings->btn_sys, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(Settings->btn_sys, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(Settings->btn_sys, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    Settings->btn_back = lv_button_create(Settings->scr);
    lv_obj_set_pos(Settings->btn_back, 0, 0);
    lv_obj_set_size(Settings->btn_back, 77, 56);
    Settings->label_back = lv_label_create(Settings->btn_back);
    lv_label_set_text(Settings->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(Settings->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(Settings->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(Settings->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(Settings->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(Settings->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(Settings->btn_back, lv_color_hex(0xfff290), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(Settings->btn_back, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(Settings->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(Settings->btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(Settings->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(Settings->btn_back, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(Settings->btn_back, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(Settings->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(Settings->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes title
    Settings->title = lv_label_create(Settings->scr);
    lv_obj_set_pos(Settings->title, 276, 21);
    lv_obj_set_size(Settings->title, 100, 32);
    lv_label_set_text(Settings->title, "");
    lv_label_set_long_mode(Settings->title, LV_LABEL_LONG_WRAP);

    // Write style for title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(Settings->title, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(Settings->title, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(Settings->title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(Settings->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(Settings->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of scr.
    lv_obj_set_style_bg_color(Settings->scr, lv_color_hex(0x181818),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // Dark background
                                                                // 顶部返回按钮（黄色背景）
    // Settings->btn_back = lv_btn_create(Settings->scr);
    lv_obj_set_pos(Settings->btn_back, 16, 16);
    lv_obj_set_size(Settings->btn_back, 80, 40);
    lv_obj_set_style_radius(Settings->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(Settings->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_add_event_cb(Settings->btn_back, events_init_screen_Settings, LV_EVENT_CLICKED, Settings);
    // 返回按钮图标
    // Settings->label_back = lv_label_create(Settings->btn_back);
    lv_label_set_text(Settings->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(Settings->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(Settings->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(Settings->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"设置"
    // Settings->title = lv_label_create(Settings->scr);
    lv_label_set_text(Settings->title, "设置");
    lv_obj_set_style_text_color(Settings->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(Settings->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(Settings->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(Settings->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(Settings->title, LV_ALIGN_TOP_MID, 0, 24);

    lv_obj_set_style_bg_color(Settings->btn_ai, lv_color_hex(0x2C2C2C),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // Darker background
    lv_obj_set_style_text_color(Settings->btn_ai, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT); // White text
    lv_obj_set_style_bg_color(Settings->btn_sys, lv_color_hex(0x2C2C2C),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // Darker background
    lv_obj_set_style_text_color(Settings->btn_sys, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT); // White text

    // Update current screen layout.
    lv_obj_update_layout(Settings->scr);

    // Init events for screen.
    events_init_screen_Settings(ui);
}

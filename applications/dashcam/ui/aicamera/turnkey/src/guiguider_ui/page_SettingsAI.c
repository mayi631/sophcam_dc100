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

static void screen_SettingsAI_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_settings.scr, g_ui.screen_Settings_del, &g_ui.screen_SettingsAI_del,
                                  setup_scr_screen_Settings, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_SettingsAI(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_settingsai.btn_back, screen_SettingsAI_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_SettingsAI(lv_ui_t *ui)
{
    MLOG_DBG("loading pageSettingsAI...\n");

    SettingsAI_t *SettingsAI = &ui->page_settingsai;
    SettingsAI->del          = true;

    // 创建主页面1 容器
    if(SettingsAI->scr != NULL) {
        if(lv_obj_is_valid(SettingsAI->scr)) {
            MLOG_DBG("page_SettingsAI->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SettingsAI->scr);
        } else {
            MLOG_DBG("page_SettingsAI->scr 已被自动销毁，仅重置指针\n");
        }
        SettingsAI->scr = NULL;
    }

    // Write codes scr
    SettingsAI->scr = lv_obj_create(NULL);
    lv_obj_set_size(SettingsAI->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SettingsAI->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingsAI->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_voice
    SettingsAI->btn_voice = lv_button_create(SettingsAI->scr);
    lv_obj_set_pos(SettingsAI->btn_voice, 380, 130);
    lv_obj_set_size(SettingsAI->btn_voice, 136, 193);
    SettingsAI->label_voice = lv_label_create(SettingsAI->btn_voice);
    lv_label_set_text(SettingsAI->label_voice, "语音音色");
    lv_label_set_long_mode(SettingsAI->label_voice, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingsAI->label_voice, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingsAI->btn_voice, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingsAI->label_voice, LV_PCT(100));

    // Write style for btn_voice, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingsAI->btn_voice, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingsAI->btn_voice, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingsAI->btn_voice, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingsAI->btn_voice, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingsAI->btn_voice, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingsAI->btn_voice, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->btn_voice, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingsAI->btn_voice, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingsAI->btn_voice, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingsAI->btn_voice, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_content
    SettingsAI->btn_content = lv_button_create(SettingsAI->scr);
    lv_obj_set_pos(SettingsAI->btn_content, 128, 130);
    lv_obj_set_size(SettingsAI->btn_content, 136, 193);
    SettingsAI->label_content = lv_label_create(SettingsAI->btn_content);
    lv_label_set_text(SettingsAI->label_content, "内容级别");
    lv_label_set_long_mode(SettingsAI->label_content, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingsAI->label_content, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingsAI->btn_content, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingsAI->label_content, LV_PCT(100));

    // Write style for btn_content, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingsAI->btn_content, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingsAI->btn_content, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingsAI->btn_content, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingsAI->btn_content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingsAI->btn_content, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingsAI->btn_content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->btn_content, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingsAI->btn_content, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingsAI->btn_content, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingsAI->btn_content, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    SettingsAI->btn_back = lv_button_create(SettingsAI->scr);
    lv_obj_set_pos(SettingsAI->btn_back, 0, 0);
    lv_obj_set_size(SettingsAI->btn_back, 77, 56);
    SettingsAI->label_back = lv_label_create(SettingsAI->btn_back);
    lv_label_set_text(SettingsAI->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(SettingsAI->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(SettingsAI->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(SettingsAI->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(SettingsAI->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingsAI->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingsAI->btn_back, lv_color_hex(0xfff290), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(SettingsAI->btn_back, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingsAI->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingsAI->btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingsAI->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->btn_back, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingsAI->btn_back, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingsAI->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingsAI->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes title
    SettingsAI->title = lv_label_create(SettingsAI->scr);
    lv_obj_set_pos(SettingsAI->title, 264, 21);
    lv_obj_set_size(SettingsAI->title, 100, 32);
    lv_label_set_text(SettingsAI->title, "");
    lv_label_set_long_mode(SettingsAI->title, LV_LABEL_LONG_WRAP);

    // Write style for title, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->title, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingsAI->title, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(SettingsAI->title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingsAI->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(SettingsAI->title, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of scr.
    lv_obj_set_style_bg_opa(SettingsAI->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingsAI->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    //  SettingsAI->btn_back = lv_button_create(SettingsAI->scr);
    lv_obj_set_pos(SettingsAI->btn_back, 20, 20);
    lv_obj_set_size(SettingsAI->btn_back, 40, 40);
    // SettingsAI->label_back = lv_label_create(SettingsAI->btn_back);
    lv_label_set_text(SettingsAI->label_back, LV_SYMBOL_LEFT);
    lv_obj_align(SettingsAI->label_back, LV_ALIGN_CENTER, 0, 0);
    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(SettingsAI->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingsAI->btn_back, lv_color_hex(0xFFD700), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SettingsAI->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->btn_back, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingsAI->btn_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes title
    //  SettingsAI->title = lv_label_create(SettingsAI->scr);
    lv_obj_set_pos(SettingsAI->title, 0, 20);
    lv_obj_set_size(SettingsAI->title, 640, 40);
    lv_label_set_text(SettingsAI->title, "AI设置");
    lv_obj_set_style_text_color(SettingsAI->title, lv_color_hex(0xFFFFFF), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingsAI->title, &lv_font_SourceHanSerifSC_Regular_16, LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingsAI->title, LV_TEXT_ALIGN_CENTER, LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(SettingsAI->btn_content, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->btn_content, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingsAI->btn_voice, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(SettingsAI->btn_voice, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(SettingsAI->scr);

    // Init events for screen.
    events_init_screen_SettingsAI(ui);
}

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

static void screen_AIDialogNZ_btn_talk_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_PRESSED: {
            break;
        }
        default: break;
    }
}

static void screen_AIDialogNZ_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aidialog.scr, g_ui.screen_AIDialog_del, &g_ui.screen_AIDialogNZ_del,
                                  setup_scr_screen_AIDialog, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_AIDialogNZ(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_aidialognz.btn_talk, screen_AIDialogNZ_btn_talk_event_handler, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->page_aidialognz.btn_back, screen_AIDialogNZ_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_AIDialogNZ(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIDialogDS...\n");

    AIDialogNZ_t *AIDialogNZ = &ui->page_aidialognz;
    AIDialogNZ->del          = true;

    // 创建主页面1 容器
    if(AIDialogNZ->scr != NULL) {
        if(lv_obj_is_valid(AIDialogNZ->scr)) {
            MLOG_DBG("page_AIDialogDS->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIDialogNZ->scr);
        } else {
            MLOG_DBG("page_AIDialogDS->scr 已被自动销毁，仅重置指针\n");
        }
        AIDialogNZ->scr = NULL;
    }

    // Write codes scr
    AIDialogNZ->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIDialogNZ->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIDialogNZ->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogNZ->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogNZ->scr, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogNZ->scr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont
    AIDialogNZ->top_cont = lv_obj_create(AIDialogNZ->scr);
    lv_obj_set_pos(AIDialogNZ->top_cont, 0, 0);
    lv_obj_set_size(AIDialogNZ->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIDialogNZ->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIDialogNZ->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogNZ->top_cont, lv_color_hex(0xfff290), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogNZ->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogNZ->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_talk
    AIDialogNZ->btn_talk = lv_button_create(AIDialogNZ->scr);
    lv_obj_set_pos(AIDialogNZ->btn_talk, 139, 428);
    lv_obj_set_size(AIDialogNZ->btn_talk, 361, 38);
    AIDialogNZ->label_talk = lv_label_create(AIDialogNZ->btn_talk);
    lv_label_set_text(AIDialogNZ->label_talk, "按住对话 ");
    lv_label_set_long_mode(AIDialogNZ->label_talk, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialogNZ->label_talk, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialogNZ->btn_talk, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialogNZ->label_talk, LV_PCT(100));

    // Write style for btn_talk, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogNZ->btn_talk, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogNZ->btn_talk, lv_color_hex(0xbad7f0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogNZ->btn_talk, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialogNZ->btn_talk, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogNZ->btn_talk, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogNZ->btn_talk, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialogNZ->btn_talk, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialogNZ->btn_talk, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialogNZ->btn_talk, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialogNZ->btn_talk, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_nezha
    AIDialogNZ->btn_nezha = lv_button_create(AIDialogNZ->scr);
    lv_obj_set_pos(AIDialogNZ->btn_nezha, 252, 130);
    lv_obj_set_size(AIDialogNZ->btn_nezha, 136, 193);
    AIDialogNZ->label_nezha = lv_label_create(AIDialogNZ->btn_nezha);
    lv_label_set_text(AIDialogNZ->label_nezha, "哪吒");
    lv_label_set_long_mode(AIDialogNZ->label_nezha, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialogNZ->label_nezha, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialogNZ->btn_nezha, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialogNZ->label_nezha, LV_PCT(100));

    // Write style for btn_nezha, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogNZ->btn_nezha, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogNZ->btn_nezha, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogNZ->btn_nezha, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialogNZ->btn_nezha, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogNZ->btn_nezha, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogNZ->btn_nezha, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialogNZ->btn_nezha, lv_color_hex(0x12548b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialogNZ->btn_nezha, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialogNZ->btn_nezha, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialogNZ->btn_nezha, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    AIDialogNZ->btn_back = lv_button_create(AIDialogNZ->scr);
    lv_obj_set_pos(AIDialogNZ->btn_back, 0, 0);
    lv_obj_set_size(AIDialogNZ->btn_back, 77, 56);
    AIDialogNZ->label_back = lv_label_create(AIDialogNZ->btn_back);
    lv_label_set_text(AIDialogNZ->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIDialogNZ->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialogNZ->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialogNZ->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialogNZ->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogNZ->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialogNZ->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogNZ->btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogNZ->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialogNZ->btn_back, lv_color_hex(0xf7d4de), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialogNZ->btn_back, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialogNZ->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialogNZ->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of scr.

    // Update current screen layout.
    lv_obj_update_layout(AIDialogNZ->scr);

    // Init events for screen.
    events_init_screen_AIDialogNZ(ui);
}

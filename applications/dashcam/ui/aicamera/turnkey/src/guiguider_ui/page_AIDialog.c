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
#include "config.h"
#include "custom.h"

static void screen_AIDialog_btn_dr_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_state(g_ui.page_aidialog.cont_dr, LV_STATE_CHECKED);
            lv_obj_clear_state(g_ui.page_aidialog.cont_nezha, LV_STATE_CHECKED);
            lv_obj_clear_state(g_ui.page_aidialog.cont_deepseek, LV_STATE_CHECKED);
            break;
        }
        default: break;
    }
}

static void screen_AIDialog_btn_nezha_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_state(g_ui.page_aidialog.cont_nezha, LV_STATE_CHECKED);
            lv_obj_clear_state(g_ui.page_aidialog.cont_dr, LV_STATE_CHECKED);
            lv_obj_clear_state(g_ui.page_aidialog.cont_deepseek, LV_STATE_CHECKED);
            break;
        }
        default: break;
    }
}

static void screen_AIDialog_btn_deepseek_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_state(g_ui.page_aidialog.cont_deepseek, LV_STATE_CHECKED);
            lv_obj_clear_state(g_ui.page_aidialog.cont_dr, LV_STATE_CHECKED);
            lv_obj_clear_state(g_ui.page_aidialog.cont_nezha, LV_STATE_CHECKED);
            break;
        }
        default: break;
    }
}

static void screen_AIDialog_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_home1.scr, g_ui.screenHome1_del, &g_ui.screen_AIDialog_del,
                                  setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screen_AIDialog_btn_talk_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取当前屏幕的UI对象
            // lv_ui_t *ui = lv_event_get_user_data(e);

            // 检查哪个按钮处于焦点状态
            if(lv_obj_has_state(g_ui.page_aidialog.cont_dr, LV_STATE_CHECKED)) {
                // 跳转到"百科博士"对话页面
                // lv_scr_load_anim(ui->screen_AIDialogDr, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
                ui_load_scr_animation(&g_ui, &g_ui.page_aidialogdr.scr, g_ui.screen_AIDialogDr_del,
                                      &g_ui.screen_AIDialog_del, setup_scr_screen_AIDialogDr, LV_SCR_LOAD_ANIM_NONE,
                                      200, 200, false, true);
            } else if(lv_obj_has_state(g_ui.page_aidialog.cont_nezha, LV_STATE_CHECKED)) {
                // 跳转到"哪吒"对话页面
                // lv_scr_load_anim(g_ui.screen_AIDialogNZ, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
                ui_load_scr_animation(&g_ui, &g_ui.page_aidialognz.scr, g_ui.screen_AIDialogNZ_del,
                                      &g_ui.screen_AIDialog_del, setup_scr_screen_AIDialogNZ, LV_SCR_LOAD_ANIM_NONE,
                                      200, 200, false, true);
            } else if(lv_obj_has_state(g_ui.page_aidialog.cont_deepseek, LV_STATE_CHECKED)) {
                // 跳转到"DeepSeek儿童版"对话页面
                // lv_scr_load_anim(g_ui.screen_AIDialogDS, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
                ui_load_scr_animation(&g_ui, &g_ui.page_aidialogds.scr, g_ui.screen_AIDialogDS_del,
                                      &g_ui.screen_AIDialog_del, setup_scr_screen_AIDialogDS, LV_SCR_LOAD_ANIM_NONE,
                                      200, 200, false, true);
            }
            break;
        }
        default: break;
    }
}

void events_init_screen_AIDialog(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_aidialog.btn_dr, screen_AIDialog_btn_dr_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aidialog.btn_nezha, screen_AIDialog_btn_nezha_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aidialog.btn_deepseek, screen_AIDialog_btn_deepseek_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(ui->page_aidialog.btn_back, screen_AIDialog_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_aidialog.btn_talk, screen_AIDialog_btn_talk_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_AIDialog(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIDialog...\n");
    AIDialog_t *AIDialog = &ui->page_aidialog;
    AIDialog->del        = true;

    // 创建主页面1 容器
    if(AIDialog->scr != NULL) {
        if(lv_obj_is_valid(AIDialog->scr)) {
            MLOG_DBG("page_AIDialog->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIDialog->scr);
        } else {
            MLOG_DBG("page_AIDialog->scr 已被自动销毁，仅重置指针\n");
        }
        AIDialog->scr = NULL;
    }

    // Write codes scr
    AIDialog->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIDialog->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIDialog->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialog->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialog->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialog->scr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes scr_cont_top (顶部栏)
    AIDialog->top_cont = lv_obj_create(AIDialog->scr);
    lv_obj_set_pos(AIDialog->top_cont, 0, 0);
    lv_obj_set_size(AIDialog->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIDialog->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIDialog->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialog->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialog->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialog->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_AIDialog_btn_back (返回按钮)
    AIDialog->btn_back = lv_button_create(AIDialog->top_cont);

    lv_obj_set_pos(AIDialog->btn_back, 16, 10);
    lv_obj_set_size(AIDialog->btn_back, 40, 40);
    lv_obj_set_style_radius(AIDialog->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialog->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);

    AIDialog->label_back = lv_label_create(AIDialog->btn_back);

    lv_label_set_text(AIDialog->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(AIDialog->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialog->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(AIDialog->label_back, LV_ALIGN_CENTER, 0, 0);

    // Write codes title (标题)
    AIDialog->title = lv_label_create(AIDialog->top_cont);

    lv_label_set_text(AIDialog->title, "AI对话");
    lv_obj_set_style_text_color(AIDialog->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialog->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(AIDialog->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialog->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(AIDialog->title, LV_ALIGN_CENTER, 0, 0);

    // --- 选中态样式 ---
    static lv_style_t style_cont_checked;
    if(style_cont_checked.prop_cnt > 1)
        lv_style_reset(&style_cont_checked);
    else
        lv_style_init(&style_cont_checked);
    lv_style_set_border_width(&style_cont_checked, 6);
    lv_style_set_border_color(&style_cont_checked, lv_color_hex(0x42A5F5));

    // --- 百科博士 ---
    AIDialog->cont_dr = lv_obj_create(AIDialog->scr);
    lv_obj_set_pos(AIDialog->cont_dr, 42, 125);
    lv_obj_set_size(AIDialog->cont_dr, 146, 207);
    lv_obj_set_style_bg_opa(AIDialog->cont_dr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialog->cont_dr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(AIDialog->cont_dr, &style_cont_checked, LV_STATE_CHECKED);
    lv_obj_set_scrollbar_mode(AIDialog->cont_dr, LV_SCROLLBAR_MODE_OFF);
    AIDialog->btn_dr = lv_imgbtn_create(AIDialog->cont_dr);
    lv_obj_set_size(AIDialog->btn_dr, 136, 197);
    lv_obj_center(AIDialog->btn_dr);
    lv_imgbtn_set_src(AIDialog->btn_dr, LV_IMGBTN_STATE_RELEASED, NULL, &DIALOG_DR_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_dr, LV_IMGBTN_STATE_PRESSED, NULL, &DIALOG_DR_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_dr, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &DIALOG_DR_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_dr, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &DIALOG_DR_IMAGE_PATH, NULL);
    lv_obj_clear_flag(AIDialog->btn_dr, LV_OBJ_FLAG_SCROLLABLE);
    AIDialog->label_dr = lv_label_create(AIDialog->scr);
    lv_label_set_text(AIDialog->label_dr, "百科博士");
    lv_obj_align_to(AIDialog->label_dr, AIDialog->cont_dr, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_text_color(AIDialog->label_dr, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialog->label_dr, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialog->label_dr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialog->label_dr, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // --- 哪吒 ---
    AIDialog->cont_nezha = lv_obj_create(AIDialog->scr);
    lv_obj_set_pos(AIDialog->cont_nezha, 247, 125);
    lv_obj_set_size(AIDialog->cont_nezha, 146, 207);
    lv_obj_set_style_bg_opa(AIDialog->cont_nezha, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialog->cont_nezha, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(AIDialog->cont_nezha, &style_cont_checked, LV_STATE_CHECKED);
    lv_obj_set_scrollbar_mode(AIDialog->cont_nezha, LV_SCROLLBAR_MODE_OFF);
    AIDialog->btn_nezha = lv_imgbtn_create(AIDialog->cont_nezha);
    lv_obj_set_size(AIDialog->btn_nezha, 136, 197);
    lv_obj_center(AIDialog->btn_nezha);
    lv_imgbtn_set_src(AIDialog->btn_nezha, LV_IMGBTN_STATE_RELEASED, NULL, &DIALOG_NEZHA_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_nezha, LV_IMGBTN_STATE_PRESSED, NULL, &DIALOG_NEZHA_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_nezha, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &DIALOG_NEZHA_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_nezha, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &DIALOG_NEZHA_IMAGE_PATH, NULL);
    lv_obj_clear_flag(AIDialog->btn_nezha, LV_OBJ_FLAG_SCROLLABLE);
    AIDialog->label_nezha = lv_label_create(AIDialog->scr);
    lv_label_set_text(AIDialog->label_nezha, "哪吒");
    lv_obj_align_to(AIDialog->label_nezha, AIDialog->cont_nezha, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_text_color(AIDialog->label_nezha, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialog->label_nezha, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialog->label_nezha, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialog->label_nezha, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // --- DeepSeek儿童版 ---
    AIDialog->cont_deepseek = lv_obj_create(AIDialog->scr);
    lv_obj_set_pos(AIDialog->cont_deepseek, 448, 125);
    lv_obj_set_size(AIDialog->cont_deepseek, 146, 207);
    lv_obj_set_style_bg_opa(AIDialog->cont_deepseek, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialog->cont_deepseek, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(AIDialog->cont_deepseek, &style_cont_checked, LV_STATE_CHECKED);
    lv_obj_set_scrollbar_mode(AIDialog->cont_deepseek, LV_SCROLLBAR_MODE_OFF);
    AIDialog->btn_deepseek = lv_imgbtn_create(AIDialog->cont_deepseek);
    lv_obj_set_size(AIDialog->btn_deepseek, 136, 197);
    lv_obj_center(AIDialog->btn_deepseek);
    lv_imgbtn_set_src(AIDialog->btn_deepseek, LV_IMGBTN_STATE_RELEASED, NULL, &DIALOG_DEEPSEEK_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_deepseek, LV_IMGBTN_STATE_PRESSED, NULL, &DIALOG_DEEPSEEK_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_deepseek, LV_IMGBTN_STATE_CHECKED_RELEASED, NULL, &DIALOG_DEEPSEEK_IMAGE_PATH, NULL);
    lv_imgbtn_set_src(AIDialog->btn_deepseek, LV_IMGBTN_STATE_CHECKED_PRESSED, NULL, &DIALOG_DEEPSEEK_IMAGE_PATH, NULL);
    lv_obj_clear_flag(AIDialog->btn_deepseek, LV_OBJ_FLAG_SCROLLABLE);
    AIDialog->label_deepseek = lv_label_create(AIDialog->scr);
    lv_label_set_text(AIDialog->label_deepseek, "DeepSeek儿童版");
    lv_obj_align_to(AIDialog->label_deepseek, AIDialog->cont_deepseek, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_text_color(AIDialog->label_deepseek, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialog->label_deepseek, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialog->label_deepseek, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialog->label_deepseek, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_talk (进入对话按钮)
    AIDialog->btn_talk = lv_button_create(AIDialog->scr);
    lv_obj_set_pos(AIDialog->btn_talk, 139, 428);
    lv_obj_set_size(AIDialog->btn_talk, 361, 38);
    AIDialog->label_talk = lv_label_create(AIDialog->btn_talk);

    lv_label_set_text(AIDialog->label_talk, "进入对话");
    lv_label_set_long_mode(AIDialog->label_talk, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialog->label_talk, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialog->btn_talk, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialog->label_talk, LV_PCT(100));

    // Write style for btn_talk, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialog->btn_talk, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialog->btn_talk, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialog->btn_talk, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialog->btn_talk, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialog->btn_talk, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialog->btn_talk, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialog->btn_talk, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialog->btn_talk, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialog->btn_talk, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialog->btn_talk, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(AIDialog->scr);

    // Init events for screen.
    events_init_screen_AIDialog(ui);
}

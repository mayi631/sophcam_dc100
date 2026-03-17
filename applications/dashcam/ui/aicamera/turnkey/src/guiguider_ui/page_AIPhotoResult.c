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

#include "custom.h"
#include "config.h"

char g_ai_photo_result_text[1024] = "AI识别结果";

static AIPhotoResult_t *AIPhotoResult;

// 事件处理函数
static void page_aiphotoresult_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphoto.scr, g_ui.screen_AIPhoto_del,
                                  &g_ui.screen_AIPhotoResult_del, setup_scr_screen_AIPhoto, LV_SCR_LOAD_ANIM_NONE, 200,
                                  200, false, true);
            break;
        }
        default: break;
    }
}

// 其他按钮事件处理函数
static void screen_AIPhotoResult_btn_science_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphotoresult1.scr, g_ui.screen_AIPhotoResult1_del,
                                  &g_ui.screen_AIPhotoResult_del, setup_scr_screen_AIPhotoResult1,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_AIPhotoResult_btn_chinese_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphotoresult2.scr, g_ui.screen_AIPhotoResult2_del,
                                  &g_ui.screen_AIPhotoResult_del, setup_scr_screen_AIPhotoResult2,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_AIPhotoResult_btn_english_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphotoresult3.scr, g_ui.screen_AIPhotoResult3_del,
                                  &g_ui.screen_AIPhotoResult_del, setup_scr_screen_AIPhotoResult3,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

static void screen_AIPhotoResult_btn_history_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphotoresult4.scr, g_ui.screen_AIPhotoResult4_del,
                                  &g_ui.screen_AIPhotoResult_del, setup_scr_screen_AIPhotoResult4,
                                  LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_AIPhotoResult(lv_ui_t *ui)
{
    // 添加事件处理
    lv_obj_add_event_cb(AIPhotoResult->btn_back, page_aiphotoresult_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(AIPhotoResult->btn_science, screen_AIPhotoResult_btn_science_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(AIPhotoResult->btn_chinese, screen_AIPhotoResult_btn_chinese_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(AIPhotoResult->btn_english, screen_AIPhotoResult_btn_english_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(AIPhotoResult->btn_history, screen_AIPhotoResult_btn_history_event_handler, LV_EVENT_CLICKED,
                        ui);
}

void setup_scr_screen_AIPhotoResult(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIPhotoResult...\n");

    AIPhotoResult      = &ui->page_aiphotoresult;
    AIPhotoResult->del = true;

    // 创建主页面容器
    if(AIPhotoResult->scr != NULL) {
        if(lv_obj_is_valid(AIPhotoResult->scr)) {
            MLOG_DBG("AIPhotoResult->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIPhotoResult->scr);
        } else {
            MLOG_DBG("AIPhotoResult->scr 已被自动销毁，仅重置指针\n");
        }
        AIPhotoResult->scr = NULL;
    }

    // Write codes screen_AIPhotoResult
    AIPhotoResult->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIPhotoResult->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIPhotoResult->scr, LV_SCROLLBAR_MODE_OFF);

    // 显示图片
    AIPhotoResult->img_main = lv_img_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->img_main, 0, 60);
    lv_obj_set_size(AIPhotoResult->img_main, 640, 360);
    if(AI_TMP_IMAGE_PATH) {
        // 检查图片文件是否存在
        const char *real_path = strchr(AI_TMP_IMAGE_PATH, '/');
        FILE *file            = fopen(real_path, "r");
        if(file != NULL) {
            fclose(file);
            MLOG_DBG("AI_TMP_IMAGE_PATH 文件存在: %s\n", AI_TMP_IMAGE_PATH);
            lv_img_set_src(AIPhotoResult->img_main, AI_TMP_IMAGE_PATH);
        } else {
            MLOG_DBG("AI_TMP_IMAGE_PATH 文件不存在: %s\n", AI_TMP_IMAGE_PATH);
        }
    } else {
        MLOG_DBG("AI_TMP_IMAGE_PATH is not exist\n");
    }

    // Write style for screen_AIPhotoResult, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    // lv_obj_set_style_bg_opa(AIPhotoResult->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(AIPhotoResult->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont (底部边栏)
    AIPhotoResult->bottom_cont = lv_obj_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->bottom_cont, 0, 420);
    lv_obj_set_size(AIPhotoResult->bottom_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIPhotoResult->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(AIPhotoResult->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AIPhotoResult->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(AIPhotoResult->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIPhotoResult->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhotoResult->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont (顶部边栏)
    AIPhotoResult->top_cont = lv_obj_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->top_cont, 0, 0);
    lv_obj_set_size(AIPhotoResult->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIPhotoResult->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIPhotoResult->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhotoResult->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (顶部返回按钮)
    AIPhotoResult->btn_back = lv_btn_create(AIPhotoResult->top_cont);
    lv_obj_set_pos(AIPhotoResult->btn_back, 0, 4);
    lv_obj_set_size(AIPhotoResult->btn_back, 60, 50);
    AIPhotoResult->label_back = lv_label_create(AIPhotoResult->btn_back);
    lv_label_set_text(AIPhotoResult->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIPhotoResult->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIPhotoResult->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIPhotoResult->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIPhotoResult->label_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIPhotoResult->label_back, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIPhotoResult->label_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIPhotoResult->label_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_science (科普按钮)
    AIPhotoResult->btn_science = lv_button_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->btn_science, 547, 75);
    lv_obj_set_size(AIPhotoResult->btn_science, 77, 56);
    AIPhotoResult->label_science = lv_label_create(AIPhotoResult->btn_science);
    lv_label_set_text(AIPhotoResult->label_science, "科普");
    lv_label_set_long_mode(AIPhotoResult->label_science, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult->label_science, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult->btn_science, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult->label_science, LV_PCT(100));
    // 按钮
    lv_obj_set_style_bg_color(AIPhotoResult->btn_science, lv_color_hex(0x42A5F5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->btn_science, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->btn_science, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    // label
    static lv_style_t style_screen_page_aiphotoresult_label_science;
    if(style_screen_page_aiphotoresult_label_science.prop_cnt > 1)
        lv_style_reset(&style_screen_page_aiphotoresult_label_science);
    else
        lv_style_init(&style_screen_page_aiphotoresult_label_science);
    lv_style_set_opa(&style_screen_page_aiphotoresult_label_science, 255);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 24, &style_screen_page_aiphotoresult_label_science);
    lv_style_set_text_color(&style_screen_page_aiphotoresult_label_science, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&style_screen_page_aiphotoresult_label_science, 255);
    lv_obj_add_style(AIPhotoResult->label_science, &style_screen_page_aiphotoresult_label_science,
                     LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_chinese (国学按钮)
    AIPhotoResult->btn_chinese = lv_button_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->btn_chinese, 547, 162);
    lv_obj_set_size(AIPhotoResult->btn_chinese, 77, 56);
    AIPhotoResult->label_chinese = lv_label_create(AIPhotoResult->btn_chinese);
    lv_label_set_text(AIPhotoResult->label_chinese, "国学");
    lv_label_set_long_mode(AIPhotoResult->label_chinese, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult->label_chinese, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult->btn_chinese, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult->label_chinese, LV_PCT(100));
    // 按钮
    lv_obj_set_style_bg_color(AIPhotoResult->btn_chinese, lv_color_hex(0x66BB6A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->btn_chinese, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->btn_chinese, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    // label
    static lv_style_t style_screen_page_aiphotoresult_label_chinese;
    if(style_screen_page_aiphotoresult_label_chinese.prop_cnt > 1)
        lv_style_reset(&style_screen_page_aiphotoresult_label_chinese);
    else
        lv_style_init(&style_screen_page_aiphotoresult_label_chinese);
    lv_style_set_opa(&style_screen_page_aiphotoresult_label_chinese, 255);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 24, &style_screen_page_aiphotoresult_label_chinese);
    lv_style_set_text_color(&style_screen_page_aiphotoresult_label_chinese, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&style_screen_page_aiphotoresult_label_chinese, 255);
    lv_obj_add_style(AIPhotoResult->label_chinese, &style_screen_page_aiphotoresult_label_chinese,
                     LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_english (英语按钮)
    AIPhotoResult->btn_english = lv_button_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->btn_english, 547, 249);
    lv_obj_set_size(AIPhotoResult->btn_english, 77, 56);
    AIPhotoResult->label_english = lv_label_create(AIPhotoResult->btn_english);
    lv_label_set_text(AIPhotoResult->label_english, "英语");
    lv_label_set_long_mode(AIPhotoResult->label_english, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult->label_english, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult->btn_english, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult->label_english, LV_PCT(100));
    // 按钮
    lv_obj_set_style_bg_color(AIPhotoResult->btn_english, lv_color_hex(0xFFA726), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->btn_english, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->btn_english, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    // label
    static lv_style_t style_screen_page_aiphotoresult_label_english;
    if(style_screen_page_aiphotoresult_label_english.prop_cnt > 1)
        lv_style_reset(&style_screen_page_aiphotoresult_label_english);
    else
        lv_style_init(&style_screen_page_aiphotoresult_label_english);
    lv_style_set_opa(&style_screen_page_aiphotoresult_label_english, 255);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 24, &style_screen_page_aiphotoresult_label_english);
    lv_style_set_text_color(&style_screen_page_aiphotoresult_label_english, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&style_screen_page_aiphotoresult_label_english, 255);
    lv_obj_add_style(AIPhotoResult->label_english, &style_screen_page_aiphotoresult_label_english,
                     LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_history (故事按钮)
    AIPhotoResult->btn_history = lv_button_create(AIPhotoResult->scr);
    lv_obj_set_pos(AIPhotoResult->btn_history, 547, 337);
    lv_obj_set_size(AIPhotoResult->btn_history, 77, 56);
    AIPhotoResult->label_history = lv_label_create(AIPhotoResult->btn_history);
    lv_label_set_text(AIPhotoResult->label_history, "故事");
    lv_label_set_long_mode(AIPhotoResult->label_history, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult->label_history, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult->btn_history, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult->label_history, LV_PCT(100));
    // 按钮
    lv_obj_set_style_bg_color(AIPhotoResult->btn_history, lv_color_hex(0xAB47BC), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->btn_history, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->btn_history, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    // label
    static lv_style_t style_screen_page_aiphotoresult_label_history;
    if(style_screen_page_aiphotoresult_label_history.prop_cnt > 1)
        lv_style_reset(&style_screen_page_aiphotoresult_label_history);
    else
        lv_style_init(&style_screen_page_aiphotoresult_label_history);
    lv_style_set_opa(&style_screen_page_aiphotoresult_label_history, 255);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 24, &style_screen_page_aiphotoresult_label_history);
    lv_style_set_text_color(&style_screen_page_aiphotoresult_label_history, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&style_screen_page_aiphotoresult_label_history, 255);
    lv_obj_add_style(AIPhotoResult->label_history, &style_screen_page_aiphotoresult_label_history,
                     LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes result_cont (AI识别结果容器)
    AIPhotoResult->result_cont = lv_obj_create(AIPhotoResult->scr);
    lv_obj_set_size(AIPhotoResult->result_cont, 300, 100);
    lv_obj_align(AIPhotoResult->result_cont, LV_ALIGN_TOP_MID, 0, 280);
    lv_obj_set_scrollbar_mode(AIPhotoResult->result_cont, LV_SCROLLBAR_MODE_OFF);
    // result_cont
    lv_obj_set_style_bg_color(AIPhotoResult->result_cont, lv_color_hex(0x2196F3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhotoResult->result_cont, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(AIPhotoResult->result_cont, lv_color_hex(0x21CBF3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AIPhotoResult->result_cont, lv_color_hex(0x2196F3), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIPhotoResult->result_cont, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult->result_cont, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult->result_cont, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(AIPhotoResult->result_cont, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(AIPhotoResult->result_cont, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhotoResult->result_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhotoResult->result_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhotoResult->result_cont, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhotoResult->result_cont, 15, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes result_label (AI识别结果文本标签)
    AIPhotoResult->result_label = lv_label_create(AIPhotoResult->result_cont);
    lv_label_set_text(AIPhotoResult->result_label, g_ai_photo_result_text);
    lv_label_set_long_mode(AIPhotoResult->result_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult->result_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(AIPhotoResult->result_label, 410);
    // result_label
    static lv_style_t style_screen_page_aiphotoresult_result_label;
    if(style_screen_page_aiphotoresult_result_label.prop_cnt > 1)
        lv_style_reset(&style_screen_page_aiphotoresult_result_label);
    else
        lv_style_init(&style_screen_page_aiphotoresult_result_label);
    lv_style_set_opa(&style_screen_page_aiphotoresult_result_label, 255);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 36, &style_screen_page_aiphotoresult_result_label);
    lv_style_set_text_color(&style_screen_page_aiphotoresult_result_label, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&style_screen_page_aiphotoresult_result_label, 255);
    lv_style_set_text_align(&style_screen_page_aiphotoresult_result_label, LV_TEXT_ALIGN_CENTER);
    lv_obj_add_style(AIPhotoResult->result_label, &style_screen_page_aiphotoresult_result_label,
                     LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(AIPhotoResult->scr);

    // 添加事件处理
    events_init_screen_AIPhotoResult(ui);
}

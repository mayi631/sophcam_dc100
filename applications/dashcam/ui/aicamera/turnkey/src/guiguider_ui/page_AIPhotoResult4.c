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

static AIPhotoResult4_t *AIPhotoResult4;

// 事件处理函数
static void page_aiphotoresult4_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aiphotoresult.scr, g_ui.screen_AIPhotoResult_del,
                                  &g_ui.screen_AIPhotoResult4_del, setup_scr_screen_AIPhotoResult, LV_SCR_LOAD_ANIM_NONE, 200,
                                  200, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_AIPhotoResult4(lv_ui_t *ui)
{
    // 添加事件处理
    lv_obj_add_event_cb(AIPhotoResult4->btn_back, page_aiphotoresult4_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_AIPhotoResult4(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIPhotoResult4...\n");

    AIPhotoResult4      = &ui->page_aiphotoresult4;
    AIPhotoResult4->del = true;

    // 创建主页面容器
    if(AIPhotoResult4->scr != NULL) {
        if(lv_obj_is_valid(AIPhotoResult4->scr)) {
            MLOG_DBG("AIPhotoResult4->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIPhotoResult4->scr);
        } else {
            MLOG_DBG("AIPhotoResult4->scr 已被自动销毁，仅重置指针\n");
        }
        AIPhotoResult4->scr = NULL;
    }

    // Write codes screen_AIPhotoResult4
    AIPhotoResult4->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIPhotoResult4->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIPhotoResult4->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_AIPhotoResult4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    // lv_obj_set_style_bg_opa(AIPhotoResult4->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(AIPhotoResult4->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont (底部边栏)
    AIPhotoResult4->bottom_cont = lv_obj_create(AIPhotoResult4->scr);
    lv_obj_set_pos(AIPhotoResult4->bottom_cont, 0, 420);
    lv_obj_set_size(AIPhotoResult4->bottom_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIPhotoResult4->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(AIPhotoResult4->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AIPhotoResult4->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(AIPhotoResult4->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIPhotoResult4->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult4->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhotoResult4->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult4->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont (顶部边栏)
    AIPhotoResult4->top_cont = lv_obj_create(AIPhotoResult4->scr);
    lv_obj_set_pos(AIPhotoResult4->top_cont, 0, 0);
    lv_obj_set_size(AIPhotoResult4->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIPhotoResult4->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIPhotoResult4->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult4->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhotoResult4->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult4->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (顶部返回按钮)
    AIPhotoResult4->btn_back = lv_btn_create(AIPhotoResult4->top_cont);
    lv_obj_set_pos(AIPhotoResult4->btn_back, 0, 4);
    lv_obj_set_size(AIPhotoResult4->btn_back, 60, 50);
    AIPhotoResult4->label_back = lv_label_create(AIPhotoResult4->btn_back);
    lv_label_set_text(AIPhotoResult4->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIPhotoResult4->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult4->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult4->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult4->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIPhotoResult4->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult4->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIPhotoResult4->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult4->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult4->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIPhotoResult4->label_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIPhotoResult4->label_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIPhotoResult4->label_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIPhotoResult4->label_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_func (故事按钮)
    AIPhotoResult4->btn_func = lv_button_create(AIPhotoResult4->scr);
    lv_obj_set_pos(AIPhotoResult4->btn_func, 514, 76);
    lv_obj_set_size(AIPhotoResult4->btn_func, 77, 56);
    AIPhotoResult4->label_func = lv_label_create(AIPhotoResult4->btn_func);
    lv_label_set_text(AIPhotoResult4->label_func, "故事");
    lv_label_set_long_mode(AIPhotoResult4->label_func, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhotoResult4->label_func, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhotoResult4->btn_func, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhotoResult4->label_func, LV_PCT(100));

    // Write style for btn_func, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIPhotoResult4->btn_func, 63, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhotoResult4->btn_func, lv_color_hex(0x7b797b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhotoResult4->btn_func, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIPhotoResult4->btn_func, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhotoResult4->btn_func, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhotoResult4->btn_func, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    font_setting(AIPhotoResult4->label_func, FONT_SC16,
                 (font_size_config_t){.mode = FONT_SIZE_MODE_CUSTOM, .size = FONT_SIZE_24}, 0xFFFFFF, 0, 0);
    lv_obj_align(AIPhotoResult4->label_func, LV_ALIGN_CENTER, 0, 0);

    // 显示图片
    AIPhotoResult4->img_main = lv_img_create(AIPhotoResult4->scr);
    lv_obj_set_pos(AIPhotoResult4->img_main, 0, 60);
    lv_obj_set_size(AIPhotoResult4->img_main, 640, 360);
    if(AI_TMP_IMAGE_PATH) {
        // 检查图片文件是否存在
        const char *real_path = strchr(AI_TMP_IMAGE_PATH, '/');
        FILE *file = fopen(real_path, "r");
        if(file != NULL) {
            fclose(file);
            MLOG_DBG("AI_TMP_IMAGE_PATH 文件存在: %s\n", AI_TMP_IMAGE_PATH);
            lv_img_set_src(AIPhotoResult4->img_main, AI_TMP_IMAGE_PATH);
        } else {
            MLOG_DBG("AI_TMP_IMAGE_PATH 文件不存在: %s\n", AI_TMP_IMAGE_PATH);
        }
    } else {
        MLOG_DBG("AI_TMP_IMAGE_PATH is not exist\n");
    }

    // Update current screen layout.
    lv_obj_update_layout(AIPhotoResult4->scr);

    // 添加事件处理
    events_init_screen_AIPhotoResult4(ui);
}

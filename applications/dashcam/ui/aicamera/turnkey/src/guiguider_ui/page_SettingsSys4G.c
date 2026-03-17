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
// 4G开关全局状态变量，0=关，1=开
static int g4_switch_state = 1;

// 4G开关事件回调
static void g4_switch_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    // 只保存开关状态
    if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        g4_switch_state = 1;
    } else {
        g4_switch_state = 0;
    }
}

// 返回按钮回调函数
static void g4_back_cb(lv_event_t *e)
{
    // lv_obj_t *btn = lv_event_get_target(e);
    lv_ui_t *ui   = lv_event_get_user_data(e);

    // 返回系统设置页面
    ui_load_scr_animation(ui, &ui->page_settingssys.scr, ui->screen_SettingsSys_del, &ui->screen_SettingsSys4G_del,
                          setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

void setup_scr_screen_SettingsSys4G(lv_ui_t *ui)
{

    MLOG_DBG("loading page_settingssys4g...\n");

    SettingsSys4G_t *SettingSys4G = &ui->page_sys4g;
    SettingSys4G->del             = true;

    // 创建主页面1 容器
    if(SettingSys4G->scr != NULL) {
        if(lv_obj_is_valid(SettingSys4G->scr)) {
            MLOG_DBG("page_settingssys4g->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SettingSys4G->scr);
        } else {
            MLOG_DBG("page_settingssys4g->scr 已被自动销毁，仅重置指针\n");
        }
        SettingSys4G->scr = NULL;
    }

    // The custom code of scr.
    SettingSys4G->scr = lv_obj_create(NULL);
    lv_obj_set_size(SettingSys4G->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SettingSys4G->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SettingSys4G->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys4G->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SettingSys4G->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(SettingSys4G->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮
    SettingSys4G->btn_back = lv_btn_create(SettingSys4G->scr);
    lv_obj_set_pos(SettingSys4G->btn_back, 16, 16);
    lv_obj_set_size(SettingSys4G->btn_back, 40, 40);
    lv_obj_set_style_radius(SettingSys4G->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SettingSys4G->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SettingSys4G->btn_back, g4_back_cb, LV_EVENT_CLICKED, ui);
    SettingSys4G->label_back = lv_label_create(SettingSys4G->btn_back);
    lv_label_set_text(SettingSys4G->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SettingSys4G->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys4G->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SettingSys4G->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"移动网络"
    SettingSys4G->title = lv_label_create(SettingSys4G->scr);
    lv_label_set_text(SettingSys4G->title, "移动网络");
    lv_obj_set_style_text_color(SettingSys4G->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SettingSys4G->title, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SettingSys4G->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SettingSys4G->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SettingSys4G->title, LV_ALIGN_TOP_MID, 0, 24);

    // 顶部"移动网络"+开关栏，白色背景
    lv_obj_t *g4_topbar = lv_obj_create(SettingSys4G->scr);
    lv_obj_set_size(g4_topbar, 400, 56);
    lv_obj_align(g4_topbar, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_radius(g4_topbar, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(g4_topbar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(g4_topbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(g4_topbar, LV_SCROLLBAR_MODE_OFF);
    // "移动网络"文字
    lv_obj_t *g4_label = lv_label_create(g4_topbar);
    lv_label_set_text(g4_label, "移动网络");
    lv_obj_set_style_text_color(g4_label, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g4_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(g4_label, LV_ALIGN_LEFT_MID, 16, 0);
    // 4G开关
    lv_obj_t *g4_switch = lv_switch_create(g4_topbar);
    lv_obj_set_size(g4_switch, 60, 32);
    lv_obj_align(g4_switch, LV_ALIGN_RIGHT_MID, -16, 0);
    lv_obj_set_style_bg_color(g4_switch, lv_color_hex(0xFFC107), LV_PART_INDICATOR | LV_STATE_CHECKED);
    if(g4_switch_state) {
        lv_obj_add_state(g4_switch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g4_switch, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(g4_switch, g4_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Update current screen layout.
    lv_obj_update_layout(SettingSys4G->scr);
}

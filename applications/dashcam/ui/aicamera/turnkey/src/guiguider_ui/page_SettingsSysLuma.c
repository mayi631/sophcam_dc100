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
// 亮度全局变量
static int luma_value = 50; // 0~100
// 亮度数值label指针（全局，便于事件中访问）
static lv_obj_t *luma_value_label = NULL;

// 亮度加减按钮事件
// static void sysluma_btn_event_cb(lv_event_t *e)
// {
//     lv_obj_t *btn   = lv_event_get_target(e);
//     lv_ui_t *ui     = lv_event_get_user_data(e);
//     const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));
//     int value       = lv_arc_get_value(ui->page_sysluma.arc);
//     if(strcmp(txt, "+") == 0) {
//         if(value < 100) value += 10;
//     } else if(strcmp(txt, "-") == 0) {
//         if(value > 0) value -= 10;
//     }
//     luma_value = value;
//     lv_arc_set_value(ui->page_sysluma.arc, value);
//     if(luma_value_label) lv_label_set_text_fmt(luma_value_label, "%d", value);
// }

// arc拖动事件回调
static void sysluma_arc_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target(e);
    int32_t value = lv_arc_get_value(arc);
    luma_value    = value;
    if(luma_value_label) lv_label_set_text_fmt(luma_value_label, "%d", value);
}

// 返回按钮事件
static void sysluma_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    ui_load_scr_animation(ui, &ui->page_settingssys.scr, ui->screen_SettingsSys_del, &ui->screen_SettingsSysLuma_del,
                          setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

void setup_scr_screen_SettingsSysLuma(lv_ui_t *ui)
{

    MLOG_DBG("loading page_SysLuma...\n");

    SettingsSysLuma_t *SysLuma = &ui->page_sysluma;
    SysLuma->del               = true;

    // 创建主页面1 容器
    if(SysLuma->scr != NULL) {
        if(lv_obj_is_valid(SysLuma->scr)) {
            MLOG_DBG("page_SysLuma->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SysLuma->scr);
        } else {
            MLOG_DBG("page_SysLuma->scr 已被自动销毁，仅重置指针\n");
        }
        SysLuma->scr = NULL;
    }

    // The custom code of scr.
    SysLuma->scr = lv_obj_create(NULL);
    lv_obj_set_size(SysLuma->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SysLuma->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SysLuma->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysLuma->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SysLuma->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(SysLuma->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮（黄色背景）
    SysLuma->btn_back = lv_btn_create(SysLuma->scr);
    lv_obj_set_pos(SysLuma->btn_back, 16, 16);
    lv_obj_set_size(SysLuma->btn_back, 40, 40);
    lv_obj_set_style_radius(SysLuma->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysLuma->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SysLuma->btn_back, sysluma_back_cb, LV_EVENT_CLICKED, ui);
    SysLuma->label_back = lv_label_create(SysLuma->btn_back);
    lv_label_set_text(SysLuma->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SysLuma->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysLuma->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysLuma->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"亮度"
    SysLuma->title = lv_label_create(SysLuma->scr);
    lv_label_set_text(SysLuma->title, "亮度");
    lv_obj_set_style_text_color(SysLuma->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysLuma->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SysLuma->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SysLuma->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysLuma->title, LV_ALIGN_TOP_MID, 0, 24);

    // 圆形亮度进度条
    SysLuma->arc = lv_arc_create(SysLuma->scr);
    lv_obj_set_size(SysLuma->arc, 220, 220);
    lv_obj_align(SysLuma->arc, LV_ALIGN_CENTER, 0, 20);
    lv_arc_set_range(SysLuma->arc, 0, 100);
    lv_arc_set_value(SysLuma->arc, luma_value);
    lv_arc_set_mode(SysLuma->arc, LV_ARC_MODE_NORMAL);
    lv_obj_set_style_arc_color(SysLuma->arc, lv_color_hex(0xFFD600), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(SysLuma->arc, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(SysLuma->arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(SysLuma->arc, LV_OPA_TRANSP, LV_PART_KNOB);
    lv_obj_add_flag(SysLuma->arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(SysLuma->arc, sysluma_arc_event_cb, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_set_style_pad_all(SysLuma->arc, 16, LV_PART_KNOB);

    // 中间太阳图标
    SysLuma->img_sun = lv_image_create(SysLuma->scr);
    lv_image_set_src(SysLuma->img_sun, &_sun_RGB565A8_100x100);
    lv_obj_align_to(SysLuma->img_sun, SysLuma->arc, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_image_opa(SysLuma->img_sun, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 圆心下方亮度数值label
    luma_value_label = lv_label_create(SysLuma->scr);
    lv_label_set_text_fmt(luma_value_label, "%d", luma_value);
    lv_obj_set_style_text_color(luma_value_label, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(luma_value_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(luma_value_label, SysLuma->arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
#if 0
// 左侧"-"按钮
// ui->screen_SettingsSysLuma_minus_btn = lv_btn_create(ui->screen_SettingsSysLuma);
    lv_obj_set_size(ui->screen_SettingsSysLuma_minus_btn, 56, 56);
    lv_obj_align(ui->screen_SettingsSysLuma_minus_btn, LV_ALIGN_LEFT_MID, 40, 20);
    lv_obj_set_style_radius(ui->screen_SettingsSysLuma_minus_btn, 28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_SettingsSysLuma_minus_btn, lv_color_hex(0x44415A), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui->screen_SettingsSysLuma_minus_btn, sysluma_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *minus_label = lv_label_create(ui->screen_SettingsSysLuma_minus_btn);
    lv_label_set_text(minus_label, "-");
    lv_obj_set_style_text_color(minus_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(minus_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(minus_label, LV_ALIGN_CENTER, 0, 0);

// 右侧"+"按钮
// ui->screen_SettingsSysLuma_plus_btn = lv_btn_create(ui->screen_SettingsSysLuma);
    lv_obj_set_size(ui->screen_SettingsSysLuma_plus_btn, 56, 56);
    lv_obj_align(ui->screen_SettingsSysLuma_plus_btn, LV_ALIGN_RIGHT_MID, -40, 20);
    lv_obj_set_style_radius(ui->screen_SettingsSysLuma_plus_btn, 28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_SettingsSysLuma_plus_btn, lv_color_hex(0x44415A), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui->screen_SettingsSysLuma_plus_btn, sysluma_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *plus_label = lv_label_create(ui->screen_SettingsSysLuma_plus_btn);
    lv_label_set_text(plus_label, "+");
    lv_obj_set_style_text_color(plus_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(plus_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(plus_label, LV_ALIGN_CENTER, 0, 0);
#endif

    // Update current screen layout.
    lv_obj_update_layout(SysLuma->scr);
}

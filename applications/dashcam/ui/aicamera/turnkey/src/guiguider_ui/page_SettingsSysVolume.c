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
// 音量全局变量
extern int volume_level;
// static int volume_value = 50; // 0~100
// 音量数值label指针（全局，便于事件中访问）
static lv_obj_t *volume_value_label = NULL;
// 音量加减按钮事件
// static void sysvolume_btn_event_cb(lv_event_t *e)
// {
//     lv_obj_t *btn   = lv_event_get_target(e);
//     lv_ui_t *ui     = lv_event_get_user_data(e);
//     const char *txt = lv_label_get_text(lv_obj_get_child(btn, 0));
//     int value       = lv_arc_get_value(ui->page_sysvolume.scr);
//     if(strcmp(txt, "+") == 0) {
//         if(value < 100) value += 10;
//     } else if(strcmp(txt, "-") == 0) {
//         if(value > 0) value -= 10;
//     }
//     // volume_value = value;
//     volume_level = value;
//     lv_arc_set_value(ui->page_sysvolume.arc, value);
//     if(volume_value_label) lv_label_set_text_fmt(volume_value_label, "%d", value);
// }

// arc拖动事件回调
static void sysvolume_arc_event_cb(lv_event_t *e)
{
    lv_obj_t *arc = lv_event_get_target(e);
    int32_t value = lv_arc_get_value(arc);
    // volume_value = value;
    volume_level = value;
    if(volume_value_label) lv_label_set_text_fmt(volume_value_label, "%d", value);
}

// 返回按钮事件
static void sysvolume_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    ui_load_scr_animation(ui, &ui->page_settingssys.scr, ui->screen_SettingsSys_del, &ui->screen_SettingsSysVolume_del,
                          setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

void setup_scr_screen_SettingsSysVolume(lv_ui_t *ui)
{

    MLOG_DBG("loading page_sysvolume...\n");

    SettingsSysVolume_t *SysVolume = &ui->page_sysvolume;
    SysVolume->del                 = true;

    // 创建主页面1 容器
    if(SysVolume->scr != NULL) {
        if(lv_obj_is_valid(SysVolume->scr)) {
            MLOG_DBG("page_sysvolume->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SysVolume->scr);
        } else {
            MLOG_DBG("page_sysvolume->scr 已被自动销毁，仅重置指针\n");
        }
        SysVolume->scr = NULL;
    }

    // The custom code of scr.
    SysVolume->scr = lv_obj_create(NULL);
    lv_obj_set_size(SysVolume->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SysVolume->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SysVolume->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysVolume->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SysVolume->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(SysVolume->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮（黄色背景）
    SysVolume->btn_back = lv_btn_create(SysVolume->scr);
    lv_obj_set_pos(SysVolume->btn_back, 16, 16);
    lv_obj_set_size(SysVolume->btn_back, 40, 40);
    lv_obj_set_style_radius(SysVolume->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysVolume->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SysVolume->btn_back, sysvolume_back_cb, LV_EVENT_CLICKED, ui);
    SysVolume->label_back = lv_label_create(SysVolume->btn_back);
    lv_label_set_text(SysVolume->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SysVolume->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysVolume->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysVolume->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"音量"
    SysVolume->title = lv_label_create(SysVolume->scr);
    lv_label_set_text(SysVolume->title, "音量");
    lv_obj_set_style_text_color(SysVolume->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysVolume->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SysVolume->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SysVolume->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysVolume->title, LV_ALIGN_TOP_MID, 0, 24);

    // 圆形音量进度条
    SysVolume->arc = lv_arc_create(SysVolume->scr);
    lv_obj_set_size(SysVolume->arc, 220, 220);
    lv_obj_align(SysVolume->arc, LV_ALIGN_CENTER, 0, 20);
    lv_arc_set_range(SysVolume->arc, 0, 100);
    // lv_arc_set_value(SysVolume->arc, volume_value);
    lv_arc_set_value(SysVolume->arc, volume_level);
    // lv_arc_set_bg_angles(SysVolume->arc, 0, 360);
    // lv_arc_set_rotation(SysVolume->arc, 135);
    lv_arc_set_mode(SysVolume->arc, LV_ARC_MODE_NORMAL);
    // lv_arc_set_angles(SysVolume->arc, 135, 135 + (270 * volume_value / 100));
    lv_obj_set_style_arc_color(SysVolume->arc, lv_color_hex(0xFFD600), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(SysVolume->arc, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(SysVolume->arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(SysVolume->arc, LV_OPA_TRANSP, LV_PART_KNOB);
    // lv_obj_clear_flag(SysVolume->arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(SysVolume->arc, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(SysVolume->arc, sysvolume_arc_event_cb, LV_EVENT_VALUE_CHANGED, ui);
    lv_obj_set_style_pad_all(SysVolume->arc, 16, LV_PART_KNOB);

    // 中间音符图标
    SysVolume->label_icon = lv_label_create(SysVolume->scr);
    lv_label_set_text(SysVolume->label_icon, LV_SYMBOL_AUDIO);
    lv_obj_set_style_text_color(SysVolume->label_icon, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysVolume->label_icon, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(SysVolume->label_icon, SysVolume->arc, LV_ALIGN_CENTER, 0, 0);

    // 圆心音量数值label
    volume_value_label = lv_label_create(SysVolume->scr);
    // lv_label_set_text_fmt(volume_value_label, "%d", volume_value);
    lv_label_set_text_fmt(volume_value_label, "%d", volume_level);
    lv_obj_set_style_text_color(volume_value_label, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(volume_value_label, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(volume_value_label, SysVolume->arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

#if 0
// 左侧"-"按钮
// ui->screen_SettingsSysVolume_minus_btn = lv_btn_create(ui->screen_SettingsSysVolume);
    lv_obj_set_size(ui->screen_SettingsSysVolume_minus_btn, 56, 56);
    lv_obj_align(ui->screen_SettingsSysVolume_minus_btn, LV_ALIGN_LEFT_MID, 40, 20);
    lv_obj_set_style_radius(ui->screen_SettingsSysVolume_minus_btn, 28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_SettingsSysVolume_minus_btn, lv_color_hex(0x44415A), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui->screen_SettingsSysVolume_minus_btn, sysvolume_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *minus_label = lv_label_create(ui->screen_SettingsSysVolume_minus_btn);
    lv_label_set_text(minus_label, "-");
    lv_obj_set_style_text_color(minus_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(minus_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(minus_label, LV_ALIGN_CENTER, 0, 0);

// 右侧"+"按钮
// ui->screen_SettingsSysVolume_plus_btn = lv_btn_create(ui->screen_SettingsSysVolume);
    lv_obj_set_size(ui->screen_SettingsSysVolume_plus_btn, 56, 56);
    lv_obj_align(ui->screen_SettingsSysVolume_plus_btn, LV_ALIGN_RIGHT_MID, -40, 20);
    lv_obj_set_style_radius(ui->screen_SettingsSysVolume_plus_btn, 28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_SettingsSysVolume_plus_btn, lv_color_hex(0x44415A), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_add_event_cb(ui->screen_SettingsSysVolume_plus_btn, sysvolume_btn_event_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *plus_label = lv_label_create(ui->screen_SettingsSysVolume_plus_btn);
    lv_label_set_text(plus_label, "+");
    lv_obj_set_style_text_color(plus_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(plus_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_align(plus_label, LV_ALIGN_CENTER, 0, 0);
#endif

    // Update current screen layout.
    lv_obj_update_layout(SysVolume->scr);
}

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
#include <inttypes.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"
#include "config.h"
// 当前选中的亮屏时间索引 (0: 5分钟, 1: 10分钟, ..., 4: 永久)
static int selected_bl_time_index = 4; // 默认选中永久

// 亮屏时间选项标签指针数组
static lv_obj_t *bl_time_btns[5]; // 存储按钮指针

// 亮屏时间选项文本数组 (用于恢复未选中状态的文本)
const char *bl_time_options_text[] = {"5分钟", "10分钟", "15分钟", "20分钟", "永久"};

// 亮屏时间选项选中标记
static void update_bl_time_selection(int selected_index)
{
    for(int i = 0; i < 5; ++i) {
        if(bl_time_btns[i]) {
            if(i == selected_index) {
                // 选中状态：黄色文本
                lv_obj_set_style_text_color(bl_time_btns[i], lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
            } else {
                // 未选中状态：黑色文本
                lv_obj_set_style_text_color(bl_time_btns[i], lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }
    selected_bl_time_index = selected_index;
    // TODO: Add logic to actually set the screen brightness time
}

// 亮屏时间选项点击事件回调
static void bl_time_option_event_cb(lv_event_t *e)
{
    // lv_obj_t *btn = lv_event_get_target(e);
    int index     = (int)lv_event_get_user_data(e);
    update_bl_time_selection(index);
}

// 返回按钮事件
static void sysbl_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    ui_load_scr_animation(ui, &ui->page_settingssys.scr, ui->screen_SettingsSys_del, &ui->screen_SettingsSysBl_del,
                          setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

void setup_scr_screen_SettingsSysBl(lv_ui_t *ui)
{

    MLOG_DBG("loading page_sysbltim...\n");

    SettingsSysBlTime_t *SysBlTim = &ui->page_sysbltim;
    SysBlTim->del                 = true;

    // 创建主页面1 容器
    if(SysBlTim->scr != NULL) {
        if(lv_obj_is_valid(SysBlTim->scr)) {
            MLOG_DBG("page_sysbltim->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SysBlTim->scr);
        } else {
            MLOG_DBG("page_sysbltim->scr 已被自动销毁，仅重置指针\n");
        }
        SysBlTim->scr = NULL;
    }

    // The custom code of scr.
    SysBlTim->scr = lv_obj_create(NULL);
    lv_obj_set_size(SysBlTim->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SysBlTim->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SysBlTim->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysBlTim->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SysBlTim->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(SysBlTim->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮（黄色背景）
    SysBlTim->btn_back = lv_btn_create(SysBlTim->scr);
    lv_obj_set_pos(SysBlTim->btn_back, 16, 16);
    lv_obj_set_size(SysBlTim->btn_back, 40, 40);
    lv_obj_set_style_radius(SysBlTim->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysBlTim->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SysBlTim->btn_back, sysbl_back_cb, LV_EVENT_CLICKED, ui);
    SysBlTim->label_back = lv_label_create(SysBlTim->btn_back);
    lv_label_set_text(SysBlTim->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SysBlTim->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysBlTim->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysBlTim->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"亮屏时间"
    SysBlTim->title = lv_label_create(SysBlTim->scr);
    lv_label_set_text(SysBlTim->title, "亮屏时间");
    lv_obj_set_style_text_color(SysBlTim->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysBlTim->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SysBlTim->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SysBlTim->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysBlTim->title, LV_ALIGN_TOP_MID, 0, 24);

    // 选项列表容器
    SysBlTim->list = lv_list_create(SysBlTim->scr);
    lv_obj_set_size(SysBlTim->list, 480, 220);
    lv_obj_align(SysBlTim->list, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_set_style_bg_color(SysBlTim->list, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(SysBlTim->list, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(SysBlTim->list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(SysBlTim->list, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysBlTim->list, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(SysBlTim->list, LV_SCROLLBAR_MODE_AUTO);

    const char *options[] = {"5分钟", "10分钟", "15分钟", "20分钟", "永久"};

    for(int i = 0; i < 5; ++i) {
        bl_time_btns[i] = lv_list_add_btn(SysBlTim->list, NULL, options[i]);
        lv_obj_set_style_text_font(bl_time_btns[i], &lv_font_SourceHanSerifSC_Regular_16,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(bl_time_btns[i], bl_time_option_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
    }

    // 初始更新选中状态
    update_bl_time_selection(selected_bl_time_index);

    // Update current screen layout.
    lv_obj_update_layout(SysBlTim->scr);
}

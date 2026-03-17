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
// 返回按钮回调函数
static void info_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // TODO: Add logic to return to the previous screen (e.g., About page)
    ui_load_scr_animation(ui, &ui->page_sysabout.scr, ui->screen_SettingsSysAbout_del, &ui->screen_SettingsSysInfo_del,
                          setup_scr_screen_SettingsSysAbout, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

void setup_scr_screen_SettingsSysInfo(lv_ui_t *ui)
{

    MLOG_DBG("loading page_sysinfo...\n");

    SettingsSysInfo_t *SysInfo = &ui->page_sysinfo;
    SysInfo->del               = true;

    // 创建主页面1 容器
    if(SysInfo->scr != NULL) {
        if(lv_obj_is_valid(SysInfo->scr)) {
            MLOG_DBG("page_sysinfo->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SysInfo->scr);
        } else {
            MLOG_DBG("page_sysinfo->scr 已被自动销毁，仅重置指针\n");
        }
        SysInfo->scr = NULL;
    }

    // The custom code of scr.
    //  Create screen
    SysInfo->scr = lv_obj_create(NULL);
    lv_obj_set_size(SysInfo->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SysInfo->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SysInfo->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysInfo->scr, lv_color_hex(0x181818),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // Dark background

    // 顶部返回按钮（黄色背景）
    SysInfo->btn_back = lv_btn_create(SysInfo->scr);
    lv_obj_set_pos(SysInfo->btn_back, 16, 16);
    lv_obj_set_size(SysInfo->btn_back, 40, 40);
    lv_obj_set_style_radius(SysInfo->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysInfo->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SysInfo->btn_back, info_back_cb, LV_EVENT_CLICKED, ui);
    // 返回按钮图标
    SysInfo->label_back = lv_label_create(SysInfo->btn_back);
    lv_label_set_text(SysInfo->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SysInfo->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysInfo->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysInfo->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"本机信息"
    SysInfo->title = lv_label_create(SysInfo->scr);
    lv_label_set_text(SysInfo->title, "本机信息");
    lv_obj_set_style_text_color(SysInfo->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysInfo->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SysInfo->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SysInfo->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysInfo->title, LV_ALIGN_TOP_MID, 0, 24);

    // TODO: Add specific content for the local info page here

    // Update current screen layout.
    lv_obj_update_layout(SysInfo->scr);
}

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
static void service_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // TODO: Add logic to return to the previous screen (e.g., About page)
    ui_load_scr_animation(ui, &ui->page_sysabout.scr, ui->screen_SettingsSysAbout_del,
                          &ui->screen_SettingsSysService_del, setup_scr_screen_SettingsSysAbout, LV_SCR_LOAD_ANIM_NONE,
                          200, 200, false, true);
}

void setup_scr_screen_SettingsSysService(lv_ui_t *ui)
{

    MLOG_DBG("loading page_sysservice...\n");

    SettingsSysService_t *SysService = &ui->page_sysservice;
    SysService->del                  = true;

    // 创建主页面1 容器
    if(SysService->scr != NULL) {
        if(lv_obj_is_valid(SysService->scr)) {
            MLOG_DBG("page_sysservice->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SysService->scr);
        } else {
            MLOG_DBG("page_sysservice->scr 已被自动销毁，仅重置指针\n");
        }
        SysService->scr = NULL;
    }

    // The custom code of scr.
    //  Create screen
    SysService->scr = lv_obj_create(NULL);
    lv_obj_set_size(SysService->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SysService->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SysService->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysService->scr, lv_color_hex(0x181818),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // Dark background

    // 顶部返回按钮（黄色背景）
    SysService->btn_back = lv_btn_create(SysService->scr);
    lv_obj_set_pos(SysService->btn_back, 16, 16);
    lv_obj_set_size(SysService->btn_back, 40, 40);
    lv_obj_set_style_radius(SysService->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysService->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SysService->btn_back, service_back_cb, LV_EVENT_CLICKED, ui);
    // 返回按钮图标
    SysService->label_back = lv_label_create(SysService->btn_back);
    lv_label_set_text(SysService->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SysService->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysService->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysService->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"服务协议"
    SysService->title = lv_label_create(SysService->scr);
    lv_label_set_text(SysService->title, "服务协议");
    lv_obj_set_style_text_color(SysService->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysService->title, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SysService->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SysService->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysService->title, LV_ALIGN_TOP_MID, 0, 24);

    // TODO: Add specific content for the service agreement page here

    // Update current screen layout.
    lv_obj_update_layout(SysService->scr);
}

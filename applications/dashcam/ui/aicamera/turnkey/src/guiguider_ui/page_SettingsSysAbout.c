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
// 返回按钮回调函数
static void about_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // 返回系统设置页面
    ui_load_scr_animation(ui, &ui->page_settingssys.scr, ui->screen_SettingsSys_del, &ui->screen_SettingsSysAbout_del,
                          setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

// 按钮事件回调
static void about_button_cb(lv_event_t *e)
{
    lv_ui_t *ui   = lv_event_get_user_data(e);
    lv_obj_t *btn = lv_event_get_target(e);

    // 根据点击的按钮跳转到对应的页面
    if(btn == ui->page_sysabout.btn_update) {
        ui_load_scr_animation(ui, &ui->page_sysupdate.scr, ui->screen_SettingsSysUpdate_del,
                              &ui->screen_SettingsSysAbout_del, setup_scr_screen_SettingsSysUpdate,
                              LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
    } else if(btn == ui->page_sysabout.btn_info) {
        ui_load_scr_animation(ui, &ui->page_sysinfo.scr, ui->screen_SettingsSysInfo_del,
                              &ui->screen_SettingsSysAbout_del, setup_scr_screen_SettingsSysInfo, LV_SCR_LOAD_ANIM_NONE,
                              200, 200, false, true);
    } else if(btn == ui->page_sysabout.btn_log) {
        ui_load_scr_animation(ui, &ui->page_syslog.scr, ui->screen_SettingsSysLog_del, &ui->screen_SettingsSysAbout_del,
                              setup_scr_screen_SettingsSysLog, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
    } else if(btn == ui->page_sysabout.btn_service) {
        ui_load_scr_animation(ui, &ui->page_sysservice.scr, ui->screen_SettingsSysService_del,
                              &ui->screen_SettingsSysAbout_del, setup_scr_screen_SettingsSysService,
                              LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
    } else if(btn == ui->page_sysabout.btn_privacy) {
        ui_load_scr_animation(ui, &ui->page_sysprivacy.scr, ui->screen_SettingsSysPrivacy_del,
                              &ui->screen_SettingsSysAbout_del, setup_scr_screen_SettingsSysPrivacy,
                              LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
    }
}

void setup_scr_screen_SettingsSysAbout(lv_ui_t *ui)
{
    MLOG_DBG("loading page_sysabout...\n");

    SettingsSysAbout_t *SysAbout = &ui->page_sysabout;
    SysAbout->del                = true;

    // 创建主页面1 容器
    if(SysAbout->scr != NULL) {
        if(lv_obj_is_valid(SysAbout->scr)) {
            MLOG_DBG("page_sysabout->scr 仍然有效，删除旧对象\n");
            lv_obj_del(SysAbout->scr);
        } else {
            MLOG_DBG("page_sysabout->scr 已被自动销毁，仅重置指针\n");
        }
        SysAbout->scr = NULL;
    }

    // The custom code of scr.
    //  Create screen
    SysAbout->scr = lv_obj_create(NULL);
    lv_obj_set_size(SysAbout->scr, 640, 480);
    lv_obj_set_scrollbar_mode(SysAbout->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(SysAbout->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysAbout->scr, lv_color_hex(0x181818),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // Dark background

    // 顶部返回按钮（黄色背景）
    SysAbout->btn_back = lv_btn_create(SysAbout->scr);
    lv_obj_set_pos(SysAbout->btn_back, 16, 16);
    lv_obj_set_size(SysAbout->btn_back, 40, 40);
    lv_obj_set_style_radius(SysAbout->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(SysAbout->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(SysAbout->btn_back, about_back_cb, LV_EVENT_CLICKED, ui);
    // 返回按钮图标
    SysAbout->label_back = lv_label_create(SysAbout->btn_back);
    lv_label_set_text(SysAbout->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(SysAbout->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysAbout->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysAbout->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题"关于"
    SysAbout->title = lv_label_create(SysAbout->scr);
    lv_label_set_text(SysAbout->title, "关于");
    lv_obj_set_style_text_color(SysAbout->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(SysAbout->title, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(SysAbout->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(SysAbout->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(SysAbout->title, LV_ALIGN_TOP_MID, 0, 24);

    // 按钮容器 (使用 Flexbox 布局)
    lv_obj_t *btn_container = lv_obj_create(SysAbout->scr);
    // Adjust container width to fit 3 buttons with spacing
    int container_width      = 640 - 2 * 16; // Screen width minus left/right padding
    int button_margin        = 20;           // Desired margin between buttons
    int num_cols             = 3;
    int calculated_btn_width = (container_width - (num_cols - 1) * button_margin) / num_cols;
    int btn_height           = 142; // Keep similar height to system settings buttons

    lv_obj_set_size(btn_container, container_width, 350);       // Adjust container height as needed
    lv_obj_align(btn_container, LV_ALIGN_TOP_MID, 0, 80);       // Position below title area
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW_WRAP); // Horizontal flow with wrapping
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START);                                 // Align to start
    lv_obj_set_style_bg_opa(btn_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // Transparent background
    lv_obj_set_style_border_width(btn_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(btn_container, 10, LV_PART_MAIN | LV_STATE_DEFAULT); // Vertical spacing between rows
    lv_obj_set_style_pad_column(btn_container, button_margin,
                                LV_PART_MAIN | LV_STATE_DEFAULT);                // Horizontal spacing between columns
    lv_obj_set_style_pad_all(btn_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // Remove container padding
    lv_obj_set_style_layout(btn_container, LV_LAYOUT_FLEX, 0);                   // Ensure flex layout is set

    const char *button_labels[] = {"系统更新", "本机信息", "日志上传", "服务协议", "隐私政策"};
    lv_obj_t *btns[5];

    // 创建按钮
    btns[0] = lv_btn_create(btn_container);
    lv_obj_set_size(btns[0], calculated_btn_width, btn_height);
    lv_obj_set_style_radius(btns[0], 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btns[0], lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT); // Darker background
    lv_obj_set_style_border_width(btns[0], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btns[0], about_button_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *label_update = lv_label_create(btns[0]);
    lv_label_set_text(label_update, button_labels[0]);
    lv_obj_set_style_text_color(label_update, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT); // White text
    lv_obj_set_style_text_font(label_update, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_update, LV_ALIGN_CENTER, 0, 0);

    btns[1] = lv_btn_create(btn_container);
    lv_obj_set_size(btns[1], calculated_btn_width, btn_height);
    lv_obj_set_style_radius(btns[1], 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btns[1], lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btns[1], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btns[1], about_button_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *label_info = lv_label_create(btns[1]);
    lv_label_set_text(label_info, button_labels[1]);
    lv_obj_set_style_text_color(label_info, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_info, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_info, LV_ALIGN_CENTER, 0, 0);

    btns[2] = lv_btn_create(btn_container);
    lv_obj_set_size(btns[2], calculated_btn_width, btn_height);
    lv_obj_set_style_radius(btns[2], 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btns[2], lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btns[2], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btns[2], about_button_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *label_log = lv_label_create(btns[2]);
    lv_label_set_text(label_log, button_labels[2]);
    lv_obj_set_style_text_color(label_log, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_log, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_log, LV_ALIGN_CENTER, 0, 0);

    btns[3] = lv_btn_create(btn_container);
    lv_obj_set_size(btns[3], calculated_btn_width, btn_height);
    lv_obj_set_style_radius(btns[3], 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btns[3], lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btns[3], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btns[3], about_button_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *label_service = lv_label_create(btns[3]);
    lv_label_set_text(label_service, button_labels[3]);
    lv_obj_set_style_text_color(label_service, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_service, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_service, LV_ALIGN_CENTER, 0, 0);

    btns[4] = lv_btn_create(btn_container);
    lv_obj_set_size(btns[4], calculated_btn_width, btn_height);
    lv_obj_set_style_radius(btns[4], 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btns[4], lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btns[4], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btns[4], about_button_cb, LV_EVENT_CLICKED, ui);
    lv_obj_t *label_privacy = lv_label_create(btns[4]);
    lv_label_set_text(label_privacy, button_labels[4]);
    lv_obj_set_style_text_color(label_privacy, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_privacy, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_privacy, LV_ALIGN_CENTER, 0, 0);

    // Store button pointers for later reference in callbacks
    SysAbout->btn_update  = btns[0];
    SysAbout->btn_info    = btns[1];
    SysAbout->btn_log     = btns[2];
    SysAbout->btn_service = btns[3];
    SysAbout->btn_privacy = btns[4];

    // Update current screen layout.
    lv_obj_update_layout(SysAbout->scr);
}

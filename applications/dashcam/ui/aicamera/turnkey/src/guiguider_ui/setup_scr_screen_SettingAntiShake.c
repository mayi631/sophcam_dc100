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
extern char g_button_labelAnti[32];

static void screen_SettingAntiShake_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);
            lv_obj_t *cont        = lv_event_get_user_data(e); // 获取容器对象
            if(!cont) return;                                  // 如果容器为空则返回

            lv_obj_t *child;
            for(child = lv_obj_get_child(cont, 0); child != NULL;
                child = lv_obj_get_child(cont, lv_obj_get_index(child) + 1)) {
                if(lv_obj_check_type(child, &lv_button_class)) {
                    if(child == btn_clicked) {
                        // 获取按钮标签文本
                        lv_obj_t *label = lv_obj_get_child(child, 0);
                        if(label && lv_obj_check_type(label, &lv_label_class)) {
                            const char *txt = lv_label_get_text(label);
                            if(txt) strncpy(g_button_labelAnti, txt, sizeof(g_button_labelAnti) - 1);
                        }
                        lv_obj_add_state(child, LV_STATE_PRESSED);
                        lv_obj_set_style_border_color(child, lv_color_hex(0xFF0000), LV_PART_MAIN);
                    } else {
                        lv_obj_clear_state(child, LV_STATE_PRESSED);
                        lv_obj_set_style_border_color(child, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                    }
                }
            }
            break;
        }
        default: break;
    }
}

void setup_scr_screen_SettingAntiShake(lv_ui_t *ui)
{
    // Write codes screen_SettingAntiShake
    ui->screen_SettingAntiShake = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_SettingAntiShake, 640, 480);
    lv_obj_set_scrollbar_mode(ui->screen_SettingAntiShake, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(ui->screen_SettingAntiShake, 0, 0);

    // Write style for screen_SettingAntiShake, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_SettingAntiShake, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui->screen_SettingAntiShake, LV_OPA_0, LV_PART_MAIN);

    // Write codes screen_SettingAntiShake_cont_top (顶部栏)
    ui->screen_SettingAntiShake_cont_top = lv_obj_create(ui->screen_SettingAntiShake);
    lv_obj_set_pos(ui->screen_SettingAntiShake_cont_top, 0, 0);
    lv_obj_set_size(ui->screen_SettingAntiShake_cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(ui->screen_SettingAntiShake_cont_top, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_SettingAntiShake_cont_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_SettingAntiShake_cont_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_SettingAntiShake_cont_top, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_SettingAntiShake_cont_top, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_SettingAntiShake_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_SettingAntiShake_btn_back (返回按钮)
    ui->screen_SettingAntiShake_btn_back = lv_button_create(ui->screen_SettingAntiShake_cont_top);
    lv_obj_set_pos(ui->screen_SettingAntiShake_btn_back, 4, 4);
    lv_obj_set_size(ui->screen_SettingAntiShake_btn_back, 60, 52);
    ui->screen_SettingAntiShake_btn_back_label = lv_label_create(ui->screen_SettingAntiShake_btn_back);
    lv_label_set_text(ui->screen_SettingAntiShake_btn_back_label, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(ui->screen_SettingAntiShake_btn_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_SettingAntiShake_btn_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_SettingAntiShake_btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_SettingAntiShake_btn_back_label, LV_PCT(100));

    // Write style for screen_SettingAntiShake_btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_SettingAntiShake_btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_SettingAntiShake_btn_back, lv_color_hex(0xFFD600),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_SettingAntiShake_btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_SettingAntiShake_btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_SettingAntiShake_btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_SettingAntiShake_btn_back, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_SettingAntiShake_btn_back, &lv_font_montserratMedium_13,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_SettingAntiShake_btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_SettingAntiShake_btn_back, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_SettingAntiShake_label_title (标题)
    ui->screen_SettingAntiShake_label_title = lv_label_create(ui->screen_SettingAntiShake_cont_top);
    lv_label_set_text(ui->screen_SettingAntiShake_label_title, "防抖");
    lv_label_set_long_mode(ui->screen_SettingAntiShake_label_title, LV_LABEL_LONG_WRAP);
    font_setting(ui->screen_SettingAntiShake_label_title, FONT_SC16,
                 (font_size_config_t){.mode = FONT_SIZE_MODE_CUSTOM, .size = FONT_SIZE_24}, 0xFFFFFF, 300, 20);
    // lv_obj_align(ui->screen_SettingAntiShake_label_title, LV_ALIGN_TOP_MID, 0, 20);

    // Write codes screen_SettingAntiShake_cont_settings (设置选项容器)
    ui->screen_SettingAntiShake_cont_settings = lv_obj_create(ui->screen_SettingAntiShake);
    lv_obj_set_size(ui->screen_SettingAntiShake_cont_settings, 600, 320);
    lv_obj_align(ui->screen_SettingAntiShake_cont_settings, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_bg_opa(ui->screen_SettingAntiShake_cont_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_SettingAntiShake_cont_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(ui->screen_SettingAntiShake_cont_settings, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui->screen_SettingAntiShake_cont_settings, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(ui->screen_SettingAntiShake_cont_settings, 10, 0);

    // 创建设置按钮
    const char *btn_labels[] = {"开启", "关闭"};
    for(int i = 0; i < 2; i++) {
        lv_obj_t *btn = lv_button_create(ui->screen_SettingAntiShake_cont_settings);
        if(!btn) continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, 40);
        lv_obj_set_style_bg_opa(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *label = lv_label_create(btn);
        if(!label) continue; // 如果标签创建失败则跳过

        lv_label_set_text(label, btn_labels[i]);
        font_setting(label, FONT_SC16, (font_size_config_t){.mode = FONT_SIZE_MODE_CUSTOM, .size = FONT_SIZE_22},
                     0xFFFFFF, 0, 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

        // 添加事件处理器，传入容器对象作为用户数据
        lv_obj_add_event_cb(btn, screen_SettingAntiShake_btn_event_handler, LV_EVENT_ALL,
                            ui->screen_SettingAntiShake_cont_settings);
    }

    // The custom code of screen_SettingAntiShake.
    if(ui->screen_SettingAntiShake_cont_settings) { // 添加容器检查
        lv_obj_t *child;
        for(child = lv_obj_get_child(ui->screen_SettingAntiShake_cont_settings, 0); child != NULL;
            child = lv_obj_get_child(ui->screen_SettingAntiShake_cont_settings, lv_obj_get_index(child) + 1)) {
            if(lv_obj_check_type(child, &lv_button_class)) {
                // 获取按钮标签文本
                lv_obj_t *label = lv_obj_get_child(child, 0);
                if(label && lv_obj_check_type(label, &lv_label_class)) {
                    const char *txt = lv_label_get_text(label);

                    // 检查是否匹配目标标签
                    if(txt && strcmp(txt, g_button_labelAnti) == 0) {
                        // 设置匹配按钮的样式和状态
                        lv_obj_set_style_border_color(child, lv_color_hex(0xFF0000), LV_PART_MAIN);
                        lv_obj_add_state(child, LV_STATE_PRESSED);
                    } else {
                        // 清除其他按钮的PRESSED状态
                        if(lv_obj_has_state(child, LV_STATE_PRESSED)) {
                            lv_obj_clear_state(child, LV_STATE_PRESSED);
                            lv_obj_set_style_border_color(child, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                        }
                    }
                }
            }
        }
    }

    // Write codes screen_SettingAntiShake_cont_bottom (底部栏)
    ui->screen_SettingAntiShake_cont_bottom = lv_obj_create(ui->screen_SettingAntiShake);
    lv_obj_set_pos(ui->screen_SettingAntiShake_cont_bottom, 0, 420);
    lv_obj_set_size(ui->screen_SettingAntiShake_cont_bottom, 640, 60);
    lv_obj_set_style_bg_color(ui->screen_SettingAntiShake_cont_bottom, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_SettingAntiShake_cont_bottom, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_SettingAntiShake_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_SettingAntiShake_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(ui->screen_SettingAntiShake);

    // Init events for screen.
    events_init_screen_SettingAntiShake(ui);
}

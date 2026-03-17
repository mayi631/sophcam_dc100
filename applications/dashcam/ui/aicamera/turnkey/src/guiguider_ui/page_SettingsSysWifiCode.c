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
#include "lvgl/src/widgets/keyboard/lv_keyboard.h"
#include "hal_wifi_ctrl.h"

extern lv_style_t ttf_font_12;
SettingsSysWifiCode_t *page_syswificode;
extern wifi_info_t *cur_wifi_info;
extern struct wpa_ctrl *wpa_ctrl_handle;
static int connection_status = 0; // 0=未连接, 1=连接中, 2=连接成功, 3=连接失败

// 定时器回调函数 - 连接成功后返回WiFi列表
static void wifi_connect_success_timer_cb(lv_timer_t *timer)
{
    lv_ui_t *ui = (lv_ui_t *)timer->user_data;
    ui_load_scr_animation(ui, &ui->page_syswifi.scr, ui->screen_SettingsSysWifi_del,
                          &ui->screen_SettingsSysWifiCode_del, setup_scr_screen_SettingsSysWifi, LV_SCR_LOAD_ANIM_NONE,
                          200, 200, false, true);
    lv_timer_del(timer);
}

// 定时器回调函数 - 连接失败后恢复按钮状态
static void wifi_connect_fail_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *btn   = (lv_obj_t *)timer->user_data;
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, "连接");
    lv_obj_add_style(label, &ttf_font_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 重新启用按钮
    lv_obj_clear_state(btn, LV_STATE_DISABLED);

    // 重置连接状态
    connection_status = 0;

    lv_timer_del(timer);
}

// 返回按钮事件回调
static void wifi_code_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // 返回WiFi设置页面
    ui_load_scr_animation(ui, &ui->page_syswifi.scr, ui->screen_SettingsSysWifi_del,
                          &ui->screen_SettingsSysWifiCode_del, setup_scr_screen_SettingsSysWifi, LV_SCR_LOAD_ANIM_NONE,
                          200, 200, false, true);
}

// 连接按钮事件回调
static void wifi_code_connect_cb(lv_event_t *e)
{
    UNUSED(e);

    lv_obj_t *ta         = page_syswificode->password_ta;
    const char *password = lv_textarea_get_text(ta);

    if(!password || strlen(password) == 0) {
        printf("Invalid WiFi name or password\n");
        return;
    }

    // 检查是否已经在连接中
    if(connection_status == 1) {
        printf("WiFi connection already in progress\n");
        return;
    }

    // 更新连接按钮状态
    lv_obj_t *btn   = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_obj_add_style(label, &ttf_font_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label, "连接中...");
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 禁用按钮，防止重复点击
    lv_obj_add_state(btn, LV_STATE_DISABLED);

    connection_status = 1; // 连接中

    // 在新线程中执行连接操作
    // 注意：这里简化处理，实际应该使用线程
    int ret = Hal_Wpa_Connect(wpa_ctrl_handle, cur_wifi_info, password);
    if(ret == 0) {
        // 连接成功
        lv_label_set_text(label, "已连接");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), LV_PART_MAIN | LV_STATE_DEFAULT);

        // 延迟返回WiFi列表页面
        lv_timer_create(wifi_connect_success_timer_cb, 2000, &g_ui);

    } else {
        // 连接失败
        lv_label_set_text(label, "连接失败");
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xF44336), LV_PART_MAIN | LV_STATE_DEFAULT);

        // 延迟恢复按钮状态
        lv_timer_create(wifi_connect_fail_timer_cb, 3000, btn);
    }
}

// 密码显示/隐藏切换按钮事件回调
static void wifi_code_toggle_password_cb(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_user_data(e);

    // 检查当前密码模式状态
    bool is_password_hidden = lv_textarea_get_password_mode(ta);

    if(is_password_hidden) {
        // 当前密码隐藏，点击后显示密码
        lv_textarea_set_password_mode(ta, false);
        // 切换图标为闭眼（表示可以点击隐藏）
        lv_label_set_text(page_syswificode->toggle_icon, LV_SYMBOL_EYE_OPEN);
    } else {
        // 当前密码显示，点击后隐藏密码
        lv_textarea_set_password_mode(ta, true);
        // 切换图标为睁眼（表示可以点击显示）
        lv_label_set_text(page_syswificode->toggle_icon, LV_SYMBOL_EYE_CLOSE);
    }
}

// 键盘事件回调
static void wifi_code_keyboard_cb(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = lv_event_get_user_data(e);

    lv_keyboard_set_textarea(kb, ta);

    // 可以根据需要处理特定的按键，例如连接、回车等
}

static void events_init_screen_page_syswificode(lv_ui_t *ui)
{
    // 注册返回按钮事件回调
    lv_obj_add_event_cb(page_syswificode->btn_back, wifi_code_back_cb, LV_EVENT_CLICKED, ui);
    // 注册密码显示/隐藏切换按钮事件回调
    lv_obj_add_event_cb(page_syswificode->btn_toggle, wifi_code_toggle_password_cb, LV_EVENT_CLICKED,
                        page_syswificode->password_ta);
    // 注册键盘回调
    lv_obj_add_event_cb(page_syswificode->password_ta, wifi_code_keyboard_cb, LV_EVENT_ALL, page_syswificode->keyboard);
    // 注册连接按钮事件回调
    lv_obj_add_event_cb(page_syswificode->btn_connect, wifi_code_connect_cb, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_SettingsSysWifiCode(lv_ui_t *ui)
{
    // The custom code of scr.
    MLOG_DBG("loading page_syswificode...\n");

    // 重置连接状态
    // connection_status = 0;

    page_syswificode      = &ui->page_syswificode;
    page_syswificode->del = true;

    // 页面容器初始化（重置）
    if(page_syswificode->scr != NULL) {
        if(lv_obj_is_valid(page_syswificode->scr)) {
            MLOG_DBG("page_syswificode->scr 仍然有效，删除旧对象\n");
            lv_obj_del(page_syswificode->scr);
        } else {
            MLOG_DBG("page_syswificode->scr 已被自动销毁，仅重置指针\n");
        }
        page_syswificode->scr = NULL;
    }

    // 创建页面容器
    page_syswificode->scr = lv_obj_create(NULL);
    lv_obj_set_size(page_syswificode->scr, 640, 480);
    lv_obj_set_scrollbar_mode(page_syswificode->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(page_syswificode->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswificode->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswificode->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(page_syswificode->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮（黄色背景）
    page_syswificode->btn_back = lv_btn_create(page_syswificode->scr);
    lv_obj_set_pos(page_syswificode->btn_back, 16, 16);
    lv_obj_set_size(page_syswificode->btn_back, 80, 40);
    lv_obj_set_style_radius(page_syswificode->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswificode->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 返回按钮图标
    page_syswificode->label_back = lv_label_create(page_syswificode->btn_back);
    lv_label_set_text(page_syswificode->label_back, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(page_syswificode->label_back, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswificode->label_back, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswificode->label_back, LV_ALIGN_CENTER, 0, 0);

    // 顶部居中标题
    page_syswificode->title = lv_label_create(page_syswificode->scr);
    char title_text[64]     = "输入密码";
    // snprintf(title_text, sizeof(title_text), "输入密码");
    lv_label_set_text(page_syswificode->title, title_text);
    lv_obj_set_style_text_color(page_syswificode->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswificode->title, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(page_syswificode->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(page_syswificode->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswificode->title, LV_ALIGN_TOP_MID, 0, 24);

    // 密码输入区域
    page_syswificode->password_ta = lv_textarea_create(page_syswificode->scr);
    lv_obj_set_size(page_syswificode->password_ta, 580, 50); // Adjust size
    //  Adjust position below title/buttons
    lv_obj_align(page_syswificode->password_ta, LV_ALIGN_TOP_MID, 0, 70);
    lv_textarea_set_placeholder_text(page_syswificode->password_ta, "输入密码");
    lv_textarea_set_password_mode(page_syswificode->password_ta, true);
    lv_obj_set_style_bg_color(page_syswificode->password_ta, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(page_syswificode->password_ta, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswificode->password_ta, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(page_syswificode->password_ta, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswificode->password_ta, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(page_syswificode->password_ta, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(page_syswificode->password_ta, lv_color_hex(0xFFC107),
                                LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(page_syswificode->password_ta, lv_color_hex(0xFFC107), LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(page_syswificode->password_ta, 255, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(page_syswificode->password_ta, 0,
                            LV_PART_SCROLLBAR | LV_STATE_DEFAULT); // 隐藏滚动条

    // 密码显示/隐藏切换按钮 (Eye icon)
    page_syswificode->btn_toggle = lv_btn_create(page_syswificode->password_ta);
    lv_obj_set_size(page_syswificode->btn_toggle, 40, 40); // Adjust size
    lv_obj_align(page_syswificode->btn_toggle, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_opa(page_syswificode->btn_toggle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswificode->btn_toggle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    page_syswificode->toggle_icon = lv_label_create(page_syswificode->btn_toggle);
    lv_label_set_text(page_syswificode->toggle_icon, LV_SYMBOL_EYE_CLOSE);
    lv_obj_set_style_text_color(page_syswificode->toggle_icon, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswificode->toggle_icon, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswificode->toggle_icon, LV_ALIGN_CENTER, 0, 0);

    // 键盘
    page_syswificode->keyboard = lv_keyboard_create(page_syswificode->scr);
    lv_obj_set_size(page_syswificode->keyboard, LV_HOR_RES, LV_VER_RES / 2); // Adjust size
    lv_obj_align(page_syswificode->keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);     // Align to bottom
    lv_keyboard_set_textarea(page_syswificode->keyboard, page_syswificode->password_ta);
    // Customize keyboard appearance
    lv_obj_set_style_bg_color(page_syswificode->keyboard, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(page_syswificode->keyboard, 5, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswificode->keyboard, lv_color_hex(0x4A4A4A), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(page_syswificode->keyboard, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswificode->keyboard, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);

    // 右上角连接按钮
    page_syswificode->btn_connect = lv_btn_create(page_syswificode->scr);
    lv_obj_set_pos(page_syswificode->btn_connect, 544, 16); // Adjust position
    lv_obj_set_size(page_syswificode->btn_connect, 80, 40);
    lv_obj_set_style_radius(page_syswificode->btn_connect, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswificode->btn_connect, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    page_syswificode->label_connect = lv_label_create(page_syswificode->btn_connect);
    lv_label_set_text(page_syswificode->label_connect, "连接");
    lv_obj_set_style_text_color(page_syswificode->label_connect, lv_color_hex(0x181818),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswificode->label_connect, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswificode->label_connect, LV_ALIGN_CENTER, 0, 0);

    events_init_screen_page_syswificode(ui);

    // Update current screen layout.
    lv_obj_update_layout(page_syswificode->scr);
}

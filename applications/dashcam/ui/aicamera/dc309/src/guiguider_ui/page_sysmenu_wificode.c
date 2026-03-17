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

#include "config.h"
#include "custom.h"
#include "hal_wifi_ctrl.h"
#include "indev.h"
#include "linux/input.h"
#include "lvgl/src/widgets/keyboard/lv_keyboard.h"
#include "page_all.h"
#include "style_common.h"

extern lv_style_t ttf_font_14;
extern lv_style_t ttf_font_18;
extern wifi_info_t *cur_wifi_info;
extern lv_style_t ttf_font_28;
static int connection_status = 0; // 0=未连接, 1=连接中, 2=连接成功, 3=连接失败

lv_obj_t *sysMenu_WifiCode_s;
lv_obj_t *password_ta;
lv_obj_t *btn_connect_s;
lv_obj_t *toggle_icon_s;

// 定时器回调函数 - 连接成功后返回WiFi列表
static void wifi_connect_success_timer_cb(lv_timer_t *timer)
{
    lv_ui_t *ui = (lv_ui_t *)timer->user_data;
    ui_load_scr_animation(ui, &sysMenu_WifiList_s, 1, NULL, sysMenu_WifiList, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

// 定时器回调函数 - 连接失败后恢复按钮状态
static void wifi_connect_fail_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *btn   = (lv_obj_t *)timer->user_data;
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, str_language_connect[get_curr_language()]);
    lv_obj_add_style(label, &ttf_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 重新启用按钮
    lv_obj_clear_state(btn, LV_STATE_DISABLED);

    // 重置连接状态
    connection_status = 0;
}

// 返回按钮事件回调
static void wifi_code_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);
    // 返回WiFi设置页面
    ui_load_scr_animation(ui, &sysMenu_WifiList_s, 1, NULL, sysMenu_WifiList, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

// 连接按钮事件回调占位符
static void wifi_code_connect_cb(lv_event_t *e)
{
    UNUSED(e);

    lv_obj_t *ta         = password_ta;
    const char *password = lv_textarea_get_text(ta);

    if(!password || strlen(password) == 0) {
        MLOG_ERR("Invalid WiFi name or password\n");
        return;
    }

    // 检查是否已经在连接中
    if(connection_status == 1) {
        MLOG_ERR("WiFi connection already in progress\n");
        return;
    }

    // 更新连接按钮状态
    lv_obj_t *btn   = lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    lv_label_set_text(label, str_language_connecting[get_curr_language()]);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
    lv_obj_invalidate(label);
    lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
    // 禁用按钮，防止重复点击
    lv_obj_add_state(btn, LV_STATE_DISABLED);

    connection_status = 1; // 连接中

    // 在新线程中执行连接操作
    // 注意：这里简化处理，实际应该使用线程
    int ret = Hal_Wpa_Connect(cur_wifi_info, password);
    if(ret == 0) {
        // 连接成功
        lv_label_set_text(label, str_language_connected[get_curr_language()]);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x4CAF50), LV_PART_MAIN | LV_STATE_DEFAULT);

        // 延迟返回WiFi列表页面
        lv_timer_t *timer = lv_timer_create(wifi_connect_success_timer_cb, 1000, &g_ui);
        lv_timer_set_repeat_count(timer, 1); // 只触发一次
    } else {
        // 连接失败
        lv_label_set_text(label, str_language_connection_failed[get_curr_language()]);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0xF44336), LV_PART_MAIN | LV_STATE_DEFAULT);

        // 延迟恢复按钮状态
        lv_timer_t *timer = lv_timer_create(wifi_connect_fail_timer_cb, 1000, btn);
        lv_timer_set_repeat_count(timer, 1); // 只触发一次
    }
}

// 密码显示/隐藏切换按钮事件回调
static void wifi_code_toggle_password_cb(lv_event_t *e)
{
    // lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *ta = lv_event_get_user_data(e);
    // lv_ui_t *ui;

    // 获取ui指针
    // extern lv_ui_t g_ui;
    // ui = &g_ui;

    // 检查当前密码模式状态
    bool is_password_hidden = lv_textarea_get_password_mode(ta);

    if(is_password_hidden) {
        // 当前密码隐藏，点击后显示密码
        lv_textarea_set_password_mode(ta, false);
        // 切换图标为闭眼（表示可以点击隐藏）
        lv_label_set_text(toggle_icon_s, LV_SYMBOL_EYE_OPEN);
    } else {
        // 当前密码显示，点击后隐藏密码
        lv_textarea_set_password_mode(ta, true);
        // 切换图标为睁眼（表示可以点击显示）
        lv_label_set_text(toggle_icon_s, LV_SYMBOL_EYE_CLOSE);
    }
}

// 键盘事件回调
static void wifi_code_keyboard_cb(lv_event_t *e)
{
    // lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *kb = lv_event_get_user_data(e);

    lv_keyboard_set_textarea(kb, ta);

    // 可以根据需要处理特定的按键，例如连接、回车等
}


static void wifi_key_event_callback(int key_code, int key_value)
{
    // 确保密码输入框存在且有效
    if(!password_ta || !lv_obj_is_valid(password_ta)) {
        return;
    }
    // 确保输入框获得焦点（显示光标）
    if(!lv_obj_has_state(password_ta, LV_STATE_FOCUSED)) {
        lv_group_t *g = lv_group_get_default();
        if(g) {
            lv_group_focus_obj(password_ta);
        } else {
            // 如果没有默认组，直接设置焦点
            lv_obj_add_state(password_ta, LV_STATE_FOCUSED);
        }
    }
    // 只有按键按下事件才处理
    if(key_value != 1) {
        return;
    }
    switch(key_code) {
        case KEY_MENU: // 菜单键
            MLOG_INFO("返回WiFi列表页面\n");
            ui_load_scr_animation(&g_ui, &sysMenu_WifiList_s, 1, NULL, sysMenu_WifiList, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            return;
        case KEY_LEFT: // 左键 - 光标左移
            // MLOG_ERR("左键 - 光标左移\n");
            lv_textarea_cursor_left(password_ta);
            break;

        case KEY_RIGHT: // 右键 - 光标右移
            // MLOG_DBG("右键 - 光标右移\n");
            lv_textarea_cursor_right(password_ta);
            break;
        case KEY_OK: // OK键连接
            lv_obj_t *ta         = password_ta;
            const char *password = lv_textarea_get_text(ta);
            if(!password || strlen(password) == 0) {
                MLOG_ERR("Invalid WiFi name or password\n");
                return;
            }
            // 检查是否已经在连接中
            if(connection_status == 1) {
                MLOG_ERR("WiFi connection already in progress\n");
                return;
            }
            // 更新连接按钮状态
            lv_obj_t *label = lv_obj_get_child(btn_connect_s, 0);
            lv_label_set_text(label, str_language_connecting[get_curr_language()]);
            lv_obj_set_style_bg_color(btn_connect_s, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT);
            // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
            lv_obj_invalidate(label);
            lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
            // 禁用按钮，防止重复点击
            lv_obj_add_state(btn_connect_s, LV_STATE_DISABLED);

            connection_status = 1; // 连接中

            // 在新线程中执行连接操作
            // 注意：这里简化处理，实际应该使用线程
            int ret = Hal_Wpa_Connect(cur_wifi_info, password);
            if(ret == 0) {
                // 连接成功
                lv_label_set_text(label, str_language_connected[get_curr_language()]);
                lv_obj_set_style_bg_color(btn_connect_s, lv_color_hex(0x4CAF50), LV_PART_MAIN | LV_STATE_DEFAULT);

                // 延迟返回WiFi列表页面
                lv_timer_t *timer = lv_timer_create(wifi_connect_success_timer_cb, 1000, &g_ui);
                lv_timer_set_repeat_count(timer, 1); // 只触发一次
            } else {
                // 连接失败
                lv_label_set_text(label, str_language_connection_failed[get_curr_language()]);
                lv_obj_set_style_bg_color(btn_connect_s, lv_color_hex(0xF44336), LV_PART_MAIN | LV_STATE_DEFAULT);

                // 延迟恢复按钮状态
                lv_timer_t *timer = lv_timer_create(wifi_connect_fail_timer_cb, 1000, btn_connect_s);
                lv_timer_set_repeat_count(timer, 1); // 只触发一次
            }
            break;

        default: break;
    }
    // 确保光标可见
    lv_textarea_set_cursor_click_pos(password_ta, true);
}


void sysMenu_WifiCode(lv_ui_t *ui)
{
    // 创建主页面1 容器
    if(sysMenu_WifiCode_s != NULL) {
        if(lv_obj_is_valid(sysMenu_WifiCode_s)) {
            MLOG_DBG("page_syswificode->scr 仍然有效，删除旧对象\n");
            lv_obj_del(sysMenu_WifiCode_s);
        } else {
            MLOG_DBG("page_syswificode->scr 已被自动销毁，仅重置指针\n");
        }
        sysMenu_WifiCode_s = NULL;
    }

    // 重置连接状态
    connection_status = 0;

    // Create screen
    sysMenu_WifiCode_s = lv_obj_create(NULL);
    lv_obj_set_size(sysMenu_WifiCode_s, H_RES, V_RES);
    lv_obj_add_style( sysMenu_WifiCode_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮（黄色背景）
    lv_obj_t* btn_back = lv_btn_create(sysMenu_WifiCode_s);
    lv_obj_set_pos(btn_back, 4, 4);
    lv_obj_set_size(btn_back, 60, 52);
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_back, wifi_code_back_cb, LV_EVENT_CLICKED, ui);
    // 返回按钮图标
    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, LV_SYMBOL_LEFT "");
    lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    // 顶部居中标题
    lv_obj_t* title = lv_label_create(sysMenu_WifiCode_s);
    char title_text[64] = "输入WiFi密码";
    // snprintf(title_text, sizeof(title_text), "输入%s密码", current_wifi_name ? current_wifi_name : "");
    lv_label_set_text(title, title_text);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    // 密码输入区域
    password_ta = lv_textarea_create(sysMenu_WifiCode_s);
    lv_obj_set_size(password_ta, 580, 50); // Adjust size
    lv_obj_align(password_ta, LV_ALIGN_TOP_MID, 0,
                 70); // Adjust position below title/buttons
    lv_textarea_set_placeholder_text(password_ta, "输入密码");
    lv_textarea_set_password_mode(password_ta, false); // 默认显示密码
    lv_obj_set_style_bg_color(password_ta, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(password_ta, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(password_ta, &ttf_font_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(password_ta, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(password_ta, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(password_ta, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(password_ta, lv_color_hex(0xFFC107), LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_color(password_ta, lv_color_hex(0xFFC107), LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(password_ta, 255, LV_PART_CURSOR | LV_STATE_FOCUSED);
    lv_obj_set_style_text_align(password_ta, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(password_ta, 0,
                            LV_PART_SCROLLBAR | LV_STATE_DEFAULT); // 隐藏滚动条

    // 密码显示/隐藏切换按钮 (Eye icon)
    lv_obj_t *btn_toggle = lv_btn_create(password_ta);
    lv_obj_set_size(btn_toggle, 40, 40); // Adjust size
    lv_obj_align(btn_toggle, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_bg_opa(btn_toggle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_toggle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_toggle, wifi_code_toggle_password_cb, LV_EVENT_CLICKED, password_ta);
    toggle_icon_s = lv_label_create(btn_toggle);
    lv_label_set_text(toggle_icon_s, LV_SYMBOL_EYE_CLOSE);
    lv_obj_set_style_text_color(toggle_icon_s, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(toggle_icon_s, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(toggle_icon_s, LV_ALIGN_CENTER, 0, 0);

    // 键盘
    lv_obj_t *keyboard = lv_keyboard_create(sysMenu_WifiCode_s);
    lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES / 2); // Adjust size
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);     // Align to bottom
    lv_keyboard_set_textarea(keyboard, password_ta);
    lv_obj_add_event_cb(password_ta, wifi_code_keyboard_cb, LV_EVENT_ALL, keyboard);
    // Customize keyboard appearance
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(keyboard, 5, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0x4A4A4A), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(keyboard, lv_color_hex(0xFFFFFF), LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(keyboard, 0, LV_PART_ITEMS | LV_STATE_DEFAULT);

    // 右上角连接按钮
    btn_connect_s = lv_btn_create(sysMenu_WifiCode_s);
    lv_obj_set_pos(btn_connect_s, 544, 16); // Adjust position
    lv_obj_set_size(btn_connect_s, 80, 40);
    lv_obj_set_style_radius(btn_connect_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_connect_s, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_connect_s, wifi_code_connect_cb, LV_EVENT_CLICKED, password_ta);
    lv_obj_t *label_connect = lv_label_create(btn_connect_s);
    lv_label_set_text(label_connect, str_language_connect[get_curr_language()]);
    lv_obj_add_style(label_connect, &ttf_font_14, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_connect, LV_ALIGN_CENTER, 0, 0);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(sysMenu_WifiCode_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    set_current_page_handler(wifi_key_event_callback);

    // Update current screen layout.
    lv_obj_update_layout(sysMenu_WifiCode_s);
}


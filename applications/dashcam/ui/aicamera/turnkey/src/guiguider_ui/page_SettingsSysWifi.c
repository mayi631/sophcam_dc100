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
#include "mlog.h"
#include "config.h"
#include "hal_wifi_ctrl.h"

extern lv_style_t ttf_font_18;

// 全局变量用于存储扫描到的WiFi信息
extern struct wpa_ctrl *wpa_ctrl_handle;
extern wifi_info_t wifi_scan_results[WIFI_INFO_MAX];
extern int wifi_scan_count;
wifi_info_t *cur_wifi_info = NULL;
// WiFi开关全局状态变量，0=关，1=开
static int wifi_switch_state = 1;
static SettingsSysWifi_t *page_syswifi;

// WiFi列表项点击事件回调
/*
 * 1. 如果wifi已连接，则跳过
 * 2. 如果wifi已保存，则连接
 * 3. 如果wifi未保存，则跳转到输入密码页面。
 */
static void wifi_list_item_clicked_cb(lv_event_t *e)
{
    int ret = 0;
    UNUSED(e);
    // 获取用户数据，即WiFi名称
    cur_wifi_info = (wifi_info_t *)lv_event_get_user_data(e);
    // 1. 如果wifi已连接，则跳过
    if(cur_wifi_info->connect_flag == true) {
        MLOG_DBG("WiFi %s is connected, skip\n", cur_wifi_info->ssid);
        return;
    }

    // 2. 如果wifi已保存，则连接
    if(cur_wifi_info->save_flag == true) {
        MLOG_DBG("WiFi %s is saved, select it\n", cur_wifi_info->ssid);
        ret = Hal_Wpa_Connect(wpa_ctrl_handle, cur_wifi_info, NULL);
        if(ret != 0) {
            MLOG_ERR("WiFi %s connect failed\n", cur_wifi_info->ssid);
            return;
        }
    }

    MLOG_DBG("cur_wifi_info->ssid: %s\n", cur_wifi_info->ssid);
    // 3. 如果wifi未保存，则跳转到输入密码页面。
    ui_load_scr_animation(&g_ui, &g_ui.page_syswificode.scr, g_ui.screen_SettingsSysWifiCode_del,
                          &g_ui.screen_SettingsSysWifi_del, setup_scr_screen_SettingsSysWifiCode, LV_SCR_LOAD_ANIM_NONE,
                          20, 20, false, true);
}

// WiFi列表刷新回调函数
/*
 * WiFi列表刷新函数
 * 1. 并显示"正在扫描WiFi..."
 * 2. 扫描WiFi网络
 * 3. 清空现有列表，
 * 4. 为每个扫描到的WiFi网络创建按钮，并添加点击事件回调
 */
static void wifi_list_refresh(lv_event_t *e)
{
    int ret = 0;
    UNUSED(e);

    // 1. 显示"正在扫描WiFi..."
    lv_obj_clear_flag(page_syswifi->label_scan, LV_OBJ_FLAG_HIDDEN);
    // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
    lv_obj_invalidate(page_syswifi->label_scan);
    lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
    MLOG_DBG("强制UI刷新完成\n");
    // 2. 扫描WiFi网络
    wifi_scan_count = WIFI_INFO_MAX;
    ret             = Hal_Wpa_GetScanResult(wpa_ctrl_handle, wifi_scan_results, &wifi_scan_count);
    if(ret != 0) {
        MLOG_ERR("WiFi scan failed, showing empty list\n");
        wifi_scan_count = 0;
    }

    // 3. 清空现有列表
    lv_obj_clean(page_syswifi->list);
    // 4. 为每个扫描到的WiFi网络创建按钮
    for(int i = 0; i < wifi_scan_count; ++i) {
        // 创建WiFi按钮
        lv_obj_t *btn = lv_list_add_btn(page_syswifi->list, LV_SYMBOL_WIFI, wifi_scan_results[i].ssid);

        // 设置字体
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_obj_set_style_text_font(label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 绑定点击事件，传递WiFi信息
        lv_obj_add_event_cb(btn, wifi_list_item_clicked_cb, LV_EVENT_CLICKED, (void *)&wifi_scan_results[i]);
    }

    lv_obj_add_flag(page_syswifi->label_scan, LV_OBJ_FLAG_HIDDEN); // 隐藏"正在扫描WiFi..."
}

// 屏幕加载完成事件回调
static void wifi_list_scroll_top_timer_cb(lv_timer_t *timer)
{
    lv_ui_t *ui = (lv_ui_t *)timer->user_data;
    wifi_list_refresh(NULL);
    if(ui && page_syswifi->list) {
        lv_obj_scroll_to_y(page_syswifi->list, 0, LV_ANIM_OFF);
    }
    static int count = 0;
    count++;
    if(count >= 1) { // 尝试1次后删除定时器
        lv_timer_del(timer);
        count = 0;
    }
}

// // 异步初始化wpa_ctrl的回调函数
// static void async_init_wpa_ctrl_cb(lv_timer_t *timer)
// {
//     // 检查是否已经初始化
//     if(wpa_ctrl_handle != NULL) {
//         printf("WPA_CTRL already initialized, skipping\n");
//         // 如果已经初始化，直接开始扫描
//         wifi_list_refresh(NULL);
//         lv_timer_del(timer);
//         return;
//     }

//     // 初始化wpa_ctrl
//     if(init_wpa_ctrl() == 0) {
//         printf("WPA_CTRL initialized successfully\n");
//         // 初始化成功后，自动开始扫描WiFi
//         wifi_list_refresh(NULL);
//     } else {
//         printf("WPA_CTRL initialization failed\n");
//     }

//     // 删除定时器
//     lv_timer_del(timer);
// }

// // 屏幕加载完成后，调用一次扫描 wifi 列表
static void screen_page_syswifi_loaded_cb(lv_event_t *e)
{
    // if(wifi_switch_state) {
    //     wifi_list_refresh(NULL);
    // }
    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);

    // 启动定时器，多次滚动到顶部
    lv_timer_create(wifi_list_scroll_top_timer_cb, 30, ui);
    // lv_event_send(&page_syswifi->label_refresh, LV_EVENT_CLICKED, NULL);
}

// 返回按钮回调函数
static void wifi_back_cb(lv_event_t *e)
{
    lv_ui_t *ui = lv_event_get_user_data(e);

    // 返回系统设置页面
    ui_load_scr_animation(ui, &ui->page_settingssys.scr, ui->screen_SettingsSys_del, &ui->screen_SettingsSysWifi_del,
                          setup_scr_screen_SettingsSys, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
}

// WiFi开关事件回调
static void wifi_switch_event_cb(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_target(e);
    // lv_ui_t *ui  = lv_event_get_user_data(e);

    if(lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        wifi_switch_state = 1;
        // 打开，显示"我的网络"标题和WiFi列表
        lv_obj_clear_flag(page_syswifi->label_my_network, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_syswifi->label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_syswifi->list, LV_OBJ_FLAG_HIDDEN);
        wifi_list_refresh(NULL);
    } else {
        wifi_switch_state = 0;
        // 关闭，隐藏"我的网络"标题和WiFi列表
        lv_obj_add_flag(page_syswifi->label_my_network, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_syswifi->label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_syswifi->list, LV_OBJ_FLAG_HIDDEN);
    }
}

static void events_init_screen_page_syswifi(lv_ui_t *ui)
{
    // 注册WiFi开关事件回调
    lv_obj_add_event_cb(page_syswifi->topbar_wifi_switch, wifi_switch_event_cb, LV_EVENT_VALUE_CHANGED, ui);
    // 注册屏幕加载完成事件，滚动WiFi列表到顶部
    lv_obj_add_event_cb(page_syswifi->scr, screen_page_syswifi_loaded_cb, LV_EVENT_SCREEN_LOADED, ui);
    // 注册刷新按钮事件回调
    lv_obj_add_event_cb(page_syswifi->label_refresh, wifi_list_refresh, LV_EVENT_CLICKED, NULL);
    // 注册返回按钮事件回调
    lv_obj_add_event_cb(page_syswifi->btn_back, wifi_back_cb, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_SettingsSysWifi(lv_ui_t *ui)
{
    MLOG_DBG("loading page_syswifi...\n");

    page_syswifi      = &ui->page_syswifi;
    page_syswifi->del = true;

    // 创建主页面1 容器
    if(page_syswifi->scr != NULL) {
        if(lv_obj_is_valid(page_syswifi->scr)) {
            MLOG_DBG("page_syswifi->scr 仍然有效，删除旧对象\n");
            lv_obj_del(page_syswifi->scr);
        } else {
            MLOG_DBG("page_syswifi->scr 已被自动销毁，仅重置指针\n");
        }
        page_syswifi->scr = NULL;
    }

    // The custom code of scr.
    page_syswifi->scr = lv_obj_create(NULL);
    lv_obj_set_size(page_syswifi->scr, 640, 480);
    lv_obj_set_scrollbar_mode(page_syswifi->scr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(page_syswifi->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswifi->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswifi->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(page_syswifi->scr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 1. 顶部返回按钮（左上角，圆形带返回箭头，标签为按钮子控件并居中）
    page_syswifi->btn_back = lv_btn_create(page_syswifi->scr);
    lv_obj_set_pos(page_syswifi->btn_back, 16, 16);
    lv_obj_set_size(page_syswifi->btn_back, 80, 40);
    lv_obj_set_style_radius(page_syswifi->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswifi->btn_back, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    page_syswifi->label_back_btn = lv_label_create(page_syswifi->btn_back);
    lv_label_set_text(page_syswifi->label_back_btn, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(page_syswifi->label_back_btn, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswifi->label_back_btn, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswifi->label_back_btn, LV_ALIGN_CENTER, 0, 0);

    // 2. 顶部居中标题"Wi-Fi"
    page_syswifi->title = lv_label_create(page_syswifi->scr);
    lv_label_set_text(page_syswifi->title, "Wi-Fi");
    lv_obj_set_style_text_color(page_syswifi->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswifi->title, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(page_syswifi->title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(page_syswifi->title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswifi->title, LV_ALIGN_TOP_MID, 0, 24);

    // 3. 顶部"Wi-Fi开关"+开关栏，居中显示且不显示滑动条
    page_syswifi->topbar_wifi_cont = lv_obj_create(page_syswifi->scr);
    lv_obj_set_size(page_syswifi->topbar_wifi_cont, 400, 56);
    lv_obj_align(page_syswifi->topbar_wifi_cont, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_radius(page_syswifi->topbar_wifi_cont, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(page_syswifi->topbar_wifi_cont, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswifi->topbar_wifi_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(page_syswifi->topbar_wifi_cont, LV_SCROLLBAR_MODE_OFF);

    // "Wi-Fi开关"文字
    page_syswifi->topbar_wifi_label = lv_label_create(page_syswifi->topbar_wifi_cont);
    lv_label_set_text(page_syswifi->topbar_wifi_label, "Wi-Fi开关");
    lv_obj_add_style(page_syswifi->topbar_wifi_label, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswifi->topbar_wifi_label, LV_ALIGN_LEFT_MID, 16, 0);

    // WiFi开关
    page_syswifi->topbar_wifi_switch = lv_switch_create(page_syswifi->topbar_wifi_cont);
    lv_obj_set_size(page_syswifi->topbar_wifi_switch, 60, 32);
    lv_obj_align(page_syswifi->topbar_wifi_switch, LV_ALIGN_RIGHT_MID, -16, 0);
    lv_obj_set_style_bg_color(page_syswifi->topbar_wifi_switch, lv_color_hex(0xFFC107),
                              LV_PART_INDICATOR | LV_STATE_CHECKED);

    // 4. WiFi列表及"我的网络"标题
    // 4.1 "我的网络"标题
    page_syswifi->label_my_network = lv_label_create(page_syswifi->scr);
    lv_label_set_text(page_syswifi->label_my_network, "网络");
    lv_obj_add_style(page_syswifi->label_my_network, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswifi->label_my_network, LV_ALIGN_TOP_LEFT, 48, 144);

    // 4.2 刷新按钮(手动触发 扫描wifi)
    page_syswifi->label_refresh = lv_label_create(page_syswifi->scr);
    lv_label_set_text(page_syswifi->label_refresh, "刷新");
    lv_obj_add_style(page_syswifi->label_refresh, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswifi->label_refresh, LV_ALIGN_TOP_LEFT, 500, 144);
    lv_obj_add_flag(page_syswifi->label_refresh, LV_OBJ_FLAG_CLICKABLE);

    // 4.3 提示正在扫描WiFi的标签
    page_syswifi->label_scan = lv_label_create(page_syswifi->scr);
    lv_label_set_text(page_syswifi->label_scan, "正在扫描WiFi...");
    lv_obj_add_style(page_syswifi->label_scan, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(page_syswifi->label_scan, LV_ALIGN_TOP_LEFT, 200, 144);
    lv_obj_add_flag(page_syswifi->label_scan, LV_OBJ_FLAG_HIDDEN);

    // 5 WiFi列表，居中放置，显示可上下拖动的滑动条
    page_syswifi->list = lv_list_create(page_syswifi->scr);
    lv_obj_set_size(page_syswifi->list, 480, 280);
    lv_obj_align(page_syswifi->list, LV_ALIGN_TOP_MID, 0, 176);
    lv_obj_set_style_bg_color(page_syswifi->list, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(page_syswifi->list, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(page_syswifi->list, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(page_syswifi->list, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(page_syswifi->list, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(page_syswifi->list, LV_SCROLLBAR_MODE_AUTO);

    // 根据当前状态设置开关和列表显示
    if(wifi_switch_state) {
        lv_obj_add_state(page_syswifi->topbar_wifi_switch, LV_STATE_CHECKED);
        lv_obj_clear_flag(page_syswifi->label_my_network, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_syswifi->label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(page_syswifi->list, LV_OBJ_FLAG_HIDDEN);
        // 不在这里调用 wifi_list_refresh，等待异步初始化完成
    } else {
        lv_obj_clear_state(page_syswifi->topbar_wifi_switch, LV_STATE_CHECKED);
        lv_obj_add_flag(page_syswifi->label_my_network, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_syswifi->label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(page_syswifi->list, LV_OBJ_FLAG_HIDDEN);
    }

    // 注册回调函数
    events_init_screen_page_syswifi(ui);

    // Update current screen layout.
    lv_obj_update_layout(page_syswifi->scr);
}

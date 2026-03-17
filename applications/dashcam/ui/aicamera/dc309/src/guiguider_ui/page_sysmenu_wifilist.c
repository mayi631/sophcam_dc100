// #############################################################################
// ! #region 1. 头文件与宏定义
// #############################################################################
#define DEBUG
#include "config.h"
#include "custom.h"
#include "gui_guider.h"
#include "hal_wifi_ctrl.h"
#include "indev.h"
#include "linux/input.h"
#include "lvgl.h"
#include "mlog.h"
#include "page_all.h"
#include "style_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #endregion
// #############################################################################
// ! #region 2. 数据结构定义
// #############################################################################

// #endregion
// #############################################################################
// ! #region 3. 全局变量 &  函数声明
// #############################################################################

lv_obj_t* sysMenu_WifiList_s;
lv_obj_t* label_my_network_s;
lv_obj_t* label_refresh;
lv_obj_t* label_notice;
lv_obj_t* wifi_list_s;

// 全局变量用于存储扫描到的WiFi信息
int wifi_scan_count = 0;
wifi_info_t wifi_scan_results[WIFI_INFO_MAX];
wifi_info_t* cur_wifi_info = NULL;

// 全局变量用于记录当前选中的WiFi项索引
static int current_wifi_index = -1;
static lv_obj_t* current_selected_btn = NULL;

// WiFi扫描状态管理
static lv_timer_t* wifi_scan_timer = NULL;

static void hide_label_notice(lv_timer_t* timer);
static void wifi_scan_check_status(lv_timer_t* timer);
static void wifi_list_item_clicked_cb(lv_event_t* e);
static void wifi_refresh_btn_cb(lv_event_t* e);

// #endregion
// #############################################################################
// ! #region 4. 内部工具函数（注意用static修饰）
// #############################################################################

// 判断WiFi是否需要密码
static bool is_wifi_password_protected(const wifi_info_t* info)
{
    if (info == NULL) {
        return false;
    }

    const char* flags = info->flags;
    if (flags == NULL || flags[0] == '\0') {
        return false;
    }

    // 检查flags字符串中是否包含表示加密类型的关键字
    if (strstr(flags, "WPA") || strstr(flags, "WEP") || strstr(flags, "SAE") || strstr(flags, "PSK")) {
        return true;
    }

    return false;
}

// 清除之前选中的样式
static void clear_selected_style(void)
{
    if (current_selected_btn != NULL) {
        lv_obj_set_style_border_color(current_selected_btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
        lv_obj_set_style_border_width(current_selected_btn, 0, LV_PART_MAIN);
        current_selected_btn = NULL;
    }
}

// 设置选中样式（蓝色边框）
static void set_selected_style(lv_obj_t* btn)
{
    clear_selected_style();
    lv_obj_set_style_border_color(btn, lv_color_hex(0x007AFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN);
    current_selected_btn = btn;
}

// 选择下一个WiFi项
static void select_next_wifi(void)
{
    if (wifi_scan_count == 0)
        return;

    if (current_wifi_index == -1) {
        current_wifi_index = 0;
    } else {
        current_wifi_index = (current_wifi_index + 1) % wifi_scan_count;
    }

    // 获取列表中的按钮
    lv_obj_t* btn = lv_obj_get_child(wifi_list_s, current_wifi_index);
    if (btn != NULL) {
        set_selected_style(btn);
        // 滚动到选中项
        lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    }
}

// 选择上一个WiFi项
static void select_prev_wifi(void)
{
    if (wifi_scan_count == 0)
        return;

    if (current_wifi_index == -1) {
        current_wifi_index = wifi_scan_count - 1;
    } else {
        current_wifi_index = (current_wifi_index - 1 + wifi_scan_count) % wifi_scan_count;
    }

    // 获取列表中的按钮
    lv_obj_t* btn = lv_obj_get_child(wifi_list_s, current_wifi_index);
    if (btn != NULL) {
        set_selected_style(btn);
        // 滚动到选中项
        lv_obj_scroll_to_view(btn, LV_ANIM_ON);
    }
}

static void wifi_scan_timer_stop(void)
{
    if (wifi_scan_timer != NULL) {
        lv_timer_del(wifi_scan_timer);
        wifi_scan_timer = NULL;
    }
}

// 确认选择当前WiFi
static void confirm_selected_wifi(void)
{
    if (current_wifi_index >= 0 && current_wifi_index < wifi_scan_count) {
        // 获取当前选中的WiFi信息
        cur_wifi_info = &wifi_scan_results[current_wifi_index];

        // 直接调用点击处理逻辑，而不是创建事件对象
        wifi_info_t* wifi_info = (wifi_info_t*)cur_wifi_info;

        // 1. 如果wifi已连接，则跳过
        if (wifi_info->connect_flag == true) {
            MLOG_DBG("WiFi %s is connected, skip\n", wifi_info->ssid);
            return;
        }

        // 2. 如果wifi已保存，则连接
        if (wifi_info->save_flag == true) {
            MLOG_DBG("WiFi %s is saved, select it\n", wifi_info->ssid);
            lv_label_set_text(label_notice, str_language_connectings[get_curr_language()]);
            lv_obj_clear_flag(label_notice, LV_OBJ_FLAG_HIDDEN);
            lv_obj_invalidate(label_notice);
            lv_refr_now(lv_disp_get_default());
            int ret = Hal_Wpa_Connect(wifi_info, NULL);
            if (ret != 0) {
                MLOG_ERR("WiFi %s connect failed\n", wifi_info->ssid);
                lv_label_set_text(label_notice, str_language_connection_failed_please_try_again[get_curr_language()]);
            } else {
                lv_label_set_text(label_notice, str_language_connection_successful[get_curr_language()]);
                wifi_refresh_btn_cb(NULL); // 连接成功后刷新列表
            }
            lv_timer_create(hide_label_notice, 1000, NULL);
            return;
        }

        MLOG_DBG("cur_wifi_info->ssid: %s\n", wifi_info->ssid);
        // 3. 如果wifi未保存，则跳转到输入密码页面。
        wifi_scan_timer_stop();
        delete_batter_tips_mbox();
        ui_load_scr_animation(&g_ui, &sysMenu_WifiCode_s, 1, NULL, sysMenu_WifiCode, LV_SCR_LOAD_ANIM_NONE, 0, 0,
            false, true);
    }
}

// 重建 WiFi列表
static void wifi_list_rebuild()
{
    // 重置选中状态
    current_wifi_index = -1;
    current_selected_btn = NULL;

    MLOG_DBG("wifi_list_refresh\n");
    // 清空现有列表
    lv_obj_clean(wifi_list_s);
    // 为每个扫描到的WiFi网络创建按钮
    if (wifi_scan_count > 0) {
        for (int i = 0; i < wifi_scan_count; ++i) {
            // 创建WiFi按钮
            lv_obj_t* btn = lv_list_add_btn(wifi_list_s, LV_SYMBOL_WIFI, wifi_scan_results[i].ssid);
            lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_FULL, LV_PART_MAIN);
            lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
            // 设置字体（SSID文字）
            lv_obj_t* label = lv_obj_get_child(btn, 1);
            lv_obj_add_style(label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

            if (wifi_scan_results[i].save_flag) {
                lv_obj_set_style_text_color(btn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
                lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
            }
            // ====== 添加已连接状态图标 ======
            lv_obj_t* icon_connected = lv_label_create(btn);
            lv_label_set_text(icon_connected, LV_SYMBOL_OK); // 先用wifi符号占位
            lv_obj_align(icon_connected, LV_ALIGN_RIGHT_MID, -40, 0); // 靠右对齐，留出空位给锁图标
            lv_obj_add_flag(icon_connected, LV_OBJ_FLAG_HIDDEN); // 如果已连接，隐藏图标
            if (wifi_scan_results[i].connect_flag) {
                lv_obj_clear_flag(icon_connected, LV_OBJ_FLAG_HIDDEN); // 如果已连接，隐藏图标
            }

            // 添加密码锁图标
            lv_obj_t* icon_lock = lv_label_create(btn);
            lv_label_set_text(icon_lock, "\xEF\x80\xA3");
            lv_obj_set_style_text_font(icon_lock, &import_symbol_lock, LV_PART_MAIN);
            lv_obj_align(icon_lock, LV_ALIGN_RIGHT_MID, -10, 0);
            lv_obj_add_flag(icon_lock, LV_OBJ_FLAG_HIDDEN);
            if (is_wifi_password_protected(&wifi_scan_results[i])) {
                lv_obj_clear_flag(icon_lock, LV_OBJ_FLAG_HIDDEN); // 如果需要密码，显示密码锁图标
            }

            // 绑定点击事件，传递WiFi信息
            lv_obj_add_event_cb(btn, wifi_list_item_clicked_cb, LV_EVENT_CLICKED, (void*)&wifi_scan_results[i]);
        }

        lv_obj_add_flag(label_notice, LV_OBJ_FLAG_HIDDEN);

        // 如果有WiFi列表，默认选中第一个
        if (wifi_scan_count > 0) {
            current_wifi_index = 0;
            lv_obj_t* first_btn = lv_obj_get_child(wifi_list_s, 0);
            if (first_btn != NULL) {
                set_selected_style(first_btn);
            }
        }
    }
}

// #endregion
// #############################################################################
// ! #region 5. 对外接口函数
// #############################################################################

// #endregion
// #############################################################################
// ! #region 6. 线程处理函数
// #############################################################################

// #endregion
// #############################################################################
// ! #region 7. 按键、手势、定时器 等事件回调函数
// #############################################################################

/*
 * @brief: 检查WiFi扫描状态并更新UI
 *         先尝试获取扫描结果（会更新内部状态），根据结果处理
 *         使用异步接口，无需线程
 */
static void wifi_scan_check_status(lv_timer_t* timer)
{
    UNUSED(timer);

    MLOG_DBG("wifi_scan_check_status called\n");

    // 先尝试获取扫描结果（这会更新内部状态）
    int count = WIFI_INFO_MAX;
    int ret = Hal_Wpa_GetScanResultAsync(wifi_scan_results, &count);

    MLOG_DBG("Hal_Wpa_GetScanResultAsync returned: %d, count: %d\n", ret, count);

    if (ret == 0) {
        // 成功获取结果
        wifi_scan_count = count;
        wifi_list_rebuild();
        wifi_scan_timer_stop();
        MLOG_DBG("WiFi scan completed, found %d networks\n", count);
    } else {
        // 获取失败，检查扫描状态
        wifi_scan_state_e state = Hal_Wpa_GetScanState();

        if (state == WIFI_SCAN_STATE_IDLE) {
            // 扫描失败或未开始
            MLOG_DBG("WiFi scan failed or not started\n");
            wifi_scan_count = 0;
            wifi_list_rebuild();
            wifi_scan_timer_stop();
        } else if (state == WIFI_SCAN_STATE_COMPLETE) {
            // 状态显示已完成，但获取结果失败（不应该发生）
            MLOG_ERR("State is COMPLETE but failed to get results\n");
            wifi_scan_count = 0;
            wifi_list_rebuild();
            wifi_scan_timer_stop();
        } else {
            // 扫描仍在进行中，继续等待
            MLOG_DBG("Scan still in progress (state=%d), will check again\n", state);
        }
    }
}

static void hide_label_notice(lv_timer_t* timer)
{
    if (lv_obj_is_valid(label_notice) == false) {
        return;
    }
    lv_label_set_text(label_notice, "");
    lv_obj_add_flag(label_notice, LV_OBJ_FLAG_HIDDEN); // 隐藏通知标签

    lv_timer_del(timer);
}

// WiFi列表项点击事件回调
static void wifi_list_item_clicked_cb(lv_event_t* e)
{
    int ret;
    // 获取用户数据，即WiFi名称
    cur_wifi_info = (wifi_info_t*)lv_event_get_user_data(e);
    // 1. 如果wifi已连接，则跳过
    if (cur_wifi_info->connect_flag == true) {
        MLOG_DBG("WiFi %s is connected, skip\n", cur_wifi_info->ssid);
        return;
    }

    // 2. 如果wifi已保存，或者不需要密码，则连接
    if (cur_wifi_info->save_flag == true || !is_wifi_password_protected(cur_wifi_info)) {
        MLOG_DBG("WiFi %s is saved, select it\n", cur_wifi_info->ssid);
        lv_label_set_text(label_notice, str_language_connectings[get_curr_language()]);
        lv_obj_clear_flag(label_notice, LV_OBJ_FLAG_HIDDEN);
        lv_obj_invalidate(label_notice); // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
        lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
        ret = Hal_Wpa_Connect(cur_wifi_info, NULL);
        if (ret != 0) {
            MLOG_ERR("WiFi %s connect failed\n", cur_wifi_info->ssid);
            lv_label_set_text(label_notice, str_language_connection_failed_please_try_again[get_curr_language()]);
        } else {
            lv_label_set_text(label_notice, str_language_connection_successful[get_curr_language()]);
            wifi_refresh_btn_cb(NULL); // 连接成功后刷新列表
        }
        lv_timer_create(hide_label_notice, 1000, NULL);
        return;
    }

    MLOG_DBG("cur_wifi_info->ssid: %s\n", cur_wifi_info->ssid);
    // 3. 如果wifi未保存，则跳转到输入密码页面。
    wifi_scan_timer_stop();
    delete_batter_tips_mbox();
    ui_load_scr_animation(&g_ui, &sysMenu_WifiCode_s, 1, NULL, sysMenu_WifiCode, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
        true);
}

// 刷新按钮事件回调
static void wifi_refresh_btn_cb(lv_event_t* e)
{
    UNUSED(e);
    MLOG_DBG("手动刷新WiFi列表\n");

    // 立即显示正在扫描的提示
    lv_label_set_text(label_notice, str_language_scanning_wifi[get_curr_language()]);
    lv_obj_clear_flag(label_notice, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_invalidate(label_notice);
    // lv_refr_now(lv_disp_get_default());

    // 启动WiFi扫描
    int ret = Hal_Wpa_Scan();
    if (ret == 0) {
        // 创建定时器检查扫描状态
        if (wifi_scan_timer == NULL) {
            wifi_scan_timer = lv_timer_create(wifi_scan_check_status, 200, NULL);
        } else {
            lv_timer_reset(wifi_scan_timer);
        }
    } else {
        MLOG_ERR("Failed to start WiFi scan\n");
    }
}

// 返回按钮回调函数
static void wifi_back_cb(lv_event_t* e)
{
    UNUSED(e);

    wifi_scan_timer_stop();

    if (wifi_check_get_return_flag() == 1) {
        wifi_return_cb_t cb = wifi_check_get_return_handler();
        wifi_return_cb_t handler = cb;
        void* userdata = wifi_check_get_return_userdata();
        wifi_check_clear_return_handler();
        handler(userdata);
    } else {
        // 返回系统设置页面
        delete_batter_tips_mbox();
        ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
            true);
    }
}

// WiFi开关事件回调
static void wifi_switch_event_cb(lv_event_t* e)
{
    lv_obj_t* sw = lv_event_get_target(e);
    // UNUSED(sw);

    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        MLOG_DBG("打开WiFi\n");
        // 检查电量
        if (!check_battery_for_wifi(sysMenu_WifiList_s)) {
            lv_obj_clear_state(sw, LV_STATE_CHECKED); // 保持开关关闭
            return;
        }

        // 先禁用所有网络，禁止自动连接
        Hal_Wpa_DisableAllNetworks();
        // 再打开WiFi
        Hal_Wpa_Up();
        // 打开，显示str_language_my_network[get_curr_language()]标题和WiFi列表
        lv_label_set_text(label_notice, str_language_scanning_wifi[get_curr_language()]);
        lv_obj_clear_flag(label_notice, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(label_my_network_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(wifi_list_s, LV_OBJ_FLAG_HIDDEN);

        // 启动WiFi扫描
        int ret = Hal_Wpa_Scan();
        if (ret == 0) {
            // 创建定时器检查扫描状态
            if (wifi_scan_timer == NULL) {
                wifi_scan_timer = lv_timer_create(wifi_scan_check_status, 200, NULL);
            } else {
                lv_timer_reset(wifi_scan_timer);
            }
        } else {
            MLOG_ERR("Failed to start WiFi scan\n");
        }
    } else {
        MLOG_DBG("关闭WiFi\n");
        wifi_scan_count = 0;
        Hal_Wpa_Down();
        // 关闭，隐藏str_language_my_network[get_curr_language()]标题和WiFi列表
        lv_obj_add_flag(label_my_network_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(wifi_list_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_notice, LV_OBJ_FLAG_HIDDEN);
    }
}

static void sysmenu_wifilist_key_handler(int key_code, int key_value)
{
    if (key_value == 1) { // 按键按下
        switch (key_code) {
        case KEY_MENU: {
            MLOG_DBG("返回页面\n");
            wifi_scan_timer_stop();
            if (wifi_check_get_return_flag() == 1) {
                wifi_return_cb_t cb = wifi_check_get_return_handler();
                wifi_return_cb_t handler = cb;
                void* userdata = wifi_check_get_return_userdata();
                wifi_check_clear_return_handler();
                handler(userdata);
            } else {
                // 返回系统设置页面
                delete_batter_tips_mbox();
                ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                    false, true);
            }
        } break;

        case KEY_UP:
            MLOG_DBG("向上选择WiFi\n");
            select_prev_wifi();
            break;

        case KEY_DOWN:
            MLOG_DBG("向下选择WiFi\n");
            select_next_wifi();
            break;

        case KEY_OK:
            MLOG_DBG("确认选择WiFi\n");
            confirm_selected_wifi();
            break;

        default:
            break;
        }
    }
}

static void gesture_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch (code) {
    case LV_EVENT_GESTURE: {
        // 获取手势方向，需要 TP 驱动支持
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch (dir) {
        case LV_DIR_RIGHT: {
            MLOG_DBG("向右滑动，返回页面\n");
            wifi_scan_timer_stop();

            if (wifi_check_get_return_flag() == 1) {
                wifi_return_cb_t cb = wifi_check_get_return_handler();
                wifi_return_cb_t handler = cb;
                void* userdata = wifi_check_get_return_userdata();
                wifi_check_clear_return_handler();
                handler(userdata);
            } else {
                // 返回系统设置页面
                delete_batter_tips_mbox();
                ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting,
                    LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

// #endregion
// #############################################################################
// ! #region 8. 初始化、去初始化、资源管理
// #############################################################################

void sysMenu_WifiList(lv_ui_t* ui)
{

    // 创建主页面1 容器
    if (sysMenu_WifiList_s != NULL) {
        if (lv_obj_is_valid(sysMenu_WifiList_s)) {
            MLOG_DBG("page_syswifi->scr 仍然有效，删除旧对象\n");
            lv_obj_del(sysMenu_WifiList_s);
        } else {
            MLOG_DBG("page_syswifi->scr 已被自动销毁，仅重置指针\n");
        }
        sysMenu_WifiList_s = NULL;
    }

    // The custom code of scr.
    sysMenu_WifiList_s = lv_obj_create(NULL);
    lv_obj_set_size( sysMenu_WifiList_s , H_RES, V_RES);
    lv_obj_add_style( sysMenu_WifiList_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(sysMenu_WifiList_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // 1. 顶部返回按钮（左上角，圆形带返回箭头，标签为按钮子控件并居中）
    lv_obj_t* btn_back = lv_btn_create(sysMenu_WifiList_s);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 4, 4);
    lv_obj_set_size(btn_back, 60, 52);
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_back, wifi_back_cb, LV_EVENT_CLICKED, ui);

    lv_obj_t* label_back_btn = lv_label_create(btn_back);
    lv_label_set_text(label_back_btn, LV_SYMBOL_LEFT "");
    lv_obj_add_style(label_back_btn, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_back_btn, LV_ALIGN_CENTER, 0, 0);
    // 2. 顶部居中标题"Wi-Fi"
    lv_obj_t* title = lv_label_create(sysMenu_WifiList_s);
    lv_label_set_text(title, "Wi-Fi");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
        LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(title, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 6);

    // 3. 顶部str_language_wireless_lan[get_curr_language()]+开关栏，居中显示且不显示滑动条
    lv_obj_t* wifi_topbar = lv_obj_create(sysMenu_WifiList_s);
    lv_obj_set_size(wifi_topbar, 400, 56);
    lv_obj_align(wifi_topbar, LV_ALIGN_TOP_MID, 0, 70);
    lv_obj_set_style_radius(wifi_topbar, 28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(wifi_topbar, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(wifi_topbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(wifi_topbar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(wifi_topbar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // str_language_wireless_lan[get_curr_language()]文字
    lv_obj_t* wifi_label = lv_label_create(wifi_topbar);
    lv_label_set_text(wifi_label, str_language_wifi_switch[get_curr_language()]);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(wifi_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(wifi_label, LV_ALIGN_LEFT_MID, 16, 0);

    // 4. WiFi列表及str_language_my_network[get_curr_language()]标题
    // 4.1 str_language_my_network[get_curr_language()]标题
    label_my_network_s = lv_label_create(sysMenu_WifiList_s);
    lv_label_set_text(label_my_network_s, str_language_network[get_curr_language()]);
    lv_obj_add_style(label_my_network_s, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_my_network_s, LV_ALIGN_TOP_LEFT, 48, 150);

    // 4.2 刷新按钮(手动触发 扫描wifi)
    label_refresh = lv_label_create(sysMenu_WifiList_s);
    lv_label_set_text(label_refresh, str_language_refresh[get_curr_language()]);
    lv_obj_add_style(label_refresh, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_refresh, LV_ALIGN_TOP_LEFT, 500, 150);
    lv_obj_add_flag(label_refresh, LV_OBJ_FLAG_CLICKABLE); // 设置为可点击
    lv_obj_add_event_cb(label_refresh, wifi_refresh_btn_cb, LV_EVENT_CLICKED, NULL);

    // 4.3 提示正在扫描WiFi的标签
    label_notice = lv_label_create(sysMenu_WifiList_s);
    lv_label_set_text(label_notice, str_language_scanning_wifi[get_curr_language()]);
    lv_obj_add_style(label_notice, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_notice, LV_ALIGN_TOP_LEFT, 200, 150);

    // 4.2 WiFi列表，居中放置，显示可上下拖动的滑动条
    wifi_list_s = lv_list_create(sysMenu_WifiList_s);
    lv_obj_set_size(wifi_list_s, 480, 280/2 + 20);
    lv_obj_align(wifi_list_s, LV_ALIGN_TOP_MID, 0, 182);
    lv_obj_set_style_bg_color(wifi_list_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(wifi_list_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(wifi_list_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(wifi_list_s, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_add_style(wifi_list_s, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(wifi_list_s, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(wifi_list_s, LV_SCROLLBAR_MODE_AUTO);

    // WiFi开关
    lv_obj_t* wifi_switch = lv_switch_create(wifi_topbar);
    lv_obj_set_size(wifi_switch, 60, 32);
    lv_obj_align(wifi_switch, LV_ALIGN_RIGHT_MID, -16, 0);
    lv_obj_set_style_bg_color(wifi_switch, lv_color_hex(0xFFC107), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_add_event_cb(wifi_switch, wifi_switch_event_cb, LV_EVENT_VALUE_CHANGED, ui);

    // 根据当前状态设置开关和列表显示
    if (Hal_Wifi_Is_Up() == 0) {
        lv_obj_add_state(wifi_switch, LV_STATE_CHECKED);
        lv_obj_clear_flag(label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(label_my_network_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(wifi_list_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(label_notice, LV_OBJ_FLAG_HIDDEN);

        // 启动WiFi扫描（异步）
        int ret = Hal_Wpa_Scan();
        if (ret == 0) {
            // 创建定时器检查扫描状态
            if (wifi_scan_timer == NULL) {
                wifi_scan_timer = lv_timer_create(wifi_scan_check_status, 200, NULL);
            }
        }
    } else {
        lv_obj_clear_state(wifi_switch, LV_STATE_CHECKED);
        lv_obj_add_flag(label_refresh, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_my_network_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(wifi_list_s, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_notice, LV_OBJ_FLAG_HIDDEN);
    }

    // 注册屏幕加载完成事件，滚动WiFi列表到顶部
    // lv_obj_add_event_cb(sysMenu_WifiList_s, screen_SettingsSysWifi_loaded_cb, LV_EVENT_SCREEN_LOADED, ui);

    // 在上方添加一条分割线
    lv_obj_t* up_line = lv_line_create(sysMenu_WifiList_s);
    static lv_point_precise_t points_line[] = { { 10, 60 }, { 640, 60 } };
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);
    set_current_page_handler(sysmenu_wifilist_key_handler);

    // Update current screen layout.
    lv_obj_update_layout(sysMenu_WifiList_s);
}

// #endregion
// #############################################################################
// ! #region 9. 调试与测试
// #############################################################################

// #endregion

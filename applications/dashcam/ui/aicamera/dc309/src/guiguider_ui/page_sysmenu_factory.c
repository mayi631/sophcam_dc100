// #############################################################################
// ! #region 1. 头文件与宏定义
// #############################################################################

// #define DEBUG
#include "config.h"
#include "custom.h"
#include "gui_guider.h"
#include "indev.h"
#include "linux/input.h"
#include "lvgl.h"
#include "page_all.h"
#include "style_common.h"
#include "ui_common.h"
#include <stdio.h>

#define GRID_COLS 1
#define GRID_ROWS 2
#define GRID_MAX_OBJECTS GRID_ROWS* GRID_COLS

// #endregion
// #############################################################################
// ! #region 2. 数据结构定义
// #############################################################################

// #endregion
// #############################################################################
// ! #region 3. 全局变量 &  函数声明
// #############################################################################

static lv_obj_t* focusable_objects[GRID_MAX_OBJECTS];

lv_obj_t* obj_sysMenu_Factory_s; // 底层窗口
lv_obj_t* obj_sysMenu_Factory_Float_s; // 提示浮窗
lv_obj_t* obj_factory_dialog_s; // 提示内容容器

static void sysMenu_Factory_Sure(void);
static void factory_display_tips_anim_complete(lv_anim_t* a); // 显示提示动画
static void sysmenu_factory_click_callback(lv_obj_t* obj);
static void sysMenu_factory_Delete_Complete_anim_cb(lv_anim_t* a);

// #endregion
// #############################################################################
// ! #region 4. 内部工具函数（注意用static修饰）
// #############################################################################

static void sysMenu_Factory_Delete_anim(void)
{
    lv_anim_t Delete_anim; // 动画渐隐句柄
    // 创建透明度动画
    lv_anim_init(&Delete_anim);
    lv_anim_set_values(&Delete_anim, 0, 100);

    lv_anim_set_time(&Delete_anim, 100);

    // lv_anim_set_exec_cb(&Delete_anim, AIanim_objSet_Opa);
    lv_anim_set_path_cb(&Delete_anim, lv_anim_path_ease_out);

    // 设置动画完成回调（销毁对象）
    lv_anim_set_completed_cb(&Delete_anim, sysMenu_factory_Delete_Complete_anim_cb);

    lv_anim_start(&Delete_anim);
}

// 删除浮窗
static void factory_delete_float_win(void)
{
    if (lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
        lv_obj_del(obj_sysMenu_Factory_Float_s);
        obj_sysMenu_Factory_Float_s = NULL;
    }
    if (lv_obj_is_valid(obj_factory_dialog_s)) {
        obj_factory_dialog_s = NULL;
    }

    // 取消浮窗的按键处理器，避免回调继续被调用
    set_current_page_handler(NULL);

    if (!lv_obj_is_valid(obj_sysMenu_Factory_s)) {
        return;
    }

    lv_obj_t* settings_cont = lv_obj_get_child(obj_sysMenu_Factory_s, 1);
    if (settings_cont == NULL) {
        return;
    }
    lv_obj_t* target_obj = lv_obj_get_child(settings_cont, 2);
    if (target_obj == NULL) {
        return;
    }

    // 初始化焦点组
    init_focus_group(settings_cont, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
        sysmenu_factory_click_callback, target_obj);
    set_current_page_handler(handle_grid_navigation);
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

static void sysMenu_factory_Delete_Complete_anim_cb(lv_anim_t* a)
{
    if (lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
        lv_obj_del(obj_sysMenu_Factory_Float_s);
        obj_sysMenu_Factory_Float_s = NULL;
    }
    if (lv_obj_is_valid(obj_factory_dialog_s)) {
        obj_factory_dialog_s = NULL;
    }
    // 取消浮窗的按键处理器
    set_current_page_handler(NULL);
    ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
        true);
}

static void sysMenu_Factory_btn_back_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch (code) {
    case LV_EVENT_CLICKED: {
        if (lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
            lv_obj_del(obj_sysMenu_Factory_Float_s);
            obj_sysMenu_Factory_Float_s = NULL;
        }
        if (lv_obj_is_valid(obj_factory_dialog_s)) {
            obj_factory_dialog_s = NULL;
        }
        // 取消浮窗的按键处理器
        set_current_page_handler(NULL);
        ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
            false, true);
        break;
    }
    default:
        break;
    }
}

static void sysMenu_Factory_OK_Cancel_btn_event_handler(lv_event_t* e)
{

    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED: {
        if (!lv_obj_is_valid(obj_factory_dialog_s)) {
            return;
        }
        lv_obj_t* btn_clicked = lv_event_get_target(e);
        lv_obj_t* parent = lv_obj_get_parent(btn_clicked); // 获取点击事件的父控件
        // lv_obj_t *cont        = lv_event_get_user_data(e);
        for (uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
            if (lv_obj_get_child(parent, i) == btn_clicked) {
                // 隐藏dialog的所有控件
                for (uint8_t j = 0; j < lv_obj_get_child_cnt(obj_factory_dialog_s); j++) {
                    if (!lv_obj_has_flag(lv_obj_get_child(obj_factory_dialog_s, j), LV_OBJ_FLAG_HIDDEN) && j != 2 && i) {
                        lv_obj_add_flag(lv_obj_get_child(obj_factory_dialog_s, j), LV_OBJ_FLAG_HIDDEN);
                    }
                }
                // factory_Current_Index_s = i;
                if (i == 1) {
                    MLOG_DBG("正在恢复出厂设置......\n");
                    MESSAGE_S Msg = { 0 };
                    Msg.topic = EVENT_MODEMNG_SETTING;
                    Msg.arg1 = PARAM_MENU_DEFAULT;
                    int32_t s32Ret = UICOMM_SendSyncMsg(&Msg, 5000); // 5秒超时
                    if (0 != s32Ret) {
                        MLOG_ERR("恢复出厂设置失败，错误码: %d\n", s32Ret);
                        // 显示失败提示
                        lv_label_set_text(lv_obj_get_child(obj_factory_dialog_s, 2), str_language_factory_reset_failed[get_curr_language()]);
                    } else {
                        // 显示成功提示
                        lv_label_set_text(lv_obj_get_child(obj_factory_dialog_s, 2), str_language_factory_reset_successful[get_curr_language()]);
                        // 从参数中更新到 UI 相关的静态变量
                        update_setting_from_param();
                    }
                    lv_obj_add_style(lv_obj_get_child(obj_factory_dialog_s, 2), &ttf_font_34, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_anim_t anim;
                    lv_anim_init(&anim);
                    lv_anim_set_values(&anim, 0, 1);
                    lv_anim_set_time(&anim, 1000);
                    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
                    lv_anim_set_completed_cb(&anim, factory_display_tips_anim_complete);
                    lv_anim_start(&anim);

                } else {
                    factory_delete_float_win();
                    break;
                }
            }
        }

    }; break;
    default:
        break;
    }
}

static void sysMenu_Factory_Select_btn_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED: {
        lv_obj_t* btn_clicked = lv_event_get_target(e);
        lv_obj_t* parent = lv_obj_get_parent(btn_clicked); // 获取点击事件的父控件
        lv_obj_t* cont = lv_event_get_user_data(e);
        for (uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
            if (lv_obj_get_child(parent, i) == cont) {
                if (i == 0) {
                    set_current_page_handler(NULL); // 取消焦点按键控制
                    sysMenu_Factory_Sure();
                } else {
                    sysMenu_Factory_Delete_anim(); // 删除当前页面并进行跳转
                }
            }
        }

    }; break;
    default:
        break;
    }
}

static void sysmenu_factory_click_callback(lv_obj_t* obj)
{
    MLOG_DBG("sysmenu_factory_click_callback\n");
    lv_obj_t* parent = lv_obj_get_parent(obj); // 获取点击事件的父控件
    for (uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
        if (lv_obj_get_child(parent, i) == obj) {
            if (i == 0) {
                sysMenu_Factory_Sure();
            } else // if()
            {
                sysMenu_Factory_Delete_anim(); // 删除当前页面并进行跳转
            }
        }
    }
}

static void sysmenu_factory_menu_callback(void)
{
    MLOG_DBG("sysmenu_factory_menu_callback\n");
    if (lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
        lv_obj_del(obj_sysMenu_Factory_Float_s);
        obj_sysMenu_Factory_Float_s = NULL;
    }
    if (lv_obj_is_valid(obj_factory_dialog_s)) {
        obj_factory_dialog_s = NULL;
    }
    // 取消浮窗的按键处理器
    set_current_page_handler(NULL);
    ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
        false, true);
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
            if (lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
                lv_obj_del(obj_sysMenu_Factory_Float_s);
                obj_sysMenu_Factory_Float_s = NULL;
            }
            if (lv_obj_is_valid(obj_factory_dialog_s)) {
                obj_factory_dialog_s = NULL;
            }
            // 取消浮窗的按键处理器
            set_current_page_handler(NULL);
            ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting,
                LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
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

void factory_display_tips_anim_complete(lv_anim_t* a)
{
    /* 动画完成回调：删除浮窗并停止动画 */
    if (lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
        factory_delete_float_win();
    }
    lv_anim_del(a, a->exec_cb);
}

void factory_key_cb(int key_code, int key_value)
{
    static uint8_t index = 0;
    if (key_code == KEY_MENU && key_value == 1) {
        factory_delete_float_win(); // 删除当前页面并进行跳转
    } else if (key_code == KEY_LEFT && key_value == 1) {
        if (!lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
            return;
        }
        index = 0;
        lv_obj_t* child = lv_obj_get_child(obj_sysMenu_Factory_Float_s, 0);
        if (child == NULL)
            return;
        child = lv_obj_get_child(child, 3);
        if (child == NULL)
            return;
        lv_obj_t* btn_cancel = lv_obj_get_child(child, 0);
        if (btn_cancel == NULL)
            return;
        lv_obj_set_style_border_color(btn_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(btn_cancel, 2, LV_PART_MAIN);

        lv_obj_t* btn_ok = lv_obj_get_child(child, 1);
        if (btn_ok == NULL)
            return;
        lv_obj_set_style_border_width(btn_ok, 0, LV_PART_MAIN);
    } else if (key_code == KEY_RIGHT && key_value == 1) {
        if (!lv_obj_is_valid(obj_sysMenu_Factory_Float_s)) {
            return;
        }
        index = 1;
        lv_obj_t* child = lv_obj_get_child(obj_sysMenu_Factory_Float_s, 0);
        if (child == NULL)
            return;
        child = lv_obj_get_child(child, 3);
        if (child == NULL)
            return;
        lv_obj_t* btn_cancel = lv_obj_get_child(child, 0);
        if (btn_cancel == NULL)
            return;
        lv_obj_set_style_border_width(btn_cancel, 0, LV_PART_MAIN);

        lv_obj_t* btn_ok = lv_obj_get_child(child, 1);
        if (btn_ok == NULL)
            return;
        lv_obj_set_style_border_color(btn_ok, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_border_width(btn_ok, 2, LV_PART_MAIN);

    } else if (key_code == KEY_OK && key_value == 1) {
        if (!lv_obj_is_valid(obj_factory_dialog_s)) {
            return;
        }

        for (uint8_t j = 0; j < lv_obj_get_child_cnt(obj_factory_dialog_s); j++) {
            if (!lv_obj_has_flag(lv_obj_get_child(obj_factory_dialog_s, j), LV_OBJ_FLAG_HIDDEN) && j != 2 && index) {
                lv_obj_add_flag(lv_obj_get_child(obj_factory_dialog_s, j), LV_OBJ_FLAG_HIDDEN);
            }
        }
        MLOG_DBG("选择%d\n", index);
        if (index == 1) {
            MLOG_DBG("正在恢复出厂设置......\n");
            MESSAGE_S Msg = { 0 };
            Msg.topic = EVENT_MODEMNG_SETTING;
            Msg.arg1 = PARAM_MENU_DEFAULT;
            int32_t s32Ret = UICOMM_SendSyncMsg(&Msg, 5000); // 5秒超时
            if (0 != s32Ret) {
                MLOG_ERR("恢复出厂设置失败，错误码: %d\n", s32Ret);
                lv_label_set_text(lv_obj_get_child(obj_factory_dialog_s, 2), str_language_factory_reset_failed[get_curr_language()]);
            } else {
                update_setting_from_param();
                lv_label_set_text(lv_obj_get_child(obj_factory_dialog_s, 2), str_language_factory_reset_successful[get_curr_language()]);
            }
            lv_obj_add_style(lv_obj_get_child(obj_factory_dialog_s, 2), &ttf_font_34, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_anim_t anim;
            lv_anim_init(&anim);
            lv_anim_set_values(&anim, 0, 1);
            lv_anim_set_time(&anim, 1000);
            lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
            lv_anim_set_completed_cb(&anim, factory_display_tips_anim_complete);
            lv_anim_start(&anim);

        } else {
            factory_delete_float_win(); // 删除当前页面并进行跳转
        }
    }
}

// #endregion
// #############################################################################
// ! #region 8. 初始化、去初始化、资源管理
// #############################################################################

static void sysMenu_Factory_Sure(void)
{
    // 创建浮层背景
    obj_sysMenu_Factory_Float_s = lv_obj_create(obj_sysMenu_Factory_s);
    lv_obj_remove_style_all(obj_sysMenu_Factory_Float_s);
    lv_obj_set_size( obj_sysMenu_Factory_Float_s , H_RES, V_RES);
    lv_obj_add_style( obj_sysMenu_Factory_Float_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj_sysMenu_Factory_Float_s, LV_OPA_70, LV_PART_MAIN);
    lv_obj_align(obj_sysMenu_Factory_Float_s, LV_ALIGN_DEFAULT, 0, 0);

    // 创建对话框容器
    obj_factory_dialog_s = lv_obj_create(obj_sysMenu_Factory_Float_s);
    lv_obj_remove_style_all(obj_factory_dialog_s);
    lv_obj_set_size(obj_factory_dialog_s, 500, 300);
    lv_obj_align(obj_factory_dialog_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(obj_factory_dialog_s, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj_factory_dialog_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj_factory_dialog_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj_factory_dialog_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(obj_factory_dialog_s, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(obj_factory_dialog_s, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(obj_factory_dialog_s, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(obj_factory_dialog_s, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(obj_factory_dialog_s, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加警告图标
    lv_obj_t* icon = lv_label_create(obj_factory_dialog_s);
    lv_label_set_text(icon, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 20);

    // 添加标题
    lv_obj_t* title = lv_label_create(obj_factory_dialog_s);
    lv_label_set_text(title, str_language_factory_settings[get_curr_language()]);
    lv_obj_add_style(title, &ttf_font_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 80);

    // 添加说明文本
    lv_obj_t* explain = lv_label_create(obj_factory_dialog_s);
    // 使用多语言字符串数组
    int lang_index = get_curr_language();
    char explain_text[200];
    snprintf(explain_text, sizeof(explain_text), "%s\n%s",
        str_language_are_you_sure_to_reset_to_factory_settings[lang_index],
        str_language_data_cannot_be_recovered_after_factory_reset[lang_index]);

    lv_label_set_text(explain, explain_text);
    lv_obj_add_style(explain, &ttf_font_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(explain, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(explain, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_height(explain, 120);
    lv_obj_set_width(explain, 450);
    lv_obj_align(explain, LV_ALIGN_TOP_MID, 0, 120);

    // 创建按钮容器
    lv_obj_t* btn_cont = lv_obj_create(obj_factory_dialog_s);
    lv_obj_remove_style_all(btn_cont);
    lv_obj_set_size(btn_cont, 400, 60);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 创建取消按钮
    lv_obj_t* btn_cancel = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_cancel, 160, 50);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 20, -6);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x505050), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_cancel, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_cancel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, str_language_cancel[get_curr_language()]);
    lv_obj_add_style(label_cancel, &ttf_font_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_cancel);

    lv_obj_add_event_cb(btn_cancel, sysMenu_Factory_OK_Cancel_btn_event_handler, LV_EVENT_CLICKED, NULL);

    // 创建确定按钮
    lv_obj_t* btn_ok = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_ok, 160, 50);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_RIGHT, -20, -6);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xE53935), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xC62828), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_ok, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label_ok = lv_label_create(btn_ok);
    lv_label_set_text(label_ok, str_language_confirm[get_curr_language()]);
    lv_obj_add_style(label_ok, &ttf_font_22, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_ok, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_ok);

    lv_obj_add_event_cb(btn_ok, sysMenu_Factory_OK_Cancel_btn_event_handler, LV_EVENT_CLICKED, NULL);
    set_current_page_handler(factory_key_cb);
}

void sysMenu_Factory(lv_ui_t* ui)
{

    // 创建主页面1 容器
    if (obj_sysMenu_Factory_s != NULL) {
        if (lv_obj_is_valid(obj_sysMenu_Factory_s)) {
            MLOG_DBG("obj_sysMenu_Factory_s 仍然有效，删除旧对象\n");
            lv_obj_del(obj_sysMenu_Factory_s);
        } else {
            MLOG_DBG("obj_sysMenu_Factory_s 已被自动销毁，仅重置指针\n");
        }
        obj_sysMenu_Factory_s = NULL;
    }

    // Write codes resscr
    obj_sysMenu_Factory_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_sysMenu_Factory_s, 640, 480);
    lv_obj_set_scrollbar_mode(obj_sysMenu_Factory_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(obj_sysMenu_Factory_s, gesture_event_handler, LV_EVENT_GESTURE, ui);
    // Write style for resscr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_add_style(obj_sysMenu_Factory_s, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_top (顶部栏)
    lv_obj_t* cont_top = lv_obj_create(obj_sysMenu_Factory_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    lv_obj_t* btn_back = lv_button_create(cont_top);
    lv_obj_set_pos(btn_back, 4, 4);
    lv_obj_set_size(btn_back, 60, 52);
    lv_obj_add_event_cb(btn_back, sysMenu_Factory_btn_back_event_handler, LV_EVENT_CLICKED, NULL);
    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(label_back, LV_PCT(100));
    lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes title (标题)
    lv_obj_t* title = lv_label_create(cont_top);
    lv_label_set_text(title, str_language_factory_reset[get_curr_language()]);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(title, &ttf_font_34, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    // 创建设置选项容器
    lv_obj_t* settings_cont = lv_obj_create(obj_sysMenu_Factory_s);
    lv_obj_set_size(settings_cont, 600, MENU_CONT_SIZE);
    lv_obj_align(settings_cont, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_bg_opa(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_flex_flow(settings_cont, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(settings_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(settings_cont, 10, 0);

    // 创建分辨率选项按钮
    const char* btn_labels[] = { str_language_confirm[get_curr_language()], str_language_cancel[get_curr_language()] };
    static lv_point_precise_t line_points_pool[sizeof(btn_labels) / sizeof(btn_labels[0])][2];

    for (int i = 0; i < 2; i++) {
        lv_obj_t* btn = lv_button_create(settings_cont);
        if (!btn)
            continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
        lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN);

        lv_obj_t* label = lv_label_create(btn);
        if (!label)
            continue; // 如果标签创建失败则跳过

        lv_label_set_text(label, btn_labels[i]);
        lv_obj_add_style(label, &ttf_font_34, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        // 添加事件处理器，传入容器对象作为用户数据
        lv_obj_add_event_cb(btn, sysMenu_Factory_Select_btn_event_handler, LV_EVENT_ALL, btn);

        if (i == 1) {
            {
                lv_obj_t* label1 = lv_label_create(btn);
                lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
            }
        }

        lv_obj_t* line = lv_line_create(settings_cont);
        int y_position = (MENU_BTN_SIZE + 10) * (i + 1) - 4; // 计算y坐标  //横线在下方,且第一个btn不用画线
        // 使用点数组池中的第i组
        line_points_pool[i][0].x = 10;
        line_points_pool[i][0].y = y_position;
        line_points_pool[i][1].x = 570;
        line_points_pool[i][1].y = y_position;
        lv_line_set_points(line, line_points_pool[i], 2);
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, lv_color_hex(0x5F5F5F), 0);
    }

    // 先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
    // 获取焦点控件
    lv_obj_t* chlid = lv_obj_get_child(settings_cont, 2);
    lv_group_focus_obj(chlid);
    lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
    // //设置焦点渐变
    // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
    // //设置焦点BG
    // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置焦点标签颜色
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_color(lv_obj_get_child(chlid,1), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t* up_line = lv_line_create(obj_sysMenu_Factory_s);
    static lv_point_precise_t points_line[] = { { 10, 60 }, { 640, 60 } };
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t* target_obj = lv_obj_get_child(settings_cont, 2);
    // 初始化焦点组
    init_focus_group(settings_cont, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS, sysmenu_factory_click_callback, target_obj);
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(sysmenu_factory_menu_callback);

    // Update current screen layout.
    lv_obj_update_layout(obj_sysMenu_Factory_s);
}

// #endregion
// #############################################################################
// ! #region 9. 调试与测试
// #############################################################################

// #endregion

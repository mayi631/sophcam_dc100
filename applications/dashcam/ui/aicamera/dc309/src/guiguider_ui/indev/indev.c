#define DEBUG
#include <stdio.h>
#include <stdbool.h>
#include "lvgl.h"
#include "indev.h"
#include <linux/input.h>
#include "mlog.h"
#include "page_all.h"
#include "common/takephoto.h"
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <linux/i2c.h>
#include <math.h>

/**
 * @brief 当前页面的按键处理回调函数指针
 *
 * 全局变量，指向当前活动页面的按键处理函数。
 * 当硬件按键事件发生时，会调用此函数指针指向的处理函数。
 * 初始值为NULL，表示没有设置按键处理函数。
 */
page_key_handler_t current_page_key_handler = NULL;

menu_callback_t g_menu_callback           = NULL;
mode_callback_t g_mode_callback           = NULL;
ok_callback_t g_ok_callback           = NULL;
long_menu_callback_t g_long_menu_callback = NULL;
long_menu_callback_t g_long_mode_callback = NULL;
play_callback_t g_play_callback           = NULL;
zoomin_callback_t g_zoomin_callback       = NULL;
zoomout_callback_t g_zoomout_callback     = NULL;
zoomout_callback_t g_down_callback        = NULL;
zoomout_callback_t g_up_callback          = NULL;
zoomout_callback_t g_left_callback        = NULL;
zoomout_callback_t g_right_callback       = NULL;
power_callback_t g_power_callback         = NULL;
// 长按菜单按键检测相关变量
lv_timer_t *menu_long_press_timer = NULL;
bool menu_long_press_triggered = false;

// 长按模式按键检测相关变量
lv_timer_t *mode_long_press_timer = NULL;
bool mode_long_press_triggered = false;

// 长按zoomin按键检测相关变量
lv_timer_t *zoomin_long_press_timer = NULL;
bool zoomin_long_press_triggered = false;

// 长按zoomout按键检测相关变量
lv_timer_t *zoomout_long_press_timer = NULL;
bool zoomout_long_press_triggered = false;

/**
 * @brief 设置当前页面的按键处理回调函数
 *
 * 该函数用于设置当前活动页面的按键处理函数。
 * 当页面切换时，需要调用此函数来更新按键处理回调。
 *
 * @param handler 按键处理回调函数指针，NULL表示禁用按键处理
 * @return void
 */
void set_current_page_handler(page_key_handler_t handler)
{
    current_page_key_handler = handler;
}

/**
 * @brief 创建导航焦点组
 *
 * 创建一个支持2D网格导航的焦点组
 *
 * @param parent 父容器对象
 * @param grid_cols 网格列数
 * @param grid_rows 网格行数
 * @param focusable_objects 可聚焦对象数组
 * @param max_objects 最大对象数量
 * @param initial_focus_obj 初始焦点对象，NULL表示聚焦到第一个对象
 * @return lv_group_t* 创建的焦点组，失败返回NULL
 */
static lv_group_t* create_navigation_group(lv_obj_t *parent, int grid_cols, int grid_rows,
                                   lv_obj_t **focusable_objects, int max_objects,
                                   lv_obj_t *initial_focus_obj)
{
    if (!parent || !focusable_objects || max_objects <= 0) {
        return NULL;
    }

    lv_group_t *focus_group = lv_group_create();
    if (!focus_group) {
        return NULL;
    }

    // 清空焦点对象数组
    for (int i = 0; i < max_objects; i++) {
        focusable_objects[i] = NULL;
    }

    // 获取所有可点击的子对象并添加到焦点组
    uint32_t child_count = lv_obj_get_child_cnt(parent);
    int focusable_count = 0;

    for (uint32_t i = 0; i < child_count && focusable_count < max_objects; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        if (child && lv_obj_has_flag(child, LV_OBJ_FLAG_CLICKABLE)) {
            focusable_objects[focusable_count] = child;
            lv_group_add_obj(focus_group, child);
            focusable_count++;
        }
    }

    // 设置初始焦点到指定对象或第一个对象
    if (focusable_count > 0) {
        lv_obj_t *target_obj = initial_focus_obj;

        // 如果指定了初始焦点对象，验证它是否在焦点组中
        if (target_obj) {
            bool found = false;
            for (int i = 0; i < focusable_count; i++) {
                if (focusable_objects[i] == target_obj) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                target_obj = focusable_objects[0]; // 如果指定的对象不在组中，使用第一个对象
            }
        } else {
            target_obj = focusable_objects[0]; // 没有指定时使用第一个对象
        }

        lv_group_focus_obj(target_obj);
    }

    return focus_group;
}

// 全局导航状态变量
static lv_group_t *g_focus_group = NULL;
static lv_obj_t **g_focusable_objects = NULL;
static int g_grid_cols = 0;
static int g_grid_rows = 0;
static int g_current_row = 0;
static int g_current_col = 0;
static int g_current_focus_index = 0;
static void (*g_click_callback)(lv_obj_t *obj) = NULL;

/**
 * @brief 初始化导航状态
 *
 * @param focus_group 焦点组
 * @param focusable_objects 可聚焦对象数组
 * @param grid_cols 网格列数
 * @param grid_rows 网格行数
 * @param click_callback 点击回调函数
 * @param initial_focus_obj 初始焦点对象
 */
static void init_navigation_state(lv_group_t *focus_group, lv_obj_t **focusable_objects,
                          int grid_cols, int grid_rows, void (*click_callback)(lv_obj_t *obj),
                          lv_obj_t *initial_focus_obj)
{
    g_focus_group = focus_group;
    g_focusable_objects = focusable_objects;
    g_grid_cols = grid_cols;
    g_grid_rows = grid_rows;
    g_click_callback = click_callback;

    // 根据初始焦点对象计算初始位置
    if (initial_focus_obj) {
        // 查找初始焦点对象在数组中的索引
        for (int i = 0; i < grid_cols * grid_rows; i++) {
            if (g_focusable_objects[i] == initial_focus_obj) {
                g_current_focus_index = i;
                g_current_row = i / grid_cols;
                g_current_col = i % grid_cols;
                break;
            }
        }
    } else {
        // 默认聚焦到第一个对象
        g_current_row = 0;
        g_current_col = 0;
        g_current_focus_index = 0;
    }
}

/**
 * @brief 通用焦点组初始化函数
 *
 * 整合了焦点组创建和导航状态初始化的完整流程。
 * 这是页面初始化的推荐接口。
 *
 * @param parent 父容器对象
 * @param grid_cols 网格列数
 * @param grid_rows 网格行数
 * @param focusable_objects 可聚焦对象数组
 * @param max_objects 最大对象数量
 * @param click_callback 点击回调函数
 * @param initial_focus_obj 初始焦点对象，NULL表示聚焦到第一个对象
 * @return lv_group_t* 创建的焦点组，失败返回NULL
 */
lv_group_t* init_focus_group(lv_obj_t *parent, int grid_cols, int grid_rows,
                            lv_obj_t **focusable_objects, int max_objects,
                            void (*click_callback)(lv_obj_t *obj),
                            lv_obj_t *initial_focus_obj)
{
    // 创建导航焦点组
    lv_group_t *focus_group = create_navigation_group(parent, grid_cols, grid_rows,
                                                    focusable_objects, max_objects, initial_focus_obj);

    if (focus_group) {
        // 初始化导航状态
        init_navigation_state(focus_group, focusable_objects, grid_cols, grid_rows, click_callback, initial_focus_obj);
        for (int i = 0; i < max_objects; i++) {
            if (focusable_objects[i] != NULL) {
                lv_obj_add_style(focusable_objects[i], &style_focus_blue, LV_STATE_FOCUS_KEY);
            }
        }
    }

    return focus_group;
}

// 页面清理函数 - 在页面关闭时调用
static void cleanup_focus_group(void)
{
    if (g_focus_group) {
        lv_group_del(g_focus_group);
        g_focus_group = NULL;
    }
    set_current_page_handler(NULL);
}

// 设置当前焦点的index
void set_focus_index(int index)
{
    g_current_focus_index = index;
}

/**
 * @brief 通用2D网格导航按键处理函数
 *
 * 提供通用的2D网格导航功能，支持上下左右移动和确认操作。
 * 使用全局状态变量，只需要传入按键参数。
 *
 * @param key_code 按键代码
 * @param key_value 按键值
 */
void handle_grid_navigation(int key_code, int key_value)
{
    if (key_value != 1 || !g_focus_group || !g_focusable_objects) {
        return;
    }

    switch (key_code) {
        case KEY_UP: // KEY_LEFT
            if (g_current_col > 0) {
                // 在当前行内向左移动
                g_current_col--;
                g_current_focus_index = g_current_row * g_grid_cols + g_current_col;
                if (g_focusable_objects[g_current_focus_index]) {
                    lv_group_focus_obj(g_focusable_objects[g_current_focus_index]);
                }
            } else if (g_current_row > 0) {
                // 到达最左列时，移动到上一行的最后一列
                g_current_row--;
                g_current_col = g_grid_cols - 1;
                g_current_focus_index = g_current_row * g_grid_cols + g_current_col;
                if (g_focusable_objects[g_current_focus_index]) {
                    lv_group_focus_obj(g_focusable_objects[g_current_focus_index]);
                }
            }
            break;
        case KEY_DOWN: // KEY_RIGHT
            if (g_current_col < g_grid_cols - 1) {
                // 在当前行内向右移动
                g_current_col++;
                g_current_focus_index = g_current_row * g_grid_cols + g_current_col;
                if (g_focusable_objects[g_current_focus_index]) {
                    lv_group_focus_obj(g_focusable_objects[g_current_focus_index]);
                }
            } else if (g_current_row < g_grid_rows - 1) {
                // 到达最右列时，移动到下一行的第一列
                g_current_row++;
                g_current_col = 0;
                g_current_focus_index = g_current_row * g_grid_cols + g_current_col;
                if (g_focusable_objects[g_current_focus_index]) {
                    lv_group_focus_obj(g_focusable_objects[g_current_focus_index]);
                }
            }
            break;
        case KEY_OK: // KEY_OK
            if(lv_scr_act() == obj_home_s && get_exit_completed() != true) return;
            cleanup_focus_group();
            if (g_focusable_objects[g_current_focus_index]) {
                if (g_click_callback) {
                    g_click_callback(g_focusable_objects[g_current_focus_index]);
                }
            }
            break;
        case KEY_POWER: // KEY_MENU
            if (g_menu_callback) {
                cleanup_focus_group();
                g_menu_callback();
            }
            break;
        case KEY_PLAY: // KEY_MENU
            if(g_play_callback) {
                cleanup_focus_group();
                g_play_callback();
            }
            break;
    }
}

// 长按菜单按键检测定时器回调函数
void menu_long_press_timer_cb(lv_timer_t *t)
{
    // 定时器触发说明按键已经持续按下800ms，执行长按逻辑
    menu_long_press_triggered = true;
    MLOG_DBG("检测到长按菜单按键\n");

    // 执行长按菜单按键处理逻辑
    if (g_long_menu_callback != NULL) {
        MLOG_DBG("执行自定义长按菜单处理逻辑\n");
        g_long_menu_callback();
    }

    // 删除定时器
    lv_timer_del(t);
    menu_long_press_timer = NULL;
}

// 长按模式按键检测定时器回调函数
void mode_long_press_timer_cb(lv_timer_t *t)
{
    // 定时器触发说明按键已经持续按下800ms，执行长按逻辑
    mode_long_press_triggered = true;
    MLOG_DBG("检测到长按模式按键\n");

    // 执行长按菜单按键处理逻辑
    if (g_long_mode_callback != NULL) {
        MLOG_DBG("执行自定义长按菜单处理逻辑\n");
        g_long_mode_callback();
    }

    // 删除定时器
    lv_timer_del(t);
    mode_long_press_timer = NULL;
}

// 注册菜单按键处理回调函数
void takephoto_register_menu_callback(menu_callback_t callback)
{
    g_menu_callback = callback;
    MLOG_DBG("注册菜单按键处理回调函数\n");
}

// 取消注册菜单按键处理回调函数
void takephoto_unregister_menu_callback(void)
{
    g_menu_callback = NULL;
    MLOG_DBG("取消注册菜单按键处理回调函数\n");
}

// 注册模式切换按键处理回调函数
void takephoto_register_mode_callback(mode_callback_t callback)
{
    g_mode_callback = callback;
    MLOG_DBG("注册模式切换按键处理回调函数\n");
}

// 取消注册模式切换按键处理回调函数
void takephoto_unregister_mode_callback(void)
{
    g_mode_callback = NULL;
    MLOG_DBG("取消注册模式切换按键处理回调函数\n");
}

// 注册ok按键处理回调函数
void takephoto_register_ok_callback(ok_callback_t callback)
{
    g_ok_callback = callback;
    MLOG_DBG("注册ok按键处理回调函数\n");
}

// 取消注册ok按键处理回调函数
void takephoto_unregister_ok_callback(void)
{
    g_ok_callback = NULL;
    MLOG_DBG("取消注册ok按键处理回调函数\n");
}

// 注册长按菜单按键处理回调函数
void takephoto_register_long_menu_callback(long_menu_callback_t callback)
{
    g_long_menu_callback = callback;
    MLOG_DBG("注册长按菜单按键处理回调函数\n");
}

// 取消注册长按菜单按键处理回调函数
void takephoto_unregister_long_menu_callback(void)
{
    g_long_menu_callback = NULL;
    MLOG_DBG("取消注册长按菜单按键处理回调函数\n");
}

// 注册长按模式按键处理回调函数
void takephoto_register_long_mode_callback(long_menu_callback_t callback)
{
    g_long_mode_callback = callback;
    MLOG_DBG("注册长按模式按键处理回调函数\n");
}

// 取消注册长按模式按键处理回调函数
void takephoto_unregister_long_mode_callback(void)
{
    g_long_mode_callback = NULL;
    MLOG_DBG("取消注册长按模式按键处理回调函数\n");
}


// 注册播放按键处理回调函数
void takephoto_register_play_callback(play_callback_t callback)
{
    g_play_callback = callback;
    MLOG_DBG("注册播放按键处理回调函数\n");
}

// 取消注册播放按键处理回调函数
void takephoto_unregister_play_callback(void)
{
    g_play_callback = NULL;
    MLOG_DBG("取消注册播放按键处理回调函数\n");
}
// 注册T按键处理回调函数
void takephoto_register_zoomin_callback(play_callback_t callback)
{
    g_zoomin_callback = callback;
}

// 取消注册T按键处理回调函数
void takephoto_unregister_zoomin_callback(void)
{
    g_zoomin_callback = NULL;
}

// 注册W按键处理回调函数
void takephoto_register_zoomout_callback(play_callback_t callback)
{
    g_zoomout_callback = callback;
}

// 取消注册w按键处理回调函数
void takephoto_unregister_zoomout_callback(void)
{
    g_zoomout_callback = NULL;
}

// 注册W按键处理回调函数
void takephoto_register_down_callback(play_callback_t callback)
{
    MLOG_DBG("注册Down按键处理回调函数\n");
    g_down_callback = callback;
}

// 取消注册w按键处理回调函数
void takephoto_unregister_down_callback(void)
{
    MLOG_DBG("取消注册Down按键处理回调函数\n");
    g_down_callback = NULL;
}

// 注册UP按键处理回调函数
void takephoto_register_up_callback(play_callback_t callback)
{
    MLOG_DBG("注册Down按键处理回调函数\n");
    g_up_callback = callback;
}

// 取消注册UP按键处理回调函数
void takephoto_unregister_up_callback(void)
{
    MLOG_DBG("取消注册Down按键处理回调函数\n");
    g_up_callback = NULL;
}

// 注册LEFT按键处理回调函数
void takephoto_register_left_callback(play_callback_t callback)
{
    MLOG_DBG("注册Down按键处理回调函数\n");
    g_left_callback = callback;
}

// 取消注册left按键处理回调函数
void takephoto_unregister_left_callback(void)
{
    MLOG_DBG("取消注册Down按键处理回调函数\n");
    g_left_callback = NULL;
}
// 注册right按键处理回调函数
void takephoto_register_right_callback(play_callback_t callback)
{
    MLOG_DBG("注册Down按键处理回调函数\n");
    g_right_callback = callback;
}

// 取消注册UP按键处理回调函数
void takephoto_unregister_right_callback(void)
{
    MLOG_DBG("取消注册Down按键处理回调函数\n");
    g_right_callback = NULL;
}

// 注册right按键处理回调函数
void takephoto_power_callback(play_callback_t callback)
{
    g_power_callback = callback;
}

// 取消注册UP按键处理回调函数
void takephoto_unregister_power_callback(void)
{
    g_power_callback = NULL;
}


// 取消注册所有的按键回调
void takephoto_unregister_all_callback(void)
{
    // 取消注册ok按键处理回调函数
    takephoto_unregister_ok_callback();
    // 取消注册模式切换按键处理回调函数
    takephoto_unregister_mode_callback();
    // 取消注册菜单按键处理回调函数
    takephoto_unregister_menu_callback();
    // 取消注册长按菜单按键处理回调函数
    takephoto_unregister_long_menu_callback();
    // 取消注册长按模式按键处理回调函数
    takephoto_unregister_long_mode_callback();
    // 取消注册播放按键处理回调函数
    takephoto_unregister_play_callback();
    // 取消注册T按键处理回调函数
    takephoto_unregister_zoomin_callback();
    // 取消注册w按键处理回调函数
    takephoto_unregister_zoomout_callback();
    takephoto_unregister_down_callback();
    takephoto_unregister_up_callback();
    // 取消注册left按键处理回调函数
    takephoto_unregister_left_callback();
    // 取消RIGHT按键处理回调函数
    takephoto_unregister_right_callback();
    // 取消注册拍照前/后处理回调函数
    takephoto_unregister_callback();
    takephoto_unregister_before_callback();
}

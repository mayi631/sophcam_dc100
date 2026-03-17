#ifndef __INDEV_H__
#define __INDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef enum {
    BUTTON_TYPE_NONE = 0,
    BUTTON_TYPE_FOCUS = 1,
    BUTTON_TYPE_PHOTO = 2,
    BUTTON_TYPE_AI = 3,
} button_type_t;

/**
 * @brief 页面按键处理回调函数类型
 *
 * 定义页面按键处理函数的签名，用于处理硬件按键事件。
 *
 * @param key_code 按键代码，如 KEY_CAMERA_FOCUS, KEY_CAMERA 等
 * @param key_value 按键值，1表示按下，0表示释放
 */
typedef void (*page_key_handler_t)(int key_code, int key_value);

// 菜单按键处理回调函数类型定义
typedef void (*menu_callback_t)(void);

// 模式切换按键处理回调函数类型定义
typedef void (*mode_callback_t)(void);

// ok按键处理回调函数类型定义
typedef void (*ok_callback_t)(void);

// 长按菜单按键处理回调函数类型定义
typedef void (*long_menu_callback_t)(void);

// 播放按键处理回调函数类型定义
typedef void (*play_callback_t)(void);

// TW按键处理回调函数类型定义
typedef void (*zoomin_callback_t)(void);
typedef void (*zoomout_callback_t)(void);
typedef void (*power_callback_t)(void);

/**
 * @brief 当前页面的按键处理回调函数指针
 *
 * 全局变量，指向当前活动页面的按键处理函数。
 * 当硬件按键事件发生时，会调用此函数指针指向的处理函数。
 */
extern page_key_handler_t current_page_key_handler;

/**
 * @brief 设置当前页面的按键处理回调函数
 *
 * 该函数用于设置当前活动页面的按键处理函数。
 * 当页面切换时，需要调用此函数来更新按键处理回调。
 *
 * @param handler 按键处理回调函数指针，NULL表示禁用按键处理
 * @return void
 */
void set_current_page_handler(page_key_handler_t handler);

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
                            lv_obj_t *initial_focus_obj);

void set_focus_index(int index);
/**
 * @brief 通用2D网格导航按键处理函数
 *
 * 提供通用的2D网格导航功能，支持上下左右移动和确认操作。
 * 使用全局状态变量，只需要传入按键参数。
 *
 * @param key_code 按键代码
 * @param key_value 按键值
 */
void handle_grid_navigation(int key_code, int key_value);

// 注册ok按键处理回调函数
void takephoto_register_ok_callback(ok_callback_t callback);

// 取消注册ok按键处理回调函数
void takephoto_unregister_ok_callback(void);

// 注册模式切换按键处理回调函数
void takephoto_register_mode_callback(mode_callback_t callback);

// 取消注册模式切换按键处理回调函数
void takephoto_unregister_mode_callback(void);

// 注册菜单按键处理回调函数
void takephoto_register_menu_callback(menu_callback_t callback);

// 取消注册菜单按键处理回调函数
void takephoto_unregister_menu_callback(void);

// 注册长按菜单按键处理回调函数
void takephoto_register_long_menu_callback(long_menu_callback_t callback);

// 取消注册长按菜单按键处理回调函数
void takephoto_unregister_long_menu_callback(void);

// 注册长按模式按键处理回调函数
void takephoto_register_long_mode_callback(long_menu_callback_t callback);

// 取消注册长按模式按键处理回调函数
void takephoto_unregister_long_mode_callback(void);

// 注册播放按键处理回调函数
void takephoto_register_play_callback(play_callback_t callback);

// 取消注册播放按键处理回调函数
void takephoto_unregister_play_callback(void);

// 长按菜单按键检测定时器回调函数
void menu_long_press_timer_cb(lv_timer_t *t);
// 长按模式按键检测定时器回调函数
void mode_long_press_timer_cb(lv_timer_t *t);
// 注册T按键处理回调函数
void takephoto_register_zoomin_callback(play_callback_t callback);
// 取消注册T按键处理回调函数
void takephoto_unregister_zoomin_callback(void);
// 注册W按键处理回调函数
void takephoto_register_zoomout_callback(play_callback_t callback);
// 取消注册w按键处理回调函数
void takephoto_unregister_zoomout_callback(void);

void takephoto_register_down_callback(play_callback_t callback);
void takephoto_unregister_down_callback(void);

void takephoto_register_up_callback(play_callback_t callback);
void takephoto_unregister_up_callback(void);
// 注册LEFT按键处理回调函数
void takephoto_register_left_callback(play_callback_t callback);
// 取消注册left按键处理回调函数
void takephoto_unregister_left_callback(void);
// 注册right按键处理回调函数
void takephoto_register_right_callback(play_callback_t callback);
// 取消RIGHT按键处理回调函数
void takephoto_unregister_right_callback(void);

void takephoto_power_callback(play_callback_t callback);
void takephoto_unregister_power_callback(void);

//取消所有注册的按键回调
void takephoto_unregister_all_callback(void);
#ifdef __cplusplus
}
#endif

#endif /* __INDEV_H__ */

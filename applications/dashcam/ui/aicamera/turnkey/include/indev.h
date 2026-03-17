#ifndef __INDEV_H__
#define __INDEV_H__

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* __INDEV_H__ */

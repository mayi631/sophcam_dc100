#include <stdio.h>
#include "indev.h"

/**
 * @brief 当前页面的按键处理回调函数指针
 *
 * 全局变量，指向当前活动页面的按键处理函数。
 * 当硬件按键事件发生时，会调用此函数指针指向的处理函数。
 * 初始值为NULL，表示没有设置按键处理函数。
 */
page_key_handler_t current_page_key_handler = NULL;

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
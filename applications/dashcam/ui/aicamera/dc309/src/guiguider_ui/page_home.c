/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
// #############################################################################
// ! #region 1. 头文件与宏定义
// #############################################################################
#define DEBUG
#include "config.h"
#include "custom.h"
#include "gui_guider.h"
#include "indev.h"
#include "lvgl.h"
#include "page_all.h"
#include "ui_common.h"
#include <stdio.h>
#include <stdlib.h>

// 2D网格导航配置 - 6个按钮，2行3列
#define GRID_COLS 5 // 每行3个按钮
#define GRID_ROWS 1 // 共2行
#define GRID_MAX_OBJECTS GRID_COLS* GRID_ROWS // 最大对象数量

#define ICON_BIG_WIDTH 240  // 大图标宽度
#define ICON_BIG_HEIGHT 284  // 大图标高度
#define ICON_WIDTH 154 // 图标宽度
#define ICON_HEIGHT 136 // 图标高度
#define W_SPACE 16 // 宽间隔
#define H_SPACE 12 // 高间隔
// #endregion
// #############################################################################
// ! #region 2. 数据结构定义
// #############################################################################

// #endregion
// #############################################################################
// ! #region 3. 全局变量 &  函数声明
// #############################################################################
lv_obj_t* obj_home_s = NULL;
lv_obj_t* img_battery = NULL;  // 电池图标

extern lv_style_t ttf_font_20;
static uint8_t curr_focus_index = 0;

// 添加焦点组和当前焦点索引
static lv_obj_t* focusable_objects[GRID_MAX_OBJECTS] = { NULL }; // 存储可聚焦的对象
static bool is_first_init = true; // 是否是第一次初始化

// #endregion
// #############################################################################
// ! #region 4. 内部工具函数（注意用static修饰）
// #############################################################################

// 创建单个功能块
static lv_obj_t* create_function_block(lv_obj_t* parent, int index, const char* icon_path, const char* text, int32_t icon_width, int32_t icon_height, int32_t pos_x, int32_t pos_y)
{
    // 创建功能块容器
    lv_obj_t* block = lv_btn_create(parent);
    lv_obj_set_size(block, icon_width, icon_height);
    lv_obj_set_pos(block, pos_x, pos_y);
    
    // 设置样式：圆角、淡蓝色边框
    lv_obj_set_style_radius(block, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(block, lv_color_hex(0x1E1E3E), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(block, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(block, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(block, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(block, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(block, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 悬停效果
    lv_obj_set_style_bg_color(block, lv_color_hex(0x2E2E5E), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_color(block, lv_color_hex(0x6DC6FF), LV_PART_MAIN | LV_STATE_PRESSED);
    
    // 创建图标
    lv_obj_t* icon = lv_img_create(block);
    lv_obj_set_size(icon, icon_width, icon_height);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
    if (strcmp(text, str_language_ai_recognition[get_curr_language()]) != 0) {
        show_image(icon, icon_path);
    } else {
        lv_obj_set_style_bg_image_src(icon, &airecognition, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // 创建文本标签
    lv_obj_t* label = lv_label_create(block);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(label, &ttf_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);
    
    return block;
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
// 自定义点击回调函数
static void home_click_callback(lv_obj_t* obj)
{
    // 根据对象在数组中的位置确定操作
    int obj_index = -1;
    for (int i = 0; i < GRID_MAX_OBJECTS; i++) {
        if (focusable_objects[i] == obj) {
            obj_index = i;
            curr_focus_index = i;
            break;
        }
    }

    if (is_first_init) {
        is_first_init = false;
        // 显示黑色背景色，盖掉开机logo。后续送流的时候，会盖掉背景色。
        system("devmem 0x0a094094 32 0x00010028");
    }
    
    if (obj_index >= 0) {
        switch (obj_index) {
        case 0: // 动物识别
        {
            MLOG_DBG("进入动物识别页面\n");
            // 添加动物识别页面的跳转逻辑
        } break;
        case 1: // AI识别
        {
            MESSAGE_S Msg = {0};
            Msg.topic     = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1      = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);
            ui_load_scr_animation(&g_ui, &page_ai_camera_s,1, NULL, create_ai_camera_screen, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
            MLOG_DBG("进入AI识别页面\n");
            // 添加AI识别页面的跳转逻辑
        } break;
        case 2: // AI夜视仪
        {
            MLOG_DBG("进入AI夜视仪页面\n");
            ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del, NULL, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
            MESSAGE_S Msg = { 0 };
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);
        } break;
        case 3: // 相册
        {
            MLOG_DBG("进入相册页面\n");
            MESSAGE_S Msg = { 0 };
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_PLAYBACK;
            MODEMNG_SendMessage(&Msg);
            ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
        } break;
        case 4: // 设置
        {
            MLOG_DBG("进入设置页面\n");
            ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
        } break;
        case 5: // AI通用功能
        {
            MLOG_DBG("进入通用AI功能页面\n");
            MESSAGE_S Msg = { 0 };
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_PHOTO;
            MODEMNG_SendMessage(&Msg);
            ui_load_scr_animation(&g_ui, &page_ai_camera_s, 1, NULL, create_ai_camera_screen, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
        } break;
        }
    }
}

// 统一的按钮事件处理函数
static void home_btn_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* btn_clicked = lv_event_get_target(e);
    
    MLOG_DBG("事件代码: %s\n", lv_event_code_get_name(code));
    
    switch (code) {
        case LV_EVENT_CLICKED: {
            // 查找点击的按钮在数组中的索引
            for (int i = 0; i < GRID_MAX_OBJECTS; i++) {
                if (btn_clicked == focusable_objects[i]) {
                    if (is_first_init) {
                        is_first_init = false;
                        system("devmem 0x0a094094 32 0x00010028");
                    }
                    
                    if (get_exit_completed() != true) {
                        MLOG_DBG("退出未完成，舍弃点击\n");
                        return;
                    }
                    
                    curr_focus_index = i;
                    lv_obj_add_state(btn_clicked, LV_STATE_FOCUS_KEY);
                    
                    // 根据索引执行相应操作
                    home_click_callback(btn_clicked);
                    break;
                }
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
void setup_scr_home1(lv_ui_t* ui)
{
    MLOG_DBG("loading page_home1...\n");
    extern uint8_t g_last_scr_mode;
    g_last_scr_mode = 0;
    // 创建主屏幕
    obj_home_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_home_s, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(obj_home_s, LV_SCROLLBAR_MODE_OFF);
    
    // 设置深蓝色星空渐变背景
    lv_obj_set_style_bg_opa(obj_home_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj_home_s, lv_color_hex(0x0A0A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_color(obj_home_s, lv_color_hex(0x1A1A4A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(obj_home_s, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(obj_home_s,&Desktop,LV_PART_MAIN | LV_STATE_DEFAULT);

    // 功能块配置：图标路径、标签文本、AI标签
    typedef struct {
        const char* icon_path;
        const char* label_text;
        int32_t width;
        int32_t height;
        int32_t x;
        int32_t y;
    } function_block_t;

    function_block_t blocks[GRID_MAX_OBJECTS] = {
        { "AI识别.png", str_language_ai_recognition[get_curr_language()], ICON_BIG_WIDTH, ICON_BIG_HEIGHT, 30, 38 }, // 索引0
        { "AI拍识万物.png", str_language_ai_photo_recognize_everything[get_curr_language()], ICON_WIDTH, ICON_HEIGHT, ICON_BIG_WIDTH + 30 + W_SPACE, 38 }, // 索引1
        { "AI夜视仪.png", str_language_ai_night_vision[get_curr_language()], ICON_WIDTH, ICON_HEIGHT, ICON_BIG_WIDTH + 30 + ICON_WIDTH + W_SPACE * 2, 38 }, // 索引2
        { "相册.png", str_language_album[get_curr_language()], ICON_WIDTH, ICON_HEIGHT, ICON_BIG_WIDTH + 30 + W_SPACE, 38 + ICON_HEIGHT + H_SPACE }, // 索引3
        { "设置.png", str_language_settings[get_curr_language()], ICON_WIDTH, ICON_HEIGHT, ICON_BIG_WIDTH + 30 + ICON_WIDTH + W_SPACE * 2, 38 + ICON_HEIGHT + H_SPACE }, // 索引4
    };

    // 创建6个功能块
    for (int i = 0; i < GRID_MAX_OBJECTS; i++) {
        lv_obj_t* block = create_function_block(obj_home_s, i,
            blocks[i].icon_path,
            blocks[i].label_text,
            blocks[i].width,
            blocks[i].height,
            blocks[i].x,
            blocks[i].y
        );

        // 添加点击事件
        lv_obj_add_event_cb(block, home_btn_event_handler, LV_EVENT_CLICKED, NULL);
        
        // 添加到焦点组
        focusable_objects[i] = block;
    }

    // 初始化焦点组
    lv_obj_t* target_obj = focusable_objects[curr_focus_index];
    init_focus_group(obj_home_s, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS, home_click_callback, target_obj);
    lv_obj_add_state(target_obj, LV_STATE_FOCUS_KEY);
    
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    
    MLOG_DBG("Home page setup completed with 6 function blocks\n");
}

// #endregion
// #############################################################################
// ! #region 9. 调试与测试
// #############################################################################

// #endregion
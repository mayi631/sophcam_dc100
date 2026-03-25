#define DEBUG
/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

 #include "config.h"
 #include "custom.h"
 #include "gui_guider.h"
 #include "indev.h"
 #include "lvgl.h"
 #include "page_all.h"
 #include "style_common.h"
 #include "ui_common.h"
 #include <stdio.h>
 #include "hal_pwm.h"
#include <math.h>

 #define GRID_COLS 1
 #define GRID_ROWS 2
 #define GRID_MAX_OBJECTS GRID_ROWS *GRID_COLS
 static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];
 lv_obj_t *obj_sysMenu_statuslight_s = NULL;
 
 extern char g_sysbtn_labelstatuslight[32];
 
 static uint8_t stlight_Current_Index_s = 2;
 
// 呼吸灯线程相关
static HAL_PWM_S pwmAttr = {0};
static pthread_t breathing_led_thread = 0;
static volatile bool breathing_led_running = false;
static volatile bool breathing_led_thread_active = false;


 uint8_t getstalight_Index(void)
 {
     return stlight_Current_Index_s / 2;
 }
 
 void setsysMenu_stlight_Index(uint8_t index)
 {
     stlight_Current_Index_s = index * 2;
 }
 
 void setsysMenu_stlight_Label(char* plabel)
 {
     memset(g_sysbtn_labelstatuslight, 0, sizeof(g_sysbtn_labelstatuslight));
     strncpy(g_sysbtn_labelstatuslight, plabel, sizeof(g_sysbtn_labelstatuslight));
 }
 
// 呼吸灯线程函数
static void* breathing_led_thread_func(void* arg)
{
    UNUSED(arg);
    prctl(PR_SET_NAME, "breathing_led", 0, 0, 0);

    breathing_led_thread_active = true;
    MLOG_INFO("Breathing LED thread started\n");

    // 配置 PWM 参数
    pwmAttr.group = 2;          // PWM 组
    pwmAttr.channel = 3;        // PWM 通道
    pwmAttr.period = 100;   // 周期（单位：纳秒）
    pwmAttr.duty_cycle = 0; // 占空比（单位：纳秒）

    // 初始化 PWM
    if (HAL_PWM_Init(pwmAttr) != 0) {
        MLOG_ERR("PWM 初始化失败\n");
        breathing_led_thread_active = false;
        return NULL;
    }
    MLOG_INFO("PWM 初始化成功\n");


    // 呼吸灯参数
    const int BREATHING_PERIOD_MS = 2000;  // 一个呼吸周期2秒
    const int UPDATE_INTERVAL_MS = 20;      // 每20ms更新一次
    const int STEPS = BREATHING_PERIOD_MS / UPDATE_INTERVAL_MS;  // 一个周期内的步数

    while (breathing_led_running) {
        // 使用正弦波实现平滑的呼吸效果
        for (int i = 0; i < STEPS && breathing_led_running; i++) {
            // 计算当前步的亮度 (0-100)
            // 使用sin函数生成0-1的波形，然后映射到1-100（避免完全熄灭）
            double angle = (double)i * 2.0 * M_PI / STEPS;
            double brightness = (sin(angle) + 1.0) / 2.0;  // 0-1
            pwmAttr.duty_cycle = (int)(brightness * 99.0 + 1.0);  // 1-100

            // 控制Linux PWM15的占空比，实现呼吸灯效果
            HAL_PWM_Set_Param(pwmAttr);

            // 等待更新间隔
            usleep(UPDATE_INTERVAL_MS * 1000);
        }
    }

    // 禁用 PWM
    if(HAL_PWM_Deinit(pwmAttr) != 0) MLOG_ERR("PWM 反初始化失败\n");

    breathing_led_thread_active = false;
    MLOG_INFO("Breathing LED thread stopped\n");

    return NULL;
}

// 启动呼吸灯线程
static int start_breathing_led_thread(void)
{
    // 如果线程已经在运行，直接返回
    if (breathing_led_thread_active) {
        MLOG_DBG("Breathing LED thread is already running\n");
        return 0;
    }

    // 如果线程已创建但未激活，等待其结束并清理
    if (breathing_led_thread != 0) {
        breathing_led_running = false;
        pthread_join(breathing_led_thread, NULL);
        breathing_led_thread = 0;
        breathing_led_thread_active = false;
    }

    // 设置运行标志
    breathing_led_running = true;
    breathing_led_thread_active = false;  // 先设为false，线程启动后会设为true

    // 创建线程
    int ret = pthread_create(&breathing_led_thread, NULL, breathing_led_thread_func, NULL);
    if (ret != 0) {
        MLOG_ERR("Failed to create breathing LED thread: %d\n", ret);
        breathing_led_running = false;
        breathing_led_thread = 0;
        return -1;
    }

    // 等待线程启动（等待线程设置breathing_led_thread_active为true）
    int wait_count = 0;
    while (!breathing_led_thread_active && wait_count < 50) {
        usleep(20000);  // 每次等待20ms
        wait_count++;
    }

    if (breathing_led_thread_active) {
        MLOG_INFO("Breathing LED thread created and started successfully\n");
        return 0;
    } else {
        MLOG_ERR("Breathing LED thread failed to start within timeout\n");
        breathing_led_running = false;
        if (breathing_led_thread != 0) {
            pthread_cancel(breathing_led_thread);
            pthread_join(breathing_led_thread, NULL);
            breathing_led_thread = 0;
        }
        return -1;
    }
}

// 停止呼吸灯线程
static void stop_breathing_led_thread(void)
{
    if (!breathing_led_thread_active && breathing_led_thread == 0) {
        return;
    }

    // 设置停止标志，通知线程退出
    breathing_led_running = false;

    // 等待线程结束
    if (breathing_led_thread != 0) {
        void* thread_result = NULL;
        int ret = pthread_join(breathing_led_thread, &thread_result);
        if (ret != 0) {
            MLOG_ERR("Failed to join breathing LED thread: %d\n", ret);
        }
        breathing_led_thread = 0;
    }

    // 确保呼吸灯关闭
    HAL_PWM_Deinit(pwmAttr);
    breathing_led_thread_active = false;

    MLOG_INFO("Breathing LED thread stopped and resources released\n");
}

// 开启状态灯
static void stlight_on(void)
{
    // 启动呼吸灯线程
    start_breathing_led_thread();
}

// 关闭状态灯
static void stlight_off(void)
{
    // 停止呼吸灯线程
    stop_breathing_led_thread();
    MLOG_INFO("Screen turned on (backlight control)\n");
}

// 根据参数初始化状态灯（开机时调用）
void stlight_init_by_param(int32_t is_on)
{
    if (is_on == 0) {
        stlight_on();
    } else {
        stlight_off();
    }
}
 static void stlight_Del_Complete_anim_cb(lv_anim_t *a)
 {
     if(obj_sysMenu_statuslight_s != NULL) {
         if(lv_obj_is_valid(obj_sysMenu_statuslight_s)) {
             lv_obj_del(obj_sysMenu_statuslight_s);
             obj_sysMenu_statuslight_s = NULL;
         }
         obj_sysMenu_statuslight_s = NULL;
         ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                               false, true);
     }
 }
 
 static void stlight_win_Delete_anim(void)
 {
     lv_anim_t Delete_anim; // 动画渐隐句柄
     // 创建透明度动画
     lv_anim_init(&Delete_anim);
     lv_anim_set_values(&Delete_anim, 0, 1);
     lv_anim_set_time(&Delete_anim, 6);
 
     // lv_anim_set_exec_cb(&Delete_anim, AIanim_objSet_Opa);
     lv_anim_set_path_cb(&Delete_anim, lv_anim_path_ease_out);
     // 设置动画完成回调（销毁对象）
     lv_anim_set_completed_cb(&Delete_anim, stlight_Del_Complete_anim_cb);
 
     lv_anim_start(&Delete_anim);
 }
 
 static void screen_Settingstlight_btn_back_event_handler(lv_event_t *e)
 {
     lv_event_code_t code = lv_event_get_code(e);
     MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
     switch(code) {
         case LV_EVENT_CLICKED: {
             ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                   false, true);
             break;
         }
         default: break;
     }
 }
 
 void photostlight_SelectFocus_OK(lv_event_t *e, lv_obj_t *obj)
 {
 
     lv_obj_t *btn_clicked = NULL;
     if(obj == NULL) {
         btn_clicked = lv_event_get_target(e); // 获取发生点击事件的控件
     } else {
         btn_clicked = obj;
     }
     lv_obj_t *parent = lv_obj_get_parent(btn_clicked); // 获取发生点击事件的父控件
     // 获取焦点控件
     lv_obj_t *chlid = lv_obj_get_child(parent, stlight_Current_Index_s);
 
     for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
         if(i == stlight_Current_Index_s) {
             // 先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
             lv_group_focus_obj(chlid);
             lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
             lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20),
                                         LV_PART_MAIN | LV_STATE_DEFAULT);
         }
 
         if((btn_clicked == lv_obj_get_child(parent, i))) {
             if(lv_obj_get_child(btn_clicked, 1) == NULL) {
                 lv_obj_t *label1 = lv_label_create(btn_clicked);
                 lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                 lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                 lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                 lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
             }
         } else {
             lv_obj_t *child = lv_obj_get_child(parent, i);
             if(lv_obj_get_child(child, 1) != NULL) {
                 lv_obj_del(lv_obj_get_child(child, 1));
             }
         }
     }
 }
 
 static void screen_Settingstlight_btn_event_handler(lv_event_t *e)
 {
     lv_event_code_t code = lv_event_get_code(e);
     switch(code) {
         case LV_EVENT_CLICKED: {
             lv_obj_t *btn_clicked = lv_event_get_target(e);         // 获取发生点击事件的控件
             lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); // 获取发生点击事件的父控件
             for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
 
                 if(lv_obj_get_child(parent, i) == btn_clicked) {
                     stlight_Current_Index_s = i;
                     // 获取按钮标签文本
                     lv_obj_t *label = lv_obj_get_child(lv_obj_get_child(parent, i), 0);
                     if(label && lv_obj_check_type(label, &lv_label_class)) {
                         const char *txt = lv_label_get_text(label);
                         if(txt) strncpy(g_sysbtn_labelstatuslight, txt, sizeof(g_sysbtn_labelstatuslight));
                     }
                    if (strcmp(g_sysbtn_labelstatuslight, str_language_on[get_curr_language()]) == 0) {
                       stlight_on();
                    } else {
                       stlight_off();
                    }

                    MESSAGE_S Msg = {0};
                    Msg.topic = EVENT_MODEMNG_SETTING;
                    Msg.arg1  = PARAM_MENU_STATUS_LIGHT;
                    Msg.arg2  = stlight_Current_Index_s / 2; // 0:ON, 1:OFF
                    MODEMNG_SendMessage(&Msg);

                    lv_obj_add_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xFF0000), LV_PART_MAIN);
                } else {
                    lv_obj_clear_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                }
            }
            stlight_win_Delete_anim();
            break;
        }
        default: break;
    }
}
 
 static void photostlight_click_callback(lv_obj_t *obj)
 {
     MLOG_DBG("photostlight_click_callback\n");
     lv_obj_t *parent = lv_obj_get_parent(obj); // 获取发生点击事件的父控件
     for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
 
         if(lv_obj_get_child(parent, i) == obj) {
             stlight_Current_Index_s = i;
             photostlight_SelectFocus_OK(NULL, obj);
             // 获取按钮标签文本
             lv_obj_t *label = lv_obj_get_child(lv_obj_get_child(parent, i), 0);
             if(label && lv_obj_check_type(label, &lv_label_class)) {
                 const char *txt = lv_label_get_text(label);
                 if(txt) strncpy(g_sysbtn_labelstatuslight, txt, sizeof(g_sysbtn_labelstatuslight));
             }
            if (strcmp(g_sysbtn_labelstatuslight, str_language_on[get_curr_language()]) == 0) {
               stlight_on();
            } else {
               stlight_off();
            }

            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_SETTING;
            Msg.arg1  = PARAM_MENU_STATUS_LIGHT;
            Msg.arg2  = stlight_Current_Index_s / 2; // 0:ON, 1:OFF
            MODEMNG_SendMessage(&Msg);

            lv_obj_add_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
            lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xFF0000), LV_PART_MAIN);
        } else {
            lv_obj_clear_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
            lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xCCCCCC), LV_PART_MAIN);
        }
    }
    stlight_win_Delete_anim();
}
 
 static void photostlight_menu_callback(void)
 {
     MLOG_DBG("photostlight_menu_callback\n");
     ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                           true);
 }
 
 static void gesture_event_handler(lv_event_t *e)
 {
     lv_event_code_t code = lv_event_get_code(e);
     MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
     switch(code) {
         case LV_EVENT_GESTURE: {
             // 获取手势方向，需要 TP 驱动支持
             lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
             switch(dir) {
                 case LV_DIR_RIGHT: {
                     ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting,
                                           LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
                 }
                 default: break;
             }
             break;
         }
         default: break;
     }
 }
 
 void sysMenu_stlight(lv_ui_t *ui)
 {
 
     MLOG_DBG("loading page_sensitivity...\n");
 
     // 创建主页面1 容器
     if(obj_sysMenu_statuslight_s != NULL) {
         if(lv_obj_is_valid(obj_sysMenu_statuslight_s)) {
             MLOG_DBG("page_scr 仍然有效，删除旧对象\n");
             lv_obj_del(obj_sysMenu_statuslight_s);
         } else {
             MLOG_DBG("page_scr 已被自动销毁，仅重置指针\n");
         }
         obj_sysMenu_statuslight_s = NULL;
     }
 
     // Write codes scr
     obj_sysMenu_statuslight_s = lv_obj_create(NULL);
     lv_obj_set_size( obj_sysMenu_statuslight_s , H_RES, V_RES);
     lv_obj_add_style( obj_sysMenu_statuslight_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_add_event_cb(obj_sysMenu_statuslight_s, gesture_event_handler, LV_EVENT_GESTURE, ui);
 
     // Write codes cont_top (顶部栏)
     lv_obj_t *cont_top = lv_obj_create(obj_sysMenu_statuslight_s);
     lv_obj_set_pos(cont_top, 0, 0);
     lv_obj_set_size(cont_top, H_RES, 60);
     lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
     lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);
 
     // Write codes btn_back (返回按钮)
     lv_obj_t* btn_back = lv_button_create(cont_top);
     lv_obj_set_pos(btn_back, 4, 4);
     lv_obj_set_size(btn_back, 60, 52);
     lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_add_event_cb(btn_back, screen_Settingstlight_btn_back_event_handler, LV_EVENT_CLICKED, ui);
 
     lv_obj_t* label_back = lv_label_create(btn_back);
     lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
     lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
     lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
     lv_obj_set_width(label_back, LV_PCT(100));
     lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
 
     // Write codes title (标题)
     lv_obj_t* title = lv_label_create(cont_top);
     lv_label_set_text(title, str_language_status_light[get_curr_language()]);
     lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
     lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);
     // Write codes cont_settings (设置选项容器)
     lv_obj_t *cont_settings = lv_obj_create(obj_sysMenu_statuslight_s);
     lv_obj_set_size(cont_settings, 600, MENU_CONT_SIZE);
     lv_obj_align(cont_settings, LV_ALIGN_TOP_MID, 0, 64);
     lv_obj_set_style_bg_opa(cont_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_set_style_border_width(cont_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_set_style_pad_all(cont_settings, 10, 0);
 
     // 创建设置按钮
     const char *btn_labels[] = {str_language_on[get_curr_language()], str_language_off[get_curr_language()]};
     static lv_point_precise_t line_points_pool[sizeof(btn_labels) / sizeof(btn_labels[0])][2];
     for(int i = 0; i < 2; i++) {
         lv_obj_t *btn = lv_button_create(cont_settings);
         if(!btn) continue; // 如果按钮创建失败则跳过
 
         lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
         lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
         lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
 
         lv_obj_t *label = lv_label_create(btn);
         if(!label) continue; // 如果标签创建失败则跳过
 
         lv_label_set_text(label, btn_labels[i]);
         lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                    LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
 
         // 添加事件处理器，传入容器对象作为用户数据
         lv_obj_add_event_cb(btn, screen_Settingstlight_btn_event_handler, LV_EVENT_ALL, cont_settings);
 
         if(i == stlight_Current_Index_s / 2) {
             {
                 lv_obj_t *label1 = lv_label_create(btn);
                 lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                 lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                 lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                 lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
             }
         }
 
         lv_obj_t *line = lv_line_create(cont_settings);
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
     lv_group_focus_obj(lv_obj_get_child(cont_settings, stlight_Current_Index_s));
     lv_obj_add_state(lv_obj_get_child(cont_settings, stlight_Current_Index_s), LV_STATE_FOCUS_KEY);
     lv_obj_scroll_to_y(cont_settings, ((stlight_Current_Index_s / 2) * MENU_BTN_SIZE), LV_ANIM_OFF);
     // 获取焦点控件
     lv_obj_t *chlid = lv_obj_get_child(cont_settings, stlight_Current_Index_s);
     // 设置焦点标签颜色
     lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
 
     // 在上方添加一条分割线
     lv_obj_t *up_line                       = lv_line_create(obj_sysMenu_statuslight_s);
     static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
     lv_line_set_points(up_line, points_line, 2);
     lv_obj_set_style_line_width(up_line, 2, 0);
     lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);
 
     lv_obj_t *target_obj = lv_obj_get_child(cont_settings, stlight_Current_Index_s);
     // 初始化焦点组
     init_focus_group(cont_settings, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
                      photostlight_click_callback, target_obj);
     // 设置当前页面的按键处理器
     set_current_page_handler(handle_grid_navigation);
     takephoto_register_menu_callback(photostlight_menu_callback);
 
     // Update current screen layout.
     lv_obj_update_layout(obj_sysMenu_statuslight_s);
 }
 
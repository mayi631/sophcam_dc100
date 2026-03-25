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
 
 #define GRID_COLS 1
 #define GRID_ROWS 7
 #define GRID_MAX_OBJECTS GRID_ROWS * GRID_COLS
 static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];
 
 lv_obj_t *brightness_scr;
 
 static uint8_t brightness_Current_Index_s = 3 * 2;
 
 extern char g_sysbtn_labellight_level[32];
 // 亮度等级定义
 brightness_level_t brightness_levels[BRIGHTNESS_LEVEL_COUNT] = {
     {1, 36,  "level 1"},  // 36/255 ≈ 14%
     {2, 72,  "level 2"},  // 72/255 ≈ 28%
     {3, 108, "level 3"},  // 108/255 ≈ 42%
     {4, 160, "level 4"},  // 160/255 ≈ 62%
     {5, 180, "level 5"},  // 180/255 ≈ 71%
     {6, 216, "level 6"},  // 216/255 ≈ 85%
     {7, 252, "level 7"},  // 252/255 ≈ 99%
 };
 

 uint8_t get_curr_brightness(void)
 {
     return brightness_Current_Index_s/2;
 }
 
 void setsysMenu_brightness_Index(uint8_t index)
 {
     brightness_Current_Index_s = index * 2;
 }
 

 // 写亮度值到系统文件
 static int write_brightness_value(int value)
 {
     int fd = -1;
     char buffer[16] = {0};
     
     // 打开亮度控制文件
     fd = open("/sys/devices/platform/backlight/backlight/backlight/brightness", O_WRONLY);
     if (fd < 0) {
         MLOG_ERR("无法打开亮度控制文件\n");
         return -1;
     }
     
     // 准备要写入的数据
     snprintf(buffer, sizeof(buffer), "%d\n", value);
     
     // 写入亮度值
     ssize_t written = write(fd, buffer, strlen(buffer));
     if (written < 0) {
         MLOG_ERR("写入亮度值失败\n");
         close(fd);
         return -1;
     }
     
     MLOG_DBG("设置亮度值: %d (等级: %d)\n", value, get_curr_brightness());
     close(fd);
     return 0;
 }
 
 // 读取当前亮度值
 static int read_current_brightness(void)
 {
     FILE *fp = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "r");
     if (!fp) {
         MLOG_ERR("无法读取当前亮度值\n");
         return 0;
     }
     
     int value = 0;
     fscanf(fp, "%d", &value);
     fclose(fp);
     
     return value;
 }

 // 设置亮度等级
 void brightness_set_level(int level)
 {
     if (level < 1 || level > BRIGHTNESS_LEVEL_COUNT) {
         MLOG_ERR("无效的亮度等级: %d\n", level);
         return;
     }
     
     // 更新当前索引
     snprintf(g_sysbtn_labellight_level,sizeof(g_sysbtn_labellight_level), "level %d", level);
     // 写入亮度值
     write_brightness_value(brightness_levels[get_curr_brightness()].value);
 }
 void setsysMenu_brightness_Label(char* plabel)
 {
     memset(g_sysbtn_labellight_level, 0, sizeof(g_sysbtn_labellight_level));
     strncpy(g_sysbtn_labellight_level, plabel, sizeof(g_sysbtn_labellight_level));
 }

 static void brightness_Del_Complete_anim_cb(lv_anim_t* a)
 {
     ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
         false, true);
 }

 static void brightness_win_Delete_anim(void)
 {
     lv_anim_t Delete_anim; //动画渐隐句柄
     // 创建透明度动画
     lv_anim_init(&Delete_anim);
     lv_anim_set_values(&Delete_anim, 0, 1);
 
     lv_anim_set_time(&Delete_anim, 6);
 
     // lv_anim_set_exec_cb(&Delete_anim, AIanim_objSet_Opa);
     lv_anim_set_path_cb(&Delete_anim, lv_anim_path_ease_out);
     // 设置动画完成回调（销毁对象）
     lv_anim_set_completed_cb(&Delete_anim, brightness_Del_Complete_anim_cb);
 
     lv_anim_start(&Delete_anim);
 }
 
 static void sysMenu_brightness_btn_back_event_handler(lv_event_t *e)
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
 
 void syamenu_brightness_SelectFocus_OK(lv_event_t *e)
 {
 
     lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
     lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
     //获取焦点控件
     lv_obj_t *chlid = lv_obj_get_child(parent, brightness_Current_Index_s);
 
     for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
         if(i == brightness_Current_Index_s) {
             //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
             lv_group_focus_obj(chlid);
             lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
             // //设置焦点渐变
             // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
             // //设置焦点BG
             // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
             //设置焦点标签颜色
             lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20),
                                         LV_PART_MAIN | LV_STATE_DEFAULT);
             // lv_obj_set_style_text_color(lv_obj_get_child(chlid,1), lv_color_hex(0xFFFFFF), LV_PART_MAIN |
             // LV_STATE_DEFAULT);
 
         } else {
             // lv_obj_t *other_child = lv_obj_get_child(parent, i);
             // //设置焦点渐变
             // lv_set_obj_grad_style(other_child, LV_GRAD_DIR_VER, lv_color_hex(0), lv_color_hex(0));
             // //设置焦点BG
             // lv_obj_set_style_bg_color(other_child, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
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
 
 static void sysMenu_brightness_Select_btn_event_handler(lv_event_t *e)
 {
     lv_event_code_t code = lv_event_get_code(e);
     switch(code) {
         case LV_EVENT_CLICKED: {
             lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
             lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
             for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
 
                 if(lv_obj_get_child(parent, i) == btn_clicked) {
                     brightness_Current_Index_s = i;
                     syamenu_brightness_SelectFocus_OK(e);
                     // 获取按钮标签文本
                     lv_obj_t *label = lv_obj_get_child(lv_obj_get_child(parent, i), 0);
                     if(label && lv_obj_check_type(label, &lv_label_class)) {
                         const char *txt = lv_label_get_text(label);
                         if(txt) strncpy(g_sysbtn_labellight_level, txt, sizeof(g_sysbtn_labellight_level));

                     }
                    brightness_set_level((i/2)+1);

                    MESSAGE_S Msg = {0};
                    Msg.topic = EVENT_MODEMNG_SETTING;
                    Msg.arg1  = PARAM_MENU_BRIGHTNESS;
                    Msg.arg2  = i / 2; // 0-6 对应 level 1-7
                    MODEMNG_SendMessage(&Msg);

                    lv_obj_add_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xFF0000), LV_PART_MAIN);
                } else {
                    lv_obj_clear_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                }
            }
            brightness_win_Delete_anim();
 
         }; break;
         default: {
             // 处理其他事件
             break;
         }
     }
 }
 
 static void sysmenu_brightness_click_callback(lv_obj_t *obj)
 {
     MLOG_DBG("sysmenu_brightness_click_callback\n");
     lv_obj_t *parent      = lv_obj_get_parent(obj); //获取发生点击事件的父控件
     for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
 
         if(lv_obj_get_child(parent, i) == obj) {
             brightness_Current_Index_s = i;
             //获取焦点控件
             lv_obj_t *chlid = lv_obj_get_child(parent, brightness_Current_Index_s);
 
             for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                 if(i == brightness_Current_Index_s) {
                     //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
                     lv_group_focus_obj(chlid);
                     lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
                     //设置焦点标签颜色
                     lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20),
                                                 LV_PART_MAIN | LV_STATE_DEFAULT);
                    brightness_set_level((i/2)+1);
                 }
                 if((obj == lv_obj_get_child(parent, i))) {
                     if(lv_obj_get_child(obj, 1) == NULL) {
                         lv_obj_t *label1 = lv_label_create(obj);
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
            // 获取按钮标签文本
            lv_obj_t *label = lv_obj_get_child(lv_obj_get_child(parent, i), 0);
            if(label && lv_obj_check_type(label, &lv_label_class)) {
                const char *txt = lv_label_get_text(label);
                if(txt) strncpy(g_sysbtn_labellight_level, txt, sizeof(g_sysbtn_labellight_level));
            }

            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_SETTING;
            Msg.arg1  = PARAM_MENU_BRIGHTNESS;
            Msg.arg2  = i / 2; // 0-6 对应 level 1-7
            MODEMNG_SendMessage(&Msg);

            lv_obj_add_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
             lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xFF0000), LV_PART_MAIN);
         } else {
             lv_obj_clear_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
             lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xCCCCCC), LV_PART_MAIN);
         }
     }
     brightness_win_Delete_anim();
 
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
 
 void sysMenu_brightness(lv_ui_t *ui)
 {

     // 读取当前亮度值，确定当前等级
     int current_value = read_current_brightness();
     for (int i = 0; i < BRIGHTNESS_LEVEL_COUNT; i++) {
         if (current_value <= brightness_levels[i].value) {
            brightness_Current_Index_s = i * 2;
            break;
         }
     }

     // Write codes resscr
     brightness_scr = lv_obj_create(NULL);
     lv_obj_set_size( brightness_scr , H_RES, V_RES);
     lv_obj_add_style( brightness_scr , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_add_event_cb(brightness_scr, gesture_event_handler, LV_EVENT_GESTURE, ui);
 
     // Write codes cont_top (顶部栏)
     lv_obj_t *cont_top = lv_obj_create(brightness_scr);
     lv_obj_set_pos(cont_top, 0, 0);
     lv_obj_set_size(cont_top, H_RES, 60);
     lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
     lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);
 
     // Write codes btn_back (返回按钮)
     lv_obj_t* btn_back = lv_button_create(cont_top);
     lv_obj_set_pos(btn_back, 4, 4);
     lv_obj_set_size(btn_back, 60, 52);
     lv_obj_add_event_cb(btn_back, sysMenu_brightness_btn_back_event_handler, LV_EVENT_CLICKED, NULL);
     lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);
 
     lv_obj_t* label_back = lv_label_create(btn_back);
     lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
     lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
     lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
     lv_obj_set_width(label_back, LV_PCT(100));
     lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
 
     // Write codes title (标题)
     lv_obj_t* title = lv_label_create(cont_top);
     lv_label_set_text(title, str_language_display_brightness[get_curr_language()]);
     lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
     lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);
 
     // 创建设置选项容器
     lv_obj_t *settings_cont = lv_obj_create(brightness_scr);
     lv_obj_set_size(settings_cont, 600, MENU_CONT_SIZE);
     lv_obj_align(settings_cont, LV_ALIGN_TOP_MID, 0, 64);
     lv_obj_set_style_bg_opa(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
     lv_obj_set_style_border_width(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
     // lv_obj_set_flex_flow(settings_cont, LV_FLEX_FLOW_COLUMN);
     // lv_obj_set_flex_align(settings_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
     lv_obj_set_style_pad_all(settings_cont, 10, 0);
 
     static lv_point_precise_t line_points_pool[sizeof(brightness_levels) / sizeof(brightness_levels[0])][2];
     for(uint8_t i = 0; i < sizeof(brightness_levels) / sizeof(brightness_levels[0]); i++) {
         lv_obj_t *btn = lv_button_create(settings_cont);
         if(!btn) continue; // 如果按钮创建失败则跳过
 
         lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
         lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
         lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
         lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
         lv_obj_set_style_radius(btn, 5, LV_PART_MAIN);
 
         lv_obj_t *label = lv_label_create(btn);
         if(!label) continue; // 如果标签创建失败则跳过
 
         lv_label_set_text(label, brightness_levels[i].label);
         lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
 
         lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
 
         // 添加事件处理器，传入容器对象作为用户数据
         lv_obj_add_event_cb(btn, sysMenu_brightness_Select_btn_event_handler, LV_EVENT_ALL, settings_cont);
 
         if(i == brightness_Current_Index_s / 2) {
             {
                 lv_obj_t *label1 = lv_label_create(btn);
                 lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                 lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                 lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                 lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
             }
         }
 
         lv_obj_t *line = lv_line_create(settings_cont);
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
 
     //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
     //获取焦点控件
     lv_obj_t *chlid = lv_obj_get_child(settings_cont, brightness_Current_Index_s);
     lv_group_focus_obj(chlid);
     lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
     lv_obj_scroll_to_y(settings_cont, ((brightness_Current_Index_s / 2) * MENU_BTN_SIZE), LV_ANIM_OFF);

     //设置焦点标签颜色
     lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
 
     // 在上方添加一条分割线
     lv_obj_t *up_line                       = lv_line_create(brightness_scr);
     static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
     lv_line_set_points(up_line, points_line, 2);
     lv_obj_set_style_line_width(up_line, 2, 0);
     lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);
 
     lv_obj_t *target_obj = lv_obj_get_child(settings_cont, brightness_Current_Index_s);
     // 初始化焦点组
     init_focus_group(settings_cont, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS, sysmenu_brightness_click_callback, target_obj);
     // 设置当前页面的按键处理器
     set_current_page_handler(handle_grid_navigation);
 
     // Update current screen layout.
     lv_obj_update_layout(brightness_scr);
 
     // // Init events for screen.
     // events_init_screen_SettingResolution(ui);
 }
 
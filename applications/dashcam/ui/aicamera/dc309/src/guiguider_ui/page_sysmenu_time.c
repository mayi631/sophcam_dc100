/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
// #define DEBUG
#include "config.h"
#include "custom.h"
#include "gui_guider.h"
#include "lvgl.h"
#include "page_all.h"
#include <stdio.h>
// #include "time_t.h"
#include "filemng.h"
#include "indev.h"
#include "style_common.h"
#include "time.h"
#include "ui_common.h"

#define GRID_COLS 1
#define GRID_ROWS 3
#define GRID_MAX_OBJECTS GRID_ROWS * GRID_COLS
#define TIME_NUM 6
static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];

lv_obj_t *obj_sysMenu_Time_s;   //底层窗口
lv_obj_t *setTimeAndDate_Win_s; //设置时间和日期控件

lv_obj_t *setScroll_Float_s;     //滚轮设置浮窗
lv_obj_t *time_Roller_s;         //滚轮控件
static lv_obj_t *label_num_s[TIME_NUM]; //数字文本控件

char num_s[TIME_NUM][5] = {"2025", "01", "01", "00", "00", "00"}; //数字文本
const char *sysmenu_time_btn_labels[] = {"开启", "关闭", "时间设置"};

extern char g_sysbtn_labelTime[32];

static uint8_t timeSettingSelect_Index_s = 0; //选择什么设置进行设置.年月日十分秒

static uint8_t timeSelect_Index_s = 0; //选择第几个按钮(时间标志开启关闭 时间设置)

static void sysMenu_TimeAndData_Setting_Create(void);
void sysMenu_Time_Scroll_Create(uint8_t index);

uint8_t getSelect_Index(void)
{
    return timeSelect_Index_s/2;
}

void SettimeSelect_Index(int32_t index)
{
    if (index == 0) {
        timeSelect_Index_s = 2;
    } else if (index == 1) {
        timeSelect_Index_s = 0;
    }
}

// 月份天数表（非闰年）
static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 判断是否为闰年
static bool is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取某年某月的天数
static uint8_t get_days_in_month(int year, int month)
{
    if(month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days_in_month[month - 1];
}

// 字符串转年份 (uint16_t)
uint16_t str_to_year(const char *str)
{
    uint16_t year = 0;
    while(*str >= '0' && *str <= '9') {
        year = year * 10 + (*str++ - '0');
    }
    return year; // 直接返回年份值
}

// 创建年份选项字符串（直接返回静态缓冲区）
static const char *get_year_options(void)
{
    // 静态缓冲区（避免动态内存分配）
    static char buffer[2200]; // 201年 * 5字符/年 + 200换行符 ≈ 2200字节

    char *ptr = buffer;
    for(int year = 1900; year <= 2100; year++) {
        // 直接写入年份（4位数字）
        *ptr++ = '0' + (year / 1000);
        *ptr++ = '0' + ((year / 100) % 10);
        *ptr++ = '0' + ((year / 10) % 10);
        *ptr++ = '0' + (year % 10);

        // 添加换行符（除了最后一个年份）
        if(year < 2100) *ptr++ = '\n';
    }
    *ptr = '\0'; // 结束符

    return buffer;
}

// 创建天数选项字符串（直接返回静态缓冲区）
static const char *get_month_options(void)
{
    const char *txt1 = lv_label_get_text(label_num_s[0]); //获取年份字符串
    uint16_t year    = str_to_year(txt1);
    const char *txt2 = lv_label_get_text(label_num_s[1]); //获取月份字符串
    uint16_t month   = str_to_year(txt2);
    uint8_t days     = get_days_in_month(year, month); //获取某年某月的天数
    MLOG_DBG("%d   %d   %d\n", year, month, days);
    // 静态缓冲区（避免动态内存分配）
    static char buffer[200]; //最大31天，每天两个字节+换行符

    char *ptr = buffer;
    for(int year = 0; year <= days; year++) {
        // 直接写入月份（2位数字）
        *ptr++ = '0' + ((year / 10) % 10);
        *ptr++ = '0' + (year % 10);

        // 添加换行符（除了最后一个年份）
        if(year < days) *ptr++ = '\n';
    }
    *ptr = '\0'; // 结束符

    return buffer;
}

static void time_Del_Complete_anim_cb(lv_anim_t *a)
{
    if(obj_sysMenu_Time_s != NULL) {
        if(lv_obj_is_valid(obj_sysMenu_Time_s)) {
            lv_obj_del(obj_sysMenu_Time_s);
        } else {
        }
        obj_sysMenu_Time_s = NULL;
        ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                              false, true);
    }
}

//最底层窗口删除
static void time_win_Delete_anim(void)
{
    lv_anim_t Delete_anim; //动画渐隐句柄
    // 创建透明度动画
    lv_anim_init(&Delete_anim);
    lv_anim_set_values(&Delete_anim, 0, 1);

    lv_anim_set_time(&Delete_anim, 6);

    // lv_anim_set_exec_cb(&Delete_anim, AIanim_objSet_Opa);
    lv_anim_set_path_cb(&Delete_anim, lv_anim_path_ease_out);
    // 设置动画完成回调（销毁对象）
    lv_anim_set_completed_cb(&Delete_anim, time_Del_Complete_anim_cb);

    lv_anim_start(&Delete_anim);
}

static void sysMenu_TimeAndData_Set_Del(void) //删除设置浮窗
{
    if(setTimeAndDate_Win_s != NULL) {
        if(lv_obj_is_valid(setTimeAndDate_Win_s)) {
            // MLOG_DBG("Floating_Win_s 仍然有效，删除旧对象\n");
            lv_obj_del(setTimeAndDate_Win_s);
        } else {
            // MLOG_DBG("Floating_Win_s 已被自动销毁，仅重置指针\n");
        }
        setTimeAndDate_Win_s = NULL;
    }
}

static void sysMenu_setScroll_DeleteFloat(void) //删除轮滚浮窗
{
    if(setScroll_Float_s != NULL) {
        if(lv_obj_is_valid(setScroll_Float_s)) {
            // MLOG_DBG("Floating_Win_s 仍然有效，删除旧对象\n");
            lv_obj_del(setScroll_Float_s);
        } else {
            // MLOG_DBG("Floating_Win_s 已被自动销毁，仅重置指针\n");
        }
        setScroll_Float_s = NULL;
    }
}

static void sysMenu_TimeAndData_Setting_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // lv_obj_t *cont        = lv_event_get_user_data(e); // 获取容器对象
    lv_obj_t *btn_clicked = lv_event_get_target(e);
    lv_obj_t *cont        = lv_event_get_user_data(e);
    lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取点击事件的父控件
    parent                = lv_obj_get_parent(parent);      //获取父控件的父控件

    switch(code) {
        case LV_EVENT_CLICKED: {
            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                if(lv_obj_get_child(parent, i) == cont) {
                    sysMenu_Time_Scroll_Create(i);
                    timeSettingSelect_Index_s = i;
                }
            }

        }; break;
        default: break;
    }
}

static void sysMenu_Time_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            if(setTimeAndDate_Win_s) {
                sysMenu_TimeAndData_Set_Del();
            } else {
                ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0,
                                      0, false, true);
            }
            break;
        }
        default: break;
    }
}

void syamenu_TimeSet_SelectFocus_OK(lv_event_t *e)
{

    lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
    lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
    //获取焦点控件
    lv_obj_t *chlid = lv_obj_get_child(parent, timeSelect_Index_s);

    for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
        if(i == timeSelect_Index_s) {
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

static void sysMenu_Time_Select_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);
            lv_obj_t *cont        = lv_event_get_user_data(e); // 获取容器对象
            if(!cont) return;                                  // 如果容器为空则返回
            static uint8_t index = 0;                          //选择第三项则不用设置文本

            lv_obj_t *child;
            for(child = lv_obj_get_child(cont, 0); child != NULL;
                child = lv_obj_get_child(cont, lv_obj_get_index(child) + 1)) {
                index++;
                if(lv_obj_check_type(child, &lv_button_class)) {
                    if(child == btn_clicked) {
                        if(index == 5) //选择设置
                        {
                            sysMenu_TimeAndData_Setting_Create(); //创建设置页面
                        } else {
                            if (timeSelect_Index_s == (index - 1)) {
                                // 如果点击的是当前选中的按钮，则不做任何操作
                                break;
                            }
                            timeSelect_Index_s = index - 1;
                            syamenu_TimeSet_SelectFocus_OK(e);

                            // 获取按钮标签文本
                            lv_obj_t *label = lv_obj_get_child(child, 0);
                            if(label && lv_obj_check_type(label, &lv_label_class)) //时间日期设置不用获取字符串
                            {
                                const char *txt = lv_label_get_text(label);
                                if(txt) strncpy(g_sysbtn_labelTime, txt, sizeof(g_sysbtn_labelTime) - 1);

                                MLOG_DBG("event: %s, timeSelect_Index_s:%d\n", txt, timeSelect_Index_s);
                                MESSAGE_S Msg = {0};
                                Msg.topic = EVENT_MODEMNG_SETTING;
                                Msg.arg1 = PARAM_MENU_OSD_STATUS;
                                Msg.arg2 = (timeSelect_Index_s == 0) ? 1:0;
                                MODEMNG_SendMessage(&Msg);
                            }
                            lv_obj_add_state(child, LV_STATE_PRESSED);
                            lv_obj_set_style_border_color(child, lv_color_hex(0xFF0000), LV_PART_MAIN);
                            time_win_Delete_anim();
                        }
                    } else {
                        lv_obj_clear_state(child, LV_STATE_PRESSED);
                        lv_obj_set_style_border_color(child, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                        // time_win_Delete_anim();
                    }
                }
            }
            index = 0;
            break;
        }
        default: break;
    }
}

//时间设置确定
static void sysMenu_TimeSetting_Select_OK_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            char str[5];
            lv_roller_get_selected_str(time_Roller_s, str, sizeof(str));
            strncpy(num_s[timeSettingSelect_Index_s], str, sizeof(num_s[timeSettingSelect_Index_s]));
            lv_label_set_text(label_num_s[timeSettingSelect_Index_s], num_s[timeSettingSelect_Index_s]);
            lv_obj_update_layout(obj_sysMenu_Time_s);
            sysMenu_setScroll_DeleteFloat();
            // 构建日期时间字符串
            char datetime[50];
            snprintf(datetime, sizeof(datetime), "%s-%s-%s %s:%s:%s",
                     num_s[0], num_s[1], num_s[2], num_s[3], num_s[4], num_s[5]);

            // 构建命令
            char cmd[100];
            snprintf(cmd, sizeof(cmd), "date -s \"%s\"", datetime);

            MLOG_DBG("执行命令: %s\n", cmd);
            system(cmd);
            // // 将系统时间写入硬件时钟
            // system("hwclock -w");

        }; break;
        default: break;
    }
}

//时间设置取消
static void sysMenu_TimeSetting_Select_Cancel_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            sysMenu_setScroll_DeleteFloat();
        }; break;
        default: break;
    }
}

//滚轮设置页
void sysMenu_Time_Scroll_Create(uint8_t index)
{
   // 创建浮动窗口
    setScroll_Float_s = lv_obj_create(setTimeAndDate_Win_s);
    lv_obj_remove_style_all(setScroll_Float_s);
    lv_obj_set_size(setScroll_Float_s, 500, 250); // 更紧凑的尺寸
    lv_obj_align(setScroll_Float_s, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(setScroll_Float_s, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_bg_color(setScroll_Float_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(setScroll_Float_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(setScroll_Float_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(setScroll_Float_s, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(setScroll_Float_s, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(setScroll_Float_s, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(setScroll_Float_s, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(setScroll_Float_s, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加标题
    const char *titles[] = {str_language_year[get_curr_language()], str_language_month[get_curr_language()], str_language_date[get_curr_language()], str_language_hour[get_curr_language()], str_language_minute[get_curr_language()], str_language_second[get_curr_language()]};
    lv_obj_t *title_label = lv_label_create(setScroll_Float_s);
    lv_label_set_text(title_label, titles[index]);
    lv_obj_set_style_text_font(title_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 26), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10);

    // 创建滚轮容器
    lv_obj_t *roller_cont = lv_obj_create(setScroll_Float_s);
    lv_obj_remove_style_all(roller_cont);
    lv_obj_set_size(roller_cont, 200, 140);
    lv_obj_align(roller_cont, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_style_bg_opa(roller_cont, LV_OPA_0, LV_PART_MAIN);

    // 创建滚轮
    time_Roller_s = lv_roller_create(roller_cont);
    lv_obj_set_size(time_Roller_s, 200, 140);
    lv_obj_set_style_text_font(time_Roller_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 24), LV_PART_MAIN);
    lv_obj_set_style_text_font(time_Roller_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 32), LV_PART_SELECTED);
    lv_obj_set_style_bg_color(time_Roller_s, lv_color_hex(0x020524), LV_PART_MAIN);
    lv_obj_set_style_bg_color(time_Roller_s, lv_color_hex(0x404040), LV_PART_SELECTED);
    lv_obj_set_style_text_color(time_Roller_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_color(time_Roller_s, lv_color_hex(0xFFD600), LV_PART_SELECTED);
    lv_obj_set_style_anim_time(time_Roller_s, 300, LV_PART_MAIN);
    lv_obj_set_style_text_line_space(time_Roller_s, 30, LV_PART_MAIN);
    lv_obj_center(time_Roller_s);

    // 获取当前日期
    time_t now         = time(NULL);
    struct tm *tm_info = localtime(&now);
    int current_year   = tm_info->tm_year + 1900;
    int current_month  = tm_info->tm_mon + 1;
    int current_day    = tm_info->tm_mday;
    int current_hour   = tm_info->tm_hour;
    int current_min    = tm_info->tm_min;
    int current_sec    = tm_info->tm_sec;
    const char *str    = NULL;

    switch(index) {
        case 0: { // 年
            str = get_year_options();
            lv_roller_set_options(time_Roller_s, str, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_selected(time_Roller_s, current_year - 1900, LV_ANIM_OFF);
            break;
        }
        case 1: { // 月
            str = "01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12";
            lv_roller_set_options(time_Roller_s, str, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_selected(time_Roller_s, current_month - 1, LV_ANIM_OFF);
            break;
        }
        case 2: { // 日
            str = get_month_options();
            lv_roller_set_options(time_Roller_s, str, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_selected(time_Roller_s, current_day - 1, LV_ANIM_OFF);
            break;
        }
        case 3: { // 时
            str = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23";
            lv_roller_set_options(time_Roller_s, str, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_selected(time_Roller_s, current_hour, LV_ANIM_OFF);
            break;
        }
        case 4: { // 分
            str = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n"
                  "25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
                  "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";
            lv_roller_set_options(time_Roller_s, str, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_selected(time_Roller_s, current_min, LV_ANIM_OFF);
            break;
        }
        case 5: { // 秒
            str = "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n"
                  "25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
                  "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";
            lv_roller_set_options(time_Roller_s, str, LV_ROLLER_MODE_NORMAL);
            lv_roller_set_selected(time_Roller_s, current_sec, LV_ANIM_OFF);
            break;
        }
    }

    // 创建按钮容器
    lv_obj_t *btn_cont = lv_obj_create(setScroll_Float_s);
    lv_obj_remove_style_all(btn_cont);
    lv_obj_set_size(btn_cont, 300, 60);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(btn_cont, 20, 0);
    lv_obj_set_style_pad_ver(btn_cont, 0, 0);
    lv_obj_set_style_pad_gap(btn_cont, 20, 0);

    // 创建取消按钮
    lv_obj_t *btn_cancel = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_cancel, 120, 50);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x505050), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_cancel, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, str_language_cancel[get_curr_language()]);
    lv_obj_set_style_text_font(label_cancel, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_cancel);

    lv_obj_add_event_cb(btn_cancel, sysMenu_TimeSetting_Select_Cancel_event_handler, LV_EVENT_CLICKED, NULL);

    // 创建确定按钮
    lv_obj_t *btn_ok = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_ok, 120, 50);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xFFC000), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_ok, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label_ok = lv_label_create(btn_ok);
    lv_label_set_text(label_ok, str_language_confirm[get_curr_language()]);
    lv_obj_set_style_text_font(label_ok, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_ok, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_ok);

    lv_obj_add_event_cb(btn_ok, sysMenu_TimeSetting_Select_OK_event_handler, LV_EVENT_CLICKED, NULL);
}

//时间和日期设置  子页面
static void sysMenu_TimeAndData_Setting_Create(void)
{

    time_t now         = time(NULL);
    struct tm *tm_info = localtime(&now);
    int time[TIME_NUM];
    time[0] = tm_info->tm_year + 1900;
    time[1] = tm_info->tm_mon + 1;
    time[2] = tm_info->tm_mday;
    time[3] = tm_info->tm_hour;
    time[4] = tm_info->tm_min;
    time[5] = tm_info->tm_sec;

    for(uint8_t i = 0; i < TIME_NUM; i++) {
        snprintf(num_s[i], sizeof(num_s[i]), "%02d", time[i]);
    }

    setTimeAndDate_Win_s = lv_obj_create(obj_sysMenu_Time_s);
    lv_obj_remove_style_all(setTimeAndDate_Win_s);
    lv_obj_set_size( setTimeAndDate_Win_s , H_RES, V_RES);
    lv_obj_add_style( setTimeAndDate_Win_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_top (顶部栏)
    lv_obj_t *cont_top = lv_obj_create(setTimeAndDate_Win_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, H_RES, 60);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    lv_obj_t* btn_back = lv_button_create(cont_top);
    lv_obj_set_pos(btn_back, 4, 4);
    lv_obj_set_size(btn_back, 60, 52);
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(label_back, LV_PCT(100));
    lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_add_event_cb(btn_back, sysMenu_Time_btn_back_event_handler, LV_EVENT_CLICKED, NULL);

    // Write codes title (标题)
    lv_obj_t* title = lv_label_create(cont_top);
    lv_label_set_text(title, str_language_time_and_date[get_curr_language()]);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* flex_Float = lv_obj_create(setTimeAndDate_Win_s);
    lv_obj_remove_style_all(flex_Float);
    lv_obj_set_size(flex_Float, H_RES, 300);
    lv_obj_align(flex_Float, LV_ALIGN_DEFAULT, 0, 80);
    lv_obj_set_style_bg_color(flex_Float, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(flex_Float, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_flex_flow(flex_Float, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(flex_Float, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // 创建样式并设置列间隔（间隔）
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_pad_column(&style, 20); // 设置行间隔为10像素
    // 将样式应用到父对象
    lv_obj_add_style(flex_Float, &style, 0);

    const char *words[] = {str_language_year_0[get_curr_language()], str_language_month_0[get_curr_language()], str_language_day_0[get_curr_language()], str_language_hour_0[get_curr_language()], str_language_minute_1[get_curr_language()], str_language_second_1[get_curr_language()]};
    for(uint8_t i = 0; i < TIME_NUM; i++) {
        lv_obj_t *box = lv_obj_create(flex_Float); //创建6个容器
        lv_obj_remove_style_all(box);
        lv_obj_set_size(box, 160, 140);
        lv_obj_set_style_bg_color(box, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(box, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_bg_opa(box, LV_OPA_50, LV_PART_MAIN);

        lv_obj_t *label_words = lv_label_create(box); //创建文字说明
        lv_label_set_text(label_words, words[i]);
        lv_obj_align(label_words, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_text_font(label_words, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label_words, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *btn = lv_button_create(box); //创建按钮
        lv_obj_set_size(btn, 160, 80);
        lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 20, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(btn, sysMenu_TimeAndData_Setting_event_handler, LV_EVENT_CLICKED, box);

        label_num_s[i] = lv_label_create(btn); //创建数字文本控件
        if(!label_num_s[i]) continue;          // 如果标签创建失败则跳过
        lv_label_set_text(label_num_s[i], num_s[i]);
        lv_obj_set_style_text_font(label_num_s[i], get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label_num_s[i], lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_align(label_num_s[i], LV_ALIGN_CENTER, 0, 0);
    }


    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(obj_sysMenu_Time_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

}

static void sysmenu_time_click_callback(lv_obj_t *obj)
{
    MLOG_DBG("sysmenu_time_click_callback\n");
    lv_obj_t *cont        = lv_obj_get_parent(obj); // 获取容器对象
    if(!cont) return;                                  // 如果容器为空则返回
    static uint8_t index = 0;                          //选择第三项则不用设置文本

    lv_obj_t *child;
    for(child = lv_obj_get_child(cont, 0); child != NULL;
        child = lv_obj_get_child(cont, lv_obj_get_index(child) + 1)) {
        index++;
        if(lv_obj_check_type(child, &lv_button_class)) {
            if(child == obj) {
                if(index == 5) //选择设置
                {
                    sysMenu_TimeAndData_Setting_Create(); //创建设置页面
                } else {
                    timeSelect_Index_s = index - 1;
                    //获取焦点控件
                    lv_obj_t *chlid = lv_obj_get_child(cont, timeSelect_Index_s);

                    for(uint8_t i = 0; i < lv_obj_get_child_cnt(cont); i++) {
                        if(i == timeSelect_Index_s) {
                            //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
                            lv_group_focus_obj(chlid);
                            lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
                            //设置焦点标签颜色
                            lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20),
                                    LV_PART_MAIN | LV_STATE_DEFAULT);

                        }

                        if((obj == lv_obj_get_child(cont, i))) {
                            if(lv_obj_get_child(obj, 1) == NULL) {
                                lv_obj_t *label1 = lv_label_create(obj);
                                lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                                lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                                lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                                lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
                            }
                        } else {
                            lv_obj_t *child = lv_obj_get_child(cont, i);
                            if(lv_obj_get_child(child, 1) != NULL) {
                                lv_obj_del(lv_obj_get_child(child, 1));
                            }
                        }
                    }
                    // 获取按钮标签文本
                    lv_obj_t *label = lv_obj_get_child(child, 0);
                    if(label && lv_obj_check_type(label, &lv_label_class)) //时间日期设置不用获取字符串
                    {
                        const char *txt = lv_label_get_text(label);
                        if(txt) strncpy(g_sysbtn_labelTime, txt, sizeof(g_sysbtn_labelTime) - 1);
                    }
                    lv_obj_add_state(child, LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(child, lv_color_hex(0xFF0000), LV_PART_MAIN);
                    time_win_Delete_anim();
                }
            } else {
                lv_obj_clear_state(child, LV_STATE_PRESSED);
                lv_obj_set_style_border_color(child, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                // time_win_Delete_anim();
            }
        }
    }
    index = 0;
}

static void sysmenu_time_menu_callback(void)
{
    MLOG_DBG("sysmenu_time_menu_callback\n");
    ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
        false, true);
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

//时间和日期设置主界面
void sysMenu_Time(lv_ui_t *ui)
{
    // 创建主页面1 容器
    if(obj_sysMenu_Time_s != NULL) {
        if(lv_obj_is_valid(obj_sysMenu_Time_s)) {
            MLOG_DBG("obj_sysMenu_Time_s 仍然有效，删除旧对象\n");
            lv_obj_del(obj_sysMenu_Time_s);
        } else {
            MLOG_DBG("obj_sysMenu_Time_s 已被自动销毁，仅重置指针\n");
        }
        obj_sysMenu_Time_s = NULL;
    }

    // Write codes resscr
    obj_sysMenu_Time_s = lv_obj_create(NULL);
    lv_obj_set_size( obj_sysMenu_Time_s , H_RES, V_RES);
    lv_obj_add_style( obj_sysMenu_Time_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(obj_sysMenu_Time_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // Write codes cont_top (顶部栏)
    lv_obj_t *cont_top = lv_obj_create(obj_sysMenu_Time_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, H_RES, 60);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    lv_obj_t *btn_back = lv_button_create(cont_top);
    lv_obj_set_pos(btn_back, 4, 4);
    lv_obj_set_size(btn_back, 60, 52);
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);


    lv_obj_t *label_back = lv_label_create(btn_back);
    lv_obj_set_style_text_color(label_back, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(label_back, LV_PCT(100));
    lv_obj_set_style_text_font(label_back, &lv_font_SourceHanSerifSC_Regular_30,
                    LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_back, sysMenu_Time_btn_back_event_handler, LV_EVENT_CLICKED, NULL);

    // Write codes title (标题)
    lv_obj_t* title = lv_label_create(cont_top);
    lv_label_set_text(title, str_language_time_and_date[get_curr_language()]);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    // 创建设置选项容器
    lv_obj_t *settings_cont = lv_obj_create(obj_sysMenu_Time_s);
    lv_obj_set_size(settings_cont, 600, MENU_CONT_SIZE);
    lv_obj_align(settings_cont, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_bg_opa(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(settings_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_flex_flow(settings_cont, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(settings_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(settings_cont, 10, 0);
    sysmenu_time_btn_labels[0] = str_language_on[get_curr_language()];
    sysmenu_time_btn_labels[1] = str_language_off[get_curr_language()];
    sysmenu_time_btn_labels[2] = str_language_time_settings[get_curr_language()];

    static lv_point_precise_t line_points_pool[sizeof(sysmenu_time_btn_labels) / sizeof(sysmenu_time_btn_labels[0])][2];
    for(int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_button_create(settings_cont);
        if(!btn) continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
        lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN);

        lv_obj_t *label = lv_label_create(btn);
        if(!label) continue; // 如果标签创建失败则跳过

        lv_label_set_text(label, sysmenu_time_btn_labels[i]);
        lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);

        // 添加事件处理器，传入容器对象作为用户数据
        lv_obj_add_event_cb(btn, sysMenu_Time_Select_btn_event_handler, LV_EVENT_ALL, settings_cont);

        if(i == timeSelect_Index_s / 2) {
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
    lv_obj_t *chlid = lv_obj_get_child(settings_cont, timeSelect_Index_s);
    lv_group_focus_obj(chlid);
    lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
    lv_obj_scroll_to_y(settings_cont, ((timeSelect_Index_s / 2) * MENU_BTN_SIZE), LV_ANIM_OFF);
    //设置焦点渐变
    // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
    // //设置焦点BG
    // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    //设置焦点标签颜色
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_color(lv_obj_get_child(chlid,1), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(obj_sysMenu_Time_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *target_obj = lv_obj_get_child(settings_cont, timeSelect_Index_s);
    // 初始化焦点组
    init_focus_group(settings_cont, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS, sysmenu_time_click_callback, target_obj);
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(sysmenu_time_menu_callback);

    // Update current screen layout.
    lv_obj_update_layout(obj_sysMenu_Time_s);
}

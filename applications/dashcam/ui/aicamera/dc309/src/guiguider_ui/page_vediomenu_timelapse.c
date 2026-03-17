#define DEBUG
#include "gui_guider.h"
#include "lvgl.h"
#include "style_common.h"
#include "ui_common.h"
#include <stdio.h>
// #include "events_init.h"
#include "common/takephoto.h"
#include "config.h"
#include "custom.h"
#include "indev.h"
#include "page_all.h"
#include <math.h>

#define GRID_COLS 1
#define GRID_ROWS 5
#define GRID_MAX_OBJECTS GRID_COLS * GRID_ROWS
static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];
static bool is_key_click = false;
static void vedioTimeLapse_click_callback(lv_obj_t *obj);
static void vedioTimeLapse_menu_callback(void);

static bool is_timelapse_active       = false; // 缩时录影是否激活
static uint8_t scheduled_start_hour   = 0;     //开始时间
static uint8_t scheduled_start_min    = 0;     //开始
static uint8_t scheduled_end_hour     = 0;      //结束时间
static uint8_t scheduled_end_min      = 0;
static uint8_t scheduled_interval_min = 0;     // 间隔时间
static uint32_t g_total_shot_count    = 0;     // 总张数

extern bool user_manual_stop;

// 全局变量声明
static timeLapseButton_t buttons[] = {
    {0, NULL, NULL, NULL, true},  // 复选框
    {1, NULL, NULL, NULL, false}, // 开始时间
    {2, NULL, NULL, NULL, false}, // 结束时间
    {3, NULL, NULL, NULL, false}, // 时间间隔
    {4, NULL, NULL, NULL, false}  // 拍摄张数
};

#define BUTTON_COUNT (sizeof(buttons) / sizeof(buttons[0]))

// 全局变量声明
lv_obj_t *obj_Vedio_TimeLapse_s; // 底层窗口
lv_obj_t *cont_settings_s;       // 设置选项控件
lv_obj_t *Floating_Win_s;        // 滚轮控件底层浮窗
lv_obj_t *roller_hour_s;
lv_obj_t *roller_min_s;
static lv_obj_t *lines[BUTTON_COUNT]; // 保存分割线对象
static lv_obj_t *kb_input = NULL;
static lv_obj_t *ta_input = NULL;
// 新增变量
static char shot_count_label[20] = "1500"; // 拍摄张数的字符串表示
static bool start_time_set       = false;  // 开始时间设置标志
static bool end_time_set         = false;  // 结束时间设置标志

static uint8_t Select_Index_s    = 0; // 选择设置的ID
static char Time_labels_s[3][20] = {"00:00", "00:00", "00:00"};
extern uint8_t is_start_video;          // 录像状态
static bool use_scheduled_time = false; // 是否使用定时拍摄

void delete_kb_ta_cb(void)
{
    if(kb_input) {
        lv_obj_del_async(kb_input);
        kb_input = NULL;
    }
    if(ta_input) {
        lv_obj_del_async(ta_input);
        ta_input = NULL;
    }
}

void set_use_scheduled_mode(timeLapse_mode mode)
{
    use_scheduled_time = mode;
}

static uint32_t get_shot_count_value(void)
{
    char *endptr;
    unsigned long value = strtoul(shot_count_label, &endptr, 10);

    // 检查转换是否有效
    if (*endptr != '\0' ||        // 存在非法字符
        endptr == shot_count_label || // 空字符串
        value > UINT16_MAX)       // 超过uint16_t范围
    {
        MLOG_ERR("Invalid shot count: %s", shot_count_label);
        return 1500; // 返回默认值
    }

    return (uint32_t)value;
}

// 计算总张数
static void calculate_total_shot_count(void)
{
    // 计算拍照张数（加1是包含开始时刻的第一张）
    g_total_shot_count = get_shot_count_value();
}

// 添加假函数实现（实际项目中需要替换为真实实现）
static uint8_t get_current_hour(void)
{
    // 这里需要实现获取当前小时的函数
    // 假代码：返回固定值用于测试
    time_t now   = time(NULL);
    struct tm *t = localtime(&now);

    return t->tm_hour;
}

static uint8_t get_current_minute(void)
{
    // 这里需要实现获取当前分钟的函数
    // 假代码：返回固定值用于测试
    time_t now   = time(NULL);
    struct tm *t = localtime(&now);

    return t->tm_min; // 假设当前是30分
}

static void start_timelapse_recording(uint8_t interval_min)
{
    //先切换到录像模式
    extern bool is_video_mode;
    MESSAGE_S Msg  = {0};
    // MLOG_DBG("开始缩时录像 是否是录像模式：%d\n",is_video_mode);
    is_video_mode = true;
    Msg.topic     = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1      = WORK_MODE_MOVIE;
    MODEMNG_SendMessage(&Msg);
    // 复位缩放
    set_zoom_level(1);
    ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, NULL, Home_Vedio, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);

    //缩时模式
    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = PARAM_MENU_RECORD_MODE;
    Msg.arg2 = 1;
    MODEMNG_SendMessage(&Msg);

    //开始
    is_start_video = VEDIO_START;
    Msg.topic      = EVENT_MODEMNG_START_REC;
    MODEMNG_SendMessage(&Msg);
    MLOG_DBG("Starting timelapse recording with interval: %d minutes\n", interval_min);
}

static void stop_timelapse_recording(void)
{
    is_start_video = VEDIO_STOP; // 停止录像录像
    MESSAGE_S Msg  = {0};
    Msg.topic      = EVENT_MODEMNG_STOP_REC;
    MODEMNG_SendMessage(&Msg);

    start_time_set = false;
    end_time_set = false;
    MLOG_DBG("Stopping timelapse recording\n");
}

// 添加新的函数来检查并启动缩时录影
void check_and_start_timelapse(void)
{
    // 获取当前时间（这里需要实现获取当前时间的函数）
    uint8_t current_hour   = get_current_hour();   // 需要实现获取当前小时的函数
    uint8_t current_minute = get_current_minute(); // 需要实现获取当前分钟的函数
    // 将当前时间转换为分钟数
    uint32_t current_total_min = current_hour * 60 + current_minute;
    uint32_t start_total_min   = scheduled_start_hour * 60 + scheduled_start_min;
    uint32_t end_total_min     = scheduled_end_hour * 60 + scheduled_end_min;

    // 检查当前时间是否在时间范围内
    bool in_time_range = false;
    if(start_total_min <= end_total_min) {
        in_time_range = (current_total_min >= start_total_min && current_total_min < end_total_min);
    } else {
        in_time_range = (current_total_min >= start_total_min || current_total_min < end_total_min);
    }

    // MLOG_DBG("use_scheduled_time:%d   %d\n",use_scheduled_time,getgraphy_mode_Index());
    if(getgraphy_mode_Index() == 1 && use_scheduled_time) { //使用定时开始和定时结束
        // 如果在时间范围内且间隔时间有效
        // MLOG_DBG("in_time_range:%d   scheduled_interval_min:%d  is_start_video:%d\n",in_time_range,scheduled_interval_min,is_start_video);
        if(in_time_range && scheduled_interval_min > 0) {
            if(!is_timelapse_active) {
                if(is_start_video == VEDIO_STOP) {
                    start_timelapse_recording(scheduled_interval_min);
                }
                is_timelapse_active = true;
            }
        } else if(is_timelapse_active) {
            if(is_start_video == VEDIO_START && (!in_time_range)) // 开始录像了，并且不在时间返回内
            {
                stop_timelapse_recording();
                is_timelapse_active = false;
            }
        }

    } else if(getgraphy_mode_Index() == 2 && !use_scheduled_time) //定时拍照模式
    {
        static uint32_t curr_shot_count = 0; // 当前拍照张数
        static uint32_t curr_start_time = 0; // 当前计数时间
        // MLOG_DBG("is_start_video:%d\n",is_start_video);
        if(is_start_video == VEDIO_START) {
            curr_start_time++;
            calculate_total_shot_count();
            // MLOG_DBG("curr_start_time:%d  scheduled_interval_min:%d   %d\n",curr_start_time,scheduled_interval_min,curr_start_time % scheduled_interval_min);
            if(curr_start_time % scheduled_interval_min == 0) {//根据间隔时间计算当前的拍照张数
                curr_shot_count++;
                // MLOG_DBG("curr_shot_count:%d  g_total_shot_count:%d\n",curr_shot_count,g_total_shot_count);
                if(curr_shot_count >= g_total_shot_count) {
                    is_start_video = VEDIO_STOP; // 停止录像录像
                    MESSAGE_S Msg  = {0};
                    Msg.topic      = EVENT_MODEMNG_STOP_REC;
                    MODEMNG_SendMessage(&Msg);
                }
            }
        }
    }
}

// 更新按钮状态（启用/禁用）
static void update_buttons_state(void)
{
    extern char g_vediobtn_labelGraphy[32];
    if(use_scheduled_time) {
        // 使用定时拍摄 - 启用开始/结束时间，时间间隔，禁用拍摄张数
        lv_obj_clear_flag(buttons[1].btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(buttons[2].btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(buttons[3].btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(buttons[4].btn, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(lines[0], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lines[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lines[2], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lines[3], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lines[4], LV_OBJ_FLAG_HIDDEN);

        lv_obj_align(buttons[3].btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * 3);
        lv_obj_align(buttons[4].btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * 4);
        //设置模式
        setgraphy_mode_Index(1);
        //更新文本
        strncpy(g_vediobtn_labelGraphy, str_language_timelapse_photography[get_curr_language()], sizeof(g_vediobtn_labelGraphy));

    } else {
        // 不使用定时拍摄 - 禁用开始/结束时间，启用时间间隔和拍摄张数
        lv_obj_add_flag(buttons[1].btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(buttons[2].btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(buttons[3].btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(buttons[4].btn, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(lines[0], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lines[1], LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(lines[2], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lines[3], LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(lines[4], LV_OBJ_FLAG_HIDDEN);

        // 移动时间间隔和拍照张数按钮回原来的位置
        lv_obj_align(buttons[3].btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * 1);
        lv_obj_align(buttons[4].btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * 2);
        //使用定时拍摄，就把设置时间标志置0，设置完成再1
        start_time_set = false;
        end_time_set = false;
        //设置模式
        setgraphy_mode_Index(2);
        //更新文本
        strncpy(g_vediobtn_labelGraphy, str_language_timed_photo[get_curr_language()], sizeof(g_vediobtn_labelGraphy));

    }

    // 更新复选框状态
    if(use_scheduled_time) {
        lv_obj_add_state(lv_obj_get_child(buttons[0].btn, 0), LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(lv_obj_get_child(buttons[0].btn, 0), LV_STATE_CHECKED);
    }
}


// 复选框点击事件处理
static void checkbox_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        use_scheduled_time = !use_scheduled_time;
        update_buttons_state();
    }
}

static void kb_ok_event_handler(lv_event_t *e)
{
    lv_obj_t *kb = lv_event_get_target(e);
    lv_obj_t *ta = lv_keyboard_get_textarea(kb);

    // 获取输入的数值
    const char *text = lv_textarea_get_text(ta);
    int count        = atoi(text);

    // 验证输入有效性
    if(count > 0 && count <= 99999) {
        snprintf(shot_count_label, sizeof(shot_count_label), "%d", count);
        lv_label_set_text(buttons[4].right_label, shot_count_label);
    }
    // 删除键盘和文本区域
    delete_kb_ta_cb();
}

static void kb_cancel_event_handler(lv_event_t *e)
{
    delete_kb_ta_cb();
}

// 拍摄张数设置回调
static void shot_count_set_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        delete_kb_ta_cb();//先删除再创建
        // 创建数字键盘
        kb_input = lv_keyboard_create(obj_Vedio_TimeLapse_s);
        lv_obj_set_size(kb_input, 640, 240);
        lv_obj_align(kb_input, LV_ALIGN_BOTTOM_MID, 0, 0);
        // 设置为数字键盘模式
        lv_keyboard_set_mode(kb_input, LV_KEYBOARD_MODE_NUMBER);

        // 创建文本区域
        ta_input = lv_textarea_create(obj_Vedio_TimeLapse_s);
        lv_obj_set_size(ta_input, 300, 50);
        lv_obj_align(ta_input, LV_ALIGN_TOP_MID, 0, 50);
        lv_textarea_set_text(ta_input, shot_count_label);
        lv_textarea_set_one_line(ta_input, true);
        lv_textarea_set_max_length(ta_input, 5); // 最大5位数

        // 关联键盘和文本区域
        lv_keyboard_set_textarea(kb_input, ta_input);

        // 添加确定按钮回调
        lv_obj_add_event_cb(kb_input, kb_ok_event_handler, LV_EVENT_READY, NULL);

        // 添加取消按钮回调
        lv_obj_add_event_cb(kb_input, kb_cancel_event_handler, LV_EVENT_CANCEL, NULL);

        // 简化键盘样式
        lv_obj_set_style_bg_color(kb_input, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
        lv_obj_set_style_bg_color(kb_input, lv_color_hex(0x404040), LV_PART_ITEMS);
        lv_obj_set_style_text_color(kb_input, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);
        lv_obj_set_style_text_color(kb_input, lv_color_hex(0), LV_PART_ITEMS | LV_STATE_CHECKED);
    }
}

void timeLapse_Set_focusColor(void)
{
    for(uint8_t i = 0; i < BUTTON_COUNT; i++) {
        if(i == Select_Index_s) {
            lv_group_focus_obj(buttons[i].btn);
            lv_obj_add_state(buttons[i].btn, LV_STATE_FOCUS_KEY);

            if(buttons[i].is_checkbox) {
                lv_obj_t *checkbox = lv_obj_get_child(buttons[i].btn, 0);
                lv_obj_set_style_text_color(checkbox, lv_color_hex(0xF09F20),
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
            } else {
                lv_obj_set_style_text_color(buttons[i].left_label, lv_color_hex(0xF09F20),
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(buttons[i].right_label, lv_color_hex(0xF09F20),
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        } else {
            if(buttons[i].is_checkbox) {
                lv_obj_t *checkbox = lv_obj_get_child(buttons[i].btn, 0);
                lv_obj_set_style_text_color(checkbox, lv_color_hex(0xFFFFFF),
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
            } else {
                lv_obj_set_style_text_color(buttons[i].left_label, lv_color_hex(0xFFFFFF),
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(buttons[i].right_label, lv_color_hex(0xFFFFFF),
                                           LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }
}

static void timeLapse_Del_Complete_anim_cb(lv_anim_t* a)
{
    delete_kb_ta_cb();
    ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
        true);
}

static void timeLapse_win_Delete_anim(void)
{
    lv_anim_t Delete_anim; //动画渐隐句柄
    // 创建透明度动画
    lv_anim_init(&Delete_anim);
    lv_anim_set_values(&Delete_anim, 0, 1);

    lv_anim_set_time(&Delete_anim, 6);

    // lv_anim_set_exec_cb(&Delete_anim, AIanim_objSet_Opa);
    lv_anim_set_path_cb(&Delete_anim, lv_anim_path_ease_out);
    // 设置动画完成回调（销毁对象）
    lv_anim_set_completed_cb(&Delete_anim, timeLapse_Del_Complete_anim_cb);

    lv_anim_start(&Delete_anim);
}

static void vedioTimeLapse_DeleteFloat(void)
{
    if(Floating_Win_s != NULL) {
        if(lv_obj_is_valid(Floating_Win_s)) {
            // MLOG_DBG("Floating_Win_s 仍然有效，删除旧对象\n");
            lv_obj_del(Floating_Win_s);
        } else {
            // MLOG_DBG("Floating_Win_s 已被自动销毁，仅重置指针\n");
        }
        Floating_Win_s = NULL;
    }
}

static void vedioTimeLapse_TimeSelect_OK_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    if(code != LV_EVENT_CLICKED) return;

    // 确保索引有效
    switch(Select_Index_s) {
        case 1: // 开始时间设置
        {
            uint8_t hour = lv_roller_get_selected(roller_hour_s);
            uint8_t min  = lv_roller_get_selected(roller_min_s);
            snprintf(Time_labels_s[0], sizeof(Time_labels_s[0]), "%02d:%02d", hour, min);
            vedioTimeLapse_DeleteFloat();
            lv_label_set_text(buttons[1].right_label, Time_labels_s[0]);
            scheduled_start_hour = hour;
            scheduled_start_min = min;
            // 标记开始时间已设置
            start_time_set = true;
            is_timelapse_active = false;//重新设置时间，那么就需要重新激活
        } break;

        case 2: // 结束时间设置
        {
            uint8_t hour = lv_roller_get_selected(roller_hour_s);
            uint8_t min  = lv_roller_get_selected(roller_min_s);
            snprintf(Time_labels_s[1], sizeof(Time_labels_s[1]), "%02d:%02d", hour, min);
            vedioTimeLapse_DeleteFloat();
            lv_label_set_text(buttons[2].right_label, Time_labels_s[1]);
            scheduled_end_hour = hour;
            scheduled_end_min = min;
            // 标记结束时间已设置
            end_time_set = true;
        } break;

        case 3: // 间隔设置
        {
            uint8_t min = lv_roller_get_selected(roller_hour_s);
            uint8_t sec  = lv_roller_get_selected(roller_min_s);
            char Temp_Time_labels_s[20] = {"00:00"};
            snprintf(Temp_Time_labels_s, sizeof(Temp_Time_labels_s), "%02d:%02d", min, sec);
            scheduled_interval_min = min * 60 + sec;//时间间隔时是分和秒
            if(strcmp(Temp_Time_labels_s, Time_labels_s[2]) == 0) {
                // 如果没有变化则不处理
            } else if(min * 60 + sec < 1) {
                MLOG_DBG("不支持时间间隔为0s\n");
            } else {
                is_start_video = 0; // 停止录像
                // 发送消息设置间隔时间
                MESSAGE_S Msg = {0};
                Msg.topic = EVENT_MODEMNG_SETTING;
                Msg.arg1 = PARAM_MENU_LAPSE_RECORD_TIME;
                // 转换为秒
                Msg.arg2 = min * 60 + sec; // 这里应该是分秒
                MODEMNG_SendMessage(&Msg);

                snprintf(Time_labels_s[2], sizeof(Time_labels_s[2]), "%02d:%02d", min, sec);
                vedioTimeLapse_DeleteFloat();
                lv_label_set_text(buttons[3].right_label, Time_labels_s[2]);
            }
        } break;
    }

    if(is_key_click) {
        lv_obj_t *target_obj = lv_obj_get_child(cont_settings_s, Select_Index_s);
        init_focus_group(cont_settings_s, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
                         vedioTimeLapse_click_callback, target_obj);
        set_current_page_handler(handle_grid_navigation);
        takephoto_register_menu_callback(vedioTimeLapse_menu_callback);
        is_key_click = false;
    }
}

static void vedioTimeLapse_TimeSelect_Cancel_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            vedioTimeLapse_DeleteFloat();
            if(is_key_click) {
                lv_obj_t *target_obj = lv_obj_get_child(cont_settings_s, Select_Index_s);
                // 初始化焦点组
                init_focus_group(cont_settings_s, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
                                vedioTimeLapse_click_callback, target_obj);
                // 设置当前页面的按键处理器
                set_current_page_handler(handle_grid_navigation);
                takephoto_register_menu_callback(vedioTimeLapse_menu_callback);
                is_key_click = false;
            }
        }; break;
        default: break;
    }
}

void vedioTimeLapse_Scroll_Floating_Create(uint8_t index)
{
    // 创建浮动窗口
    Floating_Win_s = lv_obj_create(obj_Vedio_TimeLapse_s);
    lv_obj_remove_style_all(Floating_Win_s);
    lv_obj_set_size(Floating_Win_s, 500, 300); // 更紧凑的尺寸
    lv_obj_align(Floating_Win_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(Floating_Win_s, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(Floating_Win_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(Floating_Win_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(Floating_Win_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(Floating_Win_s, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(Floating_Win_s, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(Floating_Win_s, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(Floating_Win_s, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(Floating_Win_s, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加标题
    lv_obj_t *title_label = lv_label_create(Floating_Win_s);
    lv_label_set_text(title_label, str_language_time_settings[get_curr_language()]);
    lv_obj_set_style_text_font(title_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 15);

    // 创建小时滚轮容器
    lv_obj_t *hour_cont = lv_obj_create(Floating_Win_s);
    lv_obj_remove_style_all(hour_cont);
    lv_obj_set_size(hour_cont, 100, 160);
    lv_obj_align(hour_cont, LV_ALIGN_CENTER, -70, -10);
    lv_obj_set_style_bg_opa(hour_cont, LV_OPA_0, LV_PART_MAIN);

    // 创建小时标签
    lv_obj_t *hour_label = lv_label_create(hour_cont);
    lv_obj_set_style_text_font(hour_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 18), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(hour_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(hour_label, LV_ALIGN_TOP_MID, 0, 0);

    // 创建小时/分钟滚轮
    roller_hour_s = lv_roller_create(hour_cont);
    if(index < 3) {
        lv_roller_set_options(
            roller_hour_s,
            "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23",
            LV_ROLLER_MODE_INFINITE);
            lv_label_set_text(hour_label, str_language_hour[get_curr_language()]);
    } else {
        lv_roller_set_options(
            roller_hour_s,
            "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n"
            "23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n"
            "46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
            LV_ROLLER_MODE_INFINITE);
            lv_label_set_text(hour_label, str_language_minute_1[get_curr_language()]);
    }
    lv_obj_set_size(roller_hour_s, 100, 120);
    lv_obj_set_style_text_font(roller_hour_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 28), 0);
    lv_obj_set_style_bg_color(roller_hour_s, lv_color_hex(0x020524), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_hour_s, lv_color_hex(0x404040), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_hour_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_hour_s, lv_color_hex(0xFFD600), LV_PART_SELECTED);
    lv_obj_set_style_anim_time(roller_hour_s, 300, LV_PART_MAIN);
    lv_obj_align(roller_hour_s, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 创建分隔符
    lv_obj_t *colon = lv_label_create(Floating_Win_s);
    lv_label_set_text(colon, ":");
    lv_obj_set_style_text_font(colon, get_usr_fonts(ALI_PUHUITI_FONTPATH, 32), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(colon, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(colon, LV_ALIGN_CENTER, 0, -20);

    // 创建分钟滚轮容器
    lv_obj_t *min_cont = lv_obj_create(Floating_Win_s);
    lv_obj_remove_style_all(min_cont);
    lv_obj_set_size(min_cont, 100, 160);
    lv_obj_align(min_cont, LV_ALIGN_CENTER, 70, -10);
    lv_obj_set_style_bg_opa(min_cont, LV_OPA_0, LV_PART_MAIN);

    // 创建分钟标签
    lv_obj_t *min_label = lv_label_create(min_cont);
    lv_obj_set_style_text_font(min_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 18), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(min_label, lv_color_hex(0xAAAAAA), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(min_label, LV_ALIGN_TOP_MID, 0, 0);

    if(index < 3) {
    lv_label_set_text(min_label, str_language_minute_1[get_curr_language()]);
    } else {
    lv_label_set_text(min_label, str_language_second_1[get_curr_language()]);
    }

    // 创建分钟/秒滚轮
    roller_min_s = lv_roller_create(min_cont);
    lv_roller_set_options(roller_min_s,
                          "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n"
                          "23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n"
                          "46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_size(roller_min_s, 100, 120);
    lv_obj_set_style_text_font(roller_min_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 28), 0);
    lv_obj_set_style_bg_color(roller_min_s, lv_color_hex(0x020524), LV_PART_MAIN);
    lv_obj_set_style_bg_color(roller_min_s, lv_color_hex(0x404040), LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_min_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_color(roller_min_s, lv_color_hex(0xFFD600), LV_PART_SELECTED);
    lv_obj_set_style_anim_time(roller_min_s, 300, LV_PART_MAIN);
    lv_obj_align(roller_min_s, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 创建按钮容器
    lv_obj_t *btn_cont = lv_obj_create(Floating_Win_s);
    lv_obj_remove_style_all(btn_cont);
    lv_obj_set_size(btn_cont, 400, 60);
    lv_obj_align(btn_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(btn_cont, 40, 0);
    lv_obj_set_style_pad_ver(btn_cont, 0, 0);
    lv_obj_set_style_pad_gap(btn_cont, 20, 0);

    // 创建取消按钮
    lv_obj_t *btn_cancel = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_cancel, 150, 50);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x505050), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_cancel, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(label_cancel, str_language_cancel[get_curr_language()]);
    lv_obj_set_style_text_font(label_cancel, get_usr_fonts(ALI_PUHUITI_FONTPATH, 22), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_cancel);

    lv_obj_add_event_cb(btn_cancel, vedioTimeLapse_TimeSelect_Cancel_event_handler, LV_EVENT_CLICKED, NULL);

    // 创建确定按钮
    lv_obj_t *btn_ok = lv_btn_create(btn_cont);
    lv_obj_set_size(btn_ok, 150, 50);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xFFC000), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(btn_ok, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label_ok = lv_label_create(btn_ok);
    lv_label_set_text(label_ok, str_language_confirm[get_curr_language()]);
    lv_obj_set_style_text_font(label_ok, get_usr_fonts(ALI_PUHUITI_FONTPATH, 22), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_ok, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(label_ok);

    lv_obj_add_event_cb(btn_ok, vedioTimeLapse_TimeSelect_OK_event_handler, LV_EVENT_CLICKED, NULL);

}

static void vedioTimeLapse_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            delete_kb_ta_cb();
            ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                  false, true);
            break;
        }
        default: break;
    }
}

static void vedioTimeLapse_Set_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code != LV_EVENT_CLICKED) return;
    lv_obj_t *btn_clicked = lv_event_get_target(e);
    // 查找点击的按钮
    for(uint8_t i = 0; i < BUTTON_COUNT; i++) {
        if(btn_clicked == buttons[i].btn) {
            Select_Index_s = i;
            MLOG_DBG("Select_Index_s:%d\n", Select_Index_s);
            // 处理复选框点击
            if(i == 0) {
                checkbox_event_handler(e);
                return; // 处理完成后直接返回
            }
            // 检查按钮是否可用
            if((i == 1 || i == 2) && !use_scheduled_time) {
                return; // 开始/结束时间未启用，不处理
            }
            if((i == 4) && use_scheduled_time) {
                return; // 时间间隔/拍摄张数未启用，不处理
            }
            // 处理拍摄张数按钮
            if(i == 4) {
                shot_count_set_event_handler(e);
            } else {
                vedioTimeLapse_Scroll_Floating_Create(i);
            }
            break;
        }
    }
    timeLapse_Set_focusColor();
}

static void vedioTimeLapse_click_callback(lv_obj_t *obj)
{
    MLOG_DBG("vedioTimeLapse_click_callback\n");
    // 查找点击的按钮
    for(uint8_t i = 0; i < BUTTON_COUNT; i++) {
        if(obj == buttons[i].btn) {
            Select_Index_s = i;
            break;
        }
    }
    is_key_click = true;

    // 移除复选框的特殊处理
    if(Select_Index_s == 0) {
        is_key_click = false;
        return;
    }
    // 检查按钮是否可用
    if((Select_Index_s == 1 || Select_Index_s == 2) && !use_scheduled_time) {
        is_key_click = false;
        return; // 开始/结束时间未启用，不处理
    }
    if((Select_Index_s == 4) && use_scheduled_time) {
        is_key_click = false;
        return; // 时间间隔/拍摄张数未启用，不处理
    }
    // 处理拍摄张数按钮
    if(Select_Index_s == 4) {
        shot_count_set_event_handler(NULL);
        is_key_click = false;
    } else {
        vedioTimeLapse_Scroll_Floating_Create(Select_Index_s);
    }
    timeLapse_Set_focusColor();
}

static void vedioTimeLapse_menu_callback(void)
{
    MLOG_DBG("vedioTimeLapse_menu_callback\n");
    delete_kb_ta_cb();
    ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
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
                    delete_kb_ta_cb();
                    ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0,
                                          0, false, true);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

void vedioMenuSetting_Lapse(lv_ui_t *ui)
{
    // MLOG_DBG("%s[%d]\n",__func__,__LINE__);
    // 创建主页面1 容器
    if(obj_Vedio_TimeLapse_s != NULL) {
        if(lv_obj_is_valid(obj_Vedio_TimeLapse_s)) {
            MLOG_DBG("obj_Vedio_TimeLapse_s 仍然有效，删除旧对象\n");
            lv_obj_del(obj_Vedio_TimeLapse_s);
        } else {
            MLOG_DBG("obj_Vedio_TimeLapse_s 已被自动销毁，仅重置指针\n");
        }
        obj_Vedio_TimeLapse_s = NULL;
    }

    // 创建底层控件
    obj_Vedio_TimeLapse_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_Vedio_TimeLapse_s, H_RES, V_RES);
    lv_obj_add_style(obj_Vedio_TimeLapse_s, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(obj_Vedio_TimeLapse_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // Write codes cont_top (顶部栏)
    lv_obj_t *cont_top = lv_obj_create(obj_Vedio_TimeLapse_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);

    // Write style for cont_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    lv_obj_t* btn_back = lv_button_create(cont_top);
    lv_obj_set_pos(btn_back, 4, 4);
    lv_obj_set_size(btn_back, 60, 52);
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 为返回按钮添加事件处理
    lv_obj_add_event_cb(btn_back, vedioTimeLapse_btn_back_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(label_back, LV_PCT(100));
    lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t* title = lv_label_create(cont_top);
    lv_label_set_text(title, str_language_timelapse_recording_settings[get_curr_language()]);
    lv_label_set_long_mode(title, LV_LABEL_LONG_WRAP);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 70, 50);
    lv_obj_set_style_pad_all(cont_top, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

    // Write codes cont_settings (设置选项容器)
    // 创建设置容器
    cont_settings_s = lv_obj_create(obj_Vedio_TimeLapse_s);
    lv_obj_set_size(cont_settings_s, 600, MENU_CONT_SIZE);
    lv_obj_align(cont_settings_s, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_bg_opa(cont_settings_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont_settings_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont_settings_s, 10, 0);

    // 创建按钮
    const char *btn_labels[] = {str_language_enable_time_range[get_curr_language()], str_language_start_time[get_curr_language()], str_language_end_time[get_curr_language()], str_language_time_interval[get_curr_language()], str_language_number_of_shots[get_curr_language()]};
    static lv_point_precise_t line_points_pool[sizeof(btn_labels) / sizeof(btn_labels[0])][2];

    for(uint8_t i = 0; i < BUTTON_COUNT; i++) {
        // 创建按钮
        buttons[i].btn = lv_button_create(cont_settings_s);
        lv_obj_set_size(buttons[i].btn, 560, MENU_BTN_SIZE);
        lv_obj_align(buttons[i].btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
        lv_obj_set_style_bg_opa(buttons[i].btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(buttons[i].btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(buttons[i].btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(buttons[i].btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(buttons[i].btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 特殊处理复选框
        if(i == 0) {
            lv_obj_t *checkbox = lv_checkbox_create(buttons[i].btn);
            lv_checkbox_set_text(checkbox, btn_labels[i]);
            lv_obj_set_style_text_font(checkbox, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(checkbox, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_align(checkbox, LV_ALIGN_LEFT_MID, 0, 0);
            buttons[i].left_label = NULL;
            buttons[i].right_label = NULL;
            lv_obj_clear_flag(checkbox, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(checkbox, LV_OBJ_FLAG_SCROLLABLE);
        }
        // 其他按钮
        else {
            // 左侧标签
            buttons[i].left_label = lv_label_create(buttons[i].btn);
            lv_label_set_text(buttons[i].left_label, btn_labels[i]);
            lv_obj_set_style_text_font(buttons[i].left_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(buttons[i].left_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_align(buttons[i].left_label, LV_ALIGN_LEFT_MID, 0, 0);

            // 右侧标签
            buttons[i].right_label = lv_label_create(buttons[i].btn);
            if(i == 1) {
                lv_label_set_text(buttons[i].right_label, Time_labels_s[0]);
            } else if(i == 2) {
                lv_label_set_text(buttons[i].right_label, Time_labels_s[1]);
            } else if(i == 3) {
                lv_label_set_text(buttons[i].right_label, Time_labels_s[2]);
            } else if(i == 4) {
                lv_label_set_text(buttons[i].right_label, shot_count_label);
            }
            lv_obj_set_style_text_font(buttons[i].right_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                       LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(buttons[i].right_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_align(buttons[i].right_label, LV_ALIGN_RIGHT_MID, 0, 0);
        }

        // 添加事件处理器
        lv_obj_add_event_cb(buttons[i].btn, vedioTimeLapse_Set_btn_event_handler, LV_EVENT_ALL, NULL);

        // 创建分割线
        lines[i] = lv_line_create(cont_settings_s);
        int y_position = (MENU_BTN_SIZE + 10) * (i + 1) - 4;
        line_points_pool[i][0].x = 10;
        line_points_pool[i][0].y = y_position;
        line_points_pool[i][1].x = 570;
        line_points_pool[i][1].y = y_position;
        lv_line_set_points(lines[i], line_points_pool[i], 2);
        lv_obj_set_style_line_width(lines[i], 2, 0);
        lv_obj_set_style_line_color(lines[i], lv_color_hex(0x5F5F5F), 0);
    }

    //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
    //获取焦点控件
    if(Select_Index_s == 0) {
        Select_Index_s = 1;//如果是复选框，则设置为第一个选项
    }
    lv_obj_t *chlid = lv_obj_get_child(cont_settings_s, Select_Index_s*2);
    lv_group_focus_obj(chlid);
    lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 更新按钮状态
    update_buttons_state();

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(obj_Vedio_TimeLapse_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *target_obj = lv_obj_get_child(cont_settings_s, Select_Index_s*2);
    // 初始化焦点组
    init_focus_group(cont_settings_s, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
                     vedioTimeLapse_click_callback, target_obj);
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(vedioTimeLapse_menu_callback);


}

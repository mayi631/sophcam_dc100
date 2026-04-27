// #define DEBUG
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <math.h>

#include "power.h"
#include "lvgl.h"
#include "mlog.h"
#include "page_all.h"
#include "custom.h"
#include "hal_backlight.h"
#include "time.h"
#include "mode.h"
#include "mapi_ao.h"
#include "hal_pwm.h"
#include "dialogxx.h"

// 电源管理对话框
lv_obj_t *power_dialog_bg = NULL;  // 背景遮罩
lv_obj_t *power_dialog = NULL;     // 对话框容器
lv_obj_t *power_btn_cont = NULL;   // 按钮容器
int power_dialog_selected = 0;     // 当前选中的按钮索引 (0:关机, 1:重启, 2:取消)

// 无操作自动关机管理
extern char g_sysbtn_labelPowerdown[32];
static int auto_poweroff_timeout_seconds = 3 * 60;
static struct timespec last_activity_time = {0, 0};     // 最后一次活动时间（单调时间）
static lv_timer_t *auto_poweroff_timer = NULL;  // 自动关机检查定时器

// 无操作息屏管理
#define SCREEN_TIMEOUT_MINUTES 1          // 1分钟无操作息屏
#define SCREEN_TIMEOUT_SECONDS (SCREEN_TIMEOUT_MINUTES * 60)
static bool screen_is_on = true;          // 屏幕当前状态

// 触摸屏活动检测相关
static lv_timer_t *touch_activity_timer = NULL;

// 呼吸灯线程相关
static HAL_PWM_S pwmAttr = {0};
static pthread_t breathing_led_thread = 0;
static volatile bool breathing_led_running = false;
static volatile bool breathing_led_thread_active = false;

// 销毁电源管理对话框
static void destroy_power_dialog(void)
{
    if(power_dialog_bg != NULL) {
        lv_obj_del(power_dialog_bg);
        power_dialog_bg = NULL;
        power_dialog = NULL;
        power_btn_cont = NULL;
        power_dialog_selected = 0;

        MLOG_DBG("Power dialog destroyed\n");
    }
}

// 更新电源对话框的选中状态
static void update_power_dialog_selection(void)
{
    if(power_btn_cont == NULL) return;

    for(int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_obj_get_child(power_btn_cont, i);
        if(btn != NULL) {
            if(i == power_dialog_selected) {
                lv_obj_set_style_border_width(btn, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFF00), LV_PART_MAIN | LV_STATE_DEFAULT);
            } else {
                lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            }
        }
    }
}

// 电源对话框按钮点击事件处理
static void power_dialog_btn_clicked(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MAPI_AO_HANDLE_T ao_hdl = NULL;
    int ret = MAPI_SUCCESS;
    if(code == LV_EVENT_CLICKED) {
        int btn_index = (int)(intptr_t)lv_event_get_user_data(e);
        power_dialog_selected = btn_index;

        // 执行选中的操作
        switch(btn_index) {
            case 0: // 关机
                MLOG_DBG("Power off selected\n");
                ret = MAPI_AO_GetHandle(&ao_hdl);
                if(ret == MAPI_SUCCESS) {
                    MAPI_AO_SetAmplifier(ao_hdl, CVI_FALSE);
                }
                system("poweroff");
                break;
            case 1: // 重启
                MLOG_DBG("Reboot selected\n");
                ret = MAPI_AO_GetHandle(&ao_hdl);
                if(ret == MAPI_SUCCESS) {
                    MAPI_AO_SetAmplifier(ao_hdl, CVI_FALSE);
                }
                system("reboot");
                break;
            case 2: // 取消
                MLOG_DBG("Cancel selected\n");
                destroy_power_dialog();
                break;
        }
    }
}

// 创建电源管理对话框
void create_power_dialog(void)
{
    // 如果对话框已经存在，先销毁
    if(power_dialog_bg != NULL) {
        destroy_power_dialog();
    }

    // 创建背景遮罩
    power_dialog_bg = lv_obj_create(lv_screen_active());
    lv_obj_set_size(power_dialog_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(power_dialog_bg, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(power_dialog_bg, LV_OPA_70, LV_PART_MAIN);
    lv_obj_align(power_dialog_bg, LV_ALIGN_DEFAULT, 0, 0);

    // 创建对话框容器
    power_dialog = lv_obj_create(power_dialog_bg);
    lv_obj_remove_style_all(power_dialog);
    lv_obj_set_size(power_dialog, 500, 300);
    lv_obj_align(power_dialog, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(power_dialog, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(power_dialog, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(power_dialog, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(power_dialog, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(power_dialog, lv_color_hex(0x404040), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(power_dialog, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(power_dialog, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(power_dialog, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(power_dialog, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加电源图标
    lv_obj_t *icon = lv_label_create(power_dialog);
    lv_label_set_text(icon, LV_SYMBOL_POWER);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFF6B6B), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 20);

    // 添加标题
    lv_obj_t *title = lv_label_create(power_dialog);
    lv_label_set_text(title, "电源管理");
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 80);

    // 添加说明文本
    lv_obj_t *explain = lv_label_create(power_dialog);
    lv_label_set_text(explain, "请选择操作");
    lv_obj_set_style_text_font(explain, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), 0);
    lv_obj_set_style_text_color(explain, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(explain, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_height(explain, 60);
    lv_obj_set_width(explain, 450);
    lv_obj_align(explain, LV_ALIGN_TOP_MID, 0, 120);

    // 创建按钮容器
    power_btn_cont = lv_obj_create(power_dialog);
    lv_obj_remove_style_all(power_btn_cont);
    lv_obj_set_size(power_btn_cont, 400, 80);
    lv_obj_align(power_btn_cont, LV_ALIGN_BOTTOM_MID, 0, -20);

    // 创建三个按钮：关机、重启、取消
    const char* btn_texts[] = {"关机", "重启", "取消"};
    const lv_color_t btn_colors[] = {
        lv_color_hex(0xFF6B6B),  // 关机 - 红色
        lv_color_hex(0x4ECDC4),  // 重启 - 青色
        lv_color_hex(0x95A5A6)   // 取消 - 灰色
    };

    for(int i = 0; i < 3; i++) {
        lv_obj_t *btn = lv_btn_create(power_btn_cont);
        lv_obj_set_size(btn, 110, 50);
        lv_obj_align(btn, LV_ALIGN_LEFT_MID, i * 130 + 20, 0);
        lv_obj_set_style_bg_color(btn, btn_colors[i], LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *btn_label = lv_label_create(btn);
        lv_label_set_text(btn_label, btn_texts[i]);
        lv_obj_set_style_text_font(btn_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), 0);
        lv_obj_set_style_text_color(btn_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_center(btn_label);

        // 添加点击事件
        lv_obj_add_event_cb(btn, power_dialog_btn_clicked, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    // 设置默认选中第一个按钮（关机）
    power_dialog_selected = 0;
    update_power_dialog_selection();

    MLOG_DBG("Power dialog created\n");
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

// 屏幕控制相关函数实现

// 关闭屏幕（息屏）
static void turn_screen_off(void)
{
    if(screen_is_on) {
        HAL_BACKLIGHT_SetBackLightState(BACKLIGHT_STATE_OFF);
        screen_is_on = false;
        MLOG_INFO("Screen turned off (backlight control)\n");

        // 启动呼吸灯线程
        // start_breathing_led_thread();
    }
}

// 打开屏幕（亮屏）
static void turn_screen_on(void)
{
    if(!screen_is_on) {
        // 停止呼吸灯线程
        // stop_breathing_led_thread();

        HAL_BACKLIGHT_SetBackLightState(BACKLIGHT_STATE_ON);
        screen_is_on = true;
        MLOG_INFO("Screen turned on (backlight control)\n");
    }
}

// 唤醒屏幕（用户活动时调用）
static void wake_up_screen(void)
{
    if(!screen_is_on) {
        turn_screen_on();
        MLOG_INFO("Screen waked up by user activity\n");
    }
}

// 单调时间获取函数
static void get_monotonic_time(struct timespec *ts)
{
    if (clock_gettime(CLOCK_MONOTONIC, ts) != 0) {
        // 如果获取失败，设置为0
        ts->tv_sec = 0;
        ts->tv_nsec = 0;
        MLOG_ERR("Failed to get monotonic time\n");
    }
}

// 计算两个时间点的差值（秒）
static long timespec_diff_seconds(const struct timespec *current, const struct timespec *previous)
{
    return (current->tv_sec - previous->tv_sec) +
           (current->tv_nsec - previous->tv_nsec) / 1000000000L;
}

// 自动关机相关函数实现

// 更新最后活动时间
void update_last_activity_time(void)
{
    // 直接更新活动时间（使用单调时间）
    get_monotonic_time(&last_activity_time);
    MLOG_DBG("Activity detected, reset auto poweroff timer\n");

    // 如果屏幕处于息屏状态，唤醒屏幕
    if(!screen_is_on) {
        wake_up_screen();
        chat_resume();
    }
}

// 获取自动关机时间
static int get_auto_poweroff_timeout(void)
{
    if(strcmp(g_sysbtn_labelPowerdown, "3分钟") == 0) {
        auto_poweroff_timeout_seconds = 3 * 60;
    } else if(strcmp(g_sysbtn_labelPowerdown, "5分钟") == 0) {
        auto_poweroff_timeout_seconds = 5 * 60;
    } else if(strcmp(g_sysbtn_labelPowerdown, "10分钟") == 0) {
        auto_poweroff_timeout_seconds = 10 * 60;
    }

    return auto_poweroff_timeout_seconds;
}

// 自动关机定时器回调函数
static void auto_poweroff_timer_cb(lv_timer_t *timer)
{
    UNUSED(timer);
    MAPI_AO_HANDLE_T ao_hdl = NULL;
    int ret = MAPI_SUCCESS;

    struct timespec current_time;
    get_monotonic_time(&current_time);

    // 计算距离最后一次活动的时间（秒）
    long elapsed_time = timespec_diff_seconds(&current_time, &last_activity_time);

    // 当处于拍照模式或录像模式时，不进行自动关机
    int32_t WorkMode = MODEMNG_GetCurWorkMode();
    extern uint8_t is_start_video;
    if (WorkMode == WORK_MODE_MOVIE && is_start_video == 1) {
        return;
    }

    // 检查是否需要息屏（1分钟无操作）
    if(elapsed_time >= SCREEN_TIMEOUT_SECONDS && screen_is_on && getoff_Index() == UI_SET_SCREEN_ON) {
        MLOG_INFO("Screen timeout triggered after %d minute of inactivity\n", SCREEN_TIMEOUT_MINUTES);
        turn_screen_off();
        // 休眠语音对话
        chat_suspend();
    }

    if(strcmp(g_sysbtn_labelPowerdown, "关闭") == 0) return;

    // 检查是否需要关机
    if(elapsed_time >= get_auto_poweroff_timeout()) {
        MLOG_INFO("Auto poweroff triggered after %d minutes of inactivity\n", get_auto_poweroff_timeout() / 60);
        ret = MAPI_AO_GetHandle(&ao_hdl);
        if(ret == MAPI_SUCCESS) {
            MAPI_AO_SetAmplifier(ao_hdl, CVI_FALSE);
        }
        system("poweroff");
    } else {
        MLOG_DBG("Auto poweroff check: %ld seconds since last activity (timeout: %d seconds)\n",
                elapsed_time, get_auto_poweroff_timeout());
    }
}

// 触摸屏活动检测回调函数
static void touch_activity_read_cb(lv_timer_t *timer)
{
    UNUSED(timer);

    static int touch_fd = -1;
    static bool touch_fd_opened = false;

    if(!touch_fd_opened) {
        touch_fd = open(TOUCH_PANEL_EVENT_PATH, O_RDONLY | O_NONBLOCK);
        touch_fd_opened = true;
        if(touch_fd < 0) {
            MLOG_ERR("Failed to open touch device: %s\n", TOUCH_PANEL_EVENT_PATH);
            return;
        }
    }

    if(touch_fd >= 0) {
        struct input_event ev;
        int rd = read(touch_fd, &ev, sizeof(ev));

        // 读取并处理所有可用的事件
        while(rd == sizeof(ev)) {
            // 只关心触摸按压/释放事件
            if(ev.type == EV_KEY && ev.code == BTN_TOUCH) {
                // 检测到触摸活动，更新活动时间
                update_last_activity_time();
            }

            rd = read(touch_fd, &ev, sizeof(ev));
        }
    }
}

// 启动自动关机定时器
void start_auto_poweroff_timer(void)
{
    screen_is_on = true;
    // 创建定时器定时检测触摸屏活动
    if(touch_activity_timer == NULL) {
        touch_activity_timer = lv_timer_create(touch_activity_read_cb, 30, NULL);
    }

    if(auto_poweroff_timer == NULL) {
        // 每30秒检查一次
        auto_poweroff_timer = lv_timer_create(auto_poweroff_timer_cb, 30000, NULL);
        if(auto_poweroff_timer != NULL) {
            get_monotonic_time(&last_activity_time);  // 初始化最后活动时间
            if(strcmp(g_sysbtn_labelPowerdown, "关闭") == 0)
            {
                MLOG_INFO("Auto poweroff timer started\n");
            } else {
                MLOG_INFO("Auto poweroff timer started (timeout: %d minutes)\n", get_auto_poweroff_timeout() / 60);
            }
        } else {
            MLOG_ERR("Failed to create auto poweroff timer\n");
        }
    }
}

// 停止自动关机定时器
void stop_auto_poweroff_timer(void)
{
    if(auto_poweroff_timer != NULL) {
        lv_timer_del(auto_poweroff_timer);
        auto_poweroff_timer = NULL;
        MLOG_INFO("Auto poweroff timer stopped\n");
    }

    if(touch_activity_timer != NULL) {
        lv_timer_del(touch_activity_timer);
        touch_activity_timer = NULL;
        MLOG_INFO("Touch activity timer stopped\n");
    }
}

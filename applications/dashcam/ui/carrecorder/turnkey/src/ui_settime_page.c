#include <stdio.h>
#include "ui_common.h"
#include "ui_windowmng.h"
#include "param.h"

typedef enum {
    UI_DATETIME_Y = 0,
    UI_DATETIME_M = 2,
    UI_DATETIME_D = 4,
    UI_DATETIME_HR = 5,
    UI_DATETIME_MIN = 7,
    UI_DATETIME_SEC = 9,
    UI_DATETIME_YMD = 11,
    UI_DATETIME_BUT
} _UI_DATETIME_;

#define UI_SETTIME_STYLE_MUN 3
#define UI_SETTIME_STYLE_LEN 32

static uint8_t button_cnt = 0;
static uint8_t cur_button = UI_DATETIME_Y;
static bool creat_settime_flag = false;
static uint8_t s_style_id = 0;
static UI_TIME_BUF_S time_buf = {0};

static SYSTEM_TM_S ui_time =    {DATETIME_DEFAULT_YEAR,
                                    DATETIME_DEFAULT_MONTH,
                                    DATETIME_DEFAULT_DAY,
                                    DATETIME_DEFAULT_HOUR,
                                    DATETIME_DEFAULT_MINUTE,
                                    DATETIME_DEFAULT_SECOND};

static uint8_t button_tab[7] =  {UI_DATETIME_Y,
                                UI_DATETIME_M,
                                UI_DATETIME_D,
                                UI_DATETIME_HR,
                                UI_DATETIME_MIN,
                                UI_DATETIME_SEC,
                                UI_DATETIME_YMD};

static const uint8_t DayOfMonth[2][12] =
{
    // Not leap year
    {31,28,31,30,31,30,31,31,30,31,30,31},
    // Leap year
    {31,29,31,30,31,30,31,31,30,31,30,31}
};

static char s_date_style[UI_SETTIME_STYLE_MUN][UI_SETTIME_STYLE_LEN] = {"YY/MM/DD",
                                                                        "MM/DD/YY",
                                                                        "DD/MM/YY"};

static inline bool_t Year_IsLeapYear(uint32_t u32Year)
{
    if (0U == u32Year % 400U || (0U == u32Year % 4U && 0U != u32Year % 100U))
    {
        return TRUE;
    }
    return FALSE;
}

bool_t ui_check_time_format_type(void)
{
    PARAM_MENU_S menu_param = {0};
    PARAM_GetMenuParam(&menu_param);
    if (menu_param.TimeFormat.Current == MENU_TIME_FORMAT_12) {
        return true;
    }

    return false;
}

static void ui_check_ampm_format(void)
{
    if (ui_time.s32hour < 12) {
        snprintf(time_buf.ampm, DATETIME_BUF_LEN, "%s", "AM");
    } else {
        snprintf(time_buf.ampm, DATETIME_BUF_LEN, "%s", "PM");
    }
}

ret_t ui_save_time_to_rtc(void)
{
    SYSTEM_SetDateTime(&ui_time);

    return 0;
}

// 交换日期格式的显示
static void on_settime_page_change_date_buf(void)
{
    if (s_style_id == DATE_YY_MM_DD) {
        snprintf(time_buf.year, DATETIME_BUF_LEN, "%4d", ui_time.s32year);
        snprintf(time_buf.month, DATETIME_BUF_LEN, "%02d", ui_time.s32mon);
        snprintf(time_buf.day, DATETIME_BUF_LEN, "%02d", ui_time.s32mday);
    } else if (s_style_id == DATE_MM_DD_YY) {
        snprintf(time_buf.year, DATETIME_BUF_LEN, "%02d", ui_time.s32mon);
        snprintf(time_buf.month, DATETIME_BUF_LEN, "%02d", ui_time.s32mday);
        snprintf(time_buf.day, DATETIME_BUF_LEN, "%4d", ui_time.s32year);
    } else if (s_style_id == DATE_DD_MM_YY) {
        snprintf(time_buf.year, DATETIME_BUF_LEN, "%02d", ui_time.s32mday);
        snprintf(time_buf.month, DATETIME_BUF_LEN, "%02d", ui_time.s32mon);
        snprintf(time_buf.day, DATETIME_BUF_LEN, "%4d", ui_time.s32year);
    }
    return;
}

static void year_time_add(void)
{
    if (ui_time.s32year == DATETIME_MAX_YEAR) {
        ui_time.s32year = DATETIME_DEFAULT_YEAR;
    } else {
        ui_time.s32year++;
    }
}
static void year_time_sub(void)
{
    if (ui_time.s32year == DATETIME_DEFAULT_YEAR) {
        ui_time.s32year = DATETIME_MAX_YEAR;
    } else {
        ui_time.s32year--;
    }
}
static void month_time_add(void)
{
    if(ui_time.s32mon == 12) {
        ui_time.s32mon = 1;
    } else {
        ui_time.s32mon++;
    }
}
static void month_time_sub(void)
{
    if(ui_time.s32mon == 1) {
        ui_time.s32mon = 12;
    } else {
        ui_time.s32mon--;
    }
}
static void day_time_add(void)
{
    if (ui_time.s32mday == DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon - 1]) {
        ui_time.s32mday = 1;
    } else {
        ui_time.s32mday++;
    }
}
static void day_time_sub(void)
{
    if(ui_time.s32mday == 1) {
        ui_time.s32mday = DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1];
    } else {
        ui_time.s32mday--;
    }
}

static void on_settime_date_format_add(uint32_t curbutton)
{
    switch (s_style_id)
    {
        case DATE_YY_MM_DD:
            if (curbutton == 0) {
                year_time_add();
            }
            if (curbutton == 2) {
                month_time_add();
            }
            if (curbutton == 4) {
                day_time_add();
            }
            break;
        case DATE_MM_DD_YY:
            if (curbutton == 4) {
                year_time_add();
            }
            if (curbutton == 0) {
                month_time_add();
            }
            if (curbutton == 2) {
                day_time_add();
            }
            break;
        case DATE_DD_MM_YY:
            if (curbutton == 4) {
                year_time_add();
            }
            if (curbutton == 2) {
                month_time_add();
            }
            if (curbutton == 0) {
                day_time_add();
            }
            break;
        default:
            break;
    }

    return;
}

static void on_settime_date_format_sub(uint32_t curbutton)
{
    switch (s_style_id)
    {
        case DATE_YY_MM_DD:
            if (curbutton == 0) {
                year_time_sub();
            }
            if (curbutton == 2) {
                month_time_sub();
            }
            if (curbutton == 4) {
                day_time_sub();
            }
            break;
        case DATE_MM_DD_YY:
            if (curbutton == 4) {
                year_time_sub();
            }
            if (curbutton == 0) {
                month_time_sub();
            }
            if (curbutton == 2) {
                day_time_sub();
            }
            break;
        case DATE_DD_MM_YY:
            if (curbutton == 4) {
                year_time_sub();
            }
            if (curbutton == 2) {
                month_time_sub();
            }
            if (curbutton == 0) {
                day_time_sub();
            }
            break;
        default:
            break;
    }

    return;
}

static void on_settime_page_add_daytime(uint32_t curbutton)
{
    switch(curbutton) {
        case UI_DATETIME_Y:
            on_settime_date_format_add(curbutton);
            break;
        case UI_DATETIME_M:
            on_settime_date_format_add(curbutton);
            break;
        case UI_DATETIME_D:
            on_settime_date_format_add(curbutton);
            break;
        case UI_DATETIME_HR:
            if(ui_time.s32hour == 23) {
                ui_time.s32hour = 0;
            } else {
                ui_time.s32hour++;
            }
            break;
        case UI_DATETIME_MIN:
            if(ui_time.s32min == 59) {
                ui_time.s32min = 0;
            } else {
                ui_time.s32min++;
            }
            break;
        case UI_DATETIME_SEC:
            if(ui_time.s32sec == 59) {
                ui_time.s32sec = 0;
            } else {
                ui_time.s32sec++;
            }
            break;
    }
}

static void on_settime_page_sub_daytime(uint32_t curbutton)
{
    switch(curbutton) {
        case UI_DATETIME_Y:
            on_settime_date_format_sub(curbutton);
            break;
        case UI_DATETIME_M:
            on_settime_date_format_sub(curbutton);
            break;
        case UI_DATETIME_D:
            on_settime_date_format_sub(curbutton);
            break;
        case UI_DATETIME_HR:
            if(ui_time.s32hour == 0){
                ui_time.s32hour = 24;
            } else {
                ui_time.s32hour--;
            }
            break;
        case UI_DATETIME_MIN:
            if(ui_time.s32min == 0) {
                ui_time.s32min = 59;
            } else {
                ui_time.s32min--;
            }
            break;
        case UI_DATETIME_SEC:
            if(ui_time.s32sec == 0) {
                ui_time.s32sec = 59;
            } else {
                ui_time.s32sec--;
            }
            break;
    }
}

static void on_settime_page_date_style_up(void)
{
    if (s_style_id == 2) {
        s_style_id = 0;
    }
    s_style_id++;
}
static void on_settime_page_date_style_down(void)
{
    if (s_style_id == 0) {
        s_style_id = 2;
    }
    s_style_id--;
}

uint8_t ui_settime_get_date_foramtid(void)
{
    return s_style_id;
}

static ret_t create_widget_setting_time_page(widget_t* win)
{
    widget_t* date_time = widget_lookup(win, "date_time", TRUE);
    widget_t* year_button = widget_lookup(win, "year_button", TRUE);

    button_cnt = widget_count_children(date_time);

    widget_set_prop_bool(year_button, WIDGET_STATE_FOCUSED, TRUE);
    if(ui_time.s32mday > DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1]) {
       ui_time.s32mday = DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1];
    }

    creat_settime_flag = true;
    return RET_OK;
}

static ret_t init_widget(void* ctx, const void* iter)
{
    ret_t ret;
    bool_t time_format;
    widget_t* win = WIDGET(iter);
    (void)ctx;

    ui_check_ampm_format();
    time_format = ui_check_time_format_type();

    on_settime_page_change_date_buf();
    // 12 小时只更改显示
    if ((time_format == true) && (ui_time.s32hour > 12)) {
        snprintf(time_buf.hour, DATETIME_BUF_LEN, "%02d", ui_time.s32hour - 12);
    } else {
        snprintf(time_buf.hour, DATETIME_BUF_LEN, "%02d", ui_time.s32hour);
    }
    snprintf(time_buf.min, DATETIME_BUF_LEN, "%02d", ui_time.s32min);
    snprintf(time_buf.sec, DATETIME_BUF_LEN, "%02d", ui_time.s32sec);

    if (creat_settime_flag == false) {
        ret = create_widget_setting_time_page(win);
        if (ret == RET_FAIL) {
            CVI_LOGE("create_buttons_on_option_page failed %d \n", __LINE__);
        }
    }

    if (win->name != NULL) {
        const char* name = win->name;
        if (tk_str_eq(name, "year_button")) {
            widget_set_text_utf8(win, time_buf.year);
        } else if (tk_str_eq(name, "month_button")) {
            widget_set_text_utf8(win, time_buf.month);
        } else if (tk_str_eq(name, "day_button")) {
            widget_set_text_utf8(win, time_buf.day);
        } else if (tk_str_eq(name, "hour_button")) {
            widget_set_text_utf8(win, time_buf.hour);
        } else if (tk_str_eq(name, "minute_button")) {
            widget_set_text_utf8(win, time_buf.min);
        } else if (tk_str_eq(name, "second_button")) {
            widget_set_text_utf8(win, time_buf.sec);
        } else if (tk_str_eq(name, "ampm_button")) {
            if (time_format == true) {
                widget_set_text_utf8(win, time_buf.ampm);
                widget_set_visible(win, TRUE, FALSE);
            } else {
                widget_set_visible(win, FALSE, FALSE);
            }
         } else if (tk_str_eq(name, "fomat_button")) {
            widget_set_text_utf8(win, s_date_style[s_style_id]);
        }
    }

    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

static ret_t on_settime_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    widget_t* win = WIDGET(ctx);
    static widget_t* temp = NULL;
    widget_t* date_time = widget_lookup(win, "date_time", TRUE);

    if (evt->key == UI_KEY_POWER) {
        ui_winmng_finishwin(UI_DATETIME_PAGE);
        ui_save_time_to_rtc();
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
            cur_button++;
            if (cur_button > UI_ARRAY_SIZE(button_tab)) {
                cur_button = 0;
            }
            temp = widget_get_child(date_time, button_tab[cur_button]);
            widget_set_prop_bool(temp, WIDGET_STATE_FOCUSED, TRUE);
            return RET_STOP;
    } else if (evt->key == UI_KEY_UP) {
        temp = widget_get_child(date_time, button_tab[cur_button]);
        if (button_tab[cur_button] <= UI_DATETIME_SEC) {
            on_settime_page_add_daytime(button_tab[cur_button]);
        }
        if (button_tab[cur_button] == UI_DATETIME_YMD) {
            on_settime_page_date_style_up();
        }
        init_children_widget(win);
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        temp = widget_get_child(date_time, button_tab[cur_button]);
        if (button_tab[cur_button] <= UI_DATETIME_SEC) {
            on_settime_page_sub_daytime(button_tab[cur_button]);
        }
        if (button_tab[cur_button] == UI_DATETIME_YMD) {
            on_settime_page_date_style_down();
        }
        init_children_widget(win);
        return RET_STOP;
    }

    return RET_OK;
}
ret_t ui_datetime_page_open(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    if (win) {
        // 获取RTC时间
        SYSTEM_GetRTCDateTime(&ui_time);
        CVI_LOGD("ui_ide_option_page_open \n");
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_settime_page_key_down, win);
        init_children_widget(win);
        return RET_OK;
    }

    return RET_OK;
}
ret_t ui_datetime_page_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        cur_button = 0;
        button_cnt = 0;
        creat_settime_flag = false;
        // 关闭页面保存时间到RTC
        ui_save_time_to_rtc();
    }
    return RET_OK;
}
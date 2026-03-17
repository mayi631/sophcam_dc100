#include <stdio.h>
#include "ui_windowmng.h"
#include "system.h"

typedef enum {
    UI_DATETIME_Y,
    UI_DATETIME_M,
    UI_DATETIME_D,
    UI_DATETIME_HR,
    UI_DATETIME_MIN,
    UI_DATETIME_SEC,
    UI_DATETIME_BUT
} _UI_DATETIME_;

#define DATETIME_DEFAULT_SEC    0

typedef struct app_ui_time {
    uint32_t year;
    uint32_t mouth;
    uint32_t day;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
} appui_time;

static uint8_t button_cnt = 0;
static uint32_t cur_button = UI_DATETIME_Y;

static SYSTEM_TM_S ui_time = { DATETIME_DEFAULT_YEAR, DATETIME_DEFAULT_MONTH, DATETIME_DEFAULT_DAY, \
                        DATETIME_DEFAULT_HOUR, DATETIME_DEFAULT_MINUTE, DATETIME_DEFAULT_SEC};

static char *btm_name_buf[UI_DATETIME_BUT] = { "year_button", "month_button", "day_button", \
        "hour_button", "minute_button", "second_button"};

static char   itemY_Buf[32] = "0000";
static char   itemM_Buf[32] = "00";
static char   itemD_Buf[32] = "00";
static char   itemHR_Buf[32] = "00";
static char   itemMIN_Buf[32] = "00";
static char   itemSEC_Buf[32] = "00";
static char *show_btm_buf[UI_DATETIME_BUT] = { itemY_Buf, itemM_Buf, itemD_Buf, \
            itemHR_Buf, itemMIN_Buf, itemSEC_Buf };

static const uint8_t DayOfMonth[2][12] =
{
    // Not leap year
    {31,28,31,30,31,30,31,31,30,31,30,31},
    // Leap year
    {31,29,31,30,31,30,31,31,30,31,30,31}
};

static inline bool_t Year_IsLeapYear(uint32_t u32Year)
{
    if (0U == u32Year%400U || (0U == u32Year%4U && 0U != u32Year%100U))
    {
        return TRUE;
    }
    return FALSE;
}

static void on_settime_page_add_daytime(uint32_t curbutton, void* ctx)
{
    switch(curbutton) {
        case UI_DATETIME_Y:
            if(ui_time.s32year==DATETIME_MAX_YEAR)
                ui_time.s32year = DATETIME_DEFAULT_YEAR;
            else
                ui_time.s32year++;
            break;
        case UI_DATETIME_M:
            if(ui_time.s32mon==12)
                ui_time.s32mon=1;
            else
                ui_time.s32mon++;
            break;
        case UI_DATETIME_D:
            if(ui_time.s32mday==DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1])
                ui_time.s32mday=1;
            else
                ui_time.s32mday++;
            break;
        case UI_DATETIME_HR:
            if(ui_time.s32hour==23)
                ui_time.s32hour=0;
            else
                ui_time.s32hour++;
            break;
        case UI_DATETIME_MIN:
            if(ui_time.s32min==59)
                ui_time.s32min=0;
            else
                ui_time.s32min++;
            break;
        case UI_DATETIME_SEC:
            if(ui_time.s32sec==59)
                ui_time.s32sec=0;
            else
                ui_time.s32sec++;
            break;
        default:
            printf("error set daytime\n");
    }
    widget_t* win_main  = window_manager_get_top_window(window_manager());
    widget_t* widget = widget_lookup(win_main, btm_name_buf[cur_button], TRUE);
    snprintf(show_btm_buf[cur_button], 32, "%4d", (int32_t )(*((uint32_t *)&ui_time + cur_button)));
    widget_set_text_utf8(widget, show_btm_buf[cur_button]);
}

static void on_settime_page_sub_daytime(uint32_t curbutton, void* ctx)
{
    switch(curbutton) {
        case UI_DATETIME_Y:
            if(ui_time.s32year==DATETIME_DEFAULT_YEAR)
                ui_time.s32year = DATETIME_MAX_YEAR;
            else
                ui_time.s32year--;
            break;
        case UI_DATETIME_M:
            if(ui_time.s32mon==1)
                ui_time.s32mon=12;
            else
                ui_time.s32mon--;
            break;
        case UI_DATETIME_D:
            if(ui_time.s32mday==1)
                ui_time.s32mday=DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1];
            else
                ui_time.s32mday--;
            break;
        case UI_DATETIME_HR:
            if(ui_time.s32hour==0)
                ui_time.s32hour=23;
            else
                ui_time.s32hour--;
            break;
        case UI_DATETIME_MIN:
            if(ui_time.s32min==0)
                ui_time.s32min=59;
            else
                ui_time.s32min--;
            break;
        case UI_DATETIME_SEC:
            if(ui_time.s32sec==0)
                ui_time.s32sec=59;
            else
                ui_time.s32sec--;
            break;
        default:
            printf("error set daytime\n");
    }

    widget_t* win_main  = window_manager_get_top_window(window_manager());
    widget_t* widget = widget_lookup(win_main, btm_name_buf[cur_button], TRUE);
    snprintf(show_btm_buf[cur_button], 32, "%4d", (int32_t )(*((uint32_t *)&ui_time + cur_button)));
    widget_set_text_utf8(widget, show_btm_buf[cur_button]);
}

static ret_t on_commonchange_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* widget = WIDGET(ctx);
    u_int32_t i = 0;
    widget_set_prop_bool(widget, WIDGET_STATE_FOCUSED, TRUE);

    for (i=0; i<UI_DATETIME_BUT; i++) {
        if (!strcmp(widget->name, btm_name_buf[i])) {
            cur_button = i;
            return RET_OK;
        }
    }

    cur_button = UI_DATETIME_BUT;
    return RET_OK;
}


static ret_t increase_time_event(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    on_settime_page_add_daytime(cur_button, win);
    return 0;
}

static ret_t reduce_time_event(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    on_settime_page_sub_daytime(cur_button, win);
    return 0;
}

static ret_t return_time_event(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    (void)e;
    SYSTEM_TM_S pstDateTime;
    memcpy(&pstDateTime, &ui_time, sizeof(SYSTEM_TM_S));
    SYSTEM_SetDateTime(&pstDateTime);
    window_close(win);
    ui_winmng_finishwin(UI_SETTIME_PAGE);
    return 0;
}

static ret_t init_confirm_btm_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    (void)ctx;

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "up_btm")) {
            widget_on(widget, EVT_CLICK, increase_time_event, widget);
        } else if (tk_str_eq(name, "down_btm")) {
            widget_on(widget, EVT_CLICK, reduce_time_event, widget);
        }
    }
    return RET_OK;
}

static ret_t init_time_view_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    (void)ctx;

    if(ui_time.s32mday > DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1])
        ui_time.s32mday = DayOfMonth[Year_IsLeapYear(ui_time.s32year)][ui_time.s32mon-1];

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "year_button")) {
            widget_set_prop_bool(widget, WIDGET_STATE_FOCUSED, TRUE);
            widget_on(widget, EVT_CLICK, on_commonchange_click, widget);
            snprintf(itemY_Buf, 32, "%4d", ui_time.s32year);
            widget_set_text_utf8(widget, itemY_Buf);
        } else if (tk_str_eq(name, "month_button")) {
            widget_on(widget, EVT_CLICK, on_commonchange_click, widget);
            snprintf(itemM_Buf, 32, "%02d", ui_time.s32mon);
            widget_set_text_utf8(widget, itemM_Buf);
        } else if (tk_str_eq(name, "day_button")) {
            widget_on(widget, EVT_CLICK, on_commonchange_click, widget);
            snprintf(itemD_Buf, 32, "%02d", ui_time.s32mday);
            widget_set_text_utf8(widget, itemD_Buf);
        } else if (tk_str_eq(name, "hour_button")) {
            widget_on(widget, EVT_CLICK, on_commonchange_click, widget);
            snprintf(itemHR_Buf, 32, "%02d", ui_time.s32hour);
            widget_set_text_utf8(widget, itemHR_Buf);
        } else if (tk_str_eq(name, "minute_button")) {
            widget_on(widget, EVT_CLICK, on_commonchange_click, widget);
            snprintf(itemMIN_Buf, 32, "%02d", ui_time.s32min);
            widget_set_text_utf8(widget, itemMIN_Buf);
        } else if (tk_str_eq(name, "second_button")) {
            widget_on(widget, EVT_CLICK, on_commonchange_click, widget);
            snprintf(itemSEC_Buf, 32, "%02d", ui_time.s32sec);
            widget_set_text_utf8(widget, itemSEC_Buf);
        }
    }
    return RET_OK;
}

static ret_t init_title_view_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    (void)ctx;

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "ret_btm")) {
            widget_on(widget, EVT_CLICK, return_time_event, widget);
        }
    }
    return RET_OK;
}


static void layer_init_confirm_btm_widget(widget_t* widget) {
  widget_foreach(widget, init_confirm_btm_widget, widget);
}

static void layer_init_tiem_view_widget(widget_t* widget) {
    button_cnt = widget_count_children(widget);
    widget_foreach(widget, init_time_view_widget, widget);
}

static void layer_init_tiem_title_widget(widget_t* widget) {
    widget_foreach(widget, init_title_view_widget, widget);
}


static ret_t init_widget(void* ctx, const void* iter)
{
    (void)ctx;
    widget_t* widget = WIDGET(iter);

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "confirm_view")) {
            layer_init_confirm_btm_widget(widget);
        } else if (tk_str_eq(name, "set_time_view")) {
            layer_init_tiem_view_widget(widget);
        } else if (tk_str_eq(name, "title_view")) {
            layer_init_tiem_title_widget(widget);
        }
    }
    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

ret_t open_settime_window(void* ctx, event_t* e)
{
    SYSTEM_TM_S pstDateTime;
    widget_t* win = WIDGET(ctx);
    if (win) {
        SYSTEM_GetRTCDateTime(&pstDateTime);
        memcpy(&ui_time, &pstDateTime, sizeof(SYSTEM_TM_S));
        init_children_widget(win);
        return RET_OK;
    }
    return RET_FAIL;
}

ret_t close_settime_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        SYSTEM_SetDateTime(&ui_time);
    }
    return RET_FAIL;
}

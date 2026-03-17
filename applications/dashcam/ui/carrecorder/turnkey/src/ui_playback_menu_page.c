#include <stdio.h>
#include "ui_windowmng.h"

#define BUTTON_CNT 2
#define DEL_CUR_FILE "STRID_DELETE_CURFILE_CONFIRM"
#define DEL_ALL_FILE "STRID_DELETE_ALLFILE_CONFIRM"

static uint8_t s_u8CurButtonId = 0;

static ret_t on_playback_menu_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    widget_t* win = WIDGET(ctx);
    printf("on_playback_menu_page_key_down\n");
    widget_t* view = widget_lookup(win, "scroll_view", TRUE);

    if (evt->key == UI_KEY_POWER) {
        ui_winmng_startwin(UI_PLAYBACK_PAGE, TRUE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
        switch (s_u8CurButtonId) {
            case 0: {
                ui_winmng_startwin(UI_IDENTICAL_PAGE, TRUE);
                return RET_STOP;
            }
            case 1: {
                ui_winmng_startwin(UI_IDENTICAL_PAGE, TRUE);
                return RET_STOP;
            }
            default:
                printf("Invalid buttion id!\n");
                break;
        }
        return RET_STOP;
    } else if (evt->key == UI_KEY_UP) {
        s_u8CurButtonId--;
        if (s_u8CurButtonId >=  BUTTON_CNT) {
            s_u8CurButtonId = BUTTON_CNT - 1;
        }
        widget_t* button = widget_get_child(view, s_u8CurButtonId);
        widget_set_prop_bool(button, WIDGET_STATE_FOCUSED, TRUE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        s_u8CurButtonId++;
        if (s_u8CurButtonId >=  BUTTON_CNT) {
            s_u8CurButtonId = 0;
        }
        widget_t* button = widget_get_child(view, s_u8CurButtonId);
        widget_set_prop_bool(button, WIDGET_STATE_FOCUSED, TRUE);
        return RET_STOP;
    }

    return RET_OK;
}

ret_t ui_playback_menu_page_open(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_playback_menu_page_key_down, win);
        widget_t* scroll_view = widget_lookup(win, "scroll_view", true);
        widget_focus_first(scroll_view);
    }

    return RET_OK;
}

ret_t ui_playback_menu_page_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    return RET_OK;
}

static ret_t on_identical_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    printf("on_identical_page_key_down\n");

    if (evt->key == UI_KEY_POWER) {
        ui_winmng_startwin(UI_PLAYBACK_MENU_PAGE, TRUE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
        switch (s_u8CurButtonId) {
            case 0: {
                delete_file_playback();
                ui_winmng_startwin(UI_PLAYBACK_MENU_PAGE, TRUE);
                return RET_STOP;
            }
            case 1: {
                delete_all_file_playback();
                ui_winmng_startwin(UI_PLAYBACK_MENU_PAGE, TRUE);
                return RET_STOP;
            }
            default:
                printf("Invalid buttion id!\n");
                break;
        }
    } else if (evt->key == UI_KEY_UP) {
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        return RET_STOP;
    }

    return RET_OK;
}

ret_t ui_identical_page_open(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_identical_page_key_down, win);
        widget_t* label = label_create(win, 300, 130, 200, 32);
        widget_set_style_int(label, "font_size", 28);
        if(0 == s_u8CurButtonId) {
            widget_set_tr_text(label, DEL_CUR_FILE);
        } else if(1 == s_u8CurButtonId) {
            widget_set_tr_text(label, DEL_ALL_FILE);
        }
        widget_set_style_color(label, "normal:bg_color", 0x00000000);
    }

    return RET_OK;
}

ret_t ui_identical_page_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        s_u8CurButtonId = 0;
    }
    return RET_OK;
}

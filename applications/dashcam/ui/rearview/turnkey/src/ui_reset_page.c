#include "ui_common.h"


static ret_t on_reset_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    widget_t* win = WIDGET(ctx);
    if (evt->key == UI_KEY_BACK) {
        window_close(win);
        return RET_STOP;
    } else if (evt->key == UI_KEY_OK) {
        window_close(win);
        return RET_STOP;
    } else if (evt->key == UI_KEY_UP) {
        window_close(win);
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        window_close(win);
        return RET_STOP;
    }

    return RET_OK;
}

ret_t open_reset_window(void)
{
    widget_t* win = window_open("ui_reset_page");
    if (win) {
        widget_on(win, EVT_KEY_DOWN, on_reset_page_key_down, win);
        return RET_OK;
    }
    
    return RET_FAIL;
}
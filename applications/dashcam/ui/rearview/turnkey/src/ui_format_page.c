#include "ui_windowmng.h"
#include "storagemng.h"
#include "mode.h"

bool ui_format_cardstatus(void)
{
    uint32_t type = 0x0;

    switch(MODEMNG_GetCardState()) {
        case CARD_STATE_REMOVE:
            type = MSG_EVENT_ID_NO_CARD;
            break;
        case CARD_STATE_ERROR:
            type = MSG_EVENT_ID_SDCARD_ERROR;
            break;
        case CARD_STATE_AVAILABLE:
        case CARD_STATE_UNAVAILABLE:
        case CARD_STATE_FSERROR:
        case CARD_STATE_SLOW:
        case CARD_STATE_READ_ONLY:
        case CARD_STATE_MOUNT_FAILED:
            return true;
            break;
        default:
            CVI_LOGE("value is invalid\n");
            break;
    }

    ui_wrnmsg_update_type(type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    return false;
}

static ret_t on_formatbacksetup_click(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    (void)e;
    window_close(win);
    ui_winmng_finishwin(UI_FORMAT_PAGE);

    return RET_OK;
}

static ret_t on_formatconfirm_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    int32_t  s32Ret = 0;
    if (ui_format_cardstatus() == true) {
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_CARD_FORMAT;
        s32Ret = MODEMNG_SendMessage(&Msg);
        if(0 != s32Ret) {
            CVI_LOGE("MODEMNG_Format failed\n");
            return RET_OK;
        }
    }

    return RET_OK;
}

static ret_t init_widget(void* ctx, const void* iter)
{
    (void)ctx;
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "back_button")) {
            widget_on(widget, EVT_CLICK, on_formatbacksetup_click, win);
        } else if (tk_str_eq(name, "confirm")) {
            widget_on(widget, EVT_CLICK, on_formatconfirm_click, win);
        } else if (tk_str_eq(name, "cancel")) {
            widget_on(widget, EVT_CLICK, on_formatbacksetup_click, win);
        }
    }

    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

ret_t open_format_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        init_children_widget(win);
        return RET_OK;
    }

    return RET_FAIL;
}

ret_t close_format_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    return RET_OK;
}
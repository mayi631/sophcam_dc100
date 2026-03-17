#include "ui_windowmng.h"

static widget_t* msg_label = NULL;
static uint32_t msgtype = 0;

static char ui_wrnmsg[MSG_EVENT_ID_MAX][32] = {
    "STRID_LOW_BATTERY",
    "STRID_NO_SD_CARD",
    "STRID_SD_NEED_FORMAT",
    "STRID_SD_SPEED_LOW",
    "STRID_SD_PREPARING",
    "STRID_SD_ERROR_CHANGE",
    "STRID_SD_READ_ONLY",
    "STRID_SD_ERROR_PLUG",
    "STRID_FORMATTING",
    "STRID_FORMAT_FAILED",
    "STRID_SUCCESS",
    "STRID_FILE_ERR",
    "STRID_NOVIDEO_FILE",
    "STRID_NOPHOTO_FILE",
    "STRID_NOTSUPPORT_H265",
    "STRID_RESET_FINISHED",
    "STRID_OTA_UP_FILE",
    "STRID_OTA_UP_FILE_SUCCESSED",
    "STRID_OTA_UP_FILE_FAIL",
    "STRID_OTA_UP_FILE_ERROR",
    "STRID_MODE_UVC"
};

static ret_t init_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "msg_label")) {
            msg_label = widget;
            widget_set_tr_text(msg_label, ui_wrnmsg[msgtype]);
        }
    }
    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

ret_t ui_wrnmsg_update_type(uint32_t type) 
{
    if (type == MSG_EVENT_ID_INVALID) {
        return RET_OK;
    }
    msgtype = type;
    return RET_OK;
}

ret_t ui_wrnmsg_get_type(void) 
{
    return msgtype;
}

ret_t ui_wrnmsg_update_msglael(const idle_info_t* idle)
{
    if (msg_label) {
        widget_set_tr_text(msg_label, ui_wrnmsg[msgtype]);
    }
    return RET_OK;
}

ret_t ui_wrnmsg_page_open(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        init_children_widget(win);
    }
    return RET_OK;
}

ret_t ui_wrnmsg_page_close(void* ctx, event_t* e)
{
    (void)e;
    printf("=======> %s\n", __func__);
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    return RET_OK;
}
#include "ui_windowmng.h"

static bool s_wrnmsgOpen = false;
static widget_t* msg_label = NULL;
static uint32_t msgtype = 0;
static widget_t* msg_win = NULL;

bool wrnmsg_window_isopen(void)
{
    return s_wrnmsgOpen;
}

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
    "STRID_DELETE_CURFILE_CONFIRM",
    "STRID_NOTSUPPORT_H265",
    "STRID_APP_CONNECT_SUCCESS",
    "STRID_APP_DISCONNECT"
};

ret_t ui_wrnmsg_update_type(uint32_t type)
{
    if (type == MSG_EVENT_ID_INVALID ||
        (msgtype == MSG_EVENT_APP_CONNECT_SUCCESS && type != MSG_EVENT_APP_DISCONNECT)) {
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

static ret_t on_wrnmsgback_click(void* ctx, event_t* e)
{
    (void)e;
    if(msgtype == MSG_EVENT_ID_FORMAT_PROCESS ||
        msgtype == MSG_EVENT_APP_CONNECT_SUCCESS) {
        return RET_OK;
    }

    printf("on_wrnmsgback_click %d\n", msgtype);
    s_wrnmsgOpen = false;
    ui_winmng_finishwin(UI_WRNMSG_PAGE);
    if (msgtype == EVT_NOTSUPPORT_H265) {
        back_close_window();
    }
    return RET_STOP;
}

static ret_t on_wrnmsg_state_confirm_click(void* ctx, event_t* e)
{
    (void)e;
    if(msgtype == MSG_EVENT_ID_FORMAT_PROCESS ||
        msgtype == MSG_EVENT_APP_CONNECT_SUCCESS) {
        return RET_OK;
    }

    printf("on_wrnmsg_state_switch_confirm_click %d\n", msgtype);
    s_wrnmsgOpen = false;
    ui_winmng_finishwin(UI_WRNMSG_PAGE);
    delete_file_playback();
    return RET_STOP;
}

static ret_t on_wrnmsg_state_cancel_click(void* ctx, event_t* e)
{
    (void)e;
    if(msgtype == MSG_EVENT_ID_FORMAT_PROCESS ||
        msgtype == MSG_EVENT_APP_CONNECT_SUCCESS) {
        return RET_OK;
    }

    printf("on_wrnmsg_state_switch_cancel_click %d\n", msgtype);
    s_wrnmsgOpen = false;
    ui_winmng_finishwin(UI_WRNMSG_PAGE);
    continue_play_file(PLAY_SELETE_CUR_FILE);
    return RET_STOP;
}

ret_t open_wrnmsg_window(void* ctx, event_t* e)
{

    (void)e;
    if (s_wrnmsgOpen == false) {
        msg_win = WIDGET(ctx);
        msg_label = widget_lookup(msg_win, "msg_label", TRUE);
        widget_set_tr_text(msg_label, ui_wrnmsg[msgtype]);
        s_wrnmsgOpen = true;
        if(msgtype == MSG_EVENT_ID_DELETE_FILE_CONFIRM) {
            widget_t* confirm_button = widget_lookup(msg_win, "confirm_button", TRUE);
            widget_t* cancel_button = widget_lookup(msg_win, "cancel_button", TRUE);
            widget_set_visible(confirm_button, TRUE);
            widget_set_visible(cancel_button, TRUE);
            widget_on(confirm_button, EVT_POINTER_UP, on_wrnmsg_state_confirm_click, msg_win);
            widget_on(cancel_button, EVT_POINTER_UP, on_wrnmsg_state_cancel_click, msg_win);
        } else {
            widget_on(msg_win, EVT_POINTER_UP, on_wrnmsgback_click, msg_win);
            return RET_OK;
        }
    } else {
        widget_set_tr_text(msg_label, ui_wrnmsg[msgtype]);
        return RET_OK;
    }
    return RET_FAIL;
}

ret_t close_wrnmsg_window(void* ctx, event_t* e)
{
    (void)e;
    if (s_wrnmsgOpen == true) {
        s_wrnmsgOpen = false;
        window_close(msg_win);
        msg_win = NULL;
    }
    return RET_OK;
}

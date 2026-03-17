#include "ui_windowmng.h"
#include "filemng_dtcf.h"
#include "param.h"

static CVI_DTCF_DIR_E enDirs[DTCF_DIR_BUTT];
static uint8_t s_listitemCnt = 0;
static uint8_t s_CurlistitemId = 0;

ret_t ui_dir_page_reset(void)
{
    s_CurlistitemId = 0;

    return RET_OK;
}

static ret_t on_dir_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    widget_t* win = WIDGET(ctx);
    widget_t* scroll_view = widget_lookup(win, "scroll_view", TRUE);
    printf("on_dir_page_key_down\n");
    if (evt->key == UI_KEY_POWER) {
        if(evt->alt == false) {
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_MOVIE;
            MODEMNG_SendMessage(&Msg);
            return RET_REMOVE;
        }
    } else if (evt->key == UI_KEY_MENU) {
        if(evt->alt == false) {
            ui_playback_page_updatedir(enDirs[s_CurlistitemId]);
            ui_winmng_startwin(UI_PLAYBACK_PAGE, true);
            return RET_REMOVE;
        }
    } else if (evt->key == UI_KEY_UP) {
        if(evt->alt == false) {
            if (s_CurlistitemId == 0) {
                return RET_STOP;
            }

            s_CurlistitemId--;
            widget_t* list_item = widget_get_child(scroll_view, s_CurlistitemId);
            widget_set_prop_bool(list_item, WIDGET_STATE_FOCUSED, TRUE);
        }
    } else if (evt->key == UI_KEY_DOWN) {
        if(evt->alt == false) {
            if (s_CurlistitemId >= (s_listitemCnt - 1)) {
                return RET_STOP;
            }

            s_CurlistitemId++;
            widget_t* list_item = widget_get_child(scroll_view, s_CurlistitemId);
            widget_set_prop_bool(list_item, WIDGET_STATE_FOCUSED, TRUE);
        }
    }
    return RET_STOP;
}

ret_t ui_dir_page_open(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        uint32_t i = 0;
        widget_t* scroll_view = widget_lookup(win, "scroll_view", TRUE);
        PARAM_FILEMNG_S FileMng;
        PARAM_GetFileMngParam(&FileMng);
        s_listitemCnt = 0;
        for(i = 0; i < DTCF_DIR_BUTT; i++) {
            if(0 < strnlen(FileMng.FileMngDtcf.aszDirNames[i], CVI_DIR_LEN_MAX)) {
                enDirs[s_listitemCnt] = i;
                widget_t* list_item = list_item_create(scroll_view, 0, 0, 0, 0);
                widget_set_text_utf8(list_item, FileMng.FileMngDtcf.aszDirNames[i]);
                widget_set_style_color(list_item, "normal:bg_color", 0xFF000000);
                widget_set_style_color(list_item, "focused:bg_color", 0xFFFF3600);
                widget_set_focusable(list_item, true);
                if (s_listitemCnt == s_CurlistitemId) {
                    widget_set_prop_bool(list_item, WIDGET_STATE_FOCUSED, TRUE);
                }
                s_listitemCnt++;
            }
        }
        scroll_view_set_offset(scroll_view, 0, s_CurlistitemId*60);
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_dir_page_key_down, win);
        return RET_OK;
    }
    return RET_OK;
}

ret_t ui_dir_page_close(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    return RET_OK;
}
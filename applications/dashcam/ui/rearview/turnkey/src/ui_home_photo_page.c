#include "ui_windowmng.h"

static uint32_t timer_id = 0;
static uint32_t time_handle = 0;

static bool ui_home_photo_cardstatus(void)
{
    uint32_t type = 0x0;

    if (ui_winmng_getwinisshow(UI_HOME_PHOTO_PAGE) == false) {
        CVI_LOGW("ui_home_page no open !\n");
        return false;
    }

    switch(MODEMNG_GetCardState()) {
        case CARD_STATE_REMOVE:
            type = MSG_EVENT_ID_NO_CARD;
            break;
        case CARD_STATE_AVAILABLE:
            if (wrnmsg_window_isopen() == true) {
                ui_winmng_finishwin(UI_WRNMSG_PAGE);
            }

            return true;
            break;
        case CARD_STATE_ERROR:
            type = MSG_EVENT_ID_SDCARD_ERROR;
            break;
        case CARD_STATE_FSERROR:
        case CARD_STATE_UNAVAILABLE:
            type = MSG_EVENT_ID_SDCARD_NEED_FORMAT;
            break;
        case CARD_STATE_SLOW:
            type = MSG_EVENT_ID_SDCARD_SLOW;
            break;
        case CARD_STATE_CHECKING:
            type = MSG_EVENT_ID_SDCARD_CHECKING;
            break;
        case CARD_STATE_READ_ONLY:
            type = MSG_EVENT_ID_SDCARD_READ_ONLY;
            break;
        case CARD_STATE_MOUNT_FAILED:
            type = MSG_EVENT_ID_SDCARD_MOUNT_FAILED;
            break;
        default:
            CVI_LOGE("value is invalid\n");
            MODEMNG_SetParkingRec(false);
            return false;
            //break;
    }
    MODEMNG_SetParkingRec(false);
    ui_wrnmsg_update_type(type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);

    return false;
}

static int32_t  UI_Home_OnReceiveMsgResult(EVENT_S* pstEvent)
{
    (void)pstEvent;
    close_busying_page();
    return 0;
}

static ret_t on_home_page_low_battery(void* ctx, event_t* e)
{
    ui_wrnmsg_update_type(e->type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    return RET_OK;
}

static ret_t on_home_page_move(void* ctx, event_t* e)
{
    pointer_event_t* evt = (pointer_event_t*)e;
    static int32_t  cury = 0, prey = 0;
    if (MODEMNG_GetInProgress() == true) {
        return RET_OK;
    }

    uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
    uint32_t enWind = (curWind >> 16) & 0xFFFF;
    uint32_t enSns = (curWind & 0xFFFF);
    uint32_t n = enSns;
    uint32_t count = 0;
    while(n){n = (n >> 1); count++;};
    if(enWind == enSns && count > 1){
        return RET_OK;
    }
    if(enWind == enSns){
        return RET_OK;
    }

    cury = evt->y;
    for(int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++){
        if(((enWind >> i) & 0x1) == 1){
            if (prey != 0) {
                if(cury - prey > 0) {
                    MODEMNG_LiveViewUp(i);
                } else if(cury - prey < 0) {
                    MODEMNG_LiveViewDown(i);
                }
            }
            break;
        }
    }

    prey = cury;
    return RET_OK;
}

static ret_t on_setup_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    MESSAGE_S Msg = {0};
    if (MODEMNG_GetInProgress() == true) {
        return RET_OK;
    }
    if (timer_id != 0) {
        timer_remove(timer_id);
        timer_id = 0;
    }
    if(time_handle != 0) {
        timer_remove(time_handle);
        time_handle = 0;
    }

    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = EVENT_MODEMNG_PHOTO_SET;
    MODEMNG_SendMessage(&Msg);
    ui_winmng_startwin(UI_PHOTO_SET_PAGE, false);

    return RET_OK;
}

static ret_t on_lvmodechange_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    (void)ctx;
    (void)e;

    uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
    if(curWind == 0){
        return RET_OK;
    }
    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_SWITCH_LIVEVIEW;
    uint32_t enWind = (curWind >> 16) & 0xFFFF;
    uint32_t enSns = (curWind & 0xFFFF);

RETRY:
    if(enWind == enSns){
        enWind = 0x1;
    }else if((enWind << 1) > enSns){
        enWind = enSns;
    }else{
        enWind = (enWind << 1);
    }

    if((enWind & enSns) == 0){
        goto RETRY;
    }

    Msg.arg1 = ((enWind << 16) | enSns);

    MODEMNG_SendMessage(&Msg);
    return RET_OK;
}

static ret_t on_photo_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    if(ui_home_photo_cardstatus()) {
        MESSAGE_S Msg = {0};
        int32_t  s32Ret = 0;
        Msg.topic = EVENT_MODEMNG_START_PIV;

        s32Ret = UICOMM_SendAsyncMsg(&Msg, UI_Home_OnReceiveMsgResult);
        if(s32Ret != 0) {
            return RET_OK;
        }

        open_busying_page();
    }
    return RET_OK;
}

static ret_t on_playback_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    if(ui_home_photo_cardstatus()) {
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_PLAYBACK;
        MODEMNG_SendMessage(&Msg);
    }

    return RET_OK;
}

ret_t progress_bar_on_timer(const timer_info_t* timer)
{
    #ifdef ADC_ON
    widget_t* progress_bar = (widget_t*)timer->ctx;
    USB_STATE_E penState = USB_STATE_OUT;
    if(NULL == progress_bar || NULL == progress_bar->name) {
        CVI_LOGE("progress_bar is null !\n");
        return RET_REPEAT;
    }

    USB_GetState(&penState);
    progress_bar_set_value(progress_bar, USB_STATE_INSERT == penState ? 100 : GAUGEMNG_GetPercentage());
    #endif
    return RET_REPEAT;
}

static ret_t on_startrec_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1 = WORK_MODE_MOVIE;
    MODEMNG_SendMessage(&Msg);
    return RET_OK;;
}

static ret_t init_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);
    (void)ctx;

    if (widget->name != NULL) {
        //printf("%s %d =====%s\n", __FUNCTION__, __LINE__, widget->name);
        const char* name = widget->name;
        if (tk_str_eq(name, "home_record")) {
            widget_set_visible(widget, FALSE, FALSE);
        } else if (tk_str_eq(name, "home_setup_button")) {
            widget_on(widget, EVT_CLICK, on_setup_click, win);
        } else if (tk_str_eq(name, "home_startrec_button")) {
            widget_on(widget, EVT_CLICK, on_startrec_click, win);
        } else if (tk_str_eq(name, "home_videochange_button")) {
            widget_on(widget, EVT_CLICK, on_lvmodechange_click, win);
        } else if (tk_str_eq(name, "home_photo_button")) {
            widget_on(widget, EVT_CLICK, on_photo_click, win);
        } else if (tk_str_eq(name, "home_playback_button")) {
            widget_on(widget, EVT_CLICK, on_playback_click, win);
        }
    }
    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

static ret_t on_window_close(void* ctx, event_t* e) {
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    return RET_OK;
}

/**
 * 初始化
 */
ret_t ui_home_photo_open(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    if (win) {
        widget_on(win, EVT_POINTER_MOVE, on_home_page_move, win);
        widget_on(win, EVT_WINDOW_CLOSE, on_window_close, win);
        widget_on(win, EVT_LOW_BATTERY, on_home_page_low_battery, win);
        init_children_widget(win);
        ui_home_photo_cardstatus();
    }

    return RET_OK;
}

ret_t ui_home_photo_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }

    if (timer_id != 0) {
        timer_remove(timer_id);
        timer_id = 0;
    }
    if(time_handle != 0) {
        timer_remove(time_handle);
        time_handle = 0;
    }
    return RET_OK;
}

//设置模式和回放模式返回时需要判断上一个模式是什么模式切到对应的模式下，尤其是设置模式退出时，如果模式回放模式返回到录像模式则不管
int32_t  ui_home_photo_page_eventcb(void *argv, EVENT_S *msg)
{
    uint32_t u32ModeState = 0;
    MODEMNG_GetModeState(&u32ModeState);
    CVI_LOGD("u32ModeState == %d\n", u32ModeState);
    CVI_LOGD("ui_home_photo_page_eventcb eventcb will process message topic(%x)\n", msg->topic);
    switch(msg->topic) {
        case EVENT_MODEMNG_MODEOPEN:
        {
            ui_winmng_startwin(UI_HOME_PHOTO_PAGE, false);
            break;
        }
        case EVENT_MODEMNG_MODECLOSE:
        {
            ui_winmng_closeallwin();
            break;
        }
        case EVENT_MODEMNG_CARD_FSERROR:
        case EVENT_MODEMNG_CARD_SLOW:
        case EVENT_MODEMNG_CARD_CHECKING:
        case EVENT_MODEMNG_CARD_UNAVAILABLE:
        case EVENT_MODEMNG_CARD_ERROR:
        case EVENT_MODEMNG_CARD_READ_ONLY:
        case EVENT_MODEMNG_CARD_MOUNT_FAILED:
        case EVENT_MODEMNG_CARD_REMOVE:
        case EVENT_MODEMNG_CARD_AVAILABLE:
        {
            uint32_t type = ui_wrnmsg_get_type();
            if ((u32ModeState != MEDIA_MOVIE_STATE_MENU) ||
                (type == EVT_FORMAT_PROCESS)) {
                ui_home_photo_cardstatus();
            }
            break;
        }
        case EVENT_MODEMNG_RECODER_STARTPIVSTAUE:
        {
            if (msg->arg1 == 0)  {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_PHOTO_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=false,
                };
                VOICEPLAY_Push(&stVoice, 0);
            }
            break;
        }
        default:
            break;
    }
    return 0;
}
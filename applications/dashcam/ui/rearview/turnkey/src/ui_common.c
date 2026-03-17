#include <stdio.h>
#include "ui_windowmng.h"
#include "event_recorder_player.h"
#include "netctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static uint32_t type;
static bool key_power_off = false;
int32_t  UICOMM_PowerOff(void);
int32_t  PowerButton_Event(void);

/* keyevent handling list*/
KEY_GPIO_EVENT keymngevent[] = { {KEYMNG_KEY_IDX_0, UICOMM_PowerOff, PowerButton_Event},
                                     {KEYMNG_KEY_IDX_1, UICOMM_PowerOff, PowerButton_Event},
                                     {KEYMNG_KEY_IDX_2, UICOMM_PowerOff, PowerButton_Event}, // The interface can be replaced after implementation
                                     {KEYMNG_KEY_IDX_3, UICOMM_PowerOff, PowerButton_Event}};

ret_t ui_open_homepage(const idle_info_t* idle)
{
    return RET_OK;
}

ret_t ui_open_uvc(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    widget_lookup(win, "uvc", TRUE);

    return RET_OK;
}

ret_t ui_open_storage(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    widget_lookup(win, "storage", TRUE);

    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV;
    MODEMNG_SendMessage(&Msg);

    return RET_OK;
}

ret_t ui_close_homepage(const idle_info_t* idle)
{
    (void)idle;
    void * ctx = NULL;
    event_t * e = NULL;
    ui_home_close(ctx, e);
    return RET_OK;
}

ret_t ui_open_dirpage(const idle_info_t* idle)
{
    return RET_OK;
}

ret_t ui_close_page(const idle_info_t* idle)
{
    window_manager_close_all(window_manager());
    return RET_OK;
}

static ret_t ui_open_msgpage(const idle_info_t* idle)
{
    uint32_t type = *(uint32_t*)(idle->ctx);
    ui_wrnmsg_update_type(type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    return RET_OK;
}

ret_t ui_close_msgpage(const idle_info_t* idle)
{
    ui_winmng_closeallwin();
    return RET_OK;
}

static ret_t ui_playback_reset(const idle_info_t* idle)
{
    playback_reset_time();
    return RET_OK;
}

static ret_t ui_playback_addtime(const idle_info_t* idle)
{
    playback_add_time();
    return RET_OK;
}

static int32_t  UICOMM_MessageResult(EVENT_S *pstEvent)
{
    int32_t  s32Ret = 0;
    MUTEX_LOCK(s_stMessageCtx.MsgMutex);

    if (!s_stMessageCtx.bMsgProcessed) {
        CVI_LOGD("event(%x)\n\n", pstEvent->topic);
        if ((s_stMessageCtx.stMsg.topic == pstEvent->topic)
            && (s_stMessageCtx.stMsg.arg1 == pstEvent->arg1)
            && (s_stMessageCtx.stMsg.arg2 == pstEvent->arg2)) {
            if (s_stMessageCtx.pfnMsgResultProc != NULL) {
                s32Ret = s_stMessageCtx.pfnMsgResultProc(pstEvent);
                if (0 != s32Ret) {
                    CVI_LOGE("pfnMsgResultProc() Error:%#x\n", s32Ret);
                }
            }
            s_stMessageCtx.bMsgProcessed = true;
        }
    }

    MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);

    return s32Ret;
}

bool ui_common_cardstatus(void)
{
    uint32_t CurMode = WORK_MODE_BUTT;

    switch(MODEMNG_GetCardState()) {
        case CARD_STATE_REMOVE:
            type = MSG_EVENT_ID_NO_CARD;
            break;
        case CARD_STATE_AVAILABLE:
            if (wrnmsg_window_isopen() == true &&
                ui_wrnmsg_get_type() != (ret_t)MSG_EVENT_APP_CONNECT_SUCCESS) {
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
            break;
    }
    MODEMNG_SetParkingRec(false);

    MODEMNG_GetCurMode(&CurMode);
    if (CurMode == WORK_MODE_MOVIE) {
        ui_wrnmsg_update_type(type);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    }

    return false;
}

int32_t  UICOMM_PowerOff(void)
{
    widget_t* win = window_open_and_close("ui_close_machine", window_manager_get_top_window(window_manager()));
    if (NULL == win) {
        CVI_LOGE("common window_open_and_close fail\n");
        return RET_OK;
    }
    key_power_off = true;
    return 0;
}
static bool ui_close_flag = false;
int32_t  PowerButton_Event(void)
{
    MESSAGE_S Msg = {0};
    u_int32_t s32Ret = 0;
    if (key_power_off == true) {
        Msg.topic = EVENT_MODEMNG_POWEROFF;
        s32Ret = MODEMNG_SendMessage(&Msg);
        if (0 != s32Ret) {
            CVI_LOGI("MODEMNG_SendMessage fail\n");
            return -1;
        }
    } else {
        if (ui_close_flag == false) {
            CVI_LOGE("pannel is going close\n");
#ifdef SCREEN_ON
            HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_OFF);
#endif
#ifdef CONFIG_TOUCHPAD_ON
            HAL_TOUCHPAD_Suspend();
#endif
            ui_close_flag = true;
        } else {
            CVI_LOGE("pannel is going open\n");
#ifdef SCREEN_ON
            HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_ON);
#endif
#ifdef CONFIG_TOUCHPAD_ON
            HAL_TOUCHPAD_Resume();
#endif
            ui_close_flag = false;
        }

    }
    return 0;
}

int32_t  old_mode = WORK_MODE_MOVIE;
int32_t  ui_common_eventcb(void *argv, EVENT_S *msg)
{
    MESSAGE_S Msg = {0};
    int32_t  s32Ret = 0;
    /*receive message result*/
    s32Ret = UICOMM_MessageResult(msg);
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");
    // CVI_LOGD("ui common eventcb will process message topic(%x) \n\n", msg->topic);

    if (!(EVENT_MODEMNG_RECODER_STARTSTATU  == msg->topic || EVENT_MODEMNG_RECODER_STOPSTATU ==msg->topic))
    {
        bool bEventContinueHandle  = false;
        s32Ret = UI_POWERCTRL_PreProcessEvent(msg, &bEventContinueHandle);
        APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PreProcessEvent");
        if (!bEventContinueHandle)
        {
            CVI_LOGI("Event %x has been processed by Power Control Module\n", msg->topic);
            return 0;
        }
    }

    /*get cur mode*/
    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();

    switch(msg->topic) {
        case EVENT_MODEMNG_RESET:
            {
                uint32_t u32ModeState = 0;
                MODEMNG_GetModeState(&u32ModeState);
                CVI_LOGD("u32ModeState == %d\n", u32ModeState);
                if (ui_common_cardstatus() == true && u32ModeState != MEDIA_MOVIE_STATE_MENU) {
                    Msg.topic = EVENT_MODEMNG_START_REC;
                    MODEMNG_SendMessage(&Msg);
                }
            }
            break;
        case EVENT_MODEMNG_MODEOPEN:
            {
                ui_lock();
                CVI_LOGD("EVENT_MODEMNG_MODEOPEN\n");
                s32CurMode = msg->arg1;
                ui_unlock();
                break;
            }
            break;
        case EVENT_MODEMNG_MODECLOSE:
            {
                ui_lock();
                CVI_LOGD("EVENT_MODEMNG_MODECLOSE\n");
                s32CurMode = msg->arg1;
                ui_unlock();
                break;
            }
            break;
        case EVENT_MODEMNG_RECODER_STARTSTATU:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case EVENT_MODEMNG_RECODER_STOPSTATU:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case EVENT_MODEMNG_RECODER_SPLITREC:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case EVENT_MODEMNG_RECODER_STARTEVENTSTAUE:
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        case EVENT_MODEMNG_RECODER_STOPEVENTSTAUE:
        {
            //MODEMNG_SetEmrState(false);
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case EVENT_MODEMNG_CARD_FORMATING:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_PROCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMATING\n");
            return 0;
        }
        case EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_SUCCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMAT_SUCCESSED\n");
            return 0;
        }
        case EVENT_MODEMNG_CARD_FORMAT_FAILED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_FAILED);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMAT_FAILED\n");
            return 0;
        }
        case EVENT_MODEMNG_PLAYBACK_FINISHED:
            idle_queue(ui_playback_reset, NULL);
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_FINISHED\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PROGRESS:
            idle_queue(ui_playback_addtime, NULL);
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PROGRESS\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PAUSE:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PAUSE\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_RESUME:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_RESUME\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_ABNORMAL:
        {
            type = MSG_EVENT_ID_FILE_ABNORMAL;
            idle_queue(ui_open_msgpage, (void*)&type);
            CVI_LOGD("CVI_EVENT_PLAYBACKMODE_FILE_ABNORMAL\n");
            break;
        }
        case EVENT_MODETEST_START_RECORD:
        {
            event_recorder_player_start_record("/mnt/data/ui_record.txt");
            break;
        }
        case EVENT_MODETEST_STOP_RECORD:
        {
            CVI_LOGD("EVENT_MODETEST_STOP_RECORD\n");
            event_recorder_player_stop_record();
            break;
        }
        case EVENT_MODETEST_PLAY_RECORD:
        {
            CVI_LOGD("EVENT_MODETEST_PLAY_RECORD\n");
            event_recorder_player_start_play("/mnt/data/ui_record.txt", msg->arg1);
            break;
        }
        case EVENT_SENSOR_PLUG_STATUS:
        {
            int32_t  snsid = msg->aszPayload[1];
            uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
            if (msg->arg1 == SENSOR_PLUG_OUT) {
                curWind &= (~(0x1 << snsid) & 0xFFFF);
            } else if (msg->arg1 == SENSOR_PLUG_IN) {
                curWind |= (0x1 << snsid);
            }
            curWind = (((curWind & 0xFFFF) << 16) | (curWind & 0xFFFF));
            PARAM_SetMenuParam(0, PARAM_MENU_VIEW_WIN_STATUS, curWind);

            {
                MESSAGE_S Msg = {0};
                Msg.topic = EVENT_MODEMNG_SWITCH_LIVEVIEW;
                Msg.arg1 = curWind;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case EVENT_KEYMNG_LONG_CLICK:
        {
            int32_t  keymngcunt = 0;
            int32_t  lkeyflage = 0;
            KEYGPIOEVENT longkeybackll;
            keymngcunt = (sizeof(keymngevent)/sizeof(keymngevent[0]));
            for (int32_t  i = 0; i < keymngcunt; i++) {
                if ((msg->arg1) == (int32_t )(keymngevent[i].gpioidx)) {
                    longkeybackll = keymngevent[i].longkeyback;
                    longkeybackll();
                    lkeyflage = 1;
                    break;
                }
            }

            if (0 == lkeyflage) {
                CVI_LOGE("key mng long click msg error, msg->arg1 = (%d)\n", msg->arg1);
            }

            break;
        }
        case EVENT_KEYMNG_SHORT_CLICK:
        {
            int32_t  keymngcunt = 0;
            int32_t  skeyflage = 0;
            KEYGPIOEVENT shortkeybackll;
            keymngcunt = (sizeof(keymngevent)/sizeof(keymngevent[0]));
            for (int32_t  i = 0; i < keymngcunt; i++) {
                if ((msg->arg1) == (int32_t )(keymngevent[i].gpioidx)) {
                    shortkeybackll = keymngevent[i].shortkeyback;
                    shortkeybackll();
                    skeyflage = 1;
                    break;
                }
            }

            if (0 == skeyflage) {
                CVI_LOGE("key mng short click msg error, msg->arg1 = (%d)\n", msg->arg1);
            }

            break;
        }
        case EVENT_UI_TOUCH:
        {
            VOICEPLAY_VOICE_S stVoice=
            {
                .au32VoiceIdx={UI_VOICE_TOUCH_BTN_IDX},
                .u32VoiceCnt=1,
                .bDroppable=true,
            };
            VOICEPLAY_Push(&stVoice, 0);
            break;
        }
#ifdef SERVICES_SPEECH_ON
        case EVENT_MODEMNG_SPEECHMNG_STARTREC:
        {
            uint32_t u32ModeState = 0;
            MODEMNG_GetModeState(&u32ModeState);
            if (ui_common_cardstatus() == true &&
               (u32ModeState != MEDIA_MOVIE_STATE_REC) &&
               (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC)) {
                Msg.topic = EVENT_MODEMNG_START_REC;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case EVENT_MODEMNG_SPEECHMNG_STOPREC:
        {
            uint32_t u32ModeState = 0;
            MODEMNG_GetModeState(&u32ModeState);
            if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
               (u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
                Msg.topic = EVENT_MODEMNG_STOP_REC;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case EVENT_MODEMNG_SPEECHMNG_OPENFRONT:
        {
            uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
            if (curWind == 0) {
                break;
            }
            Msg.topic = EVENT_MODEMNG_SWITCH_LIVEVIEW;
            uint32_t enWind = (curWind >> 16) & 0xFFFF;
            uint32_t enSns = (curWind & 0xFFFF);
            if (enWind == 0x1) {
                break;
            }
            Msg.arg1 = ((0x1 << 16) | enSns);
            MODEMNG_SendMessage(&Msg);
            break;
        }
        case EVENT_MODEMNG_SPEECHMNG_OPENREAR:
        {
            uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
            if (curWind == 0) {
                break;
            }
            Msg.topic = EVENT_MODEMNG_SWITCH_LIVEVIEW;
            uint32_t enWind = (curWind >> 16) & 0xFFFF;
            uint32_t enSns = (curWind & 0xFFFF);
            if (enWind == 0x10 || (enSns >> 1) != 0x1) {
                break;
            }
            Msg.arg1 = ((0x1 << 17) | enSns);
            MODEMNG_SendMessage(&Msg);
            break;
        }
#ifdef SCREEN_ON
        case EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN:
        {
            s32Ret = HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_OFF);
            if (s32Ret != 0){
                CVI_LOGI("Close screen fail\n");
            }
            break;
        }
        case EVENT_MODEMNG_SPEECHMNG_OPENSCREEN:
        {
            s32Ret = HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_ON);
            if (s32Ret != 0){
                CVI_LOGI("Open screen fail\n");
            }
            break;
        }
#endif
        case EVENT_MODEMNG_SPEECHMNG_EMRREC:
        {
            uint32_t u32ModeState = 0;
            MODEMNG_GetModeState(&u32ModeState);
            if (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC && ui_common_cardstatus()) {
                Msg.topic = EVENT_MODEMNG_START_EMRREC;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case EVENT_MODEMNG_SPEECHMNG_PIV:
        {
            if (ui_common_cardstatus() == true) {
                Msg.topic = EVENT_MODEMNG_START_PIV;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
#ifdef WIFI_ON
        case EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI:
        {
            PARAM_WIFI_S WifiParam = {0};
            MESSAGE_S Msg = {0};
            PARAM_GetWifiParam(&WifiParam);
            if (WifiParam.Enable) {
                Msg.topic = EVENT_MODEMNG_SETTING;
                Msg.arg1 = PARAM_MENU_WIFI_STATUS;
                Msg.arg2 = 0;
                MODEMNG_SendMessage(&Msg);
                widget_t* topwin = window_manager_get_top_main_window(window_manager());
                if (topwin != NULL) {
                    widget_t* wifi_image_widget = widget_lookup(topwin, "wifi_btm_image", TRUE);
                    image_base_set_image(wifi_image_widget, "wifi_off");
                }
            }
            break;
        }
        case EVENT_MODEMNG_SPEECHMNG_OPENWIFI:
        {
            PARAM_WIFI_S WifiParam = {0};
            MESSAGE_S Msg = {0};
            PARAM_GetWifiParam(&WifiParam);
            if (!WifiParam.Enable) {
                Msg.topic = EVENT_MODEMNG_SETTING;
                Msg.arg1 = PARAM_MENU_WIFI_STATUS;
                Msg.arg2 = 1;
                MODEMNG_SendMessage(&Msg);
                widget_t* topwin = window_manager_get_top_main_window(window_manager());
                if (topwin != NULL) {
                    widget_t* wifi_image_widget = widget_lookup(topwin, "wifi_btm_image", TRUE);
                    image_base_set_image(wifi_image_widget, "wifi_on");
                }
            }
            break;
        }
#endif
#endif
#ifdef SERVICES_ADAS_ON
        case EVENT_ADASMNG_CAR_MOVING:
            {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_MOVING_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                VOICEPLAY_Push(&stVoice, 0);
                break;
            }
        case EVENT_ADASMNG_CAR_CLOSING:
            {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_CLOSING_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                VOICEPLAY_Push(&stVoice, 0);
                break;
            }
        case EVENT_ADASMNG_CAR_COLLISION:
            {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_COLLISION_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                VOICEPLAY_Push(&stVoice, 0);
                break;
            }
        case EVENT_ADASMNG_CAR_LANE:
            {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_CAR_LANE_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=true,
                };
                VOICEPLAY_Push(&stVoice, 0);
                break;
            }
#endif
        default:
            break;
    }

    if (s32CurMode == WORK_MODE_MOVIE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == WORK_MODE_PLAYBACK) {
        ui_playbackpage_eventcb(argv, msg);
    } else if(s32CurMode == WORK_MODE_UPDATE) {
        // ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE);
        // ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    } else if (s32CurMode == WORK_MODE_LAPSE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == WORK_MODE_UVC) {
        if (old_mode != s32CurMode) {
            ui_winmng_startwin(UI_UVC_PAGE, false);

            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_UVC_MODE_START;
            MODEMNG_SendMessage(&Msg);
        }
        old_mode = s32CurMode;

    } else if (s32CurMode == WORK_MODE_STORAGE) {
        ui_winmng_startwin(UI_STORAGE_PAGE, false);
    } else if (s32CurMode == WORK_MODE_PHOTO) {
        ui_home_photo_page_eventcb(argv, msg);
    }

    return 0;
}

int32_t  UICOMM_SendAsyncMsg(MESSAGE_S* pstMsg, UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc)
{
    int32_t  s32Ret = 0;
    APPCOMM_CHECK_POINTER(pstMsg, -1);

    MUTEX_LOCK(s_stMessageCtx.MsgMutex);

    if (!s_stMessageCtx.bMsgProcessed) {
        CVI_LOGE("Current Msg not finished\n");
        MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
        return -1;
    }

    s_stMessageCtx.bMsgProcessed = false;
    s_stMessageCtx.stMsg.topic = pstMsg->topic;
    s_stMessageCtx.stMsg.arg1 = pstMsg->arg1;
    s_stMessageCtx.stMsg.arg2 = pstMsg->arg2;
    memcpy(s_stMessageCtx.stMsg.aszPayload, pstMsg->aszPayload, sizeof(s_stMessageCtx.stMsg.aszPayload));
    s_stMessageCtx.pfnMsgResultProc = pfnMsgResultProc;

    CVI_LOGD("[what:%#x, arg1:%#x, arg2:%#x]\n", pstMsg->topic, pstMsg->arg1, pstMsg->arg2);
    s32Ret = MODEMNG_SendMessage(pstMsg);

    if (0 != s32Ret) {
        CVI_LOGE("Error:%#x\n", s32Ret);
        s_stMessageCtx.bMsgProcessed = true;
        MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
        return -1;
    }

    MUTEX_UNLOCK(s_stMessageCtx.MsgMutex);
    return 0;
}

int32_t  ui_common_SubscribeEvents(void)
{
    int32_t  ret = 0;
    uint32_t i = 0;
    EVENTHUB_SUBSCRIBER_S stSubscriber = {"ui", NULL, ui_common_eventcb, false};
    MW_PTR SubscriberHdl = NULL;
    TOPIC_ID topic[] = {
        EVENT_MODEMNG_CARD_REMOVE,
        EVENT_MODEMNG_CARD_AVAILABLE,
        EVENT_MODEMNG_CARD_UNAVAILABLE,
        EVENT_MODEMNG_CARD_ERROR,
        EVENT_MODEMNG_CARD_FSERROR,
        EVENT_MODEMNG_CARD_SLOW,
        EVENT_MODEMNG_CARD_CHECKING,
        EVENT_MODEMNG_CARD_FORMATING,
        EVENT_MODEMNG_CARD_FORMAT_SUCCESSED,
        EVENT_MODEMNG_CARD_FORMAT_FAILED,
        EVENT_MODEMNG_CARD_READ_ONLY,
        EVENT_MODEMNG_CARD_MOUNT_FAILED,
        EVENT_MODEMNG_RESET,
        EVENT_MODEMNG_MODESWITCH,
        EVENT_MODEMNG_MODEOPEN,
        EVENT_MODEMNG_MODECLOSE,
        EVENT_MODEMNG_SETTING,
        EVENT_MODEMNG_START_PIV,
        EVENT_MODEMNG_PLAYBACK_FINISHED,
        EVENT_MODEMNG_PLAYBACK_PROGRESS,
        EVENT_MODEMNG_PLAYBACK_PAUSE,
        EVENT_MODEMNG_PLAYBACK_RESUME,
        EVENT_MODEMNG_PLAYBACK_ABNORMAL,
        EVENT_MODEMNG_RECODER_STARTSTATU,
        EVENT_MODEMNG_RECODER_STOPSTATU,
        EVENT_MODEMNG_RECODER_SPLITREC,
        EVENT_MODEMNG_RECODER_STARTEVENTSTAUE,
        EVENT_MODEMNG_RECODER_STOPEVENTSTAUE,
        EVENT_MODEMNG_RECODER_STARTEMRSTAUE,
        EVENT_MODEMNG_RECODER_STOPEMRSTAUE,
        EVENT_MODEMNG_RECODER_STARTPIVSTAUE,
        EVENT_FILEMNG_SPACE_FULL,
        // EVENT_FILEMNG_SCAN_COMPLETED,
        // EVENT_FILEMNG_SCAN_FAIL,
        EVENT_MODETEST_START_RECORD,
        EVENT_MODETEST_STOP_RECORD,
        EVENT_MODETEST_PLAY_RECORD,
        EVENT_SENSOR_PLUG_STATUS,
    #ifdef SERVICES_PHOTO_ON
        PHOTO_SERVICE_EVENT_PIV_START,
        PHOTO_SERVICE_EVENT_PIV_END,
        PHOTO_SERVICE_EVENT_PIV_ERROR,
    #endif
        EVENT_KEYMNG_LONG_CLICK,
        EVENT_KEYMNG_SHORT_CLICK,
        EVENT_UI_TOUCH,
        EVENT_STORAGEMNG_DEV_CONNECTING,
        WORK_MODE_UVC,
        WORK_MODE_STORAGE,
        EVENT_MODEMNG_UVC_MODE_START,
        EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV,
    #ifdef SERVICES_ADAS_ON
        EVENT_ADASMNG_CAR_MOVING,
        EVENT_ADASMNG_CAR_CLOSING,
        EVENT_ADASMNG_CAR_COLLISION,
        EVENT_ADASMNG_CAR_LANE,
    #ifdef SERVICES_ADAS_LABEL_CAR_ON
        EVENT_ADASMNG_LABEL_CAR,
    #endif
    #ifdef SERVICES_ADAS_LABEL_LANE_ON
        EVENT_ADASMNG_LABEL_LANE,
    #endif
    #endif
    #if defined (ENABLE_VIDEO_MD)
        EVENT_VIDEOMD_CHANGE,
    #endif
        EVENT_NETCTRL_UIUPDATE,
        EVENT_NETCTRL_APPCONNECT_SUCCESS,
        EVENT_NETCTRL_APPDISCONNECT,
        EVENT_NETCTRL_APPCONNECT_SETTING,
    #ifdef SERVICES_SPEECH_ON
        EVENT_MODEMNG_SPEECHMNG_STARTREC,
        EVENT_MODEMNG_SPEECHMNG_STOPREC,
        EVENT_MODEMNG_SPEECHMNG_OPENFRONT,
        EVENT_MODEMNG_SPEECHMNG_OPENREAR,
        EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN,
        EVENT_MODEMNG_SPEECHMNG_OPENSCREEN,
        EVENT_MODEMNG_SPEECHMNG_EMRREC,
        EVENT_MODEMNG_SPEECHMNG_PIV,
        EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI,
        EVENT_MODEMNG_SPEECHMNG_OPENWIFI,
        EVENT_MODEMNG_START_SPEECH,
        EVENT_MODEMNG_STOP_SPEECH
    #endif
    };

    ret = EVENTHUB_RegisterTopic(EVENT_UI_TOUCH);
    APPCOMM_CHECK_RETURN(ret, ret);

    ret = EVENTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);
    if (ret != 0) {
        CVI_LOGE("EVENTHUB_CreateSubscriber failed! \n");
    }

    uint32_t u32ArraySize = UI_ARRAY_SIZE(topic);

    for (i = 0; i < u32ArraySize; i++) {
        ret = EVENTHUB_Subcribe(SubscriberHdl, topic[i]);
        if (ret) {
            CVI_LOGE("Subscribe topic(%#x) failed. %#x\n", topic[i], ret);
            continue;
        }
    }

    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
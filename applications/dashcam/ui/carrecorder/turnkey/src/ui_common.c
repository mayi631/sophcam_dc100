#include <stdio.h>
#include "ui_windowmng.h"
#include "event_recorder_player.h"
#ifdef CONFIG_GSENSOR_ON
#include "gsensormng.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static bool s_bPowerOff = false;

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

static int32_t  UI_Common_PowerOff(void)
{
    MESSAGE_S Msg = {0};
    uint32_t s32Ret = 0;
#ifdef SCREEN_ON
    //close backlight
    HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_OFF);
#endif
    #ifdef CONFIG_GSENSOR_ON
    PARAM_MENU_S menu_param = {0};
    PARAM_GetMenuParam(&menu_param);
    if (menu_param.Parking.Current == MENU_PARKING_ON) {
        GSENSORMNG_OpenInterrupt(0);
    }
    #endif
    if (s_bPowerOff == false) {
        Msg.topic = EVENT_MODEMNG_POWEROFF;
        s32Ret = MODEMNG_SendMessage(&Msg);
        if (0 != s32Ret) {
            CVI_LOGI("MODEMNG_SendMessage fail\n");
            return -1;
        }
        s_bPowerOff = true;
    }

    return 0;
}

static void ui_keyton_voice(void)
{
    VOICEPLAY_VOICE_S stVoice=
    {
        .au32VoiceIdx={UI_VOICE_KEY_IDX},
        .u32VoiceCnt=1,
        .bDroppable=true,
    };
    VOICEPLAY_Push(&stVoice, 0);
}

static int32_t  ui_key_event(int32_t  keyid, bool longkey)
{
    event_queue_req_t r;
    key_event_t event;

    memset(&r, 0x00, sizeof(r));
    memset(&event, 0x00, sizeof(event));

    int32_t  key = 0;
    switch(keyid)
    {
        case KEYMNG_KEY_IDX_0:
            key = UI_KEY_POWER;
            if (longkey == true) {
                CVI_LOGD("POWER OFF !\n");
                UI_Common_PowerOff();
                return 0;
            }
            break;
        case KEYMNG_KEY_IDX_1:
            key = UI_KEY_UP;
            break;
        case KEYMNG_KEY_IDX_2:
            key = UI_KEY_MENU;
            break;
        case KEYMNG_KEY_IDX_3:
            key = UI_KEY_DOWN;
            break;
    }
    event.key = key;
    event.alt = longkey;
    event.e.type = EVT_UI_KEY_DOWN;

    r.key_event = event;
    main_loop_queue_event(main_loop(), &r);

    return 0;
}

static int32_t  ui_battery_event(int32_t  level)
{
    event_queue_req_t r;
    key_event_t event;

    memset(&r, 0x00, sizeof(r));
    memset(&event, 0x00, sizeof(event));
    event.key = level;
    event.alt = 0;
    event.e.type = EVT_LOW_BATTERY;

    r.key_event = event;
    main_loop_queue_event(main_loop(), &r);

    return 0;
}

static uint8_t langid = 0;
static ret_t ui_common_setlanguage(const idle_info_t* idle)
{
    uint8_t type = *(uint8_t*)(idle->ctx);
    option_setuiLanguage(type);

    return RET_OK;
}

int32_t  ui_common_eventcb(void *argv, EVENT_S *msg)
{
    static int32_t  key_tong;
    int32_t  s32Ret = 0;

    if (s_bPowerOff == true) {
        CVI_LOGI("power off ignore event id: %x\n", msg->topic);
        return 0;
    }

    /*play key tone*/
    PARAM_GetKeyTone(&key_tong);
    if (key_tong == MEDIA_AUDIO_KEYTONE_ON) {
        if (msg->topic == EVENT_KEYMNG_LONG_CLICK ||
            msg->topic == EVENT_KEYMNG_SHORT_CLICK) {
            ui_keyton_voice();
        }
    }

    /*receive message result*/
    s32Ret = UICOMM_MessageResult(msg);
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");

    /*receive event control power*/
    bool bEventContinueHandle  = false;
    s32Ret = UI_POWERCTRL_PreProcessEvent(msg, &bEventContinueHandle);
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "PreProcessEvent");
    if (!bEventContinueHandle) {
        CVI_LOGI("Event %x has been processed by Power Control Module\n", msg->topic);
        return 0;
    }

    /*get cur mode*/
    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();

    switch(msg->topic) {
        case EVENT_MODEMNG_CARD_FORMATING:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_PROCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMATING\n");
            return 0;
        }
        case EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_SUCCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMAT_SUCCESSED\n");
            return 0;
        case EVENT_MODEMNG_CARD_FORMAT_FAILED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FORMAT_FAILED);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMAT_FAILED\n");
            return 0;
        }
        case EVENT_MODETEST_START_RECORD:
        {
            event_recorder_player_start_record("/mnt/data/ui_record.txt");
            return 0;
        }
        case EVENT_MODETEST_STOP_RECORD:
        {
            CVI_LOGD("EVENT_MODETEST_STOP_RECORD\n");
            event_recorder_player_stop_record();
            return 0;
        }
        case EVENT_MODETEST_PLAY_RECORD:
        {
            CVI_LOGD("EVENT_MODETEST_PLAY_RECORD\n");
            event_recorder_player_start_play("/mnt/data/ui_record.txt", msg->arg1);
            return 0;
        }
        case EVENT_KEYMNG_LONG_CLICK:
        {
            if ((0 == NETCTRL_NetToUiConnectState()) &&
                (s_stMessageCtx.bMsgProcessed == true)) {
                ui_key_event(msg->arg1, true);
            }
            return 0;
        }
        case EVENT_KEYMNG_SHORT_CLICK:
        {
            if ((0 == NETCTRL_NetToUiConnectState()) &&
                (s_stMessageCtx.bMsgProcessed == true)) {
                ui_key_event(msg->arg1, false);
            }
            return 0;
        }
        case EVENT_USB_OUT:
        {
            UI_Common_PowerOff();
            return 0;
        }
        case EVENT_MODEMNG_SETTING_LANGUAGE:
        {
            langid = msg->arg1;
            idle_queue(ui_common_setlanguage, (void*)&langid);
            return 0;
        }
        case EVENT_MODEMNG_MODEOPEN:
        case EVENT_MODEMNG_MODECLOSE:
            s32CurMode = msg->arg1;
            break;
        case EVENT_MODEMNG_UPFILE_SUCCESSED:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE_SUCCESSED);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGD("EVENT_MODEMNG_START_UPFILE\n");
            return 0;
        }
        case EVENT_MODEMNG_UPFILE_FAIL:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE_FAIL);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_MOVIE;
            MODEMNG_SendMessage(&Msg);
            CVI_LOGD("EVENT_MODEMNG_UPFILE_FAIL\n");
            return 0;
        }
        case EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE_FAIL_FILE_ERROR);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_MOVIE;
            MODEMNG_SendMessage(&Msg);
            CVI_LOGD("EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR\n");
            return 0;
        }
        case EVENT_GAUGEMNG_LEVEL_CHANGE:
        {
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            ui_battery_event(msg->arg1);
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
        default:
            break;
    }
    if (s32CurMode == WORK_MODE_MOVIE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == WORK_MODE_PLAYBACK) {
        ui_playbackpage_eventcb(argv, msg);
    } else if(s32CurMode == WORK_MODE_UPDATE) {
        ui_wrnmsg_update_type(MSG_EVENT_ID_OTA_UP_FILE);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    } else if (s32CurMode == WORK_MODE_LAPSE) {
        ui_homepage_eventcb(argv, msg);
    } else if (s32CurMode == WORK_MODE_UVC) {
        ui_wrnmsg_update_type(MSG_EVENT_ID_UVC);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
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
        EVENT_MODEMNG_SETTING_LANGUAGE,
        EVENT_MODEMNG_START_UPFILE,
        EVENT_MODEMNG_UPFILE_SUCCESSED,
        EVENT_MODEMNG_UPFILE_FAIL,
        EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR,
        EVENT_MODEMNG_RECODER_STARTSTATU,
        EVENT_MODEMNG_RECODER_STOPSTATU,
        EVENT_MODEMNG_RECODER_SPLITREC,
        EVENT_MODEMNG_RECODER_STARTEVENTSTAUE,
        EVENT_MODEMNG_RECODER_STOPEVENTSTAUE,
        EVENT_MODEMNG_RECODER_STARTEMRSTAUE,
        EVENT_MODEMNG_RECODER_STOPEMRSTAUE,
        EVENT_MODEMNG_RECODER_STARTPIVSTAUE,
        EVENT_MODETEST_START_RECORD,
        EVENT_MODETEST_STOP_RECORD,
        EVENT_MODETEST_PLAY_RECORD,
        EVENT_KEYMNG_LONG_CLICK,
        EVENT_KEYMNG_SHORT_CLICK,
        EVENT_GSENSORMNG_COLLISION,
        EVENT_USB_OUT,
        EVENT_USB_INSERT,
        EVENT_NETCTRL_APPCONNECT_SUCCESS,
        EVENT_MODEMNG_CARD_FORMAT,
        EVENT_NETCTRL_UIUPDATE,
        EVENT_GAUGEMNG_LEVEL_CHANGE,
        EVENT_GAUGEMNG_LEVEL_LOW,
        EVENT_GAUGEMNG_LEVEL_ULTRALOW,
        EVENT_GAUGEMNG_LEVEL_NORMAL,
        EVENT_GAUGEMNG_CHARGESTATE_CHANGE,
    };

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
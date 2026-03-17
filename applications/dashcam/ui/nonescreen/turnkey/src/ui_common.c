#include <stdio.h>
#include "ui_common.h"
//#include "event_recorder_player.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static UI_MESSAGE_CONTEXT s_stMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static uint32_t type;
static bool key_power_off = false;
int32_t  PowerButton_Event(void);

#ifndef CONFIG_SERVICES_LIVEVIEW_ON
#define VOICE_MAX_SEGMENT_CNT (5)
typedef struct _VOICEPLAY_VOICE_S
{
    uint32_t volume;
    uint32_t u32VoiceCnt;
    uint32_t au32VoiceIdx[VOICE_MAX_SEGMENT_CNT];
    bool bDroppable;
} VOICEPLAY_VOICE_S;
#endif

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
            type = EVT_NO_SDCARD;
            break;
        case CARD_STATE_AVAILABLE:
            return true;
            break;
        case CARD_STATE_ERROR:
            type = EVT_SDCARD_ERROR;
            break;
        case CARD_STATE_FSERROR:
        case CARD_STATE_UNAVAILABLE:
            type = EVT_SDCARD_NEED_FORMAT;
            break;
        case CARD_STATE_SLOW:
            type = EVT_SDCARD_SLOW;
            break;
        case CARD_STATE_CHECKING:
            type = EVT_SDCARD_CHECKING;
            break;
        case CARD_STATE_READ_ONLY:
            type = EVT_SDCARD_READ_ONLY;
            break;
        case CARD_STATE_MOUNT_FAILED:
            type = EVT_SDCARD_MOUNT_FAILED;
            break;
        default:
            CVI_LOGE("value is invalid\n");
            break;
    }
    MODEMNG_SetParkingRec(false);

    MODEMNG_GetCurMode(&CurMode);
    if (CurMode == WORK_MODE_MOVIE) {
    }

    return false;
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
            ui_close_flag = true;
        } else {
            CVI_LOGE("pannel is going open\n");
            ui_close_flag = false;
        }

    }
    return 0;
}

int32_t  ui_common_eventcb(void *argv, EVENT_S *msg)
{
    MESSAGE_S Msg = {0};
    int32_t  s32Ret = 0;
    /*receive message result*/
    s32Ret = UICOMM_MessageResult(msg);
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");

    if (!(EVENT_MODEMNG_RECODER_STARTSTATU  == msg->topic || EVENT_MODEMNG_RECODER_STOPSTATU ==msg->topic))
    {

    }

    switch(msg->topic) {
        case EVENT_MODEMNG_CARD_REMOVE:
            {
                if (MODEMNG_GetCurWorkMode() == WORK_MODE_PLAYBACK) {
                    Msg.topic = EVENT_MODEMNG_MODESWITCH;
                    Msg.arg1 = WORK_MODE_MOVIE;
                    MODEMNG_SendMessage(&Msg);
                } else {
                    ui_common_cardstatus();
                }
            }
            break;
        case EVENT_MODEMNG_CARD_AVAILABLE:
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
        case EVENT_MODEMNG_CARD_FSERROR:
        case EVENT_MODEMNG_CARD_SLOW:
        case EVENT_MODEMNG_CARD_CHECKING:
        case EVENT_MODEMNG_CARD_UNAVAILABLE:
        case EVENT_MODEMNG_CARD_ERROR:
        case EVENT_MODEMNG_CARD_READ_ONLY:
        case EVENT_MODEMNG_CARD_MOUNT_FAILED:
            ui_common_cardstatus();
            break;
        case EVENT_MODEMNG_MODEOPEN:
            {
                CVI_LOGD("EVENT_MODEMNG_MODEOPEN\n");
                switch(msg->arg1) {
                    case WORK_MODE_MOVIE:
                        break;
                    case WORK_MODE_PLAYBACK:
                        break;
                    case WORK_MODE_USBCAM:
                        break;
                    case WORK_MODE_USB:
                        break;
                    case WORK_MODE_UVC:
                        break;
                    case WORK_MODE_STORAGE:
                        break;
                    default:
                        break;
                }
            }
            break;
        case EVENT_MODEMNG_MODECLOSE:
            {
                CVI_LOGD("EVENT_MODEMNG_MODECLOSE\n");
                switch(msg->arg1) {
                    case WORK_MODE_MOVIE:
                        break;
                    case WORK_MODE_PLAYBACK:
                        break;
                    case WORK_MODE_USBCAM:
                        break;
                    case WORK_MODE_USB:
                        break;
                    default:
                        break;
                }
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
            // MODEMNG_SetEmrState(false);
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case EVENT_MODEMNG_CARD_FORMATING:
        {
            type = EVT_FORMAT_PROCESS;
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMATING\n");
            break;
        }
        case EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
            type = EVT_FORMAT_SUCCESS;
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMAT_SUCCESSED\n");
            break;
        case EVENT_MODEMNG_CARD_FORMAT_FAILED:
        {
            type = EVT_FORMAT_FAILED;
            CVI_LOGD("EVENT_MODEMNG_CARD_FORMAT_FAILED\n");
            break;
        }
        case EVENT_MODEMNG_PLAYBACK_FINISHED:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_FINISHED\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PROGRESS:
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
            type = EVT_FILE_ABNORMAL;
            CVI_LOGD("CVI_EVENT_PLAYBACKMODE_FILE_ABNORMAL\n");
            break;
        }
        case EVENT_MODETEST_START_RECORD:
        {
            CVI_LOGD("EVENT_MODETEST_START_RECORD\n");
            CVI_LOGE(" noscreen not suport !\n");
            //event_recorder_player_start_record("/mnt/data/ui_record.txt");
            break;
        }
        case EVENT_MODETEST_STOP_RECORD:
        {
            CVI_LOGD("EVENT_MODETEST_STOP_RECORD\n");
            CVI_LOGE(" noscreen not suport !\n");
            // event_recorder_player_stop_record();
            break;
        }
        case EVENT_MODETEST_PLAY_RECORD:
        {
            CVI_LOGE(" noscreen not suport !\n");
            CVI_LOGD("EVENT_MODETEST_PLAY_RECORD\n");
            // event_recorder_player_start_play("/mnt/data/ui_record.txt", msg->arg1);
            break;
        }
#ifdef SERVICES_IMAGE_VIEWER_ON
        case CVI_EVENT_AHDMNG_PLUG_STATUS:
        {
            if (msg->arg1 == CVI_AHDMNG_PLUG_OUT) {
                PARAM_SetMenuParam(0, PARAM_MENU_VIEW_WIN_STATUS, CVI_MEDIA_VIEW_WIN_FRONT);
            } else if (msg->arg1 == CVI_AHDMNG_PLUG_IN) {
                PARAM_SetMenuParam(0, PARAM_MENU_VIEW_WIN_STATUS, CVI_MEDIA_VIEW_WIN_DOUBLE);
            }
            break;
        }
#endif
        case EVENT_KEYMNG_LONG_CLICK:
        {
            break;
        }
        case EVENT_KEYMNG_SHORT_CLICK:
        {
            break;
        }
        case EVENT_MODEMNG_RECODER_STARTPIVSTAUE:
        {
#ifdef SERVICES_LIVEVIEW_ON
            if (msg->arg1 == 0)  {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_PHOTO_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=false,
                };
                VOICEPLAY_Push(&stVoice, 0);
            }
#endif
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
        EVENT_MODETEST_START_RECORD,
        EVENT_MODETEST_STOP_RECORD,
        EVENT_MODETEST_PLAY_RECORD,
#ifdef SERVICES_IMAGE_VIEWER_ON
        CVI_EVENT_AHDMNG_PLUG_STATUS,
#endif
        EVENT_KEYMNG_LONG_CLICK,
        EVENT_KEYMNG_SHORT_CLICK,
        EVENT_STORAGEMNG_DEV_CONNECTING,
        WORK_MODE_UVC,
        WORK_MODE_STORAGE,
        EVENT_MODEMNG_UVC_MODE_START,
        EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV
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
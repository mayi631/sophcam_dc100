#include <stdio.h>
#include <sys/statfs.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <errno.h>
#include "netctrlinner.h"

#define NETCTRL_ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

static PDT_NETCTRL_MESSAGE_CONTEXT s_stNETCTRLMessageCtx = {.bMsgProcessed = true, .MsgMutex = PTHREAD_MUTEX_INITIALIZER,};
static sem_t s_NETCTRLSem;
static int32_t flag_app_connect = WIFI_APP_DISCONNECT;
static int32_t s_counttime = 0;
static int32_t sd_status = 0;
static uint32_t s_TimedTaskHdl;

void NETCTRLINNER_ScanFile()
{
}

int32_t NETCTRLINNER_InitTimer()
{
    int32_t result = 0;;
    TIMEDTASK_CFG_S stTimedTaskCfg;
    stTimedTaskCfg.timerProc = NETCTRLINNER_Enablecheckconnect;
    stTimedTaskCfg.pvPrivData = NULL;
    stTimedTaskCfg.stAttr.bEnable = true;
    stTimedTaskCfg.stAttr.u32Time_sec = 1;
    stTimedTaskCfg.stAttr.periodic = true;
    result = TIMEDTASK_Create(&stTimedTaskCfg, &s_TimedTaskHdl);
    return result;
}

int32_t NETCTRLINNER_DeInitTimer()
{
    int32_t result = 0;;
    result = TIMEDTASK_Destroy(s_TimedTaskHdl);
    return result;
}

int32_t NETCTRLINNER_StopTimer()
{
    int32_t result = 0;
    TIMEDTASK_ATTR_S TimedTaskAttr;
    TIMEDTASK_GetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    if (TimedTaskAttr.bEnable == true) {
        TimedTaskAttr.bEnable = false;
        result = TIMEDTASK_SetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    }
    return result;
}

int32_t NETCTRLINNER_StartTimer()
{
    int32_t result = 0;
    TIMEDTASK_ATTR_S TimedTaskAttr;
    TIMEDTASK_GetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    if (TimedTaskAttr.bEnable == false) {
        TimedTaskAttr.bEnable = true;
        result = TIMEDTASK_SetAttr(s_TimedTaskHdl, &TimedTaskAttr);
    }
    return result;
}

void NETCTRLINNER_UiUpdate()
{
    EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(EVENT_S));
    stEvent.topic = EVENT_NETCTRL_UIUPDATE;
    EVENTHUB_Publish(&stEvent);
}

static int NETCTRLINNER_MessageResult(EVENT_S *pstEvent)
{
    int s32Ret = 0;
    MUTEX_LOCK(s_stNETCTRLMessageCtx.MsgMutex);

    if (!s_stNETCTRLMessageCtx.bMsgProcessed) {
        CVI_LOGD("event(%x)\n\n", pstEvent->topic);
        if ((s_stNETCTRLMessageCtx.stMsg.topic == pstEvent->topic)
            && (s_stNETCTRLMessageCtx.stMsg.arg1 == pstEvent->arg1)
            && (s_stNETCTRLMessageCtx.stMsg.arg2 == pstEvent->arg2)) {
                sem_post(&s_NETCTRLSem);
                s_stNETCTRLMessageCtx.bMsgProcessed = true;
        }
    }

    MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);

    return s32Ret;
}

int32_t NETCTRLINNER_SendSyncMsg(MESSAGE_S* pstMsg, int* ps32Result)
{
    int s32Ret = 0;
    MUTEX_LOCK(s_stNETCTRLMessageCtx.MsgMutex);

    if (!s_stNETCTRLMessageCtx.bMsgProcessed) {
        CVI_LOGE("Current Msg not finished\n");
        MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
        return -1;
    }

    s_stNETCTRLMessageCtx.bMsgProcessed = false;
    s_stNETCTRLMessageCtx.stMsg.topic = pstMsg->topic;
    s_stNETCTRLMessageCtx.stMsg.arg1 = pstMsg->arg1;
    s_stNETCTRLMessageCtx.stMsg.arg2 = pstMsg->arg2;
    s_stNETCTRLMessageCtx.stMsg.s32Result = -1;
    memcpy(s_stNETCTRLMessageCtx.stMsg.aszPayload, pstMsg->aszPayload, sizeof(s_stNETCTRLMessageCtx.stMsg.aszPayload));

    s32Ret = MODEMNG_SendMessage(pstMsg);

    if (0 != s32Ret)
    {
        CVI_LOGE("Send Message Error:%#x\n", s32Ret);
        s_stNETCTRLMessageCtx.bMsgProcessed = true;
        *ps32Result = 1;
        MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);
        return -1;
    }
    MUTEX_UNLOCK(s_stNETCTRLMessageCtx.MsgMutex);

    while ((0 != sem_wait(&s_NETCTRLSem)) && (errno == EINTR));

    *ps32Result = 0;

    s_stNETCTRLMessageCtx.stMsg.topic = 0;
    s_stNETCTRLMessageCtx.stMsg.arg1 = 0;
    s_stNETCTRLMessageCtx.stMsg.arg2 = 0;
    s_stNETCTRLMessageCtx.bMsgProcessed = true;
    return 0;
}

static int NETCTRLINNER_Eventcb(void *argv, EVENT_S *msg)
{
    int s32Ret = 0;
    if (msg->topic == EVENT_NETCTRL_CONNECT) {
        NETCTRLINNER_TimeoutRest();
        return 0;
    }
    /*receive message result*/
    s32Ret = NETCTRLINNER_MessageResult(msg);
    switch(msg->topic){
        case EVENT_MODEMNG_SETTING:
            if(msg->arg1 == PARAM_MENU_DEFAULT){
                CVI_LOGD("PARAM_MENU_DEFAULT\n" );
            }
            break;

        case EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
            sd_status = WIFI_SD_FORMAT_SUCCESS;
            break;

        case EVENT_MODEMNG_CARD_FORMAT_FAILED:
            sd_status = WIFI_SD_FORMAT_FAIL;
            break;

        default:
            break;
    }
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "MessageResult");
    return 0;
}

int32_t NETCTRLINNER_SubscribeEvents(void)
{
    int32_t ret = 0;
    uint32_t i = 0;
    EVENTHUB_SUBSCRIBER_S stnetSubscriber = {"net", NULL, NETCTRLINNER_Eventcb, false};
    MW_PTR netSubscriberHdl = NULL;
    TOPIC_ID topic[] = {
        EVENT_MODEMNG_SETTING,
        EVENT_MODEMNG_STOP_REC,
        EVENT_MODEMNG_START_REC,
        EVENT_MODEMNG_LIVEVIEW_UPORDOWN,
        EVENT_MODEMNG_START_PIV,
        EVENT_MODEMNG_MODESWITCH,
        EVENT_MODEMNG_CARD_FORMAT,
        EVENT_MODEMNG_START_UPFILE,
        EVENT_MODEMNG_SWITCH_LIVEVIEW,
        EVENT_MODEMNG_CARD_FORMAT_FAILED,
        EVENT_MODEMNG_CARD_FORMAT_SUCCESSED,
        EVENT_MODEMNG_RTSP_INIT,
        EVENT_MODEMNG_RTSP_SWITCH,
        EVENT_MODEMNG_RTSP_DEINIT,
        EVENT_NETCTRL_CONNECT,
    };

    sem_init(&s_NETCTRLSem, 0, 0);

    ret = EVENTHUB_CreateSubscriber(&stnetSubscriber, &netSubscriberHdl);
    if (ret != 0) {
        CVI_LOGE("EVENTHUB_CreateSubscriber failed! \n");
    }

    uint32_t u32ArraySize = NETCTRL_ARRAY_SIZE(topic);

    for (i = 0; i < u32ArraySize; i++) {
        ret = EVENTHUB_Subcribe(netSubscriberHdl, topic[i]);
        if (ret) {
            CVI_LOGE("Subscribe topic(%#x) failed. %#x\n", topic[i], ret);
            continue;
        }
    }

    return ret;
}

void NETCTRLINNER_Enablecheckconnect(void *privData, struct timespec *now)
{
    EVENT_S stEvent;
    if (s_counttime > 0) {
        s_counttime--;
        if (flag_app_connect == WIFI_APP_DISCONNECT) {
            flag_app_connect = WIFI_APP_CONNECTTED;
            stEvent.topic = EVENT_NETCTRL_APPCONNECT_SUCCESS;
            EVENTHUB_Publish(&stEvent);
            NETCTRLINNER_InitCMDSocket();
        }
    } else {
        // If there is no message in 10 second timing, it will be disconnected by default
        if (0 == s_counttime) {
            s_counttime = -1;
            if (flag_app_connect == WIFI_APP_CONNECTTED) {
                flag_app_connect = WIFI_APP_DISCONNECT;
                stEvent.topic = EVENT_NETCTRL_APPDISCONNECT;
                stEvent.arg1 = NETCTRLINNER_GetFlagFile();
                EVENTHUB_Publish(&stEvent);
                NETCTRLINNER_DeInitCMDSocket();
            }
        }
    }
}

int32_t NETCTRLINNER_TimeoutRest(void)
{
    s_counttime = TIMECOUNTMAX;
    return 0;
}

int32_t NETCTRLINNER_APPConnetState(void)
{
    return flag_app_connect;
}

int32_t NETCTRLINNER_GetSdState(void)
{
    return sd_status;
}

void NETCTRLINNER_SetSdState(int32_t value)
{
    sd_status = value;
}
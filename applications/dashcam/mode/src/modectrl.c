#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "modeinner.h"
#include "cvi_log.h"

static TOPIC_ID topic[] = {
    EVENT_STORAGEMNG_DEV_UNPLUGED, // 0
    EVENT_STORAGEMNG_DEV_CONNECTING,
    EVENT_STORAGEMNG_DEV_ERROR,
    EVENT_STORAGEMNG_FS_CHECKING,
    EVENT_STORAGEMNG_FS_CHECK_FAILED,
    EVENT_STORAGEMNG_FS_EXCEPTION,
    EVENT_STORAGEMNG_MOUNTED,
    EVENT_STORAGEMNG_MOUNT_FAILED,
    EVENT_STORAGEMNG_MOUNT_READ_ONLY,
    EVENT_RECMNG_STARTREC,
    EVENT_RECMNG_STOPREC,
    EVENT_RECMNG_SPLITREC,
    EVENT_RECMNG_SPLITSTART,
    EVENT_RECMNG_STARTEVENTREC,
    EVENT_RECMNG_EVENTREC_END,
    EVENT_RECMNG_STARTEMRREC,
    EVENT_RECMNG_EMRREC_END,
    EVENT_RECMNG_WRITE_ERROR, // 20
    EVENT_RECMNG_OPEN_FAILED,
    EVENT_RECMNG_PIV_END,
    EVENT_RECMNG_PIV_START,
#ifdef SERVICES_PHOTO_ON
    EVENT_PHOTOMNG_PIV_START,
    EVENT_PHOTOMNG_PIV_END,
    EVENT_PHOTOMNG_PIV_ERROR,
#endif
    EVENT_FILEMNG_SCAN_FAIL,
    EVENT_FILEMNG_SCAN_COMPLETED, // 24
#ifdef SERVICES_PLAYER_ON
    EVENT_PLAYBACKMNG_PLAY,
    EVENT_PLAYBACKMNG_FINISHED,
    EVENT_PLAYBACKMNG_PROGRESS,
    EVENT_PLAYBACKMNG_PAUSE,
    EVENT_PLAYBACKMNG_RESUME,
    EVENT_PLAYBACKMNG_FILE_ABNORMAL,
#endif
    EVENT_SENSOR_PLUG_STATUS,
    EVENT_USB_UVC_READY,
    EVENT_USB_STORAGE_READY,
    EVENT_USB_HOSTUVC_READY,
#ifdef SERVICES_SPEECH_ON
    EVENT_SPEECHMNG_STARTREC,
    EVENT_SPEECHMNG_STOPREC,
    EVENT_SPEECHMNG_OPENFRONT,
    EVENT_SPEECHMNG_OPENREAR,
    EVENT_SPEECHMNG_CLOSESCREEN,
    EVENT_SPEECHMNG_OPENSCREEN,
    EVENT_SPEECHMNG_EMRREC,
    EVENT_SPEECHMNG_PIV,
    EVENT_SPEECHMNG_CLOSEWIFI,
    EVENT_SPEECHMNG_OPENWIFI,
#endif
};

int32_t MODEMNG_Eventcb(void *argv, EVENT_S *msg)
{
    int32_t s32Ret = 0;
    MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();

    /** check init */
    MODEMNG_CHECK_CHECK_INIT(pstModeMngCtx->bInited, MODE_ENOTINIT, "ModeMng module has not been inited");

    /** check paramerter */
    MODEMNG_CHECK_POINTER(msg, MODE_ENULLPTR, "msg");
    MODEMNG_CHECK_POINTER(argv, MODE_ENULLPTR, "argv");

    /** check whether it has been intialized or not */
    MUTEX_LOCK(pstModeMngCtx->Mutex);

    /** push message to state machine queue */
    s32Ret = HFSM_SendAsyncMessage(pstModeMngCtx->pvHfsmHdl, (MESSAGE_S *)msg);
    MODEMNG_CHECK_CHECK_RET_WITH_UNLOCK(s32Ret, MODE_EINTER, "send message to HFSM(from eventhub)");

    MUTEX_UNLOCK(pstModeMngCtx->Mutex);
    return s32Ret;
}

static uint32_t MODEMNG_SubscribeEvents(void)
{
    int32_t ret = 0;
    uint32_t i = 0;
    EVENTHUB_SUBSCRIBER_S stSubscriber = {"modemng", NULL, MODEMNG_Eventcb, false};
    MW_PTR SubscriberHdl = NULL;

    ret = EVENTHUB_CreateSubscriber(&stSubscriber, &SubscriberHdl);
    MODEMNG_CHECK_RET(ret, MODE_EINTER, "EVENTHUB_CreateSubscriber failed!");

    uint32_t u32ArraySize = MODE_ARRAY_SIZE(topic);
    for (i = 0; i < u32ArraySize; i++) {
        ret = EVENTHUB_Subcribe(SubscriberHdl, topic[i]);
        if (ret) {
            CVI_LOGE("Subscribe topic(%#x) failed. %#x\n", topic[i], ret);
            continue;
        }
    }
    return ret;
}

static bool MODEMNG_CheckPulishEvent(uint32_t eventid)
{
    uint32_t u32ArraySize = MODE_ARRAY_SIZE(topic);
    uint32_t i = 0;
    for (i = 0; i < u32ArraySize; i++) {
        if (eventid == topic[i]) {
            return false;
        }
    }

    return true;
}

/** message process callback function for HFSM module */
static int32_t MODEMNG_HfsmEventProc(HFSM_HANDLE pvHfsmHdl,
                                         const HFSM_EVENT_INFO_S* pstEventInfo)
{
    MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();

    MODEMNG_CHECK_POINTER(pstEventInfo, MODE_ENULLPTR, "parameter error");
    MODEMNG_CHECK_RET((pvHfsmHdl != pstModeMngCtx->pvHfsmHdl), MODE_EINVAL, "parameter error");

    /** check event info */
    if (HFSM_EVENT_UNHANDLE_MSG == pstEventInfo->enEventCode) {
        CVI_LOGD("HFSM_EVENT_UNHANDLE_MSG\n");
    } else if(HFSM_EVENT_TRANSTION_ERROR == pstEventInfo->enEventCode) {
        CVI_LOGD("HFSM_EVENT_TRANSTION_ERROR \n");
    } else if(HFSM_EVENT_HANDLE_MSG == pstEventInfo->enEventCode) {
        CVI_LOGD("HFSM_EVENT_HANDLE_MSG  %d \n", pstModeMngCtx->bInProgress);
    } else {
        CVI_LOGD("pstEventInfo enEventCode[%d] not support\n",pstEventInfo->enEventCode);
    }
    MUTEX_LOCK(pstModeMngCtx->Mutex);

    CVI_LOGD("pstEventInfo->pstunHandlerMsg->topic: %x\n", pstEventInfo->pstunHandlerMsg->topic);
    if (true == pstModeMngCtx->bInProgress) {
        pstModeMngCtx->bInProgress = false;
    }
    if (MODEMNG_CheckPulishEvent(pstEventInfo->pstunHandlerMsg->topic)){
        MODEMNG_PublishEvent(pstEventInfo->pstunHandlerMsg, 0);
    }
    MUTEX_UNLOCK(pstModeMngCtx->Mutex);

    return 0;
}

/** create HFSM instance */
static int32_t MODEMNG_CreateHFSMInstance(void)
{
    int32_t s32Ret = 0;
    HFSM_ATTR_S stHfsmAttr;
    memset(&stHfsmAttr, 0, sizeof(HFSM_ATTR_S));
    stHfsmAttr.fnHfsmEventCallback = MODEMNG_HfsmEventProc;
    stHfsmAttr.u32StateMaxAmount = WORK_MODE_BUTT+1;/**< include base */
    stHfsmAttr.u32MessageQueueSize = 32;
    s32Ret = HFSM_Create(&stHfsmAttr, &(MODEMNG_GetModeCtx()->pvHfsmHdl));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM instance create");

    return s32Ret;
}

/** destory HFSM instance */
static int32_t MODEMNG_DestoryHFSMInstance(void)
{
    int32_t s32Ret = 0;

    s32Ret = HFSM_Destroy(MODEMNG_GetModeCtx()->pvHfsmHdl);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM destory");

    return s32Ret;
}

/** init and add all states */
static int32_t MODEMNG_InitStates(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();

    s32Ret = MODEMNG_BaseStateInit();/** base state must be init at first*/
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Base mode init");

    s32Ret = MODEMNG_MovieStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Movie mode init");

    s32Ret = MODEMNG_PlaybackStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Playback mode init");

    s32Ret = MODEMNG_UpDateStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Update mode init");

    s32Ret = MODEMNG_UvcStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UVC mode init");

#ifdef SERVICES_PHOTO_ON
    s32Ret = MODEMNG_PhotoStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Photo mode init");
#endif

    s32Ret = MODEMNG_StorageStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Storage state init");

    s32Ret = MODEMNG_BootFirstStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "BootFirst mode init");

	s32Ret = MODEMNG_UsbMenuStatesInit(&(pstModeMngCtx->stBase));
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "usbmenu mode init");

#ifndef ENABLE_ISP_PQ_TOOL
    PARAM_CFG_S param;
    PARAM_GetParam(&param);
    if (param.Menu.UserData.bBootFirst) {
        pstModeMngCtx->CurWorkMode = WORK_MODE_BOOT;
    } else
#endif
    {
        pstModeMngCtx->CurWorkMode = WORK_MODE_BOOT;
    }

    return s32Ret;
}

static int32_t MODEMNG_DeinitStates(void)
{
    int32_t s32Ret = 0;
    MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();

    switch (pstModeMngCtx->CurWorkMode) {
        case WORK_MODE_MOVIE:
            s32Ret = MODEMNG_MovieStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Movie mode deinit");
            break;
        case WORK_MODE_PLAYBACK:
            s32Ret = MODEMNG_PlaybackStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Playback mode deinit");
        case WORK_MODE_UPDATE:
            s32Ret = MODEMNG_UpDateStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Update mode deinit");
            break;
        case WORK_MODE_PHOTO:
#ifdef SERVICES_PHOTO_ON
            s32Ret = MODEMNG_PhotoStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Photo mode deinit");
#endif
            break;
        case WORK_MODE_UVC:
            s32Ret = MODEMNG_UvcStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UVC mode deinit");
            break;
        case WORK_MODE_STORAGE:
            s32Ret = MODEMNG_StorageStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Storage mode deinit");
            break;
        case WORK_MODE_BOOT:
            s32Ret = MODEMNG_BootFirstStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "BOOT mode deinit");
            break;
        case WORK_MODE_USBMENU:
            s32Ret = MODEMNG_UsbMenuStatesDeinit();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Usbmenu mode deinit");
            break;
        default:
            CVI_LOGI("MODEMNG_DeinitStates unnormal state\n");
    }

    s32Ret = MODEMNG_BaseStatesDeinit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Base mode deinit");

    return s32Ret;
}

/** activate HFSM */
static int32_t MODEMNG_ActivateHFSM(WORK_MODE_E enWorkmode)
{
    int32_t s32Ret = 0;

    s32Ret = HFSM_SetInitialState(MODEMNG_GetModeCtx()->pvHfsmHdl, enWorkmode);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"HFSM set initial state");

    /** active HFSM */
    s32Ret = HFSM_Start(MODEMNG_GetModeCtx()->pvHfsmHdl);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"HFSM start");

    return s32Ret;
}

/** deactive HFSM */
static int32_t MODEMNG_DeactiveHFSM(void)
{
    int32_t s32Ret = 0;

    /** stop HFSM instance */
    s32Ret = HFSM_Stop(MODEMNG_GetModeCtx()->pvHfsmHdl);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"HFSM stop");

    return s32Ret;
}

/** Modemng initialization */
int32_t MODEMNG_Init(MODEMNG_CONFIG_S *stModemngCfg)
{
    int32_t s32Ret = 0;
    /** check the statemode_cfg*/
    if (NULL == stModemngCfg) {
        CVI_LOGE("stModemngCfg is null point\n");
        return MODE_EINTER;
    }

    /** check whether it has been intialized or not */
    if(true == MODEMNG_GetModeCtx()->bInited)
    {
        CVI_LOGE("StateMng module has already been inited\n\n");
        return MODE_EINITIALIZED;
    }

    s32Ret = MODEMNG_ContextInit(stModemngCfg);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"MODEMNG_ContextInit fail");

    /** create HFSM instance */
    s32Ret = MODEMNG_CreateHFSMInstance();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"HFSM instance create");

    /** init all States */
    s32Ret = MODEMNG_InitStates();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"add all states");

    /** subscribe event from EventHub module */
    s32Ret = MODEMNG_SubscribeEvents();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"subscribe events");
    MODEMNG_GetModeCtx()->bInited = true;

    /** activate HFSM */
    s32Ret = MODEMNG_ActivateHFSM(MODEMNG_GetModeCtx()->CurWorkMode);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"HFSM set initial state");

    /** Storage service init here */
    s32Ret = MODEMNG_InitStorage();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"Storage service init");

    return s32Ret;
}

/** Modemng deinitialization */
int32_t MODEMNG_Deinit()
{
    int32_t s32Ret = 0;

    /** check whether it has been deintialized or not */
    MODEMNG_CHECK_CHECK_INIT(MODEMNG_GetModeCtx()->bInited, 0, "StateMng module has already been deinited");

    /** deinit all states */
    s32Ret = MODEMNG_DeinitStates();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"deinit all states");

    /** deactive HFSM */
    s32Ret = MODEMNG_DeactiveHFSM();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"deactive HFSM");

    /** destory HFSM */
    s32Ret = MODEMNG_DestoryHFSMInstance();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"destory HFSM");

    /** destory Storage */
    s32Ret = MODEMNG_DeintStorage();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"destory Storage");

    MODEMNG_GetModeCtx()->bInited = false;

    return s32Ret;
}

/** send message to modemng with parameter, synchronize UI */
int32_t MODEMNG_SendMessage(const MESSAGE_S *pstMsg)
{
    int32_t s32Ret = 0;
    MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();
    CVI_LOGD("TOPIC: %s, ARG1: %d, ARG2: %d", event_topic_get_name(pstMsg->topic), pstMsg->arg1, pstMsg->arg2);

    /** check paramerter */
    MODEMNG_CHECK_POINTER(pstMsg,MODE_ENULLPTR, "pstMsg");

    /** check whether it has been intialized or not */
    MODEMNG_CHECK_CHECK_INIT(pstModeMngCtx->bInited, MODE_ENOTINIT, "ModeMng module has not been inited");

    MUTEX_LOCK(pstModeMngCtx->Mutex);
    /** check whether it was in progress or not */
    if(true == pstModeMngCtx->bInProgress)
    {
        CVI_LOGE("StateMng module is InProgress\n\n");
        // MUTEX_UNLOCK(pstModeMngCtx->Mutex);
        // return MODE_EINPROGRESS;
    }

    s32Ret = HFSM_SendAsyncMessage(pstModeMngCtx->pvHfsmHdl, (MESSAGE_S *)pstMsg);
    MODEMNG_CHECK_CHECK_RET_WITH_UNLOCK(s32Ret,MODE_EINTER,"send message to HFSM(from Terminal)");

    pstModeMngCtx->bInProgress = true;
    MUTEX_UNLOCK(pstModeMngCtx->Mutex);

    return s32Ret;
}

int32_t MODEMNG_GetCurMode(uint32_t *ModeId)
{
    int32_t s32Ret = 0;
    MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();
    STATE_S state = {0};
    s32Ret = HFSM_GetCurrentState(pstModeMngCtx->pvHfsmHdl, &state);

    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"HFSM_GetCurrentState");
    *ModeId = state.stateID;

    return s32Ret;
}

int32_t MODEMNG_SetCurModeMedia(WORK_MODE_E CurMode)
{
    int32_t s32Ret = 0;
    PARAM_CFG_S *param_ptr = (PARAM_CFG_S *)malloc(sizeof(PARAM_CFG_S));
    if(param_ptr == NULL){
        CVI_LOGE("malloc failed");
        return -1;
    }

    s32Ret = PARAM_GetParam(param_ptr);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"HFSM_GetCurrentState");
    switch (CurMode) {
        case WORK_MODE_MOVIE:
            for (uint32_t i = 0; i < param_ptr->WorkModeCfg.RecordMode.CamNum; i++) {
                param_ptr->CamCfg[i].CamMediaInfo = param_ptr->WorkModeCfg.RecordMode.CamMediaInfo[i];
            }
            param_ptr->MediaComm.Vpss = param_ptr->WorkModeCfg.RecordMode.Vpss;
            break;
        case WORK_MODE_PHOTO:
            for (uint32_t i = 0; i < param_ptr->WorkModeCfg.PhotoMode.CamNum; i++) {
                param_ptr->CamCfg[i].CamMediaInfo = param_ptr->WorkModeCfg.PhotoMode.CamMediaInfo[i];
            }
            param_ptr->MediaComm.Vpss = param_ptr->WorkModeCfg.PhotoMode.Vpss;
            break;
        case WORK_MODE_PLAYBACK:
            break;
        case WORK_MODE_USB:
            break;
        default:
            CVI_LOGE("Not suporrt mode: %d\n", CurMode);
            break;
    }
    PARAM_SetParam(param_ptr);

    if(param_ptr != NULL){
        free(param_ptr);
        param_ptr = NULL;
    }

    return s32Ret;
}

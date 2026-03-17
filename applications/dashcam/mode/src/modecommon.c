#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "mmc.h"

#include "modeinner.h"

static uint32_t g_is_mode_proc_with_sensor = 1;

static MODEMNG_S s_stModeMng = {.Mutex = PTHREAD_MUTEX_INITIALIZER,
                            .bInited = false,
                            .bSysPowerOff = false,
                            .CurWorkMode = WORK_MODE_BUTT,};

MODEMNG_S* MODEMNG_GetModeCtx(void)
{
    return &s_stModeMng;
}

int32_t MODEMNG_GetCurWorkMode(void)
{
    return s_stModeMng.CurWorkMode;
}

uint32_t MODEMNG_GetCardState(void)
{
    return s_stModeMng.u32CardState;
}

bool MODEMNG_SetCardState(uint32_t u32CardState)
{
    bool ret = true;

    CVI_LOGD("SetCardState as 0x%x\n", u32CardState);

    switch(u32CardState) {
        case CARD_STATE_REMOVE:
        case CARD_STATE_UNAVAILABLE:
        case CARD_STATE_ERROR:
        case CARD_STATE_FSERROR:
        case CARD_STATE_CHECKING:
        case CARD_STATE_SLOW:
        case CARD_STATE_READ_ONLY:
        case CARD_STATE_MOUNT_FAILED:
        case CARD_STATE_FULL_SPACE:
            s_stModeMng.u32CardState = u32CardState;
            break;

        case CARD_STATE_AVAILABLE:
        case CARD_STATE_FORMATED:
            if (CARD_STATE_REMOVE == MODEMNG_GetCardState()) {
                CVI_LOGE("Card already be removed\n");
                ret = false;
                break;
            }
            s_stModeMng.u32CardState = u32CardState;
            break;
        default:
            CVI_LOGE("Not allowed CardState: 0x%x\n", u32CardState);
            ret = false;
            break;
    }

    return ret;
}

int32_t MODEMNG_GetModeState(uint32_t *state)
{
    *state = s_stModeMng.u32ModeState;

    return 0;
}

int32_t MODEMNG_SetModeState(uint32_t state)
{
    s_stModeMng.u32ModeState = state;

    return 0;
}

int32_t MODEMNG_SetParkingRec(bool en)
{
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->bInParkingRec = en;

    return 0;
}

int32_t MODEMNG_SetEmrState(bool en)
{
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->bInEmrRec = en;

    return 0;
}

bool MODEMNG_GetEmrState(void)
{
    return s_stModeMng.bInEmrRec;
}

bool MODEMNG_GetInProgress(void)
{
    return s_stModeMng.bInProgress;
}

int32_t MODEMNG_MonitorStatusNotify(EVENT_S* pstMsg)
{
    /* Publish Event */
    EVENT_S stEvent = {0};
    memcpy(&stEvent, pstMsg, sizeof(EVENT_S));
    switch (stEvent.topic) {
        case EVENT_STORAGEMNG_DEV_UNPLUGED:
            stEvent.topic = EVENT_MODEMNG_CARD_REMOVE;
            break;
        case EVENT_STORAGEMNG_DEV_ERROR:
            stEvent.topic = EVENT_MODEMNG_CARD_ERROR;
            break;
        case EVENT_STORAGEMNG_MOUNT_FAILED:
            stEvent.topic = EVENT_MODEMNG_CARD_MOUNT_FAILED;
            break;
        case EVENT_STORAGEMNG_MOUNT_READ_ONLY:
            stEvent.topic = EVENT_MODEMNG_CARD_READ_ONLY;
            break;
        case EVENT_STORAGEMNG_FS_EXCEPTION:
        case EVENT_STORAGEMNG_FS_CHECK_FAILED:
            stEvent.topic = EVENT_MODEMNG_CARD_FSERROR;
            break;
        case EVENT_STORAGEMNG_FS_CHECKING:
        case EVENT_STORAGEMNG_DEV_CONNECTING:
            stEvent.topic = EVENT_MODEMNG_CARD_CHECKING;
            break;
        case EVENT_SENSOR_PLUG_STATUS:
            stEvent.topic = EVENT_MODEMNG_RESET;
            break;
        case EVENT_USB_UVC_READY:
            stEvent.topic = EVENT_MODEMNG_MODESWITCH;
            break;

        case EVENT_FILEMNG_SCAN_COMPLETED:
            stEvent.topic = EVENT_MODEMNG_CARD_AVAILABLE;
            break;

        case EVENT_RECMNG_STARTREC:
            stEvent.topic = EVENT_MODEMNG_RECODER_STARTSTATU;
            break;
        case EVENT_RECMNG_STOPREC:
            stEvent.topic = EVENT_MODEMNG_RECODER_STOPSTATU;
            break;
        case EVENT_RECMNG_STARTEMRREC:
            stEvent.topic = EVENT_MODEMNG_RECODER_STARTEMRSTAUE;
            break;
        case EVENT_RECMNG_EMRREC_END:
            stEvent.topic = EVENT_MODEMNG_RECODER_STOPEMRSTAUE;
            break;
        case EVENT_RECMNG_STARTEVENTREC:
            stEvent.topic = EVENT_MODEMNG_RECODER_STARTEVENTSTAUE;
            break;
        case EVENT_RECMNG_EVENTREC_END:
            stEvent.topic = EVENT_MODEMNG_RECODER_STOPEVENTSTAUE;
            break;
        case EVENT_RECMNG_SPLITREC:
            stEvent.topic = EVENT_MODEMNG_RECODER_SPLITREC;
            break;
        case EVENT_RECMNG_PIV_END:
            stEvent.topic = EVENT_MODEMNG_RECODER_STOPPIVSTAUE;
            break;
        case EVENT_RECMNG_PIV_START:
            stEvent.topic = EVENT_MODEMNG_RECODER_STARTPIVSTAUE;
            break;
#ifdef SERVICES_PHOTO_ON
        case EVENT_PHOTOMNG_PIV_START:
            stEvent.topic = EVENT_MODEMNG_PHOTO_STARTPIVSTAUE;
            break;
#endif
#ifdef SERVICES_PLAYER_ON
        case EVENT_PLAYBACKMNG_PLAY:
            stEvent.topic = EVENT_MODEMNG_PLAYBACK_PLAY;
            break;
        case EVENT_PLAYBACKMNG_FINISHED:
            stEvent.topic = EVENT_MODEMNG_PLAYBACK_FINISHED;
            break;
        case EVENT_PLAYBACKMNG_PROGRESS:
            stEvent.topic = EVENT_MODEMNG_PLAYBACK_PROGRESS;
            break;
        case EVENT_PLAYBACKMNG_PAUSE:
            stEvent.topic = EVENT_MODEMNG_PLAYBACK_PAUSE;
            break;
        case EVENT_PLAYBACKMNG_RESUME:
            stEvent.topic = EVENT_MODEMNG_PLAYBACK_RESUME;
            break;
        case EVENT_PLAYBACKMNG_FILE_ABNORMAL:
            stEvent.topic = EVENT_MODEMNG_PLAYBACK_ABNORMAL;
            break;
#endif
#ifdef SERVICES_SPEECH_ON
        case EVENT_SPEECHMNG_STARTREC:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_STARTREC;
            break;
        case EVENT_SPEECHMNG_STOPREC:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_STOPREC;
            break;
        case EVENT_SPEECHMNG_OPENFRONT:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_OPENFRONT;
            break;
        case EVENT_SPEECHMNG_OPENREAR:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_OPENREAR;
            break;
        case EVENT_SPEECHMNG_CLOSESCREEN:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN;
            break;
        case EVENT_SPEECHMNG_OPENSCREEN:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_OPENSCREEN;
            break;
        case EVENT_SPEECHMNG_EMRREC:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_EMRREC;
            break;
        case EVENT_SPEECHMNG_PIV:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_PIV;
            break;
        case EVENT_SPEECHMNG_CLOSEWIFI:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI;
            break;
        case EVENT_SPEECHMNG_OPENWIFI:
            stEvent.topic = EVENT_MODEMNG_SPEECHMNG_OPENWIFI;
            break;
#endif
        default:
            CVI_LOGW("Invalid event[%d]\n", stEvent.topic);
            return -1;
    }
    MODEMNG_UPDATESTATUS(&stEvent, true, false);

    return 0;
}

int32_t MODEMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_REMOVE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_AVAILABLE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_ERROR);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_FSERROR);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_SLOW);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_CHECKING);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_FORMAT);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_FORMATING);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_FORMAT_SUCCESSED);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_FORMAT_FAILED);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_CARD_READ_ONLY);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_RESET);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_MODESWITCH);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_MODEOPEN);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_MODECLOSE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_SETTING);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_START_PIV);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_PLAYBACK_PLAY);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_PLAYBACK_FINISHED);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_PLAYBACK_PROGRESS);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_PLAYBACK_PAUSE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_PLAYBACK_RESUME);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_PLAYBACK_ABNORMAL);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_MODEMNG_SENSOR_STATE);
#ifdef SERVICES_SPEECH_ON
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_STARTREC);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_STOPREC);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENFRONT);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENREAR);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_CLOSESCREEN);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENSCREEN);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_EMRREC);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_PIV);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_CLOSEWIFI);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENWIFI);
#endif
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"MODEMNG_RegisterEvent error");

    return 0;
}

int32_t MODEMNG_LiveViewSwitch(uint32_t viewwin)
{
#ifdef SERVICES_LIVEVIEW_ON
    PARAM_WND_ATTR_S WndParam;
    PARAM_GetWndParam(&WndParam);

    uint32_t enWinds = ((viewwin >> 16) & 0xFFFF);
    uint32_t enSns = viewwin & 0xFFFF;

    int32_t enwndnum = 0;
    for(uint32_t i = 0; i < WndParam.WndCnt && i < MAX_CAMERA_INSTANCES; i++) {
        if(((enWinds >> i) & 0x1) == 1){
            WndParam.Wnds[i].WndEnable = true;
            enwndnum++;
        }else{
            WndParam.Wnds[i].WndEnable = false;
        }
    }

    uint32_t val = 0;

    /* show ALL */
    if(enWinds == enSns){
        for(uint32_t i = 0; i < WndParam.WndCnt && i < MAX_CAMERA_INSTANCES; i++){
            if(((enWinds >> i) & 0x1) == 1){
                WndParam.Wnds[i].SmallWndEnable = true;
                /*         win en   |   small en   */
                val |= ((0x1 << (i * 2 + 1)) | (0x1 << (i * 2)));
                if(enwndnum == 1){
                    WndParam.Wnds[i].SmallWndEnable = false;
                    val &= (~(0x1 << (i * 2)));
                }
            }
        }
    }else{/*show one*/
        for(uint32_t i = 0; i < WndParam.WndCnt && i < MAX_CAMERA_INSTANCES; i++){
            WndParam.Wnds[i].SmallWndEnable = true;
            if(((enWinds >> i) & 0x1) == 1){
                val |= (0x1 << (i * 2 + 1));
                WndParam.Wnds[i].SmallWndEnable = false;
            }
        }
    }
    LIVEVIEWMNG_Switch(val);
    PARAM_SetWndParam(&WndParam);
    PARAM_SetMenuParam(0,  PARAM_MENU_VIEW_WIN_STATUS, viewwin);
#endif
    return 0;
}

int32_t MODEMNG_InitFilemng(void)
{
    int32_t s32Ret = 0;
    STG_DEV_INFO_S stgInfo;
    s32Ret = STORAGEMNG_GetInfo(&stgInfo);
    if(s32Ret != 0){
        CVI_LOGE("STORAGEMNG_GetInfo failed\n");
        return -1;
    }


    int32_t recLoop = 0;
    PARAM_GetRecLoop(&recLoop);
    PARAM_FILEMNG_S FileMng;
    PARAM_GetFileMngParam(&FileMng);

    FileMng.FileMng.comm_param.recloop_en = recLoop;
    FILEMNG_Init(&FileMng.FileMng);
    if (0 != s32Ret) {
        CVI_LOGE("FileMng init failed\n");
        return -1;
    }

    if (FILEMNG_GetStorageFormated() == 0) {
        s32Ret = FILEMNG_CreateStorageFormatedFlag();
        if (s32Ret != 0) {
            EVENT_S stEvent = {0};
            if (MODEMNG_SetCardState(CARD_STATE_UNAVAILABLE))
                stEvent.topic = EVENT_MODEMNG_CARD_UNAVAILABLE;
            EVENTHUB_Publish(&stEvent);
            CVI_LOGE("Read SD_Config_File failed Need foramt\n");
            return -1;
        }
    }
    FILEMNG_SetStorageStatus(FILEMNG_STORAGE_STATE_AVAILABLE);
    return 0;
}

int32_t MODEMNG_DeInitFilemng(void)
{
    int32_t s32Ret = 0;
    s32Ret = FILEMNG_Deinit();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"MODEMNG_DeintStorage !");
    return 0;
}

int32_t MODEMNG_InitStorage(void)
{
    int32_t s32Ret = 0;
    STG_DEVINFO_S stg_attr;
    PARAM_GetStgInfoParam(&stg_attr);
    s32Ret = STORAGEMNG_Create(&stg_attr);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"STORAGEMNG_Create !");

    return 0;
}

int32_t MODEMNG_DeintStorage(void)
{
    int32_t s32Ret = 0;
    s32Ret = STORAGEMNG_Destroy();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"MODEMNG_DeintStorage !");
    return 0;
}

int32_t MODEMNG_Format(char *labelname)
{
    int32_t s32Ret = 0;

    mmc_term();

    FILEMNG_SetStorageStatus(false);

    s32Ret = STORAGEMNG_Format(labelname);
    if(s32Ret != 0) {
        s32Ret = STORAGEMNG_Mount();
        return -1;

    } else {
        s32Ret = STORAGEMNG_Mount();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "STORAGEMNG_Mount");
        MODEMNG_SetCardState(CARD_STATE_FORMATED);
        s32Ret = FILEMNG_CreateStorageFormatedFlag();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "FILEMNG_CreateStorageFormatedFlag");
        FILEMNG_SetStorageStatus(true);
        return 0;
    }
}


/** init global context structure */
int32_t MODEMNG_ContextInit(const MODEMNG_CONFIG_S* pstModemngCfg)
{
    /** init context structure */
    memcpy(&s_stModeMng.stModemngCfg, pstModemngCfg, sizeof(MODEMNG_CONFIG_S));
    return 0;
}

/** publish message process result event */
int32_t MODEMNG_PublishEvent(MESSAGE_S* pstMsg, int32_t s32Result)
{
    int32_t s32Ret = 0;

    /** check parameter */
    MODEMNG_CHECK_POINTER(pstMsg, MODE_ENULLPTR, "pstMsg error");

    EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(EVENT_S));
    memcpy(&stEvent, pstMsg, sizeof(EVENT_S));
    stEvent.s32Result = s32Result;

    CVI_LOGD("event(%x), s32Result(0x%x)\n\n", stEvent.topic, (uint32_t)s32Result);
    s32Ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINTER,"Publish event !");

    return s32Ret;
}

int32_t MODEMNG_SetSensorState(uint32_t state)
{
    g_is_mode_proc_with_sensor = state;
    return 0;
}

int32_t MODEMNG_GetSensorState(uint32_t *state)
{
    *state = g_is_mode_proc_with_sensor;
    return 0;
}
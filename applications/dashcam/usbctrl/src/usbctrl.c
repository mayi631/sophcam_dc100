#include <pthread.h>

#include "usbctrl.h"
#include "media_init.h"
#include "sysutils_eventhub.h"
#include "appcomm.h"
#include "mode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/** usbcontrol context */
typedef struct USBCTRL_CONTEXT_S
{
    MW_PTR pSubscriber;
    pthread_mutex_t EventMutex;
    pthread_cond_t  EventCond;
    int32_t  s32WaitedValue; /* Waited Value */
    bool pause;
    bool waitedFlag;
    USB_MODE_E prevMode;
    pthread_mutex_t pauseMutex;
} APP_USBCTRL_RegisterEvent;
static APP_USBCTRL_RegisterEvent s_stUSBCTRLCtx;

static inline const char* USBCTRL_GetEventStr(int32_t  enEvent)
{
    if (EVENT_USB_OUT == enEvent) {
        return "USB Out Event";
    } else if (EVENT_USB_INSERT == enEvent) {
        return "USB Insert Event";
    } else if (EVENT_USB_UVC_READY == enEvent) {
        return "UVC Ready Event";
    } else if (EVENT_USB_STORAGE_READY == enEvent) {
        return "USB Storage Ready Event";
    } else if (enEvent == EVENT_USB_HOSTUVC_READY) {
        return "HOST UVC Ready Event";
    }  else if (enEvent == EVENT_USB_HOSTUVC_PC) {
        return "HOST UVC Source pc Event";
    } else if (enEvent == EVENT_USB_HOSTUVC_HEAD) {
        return "HOST UVC Source head Event";
    } else {
        return "Unknown Usb Event";
    }
}

static int32_t  USBCTRL_SubscribeEventProc(void *argv, EVENT_S *pstEvent)
{
    APPCOMM_CHECK_POINTER(pstEvent, -1);

    MUTEX_LOCK(s_stUSBCTRLCtx.pauseMutex);
    if (pstEvent->arg2 == s_stUSBCTRLCtx.s32WaitedValue){
        s_stUSBCTRLCtx.waitedFlag = false;
    }
    MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);

    MUTEX_LOCK(s_stUSBCTRLCtx.EventMutex);
    if (pstEvent->arg2 == s_stUSBCTRLCtx.s32WaitedValue){
        pthread_cond_signal(&s_stUSBCTRLCtx.EventCond);
        s_stUSBCTRLCtx.s32WaitedValue = -1;
    }
    MUTEX_UNLOCK(s_stUSBCTRLCtx.EventMutex);
    return 0;
}

static int32_t  USBCTRL_SubscribeEvent(APP_USBCTRL_RegisterEvent* pstCtx)
{
    EVENTHUB_SUBSCRIBER_S stSubscriber = {"USBCtrl", NULL, USBCTRL_SubscribeEventProc, true};
    EVENTHUB_CreateSubscriber(&stSubscriber, &pstCtx->pSubscriber);
    CVI_LOGD("subscribe create success[%p]\n", pstCtx->pSubscriber);
    return 0;
}

static int32_t  USBCTRL_EventProc(const USB_EVENT_INFO_S *pstEventInfo)
{
    EVENT_S stEvent;
    switch (pstEventInfo->s32EventId)
    {
        case EVENT_USB_OUT:
        {
            CVI_LOGI("%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = EVENT_USB_OUT;
            EVENTHUB_Publish(&stEvent);
            break;
        }
        case EVENT_USB_INSERT:
        {
            CVI_LOGI("%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = EVENT_USB_INSERT;
            EVENTHUB_Publish(&stEvent);
            break;
        }
        case EVENT_USB_PC_INSERT:
        {
            CVI_LOGI("%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = EVENT_USB_PC_INSERT;
            EVENTHUB_Publish(&stEvent);
            break;
        }
        case EVENT_USB_UVC_READY:{
            CVI_LOGI("s_stUSBCTRLCtx.pause=%d\n", s_stUSBCTRLCtx.pause);
            MUTEX_LOCK(s_stUSBCTRLCtx.pauseMutex);
            if(s_stUSBCTRLCtx.pause == false) {
                s_stUSBCTRLCtx.waitedFlag = true;
                MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);

                CVI_LOGI("s32EventId=%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
                stEvent.topic = EVENT_USB_UVC_READY;

                CVI_LOGD("Wait uvc workmode event...\n");
                MUTEX_LOCK(s_stUSBCTRLCtx.EventMutex);
                EVENTHUB_Publish(&stEvent);

                s_stUSBCTRLCtx.s32WaitedValue = 0;

                //pthread_cond_wait(&s_stUSBCTRLCtx.EventCond, &s_stUSBCTRLCtx.EventMutex);
                MUTEX_UNLOCK(s_stUSBCTRLCtx.EventMutex);
                CVI_LOGD("Wait OK\n");
            } else {
                CVI_LOGW("ignore UVC_READY proc\n");
                MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);
            }
            break;
        }

        case EVENT_USB_STORAGE_READY:{
            CVI_LOGI("s_stUSBCTRLCtx.pause=%d\n", s_stUSBCTRLCtx.pause);
            MUTEX_LOCK(s_stUSBCTRLCtx.pauseMutex);
            if(s_stUSBCTRLCtx.pause == false) {
                CVI_LOGI("s32EventId=%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
                s_stUSBCTRLCtx.waitedFlag = true;
                MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);

                stEvent.topic = EVENT_USB_STORAGE_READY;

                CVI_LOGI("Wait usb storage workmode event...\n");
                MUTEX_LOCK(s_stUSBCTRLCtx.EventMutex);
                EVENTHUB_Publish(&stEvent);

                s_stUSBCTRLCtx.s32WaitedValue = 1;
                pthread_cond_wait(&s_stUSBCTRLCtx.EventCond, &s_stUSBCTRLCtx.EventMutex);
                MUTEX_UNLOCK(s_stUSBCTRLCtx.EventMutex);
                CVI_LOGI("Wait OK\n");
            } else {
                CVI_LOGW("ignore STORAGE_READY proc\n");
                MUTEX_UNLOCK(s_stUSBCTRLCtx.pauseMutex);
            }
            break;
        }

        case EVENT_USB_HOSTUVC_READY: {
            CVI_LOGI("%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = EVENT_USB_HOSTUVC_READY;
            EVENTHUB_Publish(&stEvent);
            break;
        }

        case EVENT_USB_HOSTUVC_PC:
        {
            CVI_LOGI("%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = EVENT_USB_HOSTUVC_PC;
            EVENTHUB_Publish(&stEvent);
            break;
        }

        case EVENT_USB_HOSTUVC_HEAD:
        {
            CVI_LOGI("%s\n", USBCTRL_GetEventStr(pstEventInfo->s32EventId));
            stEvent.topic = EVENT_USB_HOSTUVC_HEAD;
            EVENTHUB_Publish(&stEvent);
            break;
        }
        default:
            CVI_LOGW("Invalid Event[%08x]\n", pstEventInfo->s32EventId);
            return -1;
    }

    return 0;
}

static int32_t  USBCTRL_GetStorageState(void* pvPrivData)
{
    // if(NULL == pvPrivData)
    // {
    //     return -1;
    // }

    // int32_t  ret = 0;
    // bool* pbStorageReady = (bool*)pvPrivData;
    // *pbStorageReady = false;

    // CVI_STORAGEMNG_CFG_S stStorageMngCfg;
    // ret = HI_PDT_PARAM_GetStorageCfg(&stStorageMngCfg);
    // HI_APPCOMM_CHECK_RETURN(ret, -1);

    // /** check sd state */
    // CVI_STORAGE_STATE_E enState = CVI_STORAGE_STATE_IDEL;
    // ret = CVI_STORAGEMNG_GetState(stStorageMngCfg.szMntPath, &enState);
    // HI_APPCOMM_CHECK_RETURN(ret, -1);
    // CVI_LOGI("storage state(%d)\n", enState);

    // if(CVI_STORAGE_STATE_MOUNTED == enState)
    // {
    //     *pbStorageReady = true;
    // }

    return 0;
}

int32_t  USBCTRL_Init(void)
{
    int32_t  ret = 0;
    USB_CFG_S stUsbCfg;
    PARAM_USB_MODE_S UsbModeParam = {0};
    UVC_CFG_S UvcCfg = {0};
    stUsbCfg.pfnEventProc = USBCTRL_EventProc;
    stUsbCfg.pfnGetStorageState = USBCTRL_GetStorageState;

    PARAM_GetUsbParam(&UsbModeParam);

    /* usb storage configure */
    memcpy(&stUsbCfg.stStorageCfg, &UsbModeParam.StorageCfg, sizeof(USB_STORAGE_CFG_S));

    /* usb uvc configure */
    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
    PARAM_MEDIA_SPEC_S params = {0};
    PARAM_GetMediaMode(UsbModeParam.UvcParam.VcapId, &params);
    /* get vprochdl venchdl */
    for (int32_t  z = 0; z < MAX_VPROC_CNT; z++) {
        if ((MediaParams->SysHandle.vproc[z] != NULL) &&
            (UsbModeParam.UvcParam.VprocId == (uint32_t)MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
            UvcCfg.stDataSource.VprocHdl = MediaParams->SysHandle.vproc[z];
            break;
        }
    }
    UvcCfg.stDataSource.VprocChnId = UsbModeParam.UvcParam.VprocChnId;
    memcpy(&UvcCfg.attr, &UsbModeParam.UvcParam.UvcCfg, sizeof(UVC_CFG_ATTR_S));
    memcpy(&stUsbCfg.stUvcCfg, &UvcCfg, sizeof(UVC_CFG_S));

    /* subscribe event */
    USBCTRL_SubscribeEvent(&s_stUSBCTRLCtx);
    MUTEX_INIT_LOCK(s_stUSBCTRLCtx.EventMutex);
    COND_INIT(s_stUSBCTRLCtx.EventCond);
    MUTEX_INIT_LOCK(s_stUSBCTRLCtx.pauseMutex);

    ret = USB_Init(&stUsbCfg);
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(ret, ret,"InitUsb");

    USB_MODE_E enUsbMode;
    enUsbMode = USB_MODE_CHARGE;

    ret = USB_SetMode(enUsbMode);
    s_stUSBCTRLCtx.prevMode = enUsbMode;
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(ret, ret,"SetUsbMode");

    return 0;
}

int32_t  USBCTRL_Deinit(void)
{
    return USB_Deinit();
}

int32_t  USBCTRL_RegisterEvent(void)
{
    int32_t  ret = 0;
    ret  = EVENTHUB_RegisterTopic(EVENT_USB_OUT);
    ret |= EVENTHUB_RegisterTopic(EVENT_USB_INSERT);
    ret |= EVENTHUB_RegisterTopic(EVENT_USB_UVC_READY);
    ret |= EVENTHUB_RegisterTopic(EVENT_USB_STORAGE_READY);
    ret |= EVENTHUB_RegisterTopic(EVENT_USB_HOSTUVC_READY);
    ret |= EVENTHUB_RegisterTopic(EVENT_USB_HOSTUVC_PC);
    ret |= EVENTHUB_RegisterTopic(EVENT_USB_HOSTUVC_HEAD);
    APPCOMM_CHECK_RETURN(ret, -1);
    CVI_LOGD("Success\n");
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
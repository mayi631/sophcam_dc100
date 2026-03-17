#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mode.h"
#include "media_init.h"
#include "modeinner.h"
#include "usb_storage.h"

//storage MODE
int32_t MODE_OpenStorageMode(void)
{
    // int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    // s32Ret = MEDIA_VbInit();
    // MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Vb init");

    //screen init if not screen not init
    // s32Ret = MEDIA_DispInit(true);
    // MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Disp init");

    USB_STORAGE_CFG_S StpstCfg = {0};
    USB_GetStorageCfg(&StpstCfg);

    if (USB_STORAGE_Init(StpstCfg.szDevPath) != 0) {
        printf("USB_STORAGE_Init Failed !");
        return -1;
    }

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_STORAGE;
    EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = WORK_MODE_STORAGE;

    return 0;
}

int32_t MODE_CloseStorageMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;

    s32Ret = USB_STORAGE_Deinit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "USB_STORAGE_Deinit fail");

    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_VideoDeInit fail");

    // s32Ret = MEDIA_DispDeInit();
    // MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_DispDeInit fail");

    // s32Ret = MEDIA_VbDeInit();
    // MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_VbDeInit fail");

    return 0;
}

int32_t MODEMNG_StorageModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    // int32_t s32Ret = 0;
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    STATE_S* pstStateAttr = (STATE_S*)pvArg;
    CVI_LOGI("MODEMNG_StorageModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));
    (void)pstStateAttr;
    switch (pstMsg->topic) {
    case EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV: {
        USB_STORAGE_CFG_S StpstCfg = { 0 };
        USB_GetStorageCfg(&StpstCfg);
        if (USB_STORAGE_PrepareDev(&StpstCfg) != 0) {
            CVI_LOGE("USB_STORAGE_PrepareDev");
        }
        return PROCESS_MSG_RESULTE_OK;
        }
        default:
            return PROCESS_MSG_UNHANDLER;
            break;
        }

    return PROCESS_MSG_RESULTE_OK;
}

int32_t MODEMNG_StorageStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stStorageState = {
        WORK_MODE_STORAGE,
        MODEEMNG_STATE_USB_STORAGE,
        MODE_OpenStorageMode,
        MODE_CloseStorageMode,
        MODEMNG_StorageModeMsgProc,
        NULL
    };
    stStorageState.argv = &stStorageState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
        &stStorageState,
        (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t MODEMNG_StorageStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODE_CloseStorageMode();
    return s32Ret;
}

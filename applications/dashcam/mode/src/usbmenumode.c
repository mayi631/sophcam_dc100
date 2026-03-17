#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mapi.h"
#include "mode.h"
#include "media_init.h"
#include "modeinner.h"
#include "ui_common.h"


//#error "Unimplemented"

//UsbMenu MODE
int32_t MODE_OpenUsbMenuMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    CVI_LOGD("MODE_OpenUsbMenuMode\n");

    //screen init if not screen not init
    s32Ret = MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Disp init");

    s32Ret = UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"UIAPP_Start");

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_USBMENU;
    EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = WORK_MODE_USBMENU;

    return 0;
}

int32_t MODE_CloseUsbMenuMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;

    s32Ret = MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_DispDeInit fail");

    return 0;
}


int32_t MODEMNG_UsbMenuModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    //int32_t s32Ret = 0;
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    STATE_S* pstStateAttr = (STATE_S*)pvArg;
    CVI_LOGI("MODEMNG_UsbMenuModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));
    (void)pstStateAttr;

    return PROCESS_MSG_RESULTE_OK;
}

int32_t MODEMNG_UsbMenuStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stUsbMenuState =
    {
        WORK_MODE_USBMENU,
        MODEEMNG_STATE_USBMENU,
        MODE_OpenUsbMenuMode,
        MODE_CloseUsbMenuMode,
        MODEMNG_UsbMenuModeMsgProc,
        NULL
    };
    stUsbMenuState.argv = &stUsbMenuState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stUsbMenuState,
                              (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add UsbMenu state");
    CVI_LOGD("MODEMNG_UsbMenuStatesInit\n");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t MODEMNG_UsbMenuStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODE_CloseUsbMenuMode();
    return s32Ret;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mapi.h"
#include "mode.h"
#include "media_init.h"
#include "usbctrl.h"
#include "modeinner.h"
#include "appuvc.h"

static int32_t MODE_SetUvcCfg(void)
{
    int32_t z = 0;
    int32_t ret = 0;
    PARAM_USB_MODE_S UsbModeParam = {0};
    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
    PARAM_MEDIA_SPEC_S params = {0};
    UVC_CFG_S UvcCfg = {0};

    PARAM_GetUsbParam(&UsbModeParam);
    PARAM_GetMediaMode(UsbModeParam.UvcParam.VcapId, &params);

    memcpy(&UvcCfg.attr, &UsbModeParam.UvcParam.UvcCfg, sizeof(UVC_CFG_ATTR_S));
    /* get vprochdl */
    for (z = 0; z < MAX_VPROC_CNT; z++) {
        if ((MediaParams->SysHandle.vproc[z] != NULL) &&
            (UsbModeParam.UvcParam.VprocId == (uint32_t)MAPI_VPROC_GetGrp(MediaParams->SysHandle.vproc[z]))) {
            UvcCfg.stDataSource.VprocHdl = MediaParams->SysHandle.vproc[z];
            break;
        }
    }
    UvcCfg.stDataSource.VprocChnId = UsbModeParam.UvcParam.VprocChnId;
    /* set usb uvc configure */

    ret = USB_SetUvcCfg(&UvcCfg);
    if(ret != 0) {
        CVI_LOGE("USB_SetUvcCfg faile !\n");
        return -1;
    }

    return 0;
}

//UVC MODE
int32_t MODEMNG_OpenUvcMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    s32Ret = MEDIA_VbInit();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Vb init");

    s32Ret = MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Video init");

    //screen init if not screen not init
    s32Ret = MEDIA_DispInit(true);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Disp init");

    s32Ret = MODE_SetUvcCfg();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MODE_SetUvcCfg fail");

    USB_CFG_S stUsbCfg = {0};
    USB_GetUvcCfg(&stUsbCfg);

    if (UVC_Init(&stUsbCfg.stUvcCfg.attr.stDevCap, &stUsbCfg.stUvcCfg.stDataSource, &stUsbCfg.stUvcCfg.attr.stBufferCfg) != 0) {
        printf("UVC_Init Failed !");
        return -1;
    }

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_UVC;
    EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = WORK_MODE_UVC;

    return 0;
}

int32_t MODEMNG_CloseUvcMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;

    s32Ret = UVC_Deinit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UVC_Deinit fail");

    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_VideoDeInit fail");

    s32Ret = MEDIA_DispDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_DispDeInit fail");

    s32Ret = MEDIA_VbDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MEDIA_VbDeInit fail");

    return 0;
}

int32_t MODEMNG_UvceModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
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
    CVI_LOGI("MODEMNG_UvceModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));
    (void)pstStateAttr;
    switch (pstMsg->topic) {
    case EVENT_MODEMNG_UVC_MODE_START: {
        USB_CFG_S stUsbCfg = { 0 };
        USB_GetUvcCfg(&stUsbCfg);
        char uvc_devname[32] = "/dev/video0";
        if (UVC_Start(stUsbCfg.stUvcCfg.attr.szDevPath) != 0) {
            CVI_LOGE("UVC_Start Failed, patch = %s, try open = %s!", stUsbCfg.stUvcCfg.attr.szDevPath, uvc_devname);
            if (UVC_Start(uvc_devname) != 0) {
                CVI_LOGE("UVC_Start Failed, patch = %s!", uvc_devname);
            }
        }
        return PROCESS_MSG_RESULTE_OK;
        }
        case EVENT_MODEMNG_UVC_MODE_STOP:
        {
            if (UVC_Stop() != 0) {
                CVI_LOGE("UVC_Stop Failed !");
            }
            return PROCESS_MSG_RESULTE_OK;
        }
        default:
            return PROCESS_MSG_UNHANDLER;
            break;
        }

    return PROCESS_MSG_RESULTE_OK;
}

int32_t MODEMNG_UvcStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stUvcState =
    {
        WORK_MODE_UVC,
        MODEEMNG_STATE_UVC,
        MODEMNG_OpenUvcMode,
        MODEMNG_CloseUvcMode,
        MODEMNG_UvceModeMsgProc,
        NULL
    };
    stUvcState.argv = &stUvcState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stUvcState,
                              (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t MODEMNG_UvcStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODEMNG_CloseUvcMode();
    return s32Ret;
}

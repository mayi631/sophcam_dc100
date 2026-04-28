#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mode.h"
#include "media_init.h"
#include "media_disp.h"
#include "modeinner.h"
#include "ui_common.h"

//BootFirst MODE
int32_t MODE_OpenBootFirstMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = WORK_MODE_BOOT;

    CVI_LOGD("MODE_OpenBootFirstMode\n");

    /* 仅回主界面时清屏：避免预览最后一帧残留；拍照/录像互切不在 Close* 里清，否则会闪黑 */
    {
        MEDIA_SYSHANDLE_S *sysh = &MEDIA_GetCtx()->SysHandle;
        if (sysh->dispHdl != NULL) {
            s32Ret = MEDIA_DISP_ClearBuf();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "DISP clear");
        }
    }

    s32Ret = UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"UIAPP_Start");

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_BOOT;
    EVENTHUB_Publish(&stEvent);



    return 0;
}

int32_t MODE_CloseBootFirstMode(void)
{
    int32_t s32Ret = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = WORK_MODE_BOOT;
    s32Ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Publish");

    return 0;
}

static int32_t MODEMNG_SetDefaultParamFromBin(void)
{
    int32_t ret;
    PARAM_CFG_S param;
    PARAM_MEDIA_SNS_ATTR_S sns_attr = { 0 };
    PARAM_MEDIA_VACP_ATTR_S vcap_attr = { 0 };

    // load default pararm
    ret = PARAM_LoadFromDefBin(&param);
    MODEMNG_CHECK_RET(ret, MODE_EINVAL, "reset param");

    // save param
    PARAM_SetParam(&param);
    PARAM_SaveParam();

    // reinit sensor to sync hardware state with default param
    PARAM_GetSensorParam(0, &sns_attr);
    PARAM_GetVcapParam(0, &vcap_attr);
    MEDIA_SensorSwitchInit(&sns_attr, &vcap_attr);

    CVI_LOGI("reset param finish, will use front sensor");

    return 0;
}

static void MODEMNG_SetOsdTime(int32_t value)
{
    uint32_t i = 0, z = 0;

    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++) {
            for (int32_t k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                for(z = 0; z < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].u32DispNum; z++){
                    if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.enType == MAPI_OSD_TYPE_TIME){
                        pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].astDispAttr[z].bShow = value;
                    }
                }
            }
        }
    }
    PARAM_SetMenuParam(0, PARAM_MENU_OSD_STATUS, value);
}

static int32_t MODEMNG_BootStatesSettingMsgProc(MESSAGE_S* pstMsg)
{
    int32_t s32Ret = 0;

    switch (pstMsg->arg1)
    {
        case PARAM_MENU_LIGHT_FREQUENCE:
            PARAM_SetMenuParam(0, PARAM_MENU_LIGHT_FREQUENCE, pstMsg->arg2);
            break;

        case PARAM_MENU_POWER_OFF:
            PARAM_SetMenuParam(0, PARAM_MENU_POWER_OFF, pstMsg->arg2);
            break;

        case PARAM_MENU_ACTION_AUDIO:
            PARAM_SetMenuParam(0, PARAM_MENU_ACTION_AUDIO, pstMsg->arg2);
            break;

        case PARAM_MENU_LANGUAGE:
            PARAM_SetMenuParam(0, PARAM_MENU_LANGUAGE, pstMsg->arg2);
            break;

        case PARAM_MENU_AUTO_SCREEN_OFF:
            PARAM_SetMenuParam(0, PARAM_MENU_AUTO_SCREEN_OFF, pstMsg->arg2);
            break;

        case PARAM_MENU_AO_VOLUME:
            PARAM_SetMenuParam(0, PARAM_MENU_AO_VOLUME, pstMsg->arg2);
            break;

        case PARAM_MENU_STATUS_LIGHT:
            PARAM_SetMenuParam(0, PARAM_MENU_STATUS_LIGHT, pstMsg->arg2);
            break;

        case PARAM_MENU_BRIGHTNESS:
            PARAM_SetMenuParam(0, PARAM_MENU_BRIGHTNESS, pstMsg->arg2);
            break;

        case PARAM_MENU_DEFAULT:
            s32Ret = MODEMNG_SetDefaultParamFromBin();
            MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MODEMNG_SetDefaultParamFromBin fail");
            break;

        case PARAM_MENU_OSD_STATUS:
            MODEMNG_SetOsdTime(pstMsg->arg2);
            break;

        default:
            CVI_LOGE("not support param type(%d)\n\n", pstMsg->arg1);
            return -1;
    }

    return s32Ret;
}

int32_t MODEMNG_BootFirstModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return PROCESS_MSG_RESULTE_OK;
    }

    /** check parameters */
    MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

    STATE_S* pstStateAttr = (STATE_S*)pvArg;
    CVI_LOGI("MODEMNG_BootFirstModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));
    (void)pstStateAttr;

    switch (pstMsg->topic) {
    case EVENT_MODEMNG_SETTING: {
        MODEMNG_BootStatesSettingMsgProc(pstMsg);
        return PROCESS_MSG_RESULTE_OK;
            }
        default:
        return PROCESS_MSG_UNHANDLER;
        break;
        }

    return PROCESS_MSG_RESULTE_OK;
}

int32_t MODEMNG_BootFirstStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stBootFirstState =
    {
        WORK_MODE_BOOT,
        MODEEMNG_STATE_BOOTFIRST,
        MODE_OpenBootFirstMode,
        MODE_CloseBootFirstMode,
        MODEMNG_BootFirstModeMsgProc,
        NULL
    };
    stBootFirstState.argv = &stBootFirstState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stBootFirstState,
                              (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add BootFirst state");
    CVI_LOGD("MODEMNG_BootFirstStatesInit\n");

    return s32Ret;
}

/** deinit Uvc mode */
int32_t MODEMNG_BootFirstStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODE_CloseBootFirstMode();
    return s32Ret;
}

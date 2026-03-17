#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "modeinner.h"
#include "upgrade.h"
#include "param.h"

int32_t MODEMNG_OtaUpFile(MESSAGE_S* pstMsg)
{
    char upmode[64] = {0};
	char upver[64] = {0};
    char uppath[64] = {0};
    char tmppath[64] = {0};
    int32_t result = 0;
    UPGRADE_DEV_INFO_S tDev;
    char str[128];
    memset(str, '\0', 128);
    memcpy(str, pstMsg->aszPayload, 128);

	// Read from define, or change to read from file-system
	memcpy(upmode, (pstMsg->aszPayload), (strrchr((str), '_') - (str)));
	memcpy(upver, (strrchr((str), '_') + 1), 64);
	snprintf(tDev.szSoftVersion, COMM_STR_LEN, "%s", upver);
	snprintf(tDev.szModel, COMM_STR_LEN, "%s", upmode);

    UPGRADE_Init();
    STG_DEVINFO_S SDParam = {0};
    PARAM_GetStgInfoParam(&SDParam);
    strcpy(tmppath, SDParam.aszMntPath);
    strcat(tmppath, "upgrade_%s.bin");
    snprintf(uppath, COMM_STR_LEN, tmppath, upver);
    if (UPGRADE_CheckPkg(uppath, &tDev, false) == APP_SUCCESS) {
		printf("There is a new package!!\n");
		if(pstMsg->arg2 == 1){
            CVI_LOGD("enter SD_updata");
            char mount_path[64] = {0};
            PARAM_FILEMNG_S FileMng;
            PARAM_GetFileMngParam(&FileMng);
            strcpy(mount_path, FileMng.FileMng.comm_param.storage_mount_point);
            result = UPGRADE_DoUpgradeViaSD(uppath, mount_path);
        }else{
            result = UPGRADE_DoUpgrade(uppath);
        }
	} else {
        result = -1;
    }

    if (NULL != uppath) {
        char rmpath[72] = {0};
        snprintf(rmpath, 72, "rm -rf %s", uppath);
        system(rmpath);
    }

    return result;
}

int32_t MODEMNG_OpenUpDateMode(void)
{
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_UPDATE;
    EVENTHUB_Publish(&stEvent);

    pstModeMngCtx->CurWorkMode = WORK_MODE_UPDATE;

    //set open amplifiler
#ifdef SERVICES_LIVEVIEW_ON
    VOICEPLAY_SetAmplifier(false);
#endif

    return 0;
}

int32_t MODEMNG_CloseUpDateMode(void)
{
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;


    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = WORK_MODE_UPDATE;
    EVENTHUB_Publish(&stEvent);

    //set close amplifiler
#ifdef SERVICES_LIVEVIEW_ON
    VOICEPLAY_SetAmplifier(true);
#endif

    return 0;
}

int32_t MODEMNG_UpDateModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
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
    CVI_LOGI("MODEMNG_UpDateModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));
    (void)pstStateAttr;
    switch (pstMsg->topic) {
    case EVENT_MODEMNG_START_UPFILE: {
        int32_t ret = 0;
        EVENT_S stEvent = { 0 };
        ret = MODEMNG_OtaUpFile(pstMsg);
        if (1 == ret) {
            stEvent.topic = EVENT_MODEMNG_UPFILE_FAIL;
        } else if (0 == ret) {
            stEvent.topic = EVENT_MODEMNG_UPFILE_SUCCESSED;
        } else {
            stEvent.topic = EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR;
        }
        MODEMNG_UPDATESTATUS(&stEvent, true, false);
        sleep(3);
        if (0 == ret) {
            system("reboot -f");
        }

        return PROCESS_MSG_RESULTE_OK;
        }
        default:
            return PROCESS_MSG_UNHANDLER;
        }

    return PROCESS_MSG_RESULTE_OK;
}

int32_t MODEMNG_UpDateStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stUpdateState =
    {
        WORK_MODE_UPDATE,
        MODEEMNG_STATE_UPDATE,
        MODEMNG_OpenUpDateMode,
        MODEMNG_CloseUpDateMode,
        MODEMNG_UpDateModeMsgProc,
        NULL
    };
    stUpdateState.argv = &stUpdateState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stUpdateState,
                              (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit update mode */
int32_t MODEMNG_UpDateStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODEMNG_CloseUpDateMode();
    return s32Ret;
}

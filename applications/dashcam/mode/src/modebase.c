#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "modeinner.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "volmng.h"
#endif
#include "usbctrl.h"
#include "timedtask.h"
#ifdef SCREEN_ON
#include "hal_screen_comp.h"
#endif
#ifdef SERVICES_SPEECH_ON
#include "speechmng.h"
#endif

#ifdef SERVICES_SPEECH_ON
int32_t MODEMNG_StartEventSpeech()
{
	int32_t s32Ret = 0;
	s32Ret = SPEECHMNG_StartSpeech();
	return s32Ret;
}

int32_t MODEMNG_StopEventSpeech()
{
	int32_t s32Ret = 0;
	s32Ret = SPEECHMNG_StopSpeech();
	return s32Ret;
}
#endif

static int32_t MODEMNG_BaseModeProcessMessage(MESSAGE_S *pstMsg, void *pvArg, uint32_t *pStateID)
{
	int32_t s32Ret = 0;
	MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();

	if (pstModeMngCtx->bSysPowerOff == true) {
            CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
            return PROCESS_MSG_RESULTE_OK;
        }
        CVI_LOGI("MODEMNG_BaseModeProcessMessage:%d\n", event_topic_get_name(pstMsg->topic));
        /** check parameters */
        MODEMNG_CHECK_MSGPROC_FUNC_PARAM(pvArg, pStateID, pstMsg, pstModeMngCtx->bInProgress);

        STATE_S* pstStateAttr = (STATE_S*)pvArg;
        // CVI_LOGD("camid %d base mode(%s)\n\n", camid, pstStateAttr->name);
        // CVI_LOGD("camid %d will process message topic(%x) \n\n", camid, pstMsg->topic);
        (void)pstStateAttr;
        switch (pstMsg->topic) {
	case EVENT_MODEMNG_MODESWITCH: {
		if (pstMsg->arg1 < 0 || pstMsg->arg1 > WORK_MODE_BUTT) {
			CVI_LOGE("Switch mode is illegal! modeid(%d)\n", pstMsg->arg1);
		}
		*pStateID = pstMsg->arg1;
		/*if (WORK_MODE_LAPSE == pstMsg->arg1) {
			MODEMNG_SetMediaLapseTime(pstMsg->arg2);
		}*/
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_MODEMNG_POWEROFF: {
		/*TODO*/
		CVI_LOGI("try to process message (%x) to poweroff device\n\n", pstMsg->topic);
		pstModeMngCtx->bSysPowerOff = true;
		pstModeMngCtx->stModemngCfg.pfnExitCB(MODEMNG_EXIT_MODE_POWEROFF);
		/** no need publish result event and reset g_bModeMngInProgress to false */
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_MODEMNG_SCREEN_DORMANT: {
#ifdef SCREEN_ON
		if (pstMsg->arg1 == true) {
			HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_OFF);
		} else {
			HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDXS_0, HAL_SCREEN_STATE_ON);
		}
#endif
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_MODEMNG_CARD_FORMAT: {
		EVENT_S stEvent = {0};
		stEvent.topic = EVENT_MODEMNG_CARD_FORMATING;
		EVENTHUB_Publish(&stEvent);
		s32Ret = MODEMNG_Format((char *)pstMsg->aszPayload);
		if (s32Ret != 0) {
			CVI_LOGE("MODEMNG_Format Faild!, s32Ret: %d\n", s32Ret);
			MODEMNG_SetCardState(CARD_STATE_UNAVAILABLE);
			stEvent.topic = EVENT_MODEMNG_CARD_FORMAT_FAILED;
			EVENTHUB_Publish(&stEvent);
		} else {
			CVI_LOGD("MODEMNG_Format Succes!\n");
			MODEMNG_SetCardState(CARD_STATE_FORMATED);
			MODEMNG_InitFilemng();
			stEvent.topic = EVENT_MODEMNG_CARD_FORMAT_SUCCESSED;
			EVENTHUB_Publish(&stEvent);
		}
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_MODEMNG_SWITCH_LIVEVIEW: {
		MODEMNG_LiveViewSwitch(pstMsg->arg1);
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_STORAGEMNG_DEV_UNPLUGED: {
		if (MODEMNG_SetCardState(CARD_STATE_REMOVE)) {
			MODEMNG_DeInitFilemng();
			MODEMNG_MonitorStatusNotify(pstMsg);
			FILEMNG_SetStorageStatus(false);
			return PROCESS_MSG_RESULTE_OK;
		}
	} break;
	case EVENT_STORAGEMNG_DEV_CONNECTING: {
		if (MODEMNG_SetCardState(CARD_STATE_CHECKING))
			MODEMNG_MonitorStatusNotify(pstMsg);
		return PROCESS_MSG_RESULTE_OK;
	} break;
	case EVENT_STORAGEMNG_DEV_ERROR: {
		if (MODEMNG_SetCardState(CARD_STATE_ERROR))
			MODEMNG_MonitorStatusNotify(pstMsg);
		return PROCESS_MSG_RESULTE_OK;
	} break;
	case EVENT_STORAGEMNG_MOUNT_FAILED: {
		if (MODEMNG_SetCardState(CARD_STATE_MOUNT_FAILED))
			MODEMNG_MonitorStatusNotify(pstMsg);
		return PROCESS_MSG_RESULTE_OK;
	} break;
	case EVENT_STORAGEMNG_MOUNT_READ_ONLY: {
		if (MODEMNG_SetCardState(CARD_STATE_READ_ONLY))
			MODEMNG_MonitorStatusNotify(pstMsg);

		return PROCESS_MSG_RESULTE_OK;
	} break;
	case EVENT_STORAGEMNG_FS_CHECKING: {
		if (MODEMNG_SetCardState(CARD_STATE_CHECKING))
			MODEMNG_MonitorStatusNotify(pstMsg);

		return PROCESS_MSG_RESULTE_OK;
	} break;
	case EVENT_STORAGEMNG_MOUNTED: {
		MODEMNG_InitFilemng();
		return PROCESS_MSG_RESULTE_OK;
	} break;
	case EVENT_STORAGEMNG_FS_CHECK_FAILED:
	case EVENT_STORAGEMNG_FS_EXCEPTION: {
		if (MODEMNG_SetCardState(CARD_STATE_FSERROR))
			MODEMNG_MonitorStatusNotify(pstMsg);
		return PROCESS_MSG_RESULTE_OK;
	} break;

	case EVENT_FILEMNG_SCAN_COMPLETED: {
		if (MODEMNG_SetCardState(CARD_STATE_AVAILABLE))
			MODEMNG_MonitorStatusNotify(pstMsg);
	} break;
	case EVENT_USB_UVC_READY: {
		*pStateID = WORK_MODE_USBCAM;
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_RECMNG_STOPREC:
	case EVENT_RECMNG_SPLITREC:
	case EVENT_RECMNG_EVENTREC_END: {
		if (pstMsg->topic == EVENT_RECMNG_EVENTREC_END) {
			MODEMNG_SetEmrState(false);
		}
	}
	case EVENT_RECMNG_EMRREC_END:
	case EVENT_RECMNG_PIV_END:
		MODEMNG_MonitorStatusNotify(pstMsg);
#ifdef SERVICES_PHOTO_ON
	case EVENT_PHOTOMNG_PIV_END:
#endif
	{
		int32_t camid = pstMsg->arg1;
		if (pstMsg->topic == EVENT_RECMNG_EMRREC_END) {
			FILEMNG_MoveFile(camid, FILEMNG_DIR_EMR, (char *)pstMsg->aszPayload);
		} else {
			FILEMNG_AddFile(camid, (char *)pstMsg->aszPayload);
		}
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_RECMNG_STARTREC:
	case EVENT_RECMNG_STARTEVENTREC:
	case EVENT_RECMNG_STARTEMRREC:
		MODEMNG_MonitorStatusNotify(pstMsg);
	case EVENT_RECMNG_SPLITSTART: {
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_RECMNG_OPEN_FAILED:
	case EVENT_RECMNG_WRITE_ERROR: {
		MODEMNG_StopRec();
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_RECMNG_PIV_START:
#ifdef SERVICES_PHOTO_ON
	case EVENT_PHOTOMNG_PIV_START:
#endif
		MODEMNG_MonitorStatusNotify(pstMsg);
		break;
#ifdef SERVICES_SPEECH_ON
	case EVENT_SPEECHMNG_STARTREC:
	case EVENT_SPEECHMNG_STOPREC:
	case EVENT_SPEECHMNG_OPENFRONT:
	case EVENT_SPEECHMNG_OPENREAR:
	case EVENT_SPEECHMNG_CLOSESCREEN:
	case EVENT_SPEECHMNG_OPENSCREEN:
	case EVENT_SPEECHMNG_EMRREC:
	case EVENT_SPEECHMNG_PIV:
	case EVENT_SPEECHMNG_CLOSEWIFI:
	case EVENT_SPEECHMNG_OPENWIFI:
		MODEMNG_MonitorStatusNotify(pstMsg);
		break;
	case EVENT_MODEMNG_START_SPEECH: {
		MODEMNG_StartEventSpeech();
		return PROCESS_MSG_RESULTE_OK;
	}
	case EVENT_MODEMNG_STOP_SPEECH: {
		MODEMNG_StopEventSpeech();
		return PROCESS_MSG_RESULTE_OK;
	}
#endif
	default:
		break;
	}

	return PROCESS_MSG_RESULTE_OK;
}

static int32_t MODEMNG_OpenBaseMode(void)
{
	int32_t s32Ret = 0;

	s32Ret = USBCTRL_Init();
	MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "usbctrl init");

	/** init timedtask */
	s32Ret = TIMEDTASK_Init();
	MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "timetask init");

	s32Ret = MEDIA_VbInit();
	MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Vb init");

	CVI_LOGI("MEDIA_DispInit start\n");
	s32Ret = MEDIA_DispInit(true);
	MODEMNG_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MEDIA_DispInit fail");

	return 0;
}

static int32_t MODEMNG_CloseBaseMode(void)
{
	return 0;
}

int32_t MODEMNG_BaseStateInit(void)
{
	int32_t s32Ret = 0;
	MODEMNG_S *pstModeMngCtx = MODEMNG_GetModeCtx();

	pstModeMngCtx->stBase.stateID = WORK_MODE_BUTT;
	snprintf(pstModeMngCtx->stBase.name, STATE_NAME_LEN, "%s", MODEEMNG_STATE_BASE);
	pstModeMngCtx->stBase.open = MODEMNG_OpenBaseMode;
	pstModeMngCtx->stBase.close = MODEMNG_CloseBaseMode;
	pstModeMngCtx->stBase.processMessage = MODEMNG_BaseModeProcessMessage;
	pstModeMngCtx->stBase.argv = &(pstModeMngCtx->stBase);

	s32Ret = HFSM_AddState(pstModeMngCtx->pvHfsmHdl,
						   &(pstModeMngCtx->stBase),
						   NULL);
	return s32Ret;
}

/** deinit Base mode */
int32_t MODEMNG_BaseStatesDeinit(void)
{
	int32_t s32Ret = 0;

	return s32Ret;
}

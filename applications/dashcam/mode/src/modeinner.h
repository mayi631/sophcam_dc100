#ifndef __MODEINNER_H__
#define __MODEINNER_H__

#include <stdio.h>
#include <pthread.h>

#include "cvi_log.h"
#include "osal.h"
#include "sysutils_mq.h"
#include "mapi.h"
#include "mode.h"
#include "media_init.h"
#include "media_osd.h"
#ifdef SERVICES_LIVEVIEW_ON
#include "liveviewmng.h"
#endif
#include "param.h"
#include "recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "photomng.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "playbackmng.h"
#endif
#include "storagemng.h"
#include "sysutils_eventhub.h"

#ifdef SERVICES_LIVEVIEW_ON
#include "volmng.h"
#endif
#include "powercontrol.h"
#ifdef CONFIG_GSENSOR_ON
#include "gsensormng.h"
#endif
#ifdef WIFI_ON
#include "wifimng.h"
#endif
#ifdef SERVICES_SPEECH_ON
#include "speechmng.h"
#endif

#ifdef SERVICES_ADAS_ON
#include "adasmng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define MODE_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/** mode name */
#define MODEEMNG_STATE_BASE "Base"
#define MODEEMNG_STATE_REC "Movie"
#define MODEEMNG_STATE_PHOTO "Photo"
#define MODEEMNG_STATE_PLAYBACK "PlayBack"
#define MODEEMNG_STATE_UVC "UVC"
#define MODEEMNG_STATE_USB_STORAGE "UsbStorage"
#define MODEEMNG_STATE_UPGRADE "Upgrade"
#define MODEEMNG_STATE_UPDATE "Update"
#define MODEEMNG_STATE_LAPSE "LapseTime"
#define MODEEMNG_STATE_BOOTFIRST "BootFirst"
#define MODEEMNG_STATE_USBMENU "UsbMenu"
/** NULL pointer check */
#define MODEMNG_CHECK_POINTER(ptr, errcode, string)  \
	do {                                             \
		if (NULL == ptr) {                           \
			CVI_LOGE("%s NULL pointer\n\n", string); \
			return errcode;                          \
		}                                            \
	} while (0)

/** function ret value check */
#define MODEMNG_CHECK_RET(ret, errcode, errstring)                     \
	do {                                                               \
		if (0 != ret) {                                                \
			CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, ret); \
			return errcode;                                            \
		}                                                              \
	} while (0)

/** message proc function parameter check */
#define MODEMNG_CHECK_MSGPROC_FUNC_PARAM(argv, pStateID, Message, InProgress) \
	do {                                                                      \
		if (NULL == argv || NULL == pStateID || NULL == Message) {            \
			CVI_LOGE("parameter argv or pStateID or Message NULL\n\n");       \
			MUTEX_LOCK(MODEMNG_GetModeCtx()->Mutex);                          \
			InProgress = false;                                               \
			MUTEX_UNLOCK(MODEMNG_GetModeCtx()->Mutex);                        \
			return PROCESS_MSG_RESULTE_OK;                                    \
		}                                                                     \
	} while (0)

/**check ret value, unlock mutex when error */
#define MODEMNG_CHECK_CHECK_RET_WITH_UNLOCK(retvalue, errcode, errstring)   \
	do {                                                                    \
		if (0 != retvalue) {                                                \
			CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, retvalue); \
			MUTEX_UNLOCK(MODEMNG_GetModeCtx()->Mutex);                      \
			return errcode;                                                 \
		}                                                                   \
	} while (0)

/**check init, unlock mutex when error */
#define MODEMNG_CHECK_CHECK_INIT(retvalue, errcode, errstring)              \
	do {                                                                    \
		if (0 == retvalue) {                                                \
			CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, retvalue); \
			return errcode;                                                 \
		}                                                                   \
	} while (0)

/** Expression Check Without Return */
#define MODEMNG_CHECK_EXPR_WITHOUT_RETURN(expr, errstring) \
	do {                                                   \
		if (expr) {                                        \
			CVI_LOGE("[%s] failed\n", errstring);          \
		}                                                  \
	} while (0)

int32_t MODEMNG_PublishEvent(MESSAGE_S *pstMsg, int32_t s32Result);

/** update statemng status */
#define MODEMNG_UPDATESTATUS(pstMsg, Result, bProgress) \
	do {                                                \
		MUTEX_LOCK(MODEMNG_GetModeCtx()->Mutex);        \
		MODEMNG_GetModeCtx()->bInProgress = bProgress;  \
		MUTEX_UNLOCK(MODEMNG_GetModeCtx()->Mutex);      \
		MODEMNG_PublishEvent(pstMsg, Result);           \
	} while (0)

MODEMNG_S *MODEMNG_GetModeCtx(void);

int32_t MODEMNG_BaseStateInit(void);
int32_t MODEMNG_BaseStatesDeinit(void);
int32_t MODEMNG_MovieStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_MovieStatesDeinit(void);
int32_t MODEMNG_PlaybackStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_PlaybackStatesDeinit(void);
int32_t MODEMNG_MonitorStatusNotify(EVENT_S *pstMsg);
int32_t MODEMNG_ResetMovieMode(PARAM_CFG_S *Param);
int32_t MODEMNG_InitStorage(void);
int32_t MODEMNG_StopRec(void);
int32_t MODEMNG_InitFilemng(void);
int32_t MODEMNG_DeInitFilemng(void);
int32_t MODEMNG_DeintStorage(void);
int32_t MODEMNG_UpDateStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_UpDateStatesDeinit(void);
int32_t MODEMNG_UvcStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_UvcStatesDeinit(void);
int32_t MODEMNG_StorageStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_StorageStatesDeinit(void);

bool MODEMNG_SetCardState(uint32_t u32CardState);
int32_t MODEMNG_SetEmrState(bool en);
int32_t MODEMNG_StartMemoryBufferRec(void);
int32_t MODEMNG_StopMemoryBufferRec(void);
void MODEMNG_SetMediaVencFormat(int32_t value);
int32_t MODEMNG_Format(char *labelname);
int32_t MODEMNG_LiveViewSwitch(uint32_t viewwin);
int32_t MODEMNG_OpenUvcMode(void);
int32_t MODEMNG_CloseUvcMode(void);
int32_t MODEMNG_ContextInit(const MODEMNG_CONFIG_S *pstModemngCfg);

#ifdef SERVICES_PHOTO_ON
int32_t MODEMNG_PhotoStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_PhotoStatesDeinit(void);
#endif

int32_t MODEMNG_SetCurModeMedia(WORK_MODE_E CurMode);
int32_t MODEMNG_BootFirstStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_BootFirstStatesDeinit(void);
int32_t MODEMNG_UsbMenuStatesInit(const STATE_S *pstBase);
int32_t MODEMNG_UsbMenuStatesDeinit(void);

#ifdef SERVICES_SPEECH_ON
int32_t SPEECHMNG_StartSpeech(void);
int32_t MODEMNG_StopEventSpeech(void);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MODEINNER__ */

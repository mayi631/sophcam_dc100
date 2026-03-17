#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "hal_watchdog.h"
#include "watchdogmng.h"
#include "osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct tagWATCHDOGMNG_CONTEXT_S {
	uint8_t bWDTMNGInitState;
	uint8_t bWdtCheckRun;
	pthread_t WdtCheckId;
	int32_t s32Time;
} WATCHDOGMNG_CONTEXT_S;
static WATCHDOGMNG_CONTEXT_S s_stWdtCtx;

static void *WATCHDOG_CheckThread(void *pData)
{
	CVI_LOGD("thread WATCHDOG Check enter\n");
	char szThreadName[APPCOMM_MAX_PATH_LEN];
	snprintf(szThreadName, APPCOMM_MAX_PATH_LEN, "%s", "WDT_CHECK");
	prctl(PR_SET_NAME, szThreadName, 0, 0, 0); /**< Set Task Name */

	while (s_stWdtCtx.bWdtCheckRun) {
		HAL_WATCHDOG_Feed();
		OSAL_TASK_Sleep((s_stWdtCtx.s32Time - 1) * 1000 * 1000);
	}

	CVI_LOGD("thread WATCHDOG_CheckThread exit\n");
	return NULL;
}

int32_t WATCHDOGMNG_Init(int32_t s32Time_s)
{
	int32_t s32Ret = APP_SUCCESS;
	if (s_stWdtCtx.bWDTMNGInitState == APP_TRUE) {
		CVI_LOGE("wdtmng has already been started\n");
		return APP_FAILURE;
	}

	if (s32Time_s < 2 || s32Time_s > 1000) {
		CVI_LOGE("Interval time should not be less then two and bigger then 100. %d\n", s32Time_s);
		return APP_FAILURE;
	}

	s32Ret = HAL_WATCHDOG_Init(s32Time_s);
	if (APP_SUCCESS != s32Ret) {
		CVI_LOGE("HAL_WATCHDOG_Init: failed, errno(%d)\n", s32Ret);
		return APP_FAILURE;
	}

	s_stWdtCtx.bWdtCheckRun = APP_TRUE;
	s_stWdtCtx.s32Time = s32Time_s;
	s32Ret = pthread_create(&(s_stWdtCtx.WdtCheckId), NULL, WATCHDOG_CheckThread, NULL);
	if (s32Ret != APP_SUCCESS) {
		CVI_LOGE("Create WATCHDOG_CheckThread Thread Fail!\n");
		return APP_FAILURE;
	}

	s_stWdtCtx.bWDTMNGInitState = APP_TRUE;
	return APP_SUCCESS;
}

int32_t WATCHDOGMNG_DeInit(void)
{
	int32_t s32Ret = APP_SUCCESS;

	if (s_stWdtCtx.bWDTMNGInitState == APP_FALSE) {
		CVI_LOGE("watchdogmng no init\n");
		return APP_FAILURE;
	}

	/** Destory Key Check Task */
	s_stWdtCtx.bWdtCheckRun = APP_FALSE;
	s32Ret = pthread_join(s_stWdtCtx.WdtCheckId, NULL);
	if (s32Ret != APP_SUCCESS) {
		CVI_LOGE("Join WATCHDOG_CheckThread  Fail!\n");
		return APP_FAILURE;
	}

	HAL_WATCHDOG_Deinit();
	s_stWdtCtx.bWDTMNGInitState = APP_FALSE;
	return APP_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

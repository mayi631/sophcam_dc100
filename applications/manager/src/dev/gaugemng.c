#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "osal.h"
#include "cvi_log.h"
#include "gaugemng.h"
#include "hal_adc.h"
#include "usb.h"
#include "sysutils_eventhub.h"
#include "hal_gpio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static bool s_bGAUGEMNGInitState = 0;
static pthread_mutex_t s_GAUGEMNGMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t s_GAUGEMNGCheckId;
static bool s_bGAUGEMNGCheckRun;
static uint8_t histoy_ret = 100;

/** macro define */
#define GAUGEMNG_CHECK_INTERVAL (1)	 /**< gauge check interval, unit s */
#define GAUGEMNG_LOWLEVEL_RESET (20) /**< gauge level interval, percent */

/* Vinput/Vref x 4095;Vref=1.5v */
#define REFERENCE_VOLTAGE (1500)
#define ADC_RESOLUTION (4096) // 12bit

/** gaugemng information */
typedef struct tagGAUGEMNG_INFO_S {
	int32_t s32Level;
	int32_t s32LastLevel;
	int32_t last_index;
	bool bLowLevelState;
	bool bUltraLowLevelState;
	bool bCharge;
	bool bLastCharge;
	GAUGEMNG_CFG_S stCfg;
} GAUGEMNG_INFO_S;

static GAUGEMNG_INFO_S s_stGAUGEMNGInfo;

static int32_t _GAUGEMNG_GetAverage(int32_t *battery_array, int32_t num)
{
	int32_t min = battery_array[0];
	int32_t max = battery_array[0];
	int32_t sum = 0;
	int32_t index;

	for (index = 0; index < num; index++) {
		if (battery_array[index] > max) {
			max = battery_array[index];
		} else if (battery_array[index] < min) {
			min = battery_array[index];
		}
		sum += battery_array[index];
	}

	return (num > 3 ? ((sum - max - min) / (num - 2)) : (sum / num));
}

static int32_t GAUGEMNG_InParmValidChck(const GAUGEMNG_CFG_S *stCfg)
{
	/** parm check */
	if (NULL == stCfg) {
		CVI_LOGE("GAUGEMNG_InParmValidChck fail\n");
		return -1;
	}

	if (stCfg->s32LowLevel > 100 || stCfg->s32UltraLowLevel <= 0 || stCfg->s32UltraLowLevel > stCfg->s32LowLevel) {
		CVI_LOGE("stCfg parm error\n");
		return -1;
	}
	return 0;
}

static int32_t GAUGEMNG_InternalParmInit(const GAUGEMNG_CFG_S *pstCfg)
{
	if (NULL == pstCfg) {
		CVI_LOGE("GAUGEMNG_InParmValidChck fail\n");
		return -1;
	}

	s_stGAUGEMNGInfo.s32Level = 0;
	s_stGAUGEMNGInfo.s32LastLevel = 0;
	s_stGAUGEMNGInfo.last_index = 0;
	s_stGAUGEMNGInfo.bLowLevelState = 0;
	s_stGAUGEMNGInfo.bUltraLowLevelState = 0;

	memcpy(&s_stGAUGEMNGInfo.stCfg, pstCfg, sizeof(GAUGEMNG_CFG_S));

	return 0;
}

static void GAUGEMNG_UltraLowLevelCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{
	EVENT_S stEvent;
	if (0 == pstGaugemngInfo->bUltraLowLevelState) {
		if (pstGaugemngInfo->s32Level <= pstGaugemngInfo->stCfg.s32UltraLowLevel && pstGaugemngInfo->bCharge == 0) {
			memset(&stEvent, '\0', sizeof(stEvent));
			stEvent.arg1 = pstGaugemngInfo->s32Level;
			stEvent.topic = EVENT_GAUGEMNG_LEVEL_ULTRALOW;
			EVENTHUB_Publish(&stEvent);
			CVI_LOGW("battery level ultralow event\n");
			pstGaugemngInfo->bUltraLowLevelState = 1;
		}
	}
	return;
}

static void GAUGEMNG_LowLevelCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{
	EVENT_S stEvent;

	// CVI_LOGW("s32LowLevel = %d, s32UltraLowLevel = %d\n", pstGaugemngInfo->stCfg.s32LowLevel, pstGaugemngInfo->stCfg.s32UltraLowLevel);
	if (0 == pstGaugemngInfo->bLowLevelState) {
		if (pstGaugemngInfo->s32Level <= pstGaugemngInfo->stCfg.s32LowLevel && pstGaugemngInfo->bCharge == 0) {
			memset(&stEvent, '\0', sizeof(stEvent));
			stEvent.topic = EVENT_GAUGEMNG_LEVEL_LOW;
			stEvent.arg1 = pstGaugemngInfo->s32Level;
			EVENTHUB_Publish(&stEvent);
			CVI_LOGW("battery level low event\n");
			// pstGaugemngInfo->bLowLevelState = 1;
		}
	} else {
		if (pstGaugemngInfo->s32Level >= (pstGaugemngInfo->stCfg.s32LowLevel + GAUGEMNG_LOWLEVEL_RESET)) {
			memset(&stEvent, '\0', sizeof(stEvent));
			stEvent.topic = EVENT_GAUGEMNG_LEVEL_NORMAL;
			stEvent.arg1 = pstGaugemngInfo->s32Level;
			EVENTHUB_Publish(&stEvent);
			CVI_LOGW("battery level restore normal\n");

			pstGaugemngInfo->bLowLevelState = 0;
			pstGaugemngInfo->bUltraLowLevelState = 0;
		}
	}
	return;
}

static int32_t get_usb_charging_by_io(uint32_t *value)
{
	int32_t ret = 0;

	ret = HAL_GPIO_Get_Value(s_stGAUGEMNGInfo.stCfg.s32USBChargerDetectGPIO, value);
	if (ret) {
		CVI_LOGI("HAL_GPIO_Get_Value error\n");
		return -1;
	}
	return 0;
}

static void GAUGEMNG_ChargeStateChangeCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{
	EVENT_S stEvent;

	if (NULL == pstGaugemngInfo) {
		CVI_LOGE("error the charge state\n");
		return;
	}

	if (pstGaugemngInfo->bCharge != pstGaugemngInfo->bLastCharge) {
		memset(&stEvent, '\0', sizeof(stEvent));
		stEvent.topic = EVENT_GAUGEMNG_CHARGESTATE_CHANGE;
		stEvent.arg1 = pstGaugemngInfo->bCharge;
		EVENTHUB_Publish(&stEvent);
		CVI_LOGW("battery chargestate changge event\n");
	}
	return;
}

int32_t GAUGENBG_IdexChangeCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{

	int32_t index = 0;
	int32_t s32Level = pstGaugemngInfo->s32Level;

	if (pstGaugemngInfo->bCharge == 1) {
		index = 4;
	} else {
		if (s32Level > 95 && s32Level <= 100) {
			index = 0;
		} else if (s32Level > 88 && s32Level <= 95) {
			index = 1;
		} else if (s32Level > 83 && s32Level <= 88) {
			index = 2;
		} else if (s32Level > 0 && s32Level <= 83) {
			index = 3;
		}
	}

	return index;
}

static void GAUGEMNG_LevelChangeCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{
	EVENT_S stEvent;
	int32_t index = 0;

	index = GAUGENBG_IdexChangeCheck(pstGaugemngInfo);

	if (index != pstGaugemngInfo->last_index) {
		pstGaugemngInfo->last_index = index;
		memset(&stEvent, '\0', sizeof(stEvent));
		stEvent.topic = EVENT_GAUGEMNG_LEVEL_CHANGE;
		stEvent.arg1 = index;
		EVENTHUB_Publish(&stEvent);
	}
	return;
}

static int32_t GAUGEMNG_ChargeStateCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{
	bool bCharge = 0;
	if (NULL == pstGaugemngInfo) {
		CVI_LOGE("error the charge state\n");
		return -1;
	}

	uint32_t charge = 0;
	get_usb_charging_by_io(&charge);
	// CVI_LOGE("charge = %d\n", charge);
	if (USB_STATE_INSERT == charge) {
		bCharge = 1;
	} else if (USB_STATE_OUT == charge) {
		bCharge = 0;
		// OSAL_FS_System("poweroff -f");
	}
	if (pstGaugemngInfo->bCharge != bCharge) {
		pstGaugemngInfo->bCharge = bCharge;
		GAUGEMNG_LevelChangeCheck(pstGaugemngInfo);
		// GAUGEMNG_ChargeStateChangeCheck(pstGaugemngInfo);
		pstGaugemngInfo->bLastCharge = bCharge;
	}
	return 0;
}

/*get vbattery capacity adc percentage*/
int32_t GAUGEMNG_GetPercentage(void)
{
	int32_t ret, vbat, index;
	int32_t vbat_sample[10] = {0};
	int32_t count = ARRAY_SIZE(vbat_sample);

	for (index = 0; index < count; index++) {
		ret = HAL_ADC_GetValue(s_stGAUGEMNGInfo.stCfg.s32ADCChannelVbat);
		if (-1 == ret) {
			CVI_LOGE("HAL_ADC_GetValue fail\n");
			break;
		}
		// CVI_LOGE("[S]:ret = %d, index = %d\n", ret, index);
		vbat_sample[index] = ret;
	}
	vbat = _GAUGEMNG_GetAverage(vbat_sample, count);

	ret = vbat * 100 / ADC_RESOLUTION;
	if (ret - histoy_ret > 2) {
		histoy_ret = ret;
	}
	ret = ret < histoy_ret ? ret : histoy_ret;
	histoy_ret = ret;
	// CVI_LOGE("[S]:ret = %d, history_ret = %d, vbat = %d\n", ret, histoy_ret, vbat);
	return ret;
}

static int32_t GAUGEMNG_LevelCheck(GAUGEMNG_INFO_S *pstGaugemngInfo)
{
	int32_t s32Level = 0;

	if (NULL == pstGaugemngInfo) {
		CVI_LOGE("error the charge state\n");
		return -1;
	}

	s32Level = GAUGEMNG_GetPercentage();
	if (-1 == s32Level || s32Level > 100) {
		CVI_LOGE("CVI_HAL_GAUGE_GetLevel fail\n");
		return -1;
	}

	GAUGEMNG_ChargeStateCheck(pstGaugemngInfo);
	GAUGEMNG_LowLevelCheck(pstGaugemngInfo);

	if (pstGaugemngInfo->s32Level != s32Level) {
		pstGaugemngInfo->s32Level = s32Level;
		GAUGEMNG_LevelChangeCheck(pstGaugemngInfo);
		GAUGEMNG_UltraLowLevelCheck(pstGaugemngInfo);
		pstGaugemngInfo->s32LastLevel = s32Level;
	}
	return 0;
}

static void *GAUGEMNG_LevelCheckThread(void *pData)
{
	/** Set Task Name */
	prctl(PR_SET_NAME, "GAUGE_LEVEL_CHECK", 0, 0, 0);

	while (s_bGAUGEMNGCheckRun) {
		GAUGEMNG_LevelCheck(&s_stGAUGEMNGInfo);
		// GAUGEMNG_ChargeStateCheck(&s_stGAUGEMNGInfo);
		usleep(1 * 1000 * 1000);
	}

	CVI_LOGW("gauge check thread exit\n");
	return NULL;
}

int32_t GAUGEMNG_RegisterEvent(void)
{
	int32_t s32Ret = 0;
	s32Ret = EVENTHUB_RegisterTopic(EVENT_GAUGEMNG_LEVEL_CHANGE);
	if (0 != s32Ret) {
		CVI_LOGE("Register battery level change event fail\n");
		return -1;
	}
	s32Ret = EVENTHUB_RegisterTopic(EVENT_GAUGEMNG_LEVEL_LOW);
	if (0 != s32Ret) {
		CVI_LOGE("Register battery level low event fail\n");
		return -1;
	}
	s32Ret = EVENTHUB_RegisterTopic(EVENT_GAUGEMNG_LEVEL_ULTRALOW);
	if (0 != s32Ret) {
		CVI_LOGE("Register battery level ultra low event fail\n");
		return -1;
	}
	s32Ret = EVENTHUB_RegisterTopic(EVENT_GAUGEMNG_LEVEL_NORMAL);
	if (0 != s32Ret) {
		CVI_LOGE("Register battery level recover normal event fail\n");
		return -1;
	}
	s32Ret = EVENTHUB_RegisterTopic(EVENT_GAUGEMNG_CHARGESTATE_CHANGE);
	if (0 != s32Ret) {
		CVI_LOGE("Register battery level recover normal event fail\n");
		return -1;
	}
	return 0;
}

int32_t GAUGEMNG_Init(const GAUGEMNG_CFG_S *pstCfg)
{
	int32_t s32Ret;

	if (NULL == pstCfg) {
		CVI_LOGE("parm check error\n");
		return -1;
	}
	MUTEX_LOCK(s_GAUGEMNGMutex);

	if (1 == s_bGAUGEMNGInitState) {
		CVI_LOGE("already been started\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	s32Ret = GAUGEMNG_InParmValidChck(pstCfg);
	if (0 != s32Ret) {
		CVI_LOGE("GAUGEMNG_InParmValidChck Failed\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	s32Ret = HAL_ADC_Init();
	if (0 != s32Ret) {
		CVI_LOGE("HI_HAL_GAUGE_Init Failed\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	GAUGEMNG_InternalParmInit(pstCfg);

	/* Create gauge Check Task */
	s_bGAUGEMNGCheckRun = 1;
	s32Ret = pthread_create(&s_GAUGEMNGCheckId, NULL, GAUGEMNG_LevelCheckThread, NULL);
	if (0 != s32Ret) {
		CVI_LOGE("Create GaugeCheck Thread Fail!\n");
		HAL_ADC_Deinit();
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	s_bGAUGEMNGInitState = 1;
	MUTEX_UNLOCK(s_GAUGEMNGMutex);
	return 0;
}

int32_t GAUGEMNG_GetBatteryLevel(uint8_t *ps32Level)
{
	int32_t s32Ret = 0;
	if (NULL == ps32Level) {
		CVI_LOGE("error point value\n");
		return -1;
	}

	MUTEX_LOCK(s_GAUGEMNGMutex);
	if (false == s_bGAUGEMNGInitState) {
		CVI_LOGE("gaugemng no init\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	s32Ret = GAUGENBG_IdexChangeCheck(&s_stGAUGEMNGInfo);
	if (s32Ret < 0) {
		CVI_LOGE("CVI_HAL GAUGE GetLevel fail\n");
		return -1;
	}
	*ps32Level = s32Ret;
	MUTEX_UNLOCK(s_GAUGEMNGMutex);
	return 0;
}

int32_t GAUGEMNG_GetChargeState(bool *pbCharge)
{
	uint32_t charge = 0;

	if (NULL == pbCharge) {
		CVI_LOGE("pbCharge is null point\n");
		return -1;
	}

	MUTEX_LOCK(s_GAUGEMNGMutex);
	if (0 == s_bGAUGEMNGInitState) {
		CVI_LOGE("gaugemng no init\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	get_usb_charging_by_io(&charge);
	if (USB_STATE_INSERT == charge) {
		*pbCharge = 1;
	} else {
		*pbCharge = 0;
	}
	MUTEX_UNLOCK(s_GAUGEMNGMutex);
	return 0;
}

int32_t GAUGEMNG_DeInit(void)
{
	int32_t s32Ret;
	MUTEX_LOCK(s_GAUGEMNGMutex);
	/** Destory gauge Check Task */
	if (0 == s_bGAUGEMNGInitState) {
		CVI_LOGE("gaugemng no init\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}
	s_bGAUGEMNGCheckRun = 0;
	s32Ret = pthread_join(s_GAUGEMNGCheckId, NULL);
	if (0 != s32Ret) {
		CVI_LOGE("Join GaugeCheck Thread Fail!\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}

	/** Close hal ADC */
	s32Ret = HAL_ADC_Deinit();
	if (0 != s32Ret) {
		CVI_LOGE("HAL_ADC_Deinit Fail!\n");
		MUTEX_UNLOCK(s_GAUGEMNGMutex);
		return -1;
	}
	s_bGAUGEMNGInitState = 0;

	MUTEX_UNLOCK(s_GAUGEMNGMutex);
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

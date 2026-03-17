#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <string.h>
#include "sysutils_eventhub.h"
#include "speechmng.h"
#include "osal.h"

static SPEECH_HANDLE_S *speechHdl;

static int32_t SPEECHMNG_RegisterEvent(void)
{
	int32_t s32Ret = 0;
	s32Ret = EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_STARTREC);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_STOPREC);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENFRONT);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENREAR);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_CLOSESCREEN);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENSCREEN);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_EMRREC);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_PIV);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_OPENWIFI);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_SPEECHMNG_CLOSEWIFI);
	APPCOMM_CHECK_RETURN(s32Ret, SPEECHMNG_EREGISTER_EVENT);
	return s32Ret;
}

static int32_t SPEECHMNG_Function(int32_t index)
{
	EVENT_S stEvent;
	memset(&stEvent, 0x0, sizeof(EVENT_S));
	switch (index) {
	case SPEECHMNG_TURN_OFF_RECORDING:
		stEvent.topic = EVENT_SPEECHMNG_STOPREC;
		break;
	case SPEECHMNG_OPEN_RECORDING:
		stEvent.topic = EVENT_SPEECHMNG_STARTREC;
		break;
	case SPEECHMNG_SHOW_FRONT:
		stEvent.topic = EVENT_SPEECHMNG_OPENFRONT;
		break;
	case SPEECHMNG_SHOW_REAR:
		stEvent.topic = EVENT_SPEECHMNG_OPENREAR;
		break;
	case SPEECHMNG_TURN_OFF_SCREEN:
		stEvent.topic = EVENT_SPEECHMNG_CLOSESCREEN;
		break;
	case SPEECHMNG_OPEN_SCREEN:
		stEvent.topic = EVENT_SPEECHMNG_OPENSCREEN;
		break;
	case SPEECHMNG_LOCK_VEDIO:
		stEvent.topic = EVENT_SPEECHMNG_EMRREC;
		break;
	case SPEECHMNG_TAKE_PICTURE:
		stEvent.topic = EVENT_SPEECHMNG_PIV;
		break;
	case SPEECHMNG_TURN_OFF_WIFI:
		stEvent.topic = EVENT_SPEECHMNG_CLOSEWIFI;
		break;
	case SPEECHMNG_OPEN_WIFI:
		stEvent.topic = EVENT_SPEECHMNG_OPENWIFI;
		break;
	default:
		break;
	}
	EVENTHUB_Publish(&stEvent);
	return 0;
}

int32_t SPEECHMNG_Init(SPEECHMNG_PARAM_S *SpeechCfg)
{
	int32_t s32Ret = 0;
	s32Ret = SPEECHMNG_RegisterEvent();
#ifdef __CV184X__
	s32Ret = OSAL_FS_Insmod(KOMOD_PATH "/bmtpu.ko", NULL);
#else
	s32Ret = OSAL_FS_Insmod(KOMOD_PATH "/cv181x_tpu.ko", NULL);
#endif
	SPEECH_SERVICE_Register(SPEECHMNG_Function);
	if (speechHdl == NULL) {
		speechHdl = malloc(sizeof(SPEECH_CONTEXT_HANDLE_S));
	}
	SPEECH_SERVICE_PARAM_S speechParam = {0};
	memcpy(&speechParam, SpeechCfg, sizeof(SPEECHMNG_PARAM_S));
	s32Ret = SPEECH_SERVICE_Create(speechHdl, &speechParam);
	return s32Ret;
}

int32_t SPEECHMNG_DeInit(void)
{
	int32_t s32Ret = 0;
	if (*speechHdl != NULL) {
		s32Ret = SPEECH_SERVICE_Destroy(*speechHdl);
	#ifdef __CV184X__
		OSAL_FS_Rmmod(KOMOD_PATH "/bmtpu.ko");
	#else
		OSAL_FS_Rmmod(KOMOD_PATH "/cv181x_tpu.ko");
	#endif
		if (*speechHdl != NULL) {
			free(speechHdl);
			*speechHdl = NULL;
		}
	}

	return s32Ret;
}

int32_t SPEECHMNG_StartSpeech(void)
{
	int32_t s32Ret = 0;
	s32Ret = SPEECH_SERVICE_StartSpeech(*speechHdl);
	return s32Ret;
}

int32_t SPEECHMNG_StopSpeech(void)
{
	int32_t s32Ret = 0;
	SPEECH_SERVICE_StopSpeech(*speechHdl);
	return s32Ret;
}

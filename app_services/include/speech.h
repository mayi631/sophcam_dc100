#ifndef __SPEECH_H__
#define __SPEECH_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "osal.h"
#include "cvi_tdl.h"

typedef void *SPEECH_HANDLE_S;
typedef int32_t (*FUNC_SPEECH_HANDLE)(int32_t index);

typedef struct SPEECH_SERVICE_PARAM_S {
	uint32_t enable;
	uint32_t SampleRate;
	uint32_t BitWidth;
	uint32_t AiNumPerFrm;
	char ModelPath[128];
} SPEECH_SERVICE_PARAM_S;

typedef SPEECH_SERVICE_PARAM_S speech_param_t, *speech_param_handle_t;

typedef struct __speech_condext {
	speech_param_t params;
	cvitdl_handle_t SpeechHandle;
	volatile uint32_t shutdown;
	OSAL_TASK_HANDLE_S speech_task;
	uint8_t *tmpbuff;
	pthread_mutex_t g_CryMutex;
	bool enable;
} SPEECH_CONTEXT_S, *SPEECH_CONTEXT_HANDLE_S;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t SPEECH_SERVICE_Create(SPEECH_HANDLE_S *hdl, SPEECH_SERVICE_PARAM_S *params);
int32_t SPEECH_SERVICE_Destroy(SPEECH_HANDLE_S hdl);
int32_t SPEECH_SERVICE_StartSpeech(SPEECH_HANDLE_S hdl);
int32_t SPEECH_SERVICE_StopSpeech(SPEECH_HANDLE_S hdl);
void SPEECH_SERVICE_Register(FUNC_SPEECH_HANDLE pfunction);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
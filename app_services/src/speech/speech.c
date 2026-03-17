#include <stdio.h>
#include <stdlib.h>
#include "speech.h"
#include "cvi_log.h"
#include "mapi.h"
#include "audio_service.h"

#define SPEECH_BUFFER_SECOND 2
#define SR_ORDER_PAUSE_TIME_MS (2000 * 1000)
#define SR_NO_ORDER_PAUSE_TIME_MS (100 * 1000)

static FUNC_SPEECH_HANDLE SPEECH_SERVICE_pFunc = NULL;
static int g_pack_idx = 0;
static const char *g_stEnumOrderStr[] = {"无指令", "打开前路", "打开后路", "关闭屏幕", "打开屏幕", "紧急录像", "我要拍照", "关闭录影", "打开录影", "打开wifi", "关闭wifi"};

#ifdef __cplusplus
#if __cplusplus

extern "C" {
#endif
#endif /* __cplusplus */

void SPEECH_SERVICE_Register(FUNC_SPEECH_HANDLE pfunction)
{
	SPEECH_SERVICE_pFunc = pfunction;
}

int32_t SPEECH_Audio_Buffer_Get(uint8_t *pAudioBuffer, uint32_t u32BufferLen, SPEECH_CONTEXT_HANDLE_S speech)
{
	if (pAudioBuffer == NULL) {
		printf("input buffer is NULL\n");
		return -1;
	}
	if (speech->tmpbuff == NULL) {
		printf("enter g_pSRBuffer is NULL\n");
		return -1;
	}
	pthread_mutex_lock(&speech->g_CryMutex);
	memcpy(pAudioBuffer, speech->tmpbuff, u32BufferLen);
	pthread_mutex_unlock(&speech->g_CryMutex);
	return 0;
}

static void speech_task_entry(void *arg)
{
	CVI_LOGI("enter speech_task_entry\n");
	SPEECH_CONTEXT_HANDLE_S speech = (SPEECH_CONTEXT_HANDLE_S)arg;
	uint32_t SampleRate = speech->params.SampleRate;
	uint32_t BitWidth = speech->params.BitWidth;
	// int32_t rc = 0;
	int32_t pack_idx = 0;
	int32_t pack_len = (speech->params.AiNumPerFrm + 255) & ~255;
	int32_t u32BufferSize = SampleRate * (BitWidth + 1) * SPEECH_BUFFER_SECOND;
	int32_t index = -1;
	uint8_t *buffer;
	buffer = malloc(u32BufferSize);
	VIDEO_FRAME_INFO_S Frame;
	memset(&Frame, 0, sizeof(VIDEO_FRAME_INFO_S));
	Frame.stVFrame.pu8VirAddr[0] = buffer; // Global buffer
	Frame.stVFrame.u32Height = 1;
	Frame.stVFrame.u32Width = u32BufferSize;
	while (!speech->shutdown) {
		while (!speech->shutdown && speech->enable == 0) {
			OSAL_TASK_Sleep(SR_NO_ORDER_PAUSE_TIME_MS);
		}
		pack_idx = g_pack_idx;
		SPEECH_Audio_Buffer_Get(buffer, u32BufferSize, speech);

		CVI_TDL_SoundClassificationPack(speech->SpeechHandle, &Frame, pack_idx, pack_len, &index);
		if (index > 0) {
			CVI_LOGI("esc class: %s, index = %d\n", g_stEnumOrderStr[index], index);
			if (NULL != SPEECH_SERVICE_pFunc) {
				SPEECH_SERVICE_pFunc(index);
			}
			OSAL_TASK_Sleep(SR_ORDER_PAUSE_TIME_MS);
		} else {
			OSAL_TASK_Sleep(SR_NO_ORDER_PAUSE_TIME_MS);
		}
	}
	free(buffer);
}

static int32_t speech_start_task(SPEECH_CONTEXT_HANDLE_S speech)
{
	OSAL_TASK_ATTR_S ta;
	ta.name = "speech";
	ta.entry = speech_task_entry;
	ta.param = (void *)speech;
	ta.priority = OSAL_TASK_PRI_NORMAL;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	int32_t rc = OSAL_TASK_Create(&ta, &speech->speech_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("speech task create failed, %d\n", rc);
		return -1;
	}

	return 0;
}

static int32_t speech_stop_task(SPEECH_CONTEXT_HANDLE_S speech)
{

	int32_t rc = OSAL_TASK_Join(speech->speech_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("speech task join failed, %d\n", rc);
		return -1;
	}
	OSAL_TASK_Destroy(&speech->speech_task);

	return 0;
}

static void process_speech_audio_frame(const AUDIO_FRAME_S *audio_frame, const AEC_FRAME_S *aec_frame,
									   void *arg)
{
	(void)aec_frame;
	(void)arg;
	SPEECH_CONTEXT_HANDLE_S speech = (SPEECH_CONTEXT_HANDLE_S)arg;
	uint32_t SampleRate = speech->params.SampleRate;
	uint32_t BitWidth = speech->params.BitWidth;
	uint32_t buffer_len = SampleRate * (BitWidth + 1) * SPEECH_BUFFER_SECOND;
	pthread_mutex_lock(&speech->g_CryMutex);
	memmove(speech->tmpbuff, speech->tmpbuff + audio_frame->u32Len * 2, buffer_len - audio_frame->u32Len * 2);
	memcpy(speech->tmpbuff + buffer_len - audio_frame->u32Len * 2, audio_frame->u64VirAddr[0], audio_frame->u32Len * 2);
	g_pack_idx += 1;
	pthread_mutex_unlock(&speech->g_CryMutex);
}

static int32_t Speech_Proc_Init(SPEECH_CONTEXT_HANDLE_S speech)
{
	CVI_LOGI("Speech_Proc_Init\n");
	int32_t rc = 0;
	if (speech->SpeechHandle == NULL) {
		rc = CVI_TDL_CreateHandle(&speech->SpeechHandle);
		if (rc != 0) {
			CVI_LOGI("CVI_AI_CreateHandle failed\n");
			return rc;
		}
	} else {
		CVI_LOGI("CVI_AI_CreateHandle has created\n");
		return rc;
	}
	rc = CVI_TDL_SetPerfEvalInterval(speech->SpeechHandle, CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION, 10);
	if (rc != 0) {
		CVI_LOGI("CVI_AI_SetPerfEvalInterval failed with %#x!\n", rc);
		return rc;
	}
	CVI_LOGI("enter model_path = %s\n", speech->params.ModelPath);

	rc = CVI_TDL_OpenModel(speech->SpeechHandle, CVI_TDL_SUPPORTED_MODEL_SOUNDCLASSIFICATION, speech->params.ModelPath);
	if (rc != 0) {
		CVI_LOGI("CVI_AI_SetModelPath failed with %#x! maybe reset model path\n", rc);
		return rc;
	}
	return 0;
}

int32_t SPEECH_SERVICE_Create(SPEECH_HANDLE_S *hdl, SPEECH_SERVICE_PARAM_S *params)
{
	CVI_LOGI("enter SPEECH_SERVICE_Create\n");
	int32_t rc = 0;
	g_pack_idx = 0;
	SPEECH_CONTEXT_HANDLE_S speech = (SPEECH_CONTEXT_HANDLE_S)malloc(sizeof(SPEECH_CONTEXT_S));
	memcpy(&speech->params, params, sizeof(SPEECH_SERVICE_PARAM_S));
	uint32_t SampleRate = speech->params.SampleRate;
	uint32_t BitWidth = speech->params.BitWidth;
	uint32_t buffer_len = SampleRate * (BitWidth + 1) * SPEECH_BUFFER_SECOND;
	speech->tmpbuff = malloc(buffer_len);
	speech->shutdown = 0;
	speech->enable = speech->params.enable;
	speech->SpeechHandle = NULL;
	pthread_mutex_init(&speech->g_CryMutex, NULL);
	// ai init todo
	rc = Speech_Proc_Init(speech);
	AUDIO_SERVICR_ACAP_CallbackSet("Speech_Recognition", process_speech_audio_frame, speech);
	// start speech task
	rc = speech_start_task(speech);
	*hdl = (SPEECH_HANDLE_S *)speech;
	return rc;
}

int32_t SPEECH_SERVICE_Destroy(SPEECH_HANDLE_S hdl)
{
	int32_t rc = 0;
	g_pack_idx = 0;
	SPEECH_CONTEXT_HANDLE_S speech = (SPEECH_CONTEXT_HANDLE_S)hdl;
	// send shutdown to self
	speech->shutdown = 1;
	speech->enable = 0;
	// wait for exit
	while (!speech->shutdown) {
		OSAL_TASK_Sleep(20000);
	}
	CVI_LOGI("Speech Service destroy\n");
	pthread_mutex_destroy(&speech->g_CryMutex);
	// stop video task
	rc = speech_stop_task(speech);
	if (rc != 0) {
		CVI_LOGI("speech_stop_task fail\n");
	}
	rc = AUDIO_SERVICR_ACAP_CallbackUnset("Speech_Recognition");
	if (rc != 0) {
		CVI_LOGI("AUDIO_SERVICR_ACAP_CallbackUnset fail\n");
	}
	// ai deinit todo
	rc = CVI_TDL_DestroyHandle(speech->SpeechHandle);
	if (rc != 0) {
		CVI_LOGI("CVI_AI_DestroyHandle fail\n");
	}
	CVI_LOGI("Speech Service destroy end\n");
	if (speech != NULL) {
		if (speech->tmpbuff != NULL) {
			free(speech->tmpbuff);
			speech->tmpbuff = NULL;
		}
		free(speech);
		speech = NULL;
	}
	return rc;
}

int32_t SPEECH_SERVICE_StartSpeech(SPEECH_HANDLE_S hdl)
{
	if (hdl == NULL) {
		return -1;
	}
	SPEECH_CONTEXT_HANDLE_S speech = (SPEECH_CONTEXT_HANDLE_S)hdl;
	speech->enable = 1;
	return 0;
}

int32_t SPEECH_SERVICE_StopSpeech(SPEECH_HANDLE_S hdl)
{
	if (hdl == NULL) {
		return -1;
	}
	SPEECH_CONTEXT_HANDLE_S speech = (SPEECH_CONTEXT_HANDLE_S)hdl;
	speech->enable = 0;
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#include <sys/prctl.h>
#include <pthread.h>
#include "ui_common.h"
#include "osal.h"
#include "cvi_log.h"
#include "cvi_sysutils.h"

// static OSAL_TASK_HANDLE_S      ui_task;
static bool s_buiInit = false;

#ifndef CONFIG_SERVICES_LIVEVIEW_ON
typedef struct _VOICEPLAYER_AOUT_OPT_S {
	void *hAudDevHdl;	// device id
	void *hAudTrackHdl; // chn id
} VOICEPLAY_AOUT_OPT_S;

typedef struct _VOICEPLAY_VOICETABLE_S {
	uint32_t u32VoiceIdx;
	char aszFilePath[APPCOMM_MAX_PATH_LEN];
} VOICEPLAY_VOICETABLE_S;

/** voiceplay configuration */
typedef struct __VOICEPLAY_CFG_S {
	uint32_t u32MaxVoiceCnt;
	VOICEPLAY_VOICETABLE_S *pstVoiceTab;
	VOICEPLAY_AOUT_OPT_S stAoutOpt;
} VOICEPLAY_CFG_S;
#endif

static void UI_VOLQUEUE_Init(void)
{
#ifdef SERVICES_LIVEVIEW_ON
	MEDIA_SYSHANDLE_S SysHandle = MEDIA_GetCtx()->SysHandle;
	VOICEPLAY_CFG_S stVoicePlayCfg = {0};

	stVoicePlayCfg.stAoutOpt.hAudDevHdl = SysHandle.aohdl;

	stVoicePlayCfg.u32MaxVoiceCnt = UI_VOICE_MAX_NUM;
	VOICEPLAY_VOICETABLE_S astVoiceTab[UI_VOICE_MAX_NUM] =
		{
			{UI_VOICE_START_UP_IDX, UI_VOICE_START_UP_SRC},
			{UI_VOICE_TOUCH_BTN_IDX, UI_VOICE_TOUCH_BTN_SRC},
			{UI_VOICE_CLOSE_IDX, UI_VOICE_CLOSE_SRC},
			{UI_VOICE_PHOTO_IDX, UI_VOICE_PHOTO_SRC},
		};
	stVoicePlayCfg.pstVoiceTab = astVoiceTab;
	int32_t s32Ret = VOICEPLAY_Init(&stVoicePlayCfg);
	if (s32Ret != 0) {
		CVI_LOGE("VOICEPLAY_Init failed!\n");
	}
#endif
}

int32_t UIAPP_Start(void)
{
	printf("-------------------------------------------------UIAPP_Start");
	if (s_buiInit == false) {
		ui_common_SubscribeEvents();
		UI_VOLQUEUE_Init();
		s_buiInit = true;
	} else {
		CVI_LOGI("ui already init\n");
	}

	return 0;
}

int32_t UIAPP_Stop(void)
{
	if (s_buiInit == true) {
		s_buiInit = false;
	}

	return 0;
}

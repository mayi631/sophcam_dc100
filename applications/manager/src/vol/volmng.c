#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <endian.h>
#include <pthread.h>
#include <signal.h>
#include <cvi_audio.h>
#include <sys/prctl.h>
#include <errno.h>
#include "osal.h"
#include "sysutils_queue.h"
#include "volmng.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT 0x20746d66
#define ID_DATA 0x61746164
#define VOICE_QUEUE_MAXLEN (5)
#define VOICE_TIME_INTERVAL_MS (500)

typedef struct tagVOICEPLAY_CTX_S {
	pthread_t taskid;
	pthread_mutex_t mutex;
	pthread_mutex_t playCrtlMutex;
	pthread_cond_t playCrtlCond;
	sem_t volSem;
	VOICEPLAY_CFG_S stCfg;
	QUEUE_HANDLE_T queueHdl;
	bool bRunFlag;
	bool bAmplifier;
	bool bAutoMuteEnable;  /* 是否在播放完后自动静音 */
} VOICEPLAY_CTX_S;

struct riff_wave_header {
	uint32_t riff_id;
	uint32_t riff_sz;
	uint32_t wave_id;
};

struct chunk_header {
	uint32_t id;
	uint32_t sz;
};

struct chunk_fmt {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
};
typedef struct _AO_Chn_Info_ {
	MAPI_AO_ATTR_S stAoAttr;
	MAPI_AO_HANDLE_T AoHdl;
	int32_t AoChn;
	char *pBuffer;
	int32_t buffersize;
	FILE *pfile;
        int32_t volume;
} AO_CHN_Info;

static VOICEPLAY_CTX_S s_stVOICEPLAYCtx = {
    .taskid = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .playCrtlMutex = PTHREAD_MUTEX_INITIALIZER,
    .playCrtlCond = PTHREAD_COND_INITIALIZER,
    .queueHdl = 0,
    .bRunFlag = 0,
    .bAmplifier = true,
};

static bool checkname_iswav(const char* infilename)
{
    int32_t s32InputFileLen = 0;

    s32InputFileLen = strlen(infilename);

    if (s32InputFileLen == 0) {
        CVI_LOGE("No Input File Name..force return\n");
        return false;
    }

    if (s32InputFileLen >= 4 && strcasecmp(&infilename[s32InputFileLen - 4], ".wav") == 0) {
        return true;
    }

    return false;
}

static FILE *audio_open_wavfile(const char *filename, int32_t *channels, int32_t *sample_rate)
{
	FILE *file;
	struct riff_wave_header riff_wave_header;
	struct chunk_header chunk_header;
	struct chunk_fmt chunk_fmt;
	int32_t more_chunks = 1;

	file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Unable to open file '%s'\n", filename);
		return NULL;
	}

	fread(&riff_wave_header, sizeof(riff_wave_header), 1, file);
	if ((riff_wave_header.riff_id != ID_RIFF) ||
		(riff_wave_header.wave_id != ID_WAVE)) {
		fprintf(stderr, "Error: '%s' is not a riff/wave file\n", filename);
		fclose(file);
		return NULL;
	}

	do {
		fread(&chunk_header, sizeof(chunk_header), 1, file);

		switch (chunk_header.id) {
                case ID_FMT:
                    fread(&chunk_fmt, sizeof(chunk_fmt), 1, file);
                    *sample_rate = chunk_fmt.sample_rate;
                    CVI_LOGD("chunk_fmt.num_channels = %d\n", chunk_fmt.num_channels);
                    *channels = chunk_fmt.num_channels;
                    /* If the format header is larger, skip the rest */
                    if (chunk_header.sz > sizeof(chunk_fmt))
                        fseek(file, chunk_header.sz - sizeof(chunk_fmt), SEEK_CUR);
                    break;
                case ID_DATA:
			/* Stop looking for chunks */
			more_chunks = 0;
			chunk_header.sz = le32toh(chunk_header.sz);
			break;
		default:
			/* Unknown chunk, skip bytes */
			fseek(file, chunk_header.sz, SEEK_CUR);
		}
	} while (more_chunks);
	return file;
}

int32_t VOICEPLAY_Push(const VOICEPLAY_VOICE_S* pstVoice, int32_t u32Timeout_ms)
{
    int32_t s32Ret = -1;
    int32_t u32Timewait_ms = 0;
    int32_t queue_len = 0;
    APPCOMM_CHECK_POINTER(pstVoice, VOLMNG_EINVAL);
    // MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

    queue_len = QUEUE_GetLen(s_stVOICEPLAYCtx.queueHdl);
    CVI_LOGD("voice play queue len: %d\n", queue_len);
    if (queue_len == 0) {
        /*if queue is empty ,push voice into queue*/
        s32Ret = QUEUE_Push((QUEUE_HANDLE_T)s_stVOICEPLAYCtx.queueHdl, pstVoice);
        sem_post(&s_stVOICEPLAYCtx.volSem);
    } else if (queue_len > 0) {
        /*if queue is not empty ,drop droppable voice*/
        if (pstVoice->bDroppable == true) {
            CVI_LOGW("voice queue is busy, drop this voice, queue_len=%d\n", queue_len);
            s32Ret = -1;
        } else {
            /*wait queue is not full*/
            while ((u32Timewait_ms < u32Timeout_ms) && (VOICE_QUEUE_MAXLEN == queue_len)) {
                CVI_LOGI("voice queue is full, Waiting time: %u....\n", u32Timewait_ms);
                OSAL_TASK_Sleep(VOICE_TIME_INTERVAL_MS * 1000);
                u32Timewait_ms += VOICE_TIME_INTERVAL_MS;
            }
            s32Ret = QUEUE_Push((QUEUE_HANDLE_T)s_stVOICEPLAYCtx.queueHdl, pstVoice);
            sem_post(&s_stVOICEPLAYCtx.volSem);
        }
    }
    // MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);

    return s32Ret;
}

static char* VOICEPLAY_GetPath(uint32_t u32Index)
{
	for (uint32_t i = 0; i < s_stVOICEPLAYCtx.stCfg.u32MaxVoiceCnt; i++) {
		if (u32Index == s_stVOICEPLAYCtx.stCfg.pstVoiceTab[i].u32VoiceIdx) {
			return s_stVOICEPLAYCtx.stCfg.pstVoiceTab[i].aszFilePath;
		}
	}
	return NULL;
}

static int32_t VOICE_PLAY_File(const char *pathname, VOICEPLAY_CFG_S *pvParam)
{
	APPCOMM_CHECK_POINTER(pathname, VOLMNG_EINTER);
	APPCOMM_CHECK_POINTER(pvParam, VOLMNG_EINTER);

	int32_t s32Ret = 0;
	int32_t s32FrameBytes = 0, s32CountDelay = 0;
	FILE *pfile = {NULL};
	char *inputfile = NULL;
	AUDIO_FRAME_S stFrame;
	uint32_t u32DataLen = 0;

	VOICEPLAY_CFG_S *pstCfg = (VOICEPLAY_CFG_S *)pvParam;
	MAPI_AO_CTX_T *et = (MAPI_AO_CTX_T *)pstCfg->stAoutOpt.hAudDevHdl;

	/*1.deal with the wav file*/
	if (checkname_iswav(pathname)) {
		inputfile = (char *)pathname;
	} else {
		CVI_LOGE("it is not wav file\n");
		return false;
	}

	pfile = audio_open_wavfile(inputfile, (int32_t *)&et->attr.channels, (int32_t *)&et->attr.enSampleRate);
	if (pfile == NULL) {
		CVI_LOGE("OPEN FILE ERR\n");
                return false;
        }

        CVI_LOGD("voice play file: %s\n", pathname);
        /*2.malloc men for Ao*/
        s32FrameBytes = et->attr.u32PtNumPerFrm * et->attr.channels * 2;
        uint8_t* pBuffer = malloc(s32FrameBytes); // 2ch 16bit 160 samples
        stFrame.u64VirAddr[0] = pBuffer;
        if (!stFrame.u64VirAddr[0]) {
            CVI_LOGE("unable to allocate s32FrameBytes\n");
            free(pBuffer);
            return false;
        }

        MAPI_AO_Start((void*)pstCfg->stAoutOpt.hAudDevHdl, 1);
        // MAPI_AO_SetVolume((void *)pstCfg->stAoutOpt.hAudDevHdl, VOLUME_DEFAULT);

        /*3.open the amplifier*/
        MAPI_AO_SetAmplifier((void*)pstCfg->stAoutOpt.hAudDevHdl, true);

        /*4.begin play the wav file*/
        memset(pBuffer, 0, s32FrameBytes);
        while (fread(pBuffer, 1, s32FrameBytes, pfile)) {
		stFrame.u32Len = s32FrameBytes / (et->attr.channels * 2);
		stFrame.u64TimeStamp = 0;
		stFrame.enSoundmode = (et->attr.channels == 2) ? 1 : 0;
		stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;

		s32Ret = MAPI_AO_SendFrame((void *)pstCfg->stAoutOpt.hAudDevHdl, 1, &stFrame, 1000);
		if (s32Ret) {
			CVI_LOGE("MAPI_AO_SendFrame failed!\n");
			continue;
		}
		s32CountDelay++;
		u32DataLen += stFrame.u32Len;
	}
	// printf("s32CountDelay === %d\n", (s32CountDelay * 39 - 204));
	// delay close power amplifier
	// double transfer = (et->attr.enSampleRate * ((et->attr.enSampleRate / 1000) / 8));
	// double AudioTotalduration = (u32DataLen / transfer);
	// double Aosleeptime = (AudioTotalduration * 1000);
	// usleep(Aosleeptime * 1000);
#ifdef SERVICES_ADAS_ON
	usleep(350 * 1000);
#endif

	/*5.destructor*/
	if (pfile) {
		fclose(pfile);
	}
        free(pBuffer);
        MAPI_AO_Stop((void*)pstCfg->stAoutOpt.hAudDevHdl, 1);

        // if (s_stVOICEPLAYCtx.bAmplifier == true){
        //     MAPI_AO_SetAmplifier((void *)pstCfg->stAoutOpt.hAudDevHdl, false);
        // }

        return 0;
}

static void* VOICEPLAY_TaskQueue_Proc(void* pvParam)
{
	int32_t s32Ret = 0;
	prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
	QUEUE_HANDLE_T queue = s_stVOICEPLAYCtx.queueHdl;
	VOICEPLAY_CFG_S *p_vol_play = (VOICEPLAY_CFG_S *)pvParam;

	while (s_stVOICEPLAYCtx.bRunFlag) {
		while ((0 != sem_wait(&s_stVOICEPLAYCtx.volSem)) && (errno == EINTR))
			;
		/*if not msg ,the thread will block*/
		if (0 == QUEUE_GetLen(queue)) {
			// cvi_usleep(VOICE_TIME_INTERVAL_MS * 1000);
			continue;
		} else {
			VOICEPLAY_VOICE_S stVoice;
			s32Ret = QUEUE_Pop(queue, &stVoice);
			if (0 != s32Ret) {
				CVI_LOGE("QUEUE_Pop failed\n");
				continue;
			}
			MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);
			/*get pathname from array*/
			for (uint32_t i = 0; i < stVoice.u32VoiceCnt; i++) {
				VOICE_PLAY_File(VOICEPLAY_GetPath(stVoice.au32VoiceIdx[i]), p_vol_play);
			}
			MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
		}
	}

	return NULL;
}

int32_t VOICEPLAY_Init(const VOICEPLAY_CFG_S *pstCfg)
{
	int32_t s32Ret = 0;
	APPCOMM_CHECK_POINTER(pstCfg, VOLMNG_EINVAL);
	APPCOMM_CHECK_EXPR(pstCfg->u32MaxVoiceCnt > 0, VOLMNG_EINVAL);
	MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

	/*return volpaly queue handler*/
	s_stVOICEPLAYCtx.queueHdl = QUEUE_Create(sizeof(VOICEPLAY_VOICE_S), VOICE_QUEUE_MAXLEN);
	if (s_stVOICEPLAYCtx.queueHdl == 0) {
		MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
		CVI_LOGE("QUEUE_Create failed\n");
		return APP_EINTER;
	}

	s_stVOICEPLAYCtx.bRunFlag = true;

	memcpy(&s_stVOICEPLAYCtx.stCfg, pstCfg, sizeof(VOICEPLAY_CFG_S));

	sem_init(&s_stVOICEPLAYCtx.volSem, 0, 0);

	s32Ret = pthread_create(&s_stVOICEPLAYCtx.taskid, NULL, VOICEPLAY_TaskQueue_Proc, (void *)&s_stVOICEPLAYCtx.stCfg);
	if (s32Ret != 0) {
		MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
		CVI_LOGE("CVI_pthread_create failed\n");
		return APP_EINTER;
	}

	/*pstCfg->u32MaxVoiceCnt need value from call*/
	s_stVOICEPLAYCtx.stCfg.pstVoiceTab = (VOICEPLAY_VOICETABLE_S *)malloc(pstCfg->u32MaxVoiceCnt * sizeof(VOICEPLAY_VOICETABLE_S));
	if (NULL == s_stVOICEPLAYCtx.stCfg.pstVoiceTab) {
		MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
		return APP_EINTER;
	}

	for (uint32_t i = 0; i < pstCfg->u32MaxVoiceCnt; i++) {
		s_stVOICEPLAYCtx.stCfg.pstVoiceTab[i] = pstCfg->pstVoiceTab[i];
	}

	MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
	return 0;
}

int32_t VOICEPLAY_DeInit(void)
{
	MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

	s_stVOICEPLAYCtx.bRunFlag = false;
	sem_post(&s_stVOICEPLAYCtx.volSem);
	sem_destroy(&s_stVOICEPLAYCtx.volSem);
	pthread_join(s_stVOICEPLAYCtx.taskid, NULL);

	CVI_APPCOMM_SAFE_FREE(s_stVOICEPLAYCtx.stCfg.pstVoiceTab);
	QUEUE_Destroy(s_stVOICEPLAYCtx.queueHdl);

	MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
	return 0;
}

int32_t VOICEPLAY_SetAmplifier(bool en)
{
	MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

	if (en == true) {
		MAPI_AO_SetAmplifier(s_stVOICEPLAYCtx.stCfg.stAoutOpt.hAudDevHdl, false);
	} else {
		MAPI_AO_SetAmplifier(s_stVOICEPLAYCtx.stCfg.stAoutOpt.hAudDevHdl, true);
	}

	s_stVOICEPLAYCtx.bAmplifier = en;

	MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);

	return 0;
}

int32_t VOICEPLAY_SetAmplifierFlage(bool en)
{
	MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

	s_stVOICEPLAYCtx.bAmplifier = en;

	MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);

	return 0;
}

int32_t VOICEPLAY_SetVolume(int32_t volume)
{
	int32_t s32Ret = 0;
	MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

	s32Ret = MAPI_AO_SetVolume(s_stVOICEPLAYCtx.stCfg.stAoutOpt.hAudDevHdl, volume);
	if (s32Ret != 0) {
		CVI_LOGE("MAPI_AO_SetVolume failed\n");
		MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
		return APP_EINTER;
	}

	MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);

	return 0;
}

int32_t VOICEPLAY_GetVolume(int32_t *volume)
{
	int32_t s32Ret = 0;
	MUTEX_LOCK(s_stVOICEPLAYCtx.mutex);

	s32Ret = MAPI_AO_GetVolume(s_stVOICEPLAYCtx.stCfg.stAoutOpt.hAudDevHdl, volume);
	if (s32Ret != 0) {
		CVI_LOGE("MAPI_AO_SetVolume failed\n");
		MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);
		return APP_EINTER;
	}

	MUTEX_UNLOCK(s_stVOICEPLAYCtx.mutex);

        return 0;
}

#include<stdio.h>
#include<pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

#include "mapi_aenc.h"
#include "cvi_audio.h"
#include "cvi_log.h"
#include "osal.h"
#include "mapi.h"
#include "audio_aac_adp.h"
#include "audio_dl_adp.h"

#define AENC_TASK_TIMEOUT       (2000)//ms

extern void AUDIO_INIT_ONCE(void);
#define AENC_CHN_MAX (32)
static int g_aenc_chn_used[AENC_CHN_MAX] = {0};
static pthread_mutex_t g_aenc_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MAX_AENC_CALLBACK_COUNT (5)
typedef struct MAPI_AENC_CTX_S {
    MAPI_AENC_ATTR_S           attr;
    MAPI_AENC_CALLBACK_S    *p_aenc_cb[MAX_AENC_CALLBACK_COUNT];
    OSAL_TASK_HANDLE_S      cb_task;
    sem_t                        cb_sem;
    OSAL_MUTEX_HANDLE_S    cb_mutex;
    int                           bLock;
    volatile int                   quit;
    int                         enc_chn;
    int                       bBindAcap;
    MAPI_ACAP_HANDLE_T      AcapHdl;
    int                           bMute;
    int                        bWorking;
    int                           bInit;
    pthread_mutex_t          aenc_mutex;
    int                     AencAac_chn;
    int                    bRegisterAac;
    int                       bAacencode;
    AENC_AAC_INFO_S        stAencAacInfo;
} MAPI_AENC_CTX_T;

static int getValidAudioEncChanIndex(void)
{
    int i;
    for(i = 0; i < AENC_CHN_MAX; i++) {
        if(g_aenc_chn_used[i] == 0)
            return i;
    }
    return -1;
}

int MAPI_AENC_Init(MAPI_AENC_HANDLE_T *AencHdl, const MAPI_AENC_ATTR_S *pstAencAttr)
{
    int ret = 0;
    MAPI_AENC_CTX_T *pstAecCtx;
    AENC_CHN_ATTR_S stAencAttr;
    AENC_ATTR_G711_S stAencG711;
    AENC_ATTR_AAC_S  stAencAac;

    pstAecCtx = (MAPI_AENC_CTX_T *)malloc(sizeof(MAPI_AENC_CTX_T));
    if (!pstAecCtx) {
        CVI_LOGE("malloc failed\n");
        return MAPI_ERR_NOMEM;
    }

    memset(pstAecCtx, 0, sizeof(MAPI_AENC_CTX_T));
    memcpy(&pstAecCtx->attr,pstAencAttr,sizeof(MAPI_AENC_ATTR_S));

    AUDIO_INIT_ONCE();

    pthread_mutex_lock(&g_aenc_mutex);
    pstAecCtx->enc_chn = getValidAudioEncChanIndex();
    pthread_mutex_unlock(&g_aenc_mutex);
    if(pstAecCtx->enc_chn < 0) {
        CVI_LOGE("get enc chan failed\n");
        ret= MAPI_ERR_FAILURE;
        goto err;
    }
    g_aenc_chn_used[pstAecCtx->enc_chn] = 1;

    if(pstAencAttr->enAencFormat == MAPI_AUDIO_CODEC_G711U) {
        stAencAttr.enType = PT_G711U;
        stAencAttr.pValue       = &stAencG711;
    }else if(pstAencAttr->enAencFormat == MAPI_AUDIO_CODEC_G711A) {
        stAencAttr.enType = PT_G711A;
        stAencAttr.pValue       = &stAencG711;
    }else if(pstAencAttr->enAencFormat == MAPI_AUDIO_CODEC_AAC) {

        pstAecCtx->bAacencode =1;

        stAencAttr.enType = PT_AAC;


		stAencAac.enBitWidth = AUDIO_BIT_WIDTH_16;
		stAencAac.enSmpRate = pstAencAttr->src_samplerate;
		if(pstAencAttr->src_samplerate >= 48000)
			stAencAac.enBitRate = AAC_BPS_64K;
		else
			stAencAac.enBitRate = AAC_BPS_32K;

		//stAencAac.enSoundMode = bVqe?AUDIO_SOUND_MODE_MONO:pstAencAttr->channels;//--------vbqe
		stAencAac.enSoundMode = pstAencAttr->channels -1;
        stAencAac.enTransType = AAC_TRANS_TYPE_ADTS;
		stAencAac.s16BandWidth = 0;
        stAencAac.enAACType = AAC_TYPE_AACLC;
		stAencAttr.pValue = &stAencAac;
		MAPI_MPI_AENC_AacInit();

    }else {
        CVI_LOGE("not support yet.\n");
        ret = MAPI_ERR_FAILURE;
        goto err;
    }

    stAencAttr.bFileDbgMode = false;
    stAencAttr.u32BufSize = 30;
    stAencAttr.u32PtNumPerFrm = pstAencAttr->u32PtNumPerFrm;
    pstAecCtx->stAencAacInfo.sample_rate = pstAencAttr->src_samplerate;


    ret =  CVI_AENC_CreateChn(pstAecCtx->enc_chn,&stAencAttr);
    if (ret != CVI_SUCCESS) {
         CVI_LOGE("aenc creat chn fail\n");
        ret = MAPI_ERR_FAILURE;
        goto err;
    }


    pthread_mutex_init(&pstAecCtx->aenc_mutex, NULL);
    pstAecCtx->bInit = true;
    *AencHdl = (MAPI_AENC_HANDLE_T)pstAecCtx;
    return MAPI_SUCCESS;
err:
    pstAecCtx->bInit = false;
    g_aenc_chn_used[pstAecCtx->enc_chn] = 0;
    return ret;
}

int MAPI_AENC_Deinit(MAPI_AENC_HANDLE_T AencHdl)
{
    int ret = MAPI_SUCCESS;
    //uint32_t timeout_cnt = 0;
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    pstAencCtx->quit = 1;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    CVI_AENC_DestroyChn(pstAencCtx->enc_chn);

    if (pstAencCtx->bAacencode == 1)
        MAPI_MPI_AENC_AacDeInit();

    g_aenc_chn_used[pstAencCtx->enc_chn] = 0;
    free(pstAencCtx);
    return ret;
}

int MAPI_AENC_Start(MAPI_AENC_HANDLE_T AencHdl)
{
#if 0
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&pstAencCtx->aenc_mutex);

    pstAencCtx->bWorking = 1;
    printf("pstAencCtx->AcapHdl  = %p,pstAencCtx->bBindAcap = %d\n",pstAencCtx->AcapHdl,pstAencCtx->bBindAcap);
    if(pstAencCtx->bBindAcap && pstAencCtx->AcapHdl) {
        printf("start aaaaaaaaaaaaaaaaaaaaaaa\n");
        MAPI_ACAP_Start(pstAencCtx->AcapHdl);
    }
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
#endif
    UNUSED(AencHdl);
    return MAPI_SUCCESS;
}

int MAPI_AENC_Stop(MAPI_AENC_HANDLE_T AencHdl)
{
#if 0
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    pstAencCtx->bWorking = 0;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
#endif
    UNUSED(AencHdl);
    return MAPI_SUCCESS;
}

int MAPI_AENC_SetMute(MAPI_AENC_HANDLE_T AencHdl, int bEnable)
{
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    pstAencCtx->bMute = bEnable;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    return MAPI_SUCCESS;
}

int MAPI_AENC_GetMute(MAPI_AENC_HANDLE_T AencHdl, int *pbEnable)
{
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    *pbEnable = pstAencCtx->bMute;
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
    return MAPI_SUCCESS;
}

int MAPI_AENC_RegisterCallback(MAPI_AENC_HANDLE_T AencHdl, const MAPI_AENC_CALLBACK_S* pstAencCB)
{
    int ret = MAPI_SUCCESS;
    int i;
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;

    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    for(i = 0; i < MAX_AENC_CALLBACK_COUNT; i++) {
        if(!pstAencCtx->p_aenc_cb[i]) {
            pstAencCtx->p_aenc_cb[i] = (MAPI_AENC_CALLBACK_S* )pstAencCB;
            break;
        }
    }
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    return ret;
}

int MAPI_AENC_UnregisterCallback(MAPI_AENC_HANDLE_T AencHdl, const MAPI_AENC_CALLBACK_S* pstAencCB)
{

    int ret = MAPI_SUCCESS;
    int i;
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;

    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    for(i = 0; i < MAX_AENC_CALLBACK_COUNT; i++) {
        if(pstAencCtx->p_aenc_cb[i] && pstAencCtx->p_aenc_cb[i] == pstAencCB) {
            pstAencCtx->p_aenc_cb[i] = NULL;
            CVI_LOGI("unregister suc.\n");
            break;
        }
    }
    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);

    return ret;

}

int MAPI_AENC_BindACap(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AeDev, AENC_CHN AeChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32DevId = AiDev;
	stSrcChn.s32ChnId = AiChn;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = 0;
	stDestChn.s32ChnId = AeChn;

    UNUSED(AeDev);
	return CVI_AUD_SYS_Bind(&stSrcChn, &stDestChn);
}

int MAPI_AENC_UnbindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_AI;
	stSrcChn.s32ChnId = AiChn;
	stSrcChn.s32DevId = AiDev;
	stDestChn.enModId = CVI_ID_AENC;
	stDestChn.s32DevId = AeDev;
	stDestChn.s32ChnId = AeChn;


	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);
}

int MAPI_AENC_SendFrame(MAPI_AENC_HANDLE_T AencHdl, const AUDIO_FRAME_S* pstFrm,AEC_FRAME_S* pstAecFrm)
{
    int ret = MAPI_SUCCESS;

    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAencCtx->aenc_mutex);
    ret = CVI_AENC_SendFrame(pstAencCtx->enc_chn, pstFrm,pstAecFrm);
    if(ret != CVI_SUCCESS) {
        CVI_LOGE("AENC_SendFrame failed.ret:%#x\n",ret);
        pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
        return ret;
    }

    pthread_mutex_unlock(&pstAencCtx->aenc_mutex);
    return ret;
}

int MAPI_AENC_GetStream(MAPI_AENC_HANDLE_T AencHdl, AUDIO_STREAM_S *pstStream,CVI_S32 s32MilliSec)
{
    MAPI_AENC_CTX_T *pstAencCtx = (MAPI_AENC_CTX_T *)AencHdl;
    if(!pstAencCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    return CVI_AENC_GetStream(pstAencCtx->enc_chn,pstStream,s32MilliSec);
}



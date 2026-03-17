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

#include "mapi_adec.h"
#include "cvi_audio.h"
#include "cvi_log.h"
#include "osal.h"
#include "mapi.h"
#include "audio_aac_adp.h"


typedef struct MAPI_ADEC_CTX_S {
    MAPI_ADEC_ATTR_S           attr;
    int                         dec_chn;
    int                           bMute;
    int                        bWorking;
    int                           bInit;
    AUDIO_FRAME_S               *pstFrame;
    pthread_mutex_t          adec_mutex;
    int                      bAacdecode;
} MAPI_ADEC_CTX_T;

extern void AUDIO_INIT_ONCE(void);
extern CVI_S32 MAPI_MPI_ADEC_AacInit(void);

extern CVI_S32 MAPI_MPI_ADEC_AacDeInit(void);
#define ADEC_CHN_MAX (1) //mw sdk暂不支持多个decodec同时进行
static int g_adec_chn_used[ADEC_CHN_MAX] = {0};
static pthread_mutex_t g_adec_mutex = PTHREAD_MUTEX_INITIALIZER;

static int getValidAudioDecChanIndex(void)
{
    int i;
    for(i = 0; i < ADEC_CHN_MAX; i++) {
        if(g_adec_chn_used[i] == 0)
            return i;
    }
    return -1;
}

void dumpdata(const char *filename,char *buffer, unsigned int len_byte)
{
    if(!filename) {
        return;
    }
    FILE *fp = fopen(filename, "ab");
    if(!fp) {
        return;
    }
    fwrite(buffer,len_byte, 1,fp);
    fclose(fp);
}
int MAPI_ADEC_Init(MAPI_ADEC_HANDLE_T *AdecHdl, const MAPI_ADEC_ATTR_S* pstAdecAttr)
{
    MAPI_ADEC_CTX_T *pstAdecCtx;
    ADEC_CHN_ATTR_S stAdecAttr;
    ADEC_ATTR_G711_S stAdecG711;
    ADEC_ATTR_AAC_S stAdecAac;
    int ret = MAPI_SUCCESS;

    pstAdecCtx = (MAPI_ADEC_CTX_T *)malloc(sizeof(MAPI_ADEC_CTX_T));
    if (!pstAdecCtx) {
        CVI_LOGE("malloc failed\n");
        return MAPI_ERR_NOMEM;
    }
    memset(&stAdecAttr, 0, sizeof(stAdecAttr));
    memset(pstAdecCtx, 0, sizeof(MAPI_ADEC_CTX_T));
    memcpy(&pstAdecCtx->attr,pstAdecAttr,sizeof(MAPI_ADEC_ATTR_S));

    pstAdecCtx->pstFrame = (AUDIO_FRAME_S *)malloc(sizeof(AUDIO_FRAME_S));
    if (!pstAdecCtx->pstFrame) {
        CVI_LOGE("malloc failed\n");
        return MAPI_ERR_NOMEM;
    }

    pthread_mutex_lock(&g_adec_mutex);
    pstAdecCtx->dec_chn = getValidAudioDecChanIndex();
    pthread_mutex_unlock(&g_adec_mutex);
    if(pstAdecCtx->dec_chn < 0) {
        CVI_LOGE("get dec chan failed\n");
        ret= MAPI_ERR_FAILURE;
        goto err;
    }
    g_adec_chn_used[pstAdecCtx->dec_chn] = 1;

    stAdecAttr.s32Sample_rate = pstAdecAttr->enSamplerate;
    stAdecAttr.s32ChannelNums = pstAdecAttr->enSoundmode + 1;
    stAdecAttr.u32BufSize = 20;
    stAdecAttr.enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */
    stAdecAttr.bFileDbgMode = CVI_TRUE;
    stAdecAttr.s32BytesPerSample = 2;
    stAdecAttr.s32frame_size = pstAdecAttr->frame_size;//1024

    if(pstAdecAttr->enAdecFormat == MAPI_AUDIO_CODEC_G711U) {
        stAdecAttr.enType = PT_G711U;
        stAdecAttr.pValue       = &stAdecG711;
    }else if(pstAdecAttr->enAdecFormat == MAPI_AUDIO_CODEC_G711A) {
        stAdecAttr.enType = PT_G711A;
        stAdecAttr.pValue       = &stAdecG711;

    }else if(pstAdecAttr->enAdecFormat == MAPI_AUDIO_CODEC_AAC) {
        pstAdecCtx->bAacdecode =1;
        stAdecAttr.enType = PT_AAC;
        stAdecAac.enTransType = AAC_TRANS_TYPE_ADTS;
		stAdecAac.enSoundMode = pstAdecAttr->enSoundmode;
		stAdecAac.enSmpRate = pstAdecAttr->enSamplerate;
		stAdecAttr.pValue = &stAdecAac;

		stAdecAttr.s32frame_size = AACLC_SAMPLES_PER_FRAME;//1024


        if (MAPI_MPI_ADEC_AacInit()) {
            printf("[error] aac init  faile\n");
            return -1;
        }

    }else {
        CVI_LOGE("not support yet.\n");
        ret = MAPI_ERR_FAILURE;
        goto err;
    }

    AUDIO_INIT_ONCE();
    CVI_LOGI("stAdecAttr.s32frame_size=%d\n",stAdecAttr.s32frame_size);
    CVI_ADEC_CreateChn(pstAdecCtx->dec_chn,&stAdecAttr);
    pthread_mutex_init(&pstAdecCtx->adec_mutex, NULL);

    memset(pstAdecCtx->pstFrame,0,sizeof(AUDIO_FRAME_S));

    pstAdecCtx->bInit = true;
    *AdecHdl = (MAPI_ADEC_HANDLE_T)pstAdecCtx;

    return MAPI_SUCCESS;
err:
    pstAdecCtx->bInit = false;
    pthread_mutex_lock(&g_adec_mutex);
    g_adec_chn_used[pstAdecCtx->dec_chn] = 0;
    pthread_mutex_unlock(&g_adec_mutex);
    return ret;

}

int MAPI_ADEC_Deinit(MAPI_ADEC_HANDLE_T AdecHdl)
{
    MAPI_ADEC_CTX_T *pstAdecCtx = (MAPI_ADEC_CTX_T *)AdecHdl;
    if(!pstAdecCtx) {
        return MAPI_ERR_INVALID;
    }

    CVI_ADEC_DestroyChn(pstAdecCtx->dec_chn);
    if (pstAdecCtx->bAacdecode)
        MAPI_MPI_ADEC_AacDeInit();

    pthread_mutex_lock(&g_adec_mutex);
    g_adec_chn_used[pstAdecCtx->dec_chn] = 0;
    pthread_mutex_unlock(&g_adec_mutex);
    free(pstAdecCtx->pstFrame);
    free(pstAdecCtx);

    return MAPI_SUCCESS;
}
int MAPI_ADEC_SendStream(MAPI_ADEC_HANDLE_T AdecHdl, const AUDIO_STREAM_S* pstAdecStream, int bBlock)
{
    MAPI_ADEC_CTX_T *pstAdecCtx = (MAPI_ADEC_CTX_T *)AdecHdl;
    if(!pstAdecCtx) {
        return MAPI_ERR_INVALID;
    }
    return CVI_ADEC_SendStream(pstAdecCtx->dec_chn, pstAdecStream, bBlock);
}
int MAPI_ADEC_SendEndOfStream(MAPI_ADEC_HANDLE_T AdecHdl)
{
    MAPI_ADEC_CTX_T *pstAdecCtx = (MAPI_ADEC_CTX_T *)AdecHdl;
    if(!pstAdecCtx) {
        return MAPI_ERR_INVALID;
    }

    return CVI_ADEC_SendEndOfStream(pstAdecCtx->dec_chn, CVI_FALSE);
}

int MAPI_ADEC_GetFrame(MAPI_ADEC_HANDLE_T AdecHdl, MAPI_AUDIO_FRAME_INFO_S* pstAudioFrameInfo, int bBlock)
{
    int ret;
    MAPI_ADEC_CTX_T *pstAdecCtx = (MAPI_ADEC_CTX_T *)AdecHdl;
    if(!pstAdecCtx) {
        return MAPI_ERR_INVALID;
    }

    if(!pstAudioFrameInfo->stAudFrameInfo.pstFrame) {
        pstAudioFrameInfo->stAudFrameInfo.pstFrame = pstAdecCtx->pstFrame;
    }

    ret = CVI_ADEC_GetFrame(pstAdecCtx->dec_chn, &pstAudioFrameInfo->stAudFrameInfo, bBlock);
    if(ret != CVI_SUCCESS) {
        CVI_LOGE("ADEC_GetFrame failed,ret:%d.\n",ret);
        return ret;
    }
    pstAudioFrameInfo->pstream = (void *)pstAudioFrameInfo->stAudFrameInfo.pstFrame->u64VirAddr[0];
    pstAudioFrameInfo->u32Len = pstAudioFrameInfo->stAudFrameInfo.pstFrame->u32Len;
    pstAudioFrameInfo->u64Timestamp = pstAudioFrameInfo->stAudFrameInfo.pstFrame->u64TimeStamp;

    return MAPI_SUCCESS;
}
int MAPI_ADEC_ReleaseFrame(MAPI_ADEC_HANDLE_T AdecHdl, const MAPI_AUDIO_FRAME_INFO_S* pstAudioFrameInfo)
{
    return MAPI_SUCCESS;
    int ret = MAPI_SUCCESS;
    MAPI_ADEC_CTX_T *pstAdecCtx = (MAPI_ADEC_CTX_T *)AdecHdl;
    if(!pstAdecCtx) {
        return MAPI_ERR_INVALID;
    }

    //ret = CVI_ADEC_ReleaseFrame(pstAdecCtx->dec_chn,&pstAudioFrameInfo->stAudFrameInfo);
    if(ret != CVI_SUCCESS) {
        CVI_LOGE("ADEC_ReleaseFrame failed,ret:%d.\n",ret);
        return ret;
    }

    UNUSED(pstAudioFrameInfo);
    return ret;
}


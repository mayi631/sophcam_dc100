#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>

#include <cvi_type.h>
#include <cvi_comm_aio.h>
#include "mapi_ao.h"
#include "cvi_audio.h"
#include "cvi_log.h"
// #include "acodec.h"
#include "mapi_internal.h"
#include "mapi_define.h"
#include "osal.h"
#include "mapi_adec.h"
#include "hal_gpio.h"

typedef struct MAPI_ADEC_CTX_S {
    MAPI_ADEC_ATTR_S           attr;
    int                         dec_chn;
    int                           bMute;
    int                        bWorking;
    int                           bInit;
    AUDIO_FRAME_S               *pstFrame;
    pthread_mutex_t          adec_mutex;
} MAPI_ADEC_CTX_T;

static MAPI_AO_CTX_T *g_ao_hdl;
extern void AUDIO_INIT_ONCE(void);

int MAPI_AO_GetHandle(MAPI_AO_HANDLE_T *AoHdl)
{
    if (!g_ao_hdl) {
        CVI_LOGE("Please MAPI_AO_Init first");
        return MAPI_ERR_FAILURE;
    }
    *AoHdl = (MAPI_AO_HANDLE_T)g_ao_hdl;

    return MAPI_SUCCESS;
}

int MAPI_AO_Init(MAPI_AO_HANDLE_T *AoHdl,const MAPI_AO_ATTR_S *pstAoAttr)
{
    int ret=MAPI_SUCCESS;

    AIO_ATTR_S AudoutAttr;
    AudoutAttr.enSamplerate = pstAoAttr->enSampleRate;
    AudoutAttr.u32ChnCnt = pstAoAttr->channels;
    AudoutAttr.u32PtNumPerFrm = pstAoAttr->u32PtNumPerFrm;
    AudoutAttr.enSoundmode = (pstAoAttr->AudioChannel == 2) ? AUDIO_SOUND_MODE_STEREO : AUDIO_SOUND_MODE_MONO;
    AudoutAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    AudoutAttr.u32EXFlag = 0;
    AudoutAttr.u32FrmNum = 4; /* only use in bind mode */
    AudoutAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    AudoutAttr.u32ClkSel = 0;
    AudoutAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

    AUDIO_INIT_ONCE();

    if(g_ao_hdl && (g_ao_hdl->bInit == true)) {
        CVI_LOGE("MAPI_AO_Init alread init.\n");
        return MAPI_ERR_FAILURE;
    }

    if (!g_ao_hdl) {
        g_ao_hdl = (MAPI_AO_CTX_T *)malloc(sizeof(MAPI_AO_CTX_T));
        if (!g_ao_hdl) {
            CVI_LOGE("malloc failed\n");
            return MAPI_ERR_NOMEM;
        }
    }

    memset(g_ao_hdl, 0, sizeof(MAPI_AO_CTX_T));
    memcpy(&g_ao_hdl->attr,pstAoAttr,sizeof(MAPI_AO_ATTR_S));

    ret=CVI_AO_SetPubAttr(g_ao_hdl->AoDevid, &AudoutAttr);
    if(ret){
        CVI_LOGE("%s: AO_SetPubAttr(%d) failed with %d\n", __func__,g_ao_hdl->AoDevid,ret);
        free(g_ao_hdl);
        return ret;
    }

    g_ao_hdl->bInit = true;
    ret = CVI_AO_Enable(g_ao_hdl->AoDevid);
    if (ret) {
        CVI_LOGE("%s: AO_Enable(%d) failed with %d\n", __func__,g_ao_hdl->AoDevid,ret);
        return ret;
    }
    if(pstAoAttr->volume > 0){
        CVI_AO_SetVolume(g_ao_hdl->AoDevid, pstAoAttr->volume);
    }
    CVI_AO_GetVolume(g_ao_hdl->AoDevid,&(g_ao_hdl->volume));
    CVI_LOGW("aplay_demo volume %d\n",g_ao_hdl->volume);
    *AoHdl = (MAPI_AO_HANDLE_T)g_ao_hdl;
    return ret;
}

int MAPI_AO_Start(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn)
{
    int ret=MAPI_SUCCESS;
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }
    if(et->bInit!=true){
        CVI_LOGE("Please MAPI_AO_Init first\n");
        return MAPI_ERR_FAILURE;
    }

    ret=CVI_AO_EnableChn(et->AoDevid,AoChn);
    if(ret){
        CVI_LOGE("%s: AI_Enable(%d) failed with %d\n",__func__,et->AoDevid,ret);
        return ret;
    }

    return ret;
}

int MAPI_AO_SendFrame(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout)
{
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }
    if(et->bBindAdec) {
        CVI_LOGE("audio has bind Adec and not use MAPI_AO_SendFrame()\n");
        return MAPI_ERR_INVALID;
    }
    return CVI_AO_SendFrame(et->AoDevid,AoChn,pstAudioFrame, u32Timeout);
}


int MAPI_AO_Deinit(MAPI_AO_HANDLE_T AoHdl)
{
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }
    CVI_AO_Disable(et->AoDevid);
    #ifdef CHIP_184X
    g_ao_hdl->bInit = false;
    #else
    if (g_ao_hdl) {
        free(g_ao_hdl);
        g_ao_hdl=NULL;
    }
    #endif
    return MAPI_SUCCESS;
}

int MAPI_AO_Stop(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn)
{
    int ret;
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }

    ret = CVI_AO_DisableChn(et->AoDevid,AoChn);
    if (ret) {
        CVI_LOGE("%s: AO_DisableChn failed with %#x!\n", __func__, ret);
        return ret;
    }

    return ret;
}
int MAPI_AO_EnableReSmp(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn, AUDIO_SAMPLE_RATE_E enChnSamRate)
{
    int ret = MAPI_SUCCESS;
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }

    ret = CVI_AO_EnableReSmp(et->AoDevid,AoChn,enChnSamRate);
    if(ret){
        CVI_LOGE("AO_EnableReSmp(%d,%d,%d) err\n",et->AoDevid,AoChn,enChnSamRate);
        return MAPI_ERR_FAILURE;
    }

    return ret;
}

int MAPI_AO_DisableReSmp(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn)
{
    int ret = MAPI_SUCCESS;
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }

    ret = CVI_AO_DisableReSmp(et->AoDevid, AoChn);
    if(ret){
        CVI_LOGE("AO_DisableReSmp(%d,%d) err\n", et->AoDevid, AoChn);
        return MAPI_ERR_FAILURE;
    }

    return ret;
}
int MAPI_AO_SetVolume(MAPI_AO_HANDLE_T AoHdl,int volume)
{
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }
    CVI_LOGW("AoDevid :%d and set volume: %d\n",et->AoDevid,volume);

    if(CVI_AO_SetVolume(et->AoDevid,volume)){
        CVI_LOGE("AO_SetVolume() ERR\n");
        return MAPI_ERR_FAILURE;
    }
    et->volume=volume;
    return  MAPI_SUCCESS;
}

int MAPI_AO_GetVolume(MAPI_AO_HANDLE_T AoHdl,int *volume)
{

    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }

    if(CVI_AO_GetVolume(et->AoDevid,volume)){
        CVI_LOGE("GetVolime() ERR\n");
        return MAPI_ERR_FAILURE;
    }
    CVI_LOGW("AoDevid :%d and get volume: %d\n",et->AoDevid,*volume);
    CVI_LOGW("g_ao_hdl->volume=%d\n",et->volume);
    return MAPI_SUCCESS;
}

int MAPI_AO_Mute(MAPI_AO_HANDLE_T AoHdl)
{
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }

    CVI_AO_SetMute(et->AoDevid,MAPI_TRUE,NULL);
    et->bMute = MAPI_TRUE;
    return MAPI_SUCCESS;
}

int MAPI_AO_Unmute(MAPI_AO_HANDLE_T AoHdl)
{
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }

    CVI_AO_SetMute(et->AoDevid,MAPI_FALSE,NULL);
    et->bMute = MAPI_FALSE;
    return MAPI_SUCCESS;
}

int MAPI_AO_BindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn)
{
    MMF_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId = CVI_ID_ADEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = AdChn;
    stDestChn.enModId = CVI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;

    return CVI_AUD_SYS_Bind(&stSrcChn,&stDestChn);
}
int MAPI_AO_UnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn)
{
	MMF_CHN_S stSrcChn, stDestChn;

	stSrcChn.enModId = CVI_ID_ADEC;
	stSrcChn.s32ChnId = AdChn;
	stSrcChn.s32DevId = 0;
	stDestChn.enModId = CVI_ID_AO;
	stDestChn.s32DevId = AoDev;
	stDestChn.s32ChnId = AoChn;
	return CVI_AUD_SYS_UnBind(&stSrcChn, &stDestChn);

}
int MAPI_AO_SendSysFrame(MAPI_AO_HANDLE_T AoHdl,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout)
{
    CVI_LOGW("not support.\n");
    UNUSED(AoHdl);
    UNUSED(pstAudioFrame);
    UNUSED(u32Timeout);
    return MAPI_SUCCESS;
}


int MAPI_AO_RegisterCallback(MAPI_AO_HANDLE_T AoHdl, const MAPI_AO_CALLBACK_S* pstAoCB)
{
    int ret = MAPI_SUCCESS;
    int i;
     MAPI_AO_CTX_T *pstAoCtx=(MAPI_AO_CTX_T *)AoHdl;

    if(!pstAoCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAoCtx->ao_mutex);
    for(i = 0; i < MAX_AO_CALLBACK_COUNT; i++) {
        if(!pstAoCtx->p_ao_cb[i]) {
            pstAoCtx->p_ao_cb[i] = (MAPI_AO_CALLBACK_S* )pstAoCB;
            break;
        }
    }
    pthread_mutex_unlock(&pstAoCtx->ao_mutex);

    return ret;
}

int MAPI_AO_UnregisterCallback(MAPI_AO_HANDLE_T AoHdl, const MAPI_AO_CALLBACK_S* pstAoCB)
{
    int ret = MAPI_SUCCESS;
    int i;
     MAPI_AO_CTX_T *pstAoCtx=(MAPI_AO_CTX_T *)AoHdl;

    if(!pstAoCtx) {
        CVI_LOGE("params NULL.\n");
        return MAPI_ERR_INVALID;
    }

    pthread_mutex_lock(&pstAoCtx->ao_mutex);
    for(i = 0; i < MAX_AO_CALLBACK_COUNT; i++) {
        if(pstAoCtx->p_ao_cb[i] && pstAoCtx->p_ao_cb[i] == pstAoCB) {
            pstAoCtx->p_ao_cb[i] = NULL;
            break;
        }
    }
    pthread_mutex_unlock(&pstAoCtx->ao_mutex);
    return ret;

}

int MAPI_AO_SetAmplifier(MAPI_AO_HANDLE_T AoHdl, CVI_BOOL En)
{
    MAPI_AO_CTX_T *et=(MAPI_AO_CTX_T *)AoHdl;
    if(!et) {
        CVI_LOGE("AoHdl is NULL.\n");
        return MAPI_ERR_INVALID;
    }

    HAL_GPIO_Set_Value(et->attr.u32PowerPinId, En);

    return MAPI_SUCCESS;
}

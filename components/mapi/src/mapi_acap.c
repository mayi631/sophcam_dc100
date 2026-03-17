#include<stdio.h>
#include<pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "mapi_acap.h"
#include "cvi_audio.h"
#include "cvi_log.h"
// #include "acodec.h"

typedef struct MAPI_ACAP_CTX_S {
    MAPI_ACAP_ATTR_S attr;
    int AcapDevid;
    int AcapChn;
    int bVqeOn;
    int volume;
    int bInit;
} MAPI_ACAP_CTX_T;

// #define ACODEC_ADC  "/dev/cv1835adc"
#define ACODEC_ADC  "/dev/cv182xadc"


static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_acap_mutex = PTHREAD_MUTEX_INITIALIZER;
static MAPI_ACAP_CTX_T *g_acap_hdl; //only support one record device for now.
#define AI_TALKVQE_MASK_HPF_J	0x1
#define AI_TALKVQE_MASK_AEC_J	0x3
#define AI_TALKVQE_MASK_ANR_J	0x4
#define AI_TALKVQE_MASK_AGC_J	0x8
#define AI_TALKVQE_MASK_EQ_J	0x10
void init_once(void)
{
    CVI_AUDIO_INIT();
}

void AUDIO_INIT_ONCE(void)
{
    pthread_once(&once,init_once);
}

static int MAPI_Set_ACAP_AudioVolume(int s32AudioGain)
{
    if(CVI_AI_SetVolume(0, s32AudioGain)){
        CVI_LOGE("MAPI_Set_ACAP_AudioVolume() ERR\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

static int MAPI_Get_ACAP_AudioVolume(int *ps32AudioGain)
{

    if(CVI_AI_GetVolume(0, ps32AudioGain)){
        CVI_LOGE("AI_GetVolume() ERR\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}


static int MAPI_ACAP_EnableVqes(MAPI_ACAP_HANDLE_T AcapHdl)
{
    int ret = MAPI_SUCCESS;
    AI_TALKVQE_CONFIG_S stAiVqeAttr;
    AI_AEC_CONFIG_S default_AEC_Setting;
    AUDIO_AGC_CONFIG_S default_AGC_Setting;
    AUDIO_ANR_CONFIG_S  default_ANR_Setting;
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }

    memset(&default_AGC_Setting, 0, sizeof(AUDIO_AGC_CONFIG_S));
	//default_AGC_Setting.para_agc_cut6_enable =;
	default_AGC_Setting.para_agc_max_gain = 3;
	default_AGC_Setting.para_agc_target_high = 2;
	default_AGC_Setting.para_agc_target_low = 6;
	//default_AGC_Setting.para_agc_vad_cnt = ;
	// default_AGC_Setting.para_agc_vad_enable = 1;

    memset(&default_ANR_Setting, 0, sizeof(AUDIO_ANR_CONFIG_S));
    //default_ANR_Setting.para_nr_noise_coeff = ;
    default_ANR_Setting.para_nr_snr_coeff = 15;

    memset(&default_AEC_Setting, 0, sizeof(AI_AEC_CONFIG_S));
	//default_AEC_Setting.cfg_data = ;
	default_AEC_Setting.para_aec_filter_len = 13;
	default_AEC_Setting.para_aes_std_thrd = 37;
	default_AEC_Setting.para_aes_supp_coeff = 60;

    stAiVqeAttr.stAgcCfg = default_AGC_Setting;
    stAiVqeAttr.stAnrCfg = default_ANR_Setting;
    stAiVqeAttr.stAecCfg = default_AEC_Setting;
    stAiVqeAttr.u32OpenMask = AI_TALKVQE_MASK_AEC_J |AI_TALKVQE_MASK_AGC_J |AI_TALKVQE_MASK_ANR_J;
    // stAiVqeAttr.s32FrameSample = 160;
    stAiVqeAttr.s32WorkSampleRate = et->attr.enSampleRate;
    // stAiVqeAttr.enWorkstate = VQE_WORKSTATE_COMMON;
    // stAiVqeAttr.s32RevMask = 0x11;//CVIAUDIO_ALGO_SSP;//
    if (et->attr.channel != 2) {
        CVI_LOGE("if you want to use vqe,please set channel 2\n");
        return -1;
    }
    // stAiVqeAttr.s32BytesPerSample = et->attr.channel * 2;
    printf("u32OpenMask = Ox%x\n",stAiVqeAttr.u32OpenMask);
    printf("AcapDevid =%d,AcapChn = %d, enSampleRate = %d\n",et->AcapDevid,et->AcapChn,et->attr.enSampleRate);
    ret = CVI_AI_SetTalkVqeAttr(
             et->AcapDevid,
             et->AcapChn,
             0,
             0,
             &stAiVqeAttr);
    if(ret != 0) {
        return ret;
    }

    ret = CVI_AI_EnableVqe(et->AcapDevid, et->AcapDevid);
    if(ret != 0) {
        CVI_LOGE("AI_EnableVqe failed.ret:%d.\n",ret);
        return ret;
    }
    return ret;
}



int MAPI_ACAP_Init(MAPI_ACAP_HANDLE_T *AcapHdl,const MAPI_ACAP_ATTR_S *pstACapAttr)
{
    int ret=MAPI_SUCCESS;
    AIO_ATTR_S stAiAttr;

    pthread_mutex_lock(&g_acap_mutex);
    if(g_acap_hdl) {
        CVI_LOGE("MAPI_ACAP_Init already init.\n");
        pthread_mutex_unlock(&g_acap_mutex);
        return MAPI_ERR_FAILURE;
    }
    g_acap_hdl = (MAPI_ACAP_CTX_T *)malloc(sizeof(MAPI_ACAP_CTX_T));
    if (!g_acap_hdl) {
        CVI_LOGE("malloc failed\n");
        pthread_mutex_unlock(&g_acap_mutex);
        return MAPI_ERR_NOMEM;
    }
    AUDIO_INIT_ONCE();

    memset(g_acap_hdl, 0, sizeof(MAPI_ACAP_CTX_T));
    memcpy(&g_acap_hdl->attr,pstACapAttr,sizeof(MAPI_ACAP_ATTR_S));
    g_acap_hdl->AcapDevid = 0;
    g_acap_hdl->AcapChn = 0;
    g_acap_hdl->bVqeOn = g_acap_hdl->attr.bVqeOn;

    stAiAttr.u32ChnCnt      = pstACapAttr->channel;
    stAiAttr.enSoundmode    = (pstACapAttr->AudioChannel==2)?AUDIO_SOUND_MODE_STEREO:AUDIO_SOUND_MODE_MONO;
    stAiAttr.enSamplerate   = pstACapAttr->enSampleRate;
    stAiAttr.enBitwidth =   AUDIO_BIT_WIDTH_16;
    stAiAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAiAttr.u32EXFlag      = 0;
    stAiAttr.u32FrmNum      = 4; /* only use in bind mode */
    stAiAttr.u32PtNumPerFrm = pstACapAttr->u32PtNumPerFrm; //sample rate / fps
    stAiAttr.u32ClkSel      = 0;
    stAiAttr.enI2sType = AIO_I2STYPE_INNERCODEC;

    ret = CVI_AI_SetPubAttr(g_acap_hdl->AcapDevid, &stAiAttr);
    if (ret) {
        CVI_LOGE("%s: AI_SetPubAttr failed with %d\n", __func__,ret);
        free(g_acap_hdl);
        pthread_mutex_unlock(&g_acap_mutex);
        return ret;
    }

    ret = CVI_AI_SetCard(g_acap_hdl->AcapDevid, 0);
    if (ret) {
        CVI_LOGE("%s: AI_SetCard failed with %d\n", __func__, ret);
        free(g_acap_hdl);
        pthread_mutex_unlock(&g_acap_mutex);
        return ret;
    }

    CVI_AI_Enable(g_acap_hdl->AcapDevid);

    g_acap_hdl->bInit = true;
    pthread_mutex_unlock(&g_acap_mutex);

    if(pstACapAttr->volume > 0){
        MAPI_Set_ACAP_AudioVolume(pstACapAttr->volume);
    }

    MAPI_Get_ACAP_AudioVolume(&g_acap_hdl->volume);
    *AcapHdl = (MAPI_ACAP_HANDLE_T)g_acap_hdl;

    return ret;
}

int MAPI_ACAP_Deinit(MAPI_ACAP_HANDLE_T AcapHdl)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;

    CVI_AI_DisableChn(et->AcapDevid,et->AcapChn);
    CVI_AI_Disable(et->AcapDevid);
    if(g_acap_hdl) {
        free(g_acap_hdl);
        g_acap_hdl = NULL;
    }

    return MAPI_SUCCESS;
}

int MAPI_ACAP_EnableRecordVqe (MAPI_ACAP_HANDLE_T AcapHdl)
{
    int ret;
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&g_acap_mutex);
    if(et->bVqeOn) {
        CVI_LOGE("acap vqe already on.\n");
        pthread_mutex_unlock(&g_acap_mutex);
        return MAPI_ERR_INVALID;
    }

    ret = MAPI_ACAP_EnableVqes(AcapHdl);
    pthread_mutex_unlock(&g_acap_mutex);

    return ret;
}

int MAPI_ACAP_EnableVqe(MAPI_ACAP_HANDLE_T AcapHdl)
{
    int ret;
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    pthread_mutex_lock(&g_acap_mutex);

    ret = MAPI_ACAP_EnableVqes(AcapHdl);
    if(ret) {
        CVI_LOGE("acap vqe fail.\n");
        pthread_mutex_unlock(&g_acap_mutex);
        return MAPI_ERR_INVALID;
    }
    pthread_mutex_unlock(&g_acap_mutex);
    return ret;
}

int MAPI_ACAP_DisableVqe(MAPI_ACAP_HANDLE_T AcapHdl)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }

    return CVI_AI_DisableVqe(et->AcapDevid ,et->AcapChn);;
}

int MAPI_ACAP_Start(MAPI_ACAP_HANDLE_T AcapHdl)
{
    int s32Ret = MAPI_SUCCESS;
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    if (CVI_AI_EnableChn(et->AcapDevid ,et->AcapChn)) {
        CVI_LOGE("enable chn fail\n");
        return MAPI_ERR_FAILURE;
    }
    if (et->bVqeOn == true) {
        s32Ret = MAPI_ACAP_EnableVqes(AcapHdl);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("enable vqe faile\n");
            return MAPI_ERR_INVALID;
        }
    }

    return MAPI_SUCCESS;
}

int MAPI_ACAP_Stop(MAPI_ACAP_HANDLE_T AcapHdl)
{
    int s32Ret = MAPI_SUCCESS;
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    if (et->bVqeOn == true) {
        s32Ret = MAPI_ACAP_DisableVqe(AcapHdl);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("disable vqe faile\n");
            return MAPI_ERR_INVALID;
        }
    }

    CVI_AI_DisableChn(et->AcapDevid ,et->AcapChn);

    return MAPI_SUCCESS;
}

int MAPI_ACAP_EnableReSmp(MAPI_ACAP_HANDLE_T AcapHdl, AUDIO_SAMPLE_RATE_E enOutSampleRate)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    return CVI_AI_EnableReSmp(et->AcapDevid ,et->AcapChn, enOutSampleRate);
}

int MAPI_ACAP_DisableReSmp(MAPI_ACAP_HANDLE_T AcapHdl)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    return CVI_AI_DisableReSmp(et->AcapDevid ,et->AcapChn);
}


int MAPI_ACAP_SetVolume(MAPI_ACAP_HANDLE_T AcapHdl,int s32AudioGain)
{
    int ret = 0;
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }

    ret = MAPI_Set_ACAP_AudioVolume(s32AudioGain);
    if(ret != 0) {
        CVI_LOGE("MAPI_Set_ACAP_AudioVolume failed,ret=%d.\n",ret);
        return ret;
    }

    et->volume = s32AudioGain;

    return MAPI_SUCCESS;
}

int MAPI_ACAP_GetVolume(MAPI_ACAP_HANDLE_T AcapHdl,int *ps32AudioGain)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }

    *ps32AudioGain = et->volume;

    return MAPI_SUCCESS;
}

int MAPI_ACAP_Mute(MAPI_ACAP_HANDLE_T AcapHdl)
{
    UNUSED(AcapHdl);
    return MAPI_Set_ACAP_AudioVolume(0);
}

int MAPI_ACAP_Unmute(MAPI_ACAP_HANDLE_T AcapHdl)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }

    return MAPI_Set_ACAP_AudioVolume(et->volume);
}

int MAPI_ACAP_GetFrame(MAPI_ACAP_HANDLE_T AcapHdl,AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    if(!et->bInit) {
        CVI_LOGE("acap not init.\n");
        return MAPI_ERR_FAILURE;
    }

    return CVI_AI_GetFrame(et->AcapDevid ,et->AcapChn,pstFrm,pstAecFrm, -1);
}

int MAPI_ACAP_ReleaseFrame(MAPI_ACAP_HANDLE_T AcapHdl,const AUDIO_FRAME_S *pstFrm, const AEC_FRAME_S *pstAecFrm)
{
    MAPI_ACAP_CTX_T *et = (MAPI_ACAP_CTX_T *)AcapHdl;
    if(!et) {
        return MAPI_ERR_INVALID;
    }
    if(!et->bInit) {
        CVI_LOGE("acap not init.\n");
        return MAPI_ERR_FAILURE;
    }

    return CVI_AI_ReleaseFrame(et->AcapDevid ,et->AcapChn,pstFrm,pstAecFrm);
}




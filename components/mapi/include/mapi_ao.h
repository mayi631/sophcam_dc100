#ifndef __MAPI_AO_H__
#define __MAPI_AO_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdio.h"
#include "mapi_define.h"
#include "cvi_comm_aio.h"
#include "cvi_comm_adec.h"
#include "osal.h"
#include "mapi_adec.h"
#define MAPI_TRUE  1
#define MAPI_FALSE 0

#ifdef __cplusplus
extern "C"
{
#endif
typedef MAPI_HANDLE_T MAPI_AO_HANDLE_T;
#define MAX_AO_CALLBACK_COUNT (1)

typedef struct MAPI_AO_ATTR_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;
    uint32_t channels;
    uint32_t u32PtNumPerFrm;//frames unit
    uint32_t u32PowerPinId;
    uint32_t AudioChannel;
    int32_t  volume;
} MAPI_AO_ATTR_S;

typedef int (*MAPI_AO_DATAPROC_CALLBACK_FN_PTR)(MAPI_AO_HANDLE_T AoHdl,
    AUDIO_STREAM_S* pAStreamData, void *pPrivateData);

typedef struct MAPI_AO_CALLBACK_S
{
    MAPI_AO_DATAPROC_CALLBACK_FN_PTR pfnDataCB;
    void *pPrivateData; /** private data */
} MAPI_AO_CALLBACK_S;

typedef struct AO_CB_INFO_T{
    MAPI_AUDIO_CODEC_E enCodecType;
    uint32_t u32PtNumPerFrm;
    char *pBuffer;
    char * out_filename;
    FILE *fp;
}AO_CB_INFO;

typedef struct MAPI_AO_CTX_S {
    MAPI_AO_ATTR_S      attr;
    int                     AoDevid;
    int                     AoChn;
    int                     volume;
    int                     bMute;
    int                     bInit;
    int                     bBindAdec;
    volatile int            quit;
    pthread_mutex_t         ao_mutex;
    MAPI_ADEC_HANDLE_T  AdecHdl;
    OSAL_TASK_HANDLE_S  cb_task;
    MAPI_AO_CALLBACK_S    *p_ao_cb[MAX_AO_CALLBACK_COUNT];
} MAPI_AO_CTX_T;

int MAPI_AO_GetHandle(MAPI_AO_HANDLE_T *AoHdl);
int MAPI_AO_Init(MAPI_AO_HANDLE_T *AoHdl,const MAPI_AO_ATTR_S *pstAoAttr);
int MAPI_AO_Deinit(MAPI_AO_HANDLE_T AoHdl);
int MAPI_AO_Start(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn);
int MAPI_AO_Stop(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn);
int MAPI_AO_EnableReSmp(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn, AUDIO_SAMPLE_RATE_E enChnSamRate);
int MAPI_AO_DisableReSmp(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn);
int MAPI_AO_SetVolume(MAPI_AO_HANDLE_T AoHdl,int volume);
int MAPI_AO_GetVolume(MAPI_AO_HANDLE_T AoHdl,int *volume);
int MAPI_AO_Mute(MAPI_AO_HANDLE_T AoHdl);
int MAPI_AO_Unmute(MAPI_AO_HANDLE_T AoHdl);
int MAPI_AO_SendFrame(MAPI_AO_HANDLE_T AoHdl,CVI_S32 AoChn,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout);
int MAPI_AO_BindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn);
int MAPI_AO_UnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn,ADEC_CHN AdChn);
int MAPI_AO_SendSysFrame(MAPI_AO_HANDLE_T AoHdl,const AUDIO_FRAME_S *pstAudioFrame, uint32_t u32Timeout);
int MAPI_AO_RegisterCallback(MAPI_AO_HANDLE_T AoHdl, const MAPI_AO_CALLBACK_S* pstAoCB);
int MAPI_AO_UnregisterCallback(MAPI_AO_HANDLE_T AoHdl, const MAPI_AO_CALLBACK_S* pstAoCB);
int MAPI_AO_SetAmplifier(MAPI_AO_HANDLE_T AoHdl, CVI_BOOL En);
#ifdef __cplusplus
}
#endif

#endif //__MAPI_AO_H__





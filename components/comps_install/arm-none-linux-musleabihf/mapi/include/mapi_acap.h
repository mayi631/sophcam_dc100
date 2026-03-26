#ifndef __MAPI_ACAP_H__
#define __MAPI_ACAP_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "mapi_define.h"
#include "cvi_comm_aio.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef MAPI_HANDLE_T MAPI_ACAP_HANDLE_T;

#define MAPI_ACAP_DEV_MAX_NUM (1)

typedef enum AUDIO_CAP_ERROR_E
{
    AUDIO_ACAP_OK=0,
    AUDIO_ACAP_ERROR_FAILURE=0xFFFF0001,
    AUDIO_ACAP_ERROR_INVALID_PARAMS,
    AUDIO_ACAP_ERROR_NULL_POINTER,
    AUDIO_ACAP_ERROR_BUTT,
} AUDIO_CAP_ERROR_E;

typedef struct MAPI_ACAP_ATTR_S
{
    AUDIO_SAMPLE_RATE_E enSampleRate;
    uint32_t channel;
    uint32_t u32PtNumPerFrm;//frames unit
    int bVqeOn;
    uint32_t AudioChannel;
    int32_t volume;
} MAPI_ACAP_ATTR_S;


int MAPI_ACAP_Init(MAPI_ACAP_HANDLE_T *AcapHdl,const MAPI_ACAP_ATTR_S *pstACapAttr);
int MAPI_ACAP_Deinit(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_EnableRecordVqe (MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_EnableVqe(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_DisableVqe(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_Start(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_Stop(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_EnableReSmp(MAPI_ACAP_HANDLE_T AcapHdl, AUDIO_SAMPLE_RATE_E enOutSampleRate);
int MAPI_ACAP_DisableReSmp(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_SetVolume(MAPI_ACAP_HANDLE_T AcapHdl,int s32AudioGain);
int MAPI_ACAP_GetVolume(MAPI_ACAP_HANDLE_T AcapHdl,int *ps32AudioGain);
int MAPI_ACAP_Mute(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_Unmute(MAPI_ACAP_HANDLE_T AcapHdl);
int MAPI_ACAP_GetFrame(MAPI_ACAP_HANDLE_T AcapHdl,AUDIO_FRAME_S *pstFrm, AEC_FRAME_S *pstAecFrm);
int MAPI_ACAP_ReleaseFrame(MAPI_ACAP_HANDLE_T AcapHdl,const AUDIO_FRAME_S *pstFrm, const AEC_FRAME_S *pstAecFrm);


#ifdef __cplusplus
}
#endif

#endif //__MAPI_ACAP_H__


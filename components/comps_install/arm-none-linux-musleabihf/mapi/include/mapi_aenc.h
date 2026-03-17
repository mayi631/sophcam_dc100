#ifndef __MAPI_AENC_H__
#define __MAPI_AENC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "mapi_define.h"

#include "cvi_comm_aio.h"
#include "mapi_acap.h"
#include "cvi_comm_adec.h"
#include "cvi_comm_aenc.h"
#include "osal.h"
#include <semaphore.h>
#define FILE_NAME_LEN 128
//#define AUDIO_BUFFER_MAX  (10*60 * 1024)
#ifdef __cplusplus
extern "C"
{
#endif

typedef MAPI_HANDLE_T MAPI_AENC_HANDLE_T;

typedef enum MAPI_AUDIO_CODEC_E
{
    MAPI_AUDIO_CODEC_AAC = 0, /* AAC format */
    MAPI_AUDIO_CODEC_G711A, /* G711A format */
    MAPI_AUDIO_CODEC_G711U, /* G711U format */
    MAPI_AUDIO_CODEC_BUTT
}MAPI_AUDIO_CODEC_E;

typedef int (*MAPI_AENC_DATAPROC_CALLBACK_FN_PTR)(MAPI_AENC_HANDLE_T AencHdl,
    AUDIO_STREAM_S* pAStreamData, void *pPrivateData);


typedef struct MAPI_AENC_CALLBACK_S
{
    MAPI_AENC_DATAPROC_CALLBACK_FN_PTR pfnDataCB;
    void *pPrivateData; /** private data */
} MAPI_AENC_CALLBACK_S;


typedef struct MAPI_AENC_ATTR_S
{
    MAPI_AUDIO_CODEC_E enAencFormat; /**< audio encode format type*/
    uint32_t u32PtNumPerFrm; /**< sampling point number per frame,the same as acap */
    uint32_t src_samplerate;
    uint32_t channels;
} MAPI_AENC_ATTR_S;

typedef struct _AENC_AAC_INFO_S {
    CVI_U32 sample_rate;
    CVI_S32 bit_rate;//0-32
    int32_t i32FrameSize;
    char *pOutputBuf;
}AENC_AAC_INFO_S;

int MAPI_AENC_Init(MAPI_AENC_HANDLE_T *AencHdl, const MAPI_AENC_ATTR_S *pstAencAttr);
int MAPI_AENC_Deinit(MAPI_AENC_HANDLE_T AencHdl);
int MAPI_AENC_Start(MAPI_AENC_HANDLE_T AencHdl);
int MAPI_AENC_Stop(MAPI_AENC_HANDLE_T AencHdl);
int MAPI_AENC_SetMute(MAPI_AENC_HANDLE_T AencHdl, int bEnable);
int MAPI_AENC_GetMute(MAPI_AENC_HANDLE_T AencHdl, int *pbEnable);
int MAPI_AENC_RegisterCallback(MAPI_AENC_HANDLE_T AencHdl, const MAPI_AENC_CALLBACK_S* pstAencCB);
int MAPI_AENC_UnregisterCallback(MAPI_AENC_HANDLE_T AencHdl, const MAPI_AENC_CALLBACK_S* pstAencCB);
int MAPI_AENC_BindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn);
int MAPI_AENC_UnbindACap(AUDIO_DEV AiDev, AI_CHN AiChn,AUDIO_DEV AeDev, AENC_CHN AeChn);
int MAPI_AENC_SendFrame(MAPI_AENC_HANDLE_T AencHdl, const AUDIO_FRAME_S* pstFrm,AEC_FRAME_S* pstAecFrm);
int MAPI_AENC_GetStream(MAPI_AENC_HANDLE_T AencHdl, AUDIO_STREAM_S *pstStream,CVI_S32 s32MilliSec);

#ifdef __cplusplus
}
#endif

#endif //__MAPI_AENC_H__



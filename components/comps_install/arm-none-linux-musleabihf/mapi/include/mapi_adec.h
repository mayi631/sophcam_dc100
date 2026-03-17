#ifndef __MAPI_ADEC_H__
#define __MAPI_ADEC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "mapi_define.h"

#include "cvi_comm_aio.h"
#include "cvi_comm_adec.h"

#include "mapi_aenc.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef MAPI_HANDLE_T MAPI_ADEC_HANDLE_T;

typedef struct MAPI_ADEC_ATTR_S
{
    MAPI_AUDIO_CODEC_E enAdecFormat;
    AUDIO_SOUND_MODE_E  enSoundmode;
    AUDIO_SAMPLE_RATE_E enSamplerate;
    ADEC_MODE_E enMode;
    int frame_size;
} MAPI_ADEC_ATTR_S;


typedef struct MAPI_AUDIO_FRAME_INFO_S
{
    AUDIO_FRAME_INFO_S stAudFrameInfo;
    void *pstream;
    uint32_t u32Len;
    CVI_U64 u64Timestamp;
} MAPI_AUDIO_FRAME_INFO_S;


int MAPI_ADEC_Init(MAPI_ADEC_HANDLE_T *AdecHdl, const MAPI_ADEC_ATTR_S* pstAdecAttr);
int MAPI_ADEC_Deinit(MAPI_ADEC_HANDLE_T AdecHdl);
int MAPI_ADEC_SendStream(MAPI_ADEC_HANDLE_T AdecHdl, const AUDIO_STREAM_S* pstAdecStream, int bBlock);
int MAPI_ADEC_SendEndOfStream(MAPI_ADEC_HANDLE_T AdecHdl);
int MAPI_ADEC_GetFrame(MAPI_ADEC_HANDLE_T AdecHdl, MAPI_AUDIO_FRAME_INFO_S* pstAudioFrameInfo, int bBlock);
int MAPI_ADEC_ReleaseFrame(MAPI_ADEC_HANDLE_T AdecHdl, const MAPI_AUDIO_FRAME_INFO_S* pstAudioFrameInfo);


#ifdef __cplusplus
}
#endif

#endif //__MAPI_ADEC_H__


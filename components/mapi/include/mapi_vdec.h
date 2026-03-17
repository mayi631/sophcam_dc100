#ifndef __MAPI_VDEC_H__
#define __MAPI_VDEC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "mapi_define.h"
#include "mapi_venc.h"  // MAPI_VCODEC_E
// #include "cvi_common.h"
#include "cvi_comm_vb.h"

#include "cvi_comm_video.h"
#include "cvi_comm_vdec.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef MAPI_HANDLE_T MAPI_VDEC_HANDLE_T;

typedef struct MAPI_VDEC_CHN_ATTR_S {
    MAPI_VCODEC_E codec;
    uint32_t          max_width;
    uint32_t          max_height;
    PIXEL_FORMAT_E    pixel_format;
} MAPI_VDEC_CHN_ATTR_T;

int MAPI_VDEC_InitChn(MAPI_VDEC_HANDLE_T *vdec_hdl,
        MAPI_VDEC_CHN_ATTR_T *attr);
int MAPI_VDEC_DeinitChn(MAPI_VDEC_HANDLE_T vdec_hdl);

int MAPI_VDEC_GetChn(MAPI_VDEC_HANDLE_T vdec_hdl);

int MAPI_VDEC_SendStream(MAPI_VDEC_HANDLE_T vdec_hdl,
        VDEC_STREAM_S *stream);
int MAPI_VDEC_GetFrame(MAPI_VDEC_HANDLE_T vdec_hdl,
        VIDEO_FRAME_INFO_S *frame);

int MAPI_VDEC_ReleaseFrame(MAPI_VDEC_HANDLE_T vdec_hdl,
        VIDEO_FRAME_INFO_S *frame);

void MAPI_GetMaxSizeByEncodeType(PAYLOAD_TYPE_E enType,
         uint32_t *max_width, uint32_t *max_height);

int MAPI_VDEC_SetVBMode(VB_SOURCE_E vbMode, CVI_U32 frameBufCnt);
int MAPI_VDEC_IsCodecSupported(MAPI_VCODEC_E codec);


#ifdef __cplusplus
}
#endif

#endif

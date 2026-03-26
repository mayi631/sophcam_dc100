#ifndef __MAPI_VENC_H__
#define __MAPI_VENC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "mapi_define.h"

#include "cvi_comm_video.h"
#include "cvi_comm_venc.h"
#include "osal.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef MAPI_HANDLE_T MAPI_VENC_HANDLE_T;

typedef enum _MAPI_VCODEC_E {
    MAPI_VCODEC_H264 = 0,
    MAPI_VCODEC_H265,
    MAPI_VCODEC_JPG,
    MAPI_VCODEC_MJP,
    MAPI_VCODEC_MULTI,
    MAPI_VCODEC_MAX
} MAPI_VCODEC_E;

static const PAYLOAD_TYPE_E MAPI_VCODEC_TO_PAYLOAD_TYPE[MAPI_VCODEC_MAX] = {
    PT_H264,
    PT_H265,
    PT_JPEG,
    PT_MJPEG,
    PT_BUTT    // non supported
};

typedef int (*MAPI_VENC_STREAM_CALLBACK_FN_PTR)(MAPI_VENC_HANDLE_T venc_hdl,
    VENC_STREAM_S *stream, void *private_data);

typedef struct MAPI_VENC_CHN_CB_S {
    MAPI_VENC_STREAM_CALLBACK_FN_PTR   stream_cb_func;
    void                                  *stream_cb_data;
} MAPI_VENC_CHN_CB_T;

typedef struct MAPI_VENC_CHN_PARAM_S {
    MAPI_VCODEC_E codec;
    uint32_t          width;
    uint32_t          height;
    PIXEL_FORMAT_E    pixel_format;
    int               gop;
    int               profile;
    // RCMODE: 0 - CBR, 1 - VBR, 2 - AVBR, 3 - QVBR, 4 - FIXQP, 5 - QPMAP
    int               rate_ctrl_mode;
    int               bitrate_kbps;
    int               iqp;
    int               pqp;
    int               minIqp;
    int               maxIqp;
    int               minQp;
    int               maxQp;
    int               changePos;
    int               jpeg_quality;
    int               single_EsBuf;
    uint32_t          bufSize;
    uint32_t          datafifoLen;
    uint32_t          src_framerate;
    uint32_t          dst_framerate;
    int               initialDelay;
    uint32_t          thrdLv;
    uint32_t          statTime;
    int firstFrameStartQp;
    int maxBitRate;
    int gop_mode;
    int maxIprop;
    int minIprop;
    int minStillPercent;
    int maxStillQP;
    int avbrPureStillThr;
    int motionSensitivity;
    int bgDeltaQp;
    int rowQpDelta;

    uint8_t aspectRatioInfoPresentFlag;
    uint8_t aspectRatioIdc;
    uint8_t overscanInfoPresentFlag;
    uint8_t overscanAppropriateFlag;
    uint16_t sarWidth;
    uint16_t sarHeight;

    uint8_t timingInfoPresentFlag;
    uint8_t fixedFrameRateFlag;
    uint32_t numUnitsInTick;
    uint32_t timeScale;
    uint32_t num_ticks_poc_diff_one_minus1;

    uint8_t videoSignalTypePresentFlag;
    uint8_t videoFormat;
    uint8_t videoFullRangeFlag;
    uint8_t colourDescriptionPresentFlag;
    uint8_t colourPrimaries;
    uint8_t transferCharacteristics;
    uint8_t matrixCoefficients;
    uint8_t ipqpDelta;
} MAPI_VENC_CHN_PARAM_T;

typedef struct MAPI_VENC_CHN_ATTR_S {
    bool sbm_enable;
    uint32_t  BindVprocId;
    uint32_t  BindVprocChnId;
    MAPI_VENC_CHN_PARAM_T venc_param;
    MAPI_VENC_CHN_CB_T cb;
} MAPI_VENC_CHN_ATTR_T;

typedef struct MAPI_VENC_CTX_S {
	MAPI_VENC_CHN_ATTR_T attr;
	VENC_CHN vencChn;
    VENC_PACK_S *packet;
	OSAL_TASK_HANDLE_S cbTask;
	OSAL_MUTEX_HANDLE_S cbMutex;
	sem_t cbSem;

	volatile int quit;
} MAPI_VENC_CTX_T;

int MAPI_VENC_InitChn(MAPI_VENC_HANDLE_T *venc_hdl,
        MAPI_VENC_CHN_ATTR_T *attr);
int MAPI_VENC_DeinitChn(MAPI_VENC_HANDLE_T venc_hdl);

int MAPI_VENC_GetChn(MAPI_VENC_HANDLE_T venc_hdl);

int MAPI_VENC_GetChnFd(MAPI_VENC_HANDLE_T venc_hdl);

int MAPI_VENC_SendFrame(MAPI_VENC_HANDLE_T venc_hdl,
        VIDEO_FRAME_INFO_S *frame);
int MAPI_VENC_GetStream(MAPI_VENC_HANDLE_T venc_hdl,
        VENC_STREAM_S *stream);

int MAPI_VENC_GetStreamTimeWait(MAPI_VENC_HANDLE_T venc_hdl,
        VENC_STREAM_S *stream, CVI_S32 S32MilliSec);

int MAPI_VENC_ReleaseStream(MAPI_VENC_HANDLE_T venc_hdl,
        VENC_STREAM_S *stream);

int MAPI_VENC_GetStreamStatus(MAPI_VENC_HANDLE_T venc_hdl,
        VENC_PACK_S *stream, bool *is_I_frame);

// combine MAPI_VENC_SendFrame() and MAPI_VENC_GetStream()
int MAPI_VENC_EncodeFrame(MAPI_VENC_HANDLE_T venc_hdl,
        VIDEO_FRAME_INFO_S *frame, VENC_STREAM_S *stream);

int MAPI_VENC_RequestIDR(MAPI_VENC_HANDLE_T venc_hdl);
int MAPI_VENC_SetAttr(MAPI_VENC_HANDLE_T *venc_hdl, MAPI_VENC_CHN_ATTR_T *attr);
int MAPI_VENC_GetAttr(MAPI_VENC_HANDLE_T venc_hdl, MAPI_VENC_CHN_ATTR_T *attr);
int MAPI_VENC_BindVproc(MAPI_VENC_HANDLE_T venc_hdl, int VpssGrp, int VpssChn);
int MAPI_VENC_UnBindVproc(MAPI_VENC_HANDLE_T venc_hdl, int VpssGrp, int VpssChn);
int MAPI_VENC_StartRecvFrame(MAPI_VENC_HANDLE_T venc_hdl, CVI_S32 frameCnt);
int MAPI_VENC_StopRecvFrame(MAPI_VENC_HANDLE_T venc_hdl);
int MAPI_VENC_SetBitrate(MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 bitRate);
int MAPI_VENC_SetDataFifoLen(MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 len);
int MAPI_VENC_GetDataFifoLen(MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 *len);
int MAPI_VENC_GetJpegParam(MAPI_VENC_HANDLE_T venc_hdl, VENC_JPEG_PARAM_S *param);
int MAPI_VENC_SetJpegParam(MAPI_VENC_HANDLE_T venc_hdl, VENC_JPEG_PARAM_S *param);
#ifdef __cplusplus
}
#endif

#endif

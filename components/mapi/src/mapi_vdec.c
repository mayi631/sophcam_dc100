#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include "cvi_buffer.h"
#include "cvi_vdec.h"
#include "cvi_vb.h"
// #include "sample_comm.h"
#include "mapi_vdec.h"
#include "osal.h"
#include "cvi_log.h"
#include "mapi.h"

typedef struct MAPI_VDEC_CTX_S {
	MAPI_VCODEC_E codec;
	VDEC_CHN vdecChn;
} MAPI_VDEC_CTX_T;

#define MULTI_VCODEC_PRO_MAX (4)
static int gVdecChnUsed[MULTI_VCODEC_PRO_MAX] = {0};
static pthread_mutex_t gVdecMutex[MAPI_VCODEC_MAX] = {
	[0 ... (MAPI_VCODEC_MAX - 1)] = PTHREAD_MUTEX_INITIALIZER};
VB_POOL gPicVbPool[MULTI_VCODEC_PRO_MAX] = { [0 ...(MULTI_VCODEC_PRO_MAX - 1)] = VB_INVALID_POOLID };

static VB_SOURCE_E gVbMode = VB_SOURCE_COMMON;
static CVI_U32 gFrameBufCnt = 6;

void MAPI_GetMaxSizeByEncodeType(PAYLOAD_TYPE_E enType, CVI_U32 *max_width, CVI_U32 *max_height)
{
    if (enType == PT_JPEG) {
        *max_width = JPEGD_MAX_WIDTH;
        *max_height = JPEGD_MAX_HEIGHT;
    } else if (enType == PT_H264) {
        *max_width = VEDU_H264D_MAX_WIDTH;
        *max_height = VEDU_H264D_MAX_HEIGHT;
    } else if (enType == PT_H265) {
        *max_width = VEDU_H265D_MAX_WIDTH;
        *max_height = VEDU_H265D_MAX_HEIGHT;
    }
}

static CVI_S32 _MAPI_VDEC_GetValidChan(VDEC_CHN *vdecChn, MAPI_VCODEC_E codec)
{
	CVI_U32 i;

	pthread_mutex_lock(&gVdecMutex[codec]);

	for (i = 0; i < MULTI_VCODEC_PRO_MAX; i++) {
		if (gVdecChnUsed[i] == 0)
			break;
	}

	if (i >= MULTI_VCODEC_PRO_MAX) {
		CVI_LOGE("VDEC support %d chn at most\n", MULTI_VCODEC_PRO_MAX);
		pthread_mutex_unlock(&gVdecMutex[codec]);
		return MAPI_ERR_FAILURE;
	}

	gVdecChnUsed[i] = 1;
	*vdecChn = i;
	pthread_mutex_unlock(&gVdecMutex[codec]);

	return MAPI_SUCCESS;
}

static void _MAPI_VDEC_ReleaseChan(VDEC_CHN vdecChn, MAPI_VCODEC_E codec)
{
	pthread_mutex_lock(&gVdecMutex[codec]);

	if (vdecChn < MULTI_VCODEC_PRO_MAX)
		gVdecChnUsed[vdecChn] = 0;

	pthread_mutex_unlock(&gVdecMutex[codec]);

	return;
}

int MAPI_VDEC_SetVBMode(VB_SOURCE_E vbMode, CVI_U32 frameBufCnt)
{
	if (vbMode >= VB_SOURCE_BUTT)
		return MAPI_ERR_INVALID;

	gVbMode = vbMode;
	gFrameBufCnt = frameBufCnt;

	return MAPI_SUCCESS;
}

int MAPI_VDEC_InitChn(MAPI_VDEC_HANDLE_T *vdec_hdl, MAPI_VDEC_CHN_ATTR_T *attr)
{
	VDEC_CHN vdecChn;
	int ret = MAPI_SUCCESS;
	VB_POOL_CONFIG_S stVbPoolCfg = {0};
	VDEC_CHN_ATTR_S stChnAttr = {0};
	VDEC_CHN_PARAM_S stChnParam = {0};
	VDEC_MOD_PARAM_S stModParam = {0};
	PAYLOAD_TYPE_E encodeType;
	MAPI_VDEC_CTX_T *vdecCtx = NULL;

	if (attr->codec >= MAPI_VCODEC_MAX) {
		CVI_LOGE("VDEC do not support %d\n", attr->codec);
		return MAPI_ERR_INVALID;
    }

	if (_MAPI_VDEC_GetValidChan(&vdecChn, attr->codec) != MAPI_SUCCESS)
		return MAPI_ERR_FAILURE;

	vdecCtx = (MAPI_VDEC_CTX_T *)calloc(1, sizeof(MAPI_VDEC_CTX_T));
	if (vdecCtx == NULL) {
		CVI_LOGE("malloc failed\n");
		ret = MAPI_ERR_NOMEM;
		goto err;
	}

	vdecCtx->codec = attr->codec;
	vdecCtx->vdecChn = vdecChn;
	encodeType = MAPI_VCODEC_TO_PAYLOAD_TYPE[attr->codec];
	CVI_LOG_ASSERT(encodeType < PT_BUTT, "Invalid codec %d\n", attr->codec);

	stChnAttr.enType = encodeType;
	stChnAttr.enMode = VIDEO_MODE_FRAME;
	stChnAttr.u32PicWidth = attr->max_width;
	stChnAttr.u32PicHeight = attr->max_height;
	stChnAttr.u32StreamBufSize = ALIGN(attr->max_width*attr->max_height, 0x4000);
	stChnAttr.u32FrameBufCnt = gFrameBufCnt;
	stChnAttr.u32FrameBufSize = VDEC_GetPicBufferSize(
		stChnAttr.enType, attr->max_width, attr->max_height,
		PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);

	if (gVbMode == VB_SOURCE_USER) {
		stVbPoolCfg.u32BlkSize  = stChnAttr.u32FrameBufSize;
		stVbPoolCfg.u32BlkCnt	= stChnAttr.u32FrameBufCnt;
		stVbPoolCfg.enRemapMode = VB_REMAP_MODE_NONE;

		gPicVbPool[vdecChn] = CVI_VB_CreatePool(&stVbPoolCfg);
		if (gPicVbPool[vdecChn] == VB_INVALID_POOLID) {
			CVI_LOGE("CVI_VB_CreatePool Fail\n");
			ret = MAPI_ERR_FAILURE;
			goto err;
		}
	}

	if (gVbMode != VB_SOURCE_COMMON) {
		CVI_VDEC_GetModParam(&stModParam);
		stModParam.enVdecVBSource = gVbMode;
		CVI_VDEC_SetModParam(&stModParam);
	}

	ret = CVI_VDEC_CreateChn(vdecChn, &stChnAttr);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_CreateChn Fail: 0x%x\n", ret);
		goto err;
	}

	if (gVbMode == VB_SOURCE_USER) {
		VDEC_CHN_POOL_S stPool;

		stPool.hPicVbPool = gPicVbPool[vdecChn];
		stPool.hTmvVbPool = VB_INVALID_POOLID;

		ret = CVI_VDEC_AttachVbPool(vdecChn, &stPool);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VDEC_AttachVbPool Fail: 0x%x\n", ret);
			goto err;
		}
	}

	ret = CVI_VDEC_GetChnParam(vdecChn, &stChnParam);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_GetChnParam Fail: 0x%x\n", ret);
		goto err;
	}

	stChnParam.enType = encodeType;
	stChnParam.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;

	ret = CVI_VDEC_SetChnParam(vdecChn, &stChnParam);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_SetChnParam Fail: 0x%x\n", ret);
		goto err;
	}

	ret = CVI_VDEC_StartRecvStream(vdecChn);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_StartRecvStream Fail: 0x%x\n", ret);
		goto err;
	}

	CVI_LOGI("VDEC chn %d created\n", vdecChn);

	*vdec_hdl = (MAPI_VDEC_HANDLE_T)vdecCtx;
	return MAPI_SUCCESS;

err:
	_MAPI_VDEC_ReleaseChan(vdecChn, attr->codec);
	if (vdecCtx != NULL) {
		free(vdecCtx);
	}

	return ret;
}

int MAPI_VDEC_DeinitChn(MAPI_VDEC_HANDLE_T vdec_hdl)
{
	MAPI_VDEC_CTX_T *vdecCtx = (MAPI_VDEC_CTX_T *)vdec_hdl;
	int ret = MAPI_SUCCESS;

	ret = CVI_VDEC_StopRecvStream(vdecCtx->vdecChn);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_StopRecvStream Fail: 0x%x\n", ret);
		return ret;
	}

	if (gVbMode == VB_SOURCE_USER) {
		ret = CVI_VDEC_DetachVbPool(vdecCtx->vdecChn);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VDEC_DetachVbPool Fail: 0x%x\n", ret);
			return ret;
		}
		ret = CVI_VB_DestroyPool(gPicVbPool[vdecCtx->vdecChn]);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VB_DestroyPool Fail: 0x%x\n", ret);
			return ret;
		}
		gPicVbPool[vdecCtx->vdecChn] = VB_INVALID_POOLID;
	}

	ret = CVI_VDEC_DestroyChn(vdecCtx->vdecChn);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_DestroyChn Fail: 0x%x\n", ret);
		return ret;
	}

	_MAPI_VDEC_ReleaseChan(vdecCtx->vdecChn, vdecCtx->codec);

	CVI_LOGI("VDEC chn %d destroyed\n", vdecCtx->vdecChn);

	if (vdecCtx != NULL) {
		free(vdecCtx);
		vdecCtx = NULL;
	}

	return MAPI_SUCCESS;
}

int MAPI_VDEC_GetChn(MAPI_VDEC_HANDLE_T vdec_hdl)
{
	MAPI_VDEC_CTX_T *vdecCtx = (MAPI_VDEC_CTX_T *)vdec_hdl;

	return vdecCtx->vdecChn;
}

int MAPI_VDEC_SendStream(MAPI_VDEC_HANDLE_T vdec_hdl, VDEC_STREAM_S *stream)
{
	MAPI_VDEC_CTX_T *vdecCtx = (MAPI_VDEC_CTX_T *)vdec_hdl;
	int ret;

	ret = CVI_VDEC_SendStream(vdecCtx->vdecChn, stream, 2000);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_SendStream failed, %d\n", ret);
		return ret;
	}

	return MAPI_SUCCESS;
}

int MAPI_VDEC_GetFrame(MAPI_VDEC_HANDLE_T vdec_hdl, VIDEO_FRAME_INFO_S *frame)
{
	MAPI_VDEC_CTX_T *vdecCtx = (MAPI_VDEC_CTX_T *)vdec_hdl;
	int ret;

	ret = CVI_VDEC_GetFrame(vdecCtx->vdecChn, frame, 0);
	if (ret != CVI_SUCCESS) {
		//CVI_LOGE("VDEC_GetFrame failed, %d\n", ret);
		return ret;
	}

	return MAPI_SUCCESS;
}

int MAPI_VDEC_ReleaseFrame(MAPI_VDEC_HANDLE_T vdec_hdl, VIDEO_FRAME_INFO_S *frame)
{
	MAPI_VDEC_CTX_T *vdecCtx = (MAPI_VDEC_CTX_T *)vdec_hdl;
	int ret;

	ret = CVI_VDEC_ReleaseFrame(vdecCtx->vdecChn, frame);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VDEC_ReleaseFrame failed, %d\n", ret);
		return ret;
    }

    return MAPI_SUCCESS;
}

int MAPI_VDEC_IsCodecSupported(MAPI_VCODEC_E codec)
{
	if (codec < MAPI_VCODEC_MAX && codec >= 0) {
		#ifdef CHIP_184X
		if (codec == MAPI_VCODEC_JPG ||  codec == MAPI_VCODEC_MJP) {
			return MAPI_SUCCESS;
		}
		#else
		if (codec == MAPI_VCODEC_JPG || codec == MAPI_VCODEC_H264 ||
			codec == MAPI_VCODEC_H265 || codec == MAPI_VCODEC_MJP) {
			return MAPI_SUCCESS;
		}
		#endif

		CVI_LOGE("VDEC do not support %d\n", codec);
		return MAPI_ERR_INVALID;
	} else {
		CVI_LOGE("VDEC do not support %d\n", codec);
		return MAPI_ERR_INVALID;
	}
}


#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>

// #include "cvi_common.h"
#include "cvi_venc.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "mapi_venc.h"
#include "osal.h"
#include "cvi_log.h"
#include "mapi.h"

typedef enum _MAPI_RC_E {
	MAPI_RC_CBR = 0,
	MAPI_RC_VBR,
	MAPI_RC_AVBR,
	MAPI_RC_QVBR,
	MAPI_RC_FIXQP,
	MAPI_RC_QPMAP,
	MAPI_RC_MAX
} MAPI_RC_E;

#define MULTI_VCODEC_PRO_MAX VENC_MAX_CHN_NUM

static int gVencChnUsed[MULTI_VCODEC_PRO_MAX] = {0};
static int gVencChnBind[MULTI_VCODEC_PRO_MAX] = {0};
static OSAL_MUTEX_S gVencMutex[MAPI_VCODEC_MAX] = {
	[0 ... (MAPI_VCODEC_MAX - 1)] = OSAL_MUTEX_INITIALIZER};
VB_POOL gVencPicVbPool[VB_MAX_COMM_POOLS] = { [0 ...(VB_MAX_COMM_POOLS - 1)] = VB_INVALID_POOLID };

static CVI_S32 _MAPI_VENC_GetValidChan(MAPI_VCODEC_E codec, CVI_BOOL used, VENC_CHN *vencChn)
{
	CVI_U32 i;

	OSAL_MUTEX_Lock(&gVencMutex[codec]);

	for (i = 0; i < MULTI_VCODEC_PRO_MAX; i++) {
		if (gVencChnUsed[i] == 0)
			break;
    }

	if (i >= MULTI_VCODEC_PRO_MAX) {
		CVI_LOGE("VDEC support %d chn at most\n", MULTI_VCODEC_PRO_MAX);
		OSAL_MUTEX_Unlock(&gVencMutex[codec]);
		return MAPI_ERR_FAILURE;
	}

	gVencChnUsed[i] = used;
	*vencChn = i;
	OSAL_MUTEX_Unlock(&gVencMutex[codec]);

    return MAPI_SUCCESS;
}

static void _MAPI_VENC_ReleaseChan(VENC_CHN vencChn, MAPI_VCODEC_E codec)
{
	OSAL_MUTEX_Lock(&gVencMutex[codec]);

	if (vencChn < MULTI_VCODEC_PRO_MAX)
		gVencChnUsed[vencChn] = 0;

	OSAL_MUTEX_Unlock(&gVencMutex[codec]);

	return;
}
static int _MAPI_VENC_GetStream(MAPI_VENC_HANDLE_T venc_hdl, VENC_STREAM_S *stream, CVI_S32 S32MilliSec)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	CVI_S32 ret;

	stream->pstPack = vencCtx->packet;
	if (stream->pstPack == NULL) {
		CVI_LOGE("malloc memory failed!\n");
		ret = MAPI_ERR_NOMEM;
		goto err;
	}

	ret = CVI_VENC_GetStream(vencCtx->vencChn, stream, S32MilliSec);
    if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_GetStream %d failed! %d", vencCtx->vencChn, ret);
	}

err:
    if (!gVencChnBind[vencCtx->vencChn]) {
        OSAL_MUTEX_Unlock(&gVencMutex[vencCtx->attr.venc_param.codec]);
    }

	return ret;
}

static int _MAPI_VENC_ReleaseStream(MAPI_VENC_HANDLE_T venc_hdl, VENC_STREAM_S *stream)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	int ret;

	ret = CVI_VENC_ReleaseStream(vencCtx->vencChn, stream);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_ReleaseStream, s32Ret = %d\n", ret);
	}

	return ret;
}

static void _MAPI_VENC_CbLoop(void *p)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)p;
	struct timespec timeo;
	MAPI_VENC_HANDLE_T venc_hdl = (MAPI_VENC_HANDLE_T)vencCtx;
	VENC_STREAM_S stream = {0};
	CVI_S64 timeout_us = 100000; //100 ms
	int ret;

	while (!vencCtx->quit) {
		clock_gettime(CLOCK_REALTIME, &timeo);
		timeo.tv_sec += timeout_us / 1000000;
		timeo.tv_nsec += (timeout_us % 1000000) * 1000;
		if (timeo.tv_nsec > 1000000000) {
			timeo.tv_sec++;
			timeo.tv_nsec -= 1000000000;
		}
		ret = sem_timedwait(&vencCtx->cbSem, &timeo);
		if (ret == 0) {
			if (_MAPI_VENC_GetStream(venc_hdl, &stream, -1) == CVI_SUCCESS) {
				vencCtx->attr.cb.stream_cb_func(venc_hdl, &stream, vencCtx->attr.cb.stream_cb_data);
			}

            _MAPI_VENC_ReleaseStream(venc_hdl, &stream);

			OSAL_MUTEX_Unlock(vencCtx->cbMutex);
		} else if ((ret == -1) && (errno == ETIMEDOUT)) {
		} else {
			CVI_LOG_ASSERT(0, "sem_timedwait failed with ret %d, errno %d\n", ret, errno);
		}
	}
}
static CVI_S32 _MAPI_VENC_InitVBPool(MAPI_VENC_CHN_ATTR_T *attr)
{
	return MAPI_SUCCESS;
	VB_POOL_CONFIG_S stVbPoolCfg = {0};
	VB_CONFIG_S stVbConf = {0};
	CVI_U32 u32LumaSize;
	CVI_U32 u32LumaStride;
	CVI_U32 u32ChromaSize;
	VENC_CHN vencChn;

	if ((attr->venc_param.codec != MAPI_VCODEC_H264) && (attr->venc_param.codec != MAPI_VCODEC_H265)) {
		return MAPI_ERR_FAILURE;
	}

	if (_MAPI_VENC_GetValidChan(attr->venc_param.codec, 0, &vencChn) != MAPI_SUCCESS) {
		return MAPI_ERR_FAILURE;
	}

	if (gVencPicVbPool[vencChn] != VB_INVALID_POOLID) {
		return MAPI_SUCCESS;
	}

	u32LumaStride = (attr->venc_param.width < attr->venc_param.height) ? attr->venc_param.height : attr->venc_param.width;
	u32LumaStride = (u32LumaStride > 4096) ? 8192 :
			      (u32LumaStride > 2048) ? 4096 :
			      (u32LumaStride > 1024) ? 2048 :
			      (u32LumaStride > 512) ? 1024 : 512;

	u32LumaSize = ALIGN(u32LumaStride, 32) * ALIGN(attr->venc_param.height, 64);
	u32LumaSize = ALIGN(u32LumaSize, 4096);

	u32ChromaSize = ALIGN(u32LumaStride>>1, 32) * ALIGN(attr->venc_param.height>>1, 64);
	u32ChromaSize = ALIGN(u32ChromaSize, 4096);

	stVbPoolCfg.u32BlkSize	= u32LumaSize + u32ChromaSize*2;
	stVbPoolCfg.u32BlkCnt	= 3;
	stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
	gVencPicVbPool[vencChn] = CVI_VB_CreatePool(&stVbPoolCfg);
	if (gVencPicVbPool[vencChn] == VB_INVALID_POOLID) {
		return MAPI_ERR_FAILURE;
	}

	CVI_VB_GetConfig(&stVbConf);
	stVbConf.astCommPool[gVencPicVbPool[vencChn]].u32BlkSize = stVbPoolCfg.u32BlkSize;
	stVbConf.astCommPool[gVencPicVbPool[vencChn]].u32BlkCnt = stVbPoolCfg.u32BlkCnt;
	CVI_VB_SetConfig(&stVbConf);

	return MAPI_SUCCESS;
}

static CVI_S32 _MAPI_VENC_DeInitVBPool(VENC_CHN vencChn)
{
	VB_CONFIG_S stVbConf;

	if (gVencPicVbPool[vencChn] == VB_INVALID_POOLID) {
		return MAPI_SUCCESS;
	}

	if (CVI_VB_DestroyPool(gVencPicVbPool[vencChn]) != CVI_SUCCESS) {
		return MAPI_ERR_FAILURE;
	}

	CVI_VB_GetConfig(&stVbConf);
	stVbConf.astCommPool[gVencPicVbPool[vencChn]].u32BlkSize = 0;
	stVbConf.astCommPool[gVencPicVbPool[vencChn]].u32BlkCnt = 0;
	CVI_VB_SetConfig(&stVbConf);

	gVencPicVbPool[vencChn] = VB_INVALID_POOLID;
	CVI_LOGI("VENC chn vb pool %d destroyed\n", vencChn);

	return MAPI_SUCCESS;
}

static CVI_S32 _MAPI_VENC_SetModParam(VENC_CHN vencChn, MAPI_VENC_CHN_ATTR_T *attr)
{
	CVI_S32 ret;
	VB_SOURCE_E VbSource = VB_SOURCE_COMMON;
	VENC_PARAM_MOD_S stModParam;

	if (gVencPicVbPool[vencChn] != VB_INVALID_POOLID) {
		VbSource = VB_SOURCE_USER;
	}

	if (VbSource >= VB_SOURCE_BUTT)
		VbSource = VB_SOURCE_PRIVATE;

    if(attr->venc_param.codec == MAPI_VCODEC_H264){
        stModParam.enVencModType = MODTYPE_H264E;
    }else if(attr->venc_param.codec == MAPI_VCODEC_H265){
        stModParam.enVencModType = MODTYPE_H265E;
    }
    else if(attr->venc_param.codec == MAPI_VCODEC_JPG || attr->venc_param.codec == MAPI_VCODEC_MJP){
        stModParam.enVencModType = MODTYPE_JPEGE;
    }

    ret = CVI_VENC_GetModParam(&stModParam);
    if (ret != CVI_SUCCESS) {
        CVI_LOGE("VENC_GetModParam type %d failure\n", stModParam.enVencModType);
        return CVI_FAILURE;
    }

    switch (stModParam.enVencModType) {
    case MODTYPE_H264E:
        stModParam.stH264eModParam.enH264eVBSource = VbSource;
        stModParam.stH264eModParam.bSingleEsBuf = attr->venc_param.single_EsBuf;
        stModParam.stH264eModParam.u32SingleEsBufSize = attr->venc_param.bufSize;
        break;
    case MODTYPE_H265E:
        stModParam.stH265eModParam.enH265eVBSource = VbSource;
        stModParam.stH265eModParam.u32UserDataMaxLen = 3072;
        stModParam.stH265eModParam.bSingleEsBuf = attr->venc_param.single_EsBuf;
        stModParam.stH265eModParam.u32SingleEsBufSize = attr->venc_param.bufSize;
        break;
    case MODTYPE_JPEGE:
        stModParam.stJpegeModParam.bSingleEsBuf = attr->venc_param.single_EsBuf;
    #ifdef DDR_128M
        stModParam.stJpegeModParam.u32SingleEsBufSize = attr->venc_param.bufSize;
    #else
        stModParam.stJpegeModParam.u32SingleEsBufSize = attr->venc_param.bufSize;
    #endif
        break;
    default:
        CVI_LOGE("SAMPLE_COMM_VENC_SetModParam invalid type %d failure\n", stModParam.enVencModType);
        return CVI_FAILURE;
    }

    ret = CVI_VENC_SetModParam(&stModParam);
    if (ret != CVI_SUCCESS) {
        CVI_LOGE("VENC_SetModParam type %d failure\n", stModParam.enVencModType);
        return CVI_FAILURE;
    }

	return CVI_SUCCESS;

}

static CVI_S32 _MAPI_VENC_SetChnAttr(MAPI_VENC_CHN_ATTR_T *attr, VENC_CHN_ATTR_S *pstVencChnAttr)
{
	CVI_U32 u32Gop = 30;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_BOOL bRcnRefShareBuf = CVI_FALSE;
	CVI_BOOL bVariFpsEn = CVI_FALSE;
	PAYLOAD_TYPE_E enType;
	VENC_GOP_ATTR_S stGopAttr = {0};
	CVI_U32 u32BitstreamBufSize;

	enType = MAPI_VCODEC_TO_PAYLOAD_TYPE[attr->venc_param.codec];

	if (attr->venc_param.codec != MAPI_VCODEC_JPG) {
		stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
		stGopAttr.stNormalP.s32IPQpDelta = attr->venc_param.ipqpDelta;
    }

	if (attr->venc_param.bufSize != 0) {
		u32BitstreamBufSize = attr->venc_param.bufSize;
	} else if ((attr->venc_param.width * attr->venc_param.height) > 0x400000) {
		u32BitstreamBufSize = 0;
	} else if ((attr->venc_param.width * attr->venc_param.height) > 0x200000) {
		u32BitstreamBufSize = 0x200000;
	} else if ((attr->venc_param.width * attr->venc_param.height) > 0x100000) {
		u32BitstreamBufSize = 0x100000;
	} else {
		u32BitstreamBufSize = 0x80000;
	}

	pstVencChnAttr->stVencAttr.enType = enType;
	pstVencChnAttr->stVencAttr.u32MaxPicWidth = attr->venc_param.width;
	pstVencChnAttr->stVencAttr.u32MaxPicHeight = attr->venc_param.height;
	pstVencChnAttr->stVencAttr.u32PicWidth = attr->venc_param.width;
	pstVencChnAttr->stVencAttr.u32PicHeight = attr->venc_param.height;
	pstVencChnAttr->stVencAttr.u32BufSize = u32BitstreamBufSize;
#ifdef DDR_128M
	pstVencChnAttr->stVencAttr.bSingleCore = CVI_TRUE;
#else
	pstVencChnAttr->stVencAttr.bSingleCore = CVI_FALSE;
#endif
	pstVencChnAttr->stVencAttr.u32Profile = attr->venc_param.profile;
	pstVencChnAttr->stVencAttr.bByFrame = CVI_TRUE;

	switch (enType) {
	case PT_H265: {
		#ifndef CHIP_184X
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_FALSE;
		#else
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_TRUE;
		#endif
		pstVencChnAttr->stVencAttr.bEsBufQueueEn = CVI_TRUE;
		if (attr->venc_param.rate_ctrl_mode == MAPI_RC_CBR) {
			VENC_H265_CBR_S *pstH265Cbr = &pstVencChnAttr->stRcAttr.stH265Cbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
			pstH265Cbr->u32Gop = attr->venc_param.gop;
			pstH265Cbr->u32StatTime = attr->venc_param.statTime;
			pstH265Cbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH265Cbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH265Cbr->bVariFpsEn = bVariFpsEn;
			pstH265Cbr->u32BitRate = attr->venc_param.bitrate_kbps;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_FIXQP) {
			VENC_H265_FIXQP_S *pstH265FixQp = &pstVencChnAttr->stRcAttr.stH265FixQp;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265FIXQP;
			pstH265FixQp->u32Gop = attr->venc_param.gop;
			pstH265FixQp->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH265FixQp->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH265FixQp->bVariFpsEn = bVariFpsEn;
			pstH265FixQp->u32IQp = attr->venc_param.iqp;
			pstH265FixQp->u32PQp = attr->venc_param.pqp;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_VBR) {
			VENC_H265_VBR_S *pstH265Vbr = &pstVencChnAttr->stRcAttr.stH265Vbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
			pstH265Vbr->u32Gop = attr->venc_param.gop;
			pstH265Vbr->u32StatTime = attr->venc_param.statTime;
			pstH265Vbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH265Vbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH265Vbr->bVariFpsEn = bVariFpsEn;
			pstH265Vbr->u32MaxBitRate = attr->venc_param.maxBitRate;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_AVBR) {
			VENC_H265_AVBR_S *pstH265AVbr = &pstVencChnAttr->stRcAttr.stH265AVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265AVBR;
			pstH265AVbr->u32Gop = attr->venc_param.gop;
			pstH265AVbr->u32StatTime = attr->venc_param.statTime;
			pstH265AVbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH265AVbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH265AVbr->bVariFpsEn = bVariFpsEn;
			pstH265AVbr->u32MaxBitRate = attr->venc_param.maxBitRate;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_QVBR) {
			VENC_H265_QVBR_S stH265QVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QVBR;
			stH265QVbr.u32Gop = u32Gop;
			stH265QVbr.u32StatTime = attr->venc_param.statTime;
			stH265QVbr.u32SrcFrameRate = attr->venc_param.src_framerate;
			stH265QVbr.fr32DstFrameRate = attr->venc_param.dst_framerate;
			if ((attr->venc_param.width == 1280) && (attr->venc_param.height == 720))
				stH265QVbr.u32TargetBitRate = 1024 * 2 + 1024 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 1920) && (attr->venc_param.height == 1080))
				stH265QVbr.u32TargetBitRate = 1024 * 2 + 2048 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 2592) && (attr->venc_param.height == 1944))
				stH265QVbr.u32TargetBitRate = 1024 * 3 + 3072 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 3840) && (attr->venc_param.height == 2160))
				stH265QVbr.u32TargetBitRate = 1024 * 5 + 5120 * attr->venc_param.dst_framerate / 30;
			else
				stH265QVbr.u32TargetBitRate = 1024 * 15 + 2048 * attr->venc_param.dst_framerate / 30;

			memcpy(&pstVencChnAttr->stRcAttr.stH265QVbr, &stH265QVbr, sizeof(VENC_H265_QVBR_S));
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_QPMAP) {
			VENC_H265_QPMAP_S stH265QpMap;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H265QPMAP;
			stH265QpMap.u32Gop = u32Gop;
			stH265QpMap.u32StatTime = attr->venc_param.statTime;
			stH265QpMap.u32SrcFrameRate = attr->venc_param.src_framerate;
			stH265QpMap.fr32DstFrameRate = attr->venc_param.dst_framerate;
			stH265QpMap.enQpMapMode = VENC_RC_QPMAP_MODE_MEANQP;
			memcpy(&pstVencChnAttr->stRcAttr.stH265QpMap, &stH265QpMap, sizeof(VENC_H265_QPMAP_S));
		} else {
			CVI_LOGE("enRcMode(%d) not support\n", attr->venc_param.rate_ctrl_mode);
			return CVI_FAILURE;
		}
		#ifdef CHIP_184X
		pstVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = CVI_TRUE;
		#else
		pstVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = bRcnRefShareBuf;
		pstVencChnAttr->stVencAttr.stAttrH265e.addrRemapEn = CVI_FALSE;
		#endif
	} break;
	case PT_H264: {
		#ifndef CHIP_184X
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_FALSE;
		#else
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_TRUE;
		#endif
		pstVencChnAttr->stVencAttr.bEsBufQueueEn = CVI_TRUE;
		if (attr->venc_param.rate_ctrl_mode == MAPI_RC_CBR) {
			VENC_H264_CBR_S *pstH264Cbr = &pstVencChnAttr->stRcAttr.stH264Cbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
			pstH264Cbr->u32Gop = attr->venc_param.gop;
			pstH264Cbr->u32StatTime = attr->venc_param.statTime;
			pstH264Cbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH264Cbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH264Cbr->bVariFpsEn = bVariFpsEn;
			pstH264Cbr->u32BitRate = attr->venc_param.bitrate_kbps;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_FIXQP) {
			VENC_H264_FIXQP_S *pstH264FixQp = &pstVencChnAttr->stRcAttr.stH264FixQp;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
			pstH264FixQp->u32Gop = attr->venc_param.gop;
			pstH264FixQp->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH264FixQp->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH264FixQp->bVariFpsEn = bVariFpsEn;
			pstH264FixQp->u32IQp = attr->venc_param.iqp;
			pstH264FixQp->u32PQp = attr->venc_param.pqp;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_VBR) {
			VENC_H264_VBR_S *pstH264Vbr = &pstVencChnAttr->stRcAttr.stH264Vbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264VBR;
			pstH264Vbr->u32Gop = attr->venc_param.gop;
			pstH264Vbr->u32StatTime = attr->venc_param.statTime;
			pstH264Vbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH264Vbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH264Vbr->bVariFpsEn = bVariFpsEn;
			pstH264Vbr->u32MaxBitRate = attr->venc_param.maxBitRate;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_AVBR) {
			VENC_H264_AVBR_S *pstH264AVbr = &pstVencChnAttr->stRcAttr.stH264AVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264AVBR;
			pstH264AVbr->u32Gop = attr->venc_param.gop;
			pstH264AVbr->u32StatTime = attr->venc_param.statTime;
			pstH264AVbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstH264AVbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstH264AVbr->bVariFpsEn = bVariFpsEn;
			pstH264AVbr->u32MaxBitRate = attr->venc_param.maxBitRate;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_QVBR) {
			VENC_H264_QVBR_S stH264QVbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264QVBR;
			stH264QVbr.u32Gop = u32Gop;
			stH264QVbr.u32StatTime = attr->venc_param.statTime;
			stH264QVbr.u32SrcFrameRate = attr->venc_param.src_framerate;
			stH264QVbr.fr32DstFrameRate = attr->venc_param.dst_framerate;
			if ((attr->venc_param.width == 1280) && (attr->venc_param.height == 720))
				stH264QVbr.u32TargetBitRate = 1024 * 2 + 1024 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 1920) && (attr->venc_param.height == 1080))
				stH264QVbr.u32TargetBitRate = 1024 * 2 + 2048 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 2592) && (attr->venc_param.height == 1944))
				stH264QVbr.u32TargetBitRate = 1024 * 3 + 3072 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 3840) && (attr->venc_param.height == 2160))
				stH264QVbr.u32TargetBitRate = 1024 * 5 + 5120 * attr->venc_param.dst_framerate / 30;
			else
				stH264QVbr.u32TargetBitRate = 1024 * 15 + 2048 * attr->venc_param.dst_framerate / 30;

			memcpy(&pstVencChnAttr->stRcAttr.stH264QVbr, &stH264QVbr, sizeof(VENC_H264_QVBR_S));
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_QPMAP) {
			VENC_H264_QPMAP_S stH264QpMap;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_H264QPMAP;
			stH264QpMap.u32Gop = u32Gop;
			stH264QpMap.u32StatTime = attr->venc_param.statTime;
			stH264QpMap.u32SrcFrameRate = attr->venc_param.src_framerate;
			stH264QpMap.fr32DstFrameRate = attr->venc_param.dst_framerate;
			memcpy(&pstVencChnAttr->stRcAttr.stH264QpMap, &stH264QpMap, sizeof(VENC_H264_QPMAP_S));
		} else {
			CVI_LOGE("enRcMode(%d) not support\n", attr->venc_param.rate_ctrl_mode);
			return CVI_FAILURE;
		}

		#ifdef CHIP_184X
		pstVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = CVI_TRUE;
		#else
		pstVencChnAttr->stVencAttr.stAttrH264e.bRcnRefShareBuf = bRcnRefShareBuf;
		#endif
        if ((attr->venc_param.width >= 1080) && (attr->venc_param.height >= 1080)){
			#ifndef CHIP_184X
            pstVencChnAttr->stVencAttr.stAttrH264e.addrRemapEn = CVI_TRUE;
			#endif
            pstVencChnAttr->stVencAttr.stAttrH264e.bSingleLumaBuf = CVI_FALSE;
        }else{
			#ifndef CHIP_184X
            pstVencChnAttr->stVencAttr.stAttrH264e.addrRemapEn = CVI_FALSE;
			#endif
            pstVencChnAttr->stVencAttr.stAttrH264e.bSingleLumaBuf = CVI_TRUE;
        }
	} break;
	case PT_MJPEG: {
		#ifndef CHIP_184X
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_FALSE;
		#else
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_TRUE;
		#endif
		pstVencChnAttr->stVencAttr.bEsBufQueueEn = CVI_FALSE;
		if (attr->venc_param.rate_ctrl_mode == MAPI_RC_FIXQP) {
			VENC_MJPEG_FIXQP_S *pstMjpegeFixQp = &pstVencChnAttr->stRcAttr.stMjpegFixQp;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
			pstMjpegeFixQp->u32Qfactor = attr->venc_param.jpeg_quality;
			pstMjpegeFixQp->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstMjpegeFixQp->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstMjpegeFixQp->bVariFpsEn = bVariFpsEn;
		} else if (attr->venc_param.rate_ctrl_mode == MAPI_RC_CBR) {
			VENC_MJPEG_CBR_S *pstMjpegeCbr = &pstVencChnAttr->stRcAttr.stMjpegCbr;

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
			pstMjpegeCbr->u32StatTime = attr->venc_param.statTime;
			pstMjpegeCbr->u32SrcFrameRate = attr->venc_param.src_framerate;
			pstMjpegeCbr->fr32DstFrameRate = attr->venc_param.dst_framerate;
			pstMjpegeCbr->bVariFpsEn = bVariFpsEn;
			pstMjpegeCbr->u32BitRate = attr->venc_param.bitrate_kbps;
		} else if ((attr->venc_param.rate_ctrl_mode == MAPI_RC_VBR) ||
					(attr->venc_param.rate_ctrl_mode == MAPI_RC_AVBR) ||
					(attr->venc_param.rate_ctrl_mode == MAPI_RC_QVBR)) {
			VENC_MJPEG_VBR_S stMjpegVbr;

			if (attr->venc_param.rate_ctrl_mode == MAPI_RC_AVBR)
				CVI_LOGI("Mjpege not support AVBR, so change rcmode to VBR!\n");

			pstVencChnAttr->stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
			stMjpegVbr.u32StatTime = attr->venc_param.statTime;
			stMjpegVbr.u32SrcFrameRate = attr->venc_param.src_framerate;
			stMjpegVbr.fr32DstFrameRate = 5;

			if ((attr->venc_param.width == 1280) && (attr->venc_param.height == 720))
				stMjpegVbr.u32MaxBitRate = 1024 * 5 + 1024 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 1920) && (attr->venc_param.height == 1080))
				stMjpegVbr.u32MaxBitRate = 1024 * 8 + 2048 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 2592) && (attr->venc_param.height == 1944))
				stMjpegVbr.u32MaxBitRate = 1024 * 20 + 3072 * attr->venc_param.dst_framerate / 30;
			else if ((attr->venc_param.width == 3840) && (attr->venc_param.height == 2160))
				stMjpegVbr.u32MaxBitRate = 1024 * 25 + 5120 * attr->venc_param.dst_framerate / 30;
			else
				stMjpegVbr.u32MaxBitRate = 1024 * 20 + 2048 * attr->venc_param.dst_framerate / 30;

			memcpy(&pstVencChnAttr->stRcAttr.stMjpegVbr, &stMjpegVbr, sizeof(VENC_MJPEG_VBR_S));
		} else {
			CVI_LOGE("cann't support other mode(%d) in this version!\n", attr->venc_param.rate_ctrl_mode);
			return CVI_FAILURE;
		}
	} break;

	case PT_JPEG: {
		#ifndef CHIP_184X
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_FALSE;
		#else
		pstVencChnAttr->stVencAttr.bIsoSendFrmEn = CVI_TRUE;
		#endif
		pstVencChnAttr->stVencAttr.bEsBufQueueEn = CVI_FALSE;
		VENC_ATTR_JPEG_S *pstJpegAttr = &pstVencChnAttr->stVencAttr.stAttrJpege;

		pstJpegAttr->bSupportDCF = CVI_FALSE;
		pstJpegAttr->stMPFCfg.u8LargeThumbNailNum = 0;
		pstJpegAttr->enReceiveMode = VENC_PIC_RECEIVE_SINGLE;
	} break;

	default:
		CVI_LOGE("cann't support this enType (%d) in this version!\n", enType);
		return CVI_ERR_VENC_NOT_SUPPORT;
	}

	if (PT_MJPEG == enType || PT_JPEG == enType) {
		pstVencChnAttr->stGopAttr.enGopMode = VENC_GOPMODE_NORMALP;
		pstVencChnAttr->stGopAttr.stNormalP.s32IPQpDelta = 0;
	} else {
		memcpy(&pstVencChnAttr->stGopAttr, &stGopAttr, sizeof(VENC_GOP_ATTR_S));
	}
    CVI_LOGD("pstVencChnAttr->stVencAttr.u32BufSize = %u", pstVencChnAttr->stVencAttr.u32BufSize);
	return ret;
}

static CVI_S32 _MAPI_VENC_Create(VENC_CHN vencChn, MAPI_VENC_CHN_ATTR_T *attr)
{
	CVI_S32 ret;
	VENC_CHN_ATTR_S stVencChnAttr = {0};
	VENC_CHN_PARAM_S stChnParam = {0};
	VENC_JPEG_PARAM_S stJpegParam = {0};
	VENC_RC_PARAM_S stRcParam = {0};
	VENC_REF_PARAM_S stRefParam = {0};
	VENC_FRAMELOST_S stFL = {0};

	ret = _MAPI_VENC_SetChnAttr(attr, &stVencChnAttr);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("Get picture size failed!\n");
		return CVI_FAILURE;
	}

	if (attr->sbm_enable == CVI_TRUE) {
		MMF_CHN_S stSrcChn;
		memset(&stSrcChn, 0, sizeof(MMF_CHN_S));
		stSrcChn.enModId = CVI_ID_VPSS;
		stSrcChn.s32DevId = attr->BindVprocId;
		stSrcChn.s32ChnId = attr->BindVprocChnId;

		MMF_CHN_S stdstChn;
		memset(&stdstChn, 0, sizeof(MMF_CHN_S));
		stdstChn.enModId = CVI_ID_VENC;
		stdstChn.s32DevId = 0;
		stdstChn.s32ChnId = vencChn;
		ret = CVI_SYS_Bind(&stSrcChn, &stdstChn);

		if (ret != CVI_SUCCESS) {
			CVI_LOGE("CVI_SYS_Bind failed with %#x\n", ret);
			return ret;
		}

		gVencChnBind[vencChn] = CVI_TRUE;
	}

	ret = CVI_VENC_CreateChn(vencChn, &stVencChnAttr);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_CreateChn [%d] failed with %d\n", vencChn, ret);
		return ret;
	}

	if (attr->venc_param.codec != MAPI_VCODEC_JPG) {
		ret = CVI_VENC_GetRcParam(vencChn, &stRcParam);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("GetRcParam failed!\n");
			goto ERR_VENC_CREATE;
		}

		stRcParam.s32FirstFrameStartQp = attr->venc_param.firstFrameStartQp;
		stRcParam.s32InitialDelay = attr->venc_param.initialDelay;
		stRcParam.u32ThrdLv = attr->venc_param.thrdLv;
        stRcParam.s32BgDeltaQp = attr->venc_param.bgDeltaQp;
        stRcParam.u32RowQpDelta = attr->venc_param.rowQpDelta;
		if ((attr->venc_param.codec == MAPI_VCODEC_H264) &&
			(attr->venc_param.rate_ctrl_mode == MAPI_RC_CBR)) {
			stRcParam.stParamH264Cbr.u32MaxIQp = attr->venc_param.maxIqp;
			stRcParam.stParamH264Cbr.u32MinIQp = attr->venc_param.minIqp;
			stRcParam.stParamH264Cbr.u32MaxQp = attr->venc_param.maxQp;
			stRcParam.stParamH264Cbr.u32MinQp = attr->venc_param.minQp;
			stRcParam.stParamH264Cbr.u32MaxIprop = attr->venc_param.maxIprop;
            stRcParam.stParamH264Cbr.u32MinIprop = attr->venc_param.minIprop;
		} else if ((attr->venc_param.codec == MAPI_VCODEC_H265) &&
			(attr->venc_param.rate_ctrl_mode == MAPI_RC_CBR)) {
			stRcParam.stParamH265Cbr.u32MaxIQp = attr->venc_param.maxIqp;
			stRcParam.stParamH265Cbr.u32MinIQp = attr->venc_param.minIqp;
			stRcParam.stParamH265Cbr.u32MaxQp = attr->venc_param.maxQp;
			stRcParam.stParamH265Cbr.u32MinQp = attr->venc_param.minQp;
			stRcParam.stParamH265Cbr.u32MaxIprop = attr->venc_param.maxIprop;
            stRcParam.stParamH265Cbr.u32MinIprop = attr->venc_param.minIprop;
		}  else if ((attr->venc_param.codec == MAPI_VCODEC_H264) &&
			(attr->venc_param.rate_ctrl_mode == MAPI_RC_VBR)) {
			stRcParam.stParamH264Vbr.u32MaxIQp = attr->venc_param.maxIqp;
			stRcParam.stParamH264Vbr.u32MinIQp = attr->venc_param.minIqp;
			stRcParam.stParamH264Vbr.u32MaxQp = attr->venc_param.maxQp;
			stRcParam.stParamH264Vbr.u32MinQp = attr->venc_param.minQp;
            stRcParam.stParamH264Vbr.u32MaxIprop = attr->venc_param.maxIprop;
            stRcParam.stParamH264Vbr.u32MinIprop = attr->venc_param.minIprop;
			stRcParam.stParamH264Vbr.s32ChangePos = attr->venc_param.changePos;
		}  else if ((attr->venc_param.codec == MAPI_VCODEC_H265) &&
			(attr->venc_param.rate_ctrl_mode == MAPI_RC_VBR)) {
			stRcParam.stParamH265Vbr.u32MaxIQp = attr->venc_param.maxIqp;
			stRcParam.stParamH265Vbr.u32MinIQp = attr->venc_param.minIqp;
			stRcParam.stParamH265Vbr.u32MaxQp = attr->venc_param.maxQp;
			stRcParam.stParamH265Vbr.u32MinQp = attr->venc_param.minQp;
            stRcParam.stParamH265Vbr.u32MaxIprop = attr->venc_param.maxIprop;
            stRcParam.stParamH265Vbr.u32MinIprop = attr->venc_param.minIprop;
			stRcParam.stParamH265Vbr.s32ChangePos = attr->venc_param.changePos;
		}  else if ((attr->venc_param.codec == MAPI_VCODEC_H264) &&
			(attr->venc_param.rate_ctrl_mode == MAPI_RC_AVBR)) {
			stRcParam.stParamH264AVbr.u32MaxIQp = attr->venc_param.maxIqp;
			stRcParam.stParamH264AVbr.u32MinIQp = attr->venc_param.minIqp;
			stRcParam.stParamH264AVbr.u32MaxQp = attr->venc_param.maxQp;
			stRcParam.stParamH264AVbr.u32MinQp = attr->venc_param.minQp;
            stRcParam.stParamH264AVbr.u32MaxIprop = attr->venc_param.maxIprop;
            stRcParam.stParamH264AVbr.u32MinIprop = attr->venc_param.minIprop;
			stRcParam.stParamH264AVbr.s32ChangePos = attr->venc_param.changePos;
            stRcParam.stParamH264AVbr.s32MinStillPercent = attr->venc_param.minStillPercent;
            stRcParam.stParamH264AVbr.u32MaxStillQP = attr->venc_param.maxStillQP;
            stRcParam.stParamH264AVbr.s32AvbrPureStillThr = attr->venc_param.avbrPureStillThr;
            stRcParam.stParamH264AVbr.u32MotionSensitivity = attr->venc_param.motionSensitivity;
            stRcParam.stParamH264AVbr.s32AvbrFrmLostOpen = 0;
		}  else if ((attr->venc_param.codec == MAPI_VCODEC_H265) &&
			(attr->venc_param.rate_ctrl_mode == MAPI_RC_AVBR)) {
			stRcParam.stParamH265AVbr.u32MaxIQp = attr->venc_param.maxIqp;
			stRcParam.stParamH265AVbr.u32MinIQp = attr->venc_param.minIqp;
			stRcParam.stParamH265AVbr.u32MaxQp = attr->venc_param.maxQp;
			stRcParam.stParamH265AVbr.u32MinQp = attr->venc_param.minQp;
            stRcParam.stParamH265AVbr.u32MaxIprop = attr->venc_param.maxIprop;
            stRcParam.stParamH265AVbr.u32MinIprop = attr->venc_param.minIprop;
			stRcParam.stParamH265AVbr.s32ChangePos = attr->venc_param.changePos;
            stRcParam.stParamH265AVbr.s32MinStillPercent = attr->venc_param.minStillPercent;
            stRcParam.stParamH265AVbr.u32MaxStillQP = attr->venc_param.maxStillQP;
            stRcParam.stParamH265AVbr.s32AvbrPureStillThr = attr->venc_param.avbrPureStillThr;
            stRcParam.stParamH265AVbr.u32MotionSensitivity = attr->venc_param.motionSensitivity;
            stRcParam.stParamH265AVbr.s32AvbrFrmLostOpen = 0;
		}

		ret = CVI_VENC_SetRcParam(vencChn, &stRcParam);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("SetRcParam failed!\n");
			goto ERR_VENC_CREATE;
		}

		stRefParam.u32Base = 0;
		stRefParam.u32Enhance = 0;
		stRefParam.bEnablePred = CVI_TRUE;

		ret = CVI_VENC_SetRefParam(vencChn, &stRefParam);

		if (ret != CVI_SUCCESS) {
			CVI_LOGE("SetRrfParam failed!\n");
			goto ERR_VENC_CREATE;
		}

		stFL.bFrmLostOpen = CVI_FALSE;
		stFL.enFrmLostMode = FRMLOST_PSKIP;
		stFL.u32EncFrmGaps = 1;
		stFL.u32FrmLostBpsThr = attr->venc_param.bitrate_kbps * 1.2;

		ret = CVI_VENC_SetFrameLostStrategy(vencChn, &stFL);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("SetFrameLostStrategy failed!\n");
			goto ERR_VENC_CREATE;
		}
	}

	if ((attr->venc_param.codec != MAPI_VCODEC_JPG) && (attr->venc_param.codec != MAPI_VCODEC_MJP) &&
		(gVencPicVbPool[vencChn] != VB_INVALID_POOLID)) {
		VENC_CHN_POOL_S stPool;

		stPool.hPicVbPool = gVencPicVbPool[vencChn];
		stPool.hPicInfoVbPool = VB_INVALID_POOLID;
		ret = CVI_VENC_AttachVbPool(vencChn, &stPool);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_AttachVbPool, %d\n", ret);
			goto ERR_VENC_CREATE;
		}
	}

	stChnParam.stCropCfg.bEnable = CVI_FALSE;
	stChnParam.stCropCfg.stRect.s32X = 0;
	stChnParam.stCropCfg.stRect.s32Y = 0;
	stChnParam.stCropCfg.stRect.u32Width = attr->venc_param.width;
	stChnParam.stCropCfg.stRect.u32Height = attr->venc_param.height;

	stChnParam.stFrameRate.s32SrcFrmRate = attr->venc_param.src_framerate;
	stChnParam.stFrameRate.s32DstFrmRate = attr->venc_param.dst_framerate;

	ret = CVI_VENC_SetChnParam(vencChn, &stChnParam);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_SetChnParam fail\n");
		goto ERR_VENC_CREATE;
	}

	if (attr->venc_param.codec == MAPI_VCODEC_JPG) {
		ret = CVI_VENC_GetJpegParam(vencChn, &stJpegParam);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_GetJpegParam\n");
			goto ERR_VENC_CREATE;
		}

		stJpegParam.u32Qfactor = attr->venc_param.jpeg_quality;
		stJpegParam.u32MCUPerECS = 0;

		ret = CVI_VENC_SetJpegParam(vencChn, &stJpegParam);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_SetJpegParam fail\n");
			goto ERR_VENC_CREATE;
		}
	} else if (attr->venc_param.codec == MAPI_VCODEC_H264) {
		VENC_H264_VUI_S stH264Vui;
		ret = CVI_VENC_GetH264Vui(vencChn, &stH264Vui);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_GetH264Vui\n");
			goto ERR_VENC_CREATE;
		}

		stH264Vui.stVuiTimeInfo.timing_info_present_flag = 1;
		stH264Vui.stVuiTimeInfo.fixed_frame_rate_flag = 1;
		stH264Vui.stVuiTimeInfo.num_units_in_tick = 10;
		if ((attr->venc_param.src_framerate>>16) == 0) {
			stH264Vui.stVuiTimeInfo.time_scale = attr->venc_param.src_framerate * stH264Vui.stVuiTimeInfo.num_units_in_tick * 2;
		} else {
			uint32_t fpsDenom = attr->venc_param.src_framerate>>16;
			uint32_t fpsNum   = attr->venc_param.src_framerate&0xFFFF;
			stH264Vui.stVuiTimeInfo.time_scale = fpsNum * 2 * stH264Vui.stVuiTimeInfo.num_units_in_tick / fpsDenom;
		}
		stH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = attr->venc_param.aspectRatioInfoPresentFlag;
		stH264Vui.stVuiAspectRatio.aspect_ratio_idc = attr->venc_param.aspectRatioIdc;
		stH264Vui.stVuiAspectRatio.sar_width = attr->venc_param.sarWidth;
		stH264Vui.stVuiAspectRatio.sar_height = attr->venc_param.sarHeight;

		stH264Vui.stVuiAspectRatio.overscan_info_present_flag = attr->venc_param.overscanInfoPresentFlag;
		stH264Vui.stVuiAspectRatio.overscan_appropriate_flag = attr->venc_param.overscanAppropriateFlag;

		stH264Vui.stVuiVideoSignal.video_signal_type_present_flag = attr->venc_param.videoSignalTypePresentFlag;
		stH264Vui.stVuiVideoSignal.video_format = attr->venc_param.videoFormat;
		stH264Vui.stVuiVideoSignal.video_full_range_flag = attr->venc_param.videoFullRangeFlag;
		stH264Vui.stVuiVideoSignal.colour_description_present_flag = attr->venc_param.colourDescriptionPresentFlag;
		stH264Vui.stVuiVideoSignal.colour_primaries = attr->venc_param.colourPrimaries;
		stH264Vui.stVuiVideoSignal.transfer_characteristics = attr->venc_param.transferCharacteristics;
		stH264Vui.stVuiVideoSignal.matrix_coefficients = attr->venc_param.matrixCoefficients;

		ret = CVI_VENC_SetH264Vui(vencChn, &stH264Vui);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_SetH264Vui fail\n");
			goto ERR_VENC_CREATE;
		}
	}	else if (attr->venc_param.codec == MAPI_VCODEC_H265) {
		VENC_H265_VUI_S stH265Vui;
		ret = CVI_VENC_GetH265Vui(vencChn, &stH265Vui);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_GetH265Vui\n");
			goto ERR_VENC_CREATE;
		}

		stH265Vui.stVuiTimeInfo.timing_info_present_flag = 1;
		stH265Vui.stVuiTimeInfo.num_units_in_tick = 10;
		stH265Vui.stVuiTimeInfo.num_ticks_poc_diff_one_minus1 = 1;
		if ((attr->venc_param.src_framerate>>16) == 0) {
			stH265Vui.stVuiTimeInfo.time_scale = attr->venc_param.src_framerate * stH265Vui.stVuiTimeInfo.num_units_in_tick;
		} else {
			uint32_t fpsDenom = attr->venc_param.src_framerate>>16;
			uint32_t fpsNum   = attr->venc_param.src_framerate&0xFFFF;
			stH265Vui.stVuiTimeInfo.time_scale = fpsNum * stH265Vui.stVuiTimeInfo.num_units_in_tick / fpsDenom;
		}

		stH265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = attr->venc_param.aspectRatioInfoPresentFlag;
		stH265Vui.stVuiAspectRatio.aspect_ratio_idc = attr->venc_param.aspectRatioIdc;
		stH265Vui.stVuiAspectRatio.sar_width = attr->venc_param.sarWidth;
		stH265Vui.stVuiAspectRatio.sar_height = attr->venc_param.sarHeight;

		stH265Vui.stVuiAspectRatio.overscan_info_present_flag = attr->venc_param.overscanInfoPresentFlag;
		stH265Vui.stVuiAspectRatio.overscan_appropriate_flag = attr->venc_param.overscanAppropriateFlag;

		stH265Vui.stVuiVideoSignal.video_signal_type_present_flag = attr->venc_param.videoSignalTypePresentFlag;
		stH265Vui.stVuiVideoSignal.video_format = attr->venc_param.videoFormat;
		stH265Vui.stVuiVideoSignal.video_full_range_flag = attr->venc_param.videoFullRangeFlag;
		stH265Vui.stVuiVideoSignal.colour_description_present_flag = attr->venc_param.colourDescriptionPresentFlag;
		stH265Vui.stVuiVideoSignal.colour_primaries = attr->venc_param.colourPrimaries;
		stH265Vui.stVuiVideoSignal.transfer_characteristics = attr->venc_param.transferCharacteristics;
		stH265Vui.stVuiVideoSignal.matrix_coefficients = attr->venc_param.matrixCoefficients;

		ret = CVI_VENC_SetH265Vui(vencChn, &stH265Vui);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_SetH265Vui fail\n");
			goto ERR_VENC_CREATE;
		}
	}

	return ret;

ERR_VENC_CREATE:
	CVI_VENC_DestroyChn(vencChn);

	return ret;
}
static int _MAPI_VENC_CreateCbTask(MAPI_VENC_CTX_T *vencCtx)
{
	OSAL_MUTEX_ATTR_S mutexAttr;
	OSAL_TASK_ATTR_S taskAttr;
	static char ma_name[32] = {0};
	static char ta_name[32] = {0};
	CVI_S32 ret;

	if (!vencCtx->attr.cb.stream_cb_func) {
		return MAPI_SUCCESS;
	}

	vencCtx->quit = 0;
	sem_init(&vencCtx->cbSem, 0, 0);

	snprintf(ma_name, 32, "vnec_cb_mutex_%d", vencCtx->vencChn);
	mutexAttr.name = ma_name;
	ret = OSAL_MUTEX_Create(&mutexAttr, &vencCtx->cbMutex);
	if (ret != OSAL_SUCCESS) {
		CVI_LOGE("osal_mutex_create %s failed:0x%x\n", mutexAttr.name, ret);
		return ret;
	}

	snprintf(ta_name, 32, "vnec_cb_task_%d", vencCtx->vencChn);

	taskAttr.name = ta_name;
	taskAttr.entry = _MAPI_VENC_CbLoop;
	taskAttr.param = (void *)vencCtx;
	taskAttr.priority = OSAL_TASK_PRI_NORMAL;
	taskAttr.detached = false;
	ret = OSAL_TASK_Create(&taskAttr, &vencCtx->cbTask);
	if (ret != OSAL_SUCCESS) {
		CVI_LOGE("osal_task_create %s failed\n", taskAttr.name);
		OSAL_MUTEX_Destroy(vencCtx->cbMutex);
		return ret;
	}

	return MAPI_SUCCESS;
}
static int _MAPI_VENC_DestoryCbTask(MAPI_VENC_CTX_T *vencCtx)
{
	CVI_S32 ret;

	if (!vencCtx->attr.cb.stream_cb_func) {
		return MAPI_SUCCESS;
	}

	vencCtx->quit = 1;

	ret = OSAL_TASK_Join(vencCtx->cbTask);
	if (ret != OSAL_SUCCESS) {
		CVI_LOGE("osal_task_join failed:0x%x\n", ret);
	}

	ret = OSAL_MUTEX_Destroy(vencCtx->cbMutex);
	if (ret != OSAL_SUCCESS) {
		CVI_LOGE("osal_mutex_destroy failed:0x%x\n", ret);
	}

	ret = OSAL_TASK_Destroy(&vencCtx->cbTask);
	if (ret != OSAL_SUCCESS) {
		CVI_LOGE("osal_task_destroy failed:0x%x\n", ret);
	}
	return MAPI_SUCCESS;
}

static int _MAPI_VENC_InitChn(MAPI_VENC_HANDLE_T *venc_hdl, MAPI_VENC_CHN_ATTR_T *attr)
{
	MAPI_VENC_CTX_T *vencCtx;
	CVI_S32 ret;
	VENC_CHN vencChn;

    if (attr->venc_param.codec >= MAPI_VCODEC_MAX) {
		CVI_LOGE("VENC do not support %d\n", attr->venc_param.codec);
		return MAPI_ERR_INVALID;
	}

	if (_MAPI_VENC_GetValidChan(attr->venc_param.codec, 1, &vencChn) != MAPI_SUCCESS) {
        CVI_LOGE("_MAPI_VENC_GetValidChan failed\n");
        return MAPI_ERR_FAILURE;
	}

	vencCtx = (MAPI_VENC_CTX_T *)calloc(1, sizeof(MAPI_VENC_CTX_T));
	if (!vencCtx) {
		CVI_LOGE("calloc failed\n");
		_MAPI_VENC_ReleaseChan(vencChn, attr->venc_param.codec);
		return MAPI_ERR_NOMEM;
	}

	vencCtx->attr = *attr;
	vencCtx->vencChn = vencChn;

	ret = _MAPI_VENC_SetModParam(vencChn, attr);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_SetModParam failed for %#x!\n", ret);
		goto err;
	}

	ret = _MAPI_VENC_Create(vencChn, attr);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_Create failed with %d\n", ret);
		goto err;
	}

	ret = _MAPI_VENC_CreateCbTask(vencCtx);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_CreateCbTask failed with %d\n", ret);
		goto err;
	}

    vencCtx->packet = (VENC_PACK_S *)malloc(sizeof(VENC_PACK_S) * 8);
    if(vencCtx->packet == NULL){
        CVI_LOGE("vencCtx->packet malloc failed\n");
		goto err;
    }

    CVI_LOGD("VENC chn %d created\n", vencCtx->vencChn);

    *venc_hdl = (MAPI_VENC_HANDLE_T)vencCtx;

	if (attr->sbm_enable == CVI_TRUE) {
		ret = MAPI_VENC_StartRecvFrame(venc_hdl, -1);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("MAPI_VENC_StartRecvFrame failed with %d\n", ret);
			goto err;
		}
	}

    return MAPI_SUCCESS;

err:
	_MAPI_VENC_ReleaseChan(vencChn, attr->venc_param.codec);

	if (vencCtx)
		free(vencCtx);

	return ret;
}
static int _MAPI_VENC_DeinitChn(MAPI_VENC_HANDLE_T venc_hdl)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	CVI_S32 ret;

	ret = _MAPI_VENC_DestoryCbTask(vencCtx);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_DestoryCbTask vechn[%d] failed with %#x\n",
			vencCtx->vencChn, ret);
		return ret;
	}

	if (gVencPicVbPool[vencCtx->vencChn] != VB_INVALID_POOLID) {
		ret = CVI_VENC_DetachVbPool(vencCtx->vencChn);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VENC_DetachVbPool vechn[%d] failed with %#x!\n",
					vencCtx->vencChn, ret);
			return ret;
		}
	}

	ret = CVI_VENC_ResetChn(vencCtx->vencChn);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_ResetChn vechn[%d] failed with %#x!\n",
				vencCtx->vencChn, ret);
		return ret;
	}

	ret = CVI_VENC_DestroyChn(vencCtx->vencChn);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_DestroyChn vechn[%d] failed with %#x!\n",
				vencCtx->vencChn, ret);
		return ret;
	}

	_MAPI_VENC_ReleaseChan(vencCtx->vencChn, vencCtx->attr.venc_param.codec);

	CVI_LOGI("VENC chn %d destroyed\n", vencCtx->vencChn);

    if(vencCtx->packet != NULL){
        free(vencCtx->packet);
    }

	if (vencCtx)
		free(vencCtx);

	return ret;
}
int MAPI_VENC_InitChn(MAPI_VENC_HANDLE_T *venc_hdl, MAPI_VENC_CHN_ATTR_T *attr)
{
	_MAPI_VENC_InitVBPool(attr);

	return _MAPI_VENC_InitChn(venc_hdl, attr);
}
int MAPI_VENC_DeinitChn(MAPI_VENC_HANDLE_T venc_hdl)
{
	CVI_S32 ret;
	VENC_CHN vencChn;

	if (venc_hdl == NULL){
		return MAPI_ERR_INVALID;
	}

	vencChn = MAPI_VENC_GetChn(venc_hdl);
	CVI_LOGD("vencChn %d", vencChn);

	ret = _MAPI_VENC_DeinitChn(venc_hdl);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_DeinitChn vechn[%d] failed with %#x!\n",
				vencChn, ret);
		return ret;
	}

	ret = _MAPI_VENC_DeInitVBPool(vencChn);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_DeInitVBPool vechn[%d] failed with %#x!\n",
				vencChn, ret);
		return ret;
	}

	return ret;
}

int MAPI_VENC_GetChn(MAPI_VENC_HANDLE_T venc_hdl)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	return vencCtx->vencChn;
}

int MAPI_VENC_GetChnFd(MAPI_VENC_HANDLE_T venc_hdl)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	return CVI_VENC_GetFd(vencCtx->vencChn);
}


int MAPI_VENC_SendFrame(MAPI_VENC_HANDLE_T venc_hdl, VIDEO_FRAME_INFO_S *frame)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	CVI_S32 ret;

    if (gVencChnBind[vencCtx->vencChn]) {
        CVI_LOGE("Can not call MAPI_VENC_SendFrame in bind mode");
        return MAPI_ERR_INVALID;
    }

	if (vencCtx->attr.cb.stream_cb_func) {
		OSAL_MUTEX_Lock(vencCtx->cbMutex);
	}

	OSAL_MUTEX_Lock(&gVencMutex[vencCtx->attr.venc_param.codec]);

	ret = CVI_VENC_SendFrame(vencCtx->vencChn, frame, 50000);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_SendFrame failed! %#x\n", ret);
		OSAL_MUTEX_Unlock(&gVencMutex[vencCtx->attr.venc_param.codec]);
		return ret;
	}

	if (vencCtx->attr.cb.stream_cb_func) {
		sem_post(&vencCtx->cbSem);
	}

	return MAPI_SUCCESS;
}

int MAPI_VENC_GetStream(MAPI_VENC_HANDLE_T venc_hdl, VENC_STREAM_S *stream)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	if (vencCtx->attr.cb.stream_cb_func) {
		CVI_LOGE("Can not call MAPI_VENC_GetStream in callback mode\n");
		return MAPI_ERR_INVALID;
	}

	return _MAPI_VENC_GetStream(venc_hdl, stream, -1);
}

int MAPI_VENC_GetStreamTimeWait(MAPI_VENC_HANDLE_T venc_hdl, VENC_STREAM_S *stream, CVI_S32 S32MilliSec)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	if (vencCtx->attr.cb.stream_cb_func) {
		CVI_LOGE("Can not call MAPI_VENC_GetStream in callback mode\n");
		return MAPI_ERR_INVALID;
	}

	return _MAPI_VENC_GetStream(venc_hdl, stream, S32MilliSec);
}

int MAPI_VENC_EncodeFrame(MAPI_VENC_HANDLE_T venc_hdl, VIDEO_FRAME_INFO_S *frame, VENC_STREAM_S *stream)
{
	CVI_S32 ret;

	ret = MAPI_VENC_SendFrame(venc_hdl, frame);
	if (ret != MAPI_SUCCESS) {
		return ret;
	}

	return MAPI_VENC_GetStream(venc_hdl, stream);
}

int MAPI_VENC_ReleaseStream(MAPI_VENC_HANDLE_T venc_hdl, VENC_STREAM_S *stream)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	if (vencCtx->attr.cb.stream_cb_func) {
		CVI_LOGE("Can not call MAPI_VENC_ReleaseStream in callback mode\n");
		return MAPI_ERR_INVALID;
	}

	return _MAPI_VENC_ReleaseStream(venc_hdl, stream);
}

int MAPI_VENC_GetStreamStatus(MAPI_VENC_HANDLE_T venc_hdl, VENC_PACK_S *ppack, bool *is_I_frame)
{
	H265E_NALU_TYPE_E H265Type;
	H264E_NALU_TYPE_E H264Type;
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	*is_I_frame = 0;

	if ((vencCtx->attr.venc_param.codec != MAPI_VCODEC_H264) &&
		(vencCtx->attr.venc_param.codec != MAPI_VCODEC_H265)) {
		return MAPI_SUCCESS;
	}

	if (vencCtx->attr.venc_param.codec == MAPI_VCODEC_H265) {
		H265Type = ppack->DataType.enH265EType;
		if (H265Type == H265E_NALU_ISLICE ||
			H265Type == H265E_NALU_IDRSLICE ||
			H265Type == H265E_NALU_SPS ||
			H265Type == H265E_NALU_VPS ||
			H265Type == H265E_NALU_PPS ||
			H265Type == H265E_NALU_SEI){
			*is_I_frame = 1;
		}
	} else if (vencCtx->attr.venc_param.codec == MAPI_VCODEC_H264) {
		H264Type = ppack->DataType.enH264EType;
		if (H264Type == H264E_NALU_ISLICE ||
			H264Type == H264E_NALU_SPS ||
			H264Type == H264E_NALU_IDRSLICE ||
			H264Type == H264E_NALU_SEI ||
			H264Type == H264E_NALU_PPS){
			*is_I_frame = 1;
		}
	}

	return MAPI_SUCCESS;
}

int MAPI_VENC_RequestIDR(MAPI_VENC_HANDLE_T venc_hdl)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	CVI_S32 ret;

	ret = CVI_VENC_RequestIDR(vencCtx->vencChn, CVI_TRUE);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("VENC_RequestIDR failed! %#x\n", ret);
		return ret;
	}

	return MAPI_SUCCESS;
}

int MAPI_VENC_SetAttr(MAPI_VENC_HANDLE_T *venc_hdl, MAPI_VENC_CHN_ATTR_T *attr)
{
	CVI_S32 ret;

	ret = _MAPI_VENC_DeinitChn(*venc_hdl);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_DeinitChn failed! %#x\n", ret);
		return ret;
	}

	ret = _MAPI_VENC_InitChn(venc_hdl, attr);
	if (ret != CVI_SUCCESS) {
		CVI_LOGE("_MAPI_VENC_InitChn failed! %#x\n", ret);
		return ret;
    }

	return MAPI_SUCCESS;
}

int MAPI_VENC_GetAttr(MAPI_VENC_HANDLE_T venc_hdl, MAPI_VENC_CHN_ATTR_T *attr)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

	memcpy(attr, &vencCtx->attr, sizeof(MAPI_VENC_CHN_ATTR_T));

	return MAPI_SUCCESS;
}

int MAPI_VENC_GetJpegParam(MAPI_VENC_HANDLE_T venc_hdl, VENC_JPEG_PARAM_S *param)
{
	CVI_S32 ret = CVI_SUCCESS;
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	if(vencCtx == NULL || param == NULL) {
		CVI_LOGE("venc don't exist");
		return MAPI_ERR_INVALID;
	}

	ret = CVI_VENC_GetJpegParam(vencCtx->vencChn, param);
	if(ret != CVI_SUCCESS) {
		return MAPI_ERR_FAILURE;
	}

	return MAPI_SUCCESS;
}

int MAPI_VENC_SetJpegParam(MAPI_VENC_HANDLE_T venc_hdl, VENC_JPEG_PARAM_S *param)
{
	CVI_S32 ret = CVI_SUCCESS;
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	if(vencCtx == NULL || param == NULL) {
		CVI_LOGE("venc don't exist");
		return MAPI_ERR_INVALID;
	}

	ret = CVI_VENC_SetJpegParam(vencCtx->vencChn, param);
	if(ret != CVI_SUCCESS) {
		return MAPI_ERR_FAILURE;
	}

	return MAPI_SUCCESS;
}

int MAPI_VENC_BindVproc(MAPI_VENC_HANDLE_T venc_hdl, int VpssGrp, int VpssChn)
{
    MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
    if(vencCtx == NULL || vencCtx->vencChn >= MULTI_VCODEC_PRO_MAX){
        CVI_LOGE("bind failed %d %d", VpssGrp, VpssChn);
        return -1;
    }

    if (gVencChnBind[vencCtx->vencChn]) {
        CVI_LOGE("VENC Chn %d ,already bound", vencCtx->vencChn);
        return -1;
    }

    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;

    stSrcChn.enModId = CVI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = CVI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencCtx->vencChn;

    int rc = 0;
    if (0 != CVI_SYS_Bind(&stSrcChn, &stDestChn)) {
        CVI_LOGE("SYS_Bind fail, ret=%x", rc);
        return -1;
    }

    gVencChnBind[vencCtx->vencChn] = CVI_TRUE;

    return 0;
}

int MAPI_VENC_UnBindVproc(MAPI_VENC_HANDLE_T venc_hdl, int VpssGrp, int VpssChn)
{
    MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

    if (!gVencChnBind[vencCtx->vencChn]) {
        CVI_LOGE("VENC Chn %d ,Not yet bound", vencCtx->vencChn);
        return -1;
    }

    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;

    stSrcChn.enModId = CVI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = CVI_ID_VENC;
    stDestChn.s32DevId = 0;
    stDestChn.s32ChnId = vencCtx->vencChn;

    int rc = CVI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (CVI_SUCCESS != rc) {
        CVI_LOGE("SYS_UnBind fail, ret=%#x", rc);
        return -1;
    }

    gVencChnBind[vencCtx->vencChn] = CVI_FALSE;

    return 0;
}

int MAPI_VENC_StartRecvFrame(MAPI_VENC_HANDLE_T venc_hdl, CVI_S32 frameCnt)
{
    VENC_RECV_PIC_PARAM_S stRecvParam = {0};
    MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
    if(vencCtx == NULL || vencCtx->vencChn >= MULTI_VCODEC_PRO_MAX){
        CVI_LOGE("StartRecvFrame failed %p", vencCtx);
        return -1;
    }

    stRecvParam.s32RecvPicNum = frameCnt;
    int rc = CVI_VENC_StartRecvFrame(vencCtx->vencChn, &stRecvParam);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VENC_StartRecvFrame failed with %#x", rc);
        return -1;
    }

    return 0;
}

int MAPI_VENC_StopRecvFrame(MAPI_VENC_HANDLE_T venc_hdl)
{
    MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;

    CVI_S32 rc = CVI_VENC_StopRecvFrame(vencCtx->vencChn);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VENC_StopRecvPic vechn[%d] failed with %#x!", vencCtx->vencChn, rc);
        return -1;
    }

    return 0;
}

int MAPI_VENC_SetBitrate(MAPI_VENC_HANDLE_T venc_hdl, CVI_U32 bitRate)
{
	MAPI_VENC_CTX_T *vencCtx = (MAPI_VENC_CTX_T *)venc_hdl;
	VENC_CHN_ATTR_S stChnAttr = {0};

	CVI_VENC_GetChnAttr(vencCtx->vencChn, &stChnAttr);

	if (stChnAttr.stVencAttr.enType == PT_H265) {
		if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265CBR)
			stChnAttr.stRcAttr.stH265Cbr.u32BitRate = bitRate;
		else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265VBR)
			stChnAttr.stRcAttr.stH265Vbr.u32MaxBitRate = bitRate;
		else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H265AVBR)
			stChnAttr.stRcAttr.stH265AVbr.u32MaxBitRate = bitRate;
		else
			return MAPI_ERR_INVALID;
	} else if (stChnAttr.stVencAttr.enType == PT_H264) {
		if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264CBR)
			stChnAttr.stRcAttr.stH264Cbr.u32BitRate = bitRate;
		else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264VBR)
			stChnAttr.stRcAttr.stH264Vbr.u32MaxBitRate = bitRate;
		else if (stChnAttr.stRcAttr.enRcMode == VENC_RC_MODE_H264AVBR)
			stChnAttr.stRcAttr.stH264AVbr.u32MaxBitRate = bitRate;
		else
			return MAPI_ERR_INVALID;
	} else {
		return MAPI_ERR_INVALID;
	}

	CVI_VENC_SetChnAttr(vencCtx->vencChn, &stChnAttr);

	return MAPI_SUCCESS;
}



#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <pthread.h>
#include "mapi.h"
#include "md_ser.h"
#include "cvi_sys.h"
#include "cvi_ive.h"
#include "osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef int32_t (*MD_CALLBACK)(int32_t, int32_t);

typedef enum {
	MD_STOP = 0,
	MD_RUN,
	MD_PAUSE,
	MD_BUTT
} MD_STATE;

typedef struct _MD_VPROC_ATTR_S {
	MAPI_VPROC_HANDLE_T vprocHandle;
	uint32_t vprocChnId;
	uint32_t isExtVproc;
	uint32_t w;
	uint32_t h;
} MD_VPROC_ATTR_S;

typedef struct _MD_IVE_CTX_S {
	IVE_HANDLE hdl;
	IVE_IMAGE_S srcImage0;
	IVE_IMAGE_S srcImage1;
	IVE_IMAGE_S dstImage;
} MD_IVE_CTX_S;

typedef struct _MD_CTX_S {
	int32_t run;
	int32_t id;
	int32_t threshold;
	MD_VPROC_ATTR_S vprocAttr;
	MD_IVE_CTX_S iveHdl;
	OSAL_TASK_HANDLE_S task;
	OSAL_MUTEX_HANDLE_S mutex;
	MD_CALLBACK pfnCb;
} MD_CTX_S;

static MD_CTX_S *gstMotionDetCtx[2];
static OSAL_MUTEX_ATTR_S md_mutex = OSAL_MUTEX_INITIALIZER;

static void md_Frame2Image(VIDEO_FRAME_INFO_S *frame, IVE_IMAGE_S *image)
{
	CVI_IVE_VideoFrameInfo2Image(frame, image);
}

static int32_t MD_Proc(IVE_IMAGE_S *image, int32_t threshold)
{
	uint32_t len = (int32_t)(image->u32Stride[0] * image->u32Height);
	uint32_t diff = 0;

	uint32_t *q = (uint32_t *)((uintptr_t)(image->u64VirAddr[0]));
	uint32_t n = 0;
	for (uint32_t i = 0; i < len / sizeof(uint32_t); i++) {
		n = (uint32_t)(q[i]);
		switch (n) {
		case 0x1010101:
			diff += 4;
			break;
		case 0x10101:
			diff += 3;
			break;
		case 0x101:
			diff += 2;
			break;
		case 0x1:
			diff += 1;
			break;
		default:
			break;
		}
	}

	uint32_t percent = diff * 100 / len;
	if (percent >= (uint32_t)threshold) {
		return 1;
	} else {
		return 0;
	}

	return 0;
}

void MD_SetState(int32_t id, int32_t en)
{
	CVI_LOGD("[%d] state %d", id, en);
	if (gstMotionDetCtx[id] == NULL) {
		return;
	}
	if (en == 1) {
		gstMotionDetCtx[id]->run = MD_RUN;
	} else if (en == 0) {
		gstMotionDetCtx[id]->run = MD_PAUSE;
	}
}

static void md_Image2Data(IVE_IMAGE_S *image, IVE_DATA_S *data)
{
	data->u32Height = image->u32Height;
	data->u32Width = image->u32Width;
	data->u32Stride = image->u32Stride[0];
	data->u64PhyAddr = image->u64PhyAddr[0];
	data->u64VirAddr = image->u64VirAddr[0];
}

static void md_Task(void *arg)
{
	MD_CTX_S *ctx = (MD_CTX_S *)arg;

	int32_t s32Ret = 0;
	// interval for updating background
	int32_t update_interval = 15; // Update Map Threshold

	VIDEO_FRAME_INFO_S frame;
	int32_t frameCnt = 0;
	MD_IVE_CTX_S *iveCtx = &ctx->iveHdl;

	while (ctx->run != MD_STOP) {
		if (ctx->run == MD_PAUSE) {
			OSAL_TASK_Sleep(200 * 1000);
			continue;
		}

		s32Ret = MAPI_VPROC_GetChnFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);
		if (s32Ret != 0) {
			CVI_LOGE("MAPI_VPROC_GetChnFrame chn0 failed with %#x\n", s32Ret);
			OSAL_TASK_Sleep(100 * 1000);
			continue;
		}

		if (frameCnt % update_interval == 0) {
			IVE_IMAGE_S image;
			IVE_DATA_S src, dst;
			md_Frame2Image(&frame, &image);
			md_Image2Data(&image, &src);
			md_Image2Data(&iveCtx->srcImage0, &dst);

			OSAL_MUTEX_Lock(&md_mutex);
			IVE_DMA_CTRL_S dmactrl;
			memset(&dmactrl, 0x0, sizeof(IVE_DMA_CTRL_S));
			CVI_IVE_DMA(iveCtx->hdl, &src, &dst, &dmactrl, false);
			OSAL_MUTEX_Unlock(&md_mutex);
			MAPI_VPROC_ReleaseFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);
			frameCnt = 1;
			OSAL_TASK_Sleep(100 * 1000);
			continue;
		}

		md_Frame2Image(&frame, &iveCtx->srcImage1);

		IVE_FRAME_DIFF_MOTION_CTRL_S ctrl = {
			.enSubMode = IVE_SUB_MODE_ABS,
			.enThrMode = IVE_THRESH_MODE_BINARY,
			.u8ThrLow = 30,
			.u8ThrHigh = 0,
			.u8ThrMinVal = 0,
			.u8ThrMidVal = 0,
			.u8ThrMaxVal = 255,
			.au8ErodeMask = {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
			.au8DilateMask = {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0},
		};

		OSAL_MUTEX_Lock(&md_mutex);
		memset((char *)((uintptr_t)(iveCtx->dstImage.u64VirAddr[0])), 0x0, iveCtx->dstImage.u32Width * iveCtx->dstImage.u32Height);
		s32Ret = CVI_IVE_FrameDiffMotion(iveCtx->hdl, &iveCtx->srcImage1, &iveCtx->srcImage0, &iveCtx->dstImage, &ctrl, false);
		if (s32Ret != 0) {
			MAPI_VPROC_ReleaseFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);
			OSAL_MUTEX_Unlock(&md_mutex);
			OSAL_TASK_Sleep(150 * 1000);
			continue;
		}

		unsigned char finished = 0;
		while (1) {
			CVI_IVE_QUERY(iveCtx->hdl, &finished, 0);
			if (finished == true) {
				break;
			}
			OSAL_TASK_Sleep(20 * 1000);
		}
		OSAL_MUTEX_Unlock(&md_mutex);
		MAPI_VPROC_ReleaseFrame(ctx->vprocAttr.vprocHandle, ctx->vprocAttr.vprocChnId, &frame);

		int32_t isTrig = MD_Proc(&iveCtx->dstImage, ctx->threshold);
		if (isTrig == 1) {
			ctx->pfnCb(ctx->id, MD_EVENT_CHANGE);
		}

		frameCnt++;
		OSAL_TASK_Sleep(150 * 1000);
	}
}

void MD_DeInit(int32_t id)
{
	MD_CTX_S *ctx = gstMotionDetCtx[id];
	if (ctx == NULL) {
		return;
	}

	ctx->run = MD_STOP;
	OSAL_TASK_Join(ctx->task);
	OSAL_TASK_Destroy(&ctx->task);

	if (ctx->iveHdl.hdl) {
		CVI_SYS_FreeI(ctx->iveHdl.hdl, &ctx->iveHdl.dstImage);
		CVI_SYS_FreeI(ctx->iveHdl.hdl, &ctx->iveHdl.srcImage0);
		CVI_IVE_DestroyHandle(ctx->iveHdl.hdl);
		ctx->iveHdl.hdl = NULL;
	}

	OSAL_MUTEX_Destroy(ctx->mutex);
	free(ctx);
	gstMotionDetCtx[id] = NULL;
	CVI_LOGD("gstMotionDetCtx %d init", id);
}

int32_t MD_Init(MD_ATTR_S *attr)
{
	if (attr == NULL) {
		return -1;
	}

	if (gstMotionDetCtx[attr->camid]) {
		return 0;
	}

	gstMotionDetCtx[attr->camid] = (MD_CTX_S *)malloc(sizeof(MD_CTX_S));
	if (gstMotionDetCtx[attr->camid] == NULL) {
		return -1;
	}
	memset(gstMotionDetCtx[attr->camid], 0x0, sizeof(MD_CTX_S));

	gstMotionDetCtx[attr->camid]->id = attr->camid;
	gstMotionDetCtx[attr->camid]->threshold = attr->threshold;
	gstMotionDetCtx[attr->camid]->vprocAttr.vprocHandle = attr->vprocHandle;
	gstMotionDetCtx[attr->camid]->vprocAttr.vprocChnId = attr->vprocChnId;
	gstMotionDetCtx[attr->camid]->vprocAttr.isExtVproc = attr->isExtVproc;
	gstMotionDetCtx[attr->camid]->vprocAttr.w = attr->w;
	gstMotionDetCtx[attr->camid]->vprocAttr.h = attr->h;
	gstMotionDetCtx[attr->camid]->pfnCb = (MD_CALLBACK)attr->pfnCb;

	int32_t s32Ret = 0;
	IVE_HANDLE iveHdl = CVI_IVE_CreateHandle();
	if (iveHdl == NULL) {
		free(gstMotionDetCtx[attr->camid]);
		gstMotionDetCtx[attr->camid] = NULL;
		CVI_LOGD("Motion Detect %d create failed", attr->camid);
		return -1;
	}

	s32Ret = CVI_IVE_CreateImage(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.dstImage, IVE_IMAGE_TYPE_U8C1, attr->w, attr->h);
	if (s32Ret != 0) {
		CVI_IVE_DestroyHandle(iveHdl);
		free(gstMotionDetCtx[attr->camid]);
		gstMotionDetCtx[attr->camid] = NULL;
		CVI_LOGD("Motion Detect %d CVI_IVE_CreateImage failed", attr->camid);
		return -1;
	}

	s32Ret = CVI_IVE_CreateImage(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.srcImage0, IVE_IMAGE_TYPE_U8C1, attr->w, attr->h);
	if (s32Ret != 0) {
		CVI_SYS_FreeI(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.dstImage);
		CVI_IVE_DestroyHandle(iveHdl);
		free(gstMotionDetCtx[attr->camid]);
		gstMotionDetCtx[attr->camid] = NULL;
		CVI_LOGD("Motion Detect %d CVI_IVE_CreateImage failed", attr->camid);
		return -1;
	}

	gstMotionDetCtx[attr->camid]->iveHdl.hdl = iveHdl;

	OSAL_MUTEX_Create(NULL, &gstMotionDetCtx[attr->camid]->mutex);

	if (attr->state == 1) {
		gstMotionDetCtx[attr->camid]->run = MD_RUN;
	} else if (attr->state == 0) {
		gstMotionDetCtx[attr->camid]->run = MD_PAUSE;
	}

	OSAL_MUTEX_ATTR_S md;
	static char md_name[2][16] = {0};
	snprintf(md_name[attr->camid], sizeof(md_name[attr->camid]), "motionDet_%d", attr->camid);
	md.name = md_name[attr->camid];
	md.entry = md_Task;
	md.param = (void *)gstMotionDetCtx[attr->camid];
	md.priority = OSAL_TASK_PRI_NORMAL;
	md.detached = false;
	md.stack_size = 256 * 1024;
	s32Ret = OSAL_TASK_Create(&md, &gstMotionDetCtx[attr->camid]->task);
	if (s32Ret != OSAL_SUCCESS) {
		CVI_SYS_FreeI(iveHdl, &gstMotionDetCtx[attr->camid]->iveHdl.dstImage);
		CVI_IVE_DestroyHandle(iveHdl);
		free(gstMotionDetCtx[attr->camid]);
		gstMotionDetCtx[attr->camid] = NULL;
		CVI_LOGE("CVI_VIDEOMD_Run task create failed, %d", attr->camid);
		return -1;
	}
	CVI_LOGD("gstMotionDetCtx %d init", attr->camid);
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
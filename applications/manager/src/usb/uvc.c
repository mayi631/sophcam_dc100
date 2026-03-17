#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/prctl.h>

#include "cvi_log.h"
#include "osal.h"
#include "mapi.h"
#include "usb.h"
#include "appuvc.h"
#include "uvc_gadget.h"

/** UVC Stream Context */
typedef struct tagUVC_STREAM_CONTEXT_S {
	UVC_DEVICE_CAP_S stDeviceCap;
	UVC_DATA_SOURCE_S stDataSource;
	UVC_STREAM_ATTR_S stStreamAttr; /**<stream attribute, update by uvc driver */
	bool bVcapsStart;
	bool bVencStart;
	bool bFirstFrame;
	bool bInited;
} UVC_STREAM_CONTEXT_S;
static UVC_STREAM_CONTEXT_S s_stUVCStreamCtx;

int32_t UVC_STREAM_SetAttr(UVC_STREAM_ATTR_S *pstAttr)
{
	s_stUVCStreamCtx.stStreamAttr = *pstAttr;
	CVI_LOGI("Format: %d, Resolution: %ux%u, FPS: %u, BitRate: %u", pstAttr->enFormat, pstAttr->u32Width,
			 pstAttr->u32Height, pstAttr->u32Fps, pstAttr->u32BitRate);

	s_stUVCStreamCtx.bInited = false;
	s_stUVCStreamCtx.bVcapsStart = false;
	s_stUVCStreamCtx.bVencStart = false;
	s_stUVCStreamCtx.bFirstFrame = false;

	return 0;
}

static int32_t init_vproc()
{
	UVC_STREAM_ATTR_S *pAttr = &s_stUVCStreamCtx.stStreamAttr;
	UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
	VPSS_CHN_ATTR_S stChnAttr;
	MAPI_VPROC_GetChnAttr(pstSrc->VprocHdl, pstSrc->VprocChnId, &stChnAttr);

	stChnAttr.u32Width = pAttr->u32Width;
	stChnAttr.u32Height = pAttr->u32Height;
	MAPI_VPROC_SetChnAttr(pstSrc->VprocHdl, pstSrc->VprocChnId, &stChnAttr);

	return 0;
}

static int32_t init_venc()
{
	UVC_STREAM_ATTR_S *pAttr = &s_stUVCStreamCtx.stStreamAttr;
	UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
	MAPI_VENC_CHN_ATTR_T venc_attr = {0};

	venc_attr.venc_param.width = pAttr->u32Width;
	venc_attr.venc_param.height = pAttr->u32Height;
	venc_attr.venc_param.pixel_format = PIXEL_FORMAT_YUV_PLANAR_420;
	if (pAttr->enFormat == UVC_STREAM_FORMAT_MJPEG) {
		venc_attr.venc_param.codec = MAPI_VCODEC_MJP;
		venc_attr.venc_param.jpeg_quality = 80;
		venc_attr.venc_param.initialDelay = 1000;
		venc_attr.venc_param.single_EsBuf = 1;
		venc_attr.venc_param.bufSize = 729088;
	} else if (pAttr->enFormat == UVC_STREAM_FORMAT_H264) {
		venc_attr.venc_param.codec = MAPI_VCODEC_H264;
		venc_attr.venc_param.rate_ctrl_mode = 2;
		venc_attr.venc_param.src_framerate = 25;
		venc_attr.venc_param.dst_framerate = 25;
		venc_attr.venc_param.gop = 25;
		venc_attr.venc_param.bitrate_kbps = 12000;
		venc_attr.venc_param.iqp = 36;
		venc_attr.venc_param.pqp = 36;
		venc_attr.venc_param.minIqp = 24;
		venc_attr.venc_param.maxIqp = 42;
		venc_attr.venc_param.minQp = 24;
		venc_attr.venc_param.maxQp = 46;
		venc_attr.venc_param.videoSignalTypePresentFlag = 0;
		venc_attr.venc_param.videoFormat = 0;
		venc_attr.venc_param.videoFullRangeFlag = 0;
		venc_attr.venc_param.bufSize = 1048576;
		venc_attr.venc_param.initialDelay = 1000;
		venc_attr.venc_param.thrdLv = 2;
		venc_attr.venc_param.statTime = 2;
		venc_attr.venc_param.changePos = 90;
		venc_attr.venc_param.single_EsBuf = 1;
		venc_attr.venc_param.firstFrameStartQp = 36;
		venc_attr.venc_param.maxBitRate = 16000;
		venc_attr.venc_param.gop_mode = 0;
		venc_attr.venc_param.maxIprop = 100;
		venc_attr.venc_param.minIprop = 1;
		venc_attr.venc_param.minStillPercent = 60;
		venc_attr.venc_param.maxStillQP = 38;
		venc_attr.venc_param.avbrPureStillThr = 50;
		venc_attr.venc_param.motionSensitivity = 24;
		venc_attr.venc_param.bgDeltaQp = 0;
		venc_attr.venc_param.rowQpDelta = 0;
		venc_attr.venc_param.ipqpDelta = 3;
	}

	venc_attr.cb.stream_cb_func = NULL;
	venc_attr.cb.stream_cb_data = NULL;

	APPCOMM_CHECK_RETURN(MAPI_VENC_InitChn(&pstSrc->VencHdl, &venc_attr), USB_EINVAL);

	return MAPI_VENC_StartRecvFrame(pstSrc->VencHdl, -1);
}

int32_t UVC_STREAM_Start(void)
{
	if (!s_stUVCStreamCtx.bInited) {
		if (0 != init_vproc()) {
			CVI_LOGE("init_vproc failed !");
			return -1;
		}

		if (0 != init_venc()) {
			CVI_LOGE("init_venc failed !");
			return -1;
		}
		s_stUVCStreamCtx.bInited = true;
	}

	return 0;
}

int32_t UVC_STREAM_Stop(void)
{
	if (s_stUVCStreamCtx.bInited) {
		UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
		APPCOMM_CHECK_RETURN(MAPI_VENC_StopRecvFrame(pstSrc->VencHdl), USB_EINVAL);
		APPCOMM_CHECK_RETURN(MAPI_VENC_DeinitChn(pstSrc->VencHdl), USB_EINVAL);

		s_stUVCStreamCtx.bInited = false;
	}
	return 0;
}

int32_t UVC_STREAM_CopyBitStream(void *dst)
{
	UVC_DATA_SOURCE_S *pstSrc = &s_stUVCStreamCtx.stDataSource;
	VIDEO_FRAME_INFO_S venc_frame;
	VENC_STREAM_S stream = {0};
	APPCOMM_CHECK_RETURN(MAPI_VPROC_GetChnFrame(pstSrc->VprocHdl, pstSrc->VprocChnId, &venc_frame), USB_EINVAL);
	if (MAPI_VENC_SendFrame(pstSrc->VencHdl, &venc_frame) != 0) {
		CVI_LOGE("UVC: MAPI_VENC_SendFrame failed");
		MAPI_ReleaseFrame(&venc_frame);
		return 0;
	}

	if (MAPI_VENC_GetStream(pstSrc->VencHdl, &stream) != 0) {
		CVI_LOGE("UVC: MAPI_VENC_GetStream failed");
		MAPI_ReleaseFrame(&venc_frame);
		return 0;
	}

	bool is_i_frame;

	uint32_t bitstream_size = 0;
	for (unsigned i = 0; i < stream.u32PackCount; i++) {
		VENC_PACK_S *ppack;
		ppack = &stream.pstPack[i];
		MAPI_VENC_GetStreamStatus(pstSrc->VencHdl, ppack, &is_i_frame);

		memcpy(dst + bitstream_size, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
		bitstream_size += ppack->u32Len - ppack->u32Offset;
	}

	MAPI_VENC_ReleaseStream(pstSrc->VencHdl, &stream);
	MAPI_ReleaseFrame(&venc_frame);

	return bitstream_size;
}

int32_t UVC_STREAM_ReqIDR(void)
{
	// TODO: implement
	return 0;
}

/** UVC Context */
static UVC_CONTEXT_S s_stUVCCtx = {.bRun = false, .bPCConnect = false, .TskId = 0, .Tsk2Id = 0};

bool g_bPushVencData = false;

static void *UVC_CheckTask(void *pvArg)
{
	int32_t ret = 0;
	prctl(PR_SET_NAME, "UVC_Check", 0, 0, 0);
	while (s_stUVCCtx.bRun) {
		ret = UVC_GADGET_DeviceCheck();

		if (ret < 0) {
			CVI_LOGD("UVC_GADGET_DeviceCheck %x\n", ret);
			break;
		} else if (ret == 0) {
			CVI_LOGD("Timeout Do Nothing\n");
			if (false != g_bPushVencData) {
				g_bPushVencData = false;
			}
		}
		usleep(10 * 1000);
	}

	return NULL;
}

static int32_t UVC_LoadMod(void)
{
	static bool first = true;
	if (first == false) {
		return 0;
	}
	first = false;
	CVI_LOGD("Uvc insmod ko successfully!");
	/*
		OSAL_FS_Insmod(KOMOD_PATH"/usb-common.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/udc-core.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/libcomposite.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/usbcore.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/roles.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/dwc2.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/libcomposite.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/videobuf2-common.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/videobuf2-memops.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/videobuf2-v4l2.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/videobuf2-vmalloc.ko", NULL);
		OSAL_FS_Insmod(KOMOD_PATH"/usb_f_uvc.ko", NULL);
		OSAL_FS_System("echo device > /proc/cviusb/otg_role");
	*/
	OSAL_FS_System(UVC_SCRIPTS_PATH "/run_usb.sh probe uvc");
	OSAL_FS_System(UVC_SCRIPTS_PATH "/ConfigUVC.sh");
	OSAL_FS_System(UVC_SCRIPTS_PATH "/run_usb.sh start");
	// OSAL_FS_System("devmem 0x030001DC 32 0x8844");
	return 0;
}

static int32_t UVC_UnLoadMod(void)
{
	CVI_LOGD("Do nothing now, due to the ko can NOT rmmod successfully!");
	// OSAL_FS_Rmmod(KOMOD_PATH "/videobuf2-memops.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/videobuf2-vmalloc.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/configfs.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/libcomposite.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/u_serial.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_acm.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/cvi_usb_f_cvg.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_uvc.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/u_audio.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_uac1.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_serial.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_mass_storage.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/u_ether.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_ecm.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_eem.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/usb_f_rndis.ko");
	// OSAL_FS_Rmmod(KOMOD_PATH "/cv183x_usb_gadget.ko");

	return 0;
}

int32_t UVC_Init(const UVC_DEVICE_CAP_S *pstCap, const UVC_DATA_SOURCE_S *pstDataSrc,
				 UVC_BUFFER_CFG_S *pstBufferCfg)
{
	APPCOMM_CHECK_POINTER(pstCap, -1);
	APPCOMM_CHECK_POINTER(pstDataSrc, -1);

	UVC_LoadMod();

	s_stUVCStreamCtx.stDeviceCap = *pstCap;
	s_stUVCStreamCtx.stDataSource = *pstDataSrc;
	UVC_GADGET_Init(pstCap, pstBufferCfg->u32BufSize);

	// TODO: Do we need handle UVC_BUFFER_CFG_S?
	return 0;
}

int32_t UVC_Deinit(void)
{
	UVC_UnLoadMod(); // TODO, Not work right now

	return 0;
}

int32_t UVC_Start(const char *pDevPath)
{
	APPCOMM_CHECK_POINTER(pDevPath, -1);

	if (false == s_stUVCCtx.bRun) {
		strcpy(s_stUVCCtx.szDevPath, pDevPath);

		if (UVC_GADGET_DeviceOpen(pDevPath)) {
			CVI_LOGD("UVC_GADGET_DeviceOpen Failed!");
			return -1;
		}

		s_stUVCCtx.bPCConnect = false;
		s_stUVCCtx.bRun = true;
		pthread_attr_t pthread_attr;
		pthread_attr_init(&pthread_attr);
		pthread_attr_setschedpolicy(&pthread_attr, SCHED_OTHER);
		if (pthread_create(&s_stUVCCtx.TskId, &pthread_attr, UVC_CheckTask, NULL)) {
			CVI_LOGE("UVC_CheckTask create thread failed!\n");
			pthread_attr_destroy(&pthread_attr);
			s_stUVCCtx.bRun = false;
			return -1;
		}
		pthread_attr_destroy(&pthread_attr);
		CVI_LOGD("UVC_CheckTask create thread successful\n");
	} else {
		CVI_LOGD("UVC already started\n");
	}

	return 0;
}

int32_t UVC_Stop(void)
{
	if (false == s_stUVCCtx.bRun) {
		CVI_LOGE("UVC not run\n");
		return 0;
	}

	s_stUVCCtx.bRun = false;
	pthread_join(s_stUVCCtx.TskId, NULL);

	return UVC_GADGET_DeviceClose();
}

UVC_CONTEXT_S *UVC_GetCtx(void) { return &s_stUVCCtx; }

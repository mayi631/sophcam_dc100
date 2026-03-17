#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "cvi_buffer.h"
#include "cvi_common.h"
#include "cvi_log.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"
#include "cvi_comm_vo.h"
#include "cvi_comm_aio.h"
#include "cvi_audio.h"
#include "cvi_sys.h"
#include "cvi_vo.h"
#include "cvi_vpss.h"
#include "cvi_vdec.h"
#include "cvi_vb.h"
#include "cvi_msg_client.h"
#include "cvi_comm_vdec.h"
#include "mapi_sys.h"
#include "player.h"
#include "mapi.h"
#include "hal_screen_comp.h"
#include "hal_gpio.h"

#define CVI_ALIGN_UP(x, a) ((x + a - 1) & (~(a - 1)))
#define ALIGN_DOWN(x, a) ((x) & ~((a)-1))
#define CROP_OUT

typedef struct _SAMPLE_VDEC_VIDEO_ATTR {
	VIDEO_DEC_MODE_E enDecMode;
	CVI_U32              u32RefFrameNum;
	DATA_BITWIDTH_E  enBitWidth;
} SAMPLE_VDEC_VIDEO_ATTR;

typedef struct _SAMPLE_VDEC_PICTURE_ATTR {
	CVI_U32         u32Alpha;
} SAMPLE_VDEC_PICTURE_ATTR;

typedef struct _SAMPLE_VDEC_ATTR {
	PAYLOAD_TYPE_E enType;
	PIXEL_FORMAT_E enPixelFormat;
	VIDEO_MODE_E   enMode;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32FrameBufCnt;
	CVI_U32 u32DisplayFrameNum;
	union {
		SAMPLE_VDEC_VIDEO_ATTR stSampleVdecVideo;      /* structure with video ( h265/h264) */
		SAMPLE_VDEC_PICTURE_ATTR stSampleVdecPicture; /* structure with picture (jpeg/mjpeg )*/
	};
} SAMPLE_VDEC_ATTR;

typedef enum _THREAD_CONTRL_E {
	THREAD_CTRL_START,
	THREAD_CTRL_PAUSE,
	THREAD_CTRL_STOP,
} THREAD_CONTRL_E;

typedef struct MD5state_st {
	uint32_t A, B, C, D;
	uint32_t Nl, Nh;
	uint32_t data[16];
	uint32_t num;
} MD5_CTX;
typedef struct _VDEC_THREAD_PARAM_S {
	CVI_S32 s32ChnId;
	PAYLOAD_TYPE_E enType;
	CVI_CHAR cFilePath[128];
	CVI_CHAR cFileName[128];
	CVI_S32 s32StreamMode;
	CVI_S32 s32MilliSec;
	CVI_S32 s32MinBufSize;
	CVI_S32 s32IntervalTime;
	THREAD_CONTRL_E eThreadCtrl;
	CVI_U64 u64PtsInit;
	CVI_U64 u64PtsIncrease;
	CVI_BOOL bCircleSend;
	CVI_BOOL bFileEnd;
	CVI_BOOL bDumpYUV;
	MD5_CTX tMD5Ctx;
} VDEC_THREAD_PARAM_S;
typedef struct _vdecChnCtx_ {
	VDEC_THREAD_PARAM_S stVdecThreadParamSend;
	VDEC_THREAD_PARAM_S stVdecThreadParamGet;
	SAMPLE_VDEC_ATTR stSampleVdecAttr;
	pthread_t vdecThreadSend;
	pthread_t vdecThreadGet;
	VDEC_CHN VdecChn;
	CVI_S32 bCreateChn;
} vdecChnCtx;

typedef struct {
    int32_t chn_id;
    bool repeat;
    MAPI_DISP_HANDLE_T disp;
    int32_t disp_id;
    PIXEL_FORMAT_E disp_fmt;
    ROTATION_E disp_rotate;
    ASPECT_RATIO_E disp_aspect_ratio;
    uint32_t x;
    uint32_t y;
    uint32_t width; // 0 for screen width
    uint32_t height; // 0 for screen height
} PLAYER_SAMPLE_PARAM_S;

static const int FRAME_PLANAR_NUM = 2;
static const int VDEC_CHANNEL = 0;
static const int MAX_VDEC_CHANNEL = 1;
static const int VO_PANEL_WIDTH = 1920; //屏幕的宽
static const int VO_PANEL_HEIGHT = 440; //屏幕的高

static int initVBPool(PAYLOAD_TYPE_E encode_type, int input_width, int input_height);
static int initVO();
static int initVPSS(int input_width, int input_height);
static int initVdec(PAYLOAD_TYPE_E encode_type, int width, int height);
static int initAO();
static CVI_S32 vdecInitAttr(SAMPLE_VDEC_ATTR *psvdattr, PAYLOAD_TYPE_E encode_type, int width, int height);
static void usage();
static VB_SOURCE_E g_VdecVbSrc = VB_SOURCE_COMMON;
bool find_first_sps = false;
static bool VideoCustom = true;
static bool send_vo_again = false;
static sem_t semDownload;
static MAPI_DISP_HANDLE_T psdisp;

#define CHECK_CHN_RET(express, Chn, name)                                                                              \
	do {                                                                                                           \
		CVI_S32 Ret;                                                                                           \
		Ret = express;                                                                                         \
		if (Ret != CVI_SUCCESS) {                                                                              \
			CVI_LOGI("\033[0;31m%s chn %d failed at %s: LINE: %d with %#x!\033[0;39m\n", name, Chn,          \
			       __func__, __LINE__, Ret);                                                               \
			fflush(stdout);                                                                                \
			return Ret;                                                                                    \
		}                                                                                                      \
	} while (0)
typedef struct {
    PLAYER_HANDLE_T player_handle;
    PAYLOAD_TYPE_E encode_type;
} CustomArg;

static inline int MAX(int a, int b) {
    return a > b ? a : b;
}

static PAYLOAD_TYPE_E getPayloadTypeFromName(const char *codec_name)
{
    return (strcmp(codec_name, "hevc") == 0) ? PT_H265 : PT_H264;
}

static int dump_yuv(const char *filename, VIDEO_FRAME_S stVFrame)
{
    char *w_ptr;
    unsigned int i = 0;
/*
    CVI_LOGI("write_yuv u32Width = %d, u32Height = %d\n",
            stVFrame.u32Width, stVFrame.u32Height);
    CVI_LOGI("write_yuv u32Stride[0] = %d, u32Stride[1] = %d, u32Stride[2] = %d\n",
            stVFrame.u32Stride[0], stVFrame.u32Stride[1], stVFrame.u32Stride[2]);
    CVI_LOGI("write_yuv u32Length[0] = %d, u32Length[1] = %d, u32Length[2] = %d\n",
            stVFrame.u32Length[0], stVFrame.u32Length[1], stVFrame.u32Length[2]);
*/
    if(!filename) {
        CVI_LOGE("the filename string is null\n");
        return -1;
    }
    FILE *fp = fopen(filename, "wb");
    if(!fp) {
        CVI_LOGE("open file fail\n");
        return -1;
    }

    //CVI_LOGE("stride:%d, %d, %d\n", stVFrame.u32Stride[0], stVFrame.u32Stride[1], stVFrame.u32Stride[2]);
    w_ptr = (char *)stVFrame.pu8VirAddr[0];
    for (i = 0; i < stVFrame.u32Height; i++) {
        fwrite(w_ptr, stVFrame.u32Width, 1, fp);
        w_ptr += stVFrame.u32Stride[0];
        //dumpdata(filename,w_ptr + i * stVFrame.u32Stride[0],stVFrame.u32Width);
    }
    w_ptr = (char *)stVFrame.pu8VirAddr[1];
    for (i = 0; i < stVFrame.u32Height / 2; i++) {
        fwrite(w_ptr, stVFrame.u32Width / 2, 1, fp);
        w_ptr += stVFrame.u32Stride[1];
        //dumpdata(filename,w_ptr + i * stVFrame.u32Stride[1],stVFrame.u32Width / 2);
    }
    w_ptr = (char *)stVFrame.pu8VirAddr[2];
    for (i = 0; i < stVFrame.u32Height / 2; i++) {
        fwrite(w_ptr, stVFrame.u32Width / 2, 1, fp);
        w_ptr += stVFrame.u32Stride[2];
        //dumpdata(filename,w_ptr + i * stVFrame.u32Stride[2],stVFrame.u32Width / 2);
    }
    fclose(fp);
    system("sync");

    return 0;
}

static bool needInitVideoSystem(int width, int height)
{
    return (width != 0) && (height != 0);
}

static int initVideoSystem(int width, int height, PAYLOAD_TYPE_E encode_type)
{
    if (0 != initVBPool(encode_type, width, height)) {
        CVI_LOGE("vb pool init failed.\n");
        return CVI_FAILURE;
    }

    if (0 != initVdec(encode_type, width, height)) {
        CVI_LOGE("video decoder init failed.\n");
        return CVI_FAILURE;
    }

    if (0 != initVPSS(width, height)) {
        CVI_LOGE("vpss init failed.\n");
        return CVI_FAILURE;
    }

    if (0 != initVO()) {
        CVI_LOGE("vo init failed.\n");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static int initSystem(int input_width, int input_height, PAYLOAD_TYPE_E encode_type)
{
    CVI_MSG_Init();
    CVI_S32 s32Ret = CVI_SUCCESS;
    if (needInitVideoSystem(input_width, input_height)) {
        s32Ret = initVideoSystem(input_width, input_height, encode_type);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("video system init failed.\n");
            return s32Ret;
        }
    }

    s32Ret = initAO();
    if (0 != s32Ret) {
        CVI_LOGE("ao init failed.\n");
        return s32Ret;
    }
    return CVI_SUCCESS;
}

int Play_Sample_Sys_Init(VB_CONFIG_S *pstVbConfig)
{
    CVI_S32 s32Ret = CVI_FAILURE;

    if (pstVbConfig == NULL) {
        CVI_LOGE("input parameter is null, it is invaild!\n");
        return CVI_FAILURE;
    }

    s32Ret = CVI_SYS_Init();
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_SYS_Init failed!\n");
        return s32Ret;
    }

    s32Ret = CVI_VB_SetConfig(pstVbConfig);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VB_SetConf failed!\n");
        return s32Ret;
    }

    s32Ret = CVI_VB_Init();
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VB_Init failed!\n");
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static int initVBPool(PAYLOAD_TYPE_E encode_type, int input_width, int input_height)
{
    CVI_U32 width, height;
    VB_CONFIG_S vb_conf;

    memset(&vb_conf, 0, sizeof(VB_CONFIG_S));
    vb_conf.u32MaxPoolCnt = 1;
    width = MAX(input_width, VO_PANEL_WIDTH);
    height = MAX(input_height, VO_PANEL_HEIGHT);
    CVI_U32 u32BlkSize = VDEC_GetPicBufferSize(encode_type, width,
        height, PIXEL_FORMAT_YUV_PLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
    vb_conf.astCommPool[0].u32BlkSize = u32BlkSize;
    vb_conf.astCommPool[0].u32BlkCnt = 5;
    CVI_LOGE("VDec Init Pool[%d], u32BlkSize = %d, u32BlkCnt = %d\n",
        0, vb_conf.astCommPool[0].u32BlkSize,
        vb_conf.astCommPool[0].u32BlkCnt);

    CVI_S32 s32Ret = Play_Sample_Sys_Init(&vb_conf);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("SAMPLE_COMM_SYS_Init, %d\n", s32Ret);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 Play_Sample_Vpss_Init(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	VPSS_CHN VpssChn;
	CVI_S32 s32Ret;
	CVI_S32 j;

	s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
	if (s32Ret != CVI_SUCCESS) {
		CVI_LOGE("CVI_VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_LOGE("CVI_VPSS_ResetGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
		return CVI_FAILURE;
	}

	for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
		if (pabChnEnable[j]) {
			VpssChn = j;
			s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);

			if (s32Ret != CVI_SUCCESS) {
				CVI_LOGE("CVI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}

			s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);

			if (s32Ret != CVI_SUCCESS) {
				CVI_LOGE("CVI_VPSS_EnableChn failed with %#x\n", s32Ret);
				return CVI_FAILURE;
			}
		}
	}

	return CVI_SUCCESS;
}

CVI_S32 Play_Sample_Vpss_Start(VPSS_GRP VpssGrp, CVI_BOOL *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
			      VPSS_CHN_ATTR_S *pastVpssChnAttr)
{
	CVI_S32 s32Ret;
	UNUSED(pabChnEnable);
	UNUSED(pstVpssGrpAttr);
	UNUSED(pastVpssChnAttr);

	s32Ret = CVI_VPSS_StartGrp(VpssGrp);
	if (s32Ret != CVI_SUCCESS) {
		CVI_LOGE("CVI_VPSS_StartGrp failed with %#x\n", s32Ret);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

static int initVPSS(int input_width, int input_height)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_GRP VpssGrp = 0;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN VpssChn = VPSS_CHN0;
    CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];

    stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
    stVpssGrpAttr.enPixelFormat                  = PIXEL_FORMAT_YUV_PLANAR_420;
    stVpssGrpAttr.u32MaxW                        = input_width;
    stVpssGrpAttr.u32MaxH                        = input_height;
    stVpssGrpAttr.u8VpssDev                      = 0;

    astVpssChnAttr[VpssChn].u32Width                    = VO_PANEL_WIDTH;
    astVpssChnAttr[VpssChn].u32Height                   = VO_PANEL_HEIGHT;
    astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_NV21;
    astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
    astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
    astVpssChnAttr[VpssChn].u32Depth                    = 1;
    astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
    astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
    astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    astVpssChnAttr[VpssChn].stAspectRatio.u32BgColor    = COLOR_RGB_BLACK;
    astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

    abChnEnable[VpssChn] = 1;
    s32Ret = Play_Sample_Vpss_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = Play_Sample_Vpss_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static int VpssAttrReset(VIDEO_FRAME_INFO_S *video_frame)
{
    int s32Ret = CVI_SUCCESS;
    bool is_changed = false;
    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
    if ((s32Ret=CVI_VPSS_GetGrpAttr(0, &stVpssGrpAttr)) != CVI_SUCCESS) {
        CVI_LOGE("MAPI_VPROC_GetGrpAttr failed, s32Ret:%x", s32Ret);
        return s32Ret;
    }
    //CVI_LOGE("stVpssGrpAttr.enPixelFormat:%d, video_frame.stVFrame.enPixelFormat:%d", stVpssGrpAttr.enPixelFormat, video_frame->stVFrame.enPixelFormat);
    if (stVpssGrpAttr.enPixelFormat != video_frame->stVFrame.enPixelFormat) {
        stVpssGrpAttr.enPixelFormat = video_frame->stVFrame.enPixelFormat;
        is_changed = true;
    }
    if (stVpssGrpAttr.u32MaxW != video_frame->stVFrame.u32Width) {
        stVpssGrpAttr.u32MaxW = video_frame->stVFrame.u32Width;
        is_changed = true;
    }
    if (stVpssGrpAttr.u32MaxH != video_frame->stVFrame.u32Height) {
        stVpssGrpAttr.u32MaxH = video_frame->stVFrame.u32Height;
        is_changed = true;
    }
    if (is_changed) {
        if ((s32Ret=CVI_VPSS_SetGrpAttr(0, &stVpssGrpAttr)) != CVI_SUCCESS) {
            CVI_LOGE("MAPI_VPROC_SetGrpAttr failed, s32Ret:%x", s32Ret);
            return s32Ret;
        }
    }

#ifdef CROP_OUT
    /*   (0,0)                                                   w0
    *       -----------------------------------------------------
    *       |    (x,y)               w                          |
    *       |      -------------------------------------        |
    *       |      |                                    |       |
    *       |    h |           CROP_OUT                 |       |
    *       |      |                                    |       |
    *       |      -------------------------------------        |
    *       |                                                   |
    *    h0   -----------------------------------------------------
    */
    // uint32_t in_w = video_frame->stVFrame.u32Width;
    // uint32_t in_h = video_frame->stVFrame.u32Height;
    // uint32_t resize_width = VO_PANEL_WIDTH;
    // uint32_t resize_height = VO_PANEL_HEIGHT;
    // double x_scale = 1.0 * in_w / resize_width;
    // double y_scale = 1.0 * in_h / resize_height;
    // MAPI_IPROC_RESIZE_CROP_ATTR_T crop_out = {
    //     .x = 480,
    //     .y = 110,
    //     .w = 960,
    //     .h = 220,
    // };
    MAPI_IPROC_RESIZE_CROP_ATTR_T crop_out = {
        .x = 0,
        .y = 0,
        .w = 400,
        .h = video_frame->stVFrame.u32Height,
    };
    MAPI_IPROC_RESIZE_CROP_ATTR_T crop_out_adjusted;
    memset(&crop_out_adjusted, 0x00, sizeof(crop_out_adjusted));
    // crop_out_adjusted.x = crop_out.x * x_scale;
    // crop_out_adjusted.y = crop_out.y * y_scale;
    // crop_out_adjusted.w = crop_out.w * x_scale;
    // crop_out_adjusted.h = crop_out.h * y_scale;
    crop_out_adjusted.x = crop_out.x;
    crop_out_adjusted.y = crop_out.y;
    crop_out_adjusted.w = crop_out.w;
    crop_out_adjusted.h = crop_out.h;

    VPSS_CROP_INFO_S stCropOutInfo;
    stCropOutInfo.bEnable = CVI_TRUE;
    stCropOutInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stCropOutInfo.stCropRect.s32X      = crop_out_adjusted.x;
    stCropOutInfo.stCropRect.s32Y      = crop_out_adjusted.y;
    stCropOutInfo.stCropRect.u32Width  = crop_out_adjusted.w;
    stCropOutInfo.stCropRect.u32Height = crop_out_adjusted.h;
    CVI_LOGE("Crop OUT, %d %d %d %d\n",
            stCropOutInfo.stCropRect.s32X,
            stCropOutInfo.stCropRect.s32Y,
            stCropOutInfo.stCropRect.u32Width,
            stCropOutInfo.stCropRect.u32Height);
    s32Ret = CVI_VPSS_SetChnCrop(0, 0, &stCropOutInfo);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_SetChnCrop failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }
#endif

    return CVI_SUCCESS;
}

void vo_cb(void *arg, PLAYER_FRAME_S *video_frame)
{
    if (video_frame == NULL) {
        CVI_LOGW("Frame is null");
        return;
    }

    MAPI_DISP_HANDLE_T psdisp = (MAPI_DISP_HANDLE_T)arg;
    if (VideoCustom) { // 硬解
        VIDEO_FRAME_INFO_S vo_frame = {};
        if (MAPI_AllocateFrame(&vo_frame, video_frame->width,
                video_frame->height, PIXEL_FORMAT_NV21) != 0) {
            CVI_LOGE("MAPI allocate frame failed");
            return;
        }

        MAPI_FrameMmap(&vo_frame, true);
        for (int i = 0; i < FRAME_PLANAR_NUM; ++i) {
            memcpy((void *)vo_frame.stVFrame.pu8VirAddr[i],
                (const void *)video_frame->data[i], vo_frame.stVFrame.u32Length[i]);
            vo_frame.stVFrame.u32Stride[i] = video_frame->linesize[i];
        }
        MAPI_FrameFlushCache(&vo_frame);
        MAPI_FrameMunmap(&vo_frame);

        if (MAPI_DISP_SendFrame(psdisp, &vo_frame) != 0){
            CVI_LOGE("MAPI disp send frame failed");
            MAPI_ReleaseFrame(&vo_frame);
            return;
        }
        // first send vo have no picture
        if (send_vo_again) {
            if (MAPI_DISP_SendFrame(psdisp, &vo_frame) != 0){
                CVI_LOGE("MAPI disp send frame failed");
                MAPI_ReleaseFrame(&vo_frame);
                return;
            }
            send_vo_again = false;
        }
        MAPI_ReleaseFrame(&vo_frame);
    } else { //软解
        int dst_width = CVI_ALIGN_UP(video_frame->width, 64);
        // int dst_height = CVI_ALIGN_UP(video_frame->height, 64);
        int dst_height = video_frame->height;
        VIDEO_FRAME_INFO_S rescale_frame = {};
        if (MAPI_AllocateFrame(&rescale_frame, dst_width, dst_height, PIXEL_FORMAT_YUV_PLANAR_420) != 0) {
                CVI_LOGE("MAPI allocate frame failed");
                return;
        }
        MAPI_FrameMmap(&rescale_frame, true);
        if (((video_frame->width & 63) != 0)) {

            rescale_frame.stVFrame.u32Stride[0] = dst_width;
            rescale_frame.stVFrame.u32Stride[1] = dst_width>>1;
            rescale_frame.stVFrame.u32Stride[2] = dst_width>>1;

            int src_width = video_frame->width;
            int src_height = video_frame->height;

            int frame_linesize_0 = video_frame->linesize[0];
            int frame_linesize_1 = video_frame->linesize[1];
            int frame_linesize_2 = video_frame->linesize[2];
            uint8_t* frame_data_0 = video_frame->data[0];
            uint8_t* frame_data_1 = video_frame->data[1];
            uint8_t* frame_data_2 = video_frame->data[2];
            int dst_frame_linesize_0 = rescale_frame.stVFrame.u32Stride[0];
            int dst_frame_linesize_1 = rescale_frame.stVFrame.u32Stride[1];
            int dst_frame_linesize_2 = rescale_frame.stVFrame.u32Stride[2];
            uint8_t* dst_frame_data_0 = rescale_frame.stVFrame.pu8VirAddr[0];
            uint8_t* dst_frame_data_1 = rescale_frame.stVFrame.pu8VirAddr[1];
            uint8_t* dst_frame_data_2 = rescale_frame.stVFrame.pu8VirAddr[2];

            register int scale_x = (src_width<<8) / dst_width;
            register int scale_y = (src_height<<8) / dst_height;

            int* src_x = (int*)malloc(sizeof(int) * dst_width);
            if (src_x == NULL) {
                CVI_LOGE("malloc fail");
                return;
            }
            for (register int x = 0; x < dst_width; x += 2) {
                int x1 = x + 1;
                src_x[x] = (x * scale_x)>>8;
                src_x[x1] = (x1 * scale_x)>>8;
            }

            for (register int y = 0; y < dst_height; y +=2) {
                int y0 = y;
                int y1 = y + 1;
                int src_y0 = (y0 * scale_y)>>8;
                int src_y1 = (y1* scale_y)>>8;
                uint8_t* src_p_y0 = frame_data_0 + src_y0 * frame_linesize_0;

                uint8_t* src_p_y1 = frame_data_0 + src_y1 * frame_linesize_0;
                uint8_t* src_p_u1 = frame_data_1 + (src_y1 >> 1) * frame_linesize_1;
                uint8_t* src_p_v1 = frame_data_2 + (src_y1 >> 1) * frame_linesize_2;

                uint8_t* dst_p_y0 = dst_frame_data_0 + y0 * dst_frame_linesize_0;

                uint8_t* dst_p_y1 = dst_frame_data_0 + y1 * dst_frame_linesize_0;
                uint8_t* dst_p_u1 = dst_frame_data_1 + (y1 >> 1) * dst_frame_linesize_1;
                uint8_t* dst_p_v1 = dst_frame_data_2 + (y1 >> 1) * dst_frame_linesize_2;

                for (register int x = 0; x < dst_width; x += 4) {
                    int x0 = x;
                    int x1 = x + 1;
                    int x2 = x + 2;
                    int x3 = x + 3;
                    int src_x0 = src_x[x0];
                    int src_x1 = src_x[x1];

                    uint8_t* dst_p_y_00 = x0 + dst_p_y0;
                    uint8_t* dst_p_y_10 = x1 + dst_p_y0;
                    *dst_p_y_00 = *(src_x0 + src_p_y0);
                    *dst_p_y_10 = *(src_x1 + src_p_y0);
                    uint8_t* dst_p_y_01 = x0 + dst_p_y1;
                    uint8_t* dst_p_y_11 = x1 + dst_p_y1;
                    *dst_p_y_01 = *(src_x0 + src_p_y1);
                    *dst_p_y_11 = *(src_x1 + src_p_y1);

                    int src_x2 = src_x[x2];
                    int src_x3 = src_x[x3];
                    uint8_t* dst_p_y_20 = x2 + dst_p_y0;
                    uint8_t* dst_p_y_30 = x3 + dst_p_y0;
                    *dst_p_y_20 = *(src_x2 + src_p_y0);
                    *dst_p_y_30 = *(src_x3 + src_p_y0);
                    uint8_t* dst_p_y_21 = x2 + dst_p_y1;
                    uint8_t* dst_p_y_31 = x3 + dst_p_y1;
                    *dst_p_y_21 = *(src_x2 + src_p_y1);
                    *dst_p_y_31 = *(src_x3 + src_p_y1);

                    uint8_t* dst_p_u_11 = (x1 >> 1) + dst_p_u1;
                    uint8_t* dst_p_v_11 = (x1 >> 1) + dst_p_v1;
                    *dst_p_u_11 = *((src_x1 >> 1) + src_p_u1);
                    *dst_p_v_11 = *((src_x1 >> 1) +src_p_v1);
                    uint8_t* dst_p_u_31 = (x3 >> 1) + dst_p_u1;
                    uint8_t* dst_p_v_31 = (x3 >> 1) + dst_p_v1;
                    *dst_p_u_31 = *((src_x3 >> 1) + src_p_u1);
                    *dst_p_v_31 = *((src_x3 >> 1) +src_p_v1);
                }

            }
            free(src_x);
            src_x = NULL;
        } else {
            for (int i = 0; i < 3; ++i) {
            memcpy((void *)rescale_frame.stVFrame.pu8VirAddr[i],
                (const void *)video_frame->data[i], rescale_frame.stVFrame.u32Length[i]);
            rescale_frame.stVFrame.u32Stride[i] = video_frame->linesize[i];
            }
        }
        MAPI_FrameFlushCache(&rescale_frame);
        MAPI_FrameMunmap(&rescale_frame);

        if (VpssAttrReset(&rescale_frame) != 0) {
            CVI_LOGE("VpssAttrReset failed");
            MAPI_ReleaseFrame(&rescale_frame);
            return;
        }

        int32_t s32Ret = CVI_VPSS_SendFrame(0, &rescale_frame, 1000);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("CVI_VPSS_SendFrame failed with %#x\n", s32Ret);
            MAPI_ReleaseFrame(&rescale_frame);
            return;
        }
        MAPI_ReleaseFrame(&rescale_frame);

        VIDEO_FRAME_INFO_S resize_frame = {};
        s32Ret = CVI_VPSS_GetChnFrame(0, 0, &resize_frame, 1000);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
            return;
        }

        if (MAPI_DISP_SendFrame(psdisp, &resize_frame) != 0){
            CVI_LOGE("MAPI disp send frame failed");
            MAPI_ReleaseFrame(&resize_frame);
            return;
        }
        // first send vo have no picture
        if (send_vo_again) {
            if (MAPI_DISP_SendFrame(psdisp, &resize_frame) != 0){
                CVI_LOGE("MAPI disp send frame failed");
                MAPI_ReleaseFrame(&resize_frame);
                return;
            }
            send_vo_again = false;
        }
        MAPI_ReleaseFrame(&resize_frame);
    }
}

static int initVO()
{
    MAPI_DISP_ATTR_T disp_attr = {};
    MAPI_DISP_VIDEOLAYER_ATTR_S layer_attr = {};
    disp_attr.rotate = ROTATION_90; //旋转90度
    disp_attr.window_mode = false;
    disp_attr.stPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
    disp_attr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    layer_attr.u32BufLen = 3;
    layer_attr.u32PixelFmt = 19; //12 yuv422 //13 yuv420;
    extern HAL_SCREEN_OBJ_S stHALSCREENObj;

    if (0 != HAL_SCREEN_COMM_Register(HAL_SCREEN_IDXS_0, &stHALSCREENObj)) {
        CVI_LOGE("HAL_SCREEN_Register fail");
        return -1;
    };

    if (0 != HAL_SCREEN_COMM_Init(HAL_SCREEN_IDXS_0)) {
        CVI_LOGE("HAL_SCREEN_Init fail");
        return -1;
    };


    HAL_SCREEN_ATTR_S screen_attr = {};
    if (0 != HAL_SCREEN_COMM_GetAttr(HAL_SCREEN_IDXS_0, &screen_attr)) {
        CVI_LOGE("HAL_SCREEN_GetAttr fail");
        return -1;
    };

    switch(screen_attr.enType) {
        case HAL_COMP_SCREEN_INTF_TYPE_MIPI:
            disp_attr.stPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        case HAL_COMP_SCREEN_INTF_TYPE_LCD:
            break;
        default:
            CVI_LOGE("Invalid screen type");
    }

    int32_t screen_width = 0, screen_height = 0;
    if ((disp_attr.rotate == ROTATION_90) ||
        (disp_attr.rotate == ROTATION_270)) {
        screen_width = screen_attr.stAttr.u32Height;
        screen_height = screen_attr.stAttr.u32Width;
    } else {
        screen_width = screen_attr.stAttr.u32Width;
        screen_height = screen_attr.stAttr.u32Height;
    }

    disp_attr.width = screen_width;
    disp_attr.height = screen_height;
    disp_attr.stPubAttr.stSyncInfo.bSynm = 1;
    disp_attr.stPubAttr.stSyncInfo.bIop = 1;
    disp_attr.stPubAttr.stSyncInfo.u16FrameRate = screen_attr.stAttr.u32Framerate;
    disp_attr.stPubAttr.stSyncInfo.u16Vact = screen_attr.stAttr.stSynAttr.u16Vact;
    disp_attr.stPubAttr.stSyncInfo.u16Vbb = screen_attr.stAttr.stSynAttr.u16Vbb;
    disp_attr.stPubAttr.stSyncInfo.u16Vfb = screen_attr.stAttr.stSynAttr.u16Vfb;
    disp_attr.stPubAttr.stSyncInfo.u16Hact = screen_attr.stAttr.stSynAttr.u16Hact;
    disp_attr.stPubAttr.stSyncInfo.u16Hbb = screen_attr.stAttr.stSynAttr.u16Hbb;
    disp_attr.stPubAttr.stSyncInfo.u16Hfb = screen_attr.stAttr.stSynAttr.u16Hfb;
    disp_attr.stPubAttr.stSyncInfo.u16Hpw = screen_attr.stAttr.stSynAttr.u16Hpw;
    disp_attr.stPubAttr.stSyncInfo.u16Vpw = screen_attr.stAttr.stSynAttr.u16Vpw;
    disp_attr.stPubAttr.stSyncInfo.bIdv = screen_attr.stAttr.stSynAttr.bIdv;
    disp_attr.stPubAttr.stSyncInfo.bIhs = screen_attr.stAttr.stSynAttr.bIhs;
    disp_attr.stPubAttr.stSyncInfo.bIvs = screen_attr.stAttr.stSynAttr.bIvs;

    disp_attr.pixel_format = (PIXEL_FORMAT_E)layer_attr.u32PixelFmt;

    layer_attr.u32VLFrameRate = screen_attr.stAttr.u32Framerate;
    layer_attr.stImageSize.u32Width  = screen_attr.stAttr.u32Width;
    layer_attr.stImageSize.u32Height = screen_attr.stAttr.u32Height;
    if (MAPI_DISP_Init(&psdisp, 0, &disp_attr) != 0) {
        CVI_LOGE("MAPI disp init failed");
        return CVI_FAILURE;
    }

    if (MAPI_DISP_Start(psdisp, &layer_attr) != 0) {
        CVI_LOGE("MAPI disp start failed");
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

CVI_S32 Play_Sample_Vdec_Start(vdecChnCtx *pvdchnCtx)
{
	VDEC_CHN_ATTR_S stChnAttr, *pstChnAttr = &stChnAttr;
	VDEC_CHN VdecChn = pvdchnCtx->VdecChn;
	SAMPLE_VDEC_ATTR *psvdattr = &pvdchnCtx->stSampleVdecAttr;
	VDEC_CHN_PARAM_S stChnParam;
	VDEC_MOD_PARAM_S stModParam;

	pstChnAttr->enType = psvdattr->enType;
	pstChnAttr->enMode = psvdattr->enMode;
	pstChnAttr->u32PicWidth = psvdattr->u32Width;
	pstChnAttr->u32PicHeight = psvdattr->u32Height;
	pstChnAttr->u32StreamBufSize = ALIGN(psvdattr->u32Width * psvdattr->u32Height, 0x4000);
	printf("u32StreamBufSize = 0x%X\n", pstChnAttr->u32StreamBufSize);
	pstChnAttr->u32FrameBufCnt = psvdattr->u32FrameBufCnt;

	if (psvdattr->enType == PT_JPEG || psvdattr->enType == PT_MJPEG) {
		pstChnAttr->enMode = VIDEO_MODE_FRAME;
		pstChnAttr->u32FrameBufSize = VDEC_GetPicBufferSize(
				pstChnAttr->enType, psvdattr->u32Width, psvdattr->u32Height,
				psvdattr->enPixelFormat, DATA_BITWIDTH_8, COMPRESS_MODE_NONE);
	}

	if (g_VdecVbSrc != VB_SOURCE_COMMON) {
		CVI_VDEC_GetModParam(&stModParam);
		stModParam.enVdecVBSource = g_VdecVbSrc;
		CVI_VDEC_SetModParam(&stModParam);
	}

	if (pvdchnCtx->bCreateChn == CVI_FALSE) {
		CHECK_CHN_RET(CVI_VDEC_CreateChn(VdecChn, pstChnAttr), VdecChn, "CVI_VDEC_CreateChn");
		pvdchnCtx->bCreateChn = CVI_TRUE;
	} else {
		CHECK_CHN_RET(CVI_VDEC_SetChnAttr(VdecChn, pstChnAttr), VdecChn, "CVI_VDEC_SetChnAttr");
	}

	CHECK_CHN_RET(CVI_VDEC_GetChnParam(VdecChn, &stChnParam), VdecChn, "CVI_VDEC_GetChnParam");

	if (psvdattr->enType == PT_H264 || psvdattr->enType == PT_H265) {
	} else {
		stChnParam.stVdecPictureParam.u32Alpha =
			psvdattr->stSampleVdecPicture.u32Alpha;
	}
	stChnParam.enPixelFormat = psvdattr->enPixelFormat;
	stChnParam.u32DisplayFrameNum = psvdattr->u32DisplayFrameNum;

	CHECK_CHN_RET(CVI_VDEC_SetChnParam(VdecChn, &stChnParam), VdecChn, "CVI_MPI_VDEC_GetChnParam");

	CHECK_CHN_RET(CVI_VDEC_StartRecvStream(VdecChn), VdecChn, "CVI_MPI_VDEC_StartRecvStream");

	return CVI_SUCCESS;
}

static int initVdec(PAYLOAD_TYPE_E encode_type, int width, int height)
{
    vdecChnCtx chnCtx = {};
    chnCtx.VdecChn = VDEC_CHANNEL;
    vdecInitAttr(&chnCtx.stSampleVdecAttr, encode_type, width, height);
    CVI_S32 s32Ret = Play_Sample_Vdec_Start(&chnCtx);
    if (CVI_SUCCESS != s32Ret) {
        CVI_LOGE("SAMPLE_COMM_VDEC_Start s32Ret: 0x%x !\n", s32Ret);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static CVI_S32 vdecInitAttr(SAMPLE_VDEC_ATTR *psvdattr, PAYLOAD_TYPE_E encode_type, int width, int height)
{
    CVI_S32 s32Ret = CVI_SUCCESS;

    psvdattr->enType = encode_type;
    psvdattr->u32Width = width;
    psvdattr->u32Height = height;
    psvdattr->enMode = VIDEO_MODE_FRAME;
    psvdattr->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420;
    psvdattr->stSampleVdecPicture.u32Alpha = 255;
    psvdattr->u32DisplayFrameNum = 2;
    psvdattr->u32FrameBufCnt =
        psvdattr->u32DisplayFrameNum + 1;

    return s32Ret;
}

static int initAO()
{
    AIO_ATTR_S AudoutAttr;
    AudoutAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;
    AudoutAttr.u32ChnCnt = 1;
    AudoutAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    AudoutAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    AudoutAttr.u32EXFlag = 0;
    AudoutAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    AudoutAttr.u32PtNumPerFrm = 640;
    AudoutAttr.u32ClkSel = 0;
    AudoutAttr.enI2sType = AIO_I2STYPE_INNERCODEC;
    AudoutAttr.u32FrmNum      = 10; /* only use in bind mode */


    CVI_S32 s32Ret = CVI_AUDIO_INIT();
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_AUDIO_INIT failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AO_SetPubAttr(0, &AudoutAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("set audio public attr failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AO_Enable(0);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("ao enable failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AO_SetVolume(0, 32);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_AO_SetVolume failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = HAL_GPIO_Set_Value(HAL_GPIOA_15, HAL_GPIO_VALUE_H);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_AO_SetVolume failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_AO_EnableChn(0, 0);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("ao channel 0 enable failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

void aoHandler(PLAYER_FRAME_S *frame)
{
    if ((frame == NULL) || (frame->packet_size <= 0)) {
        return;
    }

    static AUDIO_FRAME_S audio_frame;
    audio_frame.u64VirAddr[0] = frame->data[0];
    audio_frame.u32Len = frame->packet_size/2;
    audio_frame.enBitwidth = AUDIO_BIT_WIDTH_16;

    CVI_S32 s32Ret = CVI_AO_SendFrame(0, 0, (const AUDIO_FRAME_S *)&audio_frame, 1000);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGI("CVI_AO_SendFrame failed with %#x!\n", s32Ret);
    }
}

CVI_S32 Play_sample_COMM_PrepareFrame(SIZE_S stSize, PIXEL_FORMAT_E enPixelFormat, VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	VB_BLK blk;
	VB_CAL_CONFIG_S stVbCalConfig;

	if (pstVideoFrame == CVI_NULL) {
		CVI_LOGE("Null pointer!\n");
		return CVI_FAILURE;
	}

	COMMON_GetPicBufferConfig(stSize.u32Width, stSize.u32Height, enPixelFormat, DATA_BITWIDTH_8
		, COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

	memset(pstVideoFrame, 0, sizeof(*pstVideoFrame));
	pstVideoFrame->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
	pstVideoFrame->stVFrame.enPixelFormat = enPixelFormat;
	pstVideoFrame->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
	pstVideoFrame->stVFrame.enColorGamut = COLOR_GAMUT_BT601;
	pstVideoFrame->stVFrame.u32Width = stSize.u32Width;
	pstVideoFrame->stVFrame.u32Height = stSize.u32Height;
	pstVideoFrame->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
	pstVideoFrame->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
	pstVideoFrame->stVFrame.u32TimeRef = 0;
	pstVideoFrame->stVFrame.u64PTS = 0;
	pstVideoFrame->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

	blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
	if (blk == VB_INVALID_HANDLE) {
		CVI_LOGE("Can't acquire vb block\n");
		return CVI_FAILURE;
	}

	pstVideoFrame->u32PoolId = CVI_VB_Handle2PoolId(blk);
	pstVideoFrame->stVFrame.u32Length[0] = stVbCalConfig.u32MainYSize;
	pstVideoFrame->stVFrame.u32Length[1] = stVbCalConfig.u32MainCSize;
	pstVideoFrame->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
	pstVideoFrame->stVFrame.u64PhyAddr[1] = pstVideoFrame->stVFrame.u64PhyAddr[0]
		+ ALIGN(stVbCalConfig.u32MainYSize, stVbCalConfig.u16AddrAlign);
	if (stVbCalConfig.plane_num == 3) {
		pstVideoFrame->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
		pstVideoFrame->stVFrame.u32Length[2] = stVbCalConfig.u32MainCSize;
		pstVideoFrame->stVFrame.u64PhyAddr[2] = pstVideoFrame->stVFrame.u64PhyAddr[1]
			+ ALIGN(stVbCalConfig.u32MainCSize, stVbCalConfig.u16AddrAlign);
	}

	return CVI_SUCCESS;
}

void eventHandler(PLAYER_HANDLE_T player_handle, PLAYER_EVENT_S *event)
{
    if (event == NULL) {
        return;
    }

    switch(event->type) {
        case PLAYER_EVENT_PLAY_FINISHED:
            CVI_LOGI("Player play finish\n");
            usage();
            break;
        case PLAYER_EVENT_PAUSE:
            CVI_LOGI("Player pause\n");
            // CVI_AO_DisableChn(0, 0);
            // CVI_AO_Disable(0);
            break;
        case PLAYER_EVENT_RESUME:
            CVI_LOGI("Player resume\n");
            // CVI_AO_Enable(0);
            // CVI_AO_EnableChn(0, 0);
            break;
        default:
            break;
    }
}

int getFrameHandler(void *arg, PLAYER_FRAME_S *frame)
{
    UNUSED(arg);
    VIDEO_FRAME_INFO_S vdec_frame = {0};
    CVI_S32 s32Ret = CVI_VDEC_GetFrame(VDEC_CHANNEL, &vdec_frame, 0);
    if (s32Ret != CVI_SUCCESS) {
        return s32Ret;
    }

    VPSS_GRP_ATTR_S stVpssGrpAttr = {0};
    s32Ret = CVI_VPSS_GetGrpAttr(0, &stVpssGrpAttr);
    if (s32Ret != CVI_SUCCESS) {
        return s32Ret;
    }
    if (stVpssGrpAttr.enPixelFormat != vdec_frame.stVFrame.enPixelFormat) {
        stVpssGrpAttr.enPixelFormat = vdec_frame.stVFrame.enPixelFormat;
    }
    if (stVpssGrpAttr.u32MaxW != vdec_frame.stVFrame.u32Width) {
        stVpssGrpAttr.u32MaxW = vdec_frame.stVFrame.u32Width;
    }
    if (stVpssGrpAttr.u32MaxH != vdec_frame.stVFrame.u32Height) {
        stVpssGrpAttr.u32MaxH = vdec_frame.stVFrame.u32Height;
    }
    s32Ret = CVI_VPSS_SetGrpAttr(0, &stVpssGrpAttr);
    if (s32Ret != CVI_SUCCESS) {
        return s32Ret;
    }

    if (0 == sem_trywait(&semDownload)) {
        CVI_LOGE("vdec frame w == %d, h == %d, format == %d\n", vdec_frame.stVFrame.u32Width, vdec_frame.stVFrame.u32Height, vdec_frame.stVFrame.enPixelFormat);
        //MAPI_SaveFramePixelData(&vdec_frame, "/mnt/sd/dump1.yuv");
        dump_yuv("/mnt/sd/dump1.yuv", vdec_frame.stVFrame);
        CVI_LOGE("Save image from getFrameHandler success\n");
    }

    VIDEO_FRAME_INFO_S resize_frame = {0};

    s32Ret = CVI_VPSS_SendFrame(0, &vdec_frame, 1000);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_SendFrame failed with %#x\n", s32Ret);
        CVI_VDEC_ReleaseFrame(0, &vdec_frame);
        return s32Ret;
    }
    CVI_VDEC_ReleaseFrame(0, &vdec_frame);

    s32Ret = CVI_VPSS_GetChnFrame(0, 0, &resize_frame, 1000);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
        return s32Ret;
    }

    // copy resized frame to result video frame
    MAPI_FrameMmap(&resize_frame, true);
    for (int i = 0; i < FRAME_PLANAR_NUM; ++i) {
        memcpy((void *)frame->data[i], (const void *)resize_frame.stVFrame.pu8VirAddr[i], resize_frame.stVFrame.u32Length[i]);
        frame->linesize[i] = resize_frame.stVFrame.u32Stride[i];

    }
    MAPI_FrameFlushCache(&resize_frame);
    MAPI_FrameMunmap(&resize_frame);
    MAPI_ReleaseFrame(&resize_frame);

    frame->width = resize_frame.stVFrame.u32Width;
    frame->height = resize_frame.stVFrame.u32Height;
    return 0;
}

static int sendPacketToDecoder(PLAYER_PACKET_S *packet)
{
    VDEC_STREAM_S stStream;
    stStream.u64PTS = packet->pts;
    stStream.pu8Addr = packet->data;

    stStream.u32Len = packet->size;
    stStream.bEndOfFrame = CVI_TRUE;
    stStream.bEndOfStream = CVI_FALSE;
    stStream.bDisplay = 1;
    int ret = 0;
    ret = CVI_VDEC_SendStream(VDEC_CHANNEL, &stStream, 1000);

    return ret;
}

int decodePacketHandler(void *arg, PLAYER_PACKET_S *packet)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CustomArg *custom_arg = (CustomArg *)arg;

    if ((packet == NULL) || (packet->data == NULL)) {
        return -1;
    }

    // find first sps
    if (!find_first_sps) {
        if (PLAYER_PacketContainSps(custom_arg->player_handle, packet)) {
            find_first_sps = true;
        } else {
            PLAYER_PACKET_S extra_packet;
            // PLAYER_GetMediumVideoExtraPacket(custom_arg->player_handle, &extra_packet);
            PLAYER_GetVideoExtraPacket(custom_arg->player_handle, &extra_packet);
            if (PLAYER_PacketContainSps(custom_arg->player_handle, &extra_packet)) {
                find_first_sps = true;
                sendPacketToDecoder(&extra_packet);
            }
        }
    }

    // video decoder need send sps at the beginning
    if (find_first_sps) {
        sendPacketToDecoder(packet);
    }

    return s32Ret;
}

static void handleOp(PLAYER_HANDLE_T player, char op)
{
    switch(op) {
        case 'p':
            PLAYER_Pause(player);
            break;
        case 'r':
            PLAYER_Resume(player);
            break;
        case 's':
            CVI_LOGI("Save image from getFrameHandler start\n");
            sem_post(&semDownload);
            break;
        case 'd':
            CVI_LOGI("download image from PLAYER_SaveImage start\n");
            PLAYER_SaveImage(player, "/mnt/sd/image.yuv");
            system("sync");
            CVI_LOGI("download image from PLAYER_SaveImage finish\n");
            break;
        case 'i':
            {
               PLAYER_MEDIA_INFO_S info;
                PLAYER_GetMediaInfo(player, &info);
                CVI_LOGI("File: %s Media format:%s, Video codec:%s, Audio codec:%s, duration: %lf, width: %d, height: %d\n",
                    info.file_name, info.format, info.video_codec, info.audio_codec, info.duration_sec,
                    info.width, info.height);
                break;
            }
        // case 't':
        //     PLAYER_Stop(player);
        //     break;
        // case 'g':
        //     PLAYER_Play(player);
        //     break;
        case 'a':
            CVI_LOGI("Player restart again\n");
            CVI_LOGI("Player is playing\n");
            PLAYER_Seek(player, 0);
            break;
        default:
            break;
    }
}

static int32_t deinit_display(MAPI_DISP_HANDLE_T disp_handle)
{
    MAPI_DISP_Stop(disp_handle);
    MAPI_DISP_Deinit(disp_handle);

    return CVI_SUCCESS;
}

static void usage()
{
    printf("==== Command ====\n");
    printf("p: pause\n");
    printf("r: resume/replay\n");
    printf("s: save image from getFrameHandler, used for hardware decoder\n");
    printf("d: save image from PLAYER_SaveImage, used for software decoder\n");
    printf("i: print file info\n");
    printf("a: restart again\n");
    // CVI_LOGI("t: stop\n");
    // CVI_LOGI("g: play\n");
    printf("q: quit\n");
    printf("================\n");
}

CVI_S32 Play_Sample_COMM_VDEC_Stop(CVI_S32 s32ChnNum)
{
	CVI_S32 i;

	for (i = 0; i < s32ChnNum; i++) {
		CHECK_CHN_RET(CVI_VDEC_StopRecvStream(i), i, "CVI_MPI_VDEC_StopRecvStream");
		if (g_VdecVbSrc == VB_SOURCE_USER) {
			CHECK_CHN_RET(CVI_VDEC_DetachVbPool(i), i, "CVI_VDEC_DetachVbPool");
		}
		CHECK_CHN_RET(CVI_VDEC_ResetChn(i), i, "CVI_MPI_VDEC_ResetChn");
		CHECK_CHN_RET(CVI_VDEC_DestroyChn(i), i, "CVI_MPI_VDEC_DestroyChn");
	}

	return CVI_SUCCESS;
}

CVI_VOID Play_Sample_COMM_SYS_Exit(void)
{
	CVI_VB_Exit();
	CVI_SYS_Exit();
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        CVI_LOGE("usage: %s video_file_path\n", argv[0]);
        return 0;
    }

    VideoCustom = true;

    sem_init(&semDownload, 0, 0);
    PLAYER_Init();
    PLAYER_HANDLE_T player = NULL;
    PLAYER_Create(&player);
    PLAYER_SetDataSource(player, argv[1]);
    PLAYER_SetAOHandler(player, aoHandler);
    PLAYER_SetCustomArgEventHandler(player, eventHandler, player);
    PLAYER_MEDIA_INFO_S info;
    memset(&info, 0, sizeof(PLAYER_MEDIA_INFO_S));
    info.height = 0;
    info.width = 0;
    if (0 != PLAYER_GetMediaInfo(player, &info)) {
        CVI_LOGE("get media info failed\n");
        return -1;
    }

    CustomArg custom_arg = {
        .player_handle = player,
        .encode_type = getPayloadTypeFromName(info.video_codec)
    };

    CVI_LOGE("#######%s, %d, info.video_codec:%s, %d, %d\n", __func__, __LINE__, info.video_codec, info.width, info.height);
    //char* p = strrchr(info.file_name, '.');
    //CVI_LOGE("#######%s, %d, %s\n", __func__, __LINE__, p+1);
    if ((strcmp(info.video_codec, "h264") == 0) || (strcmp(info.video_codec, "mjpeg") == 0)) {
        VideoCustom = true;
    } else {
        VideoCustom = false;
    }

    if (VideoCustom) {
        PLAYER_SetVideoCustomArgDecodeHandler(player, (PLAYER_CUSTOM_ARG_DECODE_HANDLER_S) {
        .get_frame = getFrameHandler,
        .decode_packet = decodePacketHandler
        }, &custom_arg);
    }

    if (0 != initSystem(info.width, info.height, custom_arg.encode_type)) {
        CVI_LOGE("init system failed\n");
        Play_Sample_COMM_VDEC_Stop(MAX_VDEC_CHANNEL);
        Play_Sample_COMM_SYS_Exit();
        return -1;
    }
    PLAYER_SetCustomArgVOHandler(player, vo_cb, (void *)&psdisp);

    PLAYER_SetVideoParameters(player, (PLAYER_VIDEO_PARAMETERS) {
        .output_width = ALIGN(1920, DEFAULT_ALIGN),
        .output_height = 440,
        .max_packet_size = 3686400
    });
    PLAYER_Play(player);

    usage();
    CVI_LOGI("Player is playing\n");

    char op;
    do {
        usleep(10000);
        fflush(stdout); // 确保提示被打印出来
        scanf("%c", &op); // 注意%c前面的空格
        handleOp(player, op);

    } while (op != 'q');

    sem_destroy(&semDownload);

    PLAYER_Stop(player);
    CVI_AO_DisableChn(0, 0);
    CVI_AO_Disable(0);
    PLAYER_Destroy(&player);
    PLAYER_Deinit();
    deinit_display(psdisp);
    Play_Sample_COMM_VDEC_Stop(MAX_VDEC_CHANNEL);
    Play_Sample_COMM_SYS_Exit();

    return 0;
}


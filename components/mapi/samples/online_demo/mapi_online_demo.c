#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#include "mapi.h"
#include "cvi_sys.h"
#include "cvi_log.h"
#include "hal_screen_comp.h"


#ifndef CHECK_RET
#define CHECK_RET(express)                                                    \
    do {                                                                      \
        int rc = express;                                                     \
        if (rc != 0) {                                                        \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n",                       \
                    __FILE__, __LINE__, rc);                                  \
            return rc;                                                        \
        }                                                                     \
    } while (0)
#endif

#define CHECK_RET_NULL(express)                                                    \
    do {                                                                      \
        int rc = express;                                                     \
        if (rc != 0) {                                                        \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n",                       \
                    __FILE__, __LINE__, rc);                                  \
            return NULL;                                                        \
        }                                                                     \
    } while (0)


#define TEST_PASS printf("%s\n------\nSUCCESS\n------\n", __FUNCTION__)

MAPI_VCAP_ATTR_T g_vcap_attr;
MAPI_VCAP_SENSOR_HANDLE_T g_sns_hdl[VI_MAX_DEV_NUM];
MAPI_WND_HANDLE_T wnd_hdl[2];
static MEDIA_VideoOutCfg videoOutCfg;
static int t_sec;



static int DispOpen(CVI_U32 width, CVI_U32 height, ROTATION_E rotate, bool window_mode)
{
    int ret = MAPI_SUCCESS;

    MEDIA_DispCfg *dispCfg = &videoOutCfg.dispCfg[0];

    dispCfg->dispAttr.width = width;
    dispCfg->dispAttr.height = height;
    dispCfg->dispAttr.rotate = rotate;
    dispCfg->dispAttr.window_mode = window_mode;
    dispCfg->dispAttr.stPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
    dispCfg->dispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    dispCfg->videoLayerAttr.u32BufLen = 3;
    dispCfg->videoLayerAttr.u32PixelFmt = PIXEL_FORMAT_YUV_PLANAR_420;

	extern HAL_SCREEN_OBJ_S stHALSCREENObj;
    CHECK_RET(HAL_SCREEN_COMM_Register(HAL_SCREEN_IDXS_0, &stHALSCREENObj));
    CHECK_RET(HAL_SCREEN_COMM_Init(HAL_SCREEN_IDXS_0));
    HAL_SCREEN_ATTR_S screenAttr = {0};
    HAL_SCREEN_COMM_GetAttr(HAL_SCREEN_IDXS_0, &screenAttr);
    switch(screenAttr.enType) {
        case HAL_COMP_SCREEN_INTF_TYPE_MIPI:
            dispCfg->dispAttr.stPubAttr.enIntfType = VO_INTF_MIPI;
            break;
        case HAL_COMP_SCREEN_INTF_TYPE_LCD:
        default:
            CVI_LOGD("Invalid screen type\n");
            return MAPI_ERR_FAILURE;
    }

    dispCfg->dispAttr.stPubAttr.stSyncInfo.bSynm   = 1; /**<sync mode: signal */
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIop    = 1; /**<progressive display */
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16FrameRate = screenAttr.stAttr.u32Framerate;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vact = screenAttr.stAttr.stSynAttr.u16Vact;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vbb  = screenAttr.stAttr.stSynAttr.u16Vbb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vfb  = screenAttr.stAttr.stSynAttr.u16Vfb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hact = screenAttr.stAttr.stSynAttr.u16Hact;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hbb  = screenAttr.stAttr.stSynAttr.u16Hbb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hfb  = screenAttr.stAttr.stSynAttr.u16Hfb;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Hpw  = screenAttr.stAttr.stSynAttr.u16Hpw;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.u16Vpw  = screenAttr.stAttr.stSynAttr.u16Vpw;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIdv    = screenAttr.stAttr.stSynAttr.bIdv;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIhs    = screenAttr.stAttr.stSynAttr.bIhs;
    dispCfg->dispAttr.stPubAttr.stSyncInfo.bIvs    = screenAttr.stAttr.stSynAttr.bIvs;

    dispCfg->dispAttr.pixel_format = dispCfg->videoLayerAttr.u32PixelFmt;

    dispCfg->videoLayerAttr.u32VLFrameRate = screenAttr.stAttr.u32Framerate;
    dispCfg->videoLayerAttr.stImageSize.u32Width  = screenAttr.stAttr.u32Width;
    dispCfg->videoLayerAttr.stImageSize.u32Height = screenAttr.stAttr.u32Height;

    CHECK_RET(MAPI_DISP_Init(&dispCfg->dispHdl, 0, &dispCfg->dispAttr));
    CHECK_RET(MAPI_DISP_Start(dispCfg->dispHdl, &dispCfg->videoLayerAttr));

    return ret;
}

static int DispClose(MAPI_DISP_HANDLE_T disp_hdl)
{
    CHECK_RET(MAPI_DISP_Stop(disp_hdl));
    CHECK_RET(MAPI_DISP_Deinit(disp_hdl));

    return MAPI_SUCCESS;
}


static int online_demo_sys_init(void)
{
    MAPI_MEDIA_SYS_ATTR_T sys_attr = {0};

    memset(&g_vcap_attr, 0, sizeof(MAPI_VCAP_ATTR_T));
    uint8_t sns_num = g_vcap_attr.u8DevNum;
    CHECK_RET(MAPI_VCAP_GetGeneralVcapAttr(&g_vcap_attr, sns_num));

#if defined(SNS0_SONY_IMX335) && defined(SNS1_PIXELPLUS_PR2020)
    g_vcap_attr.attr_sns[0].u8I2cBusId = 0;
    g_vcap_attr.attr_chn[0].u32Width = 2560;
    g_vcap_attr.attr_chn[0].u32Height = 1600;
    g_vcap_attr.attr_chn[0].enPixelFmt = PIXEL_FORMAT_YUV_PLANAR_420;
    g_vcap_attr.attr_sns[1].u8I2cBusId = 3;
    g_vcap_attr.attr_chn[1].u32Width = 1920;
    g_vcap_attr.attr_chn[1].u32Height = 1080;
    g_vcap_attr.attr_chn[1].enPixelFmt = PIXEL_FORMAT_YUV_PLANAR_422;
#endif

    sys_attr.vb_pool_num = 5;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = g_vcap_attr.attr_chn[0].u32Width;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = g_vcap_attr.attr_chn[0].u32Height;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    =PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[0].vb_blk_num = 2;

    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = g_vcap_attr.attr_chn[1].u32Width;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = g_vcap_attr.attr_chn[1].u32Height;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[1].vb_blk_num = 2;

    sys_attr.vb_pool[2].is_frame = true;
    sys_attr.vb_pool[2].vb_blk_size.frame.width  = 1920;
    sys_attr.vb_pool[2].vb_blk_size.frame.height = 440;
    sys_attr.vb_pool[2].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[2].vb_blk_num = 10;

    sys_attr.vb_pool[3].is_frame = true;
    sys_attr.vb_pool[3].vb_blk_size.frame.width  = 640;
    sys_attr.vb_pool[3].vb_blk_size.frame.height = 480;
    sys_attr.vb_pool[3].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[3].vb_blk_num = 6;

    sys_attr.vb_pool[4].is_frame = true;
    sys_attr.vb_pool[4].vb_blk_size.frame.width  = 320;
    sys_attr.vb_pool[4].vb_blk_size.frame.height = 180;
    sys_attr.vb_pool[4].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[4].vb_blk_num = 4;


    sys_attr.stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE;
    sys_attr.stVPSSMode.enMode = VPSS_MODE_DUAL;
    sys_attr.stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
    sys_attr.stVPSSMode.ViPipe[0] = 0;
    sys_attr.stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;
    sys_attr.stVPSSMode.ViPipe[1] = 0;

    CHECK_RET(MAPI_Media_Init(&sys_attr));

    return 0;
}

static int online_demo_sys_deinit(void)
{
    CHECK_RET(MAPI_Media_Deinit());
    return 0;
}

static int vcap_sns_init(void)
{
    int i, sns_num;
    sns_num = g_vcap_attr.u8DevNum;
    for (i = 0; i < sns_num; i++) {
        CHECK_RET(MAPI_VCAP_InitSensor(&g_sns_hdl[i], i, &g_vcap_attr));
        CHECK_RET(MAPI_VCAP_InitISP(g_sns_hdl[i]));
        // CHECK_RET(MAPI_VCAP_StartISP(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_StartDev(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_StartPipe(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_StartChn(g_sns_hdl[i]));

    }
    return 0;
}

static int vcap_sns_deinit(void)
{
    int i, sns_num;
    sns_num = g_vcap_attr.u8DevNum;
    for (i = 0; i < sns_num; i++) {
        // CHECK_RET(MAPI_VCAP_StopISP(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_DeInitISP(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_StopChn(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_StopPipe(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_StopDev(g_sns_hdl[i]));
        CHECK_RET(MAPI_VCAP_DeinitSensor(g_sns_hdl[i]));
    }
    return 0;
}

void TEST_GetVprocFrame(MAPI_VPROC_HANDLE_T *vproc_hdl)
{
	int ret;
	uint32_t chn_idx;
	VIDEO_FRAME_INFO_S stFrame;

	chn_idx = 0;
	ret = MAPI_VPROC_GetChnFrame(vproc_hdl[0], chn_idx, &stFrame);
	if (ret) {
		printf("MAPI_VPROC_GetChnFrame faile, vpss:%d chn:%d \n",
			MAPI_VPROC_GetGrp(vproc_hdl[0]), chn_idx);
		usleep(100*1000);
	}
	MAPI_VPROC_ReleaseFrame(vproc_hdl[0], chn_idx, &stFrame);

	chn_idx = 0;
	ret = MAPI_VPROC_GetChnFrame(vproc_hdl[1], chn_idx, &stFrame);
	if (ret) {
		printf("MAPI_VPROC_GetChnFrame faile, vpss:%d chn:%d \n",
			MAPI_VPROC_GetGrp(vproc_hdl[1]), chn_idx);
		usleep(100*1000);
	}
	MAPI_VPROC_ReleaseFrame(vproc_hdl[1], chn_idx, &stFrame);

	chn_idx = 0;
	ret = MAPI_VPROC_GetChnFrame(vproc_hdl[2], chn_idx, &stFrame);
	if (ret) {
		printf("MAPI_VPROC_GetChnFrame faile, vpss:%d chn:%d \n",
			MAPI_VPROC_GetGrp(vproc_hdl[2]), chn_idx);
		usleep(100*1000);
	}
	MAPI_VPROC_ReleaseFrame(vproc_hdl[2], chn_idx, &stFrame);

	chn_idx = 0;
	ret = MAPI_VPROC_GetChnFrame(vproc_hdl[3], chn_idx, &stFrame);
	if (ret) {
		printf("MAPI_VPROC_GetChnFrame faile, vpss:%d chn:%d \n",
			MAPI_VPROC_GetGrp(vproc_hdl[3]), chn_idx);
		usleep(100*1000);
	}
	MAPI_VPROC_ReleaseFrame(vproc_hdl[3], chn_idx, &stFrame);

	printf("=====test pass====\n");

}

CVI_STITCH_ATTR_S wnd_attr[2];
static void* online_JTD(void* args)
{
	int* Stop = (int*)args;

    if (g_vcap_attr.u8DevNum != VI_MAX_DEV_NUM) {
        printf("sns number is missmatch with sns cfg!!!\n");
        return NULL;
    }
    CHECK_RET_NULL(vcap_sns_init());

    MAPI_VPROC_HANDLE_T vproc_hdl[4];
    MAPI_VPROC_ATTR_T vproc_attr[4];


#if defined(SNS0_SONY_IMX335) && defined(SNS1_PIXELPLUS_PR2020)
    vproc_attr[0] = MAPI_VPROC_DefaultAttr_ThreeChn(
            2560, 1600, PIXEL_FORMAT_YUV_PLANAR_420,
            2560, 1600, PIXEL_FORMAT_YUV_PLANAR_420,
            1920, 440, PIXEL_FORMAT_YUV_PLANAR_420,
            640, 480, PIXEL_FORMAT_YUV_PLANAR_420);
    vproc_attr[1] = MAPI_VPROC_DefaultAttr_ThreeChn(
            1920, 1080, PIXEL_FORMAT_YUV_PLANAR_420,
            1920, 1080, PIXEL_FORMAT_YUV_PLANAR_420,
            1920, 440, PIXEL_FORMAT_YUV_PLANAR_420,
            640, 480, PIXEL_FORMAT_YUV_PLANAR_420);

    vproc_attr[2] = MAPI_VPROC_DefaultAttr_OneChn(
            640, 480, PIXEL_FORMAT_YUV_PLANAR_420,
            320, 180, PIXEL_FORMAT_YUV_PLANAR_420);
    vproc_attr[3] = MAPI_VPROC_DefaultAttr_OneChn(
            640, 480, PIXEL_FORMAT_YUV_PLANAR_420,
            320, 180, PIXEL_FORMAT_YUV_PLANAR_420);
#endif
	vproc_attr[0].attr_inp.u8VpssDev = 1;
	vproc_attr[1].attr_inp.u8VpssDev = 1;
	vproc_attr[2].attr_inp.u8VpssDev = 0;
	vproc_attr[3].attr_inp.u8VpssDev = 0;

    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl[0], VPSS_ONLINE_GRP_0, &vproc_attr[0]));
    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl[1], VPSS_ONLINE_GRP_1, &vproc_attr[1]));

    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl[2], 2, &vproc_attr[2]));
    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl[3], 3, &vproc_attr[3]));

    CHECK_RET_NULL(MAPI_VPROC_BindVproc(vproc_hdl[0], 2, vproc_hdl[2]));
    CHECK_RET_NULL(MAPI_VPROC_BindVproc(vproc_hdl[1], 2, vproc_hdl[3]));

	TEST_GetVprocFrame(vproc_hdl);

    CHECK_RET_NULL(DispOpen(1920, 440, ROTATION_90, true));

    // MAPI_WND_ATTR_T wnd_attr0 = {0, 0, 960, 440, 0};
    // CHECK_RET_NULL(MAPI_DISP_CreateWindow(&wnd_hdl[0], 0, &wnd_attr0));

    // MAPI_WND_ATTR_T wnd_attr1 = {960, 0, 960, 440, 1};
    // CHECK_RET_NULL(MAPI_DISP_CreateWindow(&wnd_hdl[1], 0, &wnd_attr1));
    CHECK_RET_NULL(MAPI_DISP_CreateWindow(0, &wnd_attr[0]));

    CHECK_RET_NULL(MAPI_DISP_CreateWindow(1, &wnd_attr[1]));

     while(!(*Stop))
    {
        int vproc_chn_id = 1;
        VIDEO_FRAME_INFO_S frame_chn[2];
        for (int i = 0; i < 2; i++){
            CHECK_RET_NULL(MAPI_VPROC_GetChnFrame(vproc_hdl[i], vproc_chn_id, &frame_chn[i]));
            CHECK_RET_NULL(MAPI_DISP_SendWndFrame(wnd_hdl[i], &frame_chn[i]));
        }
    }

    for (int i = 0; i < 2; i++)
        CHECK_RET_NULL(MAPI_DISP_DestroyWindow(i, &wnd_attr[i]));
    CHECK_RET_NULL(DispClose(videoOutCfg.dispCfg[0].dispHdl));

    CHECK_RET_NULL(MAPI_VPROC_UnBindVproc(vproc_hdl[1], 2, vproc_hdl[3]));
    CHECK_RET_NULL(MAPI_VPROC_UnBindVproc(vproc_hdl[0], 2, vproc_hdl[2]));

    CHECK_RET_NULL(vcap_sns_deinit());

    for (int i = 3; i >= 0; i--){
    	CHECK_RET_NULL(MAPI_VPROC_Deinit(vproc_hdl[i]));
    }

    TEST_PASS;
    return NULL;
}


static int online_demo_dual_sns_JTD(void)
{
    int bStop = 0;
    pthread_t pthTest;

    if(pthread_create(&pthTest, NULL, online_JTD, (void*)&bStop) < 0){
        printf("pthread_create failed\n");
        return -1;
    }

    if(t_sec > 0)
        sleep(t_sec);
    else
    {
        int op;
        while (1) {
            printf("input 255 to exit, option: ");
            scanf("%d", &op);
            if (op == 255)
                break;
        }
    }
    bStop = 1;
    pthread_join(pthTest, NULL);

    return MAPI_SUCCESS;
}



static void usage(int argc, char *argv[])
{
    printf("Usage: %s <index> time\n", argv[0]);
    printf("index:\n");
    printf("\t 0)vi-vpss online.mode, (JTD dual sensor)\n");
    printf("time: sencond. (-1 loop)\n");
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        usage(argc, argv);
        return 0;
    }
    int op = atoi(argv[1]);
    t_sec = atoi(argv[2]);

    // CVI_LOG_INIT();
    CHECK_RET(online_demo_sys_init());

    switch(op) {
    case 0:
        CHECK_RET(online_demo_dual_sns_JTD());
        break;

    default:
        break;
    }
    CHECK_RET(online_demo_sys_deinit());
    return 0;
}

/* test vo with a yuv image */
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
#include "osal.h"
#include "cvi_log.h"
#include "hal_screen_comp.h"
#include "hal_screen_inner.h"

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

#define TEST_PASS printf("------\nSUCCESS\n------\n")

#define MAX_WIDTH    (1920)
#define MAX_HEIGHT   (1080)
#define MIN_WIDTH   (640)
#define MIN_HEIGHT  (360)
#define DEMO_WIDTH    (1280)
#define DEMO_HEIGHT   (720)
#define DEMO_FMT      (PIXEL_FORMAT_YUV_PLANAR_420)
#define DISP_BUFLEN  (3)

static int t_sec;
static MEDIA_VideoOutCfg videoOutCfg;

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

static int test_disp_vo_rotate(const char *filename) {
    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = DEMO_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = DEMO_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = DEMO_FMT;
    sys_attr.vb_pool[0].vb_blk_num = 18;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = DEMO_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = DEMO_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET(MAPI_Media_Init(&sys_attr));

    CHECK_RET(DispOpen(1280, 720, ROTATION_90, false));

    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            DEMO_WIDTH, DEMO_HEIGHT, DEMO_FMT,
            filename, 0));
    CHECK_RET(MAPI_DISP_SendFrame(videoOutCfg.dispCfg[0].dispHdl, &frame));
    CHECK_RET(MAPI_ReleaseFrame(&frame));

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

    CHECK_RET(DispClose(videoOutCfg.dispCfg[0].dispHdl));
    TEST_PASS;
    return MAPI_SUCCESS;
}


static int test_disp_gdc_rotate(const char *filename) {
    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = DEMO_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = DEMO_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = DEMO_FMT;
    sys_attr.vb_pool[0].vb_blk_num = 18;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = DEMO_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = DEMO_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET(MAPI_Media_Init(&sys_attr));

    CHECK_RET(DispOpen(720, 1280, ROTATION_0, false));

    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            DEMO_WIDTH, DEMO_HEIGHT, DEMO_FMT,
            filename, 0));

    VIDEO_FRAME_INFO_S frame_rot;
    CVI_U64 t0, t1;
    OSAL_TIME_GetBootTimeUs(&t0);
    CHECK_RET(MAPI_IPROC_Rotate(&frame, &frame_rot, ROTATION_90));
    OSAL_TIME_GetBootTimeUs(&t1);
    printf("GDC ROTATE use %.2f ms\n", (t1 - t0) / 1000.0f);
    CHECK_RET(MAPI_ReleaseFrame(&frame));

    CHECK_RET(MAPI_DISP_SendFrame(videoOutCfg.dispCfg[0].dispHdl, &frame_rot));
    CHECK_RET(MAPI_ReleaseFrame(&frame_rot));

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

    CHECK_RET(DispClose(videoOutCfg.dispCfg[0].dispHdl));
    TEST_PASS;

    return MAPI_SUCCESS;
}

static int test_disp_2Chn_from_file(const char *filename1, const char *filename2)
{
    MAPI_VPROC_HANDLE_T vproc_hdl0 = NULL;
    MAPI_VPROC_HANDLE_T vproc_hdl1 = NULL;

    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = MAX_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = MAX_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = DEMO_FMT;
    sys_attr.vb_pool[0].vb_blk_num = 10;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = MAX_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = MAX_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET(MAPI_Media_Init(&sys_attr));

    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            MAX_WIDTH, MAX_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            DEMO_WIDTH, DEMO_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);

    vproc_attr.attr_chn[0].u32Width = DEMO_WIDTH;
    vproc_attr.attr_chn[0].u32Height = DEMO_HEIGHT;
    vproc_attr.attr_chn[0].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
    vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_FALSE;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width = DEMO_WIDTH/2;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = DEMO_HEIGHT;
    CHECK_RET(MAPI_VPROC_Init(&vproc_hdl0, -1, &vproc_attr));

    vproc_attr.attr_chn[0].u32Width = DEMO_WIDTH;
    vproc_attr.attr_chn[0].u32Height = DEMO_HEIGHT;
    vproc_attr.attr_chn[0].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
    vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_FALSE;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X = DEMO_WIDTH/2;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width = DEMO_WIDTH/2;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = DEMO_HEIGHT;
    CHECK_RET(MAPI_VPROC_Init(&vproc_hdl1, -1, &vproc_attr));

    CHECK_RET(DispOpen(1280, 720, ROTATION_90, false));

    VIDEO_FRAME_INFO_S frame0;
    VIDEO_FRAME_INFO_S frame1;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame0,
            MAX_WIDTH, MAX_HEIGHT, DEMO_FMT,
            filename1, 0));
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame1,
            MAX_WIDTH, MAX_HEIGHT, DEMO_FMT,
            filename2, 0));

    CHECK_RET(MAPI_VPROC_SendFrame(vproc_hdl0, &frame0));
    int vproc_chn_id_0 = 0;
    VIDEO_FRAME_INFO_S frame_chn_0;
    CHECK_RET(MAPI_VPROC_GetChnFrame(vproc_hdl0, vproc_chn_id_0, &frame_chn_0));
    CHECK_RET(MAPI_VPROC_SendChnFrame(vproc_hdl1, vproc_chn_id_0, &frame_chn_0));
    CHECK_RET(MAPI_VPROC_ReleaseFrame(vproc_hdl0, vproc_chn_id_0, &frame_chn_0));

    VIDEO_FRAME_INFO_S frame_out;
    CHECK_RET(MAPI_VPROC_SendFrame(vproc_hdl1, &frame1));
    CHECK_RET(MAPI_VPROC_GetChnFrame(vproc_hdl1, vproc_chn_id_0, &frame_out));

    CHECK_RET(MAPI_DISP_SendFrame(videoOutCfg.dispCfg[0].dispHdl, &frame_out));
    CHECK_RET(MAPI_VPROC_ReleaseFrame(vproc_hdl1, vproc_chn_id_0, &frame_out));

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

    CHECK_RET(DispClose(videoOutCfg.dispCfg[0].dispHdl));
    CHECK_RET(MAPI_VPROC_Deinit(vproc_hdl1));
    CHECK_RET(MAPI_VPROC_Deinit(vproc_hdl0));
    TEST_PASS;

    return MAPI_SUCCESS;
}

static void* disp_double_sensor(void* args)
{
    int* Stop = (int*)args;

    MAPI_VPROC_HANDLE_T vproc_hdl0 = NULL;
    MAPI_VPROC_HANDLE_T vproc_hdl1 = NULL;

    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = MAX_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = MAX_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = DEMO_FMT;
    sys_attr.vb_pool[0].vb_blk_num = 18;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = MAX_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = MAX_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET_NULL(MAPI_Media_Init(&sys_attr));

    MAPI_VCAP_SENSOR_HANDLE_T sns[2];
    MAPI_VCAP_ATTR_T vcap_attr;
    CHECK_RET_NULL(MAPI_VCAP_GetGeneralVcapAttr(&vcap_attr, 2));

    for (int i = 0; i < vcap_attr.u8DevNum; i++) {
        CHECK_RET_NULL(MAPI_VCAP_InitSensor(&sns[i], i, &vcap_attr));
        CHECK_RET_NULL(MAPI_VCAP_InitISP(sns[i]));
        // CHECK_RET_NULL(MAPI_VCAP_StartISP(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StartDev(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StartPipe(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StartChn(sns[i]));
    }

    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            MAX_WIDTH, MAX_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            DEMO_WIDTH, DEMO_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);

    vproc_attr.attr_chn[0].u32Width = DEMO_WIDTH;
    vproc_attr.attr_chn[0].u32Height = DEMO_HEIGHT;
    vproc_attr.attr_chn[0].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
    vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_FALSE;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width = DEMO_WIDTH/2;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = DEMO_HEIGHT;
    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl0, -1, &vproc_attr));

    vproc_attr.attr_chn[0].u32Width = DEMO_WIDTH;
    vproc_attr.attr_chn[0].u32Height = DEMO_HEIGHT;
    vproc_attr.attr_chn[0].stAspectRatio.enMode = ASPECT_RATIO_MANUAL;
    vproc_attr.attr_chn[0].stAspectRatio.bEnableBgColor = CVI_FALSE;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32X = DEMO_WIDTH/2;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.s32Y = 0;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Width = DEMO_WIDTH/2;
    vproc_attr.attr_chn[0].stAspectRatio.stVideoRect.u32Height = DEMO_HEIGHT;
    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl1, -1, &vproc_attr));

    CHECK_RET_NULL(DispOpen(1280, 720, ROTATION_90, false));

    VIDEO_FRAME_INFO_S frame0;
    VIDEO_FRAME_INFO_S frame1;

    while(!(*Stop))
    {
        CHECK_RET_NULL(MAPI_VCAP_GetFrame(sns[0], &frame0));
        CHECK_RET_NULL(MAPI_VPROC_SendFrame(vproc_hdl0, &frame0));
        CHECK_RET_NULL(MAPI_VCAP_ReleaseFrame(sns[0], &frame0));
        int vproc_chn_id_0 = 0;
        VIDEO_FRAME_INFO_S frame_chn_0;
        CHECK_RET_NULL(MAPI_VPROC_GetChnFrame(vproc_hdl0, vproc_chn_id_0, &frame_chn_0));
        CHECK_RET_NULL(MAPI_VPROC_SendChnFrame(vproc_hdl1, vproc_chn_id_0, &frame_chn_0));
        CHECK_RET_NULL(MAPI_VPROC_ReleaseFrame(vproc_hdl0, vproc_chn_id_0, &frame_chn_0));

        VIDEO_FRAME_INFO_S frame_out;
        CHECK_RET_NULL(MAPI_VCAP_GetFrame(sns[1], &frame1));
        CHECK_RET_NULL(MAPI_VPROC_SendFrame(vproc_hdl1, &frame1));
        CHECK_RET_NULL(MAPI_VCAP_ReleaseFrame(sns[1], &frame1));
        CHECK_RET_NULL(MAPI_VPROC_GetChnFrame(vproc_hdl1, vproc_chn_id_0, &frame_out));

        CHECK_RET_NULL(MAPI_DISP_SendFrame(videoOutCfg.dispCfg[0].dispHdl, &frame_out));
        CHECK_RET_NULL(MAPI_VPROC_ReleaseFrame(vproc_hdl1, vproc_chn_id_0, &frame_out));
    }

    CHECK_RET_NULL(DispClose(videoOutCfg.dispCfg[0].dispHdl));
    CHECK_RET_NULL(MAPI_VPROC_Deinit(vproc_hdl1));
    CHECK_RET_NULL(MAPI_VPROC_Deinit(vproc_hdl0));

    for (int i = 0; i < 2; i++) {
        CHECK_RET_NULL(MAPI_VCAP_StopChn(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StopDev(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StopPipe(sns[i]));
        // CHECK_RET_NULL(MAPI_VCAP_StopISP(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_DeInitISP(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_DeinitSensor(sns[i]));
    }

    TEST_PASS;
    return NULL;
}

static int test_disp_2Chn_from_sensor(void)
{
    int bStop = 0;
    pthread_t pthTest;

    if(pthread_create(&pthTest, NULL, disp_double_sensor, (void*)&bStop) < 0){
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

MAPI_WND_HANDLE_T wnd_hdl[2];
CVI_STITCH_ATTR_S wnd_attr[2];
static void* disp_window_run(void* args)
{
    int* Stop = (int*)args;

    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = MAX_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = MAX_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = DEMO_FMT;
    sys_attr.vb_pool[0].vb_blk_num = 18;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = MAX_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = MAX_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET_NULL(MAPI_Media_Init(&sys_attr));

    MAPI_VCAP_SENSOR_HANDLE_T sns[2];
    MAPI_VCAP_ATTR_T vcap_attr;
    CHECK_RET_NULL(MAPI_VCAP_GetGeneralVcapAttr(&vcap_attr, 2));

    for (int i = 0; i < vcap_attr.u8DevNum; i++) {
        CHECK_RET_NULL(MAPI_VCAP_InitSensor(&sns[i], i, &vcap_attr));
        CHECK_RET_NULL(MAPI_VCAP_InitISP(sns[i]));
        // CHECK_RET_NULL(MAPI_VCAP_StartISP(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StartDev(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StartPipe(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StartChn(sns[i]));
    }

    MAPI_VPROC_HANDLE_T vproc_hdl[2];

    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            MAX_WIDTH, MAX_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            DEMO_WIDTH, DEMO_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl[0], -1, &vproc_attr));

    vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            MAX_WIDTH, MAX_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            MIN_WIDTH, MIN_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET_NULL(MAPI_VPROC_Init(&vproc_hdl[1], -1, &vproc_attr));

    for (int i = 0; i < 2; i++)
        CHECK_RET_NULL(MAPI_VPROC_BindVcap(vproc_hdl[i], sns[i], i));

    CHECK_RET_NULL(DispOpen(1280, 720, ROTATION_90, true));

    // MAPI_WND_ATTR_T wnd_attr0 = {0, 0, DEMO_WIDTH, DEMO_HEIGHT, 0};
    // CHECK_RET_NULL(MAPI_DISP_CreateWindow(&wnd_hdl[0], 0, &wnd_attr0));

    // MAPI_WND_ATTR_T wnd_attr1 = {0, 0, MIN_WIDTH, MIN_HEIGHT, 1};
    // CHECK_RET_NULL(MAPI_DISP_CreateWindow(&wnd_hdl[1], 0, &wnd_attr1));
    CHECK_RET_NULL(MAPI_DISP_CreateWindow(0, &wnd_attr[0]));

    CHECK_RET_NULL(MAPI_DISP_CreateWindow(1, &wnd_attr[1]));


    while(!(*Stop))
    {
        int vproc_chn_id_0 = 0;
        VIDEO_FRAME_INFO_S frame_chn[2];
        for (int i = 0; i < 2; i++){
            CHECK_RET_NULL(MAPI_VPROC_GetChnFrame(vproc_hdl[i], vproc_chn_id_0, &frame_chn[i]));
            CHECK_RET_NULL(MAPI_DISP_SendWndFrame(wnd_hdl[i], &frame_chn[i]));
            //CHECK_RET_NULL(MAPI_VPROC_ReleaseFrame(vproc_hdl[i], vproc_chn_id_0, &frame_chn[i]));
        }
    }

    for (int i = 0; i < 2; i++)
        CHECK_RET_NULL(MAPI_DISP_DestroyWindow(i, &wnd_attr[i]));
    CHECK_RET_NULL(DispClose(videoOutCfg.dispCfg[0].dispHdl));
    for (int i = 0; i < 2; i++) {
        CHECK_RET_NULL(MAPI_VPROC_UnBindVcap(vproc_hdl[i], sns[i], i));
        CHECK_RET_NULL(MAPI_VPROC_Deinit(vproc_hdl[i]));
        CHECK_RET_NULL(MAPI_VCAP_StopChn(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StopDev(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_StopPipe(sns[i]));
        // CHECK_RET_NULL(MAPI_VCAP_StopISP(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_DeInitISP(sns[i]));
        CHECK_RET_NULL(MAPI_VCAP_DeinitSensor(sns[i]));
    }
    TEST_PASS;
    return NULL;
}

static void* change_window_pos_size(void* args)
{
    int* Stop = (int*)args;

    while(!wnd_hdl[1])
        sleep(1);

    while(!(*Stop)){
        MAPI_WND_ATTR_T stWndAttr;
        CHECK_RET_NULL(MAPI_DISP_GetWndAttr(wnd_hdl[1], &stWndAttr));
        if (stWndAttr.wnd_w >= (DEMO_WIDTH - 32))
            stWndAttr.wnd_w = MIN_WIDTH;
        else
            stWndAttr.wnd_w += 32;

        if (stWndAttr.wnd_h >= (DEMO_HEIGHT - 32))
            stWndAttr.wnd_h = MIN_HEIGHT;
        else
            stWndAttr.wnd_h += 32;

        if(stWndAttr.wnd_x >= (DEMO_WIDTH - stWndAttr.wnd_w))
            stWndAttr.wnd_x = 0;
        else
            stWndAttr.wnd_x += 2;
        if(stWndAttr.wnd_y >= (DEMO_HEIGHT - stWndAttr.wnd_h))
            stWndAttr.wnd_y = 0;
        else
            stWndAttr.wnd_y += 2;
        CHECK_RET_NULL(MAPI_DISP_SetWndAttr(wnd_hdl[1], &stWndAttr));
        sleep(1);
    }
    TEST_PASS;
    return NULL;
}

static int test_disp_window_change_pos_size(void)
{
    int bStop = 0;
    pthread_t pthTest;
    pthread_t pthChangeAttr;

    if(pthread_create(&pthTest, NULL, disp_window_run, (void*)&bStop) < 0){
        printf("pthread_create failed\n");
        return -1;
    }
    if(pthread_create(&pthChangeAttr, NULL, change_window_pos_size, (void*)&bStop) < 0){
        printf("pthread_create failed\n");
        bStop = 1;
        pthread_join(pthTest, NULL);
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
    pthread_join(pthChangeAttr, NULL);

    return MAPI_SUCCESS;
}

static void* switch_window(void* args)
{
    int* Stop = (int*)args;
    while(!wnd_hdl[1])
        sleep(1);

    while(!(*Stop)){
        MAPI_WND_ATTR_T stWndAttr0, stWndAttr1;
        CHECK_RET_NULL(MAPI_DISP_GetWndAttr(wnd_hdl[0], &stWndAttr0));
        CHECK_RET_NULL(MAPI_DISP_GetWndAttr(wnd_hdl[1], &stWndAttr1));
        CHECK_RET_NULL(MAPI_DISP_SetWndAttr(wnd_hdl[1], &stWndAttr0));
        CHECK_RET_NULL(MAPI_DISP_SetWndAttr(wnd_hdl[0], &stWndAttr1));
        sleep(3);
    }
    TEST_PASS;
    return NULL;
}

static int test_disp_switch_window(void)
{
    int bStop = 0;
    pthread_t pthTest;
    pthread_t pthChangeAttr;

    if(pthread_create(&pthTest, NULL, disp_window_run, (void*)&bStop) < 0){
        printf("pthread_create failed\n");
        return -1;
    }
    if(pthread_create(&pthChangeAttr, NULL, switch_window, (void*)&bStop) < 0){
        printf("pthread_create failed\n");
        bStop = 1;
        pthread_join(pthTest, NULL);
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
    pthread_join(pthChangeAttr, NULL);

    return MAPI_SUCCESS;
}


static void usage(int argc, char *argv[])
{
    printf("Usage: %s op time [inputfile1] [inputfile2]\n", argv[0]);
    printf("  op = 0, disp rotate. (1280x720)\n");
    printf("  op = 1, iproc rotate. (1280x720)\n");
    printf("  op = 2, Show two channels of video. (file),(1920x1080)\n");
    printf("  op = 3, Show two channels of video. (sensor)\n");
    printf("  op = 4, two window, change position and size\n");
    printf("  op = 5, two window, switch window\n");
    printf("  time: sencond. (-1 loop)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        usage(argc, argv);
        exit(-1);
    }

    int op = atoi(argv[1]);
    t_sec = atoi(argv[2]);

    switch(op) {
    case 0:
        CHECK_RET(test_disp_vo_rotate(argv[3]));
        break;
    case 1:
        CHECK_RET(test_disp_gdc_rotate(argv[3]));
        break;
    case 2:
        CHECK_RET(test_disp_2Chn_from_file(argv[3], argv[4]));
        break;
    case 3:
        CHECK_RET(test_disp_2Chn_from_sensor());
        break;
    case 4:
        CHECK_RET(test_disp_window_change_pos_size());
        break;
    case 5:
        CHECK_RET(test_disp_switch_window());
        break;
    default:
        return -1;
    }
    return 0;
}


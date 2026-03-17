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

#define TEST_PASS printf("------\nSUCCESS\n------\n")

#define IMAGE_WIDTH   (1920)
#define IMAGE_HEIGHT  (1080)
#define INAGE_FMT     PIXEL_FORMAT_YUV_PLANAR_420
#define CHN_0_WIDTH   (1280)
#define CHN_0_HEIGHT  (720)
#define CHN_1_WIDTH   (640)
#define CHN_1_HEIGHT  (480)

int test_vproc_basic(const char *inputfile, const char *outputfile_base)
{
    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            inputfile, 0));

    // start test
    MAPI_VPROC_HANDLE_T vproc;
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_TwoChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_RGB_888_PLANAR,
            CHN_1_WIDTH, CHN_1_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(MAPI_VPROC_Init(&vproc, -1, &vproc_attr));

    CHECK_RET(MAPI_VPROC_SendFrame(vproc, &frame));
    CHECK_RET(MAPI_ReleaseFrame(&frame));

    char filename[128];
    int vproc_chn_id_0 = 0;
    VIDEO_FRAME_INFO_S frame_chn_0;
    CHECK_RET(MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_0,
                                         &frame_chn_0));
    snprintf(filename, 128, "%s_chn_%d", outputfile_base, 0);
    CHECK_RET(MAPI_SaveFramePixelData(&frame_chn_0, filename));
    CHECK_RET(MAPI_ReleaseFrame(&frame_chn_0));

    int vproc_chn_id_1 = 1;
    VIDEO_FRAME_INFO_S frame_chn_1;
    CHECK_RET(MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_1,
                                         &frame_chn_1));
    snprintf(filename, 128, "%s_chn_%d", outputfile_base, 1);
    CHECK_RET(MAPI_SaveFramePixelData(&frame_chn_1, filename));
    CHECK_RET(MAPI_ReleaseFrame(&frame_chn_1));

    CHECK_RET(MAPI_VPROC_Deinit(vproc));
    TEST_PASS;
    return 0;
}

int test_vproc_perf_1(const char *inputfile, int count)
{
    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            inputfile, 0));

    // start test
    MAPI_VPROC_HANDLE_T vproc;
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_TwoChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_RGB_888_PLANAR,
            CHN_1_WIDTH, CHN_1_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(MAPI_VPROC_Init(&vproc, -1, &vproc_attr));

    // loop
    CVI_U64 t0, t1;
    OSAL_TIME_GetBootTimeUs(&t0);
    for (int i = 0; i < count; i++) {
        CHECK_RET(MAPI_VPROC_SendFrame(vproc, &frame));

        int vproc_chn_id_0 = 0;
        VIDEO_FRAME_INFO_S frame_chn_0;
        CHECK_RET(MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_0,
                                             &frame_chn_0));
        CHECK_RET(MAPI_ReleaseFrame(&frame_chn_0));

        int vproc_chn_id_1 = 1;
        VIDEO_FRAME_INFO_S frame_chn_1;
        CHECK_RET(MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_1,
                                             &frame_chn_1));
        CHECK_RET(MAPI_ReleaseFrame(&frame_chn_1));
    }
    OSAL_TIME_GetBootTimeUs(&t1);
    printf("Perf_vproc_1: %d loop, total %.2f ms, %.2f ms per frame\n",
        count, (t1-t0) / 1000.0f, (t1-t0) / 1000.0f / count);

    // cleanup
    CHECK_RET(MAPI_ReleaseFrame(&frame));

    CHECK_RET(MAPI_VPROC_Deinit(vproc));
    TEST_PASS;
    return 0;
}

int test_vproc_perf_2(const char *inputfile, int count)
{
    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            inputfile, 0));

    // start test
    MAPI_VPROC_HANDLE_T vproc;
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(MAPI_VPROC_Init(&vproc, -1, &vproc_attr));

    // loop
    CVI_U64 t0, t1;
    OSAL_TIME_GetBootTimeUs(&t0);
    for (int i = 0; i < count; i++) {
        CHECK_RET(MAPI_VPROC_SendFrame(vproc, &frame));

        int vproc_chn_id_0 = 0;
        VIDEO_FRAME_INFO_S frame_chn_0;
        CHECK_RET(MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_0,
                                             &frame_chn_0));
        CHECK_RET(MAPI_ReleaseFrame(&frame_chn_0));
    }
    OSAL_TIME_GetBootTimeUs(&t1);
    printf("Perf_vproc_2: %d loop, total %.2f ms, %.2f ms per frame\n",
        count, (t1-t0) / 1000.0f, (t1-t0) / 1000.0f / count);

    // cleanup
    CHECK_RET(MAPI_ReleaseFrame(&frame));

    CHECK_RET(MAPI_VPROC_Deinit(vproc));
    TEST_PASS;
    return 0;
}

int test_iproc_perf(const char *inputfile, int count)
{
    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            inputfile, 0));

    // loop
    CVI_U64 t0, t1;
    OSAL_TIME_GetBootTimeUs(&t0);
    for (int i = 0; i < count; i++) {
        VIDEO_FRAME_INFO_S frame_resized;
        CHECK_RET(MAPI_IPROC_Resize(&frame,
            &frame_resized,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            false, NULL, NULL, NULL));
        CHECK_RET(MAPI_ReleaseFrame(&frame_resized));
    }
    OSAL_TIME_GetBootTimeUs(&t1);
    printf("Perf_iproc: %d loop, total %.2f ms, %.2f ms per frame\n",
        count, (t1-t0) / 1000.0f, (t1-t0) / 1000.0f / count);

    // cleanup
    CHECK_RET(MAPI_ReleaseFrame(&frame));
    TEST_PASS;
    return 0;
}

int test_rotate_perf(const char *inputfile, int count)
{
    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            inputfile, 0));

    // loop
    CVI_U64 t0, t1;
    OSAL_TIME_GetBootTimeUs(&t0);
    for (int i = 0; i < count; i++) {
        VIDEO_FRAME_INFO_S frame_rot;
        CHECK_RET(MAPI_IPROC_Rotate(&frame,
            &frame_rot, ROTATION_90));
        CHECK_RET(MAPI_ReleaseFrame(&frame_rot));
    }
    OSAL_TIME_GetBootTimeUs(&t1);
    printf("Perf_rotate: %d loop, total %.2f ms, %.2f ms per frame\n",
        count, (t1-t0) / 1000.0f, (t1-t0) / 1000.0f / count);

    // cleanup
    CHECK_RET(MAPI_ReleaseFrame(&frame));
    TEST_PASS;
    return 0;
}

struct task_attr_s {
    int id;
    char inputfile[128];
    int count;
};

static void * worker(void *arg)
{
    struct task_attr_s *attr = (struct task_attr_s *)arg;

    switch(attr->id) {
    case 0:
        test_vproc_perf_1(attr->inputfile, attr->count);
        break;
    case 1:
        test_vproc_perf_2(attr->inputfile, attr->count);
        break;
    case 2:
        test_iproc_perf(attr->inputfile, attr->count);
        break;
    case 3:
        test_rotate_perf(attr->inputfile, attr->count);
        break;
    default:
        printf("invalid task_id %d\n", attr->id);
        break;
    }
    return NULL;
}

int test_vproc_perf_both(const char *inputfile, int count)
{
    pthread_t task0, task1;
    struct task_attr_s task0_attr, task1_attr;

    task0_attr.id = 0;
    strcpy(task0_attr.inputfile, inputfile);
    task0_attr.count = count;

    task1_attr.id = 1;
    strcpy(task1_attr.inputfile, inputfile);
    task1_attr.count = count;

    pthread_create(&task0, 0, worker, &task0_attr);
    pthread_create(&task1, 0, worker, &task1_attr);

    pthread_join(task1, 0);
    pthread_join(task0, 0);

    return 0;
}

int test_vproc_and_iproc_perf(const char *inputfile, int count)
{
    pthread_t task0, task1;
    struct task_attr_s task0_attr, task1_attr;

    task0_attr.id = 0;
    strcpy(task0_attr.inputfile, inputfile);
    task0_attr.count = count;

    task1_attr.id = 2;
    strcpy(task1_attr.inputfile, inputfile);
    task1_attr.count = count;

    pthread_create(&task0, 0, worker, &task0_attr);
    pthread_create(&task1, 0, worker, &task1_attr);

    pthread_join(task1, 0);
    pthread_join(task0, 0);

    return 0;
}

int test_vproc_x2_and_iproc_perf(const char *inputfile, int count)
{
    pthread_t task0, task1, task2;
    struct task_attr_s task0_attr, task1_attr, task2_attr;

    task0_attr.id = 0;
    strcpy(task0_attr.inputfile, inputfile);
    task0_attr.count = count;

    task1_attr.id = 1;
    strcpy(task1_attr.inputfile, inputfile);
    task1_attr.count = count;

    task2_attr.id = 2;
    strcpy(task2_attr.inputfile, inputfile);
    task2_attr.count = count;

    pthread_create(&task0, 0, worker, &task0_attr);
    pthread_create(&task1, 0, worker, &task1_attr);
    pthread_create(&task2, 0, worker, &task2_attr);

    pthread_join(task2, 0);
    pthread_join(task1, 0);
    pthread_join(task0, 0);

    return 0;
}

int test_vproc_and_rotate_perf(const char *inputfile, int count)
{
    pthread_t task0, task1;
    struct task_attr_s task0_attr, task1_attr;

    task0_attr.id = 0;
    strcpy(task0_attr.inputfile, inputfile);
    task0_attr.count = count;

    task1_attr.id = 3;
    strcpy(task1_attr.inputfile, inputfile);
    task1_attr.count = count;

    pthread_create(&task0, 0, worker, &task0_attr);
    pthread_create(&task1, 0, worker, &task1_attr);

    pthread_join(task1, 0);
    pthread_join(task0, 0);

    return 0;
}

int test_iproc_and_rotate_perf(const char *inputfile, int count)
{
    pthread_t task0, task1;
    struct task_attr_s task0_attr, task1_attr;

    task0_attr.id = 2;
    strcpy(task0_attr.inputfile, inputfile);
    task0_attr.count = count;

    task1_attr.id = 3;
    strcpy(task1_attr.inputfile, inputfile);
    task1_attr.count = count;

    pthread_create(&task0, 0, worker, &task0_attr);
    pthread_create(&task1, 0, worker, &task1_attr);

    pthread_join(task1, 0);
    pthread_join(task0, 0);

    return 0;
}

int test_vproc_x2_and_iproc_and_rotate_perf(const char *inputfile, int count)
{
    pthread_t task0, task1, task2, task3;
    struct task_attr_s task0_attr, task1_attr,
                       task2_attr, task3_attr;

    task0_attr.id = 0;
    strcpy(task0_attr.inputfile, inputfile);
    task0_attr.count = count;

    task1_attr.id = 1;
    strcpy(task1_attr.inputfile, inputfile);
    task1_attr.count = count;

    task2_attr.id = 2;
    strcpy(task2_attr.inputfile, inputfile);
    task2_attr.count = count;

    task3_attr.id = 3;
    strcpy(task3_attr.inputfile, inputfile);
    task3_attr.count = count;

    pthread_create(&task0, 0, worker, &task0_attr);
    pthread_create(&task1, 0, worker, &task1_attr);
    pthread_create(&task2, 0, worker, &task2_attr);
    pthread_create(&task3, 0, worker, &task3_attr);

    pthread_join(task3, 0);
    pthread_join(task2, 0);
    pthread_join(task1, 0);
    pthread_join(task0, 0);

    return 0;
}

int save_frame_file(uint32_t Grp, uint32_t Chn, VIDEO_FRAME_INFO_S *pFrame, void *pPrivateData)
{
    char filename[128];
    snprintf(filename, 128, "/mnt/data/%s_grp%d_chn%d", (char*)pPrivateData, Grp, Chn);
    CHECK_RET(MAPI_SaveFramePixelData(pFrame, filename));
    TEST_PASS;
    return 0;
}

int test_vproc_dump_frame()
{
    MAPI_VCAP_ATTR_T vcap_attr;
    uint8_t sns_num = vcap_attr.u8DevNum;
    CHECK_RET(MAPI_VCAP_GetGeneralVcapAttr(&vcap_attr, sns_num));

    MAPI_VCAP_SENSOR_HANDLE_T sns_0;
    int sns_id = 0;
    CHECK_RET(MAPI_VCAP_InitSensor(&sns_0, sns_id, &vcap_attr));
    CHECK_RET(MAPI_VCAP_InitISP(sns_0));
    // CHECK_RET(MAPI_VCAP_StartISP(sns_0));
    CHECK_RET(MAPI_VCAP_StartDev(sns_0));
    CHECK_RET(MAPI_VCAP_StartPipe(sns_0));
    CHECK_RET(MAPI_VCAP_StartChn(sns_0));

    MAPI_VCAP_SENSOR_HANDLE_T sns_1;
    int sns_id_1 = 1;
    CHECK_RET(MAPI_VCAP_InitSensor(&sns_1, sns_id_1, &vcap_attr));
    CHECK_RET(MAPI_VCAP_InitISP(sns_1));
    // CHECK_RET(MAPI_VCAP_StartISP(sns_1));
    CHECK_RET(MAPI_VCAP_StartDev(sns_1));
    CHECK_RET(MAPI_VCAP_StartPipe(sns_1));
    CHECK_RET(MAPI_VCAP_StartChn(sns_1));

    MAPI_VPROC_HANDLE_T vproc_hdl;
    uint32_t chn_idx = 0;
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(MAPI_VPROC_Init(&vproc_hdl, -1, &vproc_attr));
    CHECK_RET(MAPI_VPROC_BindVcap(vproc_hdl, sns_0, sns_id));

    sleep(3);
    MAPI_DUMP_FRAME_CALLBACK_FUNC_T stCallbackFun;
    stCallbackFun.pPrivateData = (CVI_VOID*)__func__;
    stCallbackFun.pfunFrameProc = save_frame_file;
    CHECK_RET(MAPI_VPROC_StartChnDump(vproc_hdl, chn_idx, 1, &stCallbackFun));
    CHECK_RET(MAPI_VPROC_StopChnDump(vproc_hdl, chn_idx));

    CHECK_RET(MAPI_VPROC_UnBindVcap(vproc_hdl, sns_0, sns_id));
    CHECK_RET(MAPI_VPROC_Deinit(vproc_hdl));
    CHECK_RET(MAPI_VCAP_StopChn(sns_0));
    CHECK_RET(MAPI_VCAP_StopDev(sns_0));
    CHECK_RET(MAPI_VCAP_StopPipe(sns_0));
    // CHECK_RET(MAPI_VCAP_StopISP(sns_0));
    CHECK_RET(MAPI_VCAP_DeInitISP(sns_0));
    CHECK_RET(MAPI_VCAP_DeinitSensor(sns_0));

    CHECK_RET(MAPI_VCAP_StopChn(sns_1));
    CHECK_RET(MAPI_VCAP_StopDev(sns_1));
    CHECK_RET(MAPI_VCAP_StopPipe(sns_1));
    // CHECK_RET(MAPI_VCAP_StopISP(sns_1));
    CHECK_RET(MAPI_VCAP_DeInitISP(sns_1));
    CHECK_RET(MAPI_VCAP_DeinitSensor(sns_1));

    return 0;
}

static MEDIA_VideoOutCfg videoOutCfg;

static int DispOpen(CVI_U32 width, CVI_U32 height, ROTATION_E rotate)
{
    int ret = MAPI_SUCCESS;

    MEDIA_DispCfg *dispCfg = &videoOutCfg.dispCfg[0];

    dispCfg->dispAttr.width = width;
    dispCfg->dispAttr.height = height;
    dispCfg->dispAttr.rotate = rotate;
    dispCfg->dispAttr.window_mode = false;
    dispCfg->dispAttr.stPubAttr.u32BgColor = COLOR_10_RGB_BLUE;
    dispCfg->dispAttr.stPubAttr.enIntfSync = VO_OUTPUT_USER;
    dispCfg->videoLayerAttr.u32BufLen = 3;
    dispCfg->videoLayerAttr.u32PixelFmt = INAGE_FMT;

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

int test_vproc_crop_rotate()
{
    MAPI_VCAP_ATTR_T vcap_attr;
    uint8_t sns_num = vcap_attr.u8DevNum;
    CHECK_RET(MAPI_VCAP_GetGeneralVcapAttr(&vcap_attr, sns_num));

    MAPI_VCAP_SENSOR_HANDLE_T sns_0;
    int sns_id = 0;
    CHECK_RET(MAPI_VCAP_InitSensor(&sns_0, sns_id, &vcap_attr));
    CHECK_RET(MAPI_VCAP_InitISP(sns_0));
    // CHECK_RET(MAPI_VCAP_StartISP(sns_0));
    CHECK_RET(MAPI_VCAP_StartDev(sns_0));
    CHECK_RET(MAPI_VCAP_StartPipe(sns_0));
    CHECK_RET(MAPI_VCAP_StartChn(sns_0));

    MAPI_VCAP_SENSOR_HANDLE_T sns_1;
    int sns_id_1 = 1;
    CHECK_RET(MAPI_VCAP_InitSensor(&sns_1, sns_id_1, &vcap_attr));
    CHECK_RET(MAPI_VCAP_InitISP(sns_1));
    // CHECK_RET(MAPI_VCAP_StartISP(sns_1));
    CHECK_RET(MAPI_VCAP_StartDev(sns_1));
    CHECK_RET(MAPI_VCAP_StartPipe(sns_1));
    CHECK_RET(MAPI_VCAP_StartChn(sns_1));

    MAPI_VPROC_HANDLE_T vproc_hdl;
    uint32_t chn_idx = 0;
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_OneChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(MAPI_VPROC_Init(&vproc_hdl, -1, &vproc_attr));

    CHECK_RET(DispOpen(CHN_0_HEIGHT, CHN_0_WIDTH, ROTATION_0));

    VPSS_CROP_INFO_S stVpssCrop;
    stVpssCrop.bEnable = CVI_TRUE;
    stVpssCrop.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stVpssCrop.stCropRect.s32X = 0;
    stVpssCrop.stCropRect.s32Y = 0;
    stVpssCrop.stCropRect.u32Width = CHN_0_WIDTH;
    stVpssCrop.stCropRect.u32Height = CHN_0_HEIGHT;
    CHECK_RET(MAPI_VPROC_SetChnAttrEx(vproc_hdl, chn_idx, MAPI_VPROC_CMD_CHN_CROP,
        (CVI_VOID*)&stVpssCrop, sizeof(VPSS_CROP_INFO_S)));

    ROTATION_E enRotation = ROTATION_90;
    CHECK_RET(MAPI_VPROC_SetChnAttrEx(vproc_hdl, chn_idx, MAPI_VPROC_CMD_CHN_ROTATE,
        (CVI_VOID*)&enRotation, sizeof(ROTATION_E)));

    CHECK_RET(MAPI_VPROC_BindVcap(vproc_hdl, sns_0, sns_id));
    CHECK_RET(MAPI_DISP_BindVproc(videoOutCfg.dispCfg[0].dispHdl, vproc_hdl, chn_idx));
    sleep(10);

    CHECK_RET(MAPI_DISP_UnBindVproc(videoOutCfg.dispCfg[0].dispHdl, vproc_hdl, chn_idx));
    CHECK_RET(MAPI_VPROC_UnBindVcap(vproc_hdl, sns_0, sns_id));
    CHECK_RET(DispClose(videoOutCfg.dispCfg[0].dispHdl));
    CHECK_RET(MAPI_VPROC_Deinit(vproc_hdl));
    CHECK_RET(MAPI_VCAP_StopChn(sns_0));
    CHECK_RET(MAPI_VCAP_StopDev(sns_0));
    CHECK_RET(MAPI_VCAP_StopPipe(sns_0));
    // CHECK_RET(MAPI_VCAP_StopISP(sns_0));
    CHECK_RET(MAPI_VCAP_DeInitISP(sns_0));
    CHECK_RET(MAPI_VCAP_DeinitSensor(sns_0));
    CHECK_RET(MAPI_VCAP_StopChn(sns_1));
    CHECK_RET(MAPI_VCAP_StopDev(sns_1));
    CHECK_RET(MAPI_VCAP_StopPipe(sns_1));
    // CHECK_RET(MAPI_VCAP_StopISP(sns_1));
    CHECK_RET(MAPI_VCAP_DeInitISP(sns_1));
    CHECK_RET(MAPI_VCAP_DeinitSensor(sns_1));
    TEST_PASS;
    return 0;
}

static void usage(int argc, char *argv[])
{
    printf("Usage: %s op [inputfile] [outputfile_base]\n", argv[0]);
    printf("  op = 0, basic\n");
    printf("  op = 1, perf vproc 1 grp\n");
    printf("  op = 2, perf vproc 2 grp\n");
    printf("  op = 3, perf iproc\n");
    printf("  op = 4, perf vproc + iproc\n");
    printf("  op = 5, perf vproc * 2 + iproc\n");
    printf("  op = 6, perf gdc\n");
    printf("  op = 7, perf vproc + gdc\n");
    printf("  op = 8, perf iproc + gdc\n");
    printf("  op = 9, perf vproc * 2 + iproc + gdc\n");
    printf("  op = 10,vproc dump frame\n");
    printf("  op = 11,vproc crop and rotation\n");
}

int main(int argc, char *argv[])
{

    if (argc != 4 && argc != 3 && argc != 2) {
        usage(argc, argv);
        exit(-1);
    }
    int op = atoi(argv[1]);
    const char* inputfile = NULL;
    const char* outputfile_base = NULL;
    if (argc >= 3) {
        inputfile = argv[2];
    }
    if (argc >= 4) {
        outputfile_base = argv[3];
    }

    // CVI_LOG_INIT();

    MAPI_MEDIA_SYS_ATTR_T sys_attr;
    sys_attr.vb_pool[0].is_frame = true;
    sys_attr.vb_pool[0].vb_blk_size.frame.width  = IMAGE_WIDTH;
    sys_attr.vb_pool[0].vb_blk_size.frame.height = IMAGE_HEIGHT;
    sys_attr.vb_pool[0].vb_blk_size.frame.fmt    = PIXEL_FORMAT_YUV_PLANAR_420;
    sys_attr.vb_pool[0].vb_blk_num = 16;
    sys_attr.vb_pool[1].is_frame = true;
    sys_attr.vb_pool[1].vb_blk_size.frame.width  = IMAGE_WIDTH;
    sys_attr.vb_pool[1].vb_blk_size.frame.height = IMAGE_HEIGHT;
    sys_attr.vb_pool[1].vb_blk_size.frame.fmt    = PIXEL_FORMAT_RGB_888_PLANAR;
    sys_attr.vb_pool[1].vb_blk_num = 4;
    sys_attr.vb_pool_num = 2;
    CHECK_RET(MAPI_Media_Init(&sys_attr));

    switch(op) {
    case 0:
        CHECK_RET(test_vproc_basic(inputfile, outputfile_base));
        break;
    case 1:
        CHECK_RET(test_vproc_perf_1(inputfile, 1000));
        CHECK_RET(test_vproc_perf_2(inputfile, 1000));
        break;
    case 2:
        CHECK_RET(test_vproc_perf_both(inputfile, 1000));
        break;
    case 3:
        CHECK_RET(test_iproc_perf(inputfile, 1000));
        break;
    case 4:
        CHECK_RET(test_vproc_and_iproc_perf(inputfile, 1000));
        break;
    case 5:
        CHECK_RET(test_vproc_x2_and_iproc_perf(inputfile, 1000));
        break;
    case 6:
        CHECK_RET(test_rotate_perf(inputfile, 1000));
        break;
    case 7:
        CHECK_RET(test_vproc_and_rotate_perf(inputfile, 1000));
        break;
    case 8:
        CHECK_RET(test_iproc_and_rotate_perf(inputfile, 1000));
        break;
    case 9:
        CHECK_RET(test_vproc_x2_and_iproc_and_rotate_perf(inputfile, 1000));
        break;
    case 10:
        CHECK_RET(test_vproc_dump_frame());
        break;
    case 11:
        CHECK_RET(test_vproc_crop_rotate());
        break;
    default:
        return -1;
    }

    CHECK_RET(MAPI_Media_Deinit());

    return 0;
}

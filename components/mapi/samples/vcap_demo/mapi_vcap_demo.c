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

#define TEST_PASS printf("%s\n------\nSUCCESS\n------\n", __FUNCTION__)

MAPI_VCAP_ATTR_T g_vcap_attr;
MAPI_VCAP_SENSOR_HANDLE_T g_sns_hdl[VI_MAX_DEV_NUM];

static int vcap_demo_sys_init(void)
{
    uint32_t i;
    MAPI_MEDIA_SYS_ATTR_T sys_attr;

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

    sys_attr.vb_pool_num = g_vcap_attr.u8DevNum;
    for (i = 0; i < sys_attr.vb_pool_num; i++) {
        sys_attr.vb_pool[i].is_frame = true;
        sys_attr.vb_pool[i].vb_blk_size.frame.width  = g_vcap_attr.attr_chn[i].u32Width;
        sys_attr.vb_pool[i].vb_blk_size.frame.height = g_vcap_attr.attr_chn[i].u32Height;
        sys_attr.vb_pool[i].vb_blk_size.frame.fmt    = g_vcap_attr.attr_chn[i].enPixelFmt;
        sys_attr.vb_pool[i].vb_blk_num = 5;
    }

    CHECK_RET(MAPI_Media_Init(&sys_attr));

    return 0;
}

static int vcap_demo_sys_deinit(void)
{
    CHECK_RET(MAPI_Media_Deinit());
    return 0;
}

static int vcap_demo_sns_init(void)
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
    //waiting for isp stability
    usleep(2000 * 1000);
    return 0;
}

static int vcap_demo_sns_deinit(void)
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
        g_sns_hdl[i] = NULL;
    }
    return 0;
}

static int vcap_demo_dump_yuv(int sns_id)
{
    VIDEO_FRAME_INFO_S vcap_frame;
    char filename[128];
    CHECK_RET(MAPI_VCAP_GetFrame(g_sns_hdl[sns_id], &vcap_frame));
    snprintf(filename, 128, "vcap_demo_out_%d", sns_id);
    CHECK_RET(MAPI_SaveFramePixelData(&vcap_frame, filename));
    CHECK_RET(MAPI_ReleaseFrame(&vcap_frame));
    printf("sns[%d] dump yuv frame pass\n", sns_id);

    return 0;
}

static int vcap_demo_set_crop(int sns_id, int32_t s32X, int32_t s32Y, uint32_t u32Width, uint32_t u32Height)
{
    VI_CROP_INFO_S astCropInfo[2];
    memset(&astCropInfo, 0, sizeof(VI_CROP_INFO_S) * 2);

    astCropInfo[0].bEnable = CVI_TRUE;
    astCropInfo[0].stCropRect.s32X = s32X;
    astCropInfo[0].stCropRect.s32Y = s32Y;
    astCropInfo[0].stCropRect.u32Width = u32Width;
    astCropInfo[0].stCropRect.u32Height = u32Height;
    CHECK_RET(MAPI_VCAP_SetChnCropAttr(g_sns_hdl[sns_id], &astCropInfo[0]));

    CHECK_RET(MAPI_VCAP_GetChnCropAttr(g_sns_hdl[sns_id], &astCropInfo[1]));
    if ((astCropInfo[0].bEnable != astCropInfo[1].bEnable) ||
        (astCropInfo[0].stCropRect.s32X != astCropInfo[1].stCropRect.s32X) ||
        (astCropInfo[0].stCropRect.s32Y != astCropInfo[1].stCropRect.s32Y) ||
        (astCropInfo[0].stCropRect.u32Width != astCropInfo[1].stCropRect.u32Width) ||
        (astCropInfo[0].stCropRect.u32Height != astCropInfo[1].stCropRect.u32Height)) {
        printf("Incorrect crop attr for sns[%d]\n", sns_id);
        return -1;
    }

    return 0;
}

static int vcap_demo_set_rotation(int sns_id, ROTATION_E enRotation)
{
    ROTATION_E aenRotation[2] = {ROTATION_MAX, ROTATION_MAX};

    aenRotation[0] = enRotation;

    CHECK_RET(MAPI_VCAP_SetAttrEx(g_sns_hdl[sns_id], MAPI_VCAP_CMD_Rotate,
                                      &aenRotation[0], sizeof(ROTATION_E)));

    CHECK_RET(MAPI_VCAP_GetAttrEx(g_sns_hdl[sns_id], MAPI_VCAP_CMD_Rotate,
                                      &aenRotation[1], sizeof(ROTATION_E)));

    if (aenRotation[0] != aenRotation[1]) {
        printf("Incorrect rotation for sns[%d]\n", sns_id);
        return -1;
    }

    return 0;
}

static int vcap_demo_set_mirrorflip(int sns_id, bool mirror, bool flip)
{
    MAPI_VCAP_MIRRORFLIP_ATTR_S aenMirrorFlip[2] = {0};

    aenMirrorFlip[0].bMirror = mirror;
    aenMirrorFlip[0].bFlip = flip;

    CHECK_RET(MAPI_VCAP_SetAttrEx(g_sns_hdl[sns_id], MAPI_VCAP_CMD_MirrorFlip,
                                      &aenMirrorFlip[0], sizeof(MAPI_VCAP_MIRRORFLIP_ATTR_S)));

    CHECK_RET(MAPI_VCAP_GetAttrEx(g_sns_hdl[sns_id], MAPI_VCAP_CMD_MirrorFlip,
                                      &aenMirrorFlip[1], sizeof(MAPI_VCAP_MIRRORFLIP_ATTR_S)));

    if ((aenMirrorFlip[0].bMirror != aenMirrorFlip[1].bMirror)
        || (aenMirrorFlip[0].bFlip != aenMirrorFlip[1].bFlip)) {
        printf("Incorrect mirrorflip for sns[%d]\n", sns_id);
        return -1;
    }

    return 0;
}

static int vcap_demo_set_fps(int sns_id, float fps)
{
    CVI_FLOAT af32FrameRate[2] = {0};

    af32FrameRate[0] = fps;

    CHECK_RET(MAPI_VCAP_SetAttrEx(g_sns_hdl[sns_id], MAPI_VCAP_CMD_Fps,
                                      &af32FrameRate[0], sizeof(CVI_FLOAT)));

    CHECK_RET(MAPI_VCAP_GetAttrEx(g_sns_hdl[sns_id], MAPI_VCAP_CMD_Fps,
                                      &af32FrameRate[1], sizeof(CVI_FLOAT)));

    if (fabs(af32FrameRate[0] - af32FrameRate[1]) > (1e-6)) {
        printf("Incorrect set fps for sns[%d]\n", sns_id);
        return -1;
    }

    return 0;
}

static int vcap_demo_set_dump_raw_attr(int sns_id)
{
    VI_DUMP_ATTR_S astDumpAttr[2];
    memset(&astDumpAttr, 0, sizeof(VI_DUMP_ATTR_S) * 2);

    astDumpAttr[0].bEnable    = CVI_TRUE;
    astDumpAttr[0].u32Depth   = 0;
    astDumpAttr[0].enDumpType = VI_DUMP_TYPE_RAW;

    CHECK_RET(MAPI_VCAP_SetDumpRawAttr(g_sns_hdl[sns_id], &astDumpAttr[0]));

    CHECK_RET(MAPI_VCAP_GetDumpRawAttr(g_sns_hdl[sns_id], &astDumpAttr[1]));
    if ((astDumpAttr[0].bEnable != astDumpAttr[1].bEnable) ||
        (astDumpAttr[0].u32Depth != astDumpAttr[1].u32Depth) ||
        (astDumpAttr[0].enDumpType != astDumpAttr[1].enDumpType)) {
        printf("Incorrect dump raw attr for sns[%d]\n", sns_id);
        return -1;
    }

    return 0;
}

static int32_t RAWDATA_DumpCallback(uint32_t ViPipe, VIDEO_FRAME_INFO_S* pstVideoFrame,
                                    uint32_t u32DataNum, void* pPrivateData)
{
    uint32_t i = 0;
    uint32_t frm_num = 1;

    if (!pstVideoFrame) {
        return -1;
    }

    if (pstVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
         frm_num = 2;
    }

    for (i = 0; i < frm_num; i++) {
        size_t image_size = pstVideoFrame[i].stVFrame.u32Length[0];
        unsigned char *ptr = calloc(1, image_size);
        FILE *output;
        char img_name[128] = {0,}, order_id[8] = {0,};

        pstVideoFrame[i].stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(pstVideoFrame[i].stVFrame.u64PhyAddr[0],
                                                               pstVideoFrame[i].stVFrame.u32Length[0]);
        printf("paddr(0x%llx) vaddr(0x%llx)\n",
                    (CVI_U64)pstVideoFrame[i].stVFrame.u64PhyAddr[0],
                    (CVI_U64)(uintptr_t)pstVideoFrame[i].stVFrame.pu8VirAddr[0]);

        memcpy(ptr, (const void *)pstVideoFrame[i].stVFrame.pu8VirAddr[0],
               pstVideoFrame[i].stVFrame.u32Length[0]);
        CVI_SYS_Munmap((void *)pstVideoFrame[i].stVFrame.pu8VirAddr[0],
                       pstVideoFrame[i].stVFrame.u32Length[0]);

        switch (pstVideoFrame[i].stVFrame.enBayerFormat) {
        case BAYER_FORMAT_BG:
            snprintf(order_id, sizeof(order_id), "BG");
            break;
        case BAYER_FORMAT_GB:
            snprintf(order_id, sizeof(order_id), "GB");
            break;
        case BAYER_FORMAT_GR:
            snprintf(order_id, sizeof(order_id), "GR");
            break;
        case BAYER_FORMAT_RG:
            snprintf(order_id, sizeof(order_id), "RG");
            break;
        default:
            snprintf(order_id, sizeof(order_id), "BG");
            break;
        }

        snprintf(img_name, sizeof(img_name),
                "vcap_demo_%d_%d_%s_%s_w_%d_h_%d_x_%d_y_%d.raw",
                ViPipe, u32DataNum, (i == 0) ? "LE" : "SE", order_id,
                pstVideoFrame[i].stVFrame.u32Width,
                pstVideoFrame[i].stVFrame.u32Height,
                pstVideoFrame[i].stVFrame.s16OffsetLeft,
                pstVideoFrame[i].stVFrame.s16OffsetTop);

        printf("dump raw image %s\n", img_name);

        output = fopen(img_name, "wb");

        fwrite(ptr, image_size, 1, output);
        fclose(output);
        free(ptr);
    }
    return 0;
}

static int vcap_demo_dump_raw(const int sns_id)
{
    MAPI_VCAP_RAW_DATA_T stVCapRawData;
    stVCapRawData.pPrivateData = (CVI_VOID*)__FUNCTION__;
    stVCapRawData.pfn_VCAP_RawDataProc = (void *)RAWDATA_DumpCallback;

    CHECK_RET(MAPI_VCAP_StartDumpRaw(g_sns_hdl[sns_id], 1, &stVCapRawData));

    CHECK_RET(MAPI_VCAP_StopDumpRaw(g_sns_hdl[sns_id]));

    return 0;
}

static int vcap_demo_single_sns(const int sns_id)
{
    if ((sns_id > 0) && (g_vcap_attr.u8DevNum != VI_MAX_DEV_NUM)) {
        printf("sns number is missmatch with sns cfg!!!\n");
        return -1;
    }
    CHECK_RET(vcap_demo_sns_init());

    CHECK_RET(vcap_demo_dump_yuv(sns_id));

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static int vcap_demo_dual_sns(void)
{
    if (g_vcap_attr.u8DevNum != VI_MAX_DEV_NUM) {
        printf("sns number is missmatch with sns cfg!!!\n");
        return -1;
    }
    CHECK_RET(vcap_demo_sns_init());

    CHECK_RET(vcap_demo_dump_yuv(0));
    CHECK_RET(vcap_demo_dump_yuv(1));

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static int vcap_demo_crop(const int sns_id)
{
    CHECK_RET(vcap_demo_sns_init());

    CHECK_RET(vcap_demo_set_crop(sns_id, 0, 0, 1280, 720));
    usleep(100 * 1000);
    CHECK_RET(vcap_demo_dump_yuv(sns_id));

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static int vcap_demo_rotation(const int sns_id)
{
    CHECK_RET(vcap_demo_sns_init());

    CHECK_RET(vcap_demo_set_rotation(sns_id, ROTATION_180));
    usleep(100 * 1000);
    CHECK_RET(vcap_demo_dump_yuv(sns_id));

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static int vcap_demo_mirrorflip(const int sns_id)
{
    CHECK_RET(vcap_demo_sns_init());

    CHECK_RET(vcap_demo_set_mirrorflip(sns_id, true, true));
    usleep(100 * 1000);
    CHECK_RET(vcap_demo_dump_yuv(sns_id));

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static int vcap_demo_fps(const int sns_id)
{
    CHECK_RET(vcap_demo_sns_init());

    system("cat /proc/cvitek/vi");

    CHECK_RET(vcap_demo_set_fps(sns_id, 15));
    usleep(100 * 1000);

    system("cat /proc/cvitek/vi");

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static int vcap_demo_raw(const int sns_id)
{
    if ((sns_id > 0) && (g_vcap_attr.u8DevNum != VI_MAX_DEV_NUM)) {
        printf("sns number is missmatch with sns cfg!!!\n");
        return -1;
    }
    CHECK_RET(vcap_demo_sns_init());

    CHECK_RET(vcap_demo_set_dump_raw_attr(sns_id));
    CHECK_RET(vcap_demo_dump_raw(sns_id));

    CHECK_RET(vcap_demo_sns_deinit());

    TEST_PASS;
    return 0;
}

static void usage(int argc, char *argv[])
{
    printf("Usage: %s <index>\n", argv[0]);
    printf("index:\n");
    printf("\t 0)Sensor_0 - Dump_YUV.\n");
    printf("\t 1)Sensor_1 - Dump_YUV.\n");
    printf("\t 2)Dual_Sensor - Dump_YUV.\n");
    printf("\t 3)Sensor_0 - Crop(0,0,1280,720) - Dump_YUV.\n");
    printf("\t 4)Sensor_0 - ROTATION_180 - Dump_YUV.\n");
    printf("\t 5)Sensor_0 - Dump_RAW.\n");
    printf("\t 6)Sensor_0 - MirrorFlip - Dump_YUV.\n");
    printf("\t 7)Sensor_0 - Set 15 FPS.\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage(argc, argv);
        exit(-1);
    }

    // CVI_LOG_INIT();

    CHECK_RET(vcap_demo_sys_init());

    int op = atoi(argv[1]);
    switch(op) {
    case 0:
        CHECK_RET(vcap_demo_single_sns(0));
        break;
    case 1:
        CHECK_RET(vcap_demo_single_sns(1));
        break;
    case 2:
        CHECK_RET(vcap_demo_dual_sns());
        break;
    case 3:
        CHECK_RET(vcap_demo_crop(0));
        break;
    case 4:
        CHECK_RET(vcap_demo_rotation(0));
        break;
    case 5:
        CHECK_RET(vcap_demo_raw(0));
        break;
    case 6:
        CHECK_RET(vcap_demo_mirrorflip(0));
        break;
    case 7:
        CHECK_RET(vcap_demo_fps(0));
        break;
    default:
        break;
    }

    CHECK_RET(vcap_demo_sys_deinit());
    return 0;
}

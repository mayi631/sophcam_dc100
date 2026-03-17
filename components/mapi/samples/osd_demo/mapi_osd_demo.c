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

#define IMAGE_WIDTH   (1920)
#define IMAGE_HEIGHT  (1080)
#define INAGE_FMT     PIXEL_FORMAT_YUV_PLANAR_420
#define CHN_0_WIDTH   (1280)
#define CHN_0_HEIGHT  (720)
#define CHN_1_WIDTH   (640)
#define CHN_1_HEIGHT  (480)

static void usage(int argc, char *argv[])
{
    printf("Usage: %s op time [inputfile] [outputfile_base]\n", argv[0]);
    printf("  op = 0, create time creat bitmap creat circle crat string\n");
    printf("  time: sencond. (0 loop)\n");
}
static int t_sec;

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

int test_osd_basic(const char *inputfile, const char *outputfile_base)
{
    VIDEO_FRAME_INFO_S frame;
    CHECK_RET(MAPI_GetFrameFromFile_YUV(&frame,
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            inputfile, 0));

    // start test
    MAPI_VPROC_HANDLE_T vproc;
    MAPI_VPROC_ATTR_T vproc_attr = MAPI_VPROC_DefaultAttr_TwoChn(
            IMAGE_WIDTH, IMAGE_HEIGHT, INAGE_FMT,
            CHN_0_WIDTH, CHN_0_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420,
            CHN_1_WIDTH, CHN_1_HEIGHT, PIXEL_FORMAT_YUV_PLANAR_420);
    CHECK_RET(MAPI_VPROC_Init(&vproc, 2, &vproc_attr));

    //osd
    MAPI_OSD_FONTS_S pstFonts;
    pstFonts.u32FontWidth = 24;
    pstFonts.u32FontHeight = 24;
    MAPI_OSD_Init(&pstFonts, false);
    int osdIdx = 0;
    int ret = 0;
    MAPI_OSD_ATTR_S osdAttr = {0};
    osdAttr.u32DispNum = 1;
    osdAttr.astDispAttr[0].bShow = 1;
    osdAttr.astDispAttr[0].enBindedMod = MAPI_OSD_BINDMOD_VPROC;
    osdAttr.astDispAttr[0].ModHdl = 2;//grp id
    osdAttr.astDispAttr[0].ChnHdl = 0;//chn id
    osdAttr.astDispAttr[0].enCoordinate = MAPI_OSD_COORDINATE_RATIO_COOR;
    osdAttr.astDispAttr[0].stStartPos.s32X = 2;
    osdAttr.astDispAttr[0].stStartPos.s32Y = 96;
    osdAttr.astDispAttr[0].u32Batch = 0;
    osdAttr.stContent.enType = MAPI_OSD_TYPE_TIME;
    osdAttr.stContent.u32Color = 0xffff; //font bgcolor
    osdAttr.stContent.stTimeContent.stFontSize.u32Width = 24;
    osdAttr.stContent.stTimeContent.stFontSize.u32Height = 24;
    osdAttr.stContent.stTimeContent.enTimeFmt = MAPI_OSD_TIMEFMT_YMD24H;
    osdAttr.stContent.stTimeContent.u32BgColor = 0x7fff;//rgn bgcolor
    ret = MAPI_OSD_SetAttr(osdIdx, &osdAttr);
    if (ret == 0) {
        ret = MAPI_OSD_Start(osdIdx);
    }
    osdAttr.u32DispNum = 1;
    osdAttr.astDispAttr[0].bShow = 1;
    osdAttr.astDispAttr[0].enBindedMod = MAPI_OSD_BINDMOD_VPROC;
    osdAttr.astDispAttr[0].ModHdl = 2;//grp id
    osdAttr.astDispAttr[0].ChnHdl = 1;//chn id
    osdAttr.astDispAttr[0].enCoordinate = MAPI_OSD_COORDINATE_RATIO_COOR;
    osdAttr.astDispAttr[0].stStartPos.s32X = 2;
    osdAttr.astDispAttr[0].stStartPos.s32Y = 96;
    osdAttr.astDispAttr[0].u32Batch = 0;
    osdAttr.stContent.enType = MAPI_OSD_TYPE_TIME;
    osdAttr.stContent.u32Color = 0xffff; //font bgcolor
    osdAttr.stContent.stTimeContent.stFontSize.u32Width = 24;
    osdAttr.stContent.stTimeContent.stFontSize.u32Height = 24;
    osdAttr.stContent.stTimeContent.enTimeFmt = MAPI_OSD_TIMEFMT_YMD24H;
    osdAttr.stContent.stTimeContent.u32BgColor = 0x7fff;//rgn bgcolor
    osdIdx = 1;
    ret = MAPI_OSD_SetAttr(osdIdx, &osdAttr);
    if (ret == 0) {
        ret = MAPI_OSD_Start(osdIdx);
    }
    osdAttr.u32DispNum = 1;
    osdAttr.astDispAttr[0].bShow = 1;
    osdAttr.astDispAttr[0].enBindedMod = MAPI_OSD_BINDMOD_VPROC;
    osdAttr.astDispAttr[0].ModHdl = 2;//grp id
    osdAttr.astDispAttr[0].ChnHdl = 0;//chn id
    osdAttr.astDispAttr[0].enCoordinate = MAPI_OSD_COORDINATE_RATIO_COOR;
    osdAttr.astDispAttr[0].stStartPos.s32X = 2;
    osdAttr.astDispAttr[0].stStartPos.s32Y = 2;
    osdAttr.astDispAttr[0].u32Batch = 1;
    osdAttr.stContent.enType = MAPI_OSD_TYPE_BITMAP;
    osdAttr.stContent.u32Color = 0xffff;
    osdAttr.stContent.stBitmapContent.u32Width = 200;
    osdAttr.stContent.stBitmapContent.u32Height = 58;
    osdAttr.stContent.stBitmapContent.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    osdIdx = 2;
    ret = MAPI_OSD_SetAttr(osdIdx, &osdAttr);
    if (ret == 0) {
        ret = MAPI_OSD_Start(osdIdx);
    }
    osdAttr.u32DispNum = 1;
    osdAttr.astDispAttr[0].bShow = 1;
    osdAttr.astDispAttr[0].enBindedMod = MAPI_OSD_BINDMOD_VPROC;
    osdAttr.astDispAttr[0].ModHdl = 2;//grp id
    osdAttr.astDispAttr[0].ChnHdl = 0;//chn id
    osdAttr.astDispAttr[0].enCoordinate = MAPI_OSD_COORDINATE_RATIO_COOR;
    osdAttr.astDispAttr[0].stStartPos.s32X = 50;
    osdAttr.astDispAttr[0].stStartPos.s32Y = 50;
    osdAttr.astDispAttr[0].u32Batch = 1;
    osdAttr.stContent.enType = MAPI_OSD_TYPE_CIRCLE;
    osdAttr.stContent.u32Color = 0xffff;
    osdAttr.stContent.stCircleContent.u32Width = 50;
    osdAttr.stContent.stCircleContent.u32Height = 50;
    osdIdx = 3;
    ret = MAPI_OSD_SetAttr(osdIdx, &osdAttr);
    if (ret == 0) {
        ret = MAPI_OSD_Start(osdIdx);
    }
    osdAttr.u32DispNum = 1;
    osdAttr.astDispAttr[0].bShow = 1;
    osdAttr.astDispAttr[0].enBindedMod = MAPI_OSD_BINDMOD_VPROC;
    osdAttr.astDispAttr[0].ModHdl = 2;//grp id
    osdAttr.astDispAttr[0].ChnHdl = 0;//chn id
    osdAttr.astDispAttr[0].enCoordinate = MAPI_OSD_COORDINATE_RATIO_COOR;
    osdAttr.astDispAttr[0].stStartPos.s32X = 50;
    osdAttr.astDispAttr[0].stStartPos.s32Y = 96;
    osdAttr.astDispAttr[0].u32Batch = 1;
    osdAttr.stContent.enType = MAPI_OSD_TYPE_STRING;
    osdAttr.stContent.u32Color = 0xffff;
    osdAttr.stContent.stStrContent.stFontSize.u32Width = 24;
    osdAttr.stContent.stStrContent.stFontSize.u32Height = 24;
    snprintf(osdAttr.stContent.stStrContent.szStr, MAPI_OSD_MAX_STR_LEN, "������");//gb2312
    osdAttr.stContent.stStrContent.u32BgColor = 0x7fff;
    osdIdx = 4;
    ret = MAPI_OSD_SetAttr(osdIdx, &osdAttr);
    if (ret == 0) {
        ret = MAPI_OSD_Start(osdIdx);
    }
    //end

    CHECK_RET(DispOpen(1280, 720, ROTATION_90));
    MAPI_DISP_HANDLE_T disp = videoOutCfg.dispCfg[0].dispHdl;

    CHECK_RET(MAPI_VPROC_SendFrame(vproc, &frame));
    CHECK_RET(MAPI_ReleaseFrame(&frame));

    char filename[128];
    int vproc_chn_id_0 = 0;
    VIDEO_FRAME_INFO_S frame_chn_0;
    CHECK_RET(MAPI_VPROC_GetChnFrame(vproc, vproc_chn_id_0,
                                         &frame_chn_0));
    CHECK_RET(MAPI_DISP_SendFrame(disp, &frame_chn_0));
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

    CHECK_RET(DispClose(disp));
    CHECK_RET(MAPI_VPROC_Deinit(vproc));

    printf("------\nSUCCESS\n------\n");

    return 0;
}

int main(int argc, char *argv[])
{

    if (argc != 5 && argc != 4 && argc != 3 && argc != 2) {
        usage(argc, argv);
        exit(-1);
    }
    int op = atoi(argv[1]);
    t_sec = atoi(argv[2]);
    const char* inputfile = NULL;
    const char* outputfile_base = NULL;
    // CVI_LOG_INIT();

    if (argc >= 4) {
        inputfile = argv[3];
    }
    if (argc >= 5) {
        outputfile_base = argv[4];
    }

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
        CHECK_RET(test_osd_basic(inputfile, outputfile_base));
        break;
    default:
        return -1;
    }

    CHECK_RET(MAPI_Media_Deinit());
}

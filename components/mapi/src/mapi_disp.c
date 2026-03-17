#include <string.h>
#include <stdlib.h>
#include "cvi_comm_vo.h"
#include "cvi_vpss.h"
#include "cvi_buffer.h"
#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_isp.h"
#include "cvi_venc.h"
#include "cvi_vdec.h"
#include "cvi_gdc.h"
#include "cvi_region.h"
#include "cvi_bin.h"
#include "osal.h"
#include "cvi_log.h"

#include "mapi.h"
#include "mapi_disp.h"
#include "mapi_internal.h"

typedef struct _MAPI_WND_CTX_S {
    MAPI_DISP_HANDLE_T disp;
    MAPI_WND_ATTR_T attr;

    MAPI_VPROC_HANDLE_T vproc_hdl;
    int chn_idx;

    OSAL_MUTEX_HANDLE_S mutex;
    VIDEO_FRAME_INFO_S *cur_frame;
} MAPI_WND_CTX_T;

typedef struct MAPI_DISP_CTX_S {
    MAPI_DISP_ATTR_T attr;
    unsigned int dev_id;
    unsigned int chn_id;
    unsigned int u32DispMode;
    unsigned int u32VLWidth;
    unsigned int u32VLHeight;
    unsigned int u32VLFrameRate;
    bool use_iproc;
    bool use_vo_rotate;  // when false, use IPROC_Rotate()

    // composite task
    OSAL_TASK_HANDLE_S task;
    char volatile quit;
    OSAL_MUTEX_HANDLE_S mutex;
    VIDEO_FRAME_INFO_S frame;

    bool window_mode;
    int window_num;
    MAPI_WND_HANDLE_T windows[DISP_MAX_WND_NUM];
    MAPI_VPROC_HANDLE_T vproc; // is dt->use_iproc is false
} MAPI_DISP_CTX_S;

static int MAPI_DISP_SetRotation(MAPI_DISP_HANDLE_T disp_hdl,
        ROTATION_E rotate);
static int MAPI_DISP_SendFrameS(MAPI_DISP_HANDLE_T disp_hdl,
        VIDEO_FRAME_INFO_S *frame);
static int MAPI_DISP_GetVideoLayerSize(const VO_PUB_ATTR_S *pstPubAttr,
        unsigned int *pu32W, unsigned int *pu32H, unsigned int *pu32Frm);
static int MAPI_DISP_StartChn(VO_LAYER VoLayer, MAPI_DISP_MODE_E enMode);
static int MAPI_DISP_StopChn(VO_LAYER VoLayer, MAPI_DISP_MODE_E enMode);
static int MAPI_DISP_SetUserIntfSyncInfo(MAPI_DISP_HANDLE_T disp_hdl,
        MAPI_DISP_ATTR_T *attr);
static int MAPI_DISP_StopVideoLayer(VO_LAYER VoLayer, MAPI_DISP_MODE_E enMode);
static bool gDumpVoYuv = false;

void sort_wnd_list(MAPI_WND_HANDLE_T* wnd_list, int wnd_num)
{
    MAPI_WND_HANDLE_T cur_wnd = wnd_list[wnd_num-1];
    MAPI_WND_CTX_T *wt = (MAPI_WND_CTX_T *)cur_wnd;
    int index = -1;
    for(int i = 0; i <= wnd_num - 2; i++){
        MAPI_WND_CTX_T *wt_tmp = (MAPI_WND_CTX_T *)wnd_list[i];
        if(wt->attr.u32Priority < wt_tmp->attr.u32Priority){
            index = i;
            break;
        }
    }

    if(index >= 0){
        for(int j = wnd_num -1; j > index; j--){
            wnd_list[j] = wnd_list[j - 1];
        }
        wnd_list[index] = cur_wnd;
    }

}

void MAPI_DISP_SetDumpStatus(bool en)
{
    gDumpVoYuv = en;
}

static int MAPI_DISP_GetVideoLayerSize(const VO_PUB_ATTR_S *pstPubAttr,
        unsigned int *pu32W, unsigned int *pu32H, unsigned int *pu32Frm)
{
    switch (pstPubAttr->enIntfSync) {
    case VO_OUTPUT_PAL:
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 25;
        break;
    case VO_OUTPUT_NTSC:
        *pu32W = 720;
        *pu32H = 480;
        *pu32Frm = 30;
        break;
    case VO_OUTPUT_1080P24:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 24;
        break;
    case VO_OUTPUT_1080P25:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 25;
        break;
    case VO_OUTPUT_1080P30:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 30;
        break;
    case VO_OUTPUT_720P50:
        *pu32W = 1280;
        *pu32H = 720;
        *pu32Frm = 50;
        break;
    case VO_OUTPUT_720P60:
        *pu32W = 1280;
        *pu32H = 720;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1080P50:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 50;
        break;
    case VO_OUTPUT_1080P60:
        *pu32W = 1920;
        *pu32H = 1080;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_576P50:
        *pu32W = 720;
        *pu32H = 576;
        *pu32Frm = 50;
        break;
    case VO_OUTPUT_480P60:
        *pu32W = 720;
        *pu32H = 480;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_800x600_60:
        *pu32W = 800;
        *pu32H = 600;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1024x768_60:
        *pu32W = 1024;
        *pu32H = 768;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1280x1024_60:
        *pu32W = 1280;
        *pu32H = 1024;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1366x768_60:
        *pu32W = 1366;
        *pu32H = 768;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1440x900_60:
        *pu32W = 1440;
        *pu32H = 900;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1280x800_60:
        *pu32W = 1280;
        *pu32H = 800;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1600x1200_60:
        *pu32W = 1600;
        *pu32H = 1200;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1680x1050_60:
        *pu32W = 1680;
        *pu32H = 1050;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1920x1200_60:
        *pu32W = 1920;
        *pu32H = 1200;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_640x480_60:
        *pu32W = 640;
        *pu32H = 480;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_720x1280_60:
        *pu32W = 720;
        *pu32H = 1280;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_1080x1920_60:
        *pu32W = 1080;
        *pu32H = 1920;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_440x1920_60:
        *pu32W = 440;
        *pu32H = 1920;
        *pu32Frm = 60;
        break;
    case VO_OUTPUT_USER:
        *pu32W = pstPubAttr->stSyncInfo.u16Hact;
        *pu32H = (pstPubAttr->stSyncInfo.bIop) ? pstPubAttr->stSyncInfo.u16Vact :
            pstPubAttr->stSyncInfo.u16Vact * 2;
        *pu32Frm = 60;
        break;
    default:
        CVI_LOGE("vo enIntfSync %d not support!\n", pstPubAttr->enIntfSync);
        return MAPI_SUCCESS;
    }
    return MAPI_SUCCESS;
}

static int MAPI_DISP_StartChn(VO_LAYER VoLayer, MAPI_DISP_MODE_E enMode)
{
    int i;
    int s32Ret;
    int u32WndNum = 0;
    int u32Square = 0;
    int u32Row = 0;
    int u32Col = 0;
    int u32Width = 0;
    int u32Height = 0;
    VO_CHN_ATTR_S stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;

    switch (enMode) {
    case DISP_MODE_1MUX:
        u32WndNum = 1;
        u32Square = 1;
        break;
    case DISP_MODE_2MUX:
        u32WndNum = 2;
        u32Square = 2;
        break;
    case DISP_MODE_4MUX:
        u32WndNum = 4;
        u32Square = 2;
        break;
    case DISP_MODE_8MUX:
        u32WndNum = 8;
        u32Square = 3;
        break;
    case DISP_MODE_9MUX:
        u32WndNum = 9;
        u32Square = 3;
        break;
    case DISP_MODE_16MUX:
        u32WndNum = 16;
        u32Square = 4;
        break;
    case DISP_MODE_25MUX:
        u32WndNum = 25;
        u32Square = 5;
        break;
    case DISP_MODE_36MUX:
        u32WndNum = 36;
        u32Square = 6;
        break;
    case DISP_MODE_49MUX:
        u32WndNum = 49;
        u32Square = 7;
        break;
    case DISP_MODE_64MUX:
        u32WndNum = 64;
        u32Square = 8;
        break;
    case DISP_MODE_2X4:
        u32WndNum = 8;
        u32Square = 3;
        u32Row = 4;
        u32Col = 2;
        break;
    default:
        CVI_LOGE("Undefined DISP_MODE(%d)!\n", enMode);
        return MAPI_ERR_FAILURE;
    }

    s32Ret = CVI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_GetVideoLayerAttr failed with %#x!\n", s32Ret);
        return s32Ret;
    }
    u32Width = stLayerAttr.stDispRect.u32Width;
    u32Height = stLayerAttr.stDispRect.u32Height;
    stChnAttr.stRect.s32X = stLayerAttr.stDispRect.s32X;
    stChnAttr.stRect.s32Y = stLayerAttr.stDispRect.s32Y;
    CVI_LOGI("u32Width:%d, u32Height:%d, u32Square:%d\n", u32Width, u32Height, u32Square);
    CVI_LOGI("s32X %d s32Y %d", stChnAttr.stRect.s32X, stChnAttr.stRect.s32Y);
    for (i = 0; i < u32WndNum; i++) {
        if (enMode == DISP_MODE_1MUX || enMode == DISP_MODE_2MUX || enMode == DISP_MODE_4MUX ||
            enMode == DISP_MODE_8MUX || enMode == DISP_MODE_9MUX || enMode == DISP_MODE_16MUX ||
            enMode == DISP_MODE_25MUX || enMode == DISP_MODE_36MUX || enMode == DISP_MODE_49MUX ||
            enMode == DISP_MODE_64MUX) {
            stChnAttr.stRect.s32X += ALIGN_DOWN((u32Width / u32Square) * (i % u32Square), 2);
            stChnAttr.stRect.s32Y += ALIGN_DOWN((u32Height / u32Square) * (i / u32Square), 2);
            stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Square, 2);
            stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Square, 2);
            stChnAttr.u32Priority = 0;
        } else if (enMode == DISP_MODE_2X4) {
            stChnAttr.stRect.s32X += ALIGN_DOWN((u32Width / u32Col) * (i % u32Col), 2);
            stChnAttr.stRect.s32Y += ALIGN_DOWN((u32Height / u32Row) * (i / u32Col), 2);
            stChnAttr.stRect.u32Width = ALIGN_DOWN(u32Width / u32Col, 2);
            stChnAttr.stRect.u32Height = ALIGN_DOWN(u32Height / u32Row, 2);
            stChnAttr.u32Priority = 0;
        }

        s32Ret = CVI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("VO_SetChnAttr failed with %#x!\n", s32Ret);
            return s32Ret;
        }

        s32Ret = CVI_VO_EnableChn(VoLayer, i);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("VO_EnableChn failed with %#x!\n", s32Ret);
            return s32Ret;
        }
    }

    return MAPI_SUCCESS;
}

static int MAPI_DISP_StopChn(VO_LAYER VoLayer, MAPI_DISP_MODE_E enMode)
{
    CVI_U32 u32WndNum = 0;

    switch (enMode) {
    case DISP_MODE_1MUX: {
        u32WndNum = 1;
        break;
    }
    case DISP_MODE_2MUX: {
        u32WndNum = 2;
        break;
    }
    case DISP_MODE_4MUX: {
        u32WndNum = 4;
        break;
    }
    case DISP_MODE_8MUX: {
        u32WndNum = 8;
        break;
    }
    default:
        CVI_LOGE("invalid enMode %d!\n", enMode);
        return MAPI_ERR_FAILURE;
    }

    for (unsigned i = 0; i < u32WndNum; i++) {
        int s32Ret = CVI_VO_DisableChn(VoLayer, i);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("VO_DisableChn failed with %#x!\n", s32Ret);
            return MAPI_ERR_FAILURE;
        }
    }

    return MAPI_SUCCESS;
}

static int MAPI_DISP_SetRotation(MAPI_DISP_HANDLE_T disp_hdl,
        ROTATION_E rotate)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);

    if (rotate < ROTATION_0 || rotate >= ROTATION_MAX) {
        CVI_LOGE("Invalid disp rotation %d\n", rotate);
        return MAPI_ERR_INVALID;
    }
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;
    if (CVI_VO_SetChnRotation(dt->dev_id, dt->chn_id, rotate) != MAPI_SUCCESS) {
        CVI_LOGE("VO_SetChnRotation failed\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

static int MAPI_DISP_SendFrameS(MAPI_DISP_HANDLE_T disp_hdl,
        VIDEO_FRAME_INFO_S *frame)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_CHECK_NULL_PTR(frame);

    int s32Ret;

    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;
    s32Ret = CVI_VO_SendFrame(dt->dev_id, dt->chn_id, frame, -1);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_SendFrame failed, ret=%x\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }

    return s32Ret;
}

static int MAPI_DISP_SetUserIntfSyncInfo(MAPI_DISP_HANDLE_T disp_hdl,
        MAPI_DISP_ATTR_T *attr)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_CHECK_NULL_PTR(attr);

    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

    dt->attr.stPubAttr.stSyncInfo = attr->stPubAttr.stSyncInfo;

    return MAPI_SUCCESS;
}

static int MAPI_DISP_StopVideoLayer(VO_LAYER VoLayer, MAPI_DISP_MODE_E enMode)
{
    int s32Ret = MAPI_SUCCESS;

    s32Ret = MAPI_DISP_StopChn(VoLayer, enMode);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("MAPI_DISP_StopChn failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_DisableVideoLayer failed with %#x!\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

#define MAPI_DISP_MAX_NUM  (1)
static MAPI_DISP_HANDLE_T g_disp_list[MAPI_DISP_MAX_NUM] = {NULL};

int MAPI_DISP_Init(MAPI_DISP_HANDLE_T *disp_hdl, int disp_id,
            MAPI_DISP_ATTR_T *attr)
{
    MAPI_CHECK_NULL_PTR(attr);

    MAPI_DISP_CTX_S *dt;
    unsigned int VoDev = disp_id;
    int s32Ret;

    dt = (MAPI_DISP_CTX_S *)malloc(sizeof(MAPI_DISP_CTX_S));
    if (!dt) {
        CVI_LOGE("malloc failed\n");
        return MAPI_ERR_NOMEM;
    }
    memset(dt, 0, sizeof(MAPI_DISP_CTX_S));
    dt->attr = *attr;
    // assuming chn 0 only
    CVI_LOG_ASSERT(disp_id == 0, "support one display only\n");
    dt->dev_id = disp_id;
    dt->chn_id = disp_id;
    dt->window_mode = attr->window_mode;
    dt->window_num = 0;
    dt->use_iproc = false;
    dt->vproc = NULL;
    dt->use_vo_rotate = true;
#ifdef VO_INIT
    if (dt->attr.stPubAttr.enIntfSync == VO_OUTPUT_USER) {
        s32Ret = MAPI_DISP_SetUserIntfSyncInfo(dt, &dt->attr);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("MAPI_DISP_SetUserIntfSyncInfo failed with %#x!\n", s32Ret);
            free(dt);
            dt = NULL;
            return s32Ret;
        }
    }

    s32Ret = CVI_VO_SetPubAttr(VoDev, &dt->attr.stPubAttr);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_SetPubAttr failed with %#x!\n", s32Ret);
        free(dt);
        dt = NULL;
        return s32Ret;
    }

    s32Ret = CVI_VO_Enable(VoDev);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_Enable failed with %#x!\n", s32Ret);
        free(dt);
        dt = NULL;
        return s32Ret;
    }
#endif

    s32Ret = CVI_VO_GetPubAttr(VoDev, &dt->attr.stPubAttr);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_GetPubAttr failed with %#x!\n", s32Ret);
        free(dt);
        dt = NULL;
        return s32Ret;
    }

    s32Ret = MAPI_DISP_GetVideoLayerSize(&dt->attr.stPubAttr, &dt->u32VLWidth,
                        &dt->u32VLHeight, &dt->u32VLFrameRate);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("MAPI_DISP_GetVideoLayerSize failed with %#x!\n", s32Ret);
        free(dt);
        dt = NULL;
        return s32Ret;
    }

    *disp_hdl = (MAPI_DISP_HANDLE_T)dt;
    g_disp_list[dt->dev_id] = dt;

    return s32Ret;
}

int MAPI_DISP_Deinit(MAPI_DISP_HANDLE_T disp_hdl)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);

    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;
    int s32Ret;

    // if (dt->vproc) {
    //     MAPI_VPROC_Deinit(dt->vproc);
    // }
#ifdef VO_INIT
    VO_DEV VoDev = dt->dev_id;

    s32Ret = CVI_VO_Disable(VoDev);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_Disable failed with %#x!\n", s32Ret);
        free(dt);
        dt = NULL;
        return s32Ret;
    }
#endif
    free(dt);
    dt = NULL;

    return s32Ret;
}

int MAPI_DISP_Start(MAPI_DISP_HANDLE_T disp_hdl,
            MAPI_DISP_VIDEOLAYER_ATTR_S *pstVideoLayerAttr)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_CHECK_NULL_PTR(pstVideoLayerAttr);

    int s32Ret = 0;
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;
#ifdef VO_INIT
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    unsigned int VoLayer = dt->dev_id;
    MAPI_DISP_MODE_E enVoMode = dt->u32DispMode;

    if (pstVideoLayerAttr->u32BufLen) {
        s32Ret = CVI_VO_SetDisplayBufLen(VoLayer, pstVideoLayerAttr->u32BufLen);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("VO_SetDisplayBufLen failed with %#x!\n", s32Ret);
            MAPI_DISP_Deinit(disp_hdl);
            return s32Ret;
        }
    }

    stLayerAttr.stDispRect.s32X = DISP_VIDEOLAYER_DISPRECT_X;
    stLayerAttr.stDispRect.s32Y = DISP_VIDEOLAYER_DISPRECT_Y;
    if (dt->use_vo_rotate && (dt->attr.rotate != ROTATION_0 && dt->attr.rotate != ROTATION_180)){
        stLayerAttr.stDispRect.u32Width = dt->attr.height;
        stLayerAttr.stDispRect.u32Height = dt->attr.width;
    }else{
        stLayerAttr.stDispRect.u32Width = dt->attr.width;
        stLayerAttr.stDispRect.u32Height = dt->attr.height;
    }

    stLayerAttr.stImageSize.u32Width = dt->u32VLWidth;
    stLayerAttr.stImageSize.u32Height = dt->u32VLHeight;

    if(stLayerAttr.stDispRect.u32Width < dt->u32VLWidth){
        stLayerAttr.stDispRect.s32X = (dt->u32VLWidth - stLayerAttr.stDispRect.u32Width) / 2;
    }

    if(stLayerAttr.stDispRect.u32Height < dt->u32VLHeight){
        stLayerAttr.stDispRect.s32Y = (dt->u32VLHeight - stLayerAttr.stDispRect.u32Height) / 2;
    }

    stLayerAttr.u32DispFrmRt = dt->u32VLFrameRate;
    stLayerAttr.enPixFormat = pstVideoLayerAttr->u32PixelFmt;

    s32Ret = CVI_VO_SetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_SetVideoLayerAttr failed with %#x!\n", s32Ret);
        MAPI_DISP_Deinit(disp_hdl);
        return s32Ret;
    }

    s32Ret = CVI_VO_EnableVideoLayer(VoLayer);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("failed with %#x!\n", s32Ret);
        MAPI_DISP_Deinit(disp_hdl);
        return s32Ret;
    }

    s32Ret = MAPI_DISP_StartChn(VoLayer, enVoMode);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("MAPI_DISP_StartChn failed!\n");
        s32Ret = CVI_VO_DisableVideoLayer(VoLayer);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("VO_DisableVideoLayer failed with %#x!\n", s32Ret);
            return s32Ret;
        }
        MAPI_DISP_Deinit(disp_hdl);
        return s32Ret;
    }

    if (dt->use_vo_rotate && dt->attr.rotate != ROTATION_0) {
        s32Ret = MAPI_DISP_SetRotation(dt, dt->attr.rotate);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("MAPI_DISP_SetRotation failed. s32Ret: %#x\n", s32Ret);
            MAPI_DISP_StopVideoLayer(VoLayer, enVoMode);
            MAPI_DISP_Deinit(disp_hdl);
            return s32Ret;
        }
    }
#endif
    if (dt->window_mode) {
        // create mutex
        OSAL_MUTEX_ATTR_S ma;
        ma.name = "DISP Lock";
        s32Ret = OSAL_MUTEX_Create(&ma, &dt->mutex);
        if (s32Ret != OSAL_SUCCESS) {
            CVI_LOGE("osal_mutex_create %s failed\n", ma.name);
#ifdef VO_INIT
            MAPI_DISP_StopVideoLayer(VoLayer, enVoMode);
            MAPI_DISP_Deinit(disp_hdl);
#endif
            return s32Ret;
        }
    }

    return s32Ret;
}

int MAPI_DISP_Stop(MAPI_DISP_HANDLE_T disp_hdl)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

#ifdef VO_INIT
    VO_LAYER VoLayer = dt->dev_id;
    MAPI_DISP_MODE_E enVoMode = dt->u32DispMode;

    MAPI_DISP_StopVideoLayer(VoLayer, enVoMode);
#endif

    if (dt->window_mode) {
        OSAL_MUTEX_Destroy(dt->mutex);
    }
    return MAPI_SUCCESS;
}

int MAPI_DISP_Pause(MAPI_DISP_HANDLE_T disp_hdl)
{
    CVI_S32 ret = CVI_SUCCESS;
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

    ret = CVI_VO_PauseChn(dt->dev_id, dt->chn_id);
    if(ret != CVI_SUCCESS){
        CVI_LOGE("CVI_VO_PauseChn fail");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_DISP_Resume(MAPI_DISP_HANDLE_T disp_hdl)
{
    CVI_S32 ret = CVI_SUCCESS;
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

    ret = CVI_VO_ResumeChn(dt->dev_id, dt->chn_id);
    if(ret != CVI_SUCCESS){
        CVI_LOGE("CVI_VO_PauseChn fail");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_DISP_BindVproc(MAPI_DISP_HANDLE_T disp_hdl,
        MAPI_VPROC_HANDLE_T vproc_hdl, int vproc_chn_idx)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_CHECK_NULL_PTR(vproc_hdl);
    MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

	stSrcChn.enModId = CVI_ID_VPSS;
	stSrcChn.s32DevId = MAPI_VPROC_GetGrp(vproc_hdl);
	stSrcChn.s32ChnId = vproc_chn_idx;

	stDestChn.enModId = CVI_ID_VO;
	stDestChn.s32DevId = dt->dev_id;
	stDestChn.s32ChnId = dt->chn_id;

    if (CVI_SYS_UnBind(&stSrcChn, &stDestChn) != MAPI_SUCCESS) {
        CVI_LOGE("SAMPLE_COMM_VPSS_Bind_VO failed\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_DISP_UnBindVproc(MAPI_DISP_HANDLE_T disp_hdl,
        MAPI_VPROC_HANDLE_T vproc_hdl, int vproc_chn_idx)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);
    MAPI_CHECK_NULL_PTR(vproc_hdl);
    MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

	stSrcChn.enModId = CVI_ID_VPSS;
	stSrcChn.s32DevId = MAPI_VPROC_GetGrp(vproc_hdl);
	stSrcChn.s32ChnId = vproc_chn_idx;

	stDestChn.enModId = CVI_ID_VO;
	stDestChn.s32DevId = dt->dev_id;
	stDestChn.s32ChnId = dt->chn_id;

    if (CVI_SYS_Bind(&stSrcChn, &stDestChn) != MAPI_SUCCESS) {
        CVI_LOGE("SAMPLE_COMM_VPSS_Bind_VO failed\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_DISP_SendFrame(MAPI_DISP_HANDLE_T disp_hdl,
        VIDEO_FRAME_INFO_S *frame)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);

    #if 0
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;
    if (dt->window_mode) {
        CVI_LOGE("Can not SendFrame to display directly in window_mode\n");
        return MAPI_ERR_INVALID;
    }
    #endif
    return MAPI_DISP_SendFrameS(disp_hdl, frame);
}

int MAPI_DISP_ClearBuf(MAPI_DISP_HANDLE_T disp_hdl)
{
    MAPI_CHECK_NULL_PTR(disp_hdl);

    int s32Ret;

    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)disp_hdl;

    s32Ret = CVI_VO_ClearChnBuf(dt->dev_id, dt->chn_id, true);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("VO_ClearChnBuf failed ! ret: 0x%x \n", s32Ret);
        return s32Ret;
    }

    return MAPI_SUCCESS;
}

int MAPI_DISP_WndBindVproc(MAPI_WND_HANDLE_T wnd_hdl,
        MAPI_VPROC_HANDLE_T vproc_hdl, int vproc_chn_idx)
{
    MAPI_WND_CTX_T *wt = (MAPI_WND_CTX_T *)wnd_hdl;
    OSAL_MUTEX_Lock(wt->mutex);
    wt->vproc_hdl = vproc_hdl;
    wt->chn_idx = vproc_chn_idx;
    OSAL_MUTEX_Unlock(wt->mutex);
    return MAPI_SUCCESS;
}

int MAPI_DISP_CreateWindow(int disp_id, CVI_STITCH_ATTR_S *pstStitchAttr)
{
	CVI_LOG_ASSERT(disp_id == 0, "support one display only\n");
    if (g_disp_list[disp_id] == NULL) {
        CVI_LOGE("NULL pointer g_disp_list[%d]\n", disp_id);
        return MAPI_ERR_INVALID;
    }
    MAPI_DISP_CTX_S *dt = (MAPI_DISP_CTX_S *)g_disp_list[disp_id];

    pstStitchAttr->VoChn = disp_id;
    pstStitchAttr->enOutPixelFormat = dt->attr.pixel_format;
    pstStitchAttr->hVbPool = VB_INVALID_POOLID;
    pstStitchAttr->stOutSize.u32Width = dt->attr.width;
    pstStitchAttr->stOutSize.u32Height = dt->attr.height;
    pstStitchAttr->s32OutFps = dt->attr.fps;

    if (pstStitchAttr->u8ChnNum > 1) {
        VB_POOL_CONFIG_S stVbPoolCfg;
        stVbPoolCfg.u32BlkSize	= COMMON_GetPicBufferSize(dt->attr.width, dt->attr.height, dt->attr.pixel_format,
                    DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
        if(dt->attr.rotate == ROTATION_0){
            stVbPoolCfg.u32BlkCnt = 3;
        }else{
            stVbPoolCfg.u32BlkCnt = 1;
        }
        stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
        pstStitchAttr->hVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
    }

    return MAPI_SUCCESS;
}

int MAPI_DISP_DestroyWindow(int disp_id, CVI_STITCH_ATTR_S *pstStitchAttr)
{
    CVI_LOG_ASSERT(disp_id == 0, "support one display only\n");
    if (g_disp_list[disp_id] == NULL) {
        CVI_LOGE("NULL pointer g_disp_list[%d]\n", disp_id);
        return MAPI_ERR_INVALID;
    }
    MAPI_DISP_ClearBuf(g_disp_list[disp_id]);

    if(pstStitchAttr->hVbPool != VB_INVALID_POOLID){
        CVI_VB_DestroyPool(pstStitchAttr->hVbPool);
    }
    return MAPI_SUCCESS;
}

int MAPI_DISP_GetWndAttr(MAPI_WND_HANDLE_T wnd_hdl, MAPI_WND_ATTR_T *attr)
{
    MAPI_WND_CTX_T *wt = (MAPI_WND_CTX_T *)wnd_hdl;

    OSAL_MUTEX_Lock(wt->mutex);
    *attr = wt->attr;
    OSAL_MUTEX_Unlock(wt->mutex);
    return MAPI_SUCCESS;
}

int MAPI_DISP_SetWndAttr(MAPI_WND_HANDLE_T wnd_hdl, MAPI_WND_ATTR_T *attr)
{
    MAPI_WND_CTX_T *wt = (MAPI_WND_CTX_T *)wnd_hdl;

    OSAL_MUTEX_Lock(wt->mutex);
    wt->attr = *attr;
    OSAL_MUTEX_Unlock(wt->mutex);
    return MAPI_SUCCESS;
}

int MAPI_DISP_SendWndFrame(MAPI_WND_HANDLE_T wnd_hdl,
        VIDEO_FRAME_INFO_S *frame)
{
    MAPI_CHECK_NULL_PTR(wnd_hdl);
    MAPI_CHECK_NULL_PTR(frame);

    MAPI_WND_CTX_T *wt = (MAPI_WND_CTX_T *)wnd_hdl;
    //MAPI_DISP_CTX_S *dt = wt->disp;

    OSAL_MUTEX_Lock(wt->mutex);

    if (wt->cur_frame) {
      // decrease the ref count of the old frame
      MAPI_ReleaseFrame(wt->cur_frame);
      free(wt->cur_frame);
      wt->cur_frame = NULL;
    }

    // replace the cur_frame, by copy the frame struct
    wt->cur_frame = malloc(sizeof(VIDEO_FRAME_INFO_S));
    if(!wt->cur_frame){
        CVI_LOGE("malloc failed\n");
        OSAL_MUTEX_Unlock(wt->mutex);
        return MAPI_ERR_NOMEM;
    }
    memcpy(wt->cur_frame, frame, sizeof(VIDEO_FRAME_INFO_S));

    // increase the ref count of the new frame
    // TODO: can not increase this for now, as VB_S is not accessible
    // VB_BLK blk = CVI_VB_PhysAddr2Handle(frame->stVFrame.u64PhyAddr[0]);
    // atomic_fetch_add(&((VB_S *)blk)->usr_cnt, 1);
    // TODO: the WR here is never release frame after SendWndFrame
    // let DISP module handle the release

    OSAL_MUTEX_Unlock(wt->mutex);

    return MAPI_SUCCESS;
}

int MAPI_DISP_ReleaseWndFrame(MAPI_WND_HANDLE_T wnd_hdl)
{
    if(wnd_hdl == NULL) {
        return MAPI_SUCCESS;
    }

    MAPI_WND_CTX_T *wt = (MAPI_WND_CTX_T *)wnd_hdl;
    //MAPI_DISP_CTX_S *dt = wt->disp;

    OSAL_MUTEX_Lock(wt->mutex);

    if (wt->cur_frame) {
      // decrease the ref count of the old frame
      MAPI_ReleaseFrame(wt->cur_frame);
      free(wt->cur_frame);
      wt->cur_frame = NULL;
    }

    OSAL_MUTEX_Unlock(wt->mutex);

    return MAPI_SUCCESS;
}

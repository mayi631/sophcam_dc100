#include <string.h>
#include <stdlib.h>
#include <pthread.h>

// #include "cvi_common.h"
#include "cvi_comm_vpss.h"
#include "cvi_sys.h"

#include "osal.h"
//#define CVI_LOG_LEVEL CVI_LOG_VERBOSE
#include "cvi_log.h"

#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"
#include <cvi_comm_gdc.h>
#include <cvi_gdc.h>

#include "mapi.h"
#include "mapi_internal.h"

static pthread_mutex_t iproc_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool iproc_initialized = false;

int MAPI_IPROC_Resize(VIDEO_FRAME_INFO_S *frame_in,
        VIDEO_FRAME_INFO_S *frame_out,
        uint32_t resize_width,
        uint32_t resize_height,
        PIXEL_FORMAT_E fmt_out,
        bool keep_aspect_ratio,
        MAPI_IPROC_RESIZE_CROP_ATTR_T *crop_in,
        MAPI_IPROC_RESIZE_CROP_ATTR_T *crop_out,
        MAPI_PREPROCESS_ATTR_T *preprocess)
{
    pthread_mutex_lock(&iproc_mutex);

    if (!iproc_initialized) {
        // always dual mode, grp 0 for vproc, grp 1 for iproc
        #ifdef CHIP_184X
        CVI_VPSS_SetMode(VPSS_MODE_DUAL);
        #else
        CVI_SYS_SetVPSSMode(VPSS_MODE_DUAL);
        #endif
    }

    // IPROC always use grp 0, and dev 0
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    CVI_U8 VpssDev = 0;

    // since HW only do crop on input size
    // if do crop on output size, adjust coordinate backward
    // as well as the resize size
    MAPI_IPROC_RESIZE_CROP_ATTR_T crop_out_adjusted;
    memset(&crop_out_adjusted, 0x00, sizeof(crop_out_adjusted));
    uint32_t resize_width_adjusted  = resize_width;
    uint32_t resize_height_adjusted = resize_height;
    if (crop_out) {
        uint32_t in_w = frame_in->stVFrame.u32Width;
        uint32_t in_h = frame_in->stVFrame.u32Height;
        if (crop_in) {
            in_w = crop_in->w;
            in_h = crop_in->h;
        }
        double x_scale = 1.0 * in_w / resize_width;
        double y_scale = 1.0 * in_h / resize_height;
        crop_out_adjusted.x = crop_out->x * x_scale;
        crop_out_adjusted.y = crop_out->y * y_scale;
        crop_out_adjusted.w = crop_out->w * x_scale;
        crop_out_adjusted.h = crop_out->h * y_scale;
        resize_width_adjusted  = crop_out->w;
        resize_height_adjusted = crop_out->h;
    }

    CVI_U32 abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    abChnEnable[0] = 1;
    VPSS_GRP_ATTR_S   attr_inp;
    VPSS_CHN_ATTR_S   attr_chn[VPSS_MAX_PHY_CHN_NUM];
    memset((void*)&attr_inp, 0, sizeof(attr_inp));
    memset((void*)&attr_chn[0], 0, sizeof(attr_chn));

    attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr_inp.enPixelFormat                  = frame_in->stVFrame.enPixelFormat;
    attr_inp.u32MaxW                        = frame_in->stVFrame.u32Width;
    attr_inp.u32MaxH                        = frame_in->stVFrame.u32Height;
    attr_inp.u8VpssDev                      = VpssDev;

    attr_chn[0].u32Width                    = resize_width_adjusted;
    attr_chn[0].u32Height                   = resize_height_adjusted;
    attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr_chn[0].enPixelFormat               = fmt_out;
    attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr_chn[0].u32Depth                    = 1;  // chn output queue size
    attr_chn[0].bMirror                     = CVI_FALSE;
    attr_chn[0].bFlip                       = CVI_FALSE;
    attr_chn[0].stAspectRatio.enMode        = keep_aspect_ratio ?
                                              ASPECT_RATIO_AUTO :
                                              ASPECT_RATIO_NONE;
    if (keep_aspect_ratio) {
      attr_chn[0].stAspectRatio.bEnableBgColor = CVI_TRUE;
      attr_chn[0].stAspectRatio.u32BgColor = 0x00000000;
    }
    attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    // preprocess
    if (preprocess) {
        MAPI_PREPROCESS_ENABLE(&attr_chn[0], preprocess);
    }

    CVI_S32 rc;
    /*start vpss*/
    if (!iproc_initialized) {
        rc = Vproc_Init(VpssGrp, 1, abChnEnable, &attr_inp, NULL, attr_chn, NULL, NULL, NULL, NULL);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("Vproc_Init failed. rc: 0x%x !\n", rc);
            goto err;
        }
    } else {
        rc  = CVI_VPSS_SetGrpAttr(VpssGrp, &attr_inp);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpAttr failed. rc: 0x%x !\n", rc);
            goto err;
        }
        for (int i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
            if (abChnEnable[i]) {
                VpssChn = i;
                rc = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &attr_chn[VpssChn]);
                if (rc != CVI_SUCCESS)
                {
                    CVI_LOGE("VPSS_SetChnAttr failed. rc: 0x%x !\n", rc);
                    goto err;
                }
            }
        }
    }
    if (!iproc_initialized) {
        iproc_initialized = true;
    }

    if (crop_in && crop_out) {
        VPSS_CROP_INFO_S stCropInInfo;
        stCropInInfo.bEnable = CVI_TRUE;
        stCropInInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
        stCropInInfo.stCropRect.s32X      = crop_in->x;
        stCropInInfo.stCropRect.s32Y      = crop_in->y;
        stCropInInfo.stCropRect.u32Width  = crop_in->w;
        stCropInInfo.stCropRect.u32Height = crop_in->h;
        CVI_LOGI("Crop IN, %d %d %d %d\n",
                stCropInInfo.stCropRect.s32X,
                stCropInInfo.stCropRect.s32Y,
                stCropInInfo.stCropRect.u32Width,
                stCropInInfo.stCropRect.u32Height);
        rc = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropInInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }

        VPSS_CROP_INFO_S stCropOutInfo;
        stCropOutInfo.bEnable = CVI_TRUE;
        stCropOutInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
        stCropOutInfo.stCropRect.s32X      = crop_out_adjusted.x;
        stCropOutInfo.stCropRect.s32Y      = crop_out_adjusted.y;
        stCropOutInfo.stCropRect.u32Width  = crop_out_adjusted.w;
        stCropOutInfo.stCropRect.u32Height = crop_out_adjusted.h;
        CVI_LOGI("Crop OUT, %d %d %d %d\n",
                stCropOutInfo.stCropRect.s32X,
                stCropOutInfo.stCropRect.s32Y,
                stCropOutInfo.stCropRect.u32Width,
                stCropOutInfo.stCropRect.u32Height);
        rc = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropOutInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetChnCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }
    } else if (crop_in) {
        VPSS_CROP_INFO_S stCropInInfo;
        stCropInInfo.bEnable = CVI_TRUE;
        stCropInInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
        stCropInInfo.stCropRect.s32X      = crop_in->x;
        stCropInInfo.stCropRect.s32Y      = crop_in->y;
        stCropInInfo.stCropRect.u32Width  = crop_in->w;
        stCropInInfo.stCropRect.u32Height = crop_in->h;
        CVI_LOGI("Crop IN, %d %d %d %d\n",
                stCropInInfo.stCropRect.s32X,
                stCropInInfo.stCropRect.s32Y,
                stCropInInfo.stCropRect.u32Width,
                stCropInInfo.stCropRect.u32Height);
        rc = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropInInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }

        VPSS_CROP_INFO_S stCropDisableInfo;
        stCropDisableInfo.bEnable = CVI_FALSE;
        rc = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropDisableInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetChnCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }

    } else if (crop_out) {
        VPSS_CROP_INFO_S stCropDisableInfo;
        stCropDisableInfo.bEnable = CVI_FALSE;
        rc = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropDisableInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }

        VPSS_CROP_INFO_S stCropOutInfo;
        stCropOutInfo.bEnable = CVI_TRUE;
        stCropOutInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
        stCropOutInfo.stCropRect.s32X      = crop_out_adjusted.x;
        stCropOutInfo.stCropRect.s32Y      = crop_out_adjusted.y;
        stCropOutInfo.stCropRect.u32Width  = crop_out_adjusted.w;
        stCropOutInfo.stCropRect.u32Height = crop_out_adjusted.h;
        CVI_LOGI("Crop OUT, %d %d %d %d\n",
                stCropOutInfo.stCropRect.s32X,
                stCropOutInfo.stCropRect.s32Y,
                stCropOutInfo.stCropRect.u32Width,
                stCropOutInfo.stCropRect.u32Height);
        rc = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropOutInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetChnCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }

    } else {
        VPSS_CROP_INFO_S stCropDisableInfo;
        stCropDisableInfo.bEnable = CVI_FALSE;
        rc = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropDisableInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }
        rc = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &stCropDisableInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetChnCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }
    }

    rc = CVI_VPSS_SendFrame(VpssGrp, frame_in, -1);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VPSS_SendFrame failed. rc: 0x%x !\n", rc);
        goto err;
    }

    rc = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, frame_out, -1);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VPSS_GetChnFrame failed with %#x\n", rc);
        goto err;
    }

    pthread_mutex_unlock(&iproc_mutex);
    return MAPI_SUCCESS;
err:
    pthread_mutex_unlock(&iproc_mutex);
    return MAPI_ERR_FAILURE;

}

int MAPI_IPROC_StrechBlt(VIDEO_FRAME_INFO_S *frame_src,
        MAPI_IPROC_RECT_T *rect_src,   // if NULL, use the full frame_src
        VIDEO_FRAME_INFO_S *frame_dst,
        bool frame_dst_has_content,        // when this is true, the frame_dst is both in/out
        uint32_t frame_dst_width,          // used when frame_dst_has_content is false
        uint32_t frame_dst_height,         // used when frame_dst_has_content is false
        PIXEL_FORMAT_E frame_dst_fmt,      // used when frame_dst_has_content is false
        bool     frame_dst_eable_bg_colar, // used when frame_dst_has_content is false
        uint32_t frame_dst_bg_color,       // used when frame_dst_has_content is false
        MAPI_IPROC_RECT_T *rect_dst)
{
    pthread_mutex_lock(&iproc_mutex);

    if (!iproc_initialized) {
        // always dual mode, grp 0 for vproc, grp 1 for iproc
        #ifdef CHIP_184X
        CVI_VPSS_SetMode(VPSS_MODE_DUAL);
        #else
        CVI_SYS_SetVPSSMode(VPSS_MODE_DUAL);
        #endif
    }

    // IPROC always use grp 0
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;
    CVI_U8 VpssDev = 0;

    CVI_U32 abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
    abChnEnable[0] = 1;
    VPSS_GRP_ATTR_S   attr_inp;
    VPSS_CHN_ATTR_S   attr_chn[VPSS_MAX_PHY_CHN_NUM];
    memset((void*)&attr_inp, 0, sizeof(attr_inp));
    memset((void*)&attr_chn[0], 0, sizeof(attr_chn));

    attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr_inp.enPixelFormat                  = frame_src->stVFrame.enPixelFormat;
    attr_inp.u32MaxW                        = frame_src->stVFrame.u32Width;
    attr_inp.u32MaxH                        = frame_src->stVFrame.u32Height;
    attr_inp.u8VpssDev                      = VpssDev;

    if (frame_dst_has_content) {
        attr_chn[0].u32Width                = frame_dst->stVFrame.u32Width;
        attr_chn[0].u32Height               = frame_dst->stVFrame.u32Height;
        attr_chn[0].enPixelFormat           = frame_dst->stVFrame.enPixelFormat;
        attr_chn[0].stAspectRatio.bEnableBgColor    = CVI_FALSE;
    } else {
        attr_chn[0].u32Width                = frame_dst_width;
        attr_chn[0].u32Height               = frame_dst_height;
        attr_chn[0].enPixelFormat           = frame_dst_fmt;
        attr_chn[0].stAspectRatio.bEnableBgColor    = frame_dst_eable_bg_colar;
        attr_chn[0].stAspectRatio.u32BgColor        = frame_dst_bg_color;
    }
    attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr_chn[0].u32Depth                    = 1;  // chn output queue size
    attr_chn[0].bMirror                     = CVI_FALSE;
    attr_chn[0].bFlip                       = CVI_FALSE;
    attr_chn[0].stAspectRatio.enMode                = ASPECT_RATIO_MANUAL;
    attr_chn[0].stAspectRatio.stVideoRect.s32X      = rect_dst->x;
    attr_chn[0].stAspectRatio.stVideoRect.s32Y      = rect_dst->y;
    attr_chn[0].stAspectRatio.stVideoRect.u32Width  = rect_dst->w;
    attr_chn[0].stAspectRatio.stVideoRect.u32Height = rect_dst->h;
    attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    CVI_S32 rc;
    /*start vpss*/
    if (!iproc_initialized) {
        rc = Vproc_Init(VpssGrp, 1, abChnEnable, &attr_inp, NULL, attr_chn, NULL, NULL, NULL, NULL);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("SAMPLE_COMM_VPSS_Init failed. rc: 0x%x !\n", rc);
            goto err;
        }
    } else {
        rc  = CVI_VPSS_SetGrpAttr(VpssGrp, &attr_inp);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpAttr failed. rc: 0x%x !\n", rc);
            goto err;
        }
        for (int i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
            if (abChnEnable[i]) {
                VpssChn = i;
                rc = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &attr_chn[VpssChn]);
                if (rc != CVI_SUCCESS)
                {
                    CVI_LOGE("VPSS_SetChnAttr failed. rc: 0x%x !\n", rc);
                    goto err;
                }
            }
        }
    }
    if (!iproc_initialized) {
        iproc_initialized = true;
    }

    if (rect_src) {
        VPSS_CROP_INFO_S stCropInfo;
        stCropInfo.bEnable = CVI_TRUE;
        stCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
        stCropInfo.stCropRect.s32X      = rect_src->x;
        stCropInfo.stCropRect.s32Y      = rect_src->y;
        stCropInfo.stCropRect.u32Width  = rect_src->w;
        stCropInfo.stCropRect.u32Height = rect_src->h;
        CVI_LOGI("Crop IN, %d %d %d %d\n",
                stCropInfo.stCropRect.s32X,
                stCropInfo.stCropRect.s32Y,
                stCropInfo.stCropRect.u32Width,
                stCropInfo.stCropRect.u32Height);
        rc = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }
    } else {
        VPSS_CROP_INFO_S stCropInfo;
        stCropInfo.bEnable = CVI_FALSE;
        rc = CVI_VPSS_SetGrpCrop(VpssGrp, &stCropInfo);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed. rc: 0x%x !\n", rc);
            goto err;
        }
    }

    if (frame_dst_has_content) {
        rc = CVI_VPSS_SendChnFrame(VpssGrp, VpssChn, frame_dst, -1);
        if (rc != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SendChnFrame failed. rc: 0x%x !\n", rc);
            goto err;
        }
        // CAUTION: need to release here
        // and will getChnFrame for this frame again
        MAPI_ReleaseFrame(frame_dst);
    }

    rc = CVI_VPSS_SendFrame(VpssGrp, frame_src, -1);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VPSS_SendFrame failed. rc: 0x%x !\n", rc);
        goto err;
    }

    rc = CVI_VPSS_GetChnFrame(VpssGrp, VpssChn, frame_dst, -1);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VPSS_GetChnFrame failed with %#x\n", rc);
        goto err;
    }

    pthread_mutex_unlock(&iproc_mutex);
    return MAPI_SUCCESS;
err:
    pthread_mutex_unlock(&iproc_mutex);
    return MAPI_ERR_FAILURE;
}

int MAPI_IPROC_ReleaseFrame(VIDEO_FRAME_INFO_S *frm)
{
    // IPROC always use grp 0
    VPSS_GRP VpssGrp = 0;
    VPSS_CHN VpssChn = 0;

    CVI_S32 rc;
    rc = CVI_VPSS_ReleaseChnFrame(VpssGrp, VpssChn, frm);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VPSS_ReleaseChnFrame failed with %#x\n", rc);
        return MAPI_ERR_FAILURE;
    }

    return MAPI_SUCCESS;
}

static pthread_mutex_t gdc_mutex = PTHREAD_MUTEX_INITIALIZER;

int MAPI_IPROC_Rotate(VIDEO_FRAME_INFO_S *frame_src,
        VIDEO_FRAME_INFO_S *frame_dst,
        ROTATION_E rotation)
{
    uint32_t out_w, out_h;
    PIXEL_FORMAT_E out_fmt = frame_src->stVFrame.enPixelFormat;
    if (rotation == ROTATION_90 || rotation == ROTATION_270) {
        out_w = frame_src->stVFrame.u32Height;
        out_h = frame_src->stVFrame.u32Width;
    } else {
        out_w = frame_src->stVFrame.u32Width;
        out_h = frame_src->stVFrame.u32Height;
    }
    int rc = MAPI_AllocateFrame(frame_dst, out_w, out_h, out_fmt);
    if (rc != MAPI_SUCCESS) {
        CVI_LOGE("MAPI_AllocateFrame failed, w %d, h %d, fmt %d, rc %d\n",
            out_w, out_h, out_fmt, rc);
        return rc;
    }

    GDC_TASK_ATTR_S stTask;
    stTask.stImgIn = *frame_src;
    stTask.stImgOut = *frame_dst;

    pthread_mutex_lock(&gdc_mutex);
    GDC_HANDLE hHandle;
    CVI_GDC_BeginJob(&hHandle);
    CVI_GDC_AddRotationTask(hHandle, &stTask, rotation);
    int ret = CVI_GDC_EndJob(hHandle);
    pthread_mutex_unlock(&gdc_mutex);

    if (ret != 0) {
        CVI_LOGE("GDC_AddRotationTask failed, w %d, h %d, fmt %d, ret %d\n",
            out_w, out_h, out_fmt, ret);
        return MAPI_ERR_FAILURE;
    }

    return MAPI_SUCCESS;
}

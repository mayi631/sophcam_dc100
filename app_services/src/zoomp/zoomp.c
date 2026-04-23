#include "cvi_log.h"
#include "zoomp.h"

#ifndef ZOOM_MAX_RATIO
#define ZOOM_MAX_RATIO 8.0f
#endif

static CVI_U32 g_is_zoom_init = 0;
static RECT_S g_org_win = {0};

CVI_S32 ZOOMP_Init(RECT_S org_win){
    if(g_is_zoom_init){
        return CVI_FAILURE;
    }else{
        g_org_win = org_win;
        g_is_zoom_init = 1;
        CVI_LOGI("g_org_win_x:%d, g_org_win_y:%d g_org_win_w:%d g_org_win_h:%d",
            g_org_win.s32X, g_org_win.s32Y, g_org_win.u32Width, g_org_win.u32Height);
    }
    return CVI_SUCCESS;
}

CVI_S32 ZOOMP_DeInit(CVI_VOID){
    g_is_zoom_init = 0;
    return CVI_SUCCESS;
}

CVI_S32 ZOOMP_Reset(CVI_VOID){
    g_is_zoom_init = 0;
    return CVI_SUCCESS;
}

CVI_U32 ZOOMP_Is_Init(CVI_VOID){
    return g_is_zoom_init;
}

CVI_S32 ZOOMP_GetCropInfo(RECT_S in, RECT_S* out, float radio){
    float zoom_ratio = 1.0f;
    float level_ratio = 0.0f;
    CVI_U32 crop_width = 0;
    CVI_U32 crop_height = 0;
    CVI_S32 crop_x = 0;
    CVI_S32 crop_y = 0;

    CVI_LOGI("[in] radio :%f x:%d, y:%d w:%d h:%d", radio, in.s32X, in.s32Y, in.u32Width, in.u32Height);

    if(radio < 1.0f || radio > ZOOM_MAX_RADIO){
        CVI_LOGE("raido : %f is over", radio);
        return CVI_FAILURE;
    }

    if(g_org_win.u32Width == 0 || g_org_win.u32Height == 0){
        CVI_LOGE("invalid org win: w:h = %d:%d", g_org_win.u32Width, g_org_win.u32Height);
        return CVI_FAILURE;
    }

    /* Map level [1, ZOOM_MAX_RADIO] to zoom ratio [1x, ZOOM_MAX_RATIO]. */
    level_ratio = (radio - 1.0f) / (ZOOM_MAX_RADIO - 1.0f);
    zoom_ratio = 1.0f + level_ratio * (ZOOM_MAX_RATIO - 1.0f);

    crop_width = (CVI_U32)((float)g_org_win.u32Width / zoom_ratio);
    crop_height = (CVI_U32)((float)g_org_win.u32Height / zoom_ratio);
    crop_x = g_org_win.s32X + ((CVI_S32)g_org_win.u32Width - (CVI_S32)crop_width) / 2;
    crop_y = g_org_win.s32Y + ((CVI_S32)g_org_win.u32Height - (CVI_S32)crop_height) / 2;
    /* VPSS crop commonly expects even-aligned rectangle. */
    crop_width &= ~1U;
    crop_height &= ~1U;
    crop_x &= ~1;
    crop_y &= ~1;

    out->s32X = crop_x;
    out->s32Y = crop_y;
    out->u32Width = crop_width;
    out->u32Height = crop_height;

    CVI_LOGI("[out] zoom_ratio:%f x:%d, y:%d w:%d h:%d", zoom_ratio,
        out->s32X, out->s32Y, out->u32Width, out->u32Height);

    return CVI_SUCCESS;
}

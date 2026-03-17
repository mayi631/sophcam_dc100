#include "cvi_log.h"
#include "zoomp.h"

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
    CVI_U32 x_step = 0, y_step = 0;

    CVI_LOGI("[in] radio :%f x:%d, y:%d w:%d h:%d", radio, in.s32X, in.s32Y, in.u32Width, in.u32Height);

    if(radio > ZOOM_MAX_RADIO){
        CVI_LOGE("raido : %f is over", radio);
        return CVI_FAILURE;
    }

    /* 16 : 9 */
    if(9 * in.u32Width == 16 * in.u32Height){
        x_step = ZOOM_STEP_PER_RADIO;
        y_step = (ZOOM_STEP_PER_RADIO * 9) >> 4;
    /* 4 : 3 */
    }else if(3 * in.u32Width == 4 * in.u32Height){
        x_step = ZOOM_STEP_PER_RADIO;
        y_step = (ZOOM_STEP_PER_RADIO * 3) >> 2;
    /* 1 : 1 */
    }else if(in.u32Width == in.u32Height){
        x_step = ZOOM_STEP_PER_RADIO;
        y_step = ZOOM_STEP_PER_RADIO;
    }else{
        CVI_LOGE("don't support radio: w:h = %d:%d", in.u32Width, in.u32Height);
        return CVI_FAILURE;
    }

    CVI_LOGI("x_step:%d y_step:%d", x_step, y_step);

    out->s32X = g_org_win.s32X + (radio - 1) * x_step;
    out->s32Y = g_org_win.s32Y + (radio - 1) * y_step;
    out->u32Width = g_org_win.u32Width - (radio - 1) * x_step * 2;
    out->u32Height = g_org_win.u32Height - (radio - 1) * y_step * 2;

    CVI_LOGI("[out] x:%d, y:%d w:%d h:%d", out->s32X, out->s32Y, out->u32Width, out->u32Height);

    return CVI_SUCCESS;
}

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "hal_screen_comp.h"
#include "hal_screen_inner.h"
#include "hal_pwm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup     SCREEN */
/** @{ */  /** <!-- [SCREEN] */

static HAL_SCREEN_CTX_S s_astHALSCREENCtx[HAL_SCREEN_IDX_BUTT];


int32_t HAL_SCREEN_COMM_Register(HAL_SCREEN_IDX_E enScreenIndex, const HAL_SCREEN_OBJ_S* pstScreenObj)
{
    if (s_astHALSCREENCtx[enScreenIndex].bRegister)
    {
        printf("Screen[%d] has been registered\n", enScreenIndex);
        return -1;
    }

    memcpy(&s_astHALSCREENCtx[enScreenIndex].stScreenObj, pstScreenObj, sizeof(HAL_SCREEN_OBJ_S));
    s_astHALSCREENCtx[enScreenIndex].bRegister = true;

    return 0;
}

int32_t HAL_SCREEN_COMM_Init(HAL_SCREEN_IDX_E enScreenIndex)
{
	int32_t s32Ret = 0;

    if (s_astHALSCREENCtx[enScreenIndex].bInit)
    {
        printf("Screen[%d] has been inited\n", enScreenIndex);
        return 0;
    }

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnInit)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnInit();
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    s_astHALSCREENCtx[enScreenIndex].bInit = true;
    return s32Ret;
}

int32_t HAL_SCREEN_COMM_GetAttr(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_ATTR_S* pstAttr)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetAttr)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetAttr(pstAttr);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_GetDisplayState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E* penDisplayState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetDisplayState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetDisplayState(penDisplayState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_SetDisplayState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E enDisplayState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetDisplayState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetDisplayState(enDisplayState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_GetBackLightState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E* penBackLightState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetBackLightState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetBackLightState(penBackLightState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E enBackLightState)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetBackLightState)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetBackLightState(enBackLightState);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

HAL_SCREEN_PWM_S HAL_SCREEN_COMM_GetLuma(HAL_SCREEN_IDX_E enScreenIndex)
{

    return s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma();
    //int32_t s32Ret = 0;
    // if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma)
    // {
    //     //s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma();
    //     return s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetLuma();
    // }
    // else
    // {
    //     printf("Screen[%d] Null ptr.\n", enScreenIndex);
    //     return -1;
    // }

    // return s32Ret;
}

int32_t HAL_SCREEN_COMM_SetLuma(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_PWM_S pwmAttr)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetLuma)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetLuma(pwmAttr);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_GetSaturature(HAL_SCREEN_IDX_E enScreenIndex, uint32_t* pu32Saturature)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetSaturature)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetSaturature(pu32Saturature);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_SetSaturature(HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Saturature)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetSaturature)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetSaturature(u32Saturature);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_GetContrast(HAL_SCREEN_IDX_E enScreenIndex, uint32_t* pu32Contrast)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetContrast)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnGetContrast(pu32Contrast);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_SCREEN_COMM_SetContrast(HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Contrast)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetContrast)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnSetContrast(u32Contrast);
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    return s32Ret;
}

int32_t HAL_COMM_SCREEN_Deinit(HAL_SCREEN_IDX_E enScreenIndex)
{
    int32_t s32Ret = 0;

    if(NULL != s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnDeinit)
    {
        s32Ret = s_astHALSCREENCtx[enScreenIndex].stScreenObj.pfnDeinit();
    }
    else
    {
        printf("Screen[%d] Null ptr.\n", enScreenIndex);
        return -1;
    }

    s_astHALSCREENCtx[enScreenIndex].bInit = false;

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


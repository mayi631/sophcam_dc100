#ifndef __MAPI_VPROC_H__
#define __MAPI_VPROC_H__

#include <pthread.h>
#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"
#include "mapi_define.h"
#include "mapi_vcap.h"

#include "cvi_comm_vpss.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef MAPI_HANDLE_T MAPI_VPROC_HANDLE_T;

#define MAPI_VPROC_MAX_CHN_NUM    (4)
#define MAPI_VPROC_TIMEOUT_MS    (1000)

typedef struct MAPI_VPROC_ATTR_S {
    VPSS_GRP_ATTR_S     attr_inp;
    VPSS_CROP_INFO_S    crop_info;
    int                 chn_num;
    uint32_t            chn_enable[MAPI_VPROC_MAX_CHN_NUM];
    VPSS_CHN_ATTR_S     attr_chn[MAPI_VPROC_MAX_CHN_NUM];
    VPSS_CHN_BUF_WRAP_S chn_bufWrap[MAPI_VPROC_MAX_CHN_NUM];
    VPSS_CROP_INFO_S    chn_cropInfo[MAPI_VPROC_MAX_CHN_NUM];
    uint32_t            chn_vbcnt[MAPI_VPROC_MAX_CHN_NUM];
    uint32_t            chn_bindVbPoolID[MAPI_VPROC_MAX_CHN_NUM];
    uint32_t            lowdelay_cnt[MAPI_VPROC_MAX_CHN_NUM];
} MAPI_VPROC_ATTR_T;

typedef struct MAPI_EXTCHN_ATTR_S {
    uint32_t        ChnId;
    uint32_t        BindVprocChnId;
    VPSS_CHN_ATTR_S VpssChnAttr;
    uint32_t        chn_vbcnt[MAPI_VPROC_MAX_CHN_NUM];
    uint32_t        chn_bindVbPoolID[MAPI_VPROC_MAX_CHN_NUM];
} MAPI_EXTCHN_ATTR_T;

typedef enum MAPI_VPROC_CMD_E
{
    MAPI_VPROC_CMD_CHN_ROTATE,
    MAPI_VPROC_CMD_CHN_CROP,
    MAPI_VPROC_CMD_BUTT
}MAPI_VPROC_CMD_T;

typedef int (*PFN_VPROC_FrameDataProc)(uint32_t Grp,
    uint32_t Chn, VIDEO_FRAME_INFO_S *pFrame, void *pPrivateData);

typedef struct MAPI_DUMP_FRAME_CALLBACK_FUNC_S {
    PFN_VPROC_FrameDataProc pfunFrameProc;
    void *pPrivateData;
} MAPI_DUMP_FRAME_CALLBACK_FUNC_T;


static inline MAPI_VPROC_ATTR_T MAPI_VPROC_DefaultAttr_OneChn(
    uint32_t          width_in,
    uint32_t          height_in,
    PIXEL_FORMAT_E    pixel_format_in,
    uint32_t          width_out,
    uint32_t          height_out,
    PIXEL_FORMAT_E    pixel_format_out)
{
    MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr.attr_inp.enPixelFormat                  = pixel_format_in;
    attr.attr_inp.u32MaxW                        = width_in;
    attr.attr_inp.u32MaxH                        = height_in;
    attr.attr_inp.u8VpssDev                   = 0;

    attr.chn_num = 1;
    attr.chn_enable[0] = 1;

    attr.attr_chn[0].u32Width                    = width_out;
    attr.attr_chn[0].u32Height                   = height_out;
    attr.attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[0].enPixelFormat               = pixel_format_out;
    attr.attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[0].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[0].bMirror                     = CVI_FALSE;
    attr.attr_chn[0].bFlip                       = CVI_FALSE;
    attr.attr_chn[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    return attr;
}

static inline MAPI_VPROC_ATTR_T MAPI_VPROC_DefaultAttr_TwoChn(
    uint32_t          width_in,
    uint32_t          height_in,
    PIXEL_FORMAT_E    pixel_format_in,
    uint32_t          width_out0,
    uint32_t          height_out0,
    PIXEL_FORMAT_E    pixel_format_out0,
    uint32_t          width_out1,
    uint32_t          height_out1,
    PIXEL_FORMAT_E    pixel_format_out1)
{
    MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr.attr_inp.enPixelFormat                  = pixel_format_in;
    attr.attr_inp.u32MaxW                        = width_in;
    attr.attr_inp.u32MaxH                        = height_in;
    attr.attr_inp.u8VpssDev                   = 0;

    attr.chn_num = 2;

    attr.attr_chn[0].u32Width                    = width_out0;
    attr.attr_chn[0].u32Height                   = height_out0;
    attr.attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[0].enPixelFormat               = pixel_format_out0;
    attr.attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[0].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[0].bMirror                     = CVI_FALSE;
    attr.attr_chn[0].bFlip                       = CVI_FALSE;
    attr.attr_chn[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    attr.attr_chn[1].u32Width                    = width_out1;
    attr.attr_chn[1].u32Height                   = height_out1;
    attr.attr_chn[1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[1].enPixelFormat               = pixel_format_out1;
    attr.attr_chn[1].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[1].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[1].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[1].bMirror                     = CVI_FALSE;
    attr.attr_chn[1].bFlip                       = CVI_FALSE;
    attr.attr_chn[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[1].stNormalize.bEnable         = CVI_FALSE;

    return attr;
}

static inline MAPI_VPROC_ATTR_T MAPI_VPROC_DefaultAttr_ThreeChn(
    uint32_t          width_in,
    uint32_t          height_in,
    PIXEL_FORMAT_E    pixel_format_in,
    uint32_t          width_out0,
    uint32_t          height_out0,
    PIXEL_FORMAT_E    pixel_format_out0,
    uint32_t          width_out1,
    uint32_t          height_out1,
    PIXEL_FORMAT_E    pixel_format_out1,
    uint32_t          width_out2,
    uint32_t          height_out2,
    PIXEL_FORMAT_E    pixel_format_out2)
{
    MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    attr.attr_inp.enPixelFormat                  = pixel_format_in;
    attr.attr_inp.u32MaxW                        = width_in;
    attr.attr_inp.u32MaxH                        = height_in;
    attr.attr_inp.u8VpssDev                   = 0;

    attr.chn_num = 3;

    attr.attr_chn[0].u32Width                    = width_out0;
    attr.attr_chn[0].u32Height                   = height_out0;
    attr.attr_chn[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[0].enPixelFormat               = pixel_format_out0;
    attr.attr_chn[0].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[0].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[0].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[0].bMirror                     = CVI_FALSE;
    attr.attr_chn[0].bFlip                       = CVI_FALSE;
    attr.attr_chn[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[0].stNormalize.bEnable         = CVI_FALSE;

    attr.attr_chn[1].u32Width                    = width_out1;
    attr.attr_chn[1].u32Height                   = height_out1;
    attr.attr_chn[1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[1].enPixelFormat               = pixel_format_out1;
    attr.attr_chn[1].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[1].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[1].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[1].bMirror                     = CVI_FALSE;
    attr.attr_chn[1].bFlip                       = CVI_FALSE;
    attr.attr_chn[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[1].stNormalize.bEnable         = CVI_FALSE;

// channel[2]for venc
    attr.attr_chn[2].u32Width                    = width_out2;
    attr.attr_chn[2].u32Height                   = height_out2;
    attr.attr_chn[2].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    attr.attr_chn[2].enPixelFormat               = pixel_format_out2;
    attr.attr_chn[2].stFrameRate.s32SrcFrameRate = -1;
    attr.attr_chn[2].stFrameRate.s32DstFrameRate = -1;
    attr.attr_chn[2].u32Depth                    = 1;    // output buffer queue size
    attr.attr_chn[2].bMirror                     = CVI_FALSE;
    attr.attr_chn[2].bFlip                       = CVI_FALSE;
    attr.attr_chn[2].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    attr.attr_chn[2].stNormalize.bEnable         = CVI_FALSE;

    return attr;
}

typedef struct VPROC_DUMP_CTX_S {
    CVI_BOOL bStart;
    CVI_U32 Grp;
    CVI_S32 Chn;
    CVI_S32 s32Count;
    MAPI_DUMP_FRAME_CALLBACK_FUNC_T stCallbackFun;
    pthread_t pthreadDump;
} VPROC_DUMP_CTX_T;

typedef struct EXT_VPROC_CHN_CTX_S {
    CVI_U32     ExtChnGrp;
    CVI_BOOL    ExtChnEnable;
    MAPI_EXTCHN_ATTR_T ExtChnAttr;
} EXT_VPROC_CHN_CTX_T;

typedef struct MAPI_VPROC_CTX_S {
    VPSS_GRP VpssGrp;
    CVI_U32 chn_num;
    CVI_U32 abChnEnable[VPSS_MAX_PHY_CHN_NUM];
    MAPI_VPROC_ATTR_T attr;
    EXT_VPROC_CHN_CTX_T ExtChn[VPSS_MAX_PHY_CHN_NUM];
    VPROC_DUMP_CTX_T stVprocDumpCtx;
} MAPI_VPROC_CTX_T;

// grp_id : 0 ~ 15, use -1 for auto select
int MAPI_VPROC_Init(MAPI_VPROC_HANDLE_T *vproc_hdl,
        int grp_id, MAPI_VPROC_ATTR_T *attr);
int MAPI_VPROC_Deinit(MAPI_VPROC_HANDLE_T vproc_hdl);

int MAPI_VPROC_ExtChnStop(MAPI_VPROC_HANDLE_T vproc_hdl);

int MAPI_VPROC_GetGrp(MAPI_VPROC_HANDLE_T vproc_hdl);

int MAPI_VPROC_GetGrpAttr(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_GRP_ATTR_S *stGrpAttr);

int MAPI_VPROC_SetGrpAttr(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_GRP_ATTR_S *stGrpAttr);

int MAPI_VPROC_BindVcap(MAPI_VPROC_HANDLE_T vproc_hdl,
        MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t vichn_idx);
int MAPI_VPROC_UnBindVcap(MAPI_VPROC_HANDLE_T vproc_hdl,
        MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t vichn_idx);

int MAPI_VPROC_BindVproc(MAPI_VPROC_HANDLE_T vproc_src_hdl,
        uint32_t chn_idx, MAPI_VPROC_HANDLE_T vproc_dest_hdl);

int MAPI_VPROC_UnBindVproc(MAPI_VPROC_HANDLE_T vproc_src_hdl,
        uint32_t chn_idx, MAPI_VPROC_HANDLE_T vproc_dest_hdl);

int MAPI_VPROC_GetChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl,
                uint32_t chn_idx, VPSS_CHN_ATTR_S *pstChnAttr);

int MAPI_VPROC_SetChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl,
                uint32_t chn_idx, VPSS_CHN_ATTR_S *pstChnAttr);

int MAPI_VPROC_GetChnAttrEx(MAPI_VPROC_HANDLE_T vproc_hdl,uint32_t chn_idx,
            MAPI_VPROC_CMD_T enCMD, void *pAttr, uint32_t u32Len);

int MAPI_VPROC_SetChnAttrEx(MAPI_VPROC_HANDLE_T vproc_hdl,uint32_t chn_idx,
            MAPI_VPROC_CMD_T enCMD, void *pAttr, uint32_t u32Len);

int MAPI_VPROC_SendFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        VIDEO_FRAME_INFO_S *frame);
int MAPI_VPROC_GetChnFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame);
int MAPI_VPROC_SendChnFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame);

// Deprecated, use MAPI_ReleaseFrame instead
int MAPI_VPROC_ReleaseFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame);

int MAPI_VPROC_StartChnDump(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, int s32Count, MAPI_DUMP_FRAME_CALLBACK_FUNC_T *pstCallbackFun);

int MAPI_VPROC_StopChnDump(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);

int MAPI_VPROC_SetExtChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl,
        MAPI_EXTCHN_ATTR_T *pstExtChnAttr);
int MAPI_VPROC_GetExtChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, MAPI_EXTCHN_ATTR_T *pstExtChnAttr);

int MAPI_VPROC_GetExtChnGrp(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);
int MAPI_VPROC_IsExtChn(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);
int MAPI_VPROC_GetGrpCrop(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_CROP_INFO_S *pstCropInfo);
int MAPI_VPROC_SetGrpCrop(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_CROP_INFO_S *pstCropInfo);
int MAPI_VPROC_GetChnCrop(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        VPSS_CROP_INFO_S *pstCropInfo);
int MAPI_VPROC_SetChnCrop(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        VPSS_CROP_INFO_S *pstCropInfo);
int MAPI_VPROC_GetGrpChnNum(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t *pchn_num);
int MAPI_VPROC_SetChnRotation(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        ROTATION_E enRotation);
int MAPI_VPROC_GetChnRotation(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        ROTATION_E *penRotation);
int MAPI_VPROC_EnableChn(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);
int MAPI_VPROC_DisableChn(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx);
#ifdef DUAL_OS
int MAPI_VPROC_EnableTileMode(void);
int MAPI_VPROC_DisableTileMode(void);
#endif
#ifdef __cplusplus
}
#endif

#endif

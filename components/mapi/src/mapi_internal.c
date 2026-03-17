#include "mapi.h"
#include "cvi_buffer.h"
#include "mapi_internal.h"
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

CVI_S32 Vproc_Init(VPSS_GRP VpssGrp, CVI_S32 VpssChnNum, CVI_U32 *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
                VPSS_CROP_INFO_S *pstVpssCropInfo, VPSS_CHN_ATTR_S *pastVpssChnAttr, uint32_t *pAttachVbCnt,
                uint32_t *pAttachVbPool, VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap, VPSS_CROP_INFO_S *pstVpssChnCropInfo)
{
    VPSS_CHN VpssChn = 0;
    CVI_S32 s32Ret;

    s32Ret = CVI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_CreateGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        return s32Ret;
    }

    s32Ret = CVI_VPSS_ResetGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_ResetGrp(grp:%d) failed with %#x!\n", VpssGrp, s32Ret);
        goto exit1;
    }

    if (pstVpssCropInfo && pstVpssCropInfo->bEnable) {
        s32Ret = CVI_VPSS_SetGrpCrop(VpssGrp, pstVpssCropInfo);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetGrpCrop failed with %#x\n", s32Ret);
            goto exit1;
        }
    }

    for (unsigned j = 0; j < VpssChnNum; j++) {
        VpssChn = j;
        s32Ret = CVI_VPSS_SetChnAttr(VpssGrp, VpssChn, &pastVpssChnAttr[VpssChn]);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SetChnAttr failed with %#x\n", s32Ret);
            goto exit2;
        }

        if (pstVpssChnBufWrap && (pstVpssChnBufWrap[VpssChn].bEnable == CVI_TRUE)) {
            s32Ret = CVI_VPSS_SetChnBufWrapAttr(VpssGrp, VpssChn, &pstVpssChnBufWrap[VpssChn]);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("CVI_VPSS_SetChnBufWrapAttr failed with %#x\n", s32Ret);
                goto exit2;
            }
        }

        if (pstVpssChnCropInfo && (pstVpssChnCropInfo[VpssChn].bEnable == CVI_TRUE)) {
            s32Ret = CVI_VPSS_SetChnCrop(VpssGrp, VpssChn, &pstVpssChnCropInfo[VpssChn]);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("CVI_VPSS_SetChnCrop failed with %#x\n", s32Ret);
                goto exit2;
            }
        }

        if (pabChnEnable[j]) {
            s32Ret = CVI_VPSS_EnableChn(VpssGrp, VpssChn);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("VPSS_EnableChn failed with %#x\n", s32Ret);
                goto exit2;
            }
        }

        if ((pAttachVbCnt != NULL) && (pAttachVbCnt[j] > 0)) {
            VB_POOL_CONFIG_S stVbPoolCfg;
            VB_POOL chnVbPool;
            CVI_U32 u32BlkSize = 0;

            u32BlkSize = COMMON_GetPicBufferSize(pastVpssChnAttr[j].u32Width, pastVpssChnAttr[j].u32Height, pastVpssChnAttr[j].enPixelFormat,
                DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

            memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
            stVbPoolCfg.u32BlkSize	= u32BlkSize;
            stVbPoolCfg.u32BlkCnt	= pAttachVbCnt[j];
            stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
            chnVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
            if (chnVbPool == VB_INVALID_POOLID) {
                CVI_LOGE("VB_CreatePool failed.\n");
            } else {
                CVI_VPSS_AttachVbPool(VpssGrp, j, chnVbPool);

                pAttachVbPool[j] = chnVbPool; // save the pool id for later use
            }
        }
    }

    s32Ret = CVI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_StartGrp failed with %#x\n", s32Ret);
        goto exit2;
    }
    return CVI_SUCCESS;

exit2:
    for(signed j = 0; j < VpssChn; j++){
        if (CVI_VPSS_DisableChn(VpssGrp, j) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_DisableChn failed!\n");
        }
    }
exit1:
    if (CVI_VPSS_DestroyGrp(VpssGrp) != CVI_SUCCESS) {
        CVI_LOGE("VPSS_DestroyGrp(grp:%d) failed!\n", VpssGrp);
    }

    return s32Ret;
}

CVI_VOID Vproc_Deinit(VPSS_GRP VpssGrp, CVI_S32 VpssChnNum, uint32_t *pAttachVbCnt, uint32_t *pAttachVbPool)
{
    CVI_S32 j;
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPSS_CHN VpssChn;

    for (j = 0; j < VpssChnNum; j++) {
        VpssChn = j;

        s32Ret = CVI_VPSS_DisableChn(VpssGrp, VpssChn);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("failed with %#x!\n", s32Ret);
        }

        if ((pAttachVbCnt[j] > 0) && pAttachVbPool[j] != VB_INVALID_POOLID) {
            s32Ret = CVI_VB_DestroyPool(pAttachVbPool[j]);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("failed with %#x!\n", s32Ret);
            }
            pAttachVbPool[j] = VB_INVALID_POOLID; // reset to invalid pool id
        }
    }
    s32Ret = CVI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("failed with %#x!\n", s32Ret);
    }
    s32Ret = CVI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("failed with %#x!\n", s32Ret);
    }
}

uint32_t get_frame_size(uint32_t w, uint32_t h, PIXEL_FORMAT_E fmt) {
    // try rotate and non-rotate, choose the larger one
    uint32_t sz_0 = COMMON_GetPicBufferSize(w, h, fmt,
        DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    // uint32_t sz_1 = COMMON_GetPicBufferSize(h, w, fmt,
    //     DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    // return (sz_0 > sz_1) ? sz_0 : sz_1;
    return sz_0;
}
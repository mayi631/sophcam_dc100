#ifndef __MAPI_INTERNAL_H__
#define __MAPI_INTERNAL_H__

// #include "sample_comm.h"
#include "cvi_vpss.h"
#define MAPI_ALIGN(value, base) (((value)+(base) - 1)/(base)*(base))

CVI_S32 Vproc_Init(VPSS_GRP VpssGrp, CVI_S32 VpssChnNum, CVI_U32 *pabChnEnable, VPSS_GRP_ATTR_S *pstVpssGrpAttr,
    VPSS_CROP_INFO_S *pstVpssCropInfo, VPSS_CHN_ATTR_S *pastVpssChnAttr, uint32_t *pAttachVbCnt,
    uint32_t *pAttachVbPool, VPSS_CHN_BUF_WRAP_S *pstVpssChnBufWrap, VPSS_CROP_INFO_S *pstVpssChnCropInfo);
CVI_VOID Vproc_Deinit(VPSS_GRP VpssGrp, CVI_S32 VpssChnNum, uint32_t *pAttachVbCnt, uint32_t *pAttachVbPool);
uint32_t get_frame_size(uint32_t w, uint32_t h, PIXEL_FORMAT_E fmt);
#endif

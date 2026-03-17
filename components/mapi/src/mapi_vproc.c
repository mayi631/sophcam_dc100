#include <stdlib.h>
// #include "cvi_common.h"
#include "cvi_comm_vpss.h"
// #include "sample_comm.h"
// #include "cvi_common.h"
#include "cvi_buffer.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_comm_sns.h"
#include "cvi_comm_vi.h"
#include "cvi_comm_vpss.h"
#include "cvi_comm_vo.h"
#include "cvi_comm_venc.h"
#include "cvi_comm_vdec.h"
#include "cvi_comm_region.h"
#include "cvi_comm_adec.h"
#include "cvi_comm_aenc.h"
#include "cvi_comm_aio.h"
#include "cvi_audio.h"
// #include "cvi_defines.h"

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
//#define CVI_LOG_LEVEL CVI_LOG_VERBOSE
#include "cvi_log.h"

#include "cvi_comm_video.h"
#include "cvi_comm_vpss.h"
#include "cvi_comm_sys.h"

#include "mapi.h"
#include "mapi_internal.h"

#define MAX_VPSS_GRP_NUM    (16)
static pthread_mutex_t vproc_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_grp_used[MAX_VPSS_GRP_NUM] = {0};

#define CHECK_VPROC_GRP(grp) do { \
        if (grp >= MAX_VPSS_GRP_NUM) { \
            CVI_LOGE("VprocGrp(%d) exceeds Max(%d)\n", grp, MAX_VPSS_GRP_NUM); \
            return MAPI_ERR_INVALID; \
        } \
    } while (0)

#define CHECK_VPROC_CHN(chn) do { \
    if (chn >= MAPI_VPROC_MAX_CHN_NUM) { \
        CVI_LOGE("VprocGrp(%d) exceeds Max(%d)\n", chn, MAPI_VPROC_MAX_CHN_NUM); \
        return MAPI_ERR_INVALID; \
    } \
} while (0)

#define VPROC_CHECK_NULL_PTR(ptr) \
        do { \
            if (!(ptr)) { \
                CVI_LOGE(" NULL pointer\n"); \
                return MAPI_ERR_INVALID; \
            } \
        } while (0)

int find_valid_vpss_grp(bool ExtChnEn)
{
    int i = 0;
    int grp_id = -1;

    pthread_mutex_lock(&vproc_mutex);
    // find a valid grp slot
    VI_VPSS_MODE_S stVIVPSSMode;
    CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
    if ((stVIVPSSMode.aenMode[0] == VI_OFFLINE_VPSS_ONLINE) ||
        (stVIVPSSMode.aenMode[0] == VI_ONLINE_VPSS_ONLINE) ||
        (ExtChnEn == true)) {
#ifdef MAX_CAMERA_INSTANCES
            i = MAX_CAMERA_INSTANCES;
#endif
    }

    for (; i < MAX_VPSS_GRP_NUM; i++) {
        if (!g_grp_used[i]) {
            break;
        }
    }
    if (i >= MAX_VPSS_GRP_NUM) {
        CVI_LOGE("no empty grp_id left\n");
        pthread_mutex_unlock(&vproc_mutex);
        return grp_id;
    } else {
        grp_id = i;
    }

    // g_grp_used[grp_id] = true;
    pthread_mutex_unlock(&vproc_mutex);

    return grp_id;
}

int MAPI_VPROC_Init(MAPI_VPROC_HANDLE_T *vproc_hdl,
        int grp_id, MAPI_VPROC_ATTR_T *attr)
{
    VPROC_CHECK_NULL_PTR(attr);
    VPROC_CHECK_NULL_PTR(vproc_hdl);

    if(attr->chn_num > MAPI_VPROC_MAX_CHN_NUM){
        CVI_LOGE("attr->chn_num = %d, Exceed the maximum %d\n", attr->chn_num, MAPI_VPROC_MAX_CHN_NUM);
        return MAPI_ERR_INVALID;
    }

    if (grp_id == -1)
        grp_id = find_valid_vpss_grp(false);

    if (grp_id >= 0 && grp_id < MAX_VPSS_GRP_NUM) {
        pthread_mutex_lock(&vproc_mutex);
        if (g_grp_used[grp_id]) {
            CVI_LOGE("grp_id %d has been used\n", grp_id);
            pthread_mutex_unlock(&vproc_mutex);
            return MAPI_ERR_INVALID;
        }
        g_grp_used[grp_id] = true;
        pthread_mutex_unlock(&vproc_mutex);
    } else {
        CVI_LOGE("Invalid grp_id %d\n", grp_id);
        return MAPI_ERR_INVALID;
    }

    CVI_LOGI("Create VPROC with vpss grp id %d chn_num:%d\n", grp_id,attr->chn_num);

    CVI_S32 rc = MAPI_SUCCESS;
    /*start vpss*/
    rc = Vproc_Init(grp_id, attr->chn_num, attr->chn_enable, &attr->attr_inp,
                    &attr->crop_info, attr->attr_chn, attr->chn_vbcnt,
                    attr->chn_bindVbPoolID, attr->chn_bufWrap, attr->chn_cropInfo);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("Vproc_Init failed. rc: 0x%x !\n", rc);
        rc = MAPI_ERR_FAILURE;
        goto err1;
    }

    // for (int i = 0; i < attr->chn_num; i++) {
    //     // if (attr->lowdelay_cnt[i] > 0) {                // disable lowdelay for mars not support temporarilly
    //     //     VPSS_LOW_DELAY_INFO_S stLowDelayInfo;

    //     //     stLowDelayInfo.bEnable = CVI_TRUE;
    //     //     stLowDelayInfo.u32LineCnt = attr->lowdelay_cnt[i];
    //     //     CVI_VPSS_SetLowDelayAttr(VpssGrp, i, &stLowDelayInfo);
    //     // }
    //     if (attr->chn_vbcnt[i] > 0) {
    //         VB_POOL_CONFIG_S stVbPoolCfg;
    //         VB_POOL chnVbPool;
    //         CVI_U32 u32BlkSize = 0;

    //         u32BlkSize = COMMON_GetPicBufferSize(attr->attr_chn[i].u32Width, attr->attr_chn[i].u32Height, attr->attr_chn[i].enPixelFormat,
    //             DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

    //         memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
    //         stVbPoolCfg.u32BlkSize	= u32BlkSize;
    //         stVbPoolCfg.u32BlkCnt	= attr->chn_vbcnt[i];
    //         stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
    //         chnVbPool = CVI_VB_CreatePool(&stVbPoolCfg);
    //         if (chnVbPool == VB_INVALID_POOLID) {
    //             CVI_LOGE("CVI_VB_CreatePool failed.\n");
    //         } else {
    //             CVI_VPSS_AttachVbPool(VpssGrp, i, chnVbPool);
    //         }
    //     }
    // }

    MAPI_VPROC_CTX_T *pt;
    pt = (MAPI_VPROC_CTX_T *)malloc(sizeof(MAPI_VPROC_CTX_T));
    if (!pt) {
        CVI_LOGE("malloc failed\n");
        rc = MAPI_ERR_NOMEM;
        goto err2;
    }
    memset(pt, 0, sizeof(MAPI_VPROC_CTX_T));
    pt->VpssGrp = grp_id;
    for (int i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        pt->abChnEnable[i] = attr->chn_enable[i];
    }
    pt->attr = *attr;

    *vproc_hdl = (MAPI_VPROC_HANDLE_T)pt;
    return MAPI_SUCCESS;

err2:
    Vproc_Deinit(grp_id, attr->chn_num, attr->chn_vbcnt, attr->chn_bindVbPoolID);

err1:
    pthread_mutex_lock(&vproc_mutex);
    g_grp_used[grp_id] = false;
    pthread_mutex_unlock(&vproc_mutex);

    return rc;
}

int MAPI_VPROC_Deinit(MAPI_VPROC_HANDLE_T vproc_hdl)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    int grp_id = pt->VpssGrp;

    pthread_mutex_lock(&vproc_mutex);
    g_grp_used[grp_id] = false;
    pthread_mutex_unlock(&vproc_mutex);

    CVI_LOGI("Destroy VPROC with vpss grp id %d\n", grp_id);
    int i = 0;
    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if (pt->ExtChn[i].ExtChnEnable == true) {
            MMF_CHN_S stSrcChn;
            MMF_CHN_S stDestChn;

            stSrcChn.enModId = CVI_ID_VPSS;
            stSrcChn.s32DevId = pt->VpssGrp;
            stSrcChn.s32ChnId = pt->ExtChn[i].ExtChnAttr.BindVprocChnId;

            stDestChn.enModId = CVI_ID_VPSS;
            stDestChn.s32DevId = pt->ExtChn[i].ExtChnGrp;
            stDestChn.s32ChnId = 0;

            CVI_S32 rc = CVI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (rc != CVI_SUCCESS) {
                CVI_LOGE("SYS_UnBind, rc: 0x%x !\n", rc);
                return MAPI_ERR_FAILURE;
            }

        }
    }

    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if (pt->ExtChn[i].ExtChnEnable == true) {
            CVI_BOOL ChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
            ChnEnable[0] = true;
            g_grp_used[pt->ExtChn[i].ExtChnGrp] = false;
            Vproc_Deinit(pt->ExtChn[i].ExtChnGrp, 1, pt->ExtChn[i].ExtChnAttr.chn_vbcnt, pt->ExtChn[i].ExtChnAttr.chn_bindVbPoolID);
        }
    }

    Vproc_Deinit(pt->VpssGrp, pt->attr.chn_num, pt->attr.chn_vbcnt, pt->attr.chn_bindVbPoolID);
    free(pt);

    return MAPI_SUCCESS;
}

int MAPI_VPROC_ExtChnStop(MAPI_VPROC_HANDLE_T vproc_hdl)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    int grp_id = pt->VpssGrp;

    CVI_LOGI("Destroy VPROC ext chn with vpss grp id %d\n", grp_id);
    UNUSED(grp_id);
    int i = VPSS_MAX_PHY_CHN_NUM - 1;
    for(; i >= 0; i--) {
        if (pt->ExtChn[i].ExtChnEnable == true) {
            //suport ext chn bind ext chn
            bool ExtchnBindChn = false;
            int j = 0;
            for (j = 0; j < VPSS_MAX_PHY_CHN_NUM; j++) {
                // CVI_LOGE("pt->ExtChn[i].ExtChnAttr.BindVprocChnId == %d\n", pt->ExtChn[i].ExtChnAttr.BindVprocChnId);
                // CVI_LOGE("pt->ExtChn[j].ExtChnAttr.ChnId == %d\n", pt->ExtChn[j].ExtChnAttr.ChnId);
                if ((pt->ExtChn[j].ExtChnEnable == true)&&\
                    (pt->ExtChn[i].ExtChnAttr.BindVprocChnId == pt->ExtChn[j].ExtChnAttr.ChnId)) {
                    ExtchnBindChn = true;
                    break;
                }
            }
            MMF_CHN_S stSrcChn;
            MMF_CHN_S stDestChn;
            if (ExtchnBindChn == true) {
                // CVI_LOGE("pt->ExtChn[j].ExtChnGrp == %d\n", pt->ExtChn[j].ExtChnGrp);
                stSrcChn.enModId = CVI_ID_VPSS;
                stSrcChn.s32DevId = pt->ExtChn[j].ExtChnGrp;
                stSrcChn.s32ChnId = 0;
            } else {
                stSrcChn.enModId = CVI_ID_VPSS;
                stSrcChn.s32DevId = pt->VpssGrp;
                stSrcChn.s32ChnId = pt->ExtChn[i].ExtChnAttr.BindVprocChnId;
            }
            // CVI_LOGE("pt->ExtChn[i].ExtChnGrp == %d\n", pt->ExtChn[i].ExtChnGrp);
            stDestChn.enModId = CVI_ID_VPSS;
            stDestChn.s32DevId = pt->ExtChn[i].ExtChnGrp;
            stDestChn.s32ChnId = 0;

            CVI_S32 rc = CVI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (rc != CVI_SUCCESS) {
                CVI_LOGE("SYS_UnBind, rc: 0x%x !\n", rc);
                return MAPI_ERR_FAILURE;
            }
        }
    }

    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if (pt->ExtChn[i].ExtChnEnable == true) {
            CVI_BOOL ChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
            ChnEnable[0] = true;
            g_grp_used[pt->ExtChn[i].ExtChnGrp] = false;
            Vproc_Deinit(pt->ExtChn[i].ExtChnGrp, 1, pt->ExtChn[i].ExtChnAttr.chn_vbcnt, pt->ExtChn[i].ExtChnAttr.chn_bindVbPoolID);
            pt->ExtChn[i].ExtChnEnable = false;
        }
    }

    return MAPI_SUCCESS;
}


int MAPI_VPROC_GetGrp(MAPI_VPROC_HANDLE_T vproc_hdl)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    return pt->VpssGrp;
}

int MAPI_VPROC_GetGrpAttr(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_GRP_ATTR_S *stGrpAttr)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(stGrpAttr);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    *stGrpAttr = pt->attr.attr_inp;
    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetGrpAttr(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_GRP_ATTR_S *stGrpAttr)
{
    CVI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(stGrpAttr);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    s32Ret = CVI_VPSS_SetGrpAttr(pt->VpssGrp, stGrpAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_SetGrpAttr failed with %#x\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }

    pt->attr.attr_inp = *stGrpAttr;
    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetGrpCrop(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_CROP_INFO_S *pstCropInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(pstCropInfo);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_GetGrpCrop(pt->VpssGrp, pstCropInfo);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_GetGrpCrop failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetGrpCrop(MAPI_VPROC_HANDLE_T vproc_hdl, VPSS_CROP_INFO_S *pstCropInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(pstCropInfo);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_SetGrpCrop(pt->VpssGrp, pstCropInfo);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_SetGrpCrop failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetChnCrop(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        VPSS_CROP_INFO_S *pstCropInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(pstCropInfo);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_GetChnCrop(pt->VpssGrp, chn_idx, pstCropInfo);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_GetChnCrop failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetChnCrop(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        VPSS_CROP_INFO_S *pstCropInfo)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(pstCropInfo);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_SetChnCrop(pt->VpssGrp, chn_idx, pstCropInfo);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_SetChnCrop failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetChnRotation(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        ROTATION_E enRotation)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_SetChnRotation(pt->VpssGrp, chn_idx, enRotation);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_SetChnRotation failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetChnRotation(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
                        ROTATION_E *penRotation)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(penRotation);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_GetChnRotation(pt->VpssGrp, chn_idx, penRotation);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_GetChnRotation failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetGrpChnNum(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t *pchn_num)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    VPROC_CHECK_NULL_PTR(pchn_num);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    *pchn_num = pt->attr.chn_num;
    return MAPI_SUCCESS;
}

int MAPI_VPROC_BindVcap(MAPI_VPROC_HANDLE_T vproc_hdl,
        MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t vichn_idx)
{
    (void)sns_hdl;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;

    stSrcChn.enModId = CVI_ID_VI;
    #ifdef CHIP_184X
    stSrcChn.s32DevId = vichn_idx;
    stSrcChn.s32ChnId = 0;
    #else
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = vichn_idx;//MAPI_VCAP_GetSensorChn(sns_hdl);
    #endif

    stDestChn.enModId = CVI_ID_VPSS;
    stDestChn.s32DevId = pt->VpssGrp;
    stDestChn.s32ChnId = 0;

    CVI_LOGE("stSrcChn.s32ChnId %d stDestChn.s32DevId %d", stSrcChn.s32ChnId, stDestChn.s32DevId);

    CVI_S32 rc = CVI_SYS_Bind(&stSrcChn, &stDestChn);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("SYS_Bind, rc: 0x%x !\n", rc);
        return MAPI_ERR_FAILURE;
    }

    return MAPI_SUCCESS;
}

int MAPI_VPROC_UnBindVcap(MAPI_VPROC_HANDLE_T vproc_hdl,
        MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, uint32_t vichn_idx)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    MMF_CHN_S stSrcChn;
    MMF_CHN_S stDestChn;

    stSrcChn.enModId = CVI_ID_VI;
    #ifdef CHIP_184X
    stSrcChn.s32DevId = vichn_idx;
    stSrcChn.s32ChnId = 0;
    #else
    stSrcChn.s32DevId = MAPI_VCAP_GetSensorPipe(sns_hdl);
    stSrcChn.s32ChnId = vichn_idx;//MAPI_VCAP_GetSensorChn(sns_hdl);
    #endif

    stDestChn.enModId = CVI_ID_VPSS;
    stDestChn.s32DevId = pt->VpssGrp;
    stDestChn.s32ChnId = 0;

    CVI_S32 rc = CVI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("SYS_UnBind, rc: 0x%x !\n", rc);
        return MAPI_ERR_FAILURE;
    }

    return MAPI_SUCCESS;
}

int MAPI_VPROC_BindVproc(MAPI_VPROC_HANDLE_T vproc_src_hdl,
		uint32_t chn_idx, MAPI_VPROC_HANDLE_T vproc_dest_hdl)
{
	VPROC_CHECK_NULL_PTR(vproc_src_hdl);
	VPROC_CHECK_NULL_PTR(vproc_dest_hdl);
	CHECK_VPROC_CHN(chn_idx);
	MAPI_VPROC_CTX_T *pt_src = (MAPI_VPROC_CTX_T *)vproc_src_hdl;
	MAPI_VPROC_CTX_T *pt_dest = (MAPI_VPROC_CTX_T *)vproc_dest_hdl;
	CHECK_VPROC_GRP(pt_src->VpssGrp);
	CHECK_VPROC_GRP(pt_dest->VpssGrp);

	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;

	stSrcChn.enModId = CVI_ID_VPSS;
	stSrcChn.s32DevId = pt_src->VpssGrp;
	stSrcChn.s32ChnId = chn_idx;

	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = pt_dest->VpssGrp;
	stDestChn.s32ChnId = 0;

	CVI_S32 rc = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (rc != CVI_SUCCESS) {
		CVI_LOGE("SYS_Bind, rc: 0x%x !\n", rc);
		return MAPI_ERR_FAILURE;
	}

	return MAPI_SUCCESS;
}

int MAPI_VPROC_UnBindVproc(MAPI_VPROC_HANDLE_T vproc_src_hdl,
		uint32_t chn_idx, MAPI_VPROC_HANDLE_T vproc_dest_hdl)
{
	VPROC_CHECK_NULL_PTR(vproc_src_hdl);
	VPROC_CHECK_NULL_PTR(vproc_dest_hdl);
	CHECK_VPROC_CHN(chn_idx);
	MAPI_VPROC_CTX_T *pt_src = (MAPI_VPROC_CTX_T *)vproc_src_hdl;
	MAPI_VPROC_CTX_T *pt_dest = (MAPI_VPROC_CTX_T *)vproc_dest_hdl;
	CHECK_VPROC_GRP(pt_src->VpssGrp);
	CHECK_VPROC_GRP(pt_dest->VpssGrp);

	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;

	stSrcChn.enModId = CVI_ID_VPSS;
	stSrcChn.s32DevId = pt_src->VpssGrp;
	stSrcChn.s32ChnId = chn_idx;

	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = pt_dest->VpssGrp;
	stDestChn.s32ChnId = 0;

	CVI_S32 rc = CVI_SYS_UnBind(&stSrcChn, &stDestChn);
	if (rc != CVI_SUCCESS) {
		CVI_LOGE("SYS_UnBind, rc: 0x%x !\n", rc);
		return MAPI_ERR_FAILURE;
	}

	return MAPI_SUCCESS;
}


int MAPI_VPROC_SendFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        VIDEO_FRAME_INFO_S *frame)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    VPROC_CHECK_NULL_PTR(frame);

    CVI_S32 rc = CVI_VPSS_SendFrame(pt->VpssGrp, frame, MAPI_VPROC_TIMEOUT_MS);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("VPSS_SendFrame failed, rc: 0x%x ! GrpId :%d \n", rc, pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl,
                uint32_t chn_idx, VPSS_CHN_ATTR_S *pstChnAttr)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(pstChnAttr);

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    *pstChnAttr = pt->attr.attr_chn[chn_idx];

    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl,
                uint32_t chn_idx, VPSS_CHN_ATTR_S *pstChnAttr)
{
    CVI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(pstChnAttr);

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }
    s32Ret = CVI_VPSS_SetChnAttr(pt->VpssGrp, chn_idx, pstChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_SetChnAttr failed with %#x\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }

    pt->attr.attr_chn[chn_idx] = *pstChnAttr;

    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetChnAttrEx(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx,
            MAPI_VPROC_CMD_T enCMD, void *pAttr, uint32_t u32Len)
{
    CVI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(pAttr);
    int grp_id = pt->VpssGrp;

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    switch(enCMD){
        case MAPI_VPROC_CMD_CHN_ROTATE:
            if(u32Len < sizeof(ROTATION_E)){
                CVI_LOGE("VPROC pAttr u32Len:%x is small\n", u32Len);
                return MAPI_ERR_FAILURE;
            }
            s32Ret = CVI_VPSS_GetChnRotation(grp_id, chn_idx, (ROTATION_E *)pAttr);
            if(s32Ret != CVI_SUCCESS){
                CVI_LOGE("VPSS_GetChnRotation failed\n");
                return MAPI_ERR_FAILURE;
            }
            break;
        case MAPI_VPROC_CMD_CHN_CROP:
            if(u32Len < sizeof(VPSS_CROP_INFO_S)){
                CVI_LOGE("VPROC pAttr u32Len:%x is small\n", u32Len);
                return MAPI_ERR_FAILURE;
            }
            s32Ret = CVI_VPSS_GetChnCrop(grp_id, chn_idx, (VPSS_CROP_INFO_S *)pAttr);
            if(s32Ret != CVI_SUCCESS){
                CVI_LOGE("VPSS_GetChnCrop failed\n");
                return MAPI_ERR_FAILURE;
            }
            break;
        default:
            break;
    }

    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetChnAttrEx(MAPI_VPROC_HANDLE_T vproc_hdl,uint32_t chn_idx,
            MAPI_VPROC_CMD_T enCMD, void *pAttr, uint32_t u32Len)
{
    CVI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(pAttr);
    CVI_U32 grp_id = pt->VpssGrp;

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    switch(enCMD){
        case MAPI_VPROC_CMD_CHN_ROTATE:
            if(u32Len < sizeof(ROTATION_E)){
                CVI_LOGE("VPROC pAttr u32Len:%x is small\n", u32Len);
                return MAPI_ERR_FAILURE;
            }
            s32Ret = CVI_VPSS_SetChnRotation(grp_id, chn_idx, *((ROTATION_E *)pAttr));
            if(s32Ret != CVI_SUCCESS){
                CVI_LOGE("VPSS_SetChnRotation failed\n");
                return MAPI_ERR_FAILURE;
            }
            break;
        case MAPI_VPROC_CMD_CHN_CROP:
            if(u32Len < sizeof(VPSS_CROP_INFO_S)){
                CVI_LOGE("VPROC pAttr u32Len:%x is small\n", u32Len);
                return MAPI_ERR_FAILURE;
            }
            s32Ret = CVI_VPSS_SetChnCrop(grp_id, chn_idx, (VPSS_CROP_INFO_S *)pAttr);
            if(s32Ret != CVI_SUCCESS){
                CVI_LOGE("VPSS_SetChnCrop failed\n");
                return MAPI_ERR_FAILURE;
            }
            break;
        default:
            break;
    }

    return MAPI_SUCCESS;
}


int MAPI_VPROC_GetChnFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    // CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(frame);
    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }
    int i = 0;
    CVI_BOOL ExtChnEn = false;
    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if ((pt->ExtChn[i].ExtChnEnable == true) && (pt->ExtChn[i].ExtChnAttr.ChnId == chn_idx)) {
            ExtChnEn = true;
            break;
        }
    }
    if (ExtChnEn == true) {
        if (CVI_VPSS_GetChnFrame(pt->ExtChn[i].ExtChnGrp, 0, frame, MAPI_VPROC_TIMEOUT_MS) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_GetChnFrame failed\n");
            return MAPI_ERR_FAILURE;
        }
    } else {
        CHECK_VPROC_CHN(chn_idx);
        if (CVI_VPSS_GetChnFrame(pt->VpssGrp, chn_idx, frame, MAPI_VPROC_TIMEOUT_MS) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_GetChnFrame failed\n");
            return MAPI_ERR_FAILURE;
        }
    }

    return MAPI_SUCCESS;
}

int MAPI_VPROC_SendChnFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    // CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(frame);
    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }
    int i = 0;
    CVI_BOOL ExtChnEn = false;
    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if ((pt->ExtChn[i].ExtChnEnable == true) && (pt->ExtChn[i].ExtChnAttr.ChnId == chn_idx)) {
            ExtChnEn = true;
            break;
        }
    }
    if (ExtChnEn == true) {
        if (CVI_VPSS_SendChnFrame(pt->ExtChn[i].ExtChnGrp, 0, frame, -1) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SendChnFrame failed\n");
            return MAPI_ERR_FAILURE;
        }
    } else {
        CHECK_VPROC_CHN(chn_idx);
        if (CVI_VPSS_SendChnFrame(pt->VpssGrp, chn_idx, frame, -1) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_SendChnFrame failed\n");
            return MAPI_ERR_FAILURE;
        }
    }

    return MAPI_SUCCESS;
}

int MAPI_VPROC_ReleaseFrame(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, VIDEO_FRAME_INFO_S *frame)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    // CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(frame);
    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }
    int i = 0;
    CVI_BOOL ExtChnEn = false;
    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if ((pt->ExtChn[i].ExtChnEnable == true) && (pt->ExtChn[i].ExtChnAttr.ChnId == chn_idx)) {
            ExtChnEn = true;
            break;
        }
    }
    if (ExtChnEn == true) {
        if (CVI_VPSS_ReleaseChnFrame(pt->ExtChn[i].ExtChnGrp, 0, frame) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_ReleaseChnFrame failed\n");
            return MAPI_ERR_FAILURE;
        }
    } else {
        CHECK_VPROC_CHN(chn_idx);
        if (CVI_VPSS_ReleaseChnFrame(pt->VpssGrp, chn_idx, frame) != CVI_SUCCESS) {
            CVI_LOGE("VPSS_ReleaseChnFrame failed\n");
            return MAPI_ERR_FAILURE;
        }
    }

    return MAPI_SUCCESS;
}

int VpssDumpFrame(CVI_U32 Grp, CVI_S32 Chn, VIDEO_FRAME_INFO_S* pstFrameData,
        MAPI_DUMP_FRAME_CALLBACK_FUNC_T* pstCallBack)
{
    if (CVI_VPSS_GetChnFrame(Grp, Chn, pstFrameData, MAPI_VPROC_TIMEOUT_MS) != CVI_SUCCESS) {
        CVI_LOGE("VPSS_GetChnFrame failed\n");
        return MAPI_ERR_FAILURE;
    }

    pstCallBack->pfunFrameProc(Grp, Chn, pstFrameData, pstCallBack->pPrivateData);

    if (CVI_VPSS_ReleaseChnFrame(Grp, Chn, pstFrameData) != CVI_SUCCESS) {
        CVI_LOGE("VPSS_ReleaseChnFrame failed\n");
    }
    return MAPI_SUCCESS;
}

void *VpssDumpFramethread(void *pArg)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U32 Grp;
    CVI_U32 Chn;
    CVI_S32 s32Count;
    VPROC_DUMP_CTX_T *pstArg;
    VIDEO_FRAME_INFO_S stFrameData = {0};

    pstArg = (VPROC_DUMP_CTX_T*)pArg;
    Grp = pstArg->Grp;
    Chn = pstArg->Chn;
    s32Count = pstArg->s32Count;
    //prctl(PR_SET_NAME, (unsigned long)"VprocDumpFrame", 0, 0, 0);

    if (s32Count >= 0) {
        for (int i = 0; i < s32Count; i++) {
            s32Ret = VpssDumpFrame(Grp, Chn, &stFrameData, &pstArg->stCallbackFun);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("Vpss Dump Frame %d count fail s32Ret:%x\n", i, s32Ret);
                continue;
            }
        }
    } else if (-1 == s32Count) {
        while (pstArg->bStart) {
            s32Ret = VpssDumpFrame(Grp, Chn, &stFrameData, &pstArg->stCallbackFun);
            if (s32Ret != CVI_SUCCESS) {
                CVI_LOGE("Vpss Dump Frame fail s32Ret:%x\n", s32Ret);
                continue;
            }
        }

    } else {
        CVI_LOGE("Vpss Dump Frame input count is unsupport.\n");
        return CVI_NULL;
    }

    return CVI_NULL;
}

int MAPI_VPROC_StartChnDump(MAPI_VPROC_HANDLE_T vproc_hdl,
        uint32_t chn_idx, int s32Count, MAPI_DUMP_FRAME_CALLBACK_FUNC_T *pstCallbackFun)
{
    CVI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    CHECK_VPROC_CHN(chn_idx);
    VPROC_CHECK_NULL_PTR(pstCallbackFun);
    VPROC_CHECK_NULL_PTR(pstCallbackFun->pfunFrameProc);

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }
    if (s32Count == 0) {
        CVI_LOGE("Vproc Dump Frame input is ereor.\n");
        return MAPI_ERR_FAILURE;
    }
    if (pt->stVprocDumpCtx.bStart) {
        CVI_LOGE("Vpss Dump Frame is busy,please wait.\n");
        return MAPI_ERR_FAILURE;
    }
    pt->stVprocDumpCtx.bStart = CVI_TRUE;
    pt->stVprocDumpCtx.Grp = pt->VpssGrp;
    pt->stVprocDumpCtx.Chn = chn_idx;
    pt->stVprocDumpCtx.s32Count = s32Count;
    pt->stVprocDumpCtx.stCallbackFun = *pstCallbackFun;


    s32Ret = pthread_create(&pt->stVprocDumpCtx.pthreadDump, CVI_NULL,
                            VpssDumpFramethread, (void *)&pt->stVprocDumpCtx);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("Create Vproc dump frame thread error.\n");
        pt->stVprocDumpCtx.pthreadDump = 0;
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_StopChnDump(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx)
{
    CVI_S32 s32Ret;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    CHECK_VPROC_CHN(chn_idx);
    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    if (!pt->stVprocDumpCtx.bStart) {
        CVI_LOGE("Vproc dump frame has not been enabled.\n");
        return MAPI_ERR_FAILURE;
    }
    pt->stVprocDumpCtx.bStart = CVI_FALSE;

    if((long)pt->stVprocDumpCtx.pthreadDump != -1)
    {
        s32Ret = pthread_join(pt->stVprocDumpCtx.pthreadDump, CVI_NULL);
        if (s32Ret != CVI_SUCCESS) {
            CVI_LOGE("pthread_join failed.\n");
        }
    }

    pt->stVprocDumpCtx.pthreadDump = 0;
    pt->stVprocDumpCtx.stCallbackFun.pfunFrameProc = CVI_NULL;
    pt->stVprocDumpCtx.stCallbackFun.pPrivateData = CVI_NULL;

    return MAPI_SUCCESS;
}

int MAPI_VPROC_SetExtChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl, MAPI_EXTCHN_ATTR_T *pstExtChnAttr)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    // CHECK_VPROC_CHN(chn_idx);
    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }

    int grp_id = find_valid_vpss_grp(true);
    if (grp_id >= 0 && grp_id < MAX_VPSS_GRP_NUM) {
        pthread_mutex_lock(&vproc_mutex);
        if (g_grp_used[grp_id]) {
            CVI_LOGE("grp_id %d has been used\n", grp_id);
            pthread_mutex_unlock(&vproc_mutex);
            return MAPI_ERR_INVALID;
        }
        g_grp_used[grp_id] = true;
        pthread_mutex_unlock(&vproc_mutex);
    } else {
        CVI_LOGE("Invalid grp_id %d\n", grp_id);
        return MAPI_ERR_INVALID;
    }
    int i = 0;
    for (; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if (!pt->ExtChn[i].ExtChnEnable) {
            break;
        }
    }

    MAPI_VPROC_ATTR_T attr;
    memset((void*)&attr, 0, sizeof(attr));

    //suport ext chn bind ext chn
    bool ExtchnBindChn = false;
    int j = 0;
    for (j = 0; j < i; j++) {
        if (pt->ExtChn[j].ExtChnAttr.ChnId == pstExtChnAttr->BindVprocChnId) {
            ExtchnBindChn = true;
            break;
        }
    }

    attr.attr_inp.stFrameRate.s32SrcFrameRate    = -1;
    attr.attr_inp.stFrameRate.s32DstFrameRate    = -1;
    if (ExtchnBindChn == true) {
        attr.attr_inp.enPixelFormat                  = pt->ExtChn[j].ExtChnAttr.VpssChnAttr.enPixelFormat;
        attr.attr_inp.u32MaxW                        = pt->ExtChn[j].ExtChnAttr.VpssChnAttr.u32Width;
        attr.attr_inp.u32MaxH                        = pt->ExtChn[j].ExtChnAttr.VpssChnAttr.u32Height;
    } else {
        attr.attr_inp.enPixelFormat                  = pt->attr.attr_chn[pstExtChnAttr->BindVprocChnId].enPixelFormat;
        attr.attr_inp.u32MaxW                        = pt->attr.attr_chn[pstExtChnAttr->BindVprocChnId].u32Width;
        attr.attr_inp.u32MaxH                        = pt->attr.attr_chn[pstExtChnAttr->BindVprocChnId].u32Height;
    }

    attr.attr_inp.u8VpssDev                   = 0;
    attr.chn_num = 1;
    memcpy(&attr.attr_chn[0], &pstExtChnAttr->VpssChnAttr, sizeof(VPSS_CHN_ATTR_S));
    VPSS_GRP VpssGrp = grp_id;
    for (int i = 0; i < attr.chn_num; i++) {
        attr.chn_vbcnt[i] = pstExtChnAttr->chn_vbcnt[i];
    }

    CVI_S32 rc = MAPI_SUCCESS;
    /*start vpss*/
    rc = Vproc_Init(VpssGrp, attr.chn_num, attr.chn_enable, &attr.attr_inp, NULL,
                    attr.attr_chn, pstExtChnAttr->chn_vbcnt,
                    pstExtChnAttr->chn_bindVbPoolID, NULL, NULL);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("Vproc_Init failed. rc: 0x%x !\n", rc);
        rc = MAPI_ERR_FAILURE;
        goto err1;
    }
    pt->ExtChn[i].ExtChnGrp = VpssGrp;
    // CVI_LOGE("pt->ExtChn[i].ExtChnGrp == %d, %d\n", pt->ExtChn[i].ExtChnGrp, i);
    memcpy(&pt->ExtChn[i].ExtChnAttr, pstExtChnAttr, sizeof(MAPI_EXTCHN_ATTR_T));
    pt->ExtChn[i].ExtChnEnable = true;

    MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
    if (ExtchnBindChn == true) {
        stSrcChn.enModId = CVI_ID_VPSS;
        stSrcChn.s32DevId = pt->ExtChn[j].ExtChnGrp;
        stSrcChn.s32ChnId = 0;
    } else {
        stSrcChn.enModId = CVI_ID_VPSS;
        stSrcChn.s32DevId = pt->VpssGrp;
        stSrcChn.s32ChnId = pstExtChnAttr->BindVprocChnId;
    }

	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = VpssGrp;
	stDestChn.s32ChnId = 0;

	rc = CVI_SYS_Bind(&stSrcChn, &stDestChn);
	if (rc != CVI_SUCCESS) {
		CVI_LOGE("SYS_Bind, rc: 0x%x !\n", rc);
		return MAPI_ERR_FAILURE;
	}
    return MAPI_SUCCESS;

    err1:
    pthread_mutex_lock(&vproc_mutex);
    g_grp_used[grp_id] = false;
    pthread_mutex_unlock(&vproc_mutex);

    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetExtChnAttr(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx, MAPI_EXTCHN_ATTR_T *pstExtChnAttr)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    // CHECK_VPROC_CHN(chn_idx);
    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return MAPI_ERR_FAILURE;
    }
    int i = 0;
    CVI_BOOL ExtChnEn = false;
    for(i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if ((pt->ExtChn[i].ExtChnEnable == true) && (pt->ExtChn[i].ExtChnAttr.ChnId == chn_idx)) {
            ExtChnEn = true;
            break;
        }
    }
    if (ExtChnEn == true) {
        memcpy(pstExtChnAttr, &pt->ExtChn[i].ExtChnAttr, sizeof(MAPI_EXTCHN_ATTR_T));
    } else {
        CHECK_VPROC_CHN(chn_idx);
        CVI_LOGE("This chn_idx = %d is not ext chn !\n", chn_idx);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_GetExtChnGrp(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx)
{
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);
    // CHECK_VPROC_CHN(chn_idx);

    if(!g_grp_used[pt->VpssGrp]){
        CVI_LOGE("vproc grp %d uninitialized\n", pt->VpssGrp);
        return -1;
    }

    for(int i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if ((pt->ExtChn[i].ExtChnEnable == true) && (pt->ExtChn[i].ExtChnAttr.ChnId == chn_idx)) {
            return pt->ExtChn[i].ExtChnGrp;
        }
    }

    CVI_LOGE("This chn_idx = %d is not ext chn !\n", chn_idx);

    return -1;
}

int MAPI_VPROC_IsExtChn(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx)
{
    if (vproc_hdl == NULL) {
        return 0;
    }

    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;

    for(int i = 0; i < VPSS_MAX_PHY_CHN_NUM; i++) {
        if ((pt->ExtChn[i].ExtChnEnable == true) && (pt->ExtChn[i].ExtChnAttr.ChnId == chn_idx)) {
            return 1;
        }
    }

    return 0;
}

int MAPI_VPROC_EnableChn(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_EnableChn(pt->VpssGrp, chn_idx);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_EnableChn failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_VPROC_DisableChn(MAPI_VPROC_HANDLE_T vproc_hdl, uint32_t chn_idx)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    VPROC_CHECK_NULL_PTR(vproc_hdl);
    MAPI_VPROC_CTX_T *pt = (MAPI_VPROC_CTX_T *)vproc_hdl;
    CHECK_VPROC_GRP(pt->VpssGrp);

    s32Ret = CVI_VPSS_DisableChn(pt->VpssGrp, chn_idx);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_DisableChn failed, rc: 0x%x !\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

#ifdef DUAL_OS
int MAPI_VPROC_EnableTileMode(void)
{
    CVI_S32 s32Ret = 0;

    #ifndef CHIP_184X
    s32Ret = CVI_VPSS_EnableTileMode();
    #endif
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_EnableTileMode failed with %#x\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }

    return 0;
}

int MAPI_VPROC_DisableTileMode(void)
{
    CVI_S32 s32Ret = 0;

    #ifndef CHIP_184X
    s32Ret = CVI_VPSS_DisableTileMode();
    #endif
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VPSS_DisableTileMode failed with %#x\n", s32Ret);
        return MAPI_ERR_FAILURE;
    }

    return 0;
}
#endif

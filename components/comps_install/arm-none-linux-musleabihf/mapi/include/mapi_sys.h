#ifndef __MAPI_SYS_H__
#define __MAPI_SYS_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_vb.h"
#include "cvi_comm_vpss.h"


#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MAPI_MEDIA_SYS_VB_POOL_S {
    union mapi_vb_blk_size {
        uint32_t                   size;
        struct cvi_vb_blk_frame_s {
            uint32_t             width;
            uint32_t             height;
            PIXEL_FORMAT_E       fmt;
        } frame;
    } vb_blk_size;
    bool                         is_frame;
    uint32_t                     vb_blk_num;
} MAPI_MEDIA_SYS_VB_POOL_T;

#define MAPI_VB_POOL_MAX_NUM (16)
typedef struct MAPI_MEDIA_SYS_ATTR_S {
    MAPI_MEDIA_SYS_VB_POOL_T vb_pool[MAPI_VB_POOL_MAX_NUM];
    uint32_t                     vb_pool_num;
    VI_VPSS_MODE_S stVIVPSSMode;
    VPSS_MODE_S stVPSSMode;
} MAPI_MEDIA_SYS_ATTR_T;

void MAPI_Media_PrintVersion(void);
int MAPI_Media_Init(MAPI_MEDIA_SYS_ATTR_T *attr);
int MAPI_Media_Deinit(void);
int MAPI_SYS_VI_VPSS_Mode_Init(MAPI_MEDIA_SYS_ATTR_T *attr);
int MAPI_SYS_VB_Init(MAPI_MEDIA_SYS_ATTR_T *attr);

//
// VB Frame helper functions
//
int MAPI_ReleaseFrame(VIDEO_FRAME_INFO_S *frm);
int MAPI_AllocateFrame(VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt);
int MAPI_AllocateFrame_ByPoolID(VB_POOL pool,VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt);
int MAPI_GetFrameFromMemory_YUV(VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt, void *data);
int MAPI_GetFrameFromFile_YUV(VIDEO_FRAME_INFO_S *frame,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt,
        const char *filaneme, uint32_t frame_no);
int MAPI_SaveFramePixelData(VIDEO_FRAME_INFO_S *frm, const char *name);

int MAPI_FrameMmap(VIDEO_FRAME_INFO_S *frm, bool enable_cache);
int MAPI_FrameMunmap(VIDEO_FRAME_INFO_S *frm);
int MAPI_FrameFlushCache(VIDEO_FRAME_INFO_S *frm);
int MAPI_FrameInvalidateCache(VIDEO_FRAME_INFO_S *frm);
VB_POOL MAPI_CreateVbPool(PIXEL_FORMAT_E encode_type, int input_width, int input_height, uint32_t vb_cnt);
int MAPI_DestroyVbPool(VB_POOL VbPool);

//for ION
void *MAPI_MemAllocate(size_t size, const char *name);
void MAPI_MemFree(void *vir_addr);
//for VB
void *MAPI_MemAllocateVb(size_t size);
void MAPI_MemVbFree(void *vir_addr);

#ifdef __cplusplus
}
#endif

#endif

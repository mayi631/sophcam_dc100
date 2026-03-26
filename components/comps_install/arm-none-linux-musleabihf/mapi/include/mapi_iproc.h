#ifndef __MAPI_IPROC_H__
#define __MAPI_IPROC_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"
#include "mapi_define.h"
#include "mapi_vcap.h"
#include "mapi_preprocess.h"

#include "cvi_comm_vpss.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MAPI_IPROC_RECT_S {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} MAPI_IPROC_RECT_T, MAPI_IPROC_RESIZE_CROP_ATTR_T;

int MAPI_IPROC_Resize(VIDEO_FRAME_INFO_S *frame_in,
        VIDEO_FRAME_INFO_S *frame_out,
        uint32_t resize_width,
        uint32_t resize_height,
        PIXEL_FORMAT_E fmt_out,
        bool keep_aspect_ratio,
        MAPI_IPROC_RESIZE_CROP_ATTR_T *crop_in,
        MAPI_IPROC_RESIZE_CROP_ATTR_T *crop_out,
        MAPI_PREPROCESS_ATTR_T *preprocess);

int MAPI_IPROC_StrechBlt(VIDEO_FRAME_INFO_S *frame_src,
        MAPI_IPROC_RECT_T *rect_src,   // if NULL, use the full frame_src
        VIDEO_FRAME_INFO_S *frame_dst,
        bool frame_dst_has_content,        // when this is true, the frame_dst is both in/out
        uint32_t frame_dst_width,          // used when frame_dst_has_content is false
        uint32_t frame_dst_height,         // used when frame_dst_has_content is false
        PIXEL_FORMAT_E frame_dst_fmt,      // used when frame_dst_has_content is false
        bool     frame_dst_eable_bg_colar, // used when frame_dst_has_content is false
        uint32_t frame_dst_bg_color,       // used when frame_dst_has_content is false
        MAPI_IPROC_RECT_T *rect_dst);

int MAPI_IPROC_Rotate(VIDEO_FRAME_INFO_S *frame_src,
        VIDEO_FRAME_INFO_S *frame_dst,
        ROTATION_E rotation);

// Deprecated, use MAPI_ReleaseFrame instead
int MAPI_IPROC_ReleaseFrame(VIDEO_FRAME_INFO_S *frm);

#ifdef __cplusplus
}
#endif

#endif

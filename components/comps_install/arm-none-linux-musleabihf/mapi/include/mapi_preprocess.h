#ifndef __MAPI_PREPROCESS_H__
#define __MAPI_PREPROCESS_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "cvi_log.h"
#include "mapi_define.h"
#include "cvi_comm_vpss.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct MAPI_PREPROCESS_ATTR_S {
    bool is_rgb;           // default false
    float raw_scale;       // default 255.0 means no raw_scale
    float mean[3];         // in BGR order
    float input_scale[3];  // in BGR order, combined input_scale and std[3]
    float input_threshold;
} MAPI_PREPROCESS_ATTR_T;

static inline void MAPI_PREPROCESS_ENABLE(VPSS_CHN_ATTR_S *attr_chn,
        MAPI_PREPROCESS_ATTR_T *preprocess)
{
    attr_chn->stNormalize.bEnable     = CVI_TRUE;
    if (preprocess->is_rgb) {
        CVI_LOG_ASSERT(attr_chn->enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR,
                       "Preprocess (RGB) enabled, fmt_out needs to be "
                       "PIXEL_FORMAT_RGB_888_PLANAR\n");
    } else {
        CVI_LOG_ASSERT(attr_chn->enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR,
                       "Preprocess (BGR) enabled, fmt_out needs to be "
                       "PIXEL_FORMAT_BGR_888_PLANAR\n");
    }

    float quant_scale = 128.0f / preprocess->input_threshold;
    float factor[3];
    float mean[3];
    for (int i = 0; i < 3; i++) {
        factor[i] = preprocess->raw_scale / 255.0f;
        factor[i] *= preprocess->input_scale[i] * quant_scale;
        if (factor[i] < 1.0f / 8192) {
            factor[i] = 1.0f / 8192;
        }
        if (factor[i] > 8191.0f / 8192) {
            factor[i] = 8191.0f / 8192;
        }

        mean[i] = preprocess->mean[i];
        mean[i] *= preprocess->input_scale[i] * quant_scale;
    }
    // mean and factor are supposed to be in BGR, swap R&B if RGB
    if (preprocess->is_rgb) {
        float tmp;
        tmp = factor[0]; factor[0] = factor[2]; factor[2] = tmp;
        tmp = mean[0];   mean[0]   = mean[2];     mean[2] = tmp;
    }
    for (int i = 0; i < 3; i++) {
        attr_chn->stNormalize.factor[i] = factor[i];
        attr_chn->stNormalize.mean[i] = mean[i];
    }
    attr_chn->stNormalize.rounding = VPSS_ROUNDING_TO_EVEN;
}

#ifdef __cplusplus
}
#endif

#endif // __MAPI_PREPROCESS_H__

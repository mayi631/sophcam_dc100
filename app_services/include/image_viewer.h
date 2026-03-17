#pragma once

#include "mapi.h"
#include "cvi_comm_video.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* IMAGE_VIEWER_HANDLE_T;
typedef void (*IMAGE_VIEWER_OUTPUT_HANDLER)(VIDEO_FRAME_INFO_S *);
typedef void (*IMAGE_VIEWER_CUSTOM_ARG_OUTPUT_HANDLER)(void *,
    VIDEO_FRAME_INFO_S *);

int32_t IMAGE_VIEWER_Create(IMAGE_VIEWER_HANDLE_T *handle);
int32_t IMAGE_VIEWER_Destroy(IMAGE_VIEWER_HANDLE_T *handle);
int32_t IMAGE_VIEWER_SetDecodeHandle(IMAGE_VIEWER_HANDLE_T handle,
        MAPI_VDEC_HANDLE_T decode_handle);
int32_t IMAGE_VIEWER_SetDisplayHandle(IMAGE_VIEWER_HANDLE_T handle,
        MAPI_DISP_HANDLE_T display_handle);
int32_t IMAGE_VIEWER_DisplayThumbnail(IMAGE_VIEWER_HANDLE_T handle, const char *input,
        POINT_S pos, SIZE_S size);
int32_t IMAGE_VIEWER_SetOutputHandler(IMAGE_VIEWER_HANDLE_T handle,
        IMAGE_VIEWER_OUTPUT_HANDLER handler);
int32_t IMAGE_VIEWER_SetCustomArgOutputHandler(IMAGE_VIEWER_HANDLE_T handle,
        IMAGE_VIEWER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg);

#ifdef __cplusplus
}
#endif

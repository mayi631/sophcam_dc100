#pragma once
#include <mapi.h>
#include <mapi_venc.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef void (VIDEO_SERVICR_VCAP_GET_FRAME_CALLBACK) (const VENC_STREAM_S *venc_frame, void *arg);

int32_t VIDEO_SERVICR_CallbackSet(int8_t id, const char *name, VIDEO_SERVICR_VCAP_GET_FRAME_CALLBACK *cb, void *arg);
int32_t VIDEO_SERVICR_CallbackUnSet(int8_t id, const char *name);
int32_t VIDEO_SERVICR_TaskStart(int8_t id, MAPI_VENC_HANDLE_T venc_handle, int32_t vpss_grp, int32_t chnid);
int32_t VIDEO_SERVICR_TaskStop(int8_t id);
#ifdef __cplusplus
}
#endif

#ifndef _RTSP_SER_API_H_
#define _RTSP_SER_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mapi.h"
#include "mapi_aenc.h"

typedef enum {
    RTSP_VIDEO_CODEC_H264 = 0,
    RTSP_VIDEO_CODEC_H265,
    RTSP_VIDEO_CODEC_JPEG,
    RTSP_VIDEO_CODEC_MJPEG
} RTSP_VIDEO_CODEC_E;

typedef enum {
    RTSP_AUDIO_CODEC_NONE,
    RTSP_AUDIO_CODEC_PCM,
    RTSP_AUDIO_CODEC_AAC
} RTSP_AUDIO_CODEC_E;

#define MAX_RTSP_STREAM_NAME_LEN (32)
typedef void(RTSP_SERVICE_CALLBACK) (int32_t references, void *arg);

typedef struct {
    int32_t rtsp_id;
    char rtsp_name[MAX_RTSP_STREAM_NAME_LEN];
    int32_t max_conn;
    int32_t timeout;
    int32_t port;
    RTSP_VIDEO_CODEC_E video_codec;
    RTSP_AUDIO_CODEC_E audio_codec;
    RTSP_SERVICE_CALLBACK *rtsp_play;
    void *rtsp_play_arg;
    RTSP_SERVICE_CALLBACK *rtsp_teardown;
    void *rtsp_teardown_arg;

    uint32_t width;
    uint32_t height;
    float framerate;
    int32_t bitrate_kbps;
    int32_t audio_sample_rate;
    int32_t audio_channels;
    int32_t audio_pernum;

    int32_t chn_id;
    MAPI_VPROC_HANDLE_T vproc;
    MAPI_VENC_HANDLE_T venc_hdl;
    MAPI_ACAP_HANDLE_T acap_hdl;
    MAPI_AENC_HANDLE_T aenc_hdl;
} RTSP_SERVICE_PARAM_S;



typedef void *RTSP_SERVICE_HANDLE_T;

int32_t RTSP_SERVICE_Create(RTSP_SERVICE_HANDLE_T *hdl, RTSP_SERVICE_PARAM_S *param);
int32_t RTSP_SERVICE_Destroy(RTSP_SERVICE_HANDLE_T hdl);
int32_t RTSP_SERVICE_UpdateParam(RTSP_SERVICE_HANDLE_T hdl, RTSP_SERVICE_PARAM_S *param);
int32_t RTSP_SERVICE_StartMute(RTSP_SERVICE_HANDLE_T hdl);
int32_t RTSP_SERVICE_StopMute(RTSP_SERVICE_HANDLE_T hdl);
int32_t RTSP_SERVICE_StartStop(uint32_t value, char *name);


#ifdef __cplusplus
}
#endif

#endif



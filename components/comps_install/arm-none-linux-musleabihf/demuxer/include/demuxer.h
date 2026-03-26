#ifndef __DEMUXER_H__
#define __DEMUXER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* DEMUXER_HANDLE_T;

#define DEMUXER_STREAM_MAX_NUM 5

typedef struct DemuxerPacket
{
    uint8_t *data;
    int32_t size;
    int64_t pts;
    double duration;
    long creationtime;
    int32_t errorccode[4]; // File initialization, thumbnail, Creation time, duration
} DEMUXER_PACKET_S;

typedef struct DemuxerStreamResolution {
    int32_t stream_index;
    uint32_t width;
    uint32_t height;
    char codec[16];
} DEMUXER_STREAM_RESOLUTION_S;

typedef struct DemuxerMediaInfo {
    char file_name[64];
    char format[16];
    int32_t width;
    int32_t height;
    uint64_t file_size;
    double start_time_sec;
    double duration_sec;
    double audio_duration_sec;
    double video_duration_sec;
    DEMUXER_STREAM_RESOLUTION_S stream_resolution[DEMUXER_STREAM_MAX_NUM];
    int32_t used_video_stream_index;
    float frame_rate;
    uint64_t bit_rate;
    uint32_t audio_channel_layout;
    uint32_t sample_rate;
    int32_t used_audio_stream_index;
    char video_codec[16];
    char audio_codec[16];
} DEMUXER_MEDIA_INFO_S;

typedef struct Demuxerstreaminfo {
    int32_t videonum;
    int32_t videoden;
    int32_t audionum;
    int32_t audioden;
    int32_t videowidth;
    int32_t videoheight;
    float frame_rate;
    int64_t duration;
} DEMUXER_STREAM_INFO_S;

int32_t DEMUXER_Create(DEMUXER_HANDLE_T *handle);
int32_t DEMUXER_Destroy(DEMUXER_HANDLE_T *handle);
int32_t DEMUXER_Open(DEMUXER_HANDLE_T handle);
int32_t DEMUXER_Close(DEMUXER_HANDLE_T handle);
int32_t DEMUXER_Pause(DEMUXER_HANDLE_T handle);
int32_t DEMUXER_Resume(DEMUXER_HANDLE_T handle);
int32_t DEMUXER_SetInput(DEMUXER_HANDLE_T handle, const char *input);
int32_t DEMUXER_Read(DEMUXER_HANDLE_T handle, DEMUXER_PACKET_S *packet);
int32_t DEMUXER_Seek(DEMUXER_HANDLE_T handle, const int64_t time_in_ms);
int32_t DEMUXER_GetMediaInfo(DEMUXER_HANDLE_T handle, DEMUXER_MEDIA_INFO_S *info);

#ifdef __cplusplus
}
#endif

#endif
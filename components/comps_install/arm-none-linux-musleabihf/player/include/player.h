#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <stdint.h>
#include <stdbool.h>
#include "demuxer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef DEMUXER_PACKET_S PLAYER_PACKET_S;
typedef DEMUXER_MEDIA_INFO_S PLAYER_MEDIA_INFO_S;
typedef void* PLAYER_HANDLE_T;

typedef enum PlayerEventType
{
    PLAYER_EVENT_OPEN_FAILED,
    PLAYER_EVENT_PLAY,
    PLAYER_EVENT_PLAY_FINISHED,
    PLAYER_EVENT_PLAY_PROGRESS,
    PLAYER_EVENT_PAUSE,
    PLAYER_EVENT_RESUME
} PLAYER_EVENT_TYPE_E;

typedef struct PlayerEvent
{
    PLAYER_EVENT_TYPE_E type;
} PLAYER_EVENT_S;

typedef struct PlayerAudioParameters {
    uint32_t sample_rate;
    int32_t channel;
} PLAYER_AUDIO_PARAMETERS;

typedef struct PlayerVideoParameters {
    uint32_t output_width;
    uint32_t output_height;
    uint32_t max_packet_size;
} PLAYER_VIDEO_PARAMETERS;

typedef struct PlayerPlayInfo {
    double duration_sec;
} PLAYER_PLAY_INFO;

typedef struct PlayerFrame
{
    uint8_t **data; // pointer to AVFrame data
    int32_t *linesize; // pointer to AVFrame linesize
    int32_t width;
    int32_t height;
    int32_t packet_size;
    int64_t pts;
} PLAYER_FRAME_S;

enum COMP_ERROR
{
    NONE,
    FAILURE,
    NO_MEMORY,
    NULL_PTR,
};

enum AV_SYNC_TYPE
{
    AUDIO_MASTER,
    VIDEO_MASTER,
    EXTERNAL_CLOCK, // synchronize to an external clock
};

// handlers signature
typedef void (*PLAYER_OUTPUT_HANDLER)(PLAYER_FRAME_S *);
typedef void (*PLAYER_CUSTOM_ARG_OUTPUT_HANDLER)(void *,
        PLAYER_FRAME_S *);
typedef void (*PLAYER_EVENT_HANDLER)(PLAYER_EVENT_S *);
typedef void (*PLAYER_CUSTOM_ARG_EVENT_HANDLER)(void *,
        PLAYER_EVENT_S *);
typedef struct {
    int32_t (*get_frame)(PLAYER_FRAME_S *);
    int32_t (*decode_packet)(PLAYER_PACKET_S *);
} PLAYER_DECODE_HANDLER_S;
typedef struct {
    int32_t (*get_frame)(void *, PLAYER_FRAME_S *);
    int32_t (*decode_packet)(void *, PLAYER_PACKET_S *);
} PLAYER_CUSTOM_ARG_DECODE_HANDLER_S;

int32_t PLAYER_Init();
int32_t PLAYER_Deinit();
int32_t PLAYER_Create(PLAYER_HANDLE_T *handle);
int32_t PLAYER_Destroy(PLAYER_HANDLE_T *handle);
int32_t PLAYER_SetDataSource(PLAYER_HANDLE_T handle,
        const char *data_source);
int32_t PLAYER_GetDataSource(PLAYER_HANDLE_T handle,
        char *data_source);
int32_t PLAYER_LightOpen(PLAYER_HANDLE_T handle);
int32_t PLAYER_Play(PLAYER_HANDLE_T handle);
int32_t PLAYER_Stop(PLAYER_HANDLE_T handle);
int32_t PLAYER_Pause(PLAYER_HANDLE_T handle);
int32_t PLAYER_Resume(PLAYER_HANDLE_T handle);
int32_t PLAYER_Seek(PLAYER_HANDLE_T handle, int64_t time_in_ms);
int32_t PLAYER_TPlay(PLAYER_HANDLE_T handle, double speed);
int32_t PLAYER_SetAudioParameters(PLAYER_HANDLE_T handle,
        PLAYER_AUDIO_PARAMETERS parameters);
int32_t PLAYER_SetVideoParameters(PLAYER_HANDLE_T handle,
        PLAYER_VIDEO_PARAMETERS parameters);
int32_t PLAYER_SetAOHandler(PLAYER_HANDLE_T handle,
        PLAYER_OUTPUT_HANDLER handler);
int32_t PLAYER_SetCustomArgAOHandler(PLAYER_HANDLE_T handle,
        PLAYER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg);
int32_t PLAYER_SetVOHandler(PLAYER_HANDLE_T handle,
        PLAYER_OUTPUT_HANDLER handler);
int32_t PLAYER_SetCustomArgVOHandler(PLAYER_HANDLE_T handle,
        PLAYER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg);
int32_t PLAYER_SetEventHandler(PLAYER_HANDLE_T handle,
        PLAYER_EVENT_HANDLER handler);
int32_t PLAYER_SetCustomArgEventHandler(PLAYER_HANDLE_T handle,
        PLAYER_CUSTOM_ARG_EVENT_HANDLER handler, void *custom_arg);
int32_t PLAYER_SaveImage(PLAYER_HANDLE_T handle,
        const char *file_path);
int32_t PLAYER_GetMediaInfo(PLAYER_HANDLE_T handle,
        PLAYER_MEDIA_INFO_S *info);
int32_t PLAYER_GetPlayInfo(PLAYER_HANDLE_T handle, PLAYER_PLAY_INFO *info);
int32_t PLAYER_GetVideoFrame(PLAYER_HANDLE_T handle,
        PLAYER_FRAME_S *frame);
int32_t PLAYER_GetVideoPacket(PLAYER_HANDLE_T handle,
        PLAYER_PACKET_S *packet);
int32_t PLAYER_GetVideoExtraPacket(PLAYER_HANDLE_T handle,
        PLAYER_PACKET_S *packet);
int32_t PLAYER_SetVideoDecodeHandler(PLAYER_HANDLE_T handle,
        PLAYER_DECODE_HANDLER_S handler);
int32_t PLAYER_SetVideoCustomArgDecodeHandler(PLAYER_HANDLE_T handle,
        PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg);
int32_t PLAYER_SetAudioDecodeHandler(PLAYER_HANDLE_T handle,
        PLAYER_DECODE_HANDLER_S handler);
int32_t PLAYER_SetAudioCustomArgDecodeHandler(PLAYER_HANDLE_T handle,
        PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg);
bool PLAYER_PacketContainSps(PLAYER_HANDLE_T handle, PLAYER_PACKET_S *packet);
int32_t PLAYER_SeekPause(PLAYER_HANDLE_T handle, int64_t time_in_ms);
int32_t PLAYER_SeekFlage();
int32_t PLAYER_SeekTime();
int32_t PLAYER_PlayerSeep(PLAYER_HANDLE_T handle, int32_t speed, int32_t backforward);
int32_t PLAYER_GetForWardBackWardStatus(PLAYER_HANDLE_T handle);
void PLAYER_SetPlaySubStreamFlag(PLAYER_HANDLE_T handle, bool subflag);
#ifdef __cplusplus
}
#endif

#endif
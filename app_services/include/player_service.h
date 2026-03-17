#ifndef __PLAYER_SERVICE_H__
#define __PLAYER_SERVICE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "mapi.h"
#include "mapi_ao.h"
#include "player.h"
#include "sysutils_signal_slot.h"

typedef struct {
    int32_t chn_id;
    bool repeat;
    // handle
    MAPI_DISP_HANDLE_T disp;
    MAPI_AO_HANDLE_T ao;
    // display
    int32_t disp_id;
    PIXEL_FORMAT_E disp_fmt;
    ROTATION_E disp_rotate;
    ASPECT_RATIO_E disp_aspect_ratio;
    uint32_t x;
    uint32_t y;
    uint32_t width; // 0 for screen width
    uint32_t height; // 0 for screen height
    uint32_t SampleRate;
    uint32_t AudioChannel;
} PLAYER_SERVICE_PARAM_S;

typedef struct {
    SIGNAL_S play;
    SIGNAL_S pause;
    SIGNAL_S resume;
    SIGNAL_S finish;
} PLAYER_SERVICE_SIGNALS_S;

typedef struct {
    SLOT_S set_input;
    SLOT_S play;
    SLOT_S stop;
    SLOT_S pause;
    SLOT_S seek;
    SLOT_S resize;
    SLOT_S toggle_fullscreen;
    SLOT_S get_play_info;
} PLAYER_SERVICE_SLOTS_S;

typedef enum
{
    PLAYER_SERVICE_EVENT_UNKNOWN,
    PLAYER_SERVICE_EVENT_OPEN_FAILED,
    PLAYER_SERVICE_EVENT_PLAY,
    PLAYER_SERVICE_EVENT_PLAY_FINISHED,
    PLAYER_SERVICE_EVENT_PLAY_PROGRESS,
    PLAYER_SERVICE_EVENT_PAUSE,
    PLAYER_SERVICE_EVENT_RESUME,
    PLAYER_SERVICE_EVENT_RECOVER_START,
    PLAYER_SERVICE_EVENT_RECOVER_PROGRESS,
    PLAYER_SERVICE_EVENT_RECOVER_FAILED,
    PLAYER_SERVICE_EVENT_RECOVER_FINISHED,
} PLAYER_SERVICE_EVENT_TYPE_E;

typedef struct
{
    PLAYER_SERVICE_EVENT_TYPE_E type;
    double value;
} PLAYER_SERVICE_EVENT_S;

typedef void* PLAYER_SERVICE_HANDLE_T;

typedef void (*PLAYER_SERVICE_EVENT_HANDLER)(PLAYER_SERVICE_HANDLE_T,
    PLAYER_SERVICE_EVENT_S *);

#ifdef __cplusplus
extern "C" {
#endif

int32_t PLAYER_SERVICE_GetDefaultParam(PLAYER_SERVICE_PARAM_S *param);
int32_t PLAYER_SERVICE_Create(PLAYER_SERVICE_HANDLE_T *handle,
        PLAYER_SERVICE_PARAM_S *param);
int32_t PLAYER_SERVICE_Destroy(PLAYER_SERVICE_HANDLE_T *handle);
int32_t PLAYER_SERVICE_SetInput(PLAYER_SERVICE_HANDLE_T handle, const char *input);
int32_t PLAYER_SERVICE_GetMediaInfo(PLAYER_SERVICE_HANDLE_T handle, PLAYER_MEDIA_INFO_S *info);
int32_t PLAYER_SERVICE_GetPlayInfo(PLAYER_SERVICE_HANDLE_T handle, PLAYER_PLAY_INFO *info);
int32_t PLAYER_SERVICE_Play(PLAYER_SERVICE_HANDLE_T handle);
int32_t PLAYER_SERVICE_PlayerAndSeek(PLAYER_SERVICE_HANDLE_T handle, int64_t seektime);
int32_t PLAYER_SERVICE_Stop(PLAYER_SERVICE_HANDLE_T handle);
int32_t PLAYER_SERVICE_Pause(PLAYER_SERVICE_HANDLE_T handle);
int32_t PLAYER_SERVICE_Seek(PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms);
int32_t PLAYER_SERVICE_SetEventHandler(PLAYER_SERVICE_HANDLE_T handle,
    PLAYER_SERVICE_EVENT_HANDLER handler);
int32_t PLAYER_SERVICE_Resize(PLAYER_SERVICE_HANDLE_T handle, uint32_t width, uint32_t height);
int32_t PLAYER_SERVICE_MoveTo(PLAYER_SERVICE_HANDLE_T handle, uint32_t x, uint32_t y);
int32_t PLAYER_SERVICE_ToggleFullscreen(PLAYER_SERVICE_HANDLE_T handle);
int32_t PLAYER_SERVICE_GetSignals(PLAYER_SERVICE_HANDLE_T handle, PLAYER_SERVICE_SIGNALS_S **signals);
int32_t PLAYER_SERVICE_GetSlots(PLAYER_SERVICE_HANDLE_T handle, PLAYER_SERVICE_SLOTS_S **slots);
int32_t PLAYER_SERVICE_SeekFlage();
int32_t PLAYER_SERVICE_SeekTime();
int32_t PLAYER_SERVICE_SeekPause(PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms);
int32_t PLAYER_SERVICE_TouchSeekPause(PLAYER_SERVICE_HANDLE_T handle, int64_t time_in_ms);
int32_t PLAYER_SERVICE_PlayerSeep(PLAYER_SERVICE_HANDLE_T handle, int32_t speeds);
int32_t PLAYER_SERVICE_PlayerSeepBack(PLAYER_SERVICE_HANDLE_T handle, int32_t speeds);
int32_t PLAYER_SERVICE_GetFileMediaInfo(char *filepatch);
void PLAYER_SERVICE_SetPlaySubStreamFlag(PLAYER_SERVICE_HANDLE_T handle, bool subflag);

#ifdef __cplusplus
}
#endif

#endif // __PLAYER_SERVICE_H__

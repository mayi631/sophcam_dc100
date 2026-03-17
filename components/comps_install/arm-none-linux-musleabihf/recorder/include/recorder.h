#ifndef _RECORDER_H_
#define _RECORDER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "muxer.h"
#include <stdbool.h>

typedef void *RECORDER_HANDLE_T;

typedef enum RECORDER_EVENT_E
{
    RECORDER_EVENT_START,
    RECORDER_EVENT_STOP,
    RECORDER_EVENT_STOP_FAILED,
    RECORDER_EVENT_SPLIT,
    RECORDER_EVENT_WRITE_FRAME_DROP,
    RECORDER_EVENT_WRITE_FRAME_TIMEOUT,
    RECORDER_EVENT_WRITE_FRAME_FAILED,
    RECORDER_EVENT_OPEN_FILE_FAILED,
    RECORDER_EVENT_CLOSE_FILE_FAILED,
    RECORDER_EVENT_SHORT_FILE,
    RECORDER_EVENT_PIV_START,
    RECORDER_EVENT_PIV_END,
    RECORDER_EVENT_SYNC_DONE,
    RECORDER_EVENT_SPLIT_START,
    RECORDER_EVENT_START_EMR,
    RECORDER_EVENT_END_EMR,
    RECORDER_EVENT_FRAME_DROP,
    RECORDER_EVENT_BUTT
} RECORDER_EVENT_E;

typedef enum _RECORDER_PTS_STATE_E {
    RECORDER_PTS_STATE_INIT,
    RECORDER_PTS_STATE_SETTED,
    RECORDER_PTS_STATE_CHECKED,
    RECORDER_PTS_STATE_BUTT
} RECORDER_PTS_STATE_E;

typedef struct _RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S{
    int32_t timeout_ms;
    void *param;
} RECORDER_EVENT_WRITE_FRAME_TIMEOUT_S;

typedef int32_t (*RECORDER_GET_FILENAME_CALLBACK)(void *p, char *filename, int32_t filename_len);
typedef int32_t (*RECORDER_EVENT_CALLBACK)(RECORDER_EVENT_E event_type, const char *filename, void *p);
typedef int32_t (*RECORDER_GET_SUBTITLE_CALLBACK)(void *p, int32_t viPipe, char *str, int32_t str_len);
typedef int32_t (*RECORDER_GET_MEM_BUFFER_STOP_CALLBACK)(void *p);
typedef int32_t (*RECORDER_REQUEST_IDR_CALLBACK)(void *p, MUXER_FRAME_TYPE_E type);
typedef int32_t (*RECORDER_STOP_CALLBACK)(void *p);
typedef int32_t (*RECORDER_GET_DIR_TYPE_CALLBACK)(int32_t id, int32_t base);

typedef enum CALLBACK_TYPE{
    RECORDER_CALLBACK_TYPE_NORMAL = 0,
    RECORDER_CALLBACK_TYPE_LAPSE,
    RECORDER_CALLBACK_TYPE_EVENT,
    RECORDER_CALLBACK_TYPE_BUTT
}RECORDER_CALLBACK_TYPE_E;

typedef enum RECORDER_TYPE_INDEX_E{
    RECORDER_TYPE_NORMAL_INDEX = 0,
    RECORDER_TYPE_LAPSE_INDEX = 0,
    RECORDER_TYPE_EVENT_INDEX = 1,
    RECORDER_TYPE_BUTT_INDEX = 2
}RECORDER_TYPE_INDEX_E;


typedef struct CALLBACK_HANDLES_S{
    RECORDER_GET_SUBTITLE_CALLBACK pfn_get_subtitle_cb;
    void *pfn_get_subtitle_cb_param;
    RECORDER_GET_FILENAME_CALLBACK pfn_get_filename;
    void *pfn_get_filename_param[RECORDER_CALLBACK_TYPE_BUTT];
    RECORDER_REQUEST_IDR_CALLBACK pfn_request_idr;
    void *pfn_request_idr_param;
    RECORDER_EVENT_CALLBACK pfn_event_cb[RECORDER_TYPE_BUTT_INDEX]; /*normal && lapse share*/
    void *pfn_event_cb_param;
    RECORDER_GET_MEM_BUFFER_STOP_CALLBACK pfn_mem_buffer_stop_cb;
    void *pfn_mem_buffer_stop_cb_param;
    RECORDER_STOP_CALLBACK pfn_rec_stop_cb;
    void *pfn_rec_stop_cb_param;
    void *pfn_rec_malloc_mem;
    void *pfn_rec_free_mem;
}RECORDER_CB_HANDLES_S;

typedef enum RECORDER_RBUF_TYPE_E{
    RECORDER_RBUF_VIDEO = 0,
    RECORDER_RBUF_SUB_VIDEO,
    RECORDER_RBUF_AUDIO,
    RECORDER_RBUF_SUBTITLE,
    RECORDER_RBUF_BUTT
}RECORDER_RBUF_TYPE_E;


typedef struct RECORDER_RBUF_ATTR_S{
    uint32_t size;
    const char *name;
}RECORDER_RBUF_ATTR_S;


#define RECORDER_TRACK_MAX_CNT (RECORDER_TRACK_SOURCE_TYPE_BUTT)

typedef enum Track_SourceType_E {
    RECORDER_TRACK_SOURCE_TYPE_VIDEO = 0,
    CVI_RECORDER_TRACK_SOURCE_TYPE_SUB_VIDEO,
    RECORDER_TRACK_SOURCE_TYPE_AUDIO,
    RECORDER_TRACK_SOURCE_TYPE_PRIV,
    RECORDER_TRACK_SOURCE_TYPE_BUTT
} RECORDER_TRACK_SOURCE_TYPE_E;


/* record type enum */
typedef enum REC_TYPE_E {
    RECORDER_TYPE_NORMAL = 0, /* normal record */
    RECORDER_TYPE_LAPSE,      /* time lapse record, record a frame by an fixed time interval */
    RECORDER_TYPE_BUTT
} RECORDER_TYPE_E;

#define REC_STREAM_MAX_CNT (4)

/* splite define */
/* record split type enum */
typedef enum REC_SPLIT_TYPE_E {
    RECORDER_SPLIT_TYPE_NONE = 0, /* means split is disabled */
    RECORDER_SPLIT_TYPE_TIME,     /* record split when time reaches */
    RECORDER_SPLIT_TYPE_BUTT
} RECORDER_SPLIT_TYPE_E;

/* record split attribute param */
typedef struct REC_SPLIT_ATTR_S {
    RECORDER_SPLIT_TYPE_E enSplitType; /* split type */
    uint64_t u64SplitTimeLenMSec;       /* split time, unit in msecond(ms) */
} RECORDER_SPLIT_ATTR_S;


/* normal record attribute param */
typedef struct REC_NORMAL_ATTR_S {
    uint32_t u32Rsv; /* reserve */
} RECORDER_NORMAL_ATTR_S;

/* lapse record attribute param */
typedef struct REC_LAPSE_ATTR_S {
    uint32_t u32IntervalMs; /* lapse record time interval, unit in millisecord(ms) */
    float fFramerate;
} RECORDER_LAPSE_ATTR_S;


typedef struct Track_VideoSourceInfo_S {
    MUXER_TRACK_VIDEO_CODEC_E enCodecType;
    uint32_t u32Width;
    uint32_t u32Height;
    uint32_t u32BitRate;
    float fFrameRate;
    uint32_t u32Gop;
    float fSpeed;
} RECORDER_TRACK_VideoSourceInfo_S;

typedef struct Track_AudioSourceInfo_S {
    MUXER_TRACK_AUDIO_CODEC_E enCodecType;
    uint32_t u32ChnCnt;
    uint32_t u32SampleRate;
    uint32_t u32AvgBytesPerSec;
    uint32_t u32SamplesPerFrame;
    unsigned short u16SampleBitWidth;
    float fFramerate;
} RECORDER_TRACK_AudioSourceInfo_S;

typedef struct Track_PrivateSourceInfo_S
{
    uint32_t u32PrivateData;
    uint32_t u32FrameRate;
    uint32_t u32BytesPerSec;
    int32_t bStrictSync;
} RECORDER_TRACK_PrivateSourceInfo_S;

typedef struct Track_Source_S
{
    RECORDER_TRACK_SOURCE_TYPE_E enTrackType;
    int32_t enable;
    union
    {
        RECORDER_TRACK_VideoSourceInfo_S stVideoInfo;
        RECORDER_TRACK_AudioSourceInfo_S stAudioInfo;
        RECORDER_TRACK_PrivateSourceInfo_S stPrivInfo;
    } unTrackSourceAttr;
}RECORDER_TRACK_SOURCE_S;


/* record stream attribute */
typedef struct REC_STREAM_ATTR_S {
    uint32_t u32TrackCnt;                                            /* track cnt */
    RECORDER_TRACK_SOURCE_S aHTrackSrcHandle[RECORDER_TRACK_MAX_CNT]; /* array of track source cnt */
} RECORDER_STREAM_ATTR_S;


typedef struct RECORDER_ATTR_S {
    RECORDER_STREAM_ATTR_S astStreamAttr;
    RECORDER_CB_HANDLES_S fncallback;
    RECORDER_TYPE_E enRecType; /* record type */
    union {
        RECORDER_NORMAL_ATTR_S stNormalRecAttr; /* normal record attribute */
        RECORDER_LAPSE_ATTR_S stLapseRecAttr;   /* lapse record attribute */
    } unRecAttr;
    RECORDER_RBUF_ATTR_S stRbufAttr[RECORDER_RBUF_BUTT];
    RECORDER_SPLIT_ATTR_S stSplitAttr; /* record split attribute */
    int32_t enable_subtitle;
    int32_t enable_thumbnail;
    int32_t enable_subvideo;
    int32_t enable_file_alignment;
    int32_t enable_emrfile_from_normfile;
    float subtitle_framerate;
    uint32_t u32PreRecTimeSec;                                   /*  pre record time */
    uint32_t u32PostRecTimeSec;                                   /*  post record time */
    int32_t s32MemRecPreSec;
    char *device_model;
    int32_t prealloc_size;
    float short_file_ms;
    int32_t id;
} RECORDER_ATTR_S;

#define FRAME_STREAM_SEGMENT_MAX_NUM (8)
#define SUBTITLE_MAX_LEN (200)
#define SEND_FRAME_TIMEOUT_MS (1000)

typedef struct FRAME_STREAM_S {
    MUXER_FRAME_TYPE_E type;
    bool vftype[FRAME_STREAM_SEGMENT_MAX_NUM];
    int32_t num;
    uint64_t vi_pts[FRAME_STREAM_SEGMENT_MAX_NUM];
    unsigned char *data[FRAME_STREAM_SEGMENT_MAX_NUM];
    size_t len[FRAME_STREAM_SEGMENT_MAX_NUM];
    unsigned char *thumbnail_data;
    size_t thumbnail_len;
} RECORDER_FRAME_STREAM_S;


int32_t RECORDER_SendFrame(void *recorder, RECORDER_FRAME_STREAM_S *frame);
int32_t RECORDER_Start_MemRec(void *recorder);
int32_t RECORDER_Stop_MemRec(void *recorder);
int32_t RECORDER_Start_NormalRec(void *recorder);
int32_t RECORDER_Stop_NormalRec(void *recorder);
int32_t RECORDER_Start_EventRec(void *recorder);
int32_t RECORDER_Stop_EventRec(void *recorder);
int32_t RECORDER_ForceStop_EventRec(void *recorder);
int32_t RECORDER_Stop_EventRecPost(void *recorder);
int32_t RECORDER_Start_LapseRec(void *recorder);
int32_t RECORDER_Stop_LapseRec(void *recorder);
void RECORDER_Destroy(void **recorder);
int32_t RECORDER_Create(void **recorder, RECORDER_ATTR_S *attr);
int32_t RECORDER_Split(void *recorder);
int32_t RECORDER_Timelapse_Is_SendVenc(void *recorder, MUXER_FRAME_TYPE_E type);

uint64_t RECORDER_GetUs(void);


#ifdef __cplusplus
}
#endif
#endif

#ifndef _MUXER_H_
#define _MUXER_H_


#include <stdint.h>
#include <stddef.h>

typedef enum Track_VideoCodec_E
{
    MUXER_TRACK_VIDEO_CODEC_H264 = 96,
    MUXER_TRACK_VIDEO_CODEC_H265 = 98,
    MUXER_TRACK_VIDEO_CODEC_MJPEG = 102,
    MUXER_TRACK_VIDEO_CODEC_BUTT
} MUXER_TRACK_VIDEO_CODEC_E;


typedef enum Track_AudioCodec_E
{
    MUXER_TRACK_AUDIO_CODEC_G711Mu  = 0,   /**< G.711 Mu           */
    MUXER_TRACK_AUDIO_CODEC_G711A   = 8,   /**< G.711 A            */
    MUXER_TRACK_AUDIO_CODEC_G726    = 97,   /**< G.726              */
    MUXER_TRACK_AUDIO_CODEC_AMR     = 101,   /**< AMR encoder format */
    MUXER_TRACK_AUDIO_CODEC_ADPCM  = 104,   /**< ADPCM              */
    MUXER_TRACK_AUDIO_CODEC_AAC = 105,
    MUXER_TRACK_AUDIO_CODEC_WAV  = 108,   /**< WAV encoder        */
    MUXER_TRACK_AUDIO_CODEC_MP3 = 109,
    MUXER_TRACK_AUDIO_CODEC_BUTT
} MUXER_TRACK_AUDIO_CODEC_E;


typedef struct CODEC_VIDEO_T{
    int32_t en;
    float framerate;
    MUXER_TRACK_VIDEO_CODEC_E codec;
    uint32_t w;
    uint32_t h;
}MUXER_CODEC_VIDEO_S;

typedef struct CODEC_AUDIO_T{
    int32_t en;
    MUXER_TRACK_AUDIO_CODEC_E codec;
    uint32_t samplerate;
    uint32_t chns;
    float framerate;
}MUXER_CODEC_AUDIO_S;

typedef struct CODEC_SUBTITLE_T{
    int32_t en;
    float framerate;
    uint32_t timebase;
}MUXER_CODEC_SUBTITLE_S;

typedef struct CODEC_THUMBNAIL_T{
    int32_t en;
    int32_t res;
}MUXER_CODEC_THUMBNAIL_S;

typedef enum MUXER_EVENT_E {
    MUXER_OPEN_FILE_FAILED,
    MUXER_SEND_FRAME_FAILED,
    MUXER_SEND_FRAME_TIMEOUT,
    MUXER_PTS_JUMP,
    MUXER_STOP,
    MUXER_EVENT_BUTT
} MUXER_EVENT_E;

typedef enum MUXER_FRAME_TYPE_E {
    MUXER_FRAME_TYPE_VIDEO,
    MUXER_FRAME_TYPE_SUB_VIDEO,
    MUXER_FRAME_TYPE_AUDIO,
    MUXER_FRAME_TYPE_SUBTITLE,
    MUXER_FRAME_TYPE_THUMBNAIL,
    MUXER_FRAME_TYPE_BUTT
} MUXER_FRAME_TYPE_E;

typedef struct FRAME_INFO_T{
    uint32_t hmagic;
    MUXER_FRAME_TYPE_E type;
    int32_t isKey;
    int32_t gopInx;
    int64_t pts;
    uint64_t vpts;
    int32_t dataLen;
    int32_t extraLen;
    int32_t totalSize;
    uint32_t tmagic;
}MUXER_FRAME_INFO_S;

typedef enum _MUXER_VIDEO_TRACKIDX_E {
    MUXER_VIDEO_TRACK_IDX_0 = 0,
    MUXER_VIDEO_TRACK_IDX_1,
    MUXER_VIDEO_TRACK_IDX_BUTT
} MUXER_VIDEO_TRACKIDX_E;

typedef int32_t (*MUXER_EVENT_CALLBACK)(MUXER_EVENT_E event_type, const char *filename, void *p, void *extend);

typedef struct MUXER_ATTR_T{
    MUXER_CODEC_VIDEO_S stvideocodec[MUXER_VIDEO_TRACK_IDX_BUTT];
    MUXER_CODEC_AUDIO_S staudiocodec;
    MUXER_CODEC_SUBTITLE_S stsubtitlecodec;
    MUXER_CODEC_THUMBNAIL_S stthumbnailcodec;
    char *devmod;
    int32_t alignflag;
    int32_t presize;
    uint64_t u64SplitTimeLenMSec;
    MUXER_EVENT_CALLBACK pfncallback;
    void *pfnparam;
}MUXER_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t MUXER_Create(MUXER_ATTR_S attr, void **muxer);
int32_t MUXER_Start(void *muxer, const char *fname);
int32_t MUXER_WritePacket(void *muxer, MUXER_FRAME_TYPE_E type, MUXER_FRAME_INFO_S *packet);
void MUXER_Stop(void *muxer);
void MUXER_Destroy(void *muxer);
void MUXER_FlushPackets(void *muxer, int32_t flag);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#define MUXER_EXT_DATA_LEN 4
extern const unsigned char g_ext_audio_data[MUXER_EXT_DATA_LEN];


#endif

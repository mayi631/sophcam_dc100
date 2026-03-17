#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>

#include <libavformat/avformat.h>
#include "cvi_log.h"
#include "recorder.h"
#include "sample_recorder.h"
#include <time.h>

char* filetype;

static inline int32_t get_video_buffer_size(int32_t bitrate_kbps, int32_t buffer_sec) {
    return (1.4 * ((bitrate_kbps * buffer_sec) >> 3) + 500);
}

static inline int32_t get_audio_pcm_buffer_size(int32_t sample_rate, int32_t channels, int32_t sample_size, int32_t buffer_sec) {
    return (sample_rate * channels * sample_size * buffer_sec * 6);
}


static int32_t recorder_get_filename_cb(void *param, char *filename, int32_t filename_len) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 20, "%Y-%m-%d-%H-%M", timeinfo);
    snprintf(filename, filename_len, "%s.%s", buffer,filetype);

    UNUSED(param);
    return 0;
}

static int32_t CVI_RECORD_GetSubtitleCallBack(void *p, int32_t viPipe, char *str, int32_t str_len) {
    UNUSED(p);
    UNUSED(viPipe);
    UNUSED(str);
    UNUSED(str_len);
    return 0;
}

static int32_t recorder_request_idr_cb(void *param) {
    UNUSED(param);
    return 0;
}

/*TODO*/
static int32_t mem_buffer_stop_callback(void *param) {
    UNUSED(param);
    return 0;
}

static int32_t rec_stop_all_callback(void *param){
    UNUSED(param);
    return 0;
}

int32_t CVI_RECORDMNG_ContCallBack(RECORDER_EVENT_E event_type, const char *filename, void *param) {
    UNUSED(event_type);
    UNUSED(filename);
    UNUSED(param);
    return 0;
}

int32_t CVI_RECORDMNG_EventCallBack(RECORDER_EVENT_E event_type, const char *filename, void *param) {
    UNUSED(event_type);
    UNUSED(filename);
    UNUSED(param);
    return 0;
}

void* ringbuffer_malloc_callback(size_t size, const char *name) {
    UNUSED(name);
    return malloc(size);
}

void ringbuffer_free_callback(void *vir_addr) {
    free(vir_addr);
}

int main(int argc , char** argv) {

    // CVI_LOG_SET_LEVEL(CVI_LOG_DEBUG);

    if(argc < 5){
        printf("Usage : |MOV: H264/H265 + PCM | MP4 : H264/H265 + AAC | TS : H264/H265 + AAC |\n");
        fprintf(stderr, "Usage(command): %s <input H264/H265 file> <input PCM/AAC file> <input thumb(jpeg) file> <output filetype(MOV/MP4/TS)>\n", argv[0]);

        return -1;
    }

    char *video_name = argv[1];
    char *audio_name = argv[2];
    char *thumb_name = argv[3];
    filetype = argv[4];

    FILE* h264_file = fopen(video_name ,"r");
    FILE* PCM_file = fopen(audio_name ,"r");
    FILE* thumb_file = fopen(thumb_name ,"r");

    if(h264_file == NULL || PCM_file == NULL || thumb_file == NULL){
        CVI_LOGE("can't open input file !!!!!!!!");
    }\
    fseek(h264_file, 0 ,SEEK_END);
    size_t video_file_size = ftell(h264_file);
    fseek(h264_file, 0 ,SEEK_SET);

    fseek(PCM_file, 0 ,SEEK_END);
    size_t pcm_file_size = ftell(PCM_file);
    fseek(PCM_file, 0 ,SEEK_SET);

    fseek(thumb_file, 0 ,SEEK_END);
    size_t thumb_file_size = ftell(thumb_file);
    fseek(thumb_file, 0 ,SEEK_SET);

    char *video_buffer = (char *)malloc(video_file_size);
    char *audio_buffer = (char *)malloc(pcm_file_size);
    char *thumb_buffer = (char *)malloc(thumb_file_size);
    if (video_buffer == NULL || audio_buffer == NULL || thumb_buffer == NULL) {
        CVI_LOGE("malloc failed !!!");
        return -1;
    }

    size_t result_video = fread(video_buffer, 1, video_file_size, h264_file);
    size_t result_audio = fread(audio_buffer, 1, pcm_file_size, PCM_file);
    size_t result_thumb = fread(thumb_buffer, 1, thumb_file_size, thumb_file);
    if (result_video != video_file_size || result_audio != pcm_file_size || result_thumb != thumb_file_size) {
        CVI_LOGE("fread file failed !!!");
        return -1;
    }

    RECORDER_ATTR_S rec_attr;
    memset(&rec_attr, 0x00, sizeof(RECORDER_ATTR_S));

    rec_attr.enRecType = RECORDER_TYPE;
    rec_attr.astStreamAttr.u32TrackCnt = RECORDER_TRACK_CNT;

    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].enTrackType = RECORDER_TRACK_SOURCE_TYPE_VIDEO;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].enable = VIDEO_TRACK_ENABLE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.enCodecType = VIDEO_TRACK_CODECTYPE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.u32Height = VIDEO_TRACK_WIDTH;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.u32Width = VIDEO_TRACK_HIGHT;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.u32BitRate = VIDEO_TRACK_BITRATE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.fFrameRate = VIDEO_TRACK_FRAMERATE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.u32Gop = VIDEO_TRACK_GOP;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_VIDEO].unTrackSourceAttr.stVideoInfo.fSpeed = VIDEO_TRACK_FRAMERATE;

    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].enTrackType = RECORDER_TRACK_SOURCE_TYPE_AUDIO;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].enable = AUDIO_TRACK_ENABLE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.enCodecType = AUDIO_TRACK_CODECTYPE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.u32SampleRate = AUDIO_TRACK_SAMPLERATE;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.u32ChnCnt = AUDIO_TRACK_CHN;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.u32SamplesPerFrame = AUDIO_TRACK_SAMPLES_PERFRAME;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_AUDIO].unTrackSourceAttr.stAudioInfo.fFramerate = AUDIO_TRACK_SAMPLERATE / AUDIO_TRACK_SAMPLES_PERFRAME;

    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_PRIV].enTrackType = RECORDER_TRACK_SOURCE_TYPE_PRIV;
    rec_attr.astStreamAttr.aHTrackSrcHandle[RECORDER_TRACK_SOURCE_TYPE_PRIV].enable = PRIV_TRACK_ENABLE;

    rec_attr.fncallback.pfn_request_idr = (RECORDER_REQUEST_IDR_CALLBACK)recorder_request_idr_cb;
    rec_attr.fncallback.pfn_request_idr_param = (void *)NULL;

    rec_attr.enable_subtitle = RECORDER_SUBTITLE_ENABLE;
    if (rec_attr.enable_subtitle) {
        rec_attr.subtitle_framerate = 5;
        rec_attr.fncallback.pfn_get_subtitle_cb = CVI_RECORD_GetSubtitleCallBack;
        rec_attr.fncallback.pfn_get_subtitle_cb_param = (void *)NULL;
    }
    rec_attr.enable_thumbnail = RECORDER_THUMBNAIL_ENABLE;
    rec_attr.enable_file_alignment = RECORDER_FILE_ALIGNMENT_ENABLE;
    rec_attr.enable_emrfile_from_normfile = RECORDER_EMRFILE_FROM_NORMFILE_ENABLE;

    if (rec_attr.enRecType == RECORDER_TYPE_NORMAL) {
        rec_attr.fncallback.pfn_event_cb[RECORDER_TYPE_NORMAL_INDEX] = CVI_RECORDMNG_ContCallBack;
        rec_attr.fncallback.pfn_event_cb_param = (void *)NULL;

        rec_attr.fncallback.pfn_event_cb[RECORDER_TYPE_EVENT_INDEX] = CVI_RECORDMNG_EventCallBack;
        rec_attr.u32PostRecTimeSec = 10;
        rec_attr.u32PreRecTimeSec = 0;
    } else if (rec_attr.enRecType == RECORDER_TYPE_LAPSE) {
        rec_attr.fncallback.pfn_event_cb[RECORDER_TYPE_LAPSE_INDEX] = CVI_RECORDMNG_ContCallBack;
        rec_attr.fncallback.pfn_event_cb_param = (void *)NULL;

        rec_attr.unRecAttr.stLapseRecAttr.fFramerate = RECORDER_LAPSETIME_FRAMERATE;
        rec_attr.unRecAttr.stLapseRecAttr.u32IntervalMs = RECORDER_LAPSETIME_GOP * 1000;
    } else {
        return -1;
    }

    rec_attr.fncallback.pfn_get_filename = (RECORDER_GET_FILENAME_CALLBACK)recorder_get_filename_cb;
    rec_attr.fncallback.pfn_get_filename_param[RECORDER_CALLBACK_TYPE_NORMAL] = (void *)NULL;

    rec_attr.fncallback.pfn_rec_malloc_mem = ringbuffer_malloc_callback;
    rec_attr.fncallback.pfn_rec_free_mem = ringbuffer_free_callback;

    rec_attr.device_model = "sophgo";
    rec_attr.stSplitAttr.enSplitType = RECORDER_SPLIT_ENABLE;
    rec_attr.stSplitAttr.u64SplitTimeLenMSec = RECORDER_SPLIT_TIMEMSENC;

    /* TODO */
    rec_attr.short_file_ms = 500;
    rec_attr.prealloc_size = 20;
    rec_attr.s32MemRecPreSec = 0;
    rec_attr.u32PostRecTimeSec = 10;
    rec_attr.u32PreRecTimeSec = 0;
    rec_attr.fncallback.pfn_mem_buffer_stop_cb = mem_buffer_stop_callback;
    rec_attr.fncallback.pfn_mem_buffer_stop_cb_param = (void *)NULL;

    rec_attr.fncallback.pfn_rec_stop_cb = rec_stop_all_callback;
    rec_attr.fncallback.pfn_rec_stop_cb_param = (void *)NULL;

    int32_t presec = ((rec_attr.u32PreRecTimeSec >= (uint32_t)rec_attr.s32MemRecPreSec) ? rec_attr.u32PreRecTimeSec : (uint32_t)rec_attr.s32MemRecPreSec);
    if (presec == 0 || rec_attr.enRecType == RECORDER_TYPE_LAPSE) {
        presec = 1;
    } else {
        presec += 1;
    }
    uint32_t bitrate = VIDEO_TRACK_BITRATE;
    uint32_t sampleRate = AUDIO_TRACK_SAMPLERATE;
    uint32_t chns = AUDIO_TRACK_CHN;
    CVI_LOGD("video bitrate %u audio sampleRate %u chns %u presec %d", bitrate, sampleRate, chns, presec);
    rec_attr.stRbufAttr[RECORDER_RBUF_VIDEO].size = get_video_buffer_size(bitrate, presec) * 1024;
    rec_attr.stRbufAttr[RECORDER_RBUF_VIDEO].name = (const char *)"rs_v";
    rec_attr.stRbufAttr[RECORDER_RBUF_AUDIO].size =  get_audio_pcm_buffer_size(sampleRate, chns, 2, presec);
    rec_attr.stRbufAttr[RECORDER_RBUF_AUDIO].name = (const char *)"rs_a";
    rec_attr.stRbufAttr[RECORDER_RBUF_SUBTITLE].size = 50 * 1024;
    rec_attr.stRbufAttr[RECORDER_RBUF_SUBTITLE].name = (const char *)"rs_s";

    rec_attr.id = 0;
    void* recorder = NULL;
    RECORDER_Create(&recorder, &rec_attr);

    // CVI_AUDIO_SERVICR_ACAP_TaskStart();

    RECORDER_Start_NormalRec(recorder);

    RECORDER_FRAME_STREAM_S frame_stream;
    memset(&frame_stream, 0x0, sizeof(RECORDER_FRAME_STREAM_S));

    uint64_t timestamp = RECORDER_GetUs();
    frame_stream.data[0] = (unsigned char *)video_buffer;
    frame_stream.len[0] = video_file_size;
    frame_stream.vi_pts[0] = timestamp;
    frame_stream.vftype[0] = 1; // 1 Iframe 0 Pframe
    frame_stream.vftype[1] = 1;
    frame_stream.vftype[2] = 1;

    uint8_t *thumbnail_data = (uint8_t *)thumb_buffer;
    uint32_t thumbnail_len = thumb_file_size;

    frame_stream.num = 3;
    frame_stream.type = MUXER_FRAME_TYPE_VIDEO;
    if (rec_attr.enable_thumbnail && thumbnail_len > 0) {
        frame_stream.thumbnail_len = thumbnail_len;
        frame_stream.thumbnail_data = thumbnail_data;
    }

    RECORDER_FRAME_STREAM_S audio_stream;
    memset(&audio_stream, 0x0, sizeof(RECORDER_FRAME_STREAM_S));
    audio_stream.type = MUXER_FRAME_TYPE_AUDIO;
    audio_stream.num = 1;
    audio_stream.data[0] = (unsigned char *)audio_buffer;
    audio_stream.len[0] = pcm_file_size;
    audio_stream.vi_pts[0] = timestamp;

    for(int i = 0; i < 1500 ; i++){
        if(i > 1){ /*封装过程中只需在某一帧中携带thumb即可*/
            frame_stream.thumbnail_len = 0;
            frame_stream.thumbnail_data = NULL;
        }
        frame_stream.vi_pts[0] = timestamp + i * 40 * 1000;
        audio_stream.vi_pts[0] = timestamp + i * 40 * 1000;
        RECORDER_SendFrame(recorder,&frame_stream);
        RECORDER_SendFrame(recorder,&audio_stream);
        usleep(40 * 1000);

    }

    usleep(100 *1000);

    RECORDER_Stop_NormalRec(recorder);

    free(video_buffer);
    fclose(h264_file);

    free(audio_buffer);
    fclose(PCM_file);

    free(thumb_buffer);
    fclose(thumb_file);

    RECORDER_Destroy(&recorder);

    return 0;
}
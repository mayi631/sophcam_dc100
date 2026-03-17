#ifndef __RS_H__
#define __RS_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mapi.h"
#include <mapi_aenc.h>

typedef enum _RECORD_SERVICE_FILE_TYPE_E {
    RECORD_SERVICE_FILE_TYPE_MP4 = 0,
    RECORD_SERVICE_FILE_TYPE_MOV,
    RECORD_SERVICE_FILE_TYPE_TS,
    RECORD_SERVICE_FILE_TYPE_ES,    // *.264 or *.265 file
    RECORD_SERVICE_FILE_TYPE_JPEG,
    RECORD_SERVICE_FILE_TYPE_NONE,  // No saving
    RECORD_SERVICE_FILE_TYPE_MAX
} RECORD_SERVICE_FILE_TYPE_E;

extern const char *FILE_TYPE_SUFFIX[RECORD_SERVICE_FILE_TYPE_MAX];

typedef enum _RECORD_SERVICE_VIDEO_CODEC_E {
    RECORD_SERVICE_VIDEO_CODEC_H264 = 0,
    RECORD_SERVICE_VIDEO_CODEC_H265,
    RECORD_SERVICE_VIDEO_CODEC_MAX,
} RECORD_SERVICE_VIDEO_CODEC_E;

typedef enum _RECORD_SERVICE_AUDIO_CODEC_E {
    RECORD_SERVICE_AUDIO_CODEC_NONE,
    RECORD_SERVICE_AUDIO_CODEC_PCM,
    RECORD_SERVICE_AUDIO_CODEC_AAC,  // not support yet
    RECORD_SERVICE_AUDIO_CODEC_BUTT
} RECORD_SERVICE_AUDIO_CODEC_E;

typedef enum _RECORD_SERVICE_VENC_BIND_MODE_E {
    RECORD_SERVICE_VENC_BIND_MODE_NONE,
    RECORD_SERVICE_VENC_BIND_MODE_VPSS,
    RECORD_SERVICE_VENC_BIND_MODE_VI, // not support yet
    RECORD_SERVICE_VENC_BIND_MODE_BUTT
} RECORD_SERVICE_VENC_BIND_MODE_E;

typedef enum _RECORD_SERVICE_FLASH_LED_MODE_E {
    RECORD_SERVICE_FLASH_LED_MODE_NC = 0,
    RECORD_SERVICE_FLASH_LED_MODE_NP
} RECORD_SERVICE_FLASH_LED_MODE_E;

typedef struct _CVI_RECORD_SERVICE_PARAM_S {
    int32_t recorder_id;

    bool enable_record_on_start;
    bool enable_perf_on_start;
    bool enable_debug_on_start;
    bool enable_subtitle;
    bool enable_thumbnail;
    bool enable_subvideo;

    // Recorder
    int32_t rec_mode;
    uint32_t rec_width;
    uint32_t rec_height;
    float framerate;
    uint32_t gop;
    int32_t bitrate_kbps;
    int32_t recorder_file_type;
    RECORD_SERVICE_VIDEO_CODEC_E recorder_video_codec;
    RECORD_SERVICE_VIDEO_CODEC_E sub_recorder_video_codec;
    RECORD_SERVICE_AUDIO_CODEC_E recorder_audio_codec;
    uint64_t recorder_split_interval_ms;
#define CS_PARAM_MAX_FILENAME_LEN (128)
    char recorder_save_dir_base[CS_PARAM_MAX_FILENAME_LEN];
    bool audio_recorder_enable;
    int32_t audio_sample_rate;
    int32_t audio_channels;
    int32_t audio_num_per_frame;
    int32_t event_recorder_pre_recording_sec;
    int32_t event_recorder_post_recording_sec;
    float timelapse_recorder_framerate;
    int32_t timelapse_recorder_gop_interval;
    int32_t memory_buffer_sec;
    int32_t pre_alloc_unit;
    void *cont_recorder_event_cb;
    void *event_recorder_event_cb;
    void *timelapse_recorder_event_cb;
    void *get_subtitle_cb;
    void *generate_filename_cb;
    void *get_gps_info_cb;
    char mntpath[32];

    // PIV
    uint32_t prealloclen;

    int32_t vproc_chn_id_venc;
    int32_t vproc_chn_id_thumbnail;
    RECORD_SERVICE_VENC_BIND_MODE_E venc_bind_mode;

    /*new sub*/
    MAPI_VENC_HANDLE_T sub_rec_venc_hdl;
    MAPI_VPROC_HANDLE_T sub_rec_vproc;
    int32_t sub_vproc_chn_id_venc;
    uint32_t sub_rec_width;
    uint32_t sub_rec_height;
    float sub_framerate;
    uint32_t sub_gop;
    int32_t sub_bitrate_kbps;

    MAPI_VPROC_HANDLE_T rec_vproc;
    MAPI_VPROC_HANDLE_T thumbnail_vproc;
    MAPI_VENC_HANDLE_T rec_venc_hdl;
    MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
    uint32_t thumbnail_bufsize;
    MAPI_VENC_HANDLE_T piv_venc_hdl;
    uint32_t piv_bufsize;
    int32_t vproc_id_rec;

    float normal_extend_video_buffer_sec;
    float event_extend_video_buffer_sec;
    float extend_other_buffer_sec;
    float short_file_ms;
    char devmodel[32];

    char normal_dir_type[2];
    char park_dir_type[2];
    char event_dir_type[2];
    char snap_dir_type;
    uint32_t flash_led_gpio;
    uint32_t flash_led_pulse;
    uint32_t focus_pos;
    uint32_t focus_pos_lock;
} RECORD_SERVICE_PARAM_S;

typedef void *RECORD_SERVICE_HANDLE_T;

typedef struct MEDIA_GPS_RMCINFO{
                         //都是采集同一条RMC语句里的数据，GNRMC 或GPRMC
    uint32_t Hour;       //gps信息里输出的小时
    uint32_t Minute;     //gps信息里输出的分
    uint32_t Second;     //gps信息里输出的秒
    uint32_t Year;       //gps信息里输出的年
    uint32_t Month;      //gps信息里输出的月
    uint32_t Day;        //gps信息里输出的天
    char Status;         //gps信息里输出的状态
    char NSInd;          //gps信息里输出的NSInd
    char EWInd;          //gps信息里输出的EWInd
    char reserved;       //gps信息里输出的reserved
    double Latitude;      //gps信息里输出的纬度,注意这是double型，RMC语句里，直接采集进去，不需要任何转化
    double Longitude;     //gps信息里输出的经度，注意这是double型，RMC语句里，直接采集进去，不需要任何转化
    float Speed;         //gps信息里输出的速度
    float Angle;         //gps信息里输出的方向
    char ID[20];         //这个ID，读取RMC语法的ID,加密模组才有
    uint32_t GsensorX;   //重力加速X
    uint32_t GsensorY;   //重力加速Y
    uint32_t GsensorZ;   //重力加速Z
}RECORD_SERVICE_GPS_RMCINFO_S;

typedef struct MEDIA_GPS_INFO{
    int32_t init;
    RECORD_SERVICE_GPS_RMCINFO_S rmc_info;
}RECORD_SERVICE_GPS_INFO_S;

#define MAX_CONTEXT_CNT 4

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t RECORD_SERVICE_Create(RECORD_SERVICE_HANDLE_T *hdl, RECORD_SERVICE_PARAM_S *param);
int32_t RECORD_SERVICE_Destroy(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_UpdateParam(RECORD_SERVICE_HANDLE_T hdl, RECORD_SERVICE_PARAM_S *param);
int32_t RECORD_SERVICE_StartRecord(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StopRecord(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StartTimelapseRecord(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StopTimelapseRecord(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_EventRecord(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StopEventRecord(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StartMute(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StopMute(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_PivCapture(RECORD_SERVICE_HANDLE_T hdl, char *file_name);
void RECORD_SERVICE_WaitPivFinish(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StartMemoryBuffer(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_StopMemoryBuffer(RECORD_SERVICE_HANDLE_T hdl);
int32_t RECORD_SERVICE_AdjustFocus(RECORD_SERVICE_HANDLE_T hdl, char* ratio);
int32_t RECORD_SERVICE_SetFlashLed(RECORD_SERVICE_HANDLE_T hdl, RECORD_SERVICE_FLASH_LED_MODE_E mode);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  // __RS_H__

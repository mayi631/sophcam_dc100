#ifndef __PARAM_H__
#define __PARAM_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "stg.h"
#include "hal_wifi.h"
#include "hal_screen_comp.h"
#include "mapi.h"
#include "mapi_ao.h"
#include "filemng.h"
#include "usb.h"
#include "keymng.h"
#include "gsensormng.h"
#include "gaugemng.h"
#ifdef SERVICES_SPEECH_ON
#include "speechmng.h"
#endif
#ifdef SERVICES_LIVEVIEW_ON
#include "liveview.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "adas_service.h"
typedef ADAS_SERVICE_MODEL_ATTR_S PARAM_ADAS_MODEL_ATTR_S;
#endif
#ifdef SERVICES_FACEP_ON
#include "facep_service.h"
#endif
#include "flash.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#pragma pack(push)
#pragma pack(8)

/* global definitions */
#define PARAM_MAGIC_START           (0xAAAAAAAA)
#define PARAM_MAGIC_END             (0x55555555)

/* error numbers */
#define PARAM_EINVAL          APP_APPCOMM_ERR_ID(APP_MOD_PARAM, APP_EINVAL)                  /**<Invalid argument */
#define PARAM_ENOTINIT        APP_APPCOMM_ERR_ID(APP_MOD_PARAM, APP_ENOINIT)                 /**<Not inited */
#define PARAM_EINITIALIZED    APP_APPCOMM_ERR_ID(APP_MOD_PARAM, APP_EINITIALIZED)            /**<Already Initialized */


/* menu definitions */
#define PARAM_MENU_ITEM_MAX         (8)
#define PARAM_MENU_ITEM_DESC_LEN    (64)

#define MAX_PLAYER_INSTANCES            (1)
#define MAX_VPROC_CNT                   (16)
#define MAX_VENC_CNT                    (MAX_CAMERA_INSTANCES * 4)
#define MAX_APP_RTSP_CNT                (1)
#define MAX_RTSP_CNT                    (MAX_CAMERA_INSTANCES + MAX_APP_RTSP_CNT)
#define PARAM_WND_NUM_MAX           (16)
#define PARAM_FACEP_NUM_MAX           (2)
#define MEDIA_MAX_OSD_IDX           (16)
#define PARAM_WORK_MODE_MAX         (4)
#define PARAM_WORK_MODE_LEN         (32)
#define APP_RTSP_NAME                   "cvi_cam_phone_app"

//menu param
typedef enum _MEDIA_VIDEO_SIZE_E {
    MEDIA_VIDEO_SIZE_640X480P25,
    MEDIA_VIDEO_SIZE_1280X720P25,
    MEDIA_VIDEO_SIZE_1280X720P60,
    MEDIA_VIDEO_SIZE_1280X720P30,
    MEDIA_VIDEO_SIZE_1920X1080P25,
    MEDIA_VIDEO_SIZE_1920X1080P60,
    MEDIA_VIDEO_SIZE_1920X1080P30,
    MEDIA_VIDEO_SIZE_2304X1296P25,
    MEDIA_VIDEO_SIZE_2304X1296P30,
    MEDIA_VIDEO_SIZE_2560X1440P25,
    MEDIA_VIDEO_SIZE_2560X1440P30,
    MEDIA_VIDEO_SIZE_2688X1512P25,
    MEDIA_VIDEO_SIZE_2688X1520P25,
    MEDIA_VIDEO_SIZE_2560X1600P25,
    MEDIA_VIDEO_SIZE_2560X1600P30,
    MEDIA_VIDEO_SIZE_3840X2160P25,
    MEDIA_VIDEO_SIZE_3840X2160P30,
    MEDIA_PHOTO_SIZE_8192X6144P,
    MEDIA_PHOTO_SIZE_8064X4536P,
    MEDIA_PHOTO_SIZE_8192X8192P,
    MEDIA_PHOTO_SIZE_8000X6000P,
    MEDIA_PHOTO_SIZE_7680X4320P,
    MEDIA_PHOTO_SIZE_5760X3240P,
    MEDIA_PHOTO_SIZE_5600X4200P,
    MEDIA_PHOTO_SIZE_4608X3456P,
    MEDIA_PHOTO_SIZE_4000X3000P,
    MEDIA_PHOTO_SIZE_3840X2160P,
    MEDIA_PHOTO_SIZE_2688X1512P,
    MEDIA_PHOTO_SIZE_2592X1944P,
    MEDIA_PHOTO_SIZE_2560X1440P,
    MEDIA_PHOTO_SIZE_1920X1080P,
    MEDIA_PHOTO_SIZE_1280X720P,
    MEDIA_PHOTO_SIZE_640X480P,
    MEDIA_VIDEO_SIZE_1920X1080_NEW,
    MEDIA_PHOTO_SIZE_1920X1080P_NEW,
    MEDIA_VIDEO_SIZE_BUIT
} MEDIA_VIDEO_SIZE_E;

typedef enum _MEDIA_VIDEO_LOOP_E
{
    MEDIA_VIDEO_LOOP_1MIN,
    MEDIA_VIDEO_LOOP_3MIN,
    MEDIA_VIDEO_LOOP_5MIN,
    MEDIA_VIDEO_LOOP_BUIT
} MEDIA_VIDEO_LOOP_E;

typedef enum _MEDIA_PHOTO_SIZE_E {
    MEDIA_PHOTO_SIZE_VGA,
    MEDIA_PHOTO_SIZE_2M,
    MEDIA_PHOTO_SIZE_4M,
    MEDIA_PHOTO_SIZE_5M,
    MEDIA_PHOTO_SIZE_8M,
    MEDIA_PHOTO_SIZE_10M,
    MEDIA_PHOTO_SIZE_12M,
    MEDIA_PHOTO_SIZE_16M,
    MEDIA_PHOTO_SIZE_24M,
    MEDIA_PHOTO_SIZE_32M,
    MEDIA_PHOTO_SIZE_36M,
    MEDIA_PHOTO_SIZE_48M,
    MEDIA_PHOTO_SIZE_64M,
    MEDIA_PHOTO_SIZE_BUIT
} MEDIA_PHOTO_SIZE_E;

typedef enum _MEDIA_PWM_BRI_E
{
    MEDIA_PWM_BRI_LOW,
    MEDIA_PWM_BRI_MID,
    MEDIA_PWM_BRI_HIGH,
    MEDIA_PWM_BRI_BUIT
} MEDIA_PWM_BRI_E;

typedef enum _MEDIA_VIDEO_VENCTYPE_E
{
    MEDIA_VIDEO_VENCTYPE_H264,
    MEDIA_VIDEO_VENCTYPE_H265,
    MEDIA_VIDEO_VENCTYPE_BUIT
} MEDIA_VIDEO_VENCTYPE_E;

typedef enum _MEDIA_VIDEO_LAPSETIME_E
{
    MEDIA_VIDEO_LAPSETIME_OFF,
    MEDIA_VIDEO_LAPSETIME_1S,
    MEDIA_VIDEO_LAPSETIME_2S,
    MEDIA_VIDEO_LAPSETIME_3S,
    MEDIA_VIDEO_LAPSETIME_BUIT
} MEDIA_VIDEO_LAPSETIME_E;

typedef enum _MEDIA_VIDEO_AUDIO_E
{
    MEDIA_VIDEO_AUDIO_OFF,
    MEDIA_VIDEO_AUDIO_ON,
    MEDIA_VIDEO_AUDIO_BUIT
} MEDIA_VIDEO_AUDIO_E;

typedef enum _MEDIA_MOTION_E
{
    MEDIA_MOTION_OFF,
    MEDIA_MOTION_ON,
    MEDIA_MOTION_BUIT
} MEDIA_MOTION_E;
typedef enum _MEDIA_VIDEO_OSD_E
{
    MEDIA_VIDEO_OSD_OFF,
    MEDIA_VIDEO_OSD_ON,
    MEDIA_VIDEO_OSD_BUIT
} MEDIA_VIDEO_OSD_E;

typedef enum _MEDIA_AUDIO_KEYTONE_E
{
    MEDIA_AUDIO_KEYTONE_OFF,
    MEDIA_AUDIO_KEYTONE_ON,
    MEDIA_AUDIO_KEYTONE_BUIT
} MEDIA_AUDIO_KEYTONE_E;

typedef enum _MEDIA_VENC_BIND_MODE_E
{
    MEDIA_VENC_BIND_NONE,
    MEDIA_VENC_BIND_VPSS,
    MEDIA_VENC_BIND_VI,
    MEDIA_BIND_MODE_BUIT
} MEDIA_VENC_BIND_MODE_E;

typedef enum _MENU_SCREENDORMANT_E
{
    MENU_SCREENDORMANT_OFF,
    MENU_SCREENDORMANT_1MIN,
    MENU_SCREENDORMANT_3MIN,
    MENU_SCREENDORMANT_5MIN,
    MENU_SCREENDORMANT_BUIT
} MENU_SCREENDORMANT_E;

typedef enum _MENU_FATIGUEDRIVE_E
{
    MENU_FATIGUEDRIVE_OFF,
    MENU_FATIGUEDRIVE_1HOUR,
    MENU_FATIGUEDRIVE_2HOUR,
    MENU_FATIGUEDRIVE_3HOUR,
    MENU_FATIGUEDRIVE_4HOUR,
    MENU_FATIGUEDRIVE_BUIT
} MENU_FATIGUEDRIVE_E;

typedef enum _MENU_SPEEDSTAMP_E
{
    MENU_SPEEDSTAMP_OFF,
    MENU_SPEEDSTAMP_ON,
    MENU_SPEEDSTAMP_BUIT
} MENU_SPEEDSTAMP_E;

typedef enum _MENU_GENSOR_E
{
    MENU_GENSOR_OFF,
    MENU_GENSOR_LOW,
    MENU_GENSOR_MID,
    MENU_GENSOR_HIGH,
    MENU_GENSOR_BUIT
} MENU_GENSOR_E;
typedef enum _MENU_PHOTORES_E
{
    MENU_PHOTORES_VGA,
    MENU_PHOTORES_SVGA,
    MENU_PHOTORES_2M,
    MENU_PHOTORES_3M,
    MENU_PHOTORES_5M,
    MENU_PHOTORES_8M,
    MENU_PHOTORES_10M,
    MENU_PHOTORES_12M,
    MENU_PHOTORES_BUIT
} MENU_PHOTORES_E;

typedef enum _MENU_CAP_QUALITY_E
{
    MENU_CAP_QUALITY_LOW,
    MENU_CAP_QUALITY_MID,
    MENU_CAP_QUALITY_HIGH,
    MENU_CAP_QUALITY_BUIT
} MENU_CAP_QUALITY_E;

typedef enum _MENU_GPSSTAMP_E
{
    MENU_GPSSTAMP_OFF,
    MENU_GPSSTAMP_ON,
    MENU_GPSSTAMP_BUIT
} MENU_GPSSTAMP_E;

typedef enum _MENU_GPSINFO_E
{
    CMENU_GPSINFO_OFF,
    MENU_GPSINFO_ON,
    MENU_GPSINFO_BUIT
} MENU_GPSINFO_E;

typedef enum _MENU_SPEEDUNIT_E
{
    MENU_SPEEDUNIT_KMH,
    MENU_SPEEDUNIT_MPH,
    MENU_SPEEDUNIT_BUIT
} MENU_SPEEDUNIT_E;

typedef enum _MENU_REARCAM_MIRROR_E
{
    MENU_REARCAM_MIRROR_OFF,
    MENU_REARCAM_MIRROR_ON,
    MENU_REARCAM_MIRROR_BUIT
} MENU_REARCAM_MIRROR_E;

typedef enum _MENU_WIFI_E
{
    MENU_WIFI_OFF,
    MENU_WIFI_ON,
    MENU_WIFI_BUIT
} MENU_WIFI_E;

typedef enum _MENU_LANGUAGE_E
{
    MENU_LANGUAGE_CHN,
    MENU_LANGUAGE_ENG,
    MENU_LANGUAGE_BUIT
} MENU_LANGUAGE_E;

typedef enum _MENU_TIME_FORMAT_E
{
    MENU_TIME_FORMAT_12,
    MENU_TIME_FORMAT_24,
    MENU_TIME_FORMAT_BUIT
} MENU_TIME_FORMAT_E;

typedef enum _MENU_TIME_ZONE_E
{
    MENU_TIME_ZONE_CHN,
    MENU_TIME_ZONE_ENG,
    MENU_TIME_ZONE_UNI,
    MENU_TIME_ZONE_JAP,
    MENU_TIME_ZONE_EGP,
    MENU_TIME_ZONE_BUIT
} MENU_TIME_ZONE_E;

typedef enum _MENU_FREQUENCY_E
{
    MENU_FREQUENCY_OFF,
    MENU_FREQUENCY_50,
    MENU_FREQUENCY_60,
    MENU_FREQUENCY_BUIT
} MENU_FREQUENCY_E;

typedef enum _MENU_PARKING_E {
    MENU_PARKING_OFF,
    MENU_PARKING_ON,
    MENU_PARKING_BUIT
} MENU_PARKING_E;

typedef enum _PARAM_MENU_E {
    PARAM_MENU_VIDEO_SIZE,
    PARAM_MENU_VIDEO_LOOP,
    PARAM_MENU_VIDEO_EFFECT,
    PARAM_MENU_GSENSOR,
    PARAM_MENU_CARNUM,
    PARAM_MENU_VIDEO_CODEC,
    PARAM_MENU_LAPSE_TIME,
    PARAM_MENU_AUDIO_STATUS,
    PARAM_MENU_OSD_STATUS,
    PARAM_MENU_PWM_BRI_STATUS,
    PARAM_MENU_VIEW_WIN_STATUS,
    PARAM_MENU_SCREENDORMANT,
    PARAM_MENU_KEYTONE,
    PARAM_MENU_FATIGUE_DRIVE,
    PARAM_MENU_SPEED_STAMP,
    PARAM_MENU_GPS_STAMP,
    PARAM_MENU_SPEED_UNIT,
    PARAM_MENU_REARCAM_MIRROR,
    PARAM_MENU_WIFI_STATUS,
    PARAM_MENU_LANGUAGE,
    PARAM_MENU_TIME_FORMAT,
    PARAM_MENU_TIME_ZONE,
    PARAM_MENU_FREQUENCY,
    PARAM_MENU_DEFAULT,
    PARAM_MENU_PARKING,
    PARAM_MENU_PHOTO_SIZE,
    PARAM_MENU_PHOTO_QUALITY,
    PARAM_MENU_PHOTO_EFFECT,
    PARAM_MENU_MOTION_DET,
    PARAM_MENU_REC_INX,
    PARAM_MENU_REC_LOOP,
    PARAM_MENU_SET_CARNUM_OSD,
    PARAM_MENU_LIGHT_FREQUENCE,
    PARAM_MENU_RECORD_MODE,
    PARAM_MENU_LAPSE_RECORD_TIME,
    PARAM_MENU_SENSOR_SWITCH,
    PARAM_MENU_FACE_DET,
    PARAM_MENU_FACE_SMILE,
    PARAM_MENU_POWER_OFF,
    PARAM_MENU_ACTION_AUDIO,
    PARAM_MENU_FLASH_LED,
    PARAM_MENU_ISP_EFFECT,
    PARAM_MENU_AUTO_SCREEN_OFF,
    PARAM_MENU_AO_VOLUME,
    PARAM_MENU_STATUS_LIGHT,
    PARAM_MENU_BRIGHTNESS,
    PARAM_MENU_BUIT
} PARAM_MENU_E;

/* menu manager */
typedef struct _PARAM_MENU_ITEM_S {
    char    Desc[PARAM_MENU_ITEM_DESC_LEN];
    int32_t     Value;
} PARAM_MENU_ITEM_S;

typedef struct _PARAM_VALUESET_S {
    uint32_t ItemCnt;
    uint32_t Current;
    PARAM_MENU_ITEM_S Items[PARAM_MENU_ITEM_MAX];
} PARAM_VALUESET_S;

typedef struct _PARAM_VALUESET_2X_S {
    uint32_t ItemCnt;
    uint32_t Current;
    PARAM_MENU_ITEM_S Items[PARAM_MENU_ITEM_MAX * 2];
} PARAM_VALUESET_2X_S;

typedef struct _PARAM_VALUESET_4X_S {
    uint32_t ItemCnt;
    uint32_t Current;
    PARAM_MENU_ITEM_S Items[PARAM_MENU_ITEM_MAX * 4];
} PARAM_VALUESET_4X_S;

typedef struct _PARAM_USER_MENU_S {
    uint8_t                    bBootFirst;
    char                    cUserReserved[3];
    uint32_t                     u32UserCarNum[8];
    char                    cUserCarName[16];
    uint32_t                     u32UserReserved[3];
} PARAM_USER_MENU_S;

typedef struct _PARAM_MENU_S {
    PARAM_VALUESET_S VideoSize;
    PARAM_VALUESET_S VideoLoop;
    PARAM_VALUESET_S VideoCodec;
    PARAM_VALUESET_S LapseTime;
    PARAM_VALUESET_S AudioEnable;
    PARAM_VALUESET_S Osd;
    PARAM_VALUESET_S PwmBri;
    PARAM_VALUESET_S ViewWin;
    PARAM_VALUESET_S LcdContrl;
    PARAM_VALUESET_S ScreenDormant;
    PARAM_VALUESET_S KeyTone;
    PARAM_VALUESET_S FatigueDirve;
    PARAM_VALUESET_S SpeedStamp;
    PARAM_VALUESET_S GPSStamp;
    PARAM_VALUESET_S SpeedUnit;
    PARAM_VALUESET_S CamMirror;
    PARAM_VALUESET_S TimeFormat;
    PARAM_VALUESET_S TimeZone;
    PARAM_VALUESET_S Frequence;
    PARAM_VALUESET_S Parking;
    PARAM_VALUESET_S PhotoSize;
    PARAM_VALUESET_S PhotoQuality;
    PARAM_VALUESET_S MotionDet;
    PARAM_VALUESET_S CarNumStamp;
    PARAM_VALUESET_S RecLoop;
    PARAM_VALUESET_S LightFrequence;
    PARAM_VALUESET_S FaceDet;
    PARAM_VALUESET_S FaceSmile;
    PARAM_VALUESET_S PowerOff;
    PARAM_VALUESET_S ActionAudio;
    PARAM_VALUESET_S FlashLed;
    PARAM_VALUESET_4X_S IspEffect;
    PARAM_VALUESET_2X_S Language;
    PARAM_VALUESET_S AutoScreenOff;
    PARAM_VALUESET_S StatusLight;
    PARAM_VALUESET_S Brightness;

    PARAM_USER_MENU_S       UserData;//             bBootFirst;
} PARAM_MENU_S;

/* global parameters manager */
typedef struct _PARAM_HEAD_S {
    uint32_t ParamLen;
} PARAM_HEAD_S;

typedef struct MEDIA_FILEMNG_ATTR_S {
    FILEMNG_PARAM_S FileMng;
} PARAM_FILEMNG_S;

typedef struct _PARAM_WIFI_S {
    bool                  Enable;
    char                  WifiDefaultSsid[HAL_WIFI_SSID_LEN];
    HAL_WIFI_CFG_S    WifiCfg;
} PARAM_WIFI_S;

typedef struct _PARAM_PWM_S {
    bool                  Enable;
    HAL_SCREEN_PWM_S    PWMCfg;
} PARAM_PWM_S;

typedef struct _PARAM_FLASH_LED_S {
    uint32_t GpioNum;
    uint32_t Pulse;
    int32_t Thres;
} PARAM_FLASH_LED_S;

typedef struct _PARAM_DEVMNG_S {
    STG_DEVINFO_S         Stg;
    PARAM_WIFI_S      Wifi;
    KEYMNG_CFG_S      stkeyMngCfg;
    GSENSORMNG_CFG_S  Gsensor;
    GAUGEMNG_CFG_S    GaugeCfg;
    PARAM_PWM_S       PWM;
    PARAM_FLASH_LED_S FlashLed;
} PARAM_DEVMNG_S;

typedef struct _PARAM_DISP_ATTR_S {
    uint32_t    Width;
    uint32_t    Height;
    uint32_t    Rotate;
    int32_t     Fps;
    uint32_t    EnIntfSync;
    uint32_t    frame_fmt;
} PARAM_DISP_ATTR_S;

typedef struct _PARAM_WND_ATTR_S {
    uint32_t        WndCnt;
#ifdef SERVICES_LIVEVIEW_ON
    LIVEVIEW_SERVICE_WNDATTR_S Wnds[PARAM_WND_NUM_MAX];
#endif
} PARAM_WND_ATTR_S;

typedef struct _PARAM_FACEP_ATTR_S{
    CVI_S32 facep_cnt;
#ifdef SERVICES_FACEP_ON
    FACEP_SERVICE_PARAM_S sv_param[PARAM_FACEP_NUM_MAX];
#endif
}PARAM_FACEP_ATTR_S;

typedef struct _PARAM_RECORD_CHN_ATTR_S {
    bool        Enable;
    bool        Subvideoen;
    bool        AudioStatus;
    uint32_t     SubBindVencId;
    uint32_t    BindVencId;
    uint32_t    FileType;
    uint64_t    SplitTime;
    uint32_t    PreTime;
    uint32_t    PostTime;
    float       TimelapseFps;
    uint32_t    TimelapseGop;
    uint32_t    MemoryBufferSec;
    uint32_t    PreallocUnit;
    float       NormalExtendVideoBufferSec;
    float       EventExtendVideoBufferSec;
    float       ExtendOtherBufferSec;
    float       ShortFileMs;
    uint32_t    FocusPos;
    uint32_t    FocusPosLock;
    char        devmodel[32];
} PARAM_RECORD_CHN_ATTR_S;

typedef struct _PARAM_RECORD_ATTR_S {
    uint32_t                    ChnCnt;
    PARAM_RECORD_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_RECORD_ATTR_S;

typedef struct _PARAM_RTSP_CHN_ATTR_S {
    bool        Enable;
    uint32_t    BindVencId;
    int32_t         MaxConn;
    int32_t         Timeout;
    int32_t         Port;
    char        Name[32];
} PARAM_RTSP_CHN_ATTR_S;

typedef struct _PARAM_RTSP_ATTR_S {
    PARAM_RTSP_CHN_ATTR_S   ChnAttrs[MAX_RTSP_CNT];
} PARAM_RTSP_ATTR_S;

typedef struct _PARAM_PIV_CHN_ATTR_S {
    uint32_t BindVencId;
} PARAM_PIV_CHN_ATTR_S;

typedef struct _PARAM_PIV_ATTR_S {
    PARAM_PIV_CHN_ATTR_S    ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_PIV_ATTR_S;

typedef struct _PARAM_THUMBNAIL_CHN_ATTR_S {
    uint32_t            BindVencId;
} PARAM_THUMBNAIL_CHN_ATTR_S;

typedef struct _PARAM_THUMBNAIL_ATTR_S {
    PARAM_THUMBNAIL_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_THUMBNAIL_ATTR_S;

typedef struct _PARAM_SUB_PIC_CHN_ATTR_S {
    uint32_t            BindVencId;
} PARAM_SUB_PIC_CHN_ATTR_S;

typedef struct _PARAM_SUB_PIC_ATTR_S {
    PARAM_SUB_PIC_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_SUB_PIC_ATTR_S;

typedef struct _PARAM_PHOTO_CHN_ATTR_S{
    bool        Enable;
    uint32_t    BindVencId;
    uint32_t    BindVcapId;
    uint32_t    EnableDumpRaw;
} PARAM_PHOTO_CHN_ATTR_S;

typedef struct _PARAM_PHOTO_ATTR_S {
    uint32_t    photoid;
    uint32_t    VprocDev_id;
    PARAM_PHOTO_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_PHOTO_ATTR_S;

typedef struct _PARAM_MD_CHN_ATTR_S{
    bool        Enable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
} PARAM_MD_CHN_ATTR_S;

typedef struct _PARAM_MD_ATTR_S {
    uint32_t    motionSensitivity;
    PARAM_MD_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_MD_ATTR_S;

typedef struct _PARAM_VPSS_ATTR_S {
    VI_VPSS_MODE_S stVIVPSSMode;
    VPSS_MODE_S stVPSSMode;
} PARAM_VPSS_ATTR_S;

#ifdef SERVICES_ADAS_ON
typedef struct _PARAM_ADAS_CHN_ATTR_S{
    bool        Enable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
} PARAM_ADAS_CHN_ATTR_S;

typedef struct _PARAM_ADAS_ATTR_S {
    int32_t adas_cnt;
    PARAM_ADAS_MODEL_ATTR_S stADASModelAttr;
    PARAM_ADAS_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_ADAS_ATTR_S;
#endif

#ifdef SERVICES_QRCODE_ON
typedef struct _PARAM_QRCODE_CHN_ATTR_S {
    bool        Enable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
} PARAM_QRCODE_CHN_ATTR_S;

typedef struct _PARAM_QRCODE_ATTR_S {
    int32_t qrcode_cnt;
    PARAM_QRCODE_CHN_ATTR_S ChnAttrs[MAX_CAMERA_INSTANCES];
} PARAM_QRCODE_ATTR_S;
#endif

typedef struct _PARAM_MEDIA_COMM_S {
    uint32_t                    PowerOnMode;    // 0: record 1:photo 2:playback 3:usb
    PARAM_VPSS_ATTR_S       Vpss;
    PARAM_DISP_ATTR_S       Vo;             // vo config
    PARAM_WND_ATTR_S        Window;         // window config
    MAPI_ACAP_ATTR_S        Ai;             // ai config
    MAPI_AENC_ATTR_S        Aenc;           // aecn config
    MAPI_AO_ATTR_S          Ao;             // ao config
    PARAM_RECORD_ATTR_S     Record;         // record
    PARAM_RTSP_ATTR_S       Rtsp;           // rtsp
    PARAM_PIV_ATTR_S        Piv;            // PIV
    PARAM_THUMBNAIL_ATTR_S  Thumbnail;      // Thumbnail
    PARAM_SUB_PIC_ATTR_S    SubPic;         // SubPic
    PARAM_PHOTO_ATTR_S      Photo;          // photo resolution ratio
    PARAM_MD_ATTR_S         Md;             // video motion detect
    PARAM_FACEP_ATTR_S      Facep;          // face process
#ifdef SERVICES_SPEECH_ON
    SPEECHMNG_PARAM_S       Speech;         // speech config
#endif
#ifdef SERVICES_ADAS_ON
    PARAM_ADAS_ATTR_S       ADAS;           // ADAS
#endif
#ifdef SERVICES_QRCODE_ON
    PARAM_QRCODE_ATTR_S     QRCODE;
#endif
} PARAM_MEDIA_COMM_S;

typedef struct _PARAM_MEDIA_SNS_ATTR_S {
    bool                        SnsEnable;
    MAPI_VCAP_SENSOR_ATTR_T     SnsChnAttr;
    MAPI_VCAP_SENSOR_ATTR_T     SnsSwitchChnAttr;
} PARAM_MEDIA_SNS_ATTR_S;

typedef struct _PARAM_MEDIA_VACP_ATTR_S {
    bool                        VcapEnable;
    uint32_t                    VcapId;
    MAPI_VCAP_CHN_ATTR_T    VcapChnAttr;
    MAPI_VCAP_CHN_ATTR_T    VcapSwitchChnAttr;
} PARAM_MEDIA_VACP_ATTR_S;

typedef struct _MEDIA_VPROC_CHN_ATTR_S {
    uint32_t        VprocChnid;
    bool            VprocChnEnable;
    uint32_t        VprocChnVbCnt;
    uint32_t        VprocChnLowDelayCnt;
    VPSS_CHN_ATTR_S VpssChnAttr;
    VPSS_CHN_BUF_WRAP_S VpssBufWrap;
    VPSS_CROP_INFO_S VpssChnCropInfo;
    ROTATION_E      enRotation;
} MEDIA_VPROC_CHN_ATTR_S;

typedef struct _MEDIA_EXT_VPROC_CHN_ATTR_S {
    bool                    ChnEnable;
    MAPI_EXTCHN_ATTR_T  ChnAttr;
} MEDIA_EXT_VPROC_CHN_ATTR_S;

typedef struct _PARAM_MEDIA_VPROC_GRP_ATTR_S {
    bool                        VprocEnable;
    bool                        BindEnable;
    uint32_t                    Vprocid;
    uint32_t                    VpssDev;
    uint32_t                    ChnCnt;
    MMF_CHN_S                   stSrcChn;
    MMF_CHN_S                   stDestChn;
    VPSS_GRP_ATTR_S             VpssGrpAttr;
    VPSS_CROP_INFO_S            VpssCropInfo;
    MEDIA_VPROC_CHN_ATTR_S      VprocChnAttr[MAPI_VPROC_MAX_CHN_NUM];
    // MEDIA_EXT_VPROC_CHN_ATTR_S  ExtChnAttr[MAPI_VPROC_MAX_CHN_NUM];
} PARAM_MEDIA_VPROC_GRP_ATTR_S;

typedef struct _PARAM_MEDIA_VPROC_ATTR_S {
    uint32_t                    VprocCnt;
    PARAM_MEDIA_VPROC_GRP_ATTR_S   VprocGrpAttr[MAX_VPROC_CNT];
} PARAM_MEDIA_VPROC_ATTR_S;

typedef struct _MEDIA_VENC_CHN_ATTR_S {
    bool                        VencChnEnable;
    uint32_t                    VencId;
    uint32_t                    sbm_enable;
    uint32_t                    BindVprocId;
    uint32_t                    BindVprocChnId;
    float                       framerate;
    MEDIA_VENC_BIND_MODE_E  bindMode;
    MAPI_VENC_CHN_PARAM_T   MapiVencAttr;
} MEDIA_VENC_CHN_ATTR_S;

typedef struct _PARAM_MEDIA_VENC_ATTR_S {
    MEDIA_VENC_CHN_ATTR_S   VencChnAttr[MAX_VENC_CNT];
} PARAM_MEDIA_VENC_ATTR_S;

typedef struct _PARAM_MEDIA_VB_ATTR_S {
    uint32_t                     Poolcnt;
    MAPI_MEDIA_SYS_VB_POOL_T Vbpool[MAPI_VB_POOL_MAX_NUM];
} PARAM_MEDIA_VB_ATTR_S;

typedef struct _PARAM_MEDIA_OSD_ATTR_S {
    bool                 bOsdc;
    int32_t                 OsdCnt;
    MAPI_OSD_ATTR_S OsdAttrs[MEDIA_MAX_OSD_IDX];
} PARAM_MEDIA_OSD_ATTR_S;

typedef struct _PARAM_MEDIA_SPEC_S {
    uint32_t                             MediaMode;
    PARAM_MEDIA_SNS_ATTR_S           SnsAttr;
    PARAM_MEDIA_VACP_ATTR_S          VcapAttr;
    PARAM_MEDIA_VPROC_ATTR_S         VprocAttr;
    PARAM_MEDIA_VENC_ATTR_S          VencAttr;
    PARAM_MEDIA_VB_ATTR_S            Vb;             // vb config
    PARAM_MEDIA_OSD_ATTR_S           Osd;            // osd config
} PARAM_MEDIA_SPEC_S;

typedef struct _PARAM_CAM_MEDIA {
    uint32_t                    CamID;
    uint32_t                    CurMediaMode;
} PARAM_CAM_MEDIA;

/* work mode manager */
typedef struct _PARAM_CAM_CFG {
    bool                        CamEnable;
    bool                        CamIspEnable;
    PARAM_CAM_MEDIA         CamMediaInfo;
    uint32_t                    MediaModeCnt;
    PARAM_MEDIA_SPEC_S      MediaSpec[PARAM_MEDIA_CNT];
} PARAM_CAM_CFG;

typedef struct _PARAM_UVC_PARAM_S {
    uint32_t                VcapId;
    uint32_t                VprocId;
    uint32_t                VprocChnId;
    uint32_t                AcapId;
    UVC_CFG_ATTR_S      UvcCfg;
} PARAM_UVC_PARAM_S;

typedef struct _PARAM_MODE_S {
    uint32_t                CamNum;
    PARAM_VPSS_ATTR_S   Vpss;
    PARAM_CAM_MEDIA     CamMediaInfo[PARAM_MEDIA_CNT];
} PARAM_MODE_S;

typedef struct _PARAM_USB_MODE_S {
    uint32_t                UsbWorkMode;
    PARAM_UVC_PARAM_S   UvcParam;
    USB_STORAGE_CFG_S   StorageCfg;
} PARAM_USB_MODE_S;

typedef struct _PARAM_WORK_MODE_S {
    PARAM_MODE_S        RecordMode;
    PARAM_MODE_S        PhotoMode;
    PARAM_USB_MODE_S    UsbMode;
} PARAM_WORK_MODE_S;

typedef struct _PARAM_CFG_S {
    uint32_t MagicStart;
    PARAM_HEAD_S            Head;
    PARAM_FILEMNG_S         FileMng;
    PARAM_DEVMNG_S          DevMng;
    PARAM_CAM_CFG           CamCfg[MAX_CAMERA_INSTANCES];
    PARAM_WORK_MODE_S       WorkModeCfg;
    PARAM_MEDIA_COMM_S      MediaComm;
    PARAM_MENU_S            Menu;
    uint32_t MagicEnd;
    uint32_t crc32;
} PARAM_CFG_S;

typedef struct _PARAM_CONTEXT_S {
    bool bInit;
    bool bChanged;
    pthread_mutex_t mutexLock;
    PARAM_CFG_S *pstCfg;
} PARAM_CONTEXT_S;

int32_t PARAM_Init(void);
int32_t PARAM_Deinit(void);
int32_t PARAM_Ini2bin(void);
PARAM_CONTEXT_S *PARAM_GetCtx(void);
int32_t PARAM_GetParam(PARAM_CFG_S *param);
int32_t PARAM_Save2Bin(void);
int32_t PARAM_LoadFromBin(const char *path);
int32_t PARAM_LoadFromDefBin(PARAM_CFG_S* param);
int32_t PARAM_LoadFromFlash(hal_flash *flash, char *partition, uint64_t addr, uint64_t len);
int32_t PARAM_SaveParam(void);
void PARAM_SetSaveFlg(void);
int32_t PARAM_GetCamStatus(uint32_t CamId, bool *Param);
int32_t PARAM_GetMediaMode(uint32_t CamId, PARAM_MEDIA_SPEC_S *Param);
int32_t PARAM_GetVbParam(MAPI_MEDIA_SYS_ATTR_T *Param);
int32_t PARAM_GetSensorParam(uint32_t CamId, PARAM_MEDIA_SNS_ATTR_S *Param);
int32_t PARAM_SetSensorParam(uint32_t CamId, PARAM_MEDIA_SNS_ATTR_S *Param);
int32_t PARAM_GetVcapParam(uint32_t CamId, PARAM_MEDIA_VACP_ATTR_S *Param);
int32_t PARAM_SetVcapParam(uint32_t CamId, PARAM_MEDIA_VACP_ATTR_S *Param);
int32_t PARAM_GetVpssModeParam(PARAM_VPSS_ATTR_S *Param);
int32_t PARAM_GetWorkModeParam(PARAM_WORK_MODE_S *Param);
int32_t PARAM_GetVoParam(PARAM_DISP_ATTR_S *Param);
int32_t PARAM_GetWndParam(PARAM_WND_ATTR_S *Param);
int32_t PARAM_GetAiParam(MAPI_ACAP_ATTR_S *Param);
int32_t PARAM_GetAencParam(MAPI_AENC_ATTR_S *Param);
int32_t PARAM_GetAoParam(MAPI_AO_ATTR_S *Param);
int32_t PARAM_GetOsdParam(PARAM_MEDIA_OSD_ATTR_S *Param);
int32_t PARAM_GetStgInfoParam(STG_DEVINFO_S *Param);
int32_t PARAM_GetFileMngParam(PARAM_FILEMNG_S *Param);
int32_t PARAM_GetUsbParam(PARAM_USB_MODE_S *Param);
int32_t PARAM_GetMenuParam(PARAM_MENU_S *Param);
int32_t PARAM_GetDevParam(PARAM_DEVMNG_S *Param);
int32_t PARAM_GetMenuScreenDormantParam(int32_t *Value);
int32_t PARAM_SetParam(PARAM_CFG_S *param);
int32_t PARAM_SetCamStatus(uint32_t CamId, bool Param);
int32_t PARAM_SetCamIspInfoStatus(uint32_t CamId, bool Param);
int32_t PARAM_SetWndParam(PARAM_WND_ATTR_S *Param);
int32_t PARAM_SetOsdParam(PARAM_MEDIA_OSD_ATTR_S *Param);
int32_t PARAM_SetMenuParam(uint32_t CamId, PARAM_MENU_E MenuItem, int32_t Value);
int32_t PARAM_GetKeyMngCfg(KEYMNG_CFG_S *Param);
int32_t PARAM_GetGaugeMngCfg(GAUGEMNG_CFG_S *Param);

int32_t PARAM_GetWifiParam(PARAM_WIFI_S *Param);
int32_t PARAM_SetWifiParam(PARAM_WIFI_S *Param);

int32_t PARAM_GetPWMParam(PARAM_PWM_S *Param);
int32_t PARAM_SetPWMParam(PARAM_PWM_S *Param);

int32_t PARAM_GetFaceParam(PARAM_FACEP_ATTR_S *Param);
int32_t PARAM_SetFaceParam(PARAM_FACEP_ATTR_S *Param);

int32_t PARAM_GetGsensorParam(GSENSORMNG_CFG_S *Param);
int32_t PARAM_SetGsensorParam(GSENSORMNG_CFG_S *Param);
int32_t PARAM_GetKeyTone(int32_t *Value);
int32_t PARAM_GetFatigueDrive(int32_t *Value);
int32_t PARAM_GetMenuSpeedStampParam(int32_t *Value);
int32_t PARAM_GetMenuGPSStampParam(int32_t *Value);
int32_t PARAM_LoadDefaultParamFromFlash(PARAM_CFG_S* param);
int32_t PARAM_GetVideoSizeEnum(int32_t Value, MEDIA_VIDEO_SIZE_E *VideoSize);
int32_t PARAM_SetVENCParam();
void PARAM_SetMediaModeParam(int32_t cammodevule);
int32_t PARAM_GetMediaPhotoSize(PARAM_MEDIA_COMM_S *Param);
int32_t PARAM_GetMediaComm(PARAM_MEDIA_COMM_S *Param);
int32_t PARAM_SetMediaComm(PARAM_MEDIA_COMM_S *Param);
int32_t PARAM_SetBootFirstFlag(uint8_t Value);
void PARAM_GetOsdCarNameParam(char *string_carnum_stamp, MENU_LANGUAGE_E* lang);
void PARAM_SetOsdCarNameParam(char *string_carnum_stamp);
void PARAM_GetMdConfigParam(PARAM_MD_ATTR_S *Md);
int32_t PARAM_GetRecLoop(int32_t *Value);
int32_t PARAM_Get_View_Win(void);
int32_t PARAM_GetAhdDefaultMode(uint32_t CamId, int32_t *mode);
#ifdef SERVICES_SPEECH_ON
int32_t PARAM_GetSpeechParam(SPEECHMNG_PARAM_S *Param);
int32_t PARAM_SetSpeechParam(SPEECHMNG_PARAM_S *Param);
#endif
#ifdef SERVICES_ADAS_ON
void PARAM_GetADASConfigParam(PARAM_ADAS_ATTR_S *ADAS);
#endif
#ifdef SERVICES_QRCODE_ON
void PARAM_GetQRCodeConfigParam(PARAM_QRCODE_ATTR_S *QRCODE);
#endif
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __PARAM_H__ */

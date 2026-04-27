#ifndef __MEDIA_INIT_H__
#define __MEDIA_INIT_H__

#include "param.h"
#include "modetest.h"
#include "appcomm.h"
#include "record_service.h"
#ifdef SERVICES_PHOTO_ON
#include "photo_service.h"
#endif
#ifdef SERVICES_RTSP_ON
#include "rtsp_service.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "player_service.h"
#endif
#ifdef SERVICES_LIVEVIEW_ON
#include "liveview.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "adas_service.h"
#endif

#ifdef SERVICES_QRCODE_ON
#include "qrcode_ser.h"
#endif

#ifdef SERVICES_FACEP_ON
#include "facep_service.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

//media param
typedef struct MEDIA_SYSHANDLE_S {
    MAPI_VCAP_SENSOR_HANDLE_T   sns[MAX_DEV_INSTANCES];
    MAPI_VPROC_HANDLE_T         vproc[MAX_VPROC_CNT];
    MAPI_VENC_HANDLE_T          venchdl[MAX_CAMERA_INSTANCES][MAX_VENC_CNT];
    MAPI_ACAP_HANDLE_T          aihdl;
    MAPI_AENC_HANDLE_T          aenchdl;
    MAPI_DISP_HANDLE_T          dispHdl;
    MAPI_AO_HANDLE_T            aohdl;
} MEDIA_SYSHANDLE_S;

typedef struct CAMERA_SERVICE_S {
    RECORD_SERVICE_HANDLE_T     RecordHdl[MAX_CAMERA_INSTANCES];
    RECORD_SERVICE_PARAM_S      RecordParams[MAX_CAMERA_INSTANCES];
#ifdef SERVICES_PHOTO_ON
    PHOTO_SERVICE_HANDLE_T      PhotoHdl[MAX_CAMERA_INSTANCES];
    PHOTO_SERVICE_PARAM_S       PhotoParams[MAX_CAMERA_INSTANCES];
#endif
#ifdef SERVICES_RTSP_ON
    RTSP_SERVICE_HANDLE_T       RtspHdl[MAX_RTSP_CNT];
    RTSP_SERVICE_PARAM_S        RtspParams[MAX_RTSP_CNT];
#endif
#ifdef SERVICES_ADAS_ON
    ADAS_SERVICE_HANDLE_T       ADASHdl[MAX_CAMERA_INSTANCES];
    ADAS_SERVICE_PARAM_S        ADASParams[MAX_CAMERA_INSTANCES];
#endif
#ifdef SERVICES_PLAYER_ON
    PLAYER_SERVICE_HANDLE_T     PsHdl;
    PLAYER_SERVICE_PARAM_S      PsParam;
#endif
#ifdef SERVICES_LIVEVIEW_ON
    LIVEVIEW_SERVICE_HANDLE_T   LvHdl;
    LIVEVIEW_SERVICE_PARAM_S    LvParams;
#endif
#ifdef SERVICES_QRCODE_ON
    QRCODE_SERVICE_HANDLE_T     QRCodeHdl[MAX_CAMERA_INSTANCES];
    QRCODE_SERVICE_PARAM_S      QRCodeParams[MAX_CAMERA_INSTANCES];
#endif
    MT_HANDLE_T                 MtHdl;
    MT_SERVICE_PARAM_T          MtParam;
#ifdef SERVICES_FACEP_ON
    FACEP_SERVICE_HANDLE_T      FacepHdl[MAX_CAMERA_INSTANCES];
    FACEP_SERVICE_PARAM_S       FacepParams[MAX_CAMERA_INSTANCES];
#endif
} MEDIA_CAMERA_SERVICE_S;

typedef struct MEDIA_PARAM_INIT_S {
    bool                        bInited; /**< Is media module inited */
    MEDIA_SYSHANDLE_S           SysHandle;
    MEDIA_CAMERA_SERVICE_S            SysServices;
} MEDIA_PARAM_INIT_S;

#define APP_MEDIA_EINITIALIZED      APP_APPCOMM_ERR_ID(APP_MOD_MEDIA,APP_EINITIALIZED)/**<Initialized already */
#define APP_MEDIA_ENOTINIT          APP_APPCOMM_ERR_ID(APP_MOD_MEDIA,APP_ENOINIT)/**<Not inited */
#define APP_MEDIA_EINVAL            APP_APPCOMM_ERR_ID(APP_MOD_MEDIA,APP_EINTER)/**<Parameter invalid */
#define APP_MEDIA_EINTER            APP_APPCOMM_ERR_ID(APP_MOD_MEDIA,APP_EINTER)/**<Internal error */
#define APP_MEDIA_ENULLPTR          APP_APPCOMM_ERR_ID(APP_MOD_MEDIA,APP_ERRNO_CUSTOM_BOTTOM + 1)/**<Null pointer */
#define APP_MEDIA_EINPROGRESS       APP_APPCOMM_ERR_ID(APP_MOD_MEDIA,APP_ERRNO_CUSTOM_BOTTOM + 2)/**<Operation now in progress */

/** NULL pointer check */
#define MEDIA_CHECK_POINTER(ptr,errcode,string)\
do{\
    if(NULL == ptr)\
     {\
        CVI_LOGE("%s NULL pointer\n\n",string);\
        return errcode;\
     }\
  }while(0)

/** function ret value check */
#define MEDIA_CHECK_RET(ret,errcode,errstring)\
do{\
    if(0 != ret)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, ret);\
        return errcode;\
    }\
  }while(0)

/**check init, unlock mutex when error */
#define MEDIA_CHECK_CHECK_INIT(retvalue,errcode,errstring)\
do{\
    if(0 == retvalue)\
    {\
        CVI_LOGE("%s failed, s32Ret(0x%08X)\n\n", errstring, retvalue);\
        return errcode;\
    }\
  }while(0)

typedef enum EVENT_SENSOR_E
{
    EVENT_SENSOR_PLUG_STATUS = APPCOMM_EVENT_ID(APP_MOD_MEDIA, 0), /**<plug in or plug out event*/
    EVENT_SENSOR_BUTT
} EVENT_SENSOR_E;

typedef enum SENSOR_PLUG_E
{
    SENSOR_PLUG_IN = 0,
    SENSOR_PLUG_OUT,
    SENSOR_PLUG_BUTT,
} SENSOR_PLUG_E;

int32_t MEDIA_StartAudioInTask(void);
int32_t MEDIA_StopAudioInTask(void);
int32_t MEDIA_VideoInit(void);
int32_t MEDIA_VideoDeInit(void);
int32_t MEDIA_SensorInit(void);
int32_t MEDIA_SensorDeInit(void);
int32_t MEDIA_VbInit(void);
int32_t MEDIA_VbInitPlayBack(void);
int32_t MEDIA_VbDeInit(void);
int32_t MEDIA_VI_VPSS_Mode_Init(void);
int32_t MEDIA_DispInit(bool windowMode);
int32_t MEDIA_DispDeInit(void);
int32_t MEDIA_LiveViewSerInit(void);
int32_t MEDIA_LiveViewSerDeInit(void);
int32_t MEDIA_AiInit(void);
int32_t MEDIA_VencInit(void);
int32_t MEDIA_VencDeInit(void);
int32_t MEDIA_AiDeInit(void);
int32_t MEDIA_AencInit(void);
int32_t MEDIA_AencDeInit(void);
int32_t MEDIA_RecordSerInit(void);
int32_t MEDIA_RecordSerDeInit(void);
int32_t MEDIA_RtspSerInit(void);
int32_t MEDIA_RtspSerDeInit(void);
int32_t MEDIA_AoInit(void);
int32_t MEDIA_PlayBootSound(void);
int32_t MEDIA_AoDeInit(void);
int32_t MEDIA_PlayBackSerInit(void);
int32_t MEDIA_PlayBackSerDeInit(void);
uint32_t MEDIA_Res2RecordMediaMode(int32_t res);
int32_t MEDIA_Res2PhotoMediaMode(int32_t res);
int32_t MEDIA_Size2PhotoMediaMode(int32_t width, int32_t height, const char *desc);
int32_t MEDIA_Size2VideoMediaMode(int32_t width, int32_t height, char *str);
uint32_t MEDIA_AhdResToRecordMediaSize(int32_t res);
uint32_t MEDIA_AhdResToPhotoMediaSize(int32_t res);
int32_t MEDIA_SetAntiFlicker(void);
bool MEDIA_Is_CameraEnabled(int32_t cam_index);
MEDIA_PARAM_INIT_S* MEDIA_GetCtx(void);
int32_t MEDIA_VIDEOMD_Init(void);
int32_t MEDIA_VIDEOMD_DeInit(void);
int32_t MEDIA_APP_RTSP_Init(uint32_t id, char *name);
int32_t MEDIA_SwitchRTSPChanel(uint32_t value, uint32_t id, char *name);
int32_t MEDIA_APP_RTSP_DeInit();
int32_t MEDIA_SensorSwitchInit(PARAM_MEDIA_SNS_ATTR_S *psns_attr,
                            PARAM_MEDIA_VACP_ATTR_S *pvcap_attr);
#ifdef SERVICES_FACEP_ON
int32_t MEDIA_FacepInit(SmileFun smile_callback_fun);
int32_t MEDIA_FacepDeInit(void);
#endif

#ifdef SERVICES_PHOTO_ON
int32_t MEDIA_PhotoVprocInit(void);
int32_t MEDIA_PhotoVprocDeInit(void);
int32_t MEDIA_PhotoSerInit(void);
int32_t MEDIA_PhotoSerDeInit(void);
#endif

#ifdef SERVICES_ADAS_ON
int32_t MEDIA_ADASInit(void);
int32_t MEDIA_ADASDeInit(void);
#endif

#ifdef SERVICES_QRCODE_ON
int32_t MEDIA_QRCodeInit(void);
int32_t MEDIA_QRCodeDeInit(void);
#endif
int32_t MEDIA_SensorDet(void);

#ifdef SERVICES_SUBVIDEO_ON
int32_t MEDIA_StartVideoInTask(void);
int32_t MEDIA_StopVideoInTask(void);
#endif

int32_t MEDIA_SetLightFrequence(void);
int32_t MEDIA_VIVPSSInitPlayBack(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

/*
 * Copyright (C) Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: usb.h
 * Description:
 *   usb module interface decalration
 */

#ifndef __USB_H__
#define __USB_H__

#include <sys/types.h>
#include "appcomm.h"
#include "mapi_define.h"
#include "usb_storage.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define USB_EINVAL               APP_APPCOMM_ERR_ID(APP_MOD_USBMNG, APP_EINVAL)       /**<Invalid argument */
#define USB_ENOTINIT             APP_APPCOMM_ERR_ID(APP_MOD_USBMNG, APP_ENOINIT)      /**<Not inited */
#define USB_EINITIALIZED         APP_APPCOMM_ERR_ID(APP_MOD_USBMNG, APP_EINITIALIZED) /**<reinitialized */


/** usb event define */
#define EVENT_USB_OUT            APPCOMM_EVENT_ID(APP_MOD_USBMNG, 0) /**<usb out */
#define EVENT_USB_INSERT         APPCOMM_EVENT_ID(APP_MOD_USBMNG, 1) /**<usb insert */
#define EVENT_USB_UVC_READY      APPCOMM_EVENT_ID(APP_MOD_USBMNG, 2) /**<uvc mode: pc ready */
#define EVENT_USB_STORAGE_READY  APPCOMM_EVENT_ID(APP_MOD_USBMNG, 3) /**<storage mode: pc ready */
#define EVENT_USB_HOSTUVC_READY  APPCOMM_EVENT_ID(APP_MOD_USBMNG, 4) /**<host uvc mode: camera ready */
#define EVENT_USB_HOSTUVC_PC     APPCOMM_EVENT_ID(APP_MOD_USBMNG, 5) /**<usb source pc */
#define EVENT_USB_HOSTUVC_HEAD   APPCOMM_EVENT_ID(APP_MOD_USBMNG, 6) /**<usb source head */
#define EVENT_USB_PC_INSERT      APPCOMM_EVENT_ID(APP_MOD_USBMNG, 7) /**<pc usb insert */

#define USB_HOSTUVC_CAP_MAXCNT            24U
#define USB_HOSTUVC_LIMITLESS_FRAME_COUNT (-1)

/** ---------------- UVC Define Begin ---------------- */

/** uvc stream format enum */
typedef enum UVC_STREAM_FORMAT_E
{
    UVC_STREAM_FORMAT_YUV420 = 0,
    UVC_STREAM_FORMAT_MJPEG,
    UVC_STREAM_FORMAT_H264,
    UVC_STREAM_FORMAT_BUTT
} UVC_STREAM_FORMAT_E;

/** uvc video mode enum, including resolution and framerate
   * should be a subset of driver capabilities */
typedef enum UVC_VIDEOMODE_E
{
    UVC_VIDEOMODE_VGA30 = 0, /**<640x360   30fps */
    UVC_VIDEOMODE_720P30,    /**<1280x720  30fps */
    UVC_VIDEOMODE_1080P30,   /**<1920x1080 30fps */
    UVC_VIDEOMODE_4K30,      /**<3840x2160 30fps */
    UVC_VIDEOMODE_BUTT
} UVC_VIDEOMODE_E;

typedef struct UVC_VIDEOATTR_S
{
    uint32_t u32BitRate;
    UVC_VIDEOMODE_E enMode;
} UVC_VIDEOATTR_S;

/** uvc format capabilities, including videomode cnt and list */
typedef struct UVC_FORMAT_CAP_S
{
    uint32_t u32Cnt;
    UVC_VIDEOMODE_E enDefMode;
    UVC_VIDEOATTR_S astModes[UVC_VIDEOMODE_BUTT];
} UVC_FORMAT_CAP_S;

/** uvc device capabilities, including all format */
typedef struct UVC_DEVICE_CAP_S
{
    UVC_FORMAT_CAP_S astFmtCaps[UVC_STREAM_FORMAT_BUTT];
} UVC_DEVICE_CAP_S;

/** uvc data source handle */
typedef struct UVC_DATA_SOURCE_S
{
    MAPI_HANDLE_T VcapHdl; /**<vcap handle */
    MAPI_HANDLE_T VprocHdl; /**<vproc handle */
    MAPI_HANDLE_T VencHdl;  /**<venc handle */
    MAPI_HANDLE_T AcapHdl;  /**<audio handle */
    uint32_t          VprocChnId; /**<vproc chn id */
} UVC_DATA_SOURCE_S;

/** uvc data source handle */
typedef struct UVC_BUFFER_CFG_S
{
    uint32_t u32BufCnt;
    uint32_t u32BufSize;
} UVC_BUFFER_CFG_S;

/** uvc configure */
typedef struct UVC_CFG_ATTR_S
{
    char    szDevPath[APPCOMM_MAX_PATH_LEN]; /**<uvc device path */
    UVC_DEVICE_CAP_S  stDevCap;
    UVC_BUFFER_CFG_S stBufferCfg;
} UVC_CFG_ATTR_S;

typedef struct UVC_CFG_S
{
    UVC_CFG_ATTR_S    attr;
    UVC_DATA_SOURCE_S stDataSource;
} UVC_CFG_S;

/** ---------------- UVC Define End ---------------- */

/** usb event information */
typedef struct USB_EVENT_INFO_S
{
    int32_t     s32EventId;
    char        szPayload[APPCOMM_MAX_PATH_LEN];
} USB_EVENT_INFO_S;

/** usb state callback */
typedef int32_t (*USB_EVENTPROC_CALLBACK_FN_PTR)(const USB_EVENT_INFO_S *pstEventInfo);

/** usb state enum, including uvc/storage state */
typedef enum USB_STATE_E
{
    USB_STATE_OUT = 0,          /**<usb out */
    USB_STATE_INSERT,           /**<usb insert */
    USB_STATE_PC_INSERT,        /**<PC usb insert */
    USB_STATE_UVC_READY,        /**<uvc driver load ready */
    USB_STATE_UVC_PC_READY,     /**<uvc pc interaction ready */
    USB_STATE_UVC_MEDIA_READY,  /**<uvc media ready */
    USB_STATE_STORAGE_READY,    /**<storage driver load ready */
    USB_STATE_STORAGE_PC_READY, /**<storage pc interaction ready */
    USB_STATE_STORAGE_SD_READY, /**<storage sd device ready */
    USB_STATE_HOSTUVC_READY,        /* host uvc driver load ready */
    USB_STATE_HOSTUVC_CAMERA_READY, /* host uvc camera ready */
    USB_STATE_HOSTUVC_MEDIA_READY,  /* host uvc media ready */
    USB_STATE_BUTT
} USB_STATE_E;

/** usb mode enum */
typedef enum USB_MODE_E
{
    USB_MODE_CHARGE = 0,
    USB_MODE_UVC,
    USB_MODE_STORAGE,
    USB_MODE_HOSTUVC, /* host uvc not suport */
    USB_MODE_BUTT
} USB_MODE_E;

typedef int32_t (*USB_GET_STORAGE_STATE_FN_PTR)(void* pvPrivData);

/** usb configure */
typedef struct USB_CFG_S
{
    UVC_CFG_S stUvcCfg;
    USB_STORAGE_CFG_S stStorageCfg;
    USB_EVENTPROC_CALLBACK_FN_PTR pfnEventProc;
    USB_GET_STORAGE_STATE_FN_PTR pfnGetStorageState;
} USB_CFG_S;

/** usb connect power source */
typedef enum tagUSB_POWER_SOURCE_E
{
    USB_POWER_SOURCE_NONE = 0,
    USB_POWER_SOURCE_PC,
    USB_POWER_SOURCE_POWER,
    USB_POWER_SOURCE_BUTT
} USB_POWER_SOURCE_E;

int32_t USB_Init(const USB_CFG_S* pstCfg);
int32_t USB_Deinit(void);
int32_t USB_SetMode(USB_MODE_E enMode);
int32_t USB_SetUvcCfg(const UVC_CFG_S* pstCfg);
int32_t USB_SetStorageCfg(const USB_STORAGE_CFG_S* pstCfg);
int32_t USB_GetUvcCfg(USB_CFG_S* pstCfg);
int32_t USB_GetStorageCfg(USB_STORAGE_CFG_S* pstCfg);
void USB_CheckPower_Soure(USB_POWER_SOURCE_E* PowerSoureState);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#endif /* __USB_H__ */
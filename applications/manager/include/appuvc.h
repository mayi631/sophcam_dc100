/*
 * Copyright (C) Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: uvc.h
 * Description:
 *   uvc module interface decalration
 */

#ifndef __UVC_H__
#define __UVC_H__

#include "appcomm.h"
#include "usb.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** uvc stream attribute */
typedef struct tagUVC_STREAM_ATTR_S {
    UVC_STREAM_FORMAT_E enFormat;
    u_int32_t u32Width;
    u_int32_t u32Height;
    u_int32_t u32Fps;
    u_int32_t u32BitRate;
} UVC_STREAM_ATTR_S;

/* UVC Context */
typedef struct tagUVC_CONTEXT_S {
    char szDevPath[APPCOMM_MAX_PATH_LEN];
    bool bRun;
    bool bPCConnect;
    pthread_t TskId;
    pthread_t Tsk2Id;
} UVC_CONTEXT_S;

int32_t UVC_Init(const UVC_DEVICE_CAP_S *pstCap, const UVC_DATA_SOURCE_S *pstDataSrc,
                 UVC_BUFFER_CFG_S *pstBufferCfg);
int32_t UVC_Deinit(void);
int32_t UVC_Start(const char *pDevPath);
int32_t UVC_Stop(void);

int32_t UVC_STREAM_SetAttr(UVC_STREAM_ATTR_S *pstAttr);
int32_t UVC_STREAM_Start(void);
int32_t UVC_STREAM_Stop(void);
int32_t UVC_STREAM_ReqIDR(void);
int32_t UVC_STREAM_CopyBitStream(void *dst);
int32_t USB_GetMode(USB_MODE_E *penMode);
int32_t USB_GetState(USB_STATE_E *penState);

UVC_CONTEXT_S *UVC_GetCtx(void);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#endif /* __UVC_H__ */
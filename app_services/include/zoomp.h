#ifndef __ZOOMP_H__
#define __ZOOMP_H__

/* zoom process */

#include "cvi_type.h"
#include "mapi.h"

#define ZOOM_STEP_PER_RADIO 64
#define ZOOM_MAX_RADIO 20
#define ZOOM_MAX_RATIO 8.0f

CVI_S32 ZOOMP_Init(RECT_S org_win);
CVI_S32 ZOOMP_DeInit(CVI_VOID);
CVI_U32 ZOOMP_Is_Init(CVI_VOID);
CVI_S32 ZOOMP_Reset(CVI_VOID);
CVI_S32 ZOOMP_GetCropInfo(RECT_S in, RECT_S* out, float radio);

#endif

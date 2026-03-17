#ifndef __MAPI_OSD_H__
#define __MAPI_OSD_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "string.h"
#include "cvi_comm_region.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAPI_OSD_MAX_CNT  (16)
#define MAPI_OSD_MAX_DISP_CNT (1)
#define MAPI_OSD_MAX_STR_LEN  (120)

#define CVI_MAPI_OSDC_MAX_REC_CNT  (8)
#define CVI_MAPI_OSDC_MAX_LINE_CNT  (2)

typedef int (*MAPI_OSD_GETFONTMOD_CALLBACK_FN_PTR)(char* Character,uint8_t** FontMod,int* FontModLen);

typedef enum MAPI_OSD_BIND_MOD_E {
    MAPI_OSD_BINDMOD_VPROC = 0,
    MAPI_OSD_BINDMOD_DISP,
    MAPI_OSD_BINDMOD_BUTT
} MAPI_OSD_BIND_MOD_E;

typedef enum MAPI_OSD_COORDINATE_E {
    MAPI_OSD_COORDINATE_RATIO_COOR = 0,
    MAPI_OSD_COORDINATE_ABS_COOR
} MAPI_OSD_COORDINATE_E;

typedef enum MAPI_OSD_TYPE_E {
    MAPI_OSD_TYPE_TIME = 0,
    MAPI_OSD_TYPE_STRING,
    MAPI_OSD_TYPE_BITMAP,
    MAPI_OSD_TYPE_CIRCLE,
    MAPI_OSD_TYPE_OBJECT,
    MAPI_OSD_TYPE_BUTT
} MAPI_OSD_TYPE_E;

typedef enum MAPI_OSD_TIMEFMT_E {
    MAPI_OSD_TIMEFMT_YMD24H = 0, /**< eg. 2017-03-10 23:00:59 */
    MAPI_OSD_TIMEFMT_BUTT
} MAPI_OSD_TIMEFMT_E;

typedef struct MAPI_OSD_TIME_CONTENT_S {
    MAPI_OSD_TIMEFMT_E enTimeFmt;
    SIZE_S  stFontSize;
    uint32_t  u32BgColor;
}MAPI_OSD_TIME_CONTENT_S;

typedef struct MAPI_OSD_STR_CONTENT_S {
    char szStr[MAPI_OSD_MAX_STR_LEN];
    SIZE_S  stFontSize;
    uint32_t  u32BgColor;
}MAPI_OSD_STR_CONTENT_S;

typedef struct MAPI_OSD_CIRCLE_CONTENT_S {
    uint32_t u32Width;
    uint32_t u32Height;
}MAPI_OSD_CIRCLE_CONTENT_S;

typedef struct MAPI_OSD_DISP_ATTR_S {
    bool bShow;
    MAPI_OSD_BIND_MOD_E enBindedMod;
    uint32_t ModHdl;
    uint32_t ChnHdl;
    MAPI_OSD_COORDINATE_E enCoordinate;
    POINT_S stStartPos;
    uint32_t u32Batch;
    RGN_CMPR_TYPE_E enRgnCmprType;
    union {
        MAPI_OSD_TIME_CONTENT_S stTimeContent;
        MAPI_OSD_STR_CONTENT_S stStrContent;
        MAPI_OSD_CIRCLE_CONTENT_S stCircleContent;
        BITMAP_S stBitmapContent;
    };
    uint32_t maxlen;
    unsigned long long int u64BitmapPhyAddr;
    void *pBitmapVirAddr;
} MAPI_OSD_DISP_ATTR_S;

typedef struct _CVI_MAPI_OSD_OBJECTINFO_S {
    int32_t camid;
    uint32_t rec_cnt;
    uint32_t line_cnt;
    int32_t rec_coordinates[CVI_MAPI_OSDC_MAX_REC_CNT * 4]; // [x1,y1,x2,y2]
    int32_t line_coordinates[CVI_MAPI_OSDC_MAX_LINE_CNT * 4]; // [x1,y1,x2,y2]
}CVI_MAPI_OSD_OBJECTINFO_S;

typedef struct cviMAPI_OSD_OBJECT_CONTENT_S {
    uint32_t u32Width;
    uint32_t u32Height;
    CVI_MAPI_OSD_OBJECTINFO_S objectInfo;
}CVI_MAPI_OSD_OBJECT_CONTENT_S;

typedef struct MAPI_OSD_CONTENT_S {
    MAPI_OSD_TYPE_E enType;
    uint32_t  u32Color;
    union {
        MAPI_OSD_TIME_CONTENT_S stTimeContent;
        MAPI_OSD_STR_CONTENT_S stStrContent;
        MAPI_OSD_CIRCLE_CONTENT_S stCircleContent;
        CVI_MAPI_OSD_OBJECT_CONTENT_S stObjectContent;
        BITMAP_S stBitmapContent;
    };
} MAPI_OSD_CONTENT_S;

typedef struct MAPI_OSD_ATTR_S {
    uint32_t u32DispNum;
    bool bFlip;
    bool bMirror;
    MAPI_OSD_DISP_ATTR_S astDispAttr[MAPI_OSD_MAX_DISP_CNT];
    MAPI_OSD_CONTENT_S stContent;
} MAPI_OSD_ATTR_S;

typedef struct MAPI_OSD_FONTS_S {
    uint32_t u32FontWidth;
    uint32_t u32FontHeight;
} MAPI_OSD_FONTS_S;

int MAPI_OSD_Init(const MAPI_OSD_FONTS_S* pstFonts, bool bOsdcEnable);
int MAPI_OSD_Deinit(void);
int MAPI_OSD_SetAttr(int s32OsdIdx, MAPI_OSD_ATTR_S* pstAttr);
int MAPI_OSD_GetAttr(int s32OsdIdx, MAPI_OSD_ATTR_S* pstAttr);
int MAPI_OSD_Start(int s32OsdIdx);
int MAPI_OSD_Stop(int s32OsdIdx);
int MAPI_OSD_Batch(uint32_t u32Batch, bool bShow);
int MAPI_OSD_Show(int32_t s32OsdIdx, uint32_t u32DispIdx, bool bShow);

#ifdef __cplusplus
}
#endif

#endif
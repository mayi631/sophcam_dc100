#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
// #include "cvi_common.h"
#include "cvi_region.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "mapi_define.h"
#include "cvi_sys.h"
#include "mapi_osd.h"
#include "cvi_log.h"
#include "mapi_fontmod.h"
#include "mapi_internal.h"

#define IsASCII(a)              (((a) >= 0x00 && (a) <= 0x7F) ? 1 : 0)
#define BYTE_BITS               8
#define NOASCII_CHARACTER_BYTES 2 /*   Number of bytes occupied by each Chinese character   */

/*   OSD Font Step In Lib, in bytes   */
#define OSD_LIB_FONT_W                 (g_stOsdFonts.u32FontWidth)
#define OSD_LIB_FONT_H                 (g_stOsdFonts.u32FontHeight)
#define OSD_LIB_FONT_STEP              (OSD_LIB_FONT_W * OSD_LIB_FONT_H / BYTE_BITS)
#define OSD_RATIO_COORDINATE_MAX_VALUE 100

#define MAPI_ADEC_WAIT_TIME_CHANGE_USLEEP_TIME (10 * 1000)
#define MAPI_ADEC_WAIT_UPDATE_TIME_USLEEP_TIME (500 * 1000)

typedef struct tagOSD_PARAM_S {
    MAPI_OSD_ATTR_S stAttr;
    SIZE_S stMaxSize;
    pthread_mutex_t mutexLock;
    bool bInit; /*   OSD Attribute Set or not, Canbe modified only MAPI_OSD_SetAttr   */
    bool bStart;   /*   OSD start/stop Flag, Canbe modified only by MAPI_OSD_Start/MAPI_OSD_Stop   */
    bool bOsdcEnable; /*for phobos, use osdc*/
    char szStr[MAPI_OSD_MAX_STR_LEN];
    RGN_TYPE_E enRgnType;
} OSD_PARAM_S;

/*  * OSD Time Update Runing Flag
  Canbe modified only by PDT_OSD_Init/PDT_OSD_DeInit   */
static bool g_bOsdTimeRun = false;

/*  * OSD Module Init Flag
  Canbe modified only by PDT_OSD_Init/PDT_OSD_DeInit   */
static bool g_bOsdInitFlg = false;

#define OSD_COMPRESSED_SIZE 262144 //256K, Store the compressed memory size of OSD, 256K is enough for OSD

/*   OSD Fonts Lib, inited by PDT_OSD_Init   */
static MAPI_OSD_FONTS_S g_stOsdFonts;

/*   OSD Parameter Array   */
static OSD_PARAM_S g_astOSDParam[MAPI_OSD_MAX_CNT];

/*   Time OSD Update Task Thread ID, created by PDT_OSD_Init, destroyed by MAPI_OSD_DeInit   */
static pthread_t g_pOsdTimeTskId = 0;

static void OSD_GetRgnAttr(RGN_HANDLE RgnHdl, const MAPI_OSD_ATTR_S* pstAttr, RGN_ATTR_S* pstRgnAttr);

static int MAPI_OSD_GetFontMod(CVI_CHAR* Character,uint8_t** FontMod,int* FontModLen)
{
    /* Get Font Mod in GB2312 Fontlib*/
    uint32_t offset = 0;
	uint32_t areacode = 0;
	uint32_t bitcode = 0;
	if(IsASCII(Character[0])) {
		areacode = 3;
		bitcode = (uint32_t)((uint8_t)Character[0]-0x20);
	} else {
		areacode = (uint32_t)((uint8_t)Character[0]-0xA0);
		bitcode = (uint32_t)((uint8_t)Character[1]-0xA0);
	}
	offset = (94*(areacode-1)+(bitcode-1))*(OSD_LIB_FONT_W*OSD_LIB_FONT_H/8);
	*FontMod = (uint8_t*)g_fontLib+offset;
	*FontModLen = OSD_LIB_FONT_W*OSD_LIB_FONT_H/8;
	return CVI_SUCCESS;
}

static int OSD_GetNonASCNum(char* string, int len)
{
    int i;
    int n = 0;
    for (i = 0; i < len; i++) {
        if (string[i] == '\0') {
            break;
        }
        if (!IsASCII(string[i])) {
            i++;
            n++;
        }
    }

    return n;
}

static void OSD_GetTimeStr(const struct tm* pstTime, MAPI_OSD_TIMEFMT_E enFmt, char* pazStr, int s32Len)
{
    /*   Get Time   */
    time_t nowTime;
    struct tm stTime = {
        0,
    };

    if (!pstTime) {
        time(&nowTime);
        localtime_r(&nowTime, &stTime);
        pstTime = &stTime;
    }

    /*   Generate Time String   */
    switch (enFmt) {
        case MAPI_OSD_TIMEFMT_YMD24H:
        default:
            snprintf(pazStr, s32Len, "%04d-%02d-%02d %02d:%02d:%02d",
                     pstTime->tm_year + 1900, pstTime->tm_mon + 1, pstTime->tm_mday, // 1900 is struct tm year reference
                     pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec);
            break;
    }

    return;
}

static int OSD_Ratio2Absolute(MMF_CHN_S stChn, const POINT_S* pstRatioCoor, POINT_S* pstAbsCoor)
{
    int s32Ret = MAPI_SUCCESS;
    SIZE_S stImageSize;

    if (pstRatioCoor->s32X < 0 || pstRatioCoor->s32X > OSD_RATIO_COORDINATE_MAX_VALUE ||
        pstRatioCoor->s32Y < 0 || pstRatioCoor->s32Y > OSD_RATIO_COORDINATE_MAX_VALUE) {
        CVI_LOGE("invalide Ratio coordinate(%d,%d)\n", pstRatioCoor->s32X, pstRatioCoor->s32Y);
        return MAPI_ERR_FAILURE;
    }

    switch (stChn.enModId) {

        case CVI_ID_VPSS: {
            VPSS_CHN_ATTR_S stChnAttr;
            s32Ret = CVI_VPSS_GetChnAttr(stChn.s32DevId, stChn.s32ChnId, &stChnAttr);

            if (s32Ret != MAPI_SUCCESS) {
                CVI_LOGE("VPSS_GetChnAttr(%d, %d) fail,Error Code: [0x%08X]\n",
                    stChn.s32DevId, stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize.u32Width = stChnAttr.u32Width;
            stImageSize.u32Height = stChnAttr.u32Height;
            break;
        }

        case CVI_ID_VO: {
            VO_CHN_ATTR_S stChnAttr;
            s32Ret = CVI_VO_GetChnAttr(stChn.s32DevId, stChn.s32ChnId, &stChnAttr);

            if (s32Ret != MAPI_SUCCESS) {
                CVI_LOGE("VO_GetChnAttr(%d,%d) fail,Error Code: [0x%08X]\n",
                    stChn.s32DevId, stChn.s32ChnId, s32Ret);
                return s32Ret;
            }

            stImageSize.u32Width = stChnAttr.stRect.u32Width;
            stImageSize.u32Height = stChnAttr.stRect.u32Height;
            break;
        }

        default:
            CVI_LOGE("invalide mode id [%d]\n", stChn.enModId);
            return MAPI_ERR_FAILURE;
    }

    pstAbsCoor->s32X = MAPI_ALIGN(stImageSize.u32Width * pstRatioCoor->s32X /
        OSD_RATIO_COORDINATE_MAX_VALUE, 2); // 2: s32X align at 2 pixel
    pstAbsCoor->s32Y = MAPI_ALIGN(stImageSize.u32Height * pstRatioCoor->s32Y /
        OSD_RATIO_COORDINATE_MAX_VALUE, 2); // 2: s32X align at 2 pixel
    return MAPI_SUCCESS;
}

static int OSD_UpdateDisplay(RGN_HANDLE RgnHdl, const MAPI_OSD_ATTR_S* pstAttr)
{
    int s32Ret;
    uint32_t u32DispIdx;
    RGN_CHN_ATTR_S stRgnChnAttr;
    MMF_CHN_S stChn;
    memset(&stRgnChnAttr, 0x0, sizeof(RGN_CHN_ATTR_S));
    for (u32DispIdx = 0; u32DispIdx < pstAttr->u32DispNum; ++u32DispIdx) {
        stChn.s32DevId = pstAttr->astDispAttr[u32DispIdx].ModHdl;
        stChn.s32ChnId = pstAttr->astDispAttr[u32DispIdx].ChnHdl;
        switch (pstAttr->astDispAttr[u32DispIdx].enBindedMod) {
            case MAPI_OSD_BINDMOD_VPROC:
                stChn.enModId = CVI_ID_VPSS;
                break;

            case MAPI_OSD_BINDMOD_DISP:
                stChn.enModId = CVI_ID_VO;
                break;

            default:
                CVI_LOGE("RgnHdl[%d] invalide bind mode [%d]\n",RgnHdl, pstAttr->astDispAttr[u32DispIdx].enBindedMod);
                return MAPI_ERR_FAILURE;
        }

        s32Ret = CVI_RGN_GetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);

        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("RGN_GetDisplayAttr fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
                RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
            return s32Ret;
        }

        stRgnChnAttr.bShow = pstAttr->astDispAttr[u32DispIdx].bShow;

        POINT_S stStartPos;

        if (pstAttr->astDispAttr[u32DispIdx].enCoordinate == MAPI_OSD_COORDINATE_RATIO_COOR) {
            s32Ret = OSD_Ratio2Absolute(stChn, &pstAttr->astDispAttr[u32DispIdx].stStartPos, &stStartPos);

            if (s32Ret != MAPI_SUCCESS) {
                CVI_LOGE("OSD_Ratio2Absolute fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
                return s32Ret;
            }
        } else {
            stStartPos = pstAttr->astDispAttr[u32DispIdx].stStartPos;
        }

        if (stRgnChnAttr.enType == OVERLAY_RGN) {
            stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = stStartPos.s32X;
            stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = stStartPos.s32Y;
        } else if (stRgnChnAttr.enType == COVER_RGN) {
            stRgnChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT;
            stRgnChnAttr.unChnAttr.stCoverExChn.stRect.u32Width = pstAttr->stContent.stCircleContent.u32Width;
            stRgnChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = pstAttr->stContent.stCircleContent.u32Height;
            stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32X = stStartPos.s32X;
            stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = stStartPos.s32X;
        }

        s32Ret = CVI_RGN_SetDisplayAttr(RgnHdl, &stChn, &stRgnChnAttr);

        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("RGN_SetDisplayAttr fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
                RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
            return s32Ret;
        }
    }

    return MAPI_SUCCESS;
}

static int OSD_UpdateTextBitmap(RGN_HANDLE RgnHdl, const MAPI_OSD_CONTENT_S* pstContent)
{
    int s32Ret;
    uint32_t u32CanvasWidth,u32CanvasHeight,u32BgColor;
    SIZE_S stFontSize;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[RgnHdl];
    int s32StrLen = strnlen(pstOsdParam->szStr, MAPI_OSD_MAX_STR_LEN);
    int NonASCNum = OSD_GetNonASCNum(pstOsdParam->szStr, s32StrLen);
    if (pstContent->enType == MAPI_OSD_TYPE_STRING) {
        u32CanvasWidth =
            pstContent->stStrContent.stFontSize.u32Width * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
        u32CanvasHeight = pstContent->stStrContent.stFontSize.u32Height;
        stFontSize.u32Width = pstContent->stStrContent.stFontSize.u32Width;
        stFontSize.u32Height = pstContent->stStrContent.stFontSize.u32Height;
        u32BgColor = pstContent->stStrContent.u32BgColor;
    } else {
        u32CanvasWidth =
            pstContent->stTimeContent.stFontSize.u32Width * (s32StrLen - NonASCNum * (NOASCII_CHARACTER_BYTES - 1));
        u32CanvasHeight = pstContent->stTimeContent.stFontSize.u32Height;
        stFontSize.u32Width = pstContent->stTimeContent.stFontSize.u32Width;
        stFontSize.u32Height = pstContent->stTimeContent.stFontSize.u32Height;
        u32BgColor = pstContent->stTimeContent.u32BgColor;
    }
    RGN_CANVAS_INFO_S stCanvasInfo;
    memset(&stCanvasInfo, 0x0, sizeof(RGN_CANVAS_INFO_S));
    s32Ret = CVI_RGN_GetCanvasInfo(RgnHdl, &stCanvasInfo);

    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_GetCanvasInfo fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    /*   Generate Bitmap   */
    uint16_t *puBmData = CVI_SYS_Mmap(stCanvasInfo.u64PhyAddr, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
    uint32_t u32BmRow, u32BmCol; /*   Bitmap Row/Col Index   */
    for (u32BmRow = 0; u32BmRow < u32CanvasHeight; ++u32BmRow) {
        int NonASCShow = 0;
        for (u32BmCol = 0; u32BmCol < u32CanvasWidth; ++u32BmCol) {
            /*   Bitmap Data Offset for the point   */
            int s32BmDataIdx = u32BmRow * stCanvasInfo.u32Stride / 2 + u32BmCol;
            /*   Character Index in Text String   */
            int s32CharIdx = u32BmCol / stFontSize.u32Width;
            int s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
            if (NonASCNum > 0 && s32CharIdx > 0) {
                NonASCShow = OSD_GetNonASCNum(pstOsdParam->szStr, s32StringIdx);
                s32StringIdx = s32CharIdx + NonASCShow * (NOASCII_CHARACTER_BYTES - 1);
            }
            /*   Point Row/Col Index in Character   */
            int s32CharCol = (u32BmCol - (stFontSize.u32Width * s32CharIdx)) * OSD_LIB_FONT_W /
                                stFontSize.u32Width;
            int s32CharRow = u32BmRow * OSD_LIB_FONT_H / stFontSize.u32Height;
            int s32HexOffset = s32CharRow * OSD_LIB_FONT_W / BYTE_BITS + s32CharCol / BYTE_BITS;
            int s32BitOffset = s32CharCol % BYTE_BITS;
            uint8_t *FontMod = NULL;
            int FontModLen = 0;
            if (MAPI_SUCCESS == MAPI_OSD_GetFontMod(&pstOsdParam->szStr[s32StringIdx], &FontMod, &FontModLen)) {
                if (FontMod != NULL && s32HexOffset < FontModLen) {
                    uint8_t temp = FontMod[s32HexOffset];
                    if ((temp >> ((BYTE_BITS - 1) - s32BitOffset)) & 0x1) {
                        puBmData[s32BmDataIdx] = (uint16_t)pstContent->u32Color;
                    } else {
                        puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
                    }
                    continue;
                }
            }
            CVI_SYS_Munmap(puBmData, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
            CVI_LOGE("GetFontMod Fail\n");
            return MAPI_ERR_FAILURE;
        }

        for (u32BmCol = u32CanvasWidth; u32BmCol < g_astOSDParam[RgnHdl].stMaxSize.u32Width; ++u32BmCol) {
            int s32BmDataIdx = u32BmRow * stCanvasInfo.u32Stride / 2 + u32BmCol;
            puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
        }
    }

    for (u32BmRow = u32CanvasHeight; u32BmRow < g_astOSDParam[RgnHdl].stMaxSize.u32Height; ++u32BmRow) {
        for (u32BmCol = 0; u32BmCol < g_astOSDParam[RgnHdl].stMaxSize.u32Width; ++u32BmCol) {
            int s32BmDataIdx = u32BmRow * stCanvasInfo.u32Stride / 2 + u32BmCol;
            puBmData[s32BmDataIdx] = (uint16_t)u32BgColor;
        }
    }

    stCanvasInfo.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    stCanvasInfo.stSize.u32Width = u32CanvasWidth;
    stCanvasInfo.stSize.u32Height = u32CanvasHeight;

    uint32_t u32DataSize = stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height;
    if (g_astOSDParam[RgnHdl].bOsdcEnable == true) {
        uint32_t u32DispIdx;
        for (u32DispIdx = 0; u32DispIdx < pstOsdParam->stAttr.u32DispNum; ++u32DispIdx) {
            if (pstOsdParam->stAttr.astDispAttr[u32DispIdx].enRgnCmprType == RGN_CMPR_BIT_MAP) {
                if (u32DataSize > pstOsdParam->stAttr.astDispAttr[u32DispIdx].maxlen) {
                    if (pstOsdParam->stAttr.astDispAttr[u32DispIdx].maxlen) {
                        CVI_SYS_IonFree(pstOsdParam->stAttr.astDispAttr[u32DispIdx].u64BitmapPhyAddr, pstOsdParam->stAttr.astDispAttr[u32DispIdx].pBitmapVirAddr);
                        pstOsdParam->stAttr.astDispAttr[u32DispIdx].u64BitmapPhyAddr = (unsigned long long int)0;
                        pstOsdParam->stAttr.astDispAttr[u32DispIdx].pBitmapVirAddr = NULL;
                    }
                    s32Ret = CVI_SYS_IonAlloc(&(pstOsdParam->stAttr.astDispAttr[u32DispIdx].u64BitmapPhyAddr), (void **)&(pstOsdParam->stAttr.astDispAttr[u32DispIdx].pBitmapVirAddr),"rgn_cmpr_time", u32DataSize);
                    if (s32Ret != 0) {
                        CVI_LOGE("CVI_SYS_IonAlloc fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
                        return s32Ret;
                    }
                    pstOsdParam->stAttr.astDispAttr[u32DispIdx].maxlen = u32DataSize;
                }

                RGN_CMPR_OBJ_ATTR_S *pstObjAttr;
                memcpy(pstOsdParam->stAttr.astDispAttr[u32DispIdx].pBitmapVirAddr, puBmData, u32DataSize);
                pstObjAttr = stCanvasInfo.pstObjAttr;
                pstObjAttr[u32DispIdx].stBitmap.stRect.s32X = pstOsdParam->stAttr.astDispAttr[u32DispIdx].stStartPos.s32X;
                pstObjAttr[u32DispIdx].stBitmap.stRect.s32Y = pstOsdParam->stAttr.astDispAttr[u32DispIdx].stStartPos.s32Y;
                pstObjAttr[u32DispIdx].stBitmap.stRect.u32Width = stCanvasInfo.u32Stride;
                pstObjAttr[u32DispIdx].stBitmap.stRect.u32Height = stCanvasInfo.stSize.u32Height;
                #ifdef CHIP_184X
                pstObjAttr[u32DispIdx].stBitmap.u64BitmapPAddr = (uint32_t)pstOsdParam->stAttr.astDispAttr[u32DispIdx].u64BitmapPhyAddr;
                #else
                pstObjAttr[u32DispIdx].stBitmap.u32BitmapPAddr = (uint32_t)pstOsdParam->stAttr.astDispAttr[u32DispIdx].u64BitmapPhyAddr;
                #endif
            }

        }
        RGN_CANVAS_CMPR_ATTR_S * pstCanvasCmprAttr;
        pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
        RGN_ATTR_S stRegion;
        memset(&stRegion, 0x0, sizeof(RGN_ATTR_S));
        OSD_GetRgnAttr(RgnHdl, &pstOsdParam->stAttr, &stRegion);
        pstCanvasCmprAttr->u32Width = stRegion.unAttr.stOverlay.stSize.u32Width;
        pstCanvasCmprAttr->u32Height = stRegion.unAttr.stOverlay.stSize.u32Height;
        pstCanvasCmprAttr->u32BgColor =  stRegion.unAttr.stOverlay.u32BgColor;
        pstCanvasCmprAttr->enPixelFormat = stRegion.unAttr.stOverlay.enPixelFormat;
        pstCanvasCmprAttr->u32BsSize = OSD_COMPRESSED_SIZE;
        pstCanvasCmprAttr->u32ObjNum = pstOsdParam->stAttr.u32DispNum;

    }

    s32Ret = CVI_RGN_UpdateCanvas(RgnHdl);
    CVI_SYS_Munmap(puBmData, stCanvasInfo.u32Stride * stCanvasInfo.stSize.u32Height);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_UpdateCanvas fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static int OSD_UpdateObj(RGN_HANDLE RgnHdl, const MAPI_OSD_CONTENT_S* pstContent)
{
    int s32Ret;
    uint32_t u32CanvasWidth,u32CanvasHeight,u32BgColor;
    SIZE_S stFontSize;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[RgnHdl];
    int s32StrLen = strnlen(pstOsdParam->szStr, MAPI_OSD_MAX_STR_LEN);
    int NonASCNum = OSD_GetNonASCNum(pstOsdParam->szStr, s32StrLen);
    CVI_U32 disp_idx = 0;
    RGN_CANVAS_INFO_S stCanvasInfo;

    memset(&stCanvasInfo, 0x0, sizeof(RGN_CANVAS_INFO_S));
    s32Ret = CVI_RGN_GetCanvasInfo(RgnHdl, &stCanvasInfo);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_GetCanvasInfo fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    CVI_MAPI_OSD_OBJECT_CONTENT_S stObject = pstContent->stObjectContent;
    for (disp_idx = 0; disp_idx < pstOsdParam->stAttr.u32DispNum; ++disp_idx) {
        if(pstOsdParam->stAttr.astDispAttr[disp_idx].enRgnCmprType == RGN_CMPR_RECT){
            RGN_CANVAS_CMPR_ATTR_S *pstCanvasCmprAttr;
            RGN_CMPR_OBJ_ATTR_S *pstObjAttr;

            pstCanvasCmprAttr = stCanvasInfo.pstCanvasCmprAttr;
            pstCanvasCmprAttr->u32ObjNum = 0;
            pstObjAttr = stCanvasInfo.pstObjAttr;
            pstCanvasCmprAttr->u32Width = stObject.u32Width;
            pstCanvasCmprAttr->u32Height = stObject.u32Height;
            pstCanvasCmprAttr->u32BgColor =  0x00;
            pstCanvasCmprAttr->enPixelFormat = PIXEL_FORMAT_ARGB_1555;
            pstCanvasCmprAttr->u32BsSize = 128000;
            pstCanvasCmprAttr->u32ObjNum += stObject.objectInfo.rec_cnt;

            uint32_t k = 0;
            for(uint32_t i = 0; i < stObject.objectInfo.rec_cnt; i++){
                pstObjAttr[k].stRgnRect.stRect.s32X = stObject.objectInfo.rec_coordinates[4*i];
                pstObjAttr[k].stRgnRect.stRect.s32Y = stObject.objectInfo.rec_coordinates[4*i+1];
                pstObjAttr[k].stRgnRect.stRect.u32Width = stObject.objectInfo.rec_coordinates[4*i+2] - stObject.objectInfo.rec_coordinates[2*i];
                pstObjAttr[k].stRgnRect.stRect.u32Height = stObject.objectInfo.rec_coordinates[4*i+3] - stObject.objectInfo.rec_coordinates[2*i+1];
                pstObjAttr[k].stRgnRect.u32Thick = 2;
                pstObjAttr[k].stRgnRect.u32Color = 0x83E0;
                pstObjAttr[k].stRgnRect.u32IsFill = false;
                pstObjAttr[k].enObjType = RGN_CMPR_RECT;
                k++;
            }
            pstCanvasCmprAttr->u32ObjNum += stObject.objectInfo.line_cnt;
            for(uint32_t i = 0; i < stObject.objectInfo.line_cnt; i++){
                pstObjAttr[k].stLine.stPointStart.s32X = stObject.objectInfo.line_coordinates[4*i];
                pstObjAttr[k].stLine.stPointStart.s32Y = stObject.objectInfo.line_coordinates[4*i+1];
                pstObjAttr[k].stLine.stPointEnd.s32X = stObject.objectInfo.line_coordinates[4*i+2];
                pstObjAttr[k].stLine.stPointEnd.s32Y = stObject.objectInfo.line_coordinates[4*i+3];
                pstObjAttr[k].stLine.u32Thick = 2;
                pstObjAttr[k].stLine.u32Color = 0x83E0;
                pstObjAttr[k].enObjType = RGN_CMPR_LINE;
                k++;
            }
        }
    }

    stCanvasInfo.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
    stCanvasInfo.stSize.u32Width = stObject.u32Width;
    stCanvasInfo.stSize.u32Height = stObject.u32Height;
    s32Ret = CVI_RGN_UpdateCanvas(RgnHdl);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("CVI_RGN_UpdateCanvas fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static void* OSD_TimeUpdateThread(void* pvParam)
{
    int s32Ret = 0;
    int s32OsdIdx;
    time_t nowTime = 0;
    time_t lastTime = 0;
    struct tm stTime = {
        0,
    };

    UNUSED(pvParam);
    prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);

    while (g_bOsdTimeRun) {
        time(&nowTime);
        if (nowTime == lastTime) {
            usleep(MAPI_ADEC_WAIT_TIME_CHANGE_USLEEP_TIME);
            continue;
        } else {
            localtime_r(&nowTime, &stTime);
            for (s32OsdIdx = 0; s32OsdIdx < MAPI_OSD_MAX_CNT; ++s32OsdIdx) {
                pthread_mutex_lock(&g_astOSDParam[s32OsdIdx].mutexLock);
                if (g_astOSDParam[s32OsdIdx].stAttr.stContent.enType == MAPI_OSD_TYPE_TIME &&
                    g_astOSDParam[s32OsdIdx].bStart) {
                    /*   Update OSD Time String   */
                    OSD_GetTimeStr(&stTime, g_astOSDParam[s32OsdIdx].stAttr.stContent.stTimeContent.enTimeFmt,
                                    g_astOSDParam[s32OsdIdx].szStr, MAPI_OSD_MAX_STR_LEN);
                    /*   Update OSD Text Bitmap   */
                    s32Ret = OSD_UpdateTextBitmap(s32OsdIdx, &g_astOSDParam[s32OsdIdx].stAttr.stContent);
                    if (s32Ret != MAPI_SUCCESS) {
                        pthread_mutex_unlock(&g_astOSDParam[s32OsdIdx].mutexLock);
                        CVI_LOGE("Update Text Bitmap failed\n");
                        continue;
                    }
                    if (g_astOSDParam[s32OsdIdx].bOsdcEnable  == false) {
                        /*   Update OSD Attribute   */
                        s32Ret = OSD_UpdateDisplay(s32OsdIdx, &g_astOSDParam[s32OsdIdx].stAttr);
                        if (s32Ret != MAPI_SUCCESS) {
                            CVI_LOGE("Update Attribute failed\n");
                        }
                    }
                }
                pthread_mutex_unlock(&g_astOSDParam[s32OsdIdx].mutexLock);
            }
            lastTime = nowTime; /*   update time   */
        }
        usleep(MAPI_ADEC_WAIT_UPDATE_TIME_USLEEP_TIME);
    }

    return NULL;
}

static int OSD_CheckAttr(int s32OsdIdx, MAPI_OSD_ATTR_S* pstAttr)
{
    if(s32OsdIdx < 0 || s32OsdIdx > MAPI_OSD_MAX_CNT
        || pstAttr->stContent.enType >= MAPI_OSD_TYPE_BUTT) {
        CVI_LOGE("OSD attr param is error ! \n");
        return MAPI_ERR_FAILURE;
    }
    // for osd, disp num must be less than MAPI_OSD_MAX_DISP_CNT. not for osdc
    if ((!g_astOSDParam[s32OsdIdx].bOsdcEnable) && (pstAttr->u32DispNum > MAPI_OSD_MAX_DISP_CNT)) {
        CVI_LOGE("OSD disp num is error ! \n");
        return MAPI_ERR_FAILURE;
    }

    uint32_t i;
    for (i = 0; i < pstAttr->u32DispNum; i++) {
        if(pstAttr->astDispAttr[i].enBindedMod >= MAPI_OSD_BINDMOD_BUTT || \
            pstAttr->astDispAttr[i].enCoordinate > MAPI_OSD_COORDINATE_ABS_COOR) {
            CVI_LOGE("OSD astDispAttr is error ! \n");
            return MAPI_ERR_FAILURE;
        }
    }

    if (pstAttr->stContent.enType == MAPI_OSD_TYPE_TIME) {
        if(pstAttr->stContent.stTimeContent.enTimeFmt >= MAPI_OSD_TIMEFMT_BUTT) {
           CVI_LOGE("TimeFmt is error ! \n");
           return MAPI_ERR_FAILURE;
        }
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_BITMAP) {
        if(pstAttr->stContent.stBitmapContent.pData == NULL) {
           CVI_LOGE("stContent.stBitmapContent.pData is null ! \n");
           return MAPI_ERR_FAILURE;
        }
    }

    return MAPI_SUCCESS;
}

static void OSD_GetMaxSize(MAPI_OSD_ATTR_S* pstAttr, SIZE_S* pstOsdSize)
{
    uint32_t u32StrLen = strnlen(pstAttr->stContent.stStrContent.szStr, MAPI_OSD_MAX_STR_LEN);
    uint32_t u32Width;
    uint32_t u32Height;

    if (pstAttr->stContent.enType == MAPI_OSD_TYPE_BITMAP) {
        u32Width = pstAttr->stContent.stBitmapContent.u32Width;
        u32Height = pstAttr->stContent.stBitmapContent.u32Height;
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_STRING) {
        u32Width = u32StrLen * pstAttr->stContent.stStrContent.stFontSize.u32Width;
        u32Height = pstAttr->stContent.stStrContent.stFontSize.u32Height;
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_CIRCLE) {
        u32Width = pstAttr->stContent.stCircleContent.u32Width;
        u32Height = pstAttr->stContent.stCircleContent.u32Height;
    } else {
        char szTimeStr[MAPI_OSD_MAX_STR_LEN];
        OSD_GetTimeStr(NULL, pstAttr->stContent.stTimeContent.enTimeFmt, szTimeStr,
            MAPI_OSD_MAX_STR_LEN);
        u32StrLen = strnlen(szTimeStr, MAPI_OSD_MAX_STR_LEN);
        u32Width = u32StrLen * pstAttr->stContent.stTimeContent.stFontSize.u32Width;
        u32Height = pstAttr->stContent.stTimeContent.stFontSize.u32Height;
    }

    pstOsdSize->u32Width = u32Width;
    pstOsdSize->u32Height = u32Height;
}

static bool OSD_CheckIfRebuild(int s32OsdIdx, MAPI_OSD_ATTR_S* pstAttr, SIZE_S* pstOsdMaxSize)
{
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];
    /* return TRUE means go rebuild process */
    if ((pstAttr->stContent.enType == MAPI_OSD_TYPE_BITMAP) ||
        (pstAttr->stContent.enType == MAPI_OSD_TYPE_STRING) ||
        (pstAttr->stContent.enType == MAPI_OSD_TYPE_TIME)) {
        if ( pstOsdMaxSize->u32Width > pstOsdParam->stMaxSize.u32Width ||
            pstOsdMaxSize->u32Height > pstOsdParam->stMaxSize.u32Height) {
            return true;
        }
        /* if the string content is not extend, it will not rebuild for performance */
    }

    if (pstOsdParam->stAttr.u32DispNum != pstAttr->u32DispNum) {
        /* if disp num is added, we need attach new chn */
        return true;
    }

    return false;
}

static int OSD_DetachRgn(RGN_HANDLE RgnHdl, const MAPI_OSD_DISP_ATTR_S* pstDispAttr)
{
    int s32Ret;
    MMF_CHN_S stChn;

    stChn.s32DevId = pstDispAttr->ModHdl;
    stChn.s32ChnId = pstDispAttr->ChnHdl;
    switch (pstDispAttr->enBindedMod) {
        case MAPI_OSD_BINDMOD_VPROC:
            stChn.enModId = CVI_ID_VPSS;
            break;
        case MAPI_OSD_BINDMOD_DISP:
            stChn.enModId = CVI_ID_VO;
            break;

        default:
            CVI_LOGE("RgnHdl[%d] invalide bind mode [%d]\n", RgnHdl, pstDispAttr->enBindedMod);
            return MAPI_ERR_FAILURE;
    }

    s32Ret = CVI_RGN_DetachFromChn(RgnHdl, &stChn);

    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_DetachFromChn fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
            RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
        return s32Ret;
    }

    return MAPI_SUCCESS;
}

static int OSD_DestroyRgn(RGN_HANDLE RgnHdl, MAPI_OSD_ATTR_S* pstAttr)
{
    int s32Ret;
    uint32_t u32DispIdx;

    for (u32DispIdx = 0; u32DispIdx < pstAttr->u32DispNum; ++u32DispIdx) {
        s32Ret = OSD_DetachRgn(RgnHdl, &pstAttr->astDispAttr[u32DispIdx]);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("OSD_DetachRgn fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
            return s32Ret;
        }
    }

    s32Ret = CVI_RGN_Destroy(RgnHdl);

    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_Destroy fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    if (g_astOSDParam[RgnHdl].bOsdcEnable  == true) {
        for (u32DispIdx = 0; u32DispIdx < pstAttr->u32DispNum; ++u32DispIdx) {
            if ((pstAttr->astDispAttr[u32DispIdx].enRgnCmprType == RGN_CMPR_BIT_MAP) && (pstAttr->astDispAttr[u32DispIdx].maxlen > 0)) {
                CVI_SYS_IonFree(pstAttr->astDispAttr[u32DispIdx].u64BitmapPhyAddr, pstAttr->astDispAttr[u32DispIdx].pBitmapVirAddr);
                pstAttr->astDispAttr[u32DispIdx].u64BitmapPhyAddr = (unsigned long long int)0;
                pstAttr->astDispAttr[u32DispIdx].pBitmapVirAddr = NULL;
                pstAttr->astDispAttr[u32DispIdx].maxlen = 0;
            }
        }
    }

    return MAPI_SUCCESS;
}

static void OSD_GetRgnAttr(RGN_HANDLE RgnHdl, const MAPI_OSD_ATTR_S* pstAttr, RGN_ATTR_S* pstRgnAttr)
{

    SIZE_S stRgnSize = {0};
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[RgnHdl];
    uint32_t u32StrLen = strnlen(pstOsdParam->szStr, MAPI_OSD_MAX_STR_LEN);

    if(pstAttr->stContent.enType == MAPI_OSD_TYPE_BITMAP) {
        stRgnSize.u32Width = pstAttr->stContent.stBitmapContent.u32Width;
        stRgnSize.u32Height = pstAttr->stContent.stBitmapContent.u32Height;
        pstRgnAttr->enType = OVERLAY_RGN;
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_TIME) {
        stRgnSize.u32Width = u32StrLen * pstAttr->stContent.stTimeContent.stFontSize.u32Width;
        stRgnSize.u32Height = pstAttr->stContent.stTimeContent.stFontSize.u32Height;
        pstRgnAttr->enType = OVERLAY_RGN;
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_STRING) {
        stRgnSize.u32Width = u32StrLen * pstAttr->stContent.stStrContent.stFontSize.u32Width;
        stRgnSize.u32Height = pstAttr->stContent.stStrContent.stFontSize.u32Height;
        pstRgnAttr->enType = OVERLAY_RGN;
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_OBJECT) {
        stRgnSize.u32Width = pstAttr->stContent.stObjectContent.u32Width;
        stRgnSize.u32Height = pstAttr->stContent.stObjectContent.u32Height;
        pstRgnAttr->enType = OVERLAY_RGN;
    } else {
        pstRgnAttr->enType = COVER_RGN;
    }

    if (pstRgnAttr->enType == OVERLAY_RGN) {
        pstRgnAttr->unAttr.stOverlay.enPixelFormat = PIXEL_FORMAT_ARGB_1555;
        pstRgnAttr->unAttr.stOverlay.u32BgColor = pstAttr->stContent.u32Color;
        pstRgnAttr->unAttr.stOverlay.stSize.u32Width = stRgnSize.u32Width;
        pstRgnAttr->unAttr.stOverlay.stSize.u32Height = stRgnSize.u32Height;
        pstRgnAttr->unAttr.stOverlay.u32CanvasNum = (MAPI_OSD_TYPE_BITMAP == pstAttr->stContent.enType) ? 1 : 2;
        pstRgnAttr->unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
        if(pstAttr->stContent.enType == MAPI_OSD_TYPE_OBJECT){
            pstRgnAttr->unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_HW;
            pstRgnAttr->unAttr.stOverlay.stCompressInfo.u32CompressedSize = 128000;
        } else {
            pstRgnAttr->unAttr.stOverlay.stCompressInfo.enOSDCompressMode = OSD_COMPRESS_MODE_NONE;
        }
    }
    pstOsdParam->enRgnType = pstRgnAttr->enType;
    return;
}

static int OSD_UpdateRgnContent(RGN_HANDLE RgnHdl, const MAPI_OSD_ATTR_S* pstAttr)
{
    int s32Ret = MAPI_SUCCESS;
    if (pstAttr->stContent.enType == MAPI_OSD_TYPE_BITMAP) {
        BITMAP_S stBitmap;
        stBitmap.enPixelFormat = pstAttr->stContent.stBitmapContent.enPixelFormat;
        stBitmap.u32Width = pstAttr->stContent.stBitmapContent.u32Width;
        stBitmap.u32Height = pstAttr->stContent.stBitmapContent.u32Height;
        stBitmap.pData = pstAttr->stContent.stBitmapContent.pData;

        s32Ret = CVI_RGN_SetBitMap(RgnHdl, &stBitmap);

        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("RGN_SetBitMap fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
            return s32Ret;
        }
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_TIME ||
        pstAttr->stContent.enType == MAPI_OSD_TYPE_STRING){
        s32Ret = OSD_UpdateTextBitmap(RgnHdl, &pstAttr->stContent);

        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("OSD_UpdateTextBitmap failed\n");
            return s32Ret;
        }
    } else if (pstAttr->stContent.enType == MAPI_OSD_TYPE_OBJECT){
        s32Ret = OSD_UpdateObj(RgnHdl, &pstAttr->stContent);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("OSD_UpdateObj failed\n");
            return s32Ret;
        }
    }
    return s32Ret;

}

static int OSD_AttachRgn(RGN_HANDLE RgnHdl, const MAPI_OSD_DISP_ATTR_S* pstDispAttr,
    const MAPI_OSD_CONTENT_S* pstOsdContent)
{
    int s32Ret = MAPI_SUCCESS;
    RGN_CHN_ATTR_S stRgnChnAttr;
    MMF_CHN_S stChn;
    stChn.s32DevId = pstDispAttr->ModHdl;
    stChn.s32ChnId = pstDispAttr->ChnHdl;
    memset(&stRgnChnAttr, 0x0, sizeof(RGN_CHN_ATTR_S));
    stRgnChnAttr.bShow = pstDispAttr->bShow;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[RgnHdl];
    stRgnChnAttr.enType = pstOsdParam->enRgnType;

    switch (pstDispAttr->enBindedMod) {
        case MAPI_OSD_BINDMOD_VPROC:
            stChn.enModId = CVI_ID_VPSS;
            break;

        case MAPI_OSD_BINDMOD_DISP:
            stChn.enModId = CVI_ID_VO;
            break;

        default:
            CVI_LOGE("RgnHdl[%d] invalide bind mode [%d]\n", RgnHdl, pstDispAttr->enBindedMod);
            return MAPI_ERR_FAILURE;
    }

    POINT_S stStartPos;

    if (pstDispAttr->enCoordinate == MAPI_OSD_COORDINATE_RATIO_COOR) {
        s32Ret = OSD_Ratio2Absolute(stChn, &pstDispAttr->stStartPos, &stStartPos);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("OSD_Ratio2Absolute fail,RgnHdl[%d] Error Code: [0x%08X]\n",
                RgnHdl, s32Ret);
            return s32Ret;
        }
    } else {
        stStartPos = pstDispAttr->stStartPos;
    }

    if (stRgnChnAttr.enType == OVERLAY_RGN) {
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32X = stStartPos.s32X;
        stRgnChnAttr.unChnAttr.stOverlayExChn.stPoint.s32Y = stStartPos.s32Y;
        stRgnChnAttr.unChnAttr.stOverlayExChn.u32Layer = 0;
    } else {
        stRgnChnAttr.unChnAttr.stCoverExChn.u32Layer = 0;
        stRgnChnAttr.unChnAttr.stCoverExChn.u32Color = pstOsdContent->u32Color;
        stRgnChnAttr.unChnAttr.stCoverExChn.enCoverType = AREA_RECT; //AREA_QUAD_RANGLE not support
        stRgnChnAttr.unChnAttr.stCoverExChn.stRect.u32Width = pstOsdContent->stCircleContent.u32Width;
        stRgnChnAttr.unChnAttr.stCoverExChn.stRect.u32Height = pstOsdContent->stCircleContent.u32Height;
        stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32X = stStartPos.s32X;
        stRgnChnAttr.unChnAttr.stCoverExChn.stRect.s32Y = stStartPos.s32Y;

    }

    s32Ret = CVI_RGN_AttachToChn(RgnHdl, &stChn, &stRgnChnAttr);

    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_AttachToChn fail,RgnHdl[%d] stChn[%d,%d,%d] Error Code: [0x%08X]\n",
                       RgnHdl, stChn.enModId, stChn.s32DevId, stChn.s32ChnId, s32Ret);
        return s32Ret;
    }
    return MAPI_SUCCESS;
}

static int OSD_CreateRgn(RGN_HANDLE RgnHdl, MAPI_OSD_ATTR_S* pstAttr)
{
    int s32Ret;
    RGN_ATTR_S stRgnAttr;
    memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
    uint32_t u32DispIdx;

    if (g_astOSDParam[RgnHdl].bOsdcEnable  == true) {
        for (u32DispIdx = 0; u32DispIdx < pstAttr->u32DispNum; ++u32DispIdx) {
            if (pstAttr->astDispAttr[u32DispIdx].enRgnCmprType == RGN_CMPR_BIT_MAP) {
                CVI_SYS_IonFree(pstAttr->astDispAttr[u32DispIdx].u64BitmapPhyAddr, pstAttr->astDispAttr[u32DispIdx].pBitmapVirAddr);
                pstAttr->astDispAttr[u32DispIdx].u64BitmapPhyAddr = (unsigned long long int)0;
                pstAttr->astDispAttr[u32DispIdx].pBitmapVirAddr = NULL;
                pstAttr->astDispAttr[u32DispIdx].maxlen = 0;
            }
        }
    }

    OSD_GetRgnAttr(RgnHdl,pstAttr,&stRgnAttr);
    s32Ret = CVI_RGN_Create(RgnHdl, &stRgnAttr);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("RGN_Create fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }

    for (u32DispIdx = 0; u32DispIdx < pstAttr->u32DispNum; ++u32DispIdx) {
        s32Ret = OSD_AttachRgn(RgnHdl, &pstAttr->astDispAttr[u32DispIdx], &pstAttr->stContent);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("OSD_AttachRgn fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
            return s32Ret;
        }
    }

    s32Ret = OSD_UpdateRgnContent(RgnHdl, pstAttr);
    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("osd update content fail,RgnHdl[%d] Error Code: [0x%08X]\n", RgnHdl, s32Ret);
        return s32Ret;
    }
    return MAPI_SUCCESS;
}

static int OSD_Start(int s32OsdIdx)
{
    int s32Ret;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];

    /*   Time OSD: Update time string */
    if (pstOsdParam->stAttr.stContent.enType == MAPI_OSD_TYPE_TIME) {
        OSD_GetTimeStr(NULL, pstOsdParam->stAttr.stContent.stTimeContent.enTimeFmt, pstOsdParam->szStr,
                       MAPI_OSD_MAX_STR_LEN);
    }

    s32Ret = OSD_CreateRgn(s32OsdIdx, &pstOsdParam->stAttr);

    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("OSD_CreateRgn s32OsdIdx[%d] failed:[0x%08X]\n", s32OsdIdx, s32Ret);
        return s32Ret;
    }

    pstOsdParam->bStart = true;
    return MAPI_SUCCESS;
}

static int OSD_Stop(int s32OsdIdx)
{
    int s32Ret;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];
    s32Ret = OSD_DestroyRgn(s32OsdIdx, &pstOsdParam->stAttr);

    if (s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("OSD_DestroyRgn s32OsdIdx[%d] failed:[0x%08X]\n", s32OsdIdx, s32Ret);
        return s32Ret;
    }

    pstOsdParam->bStart = false;
    return MAPI_SUCCESS;
}

int MAPI_OSD_Init(const MAPI_OSD_FONTS_S* pstFonts, bool bOsdcEnable)
{
    int s32Ret = MAPI_SUCCESS;
    int s32Idx = 0;
    if (pstFonts != NULL) {
        if (pstFonts->u32FontWidth % BYTE_BITS) {
            CVI_LOGE("FontWidth must be a multiple of %d.", BYTE_BITS);
            return MAPI_ERR_FAILURE;
        }
        memcpy(&g_stOsdFonts, pstFonts, sizeof(MAPI_OSD_FONTS_S));
    } else {
        memset(&g_stOsdFonts, 0, sizeof(MAPI_OSD_FONTS_S));
    }

    /*   Init OSD Param   */
    for (s32Idx = 0; s32Idx < MAPI_OSD_MAX_CNT; ++s32Idx) {
        pthread_mutex_init(&g_astOSDParam[s32Idx].mutexLock, NULL);
        pthread_mutex_lock(&g_astOSDParam[s32Idx].mutexLock);
        memset(&g_astOSDParam[s32Idx].stAttr, 0, sizeof(MAPI_OSD_ATTR_S));
        memset(&g_astOSDParam[s32Idx].stMaxSize, 0, sizeof(SIZE_S));
        g_astOSDParam[s32Idx].bInit = false;
        g_astOSDParam[s32Idx].bStart = false;
        g_astOSDParam[s32Idx].bOsdcEnable  = bOsdcEnable;
        g_astOSDParam[s32Idx].enRgnType = RGN_BUTT;
        memset(&g_astOSDParam[s32Idx].szStr, 0, sizeof(char) * MAPI_OSD_MAX_STR_LEN);
        pthread_mutex_unlock(&g_astOSDParam[s32Idx].mutexLock);
    }

    if (pstFonts != NULL) {
        /*   Create Time OSD Update Thread   */
        g_bOsdTimeRun = true;

        s32Ret = pthread_create(&g_pOsdTimeTskId, NULL, OSD_TimeUpdateThread, NULL);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("create OSD_TimeUpdateThread failed:%s\n", strerror(errno));
            return MAPI_ERR_FAILURE;
        }
    }

    g_bOsdInitFlg = true;

    return MAPI_SUCCESS;
}

int MAPI_OSD_Deinit(void)
{
    if (g_pOsdTimeTskId != 0) {
        /*   Destroy Time OSD Update Task   */
        g_bOsdTimeRun = false;
        pthread_join(g_pOsdTimeTskId, NULL);
    }

    int s32Idx;
    for (s32Idx = 0; s32Idx < MAPI_OSD_MAX_CNT; ++s32Idx) {
        pthread_mutex_lock(&g_astOSDParam[s32Idx].mutexLock);
        /*   Stop Osd   */
        // if (g_astOSDParam[s32Idx].bStart) {
        //     int s32Ret = OSD_Stop(s32Idx);
        //     if (s32Ret != MAPI_SUCCESS) {
        //         CVI_LOGE("OSD[%d] Stop error:%#x\n", s32Idx, s32Ret);
        //         pthread_mutex_unlock(&g_astOSDParam[s32Idx].mutexLock);
        //         return s32Ret;
        //     }
        // }
        pthread_mutex_unlock(&g_astOSDParam[s32Idx].mutexLock);
        pthread_mutex_destroy(&g_astOSDParam[s32Idx].mutexLock);
    }

    g_bOsdInitFlg = false;

    return MAPI_SUCCESS;
}

int MAPI_OSD_SetAttr(int s32OsdIdx, MAPI_OSD_ATTR_S* pstAttr)
{
    /*   Check Module Init or not   */
    if(g_bOsdInitFlg != true) {
        CVI_LOGE("g_bOsdInitFlg = %d osd not init!\n",g_bOsdInitFlg);
        return MAPI_ERR_FAILURE;
    }
    int s32Ret;
    s32Ret = OSD_CheckAttr(s32OsdIdx, pstAttr);
    if(s32Ret != MAPI_SUCCESS) {
        CVI_LOGE("s32Ret = %d OSD_CheckAttr faile!\n",s32Ret);
        return MAPI_ERR_FAILURE;
    }

    SIZE_S stOsdMaxSize;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];
    OSD_GetMaxSize(pstAttr, &stOsdMaxSize);
    pthread_mutex_lock(&pstOsdParam->mutexLock);

    /* pstAttr do not save time osd str ,so will use pstOsdParam->szStr save time osd and str osd content */
    if (pstAttr->stContent.enType == MAPI_OSD_TYPE_STRING) {
        memset(pstOsdParam->szStr, 0, MAPI_OSD_MAX_STR_LEN);
        snprintf(pstOsdParam->szStr, MAPI_OSD_MAX_STR_LEN, "%s", pstAttr->stContent.stStrContent.szStr);
    } else if(pstAttr->stContent.enType == MAPI_OSD_TYPE_TIME) {
        OSD_GetTimeStr(NULL, pstAttr->stContent.stTimeContent.enTimeFmt, pstOsdParam->szStr,
            MAPI_OSD_MAX_STR_LEN);
    }

    bool bStart = pstOsdParam->bStart;
    if (bStart) {
        if (OSD_CheckIfRebuild(s32OsdIdx, pstAttr, &stOsdMaxSize)) {
            /*  rebuild RGN  */
            s32Ret = OSD_DestroyRgn(s32OsdIdx, &pstOsdParam->stAttr);
            if (s32Ret != MAPI_SUCCESS) {
                pthread_mutex_unlock(&pstOsdParam->mutexLock);
                return s32Ret;
            }

            s32Ret = OSD_CreateRgn(s32OsdIdx, pstAttr);
            if (s32Ret != MAPI_SUCCESS) {
                pthread_mutex_unlock(&pstOsdParam->mutexLock);
                return s32Ret;
            }

            pstOsdParam->stMaxSize.u32Width = stOsdMaxSize.u32Width;
            pstOsdParam->stMaxSize.u32Height = stOsdMaxSize.u32Height;
        } else {
            /*  Update RGN Content  */
            s32Ret = OSD_UpdateRgnContent(s32OsdIdx, pstAttr);
            if (s32Ret != MAPI_SUCCESS) {
                CVI_LOGE("osd update rgn content fail,RgnHdl[%d] Error Code: [0x%08X]\n", s32OsdIdx, s32Ret);
                pthread_mutex_unlock(&pstOsdParam->mutexLock);
                return s32Ret;
            }
            if (g_astOSDParam[s32OsdIdx].bOsdcEnable  == false) {
                /*  Update OSD Display  */
                s32Ret = OSD_UpdateDisplay(s32OsdIdx, pstAttr);
                if (s32Ret != MAPI_SUCCESS) {
                    pthread_mutex_unlock(&pstOsdParam->mutexLock);
                    return s32Ret;
                }
            }
        }
    } else {
        pstOsdParam->stMaxSize.u32Width = stOsdMaxSize.u32Width;
        pstOsdParam->stMaxSize.u32Height = stOsdMaxSize.u32Height;
    }

    memcpy(&pstOsdParam->stAttr, pstAttr, sizeof(MAPI_OSD_ATTR_S));
    pstOsdParam->bInit = true;
    pthread_mutex_unlock(&pstOsdParam->mutexLock);
    return MAPI_SUCCESS;
}

int MAPI_OSD_GetAttr(int s32OsdIdx, MAPI_OSD_ATTR_S* pstAttr)
{
    if(g_bOsdInitFlg != true || s32OsdIdx < 0 || s32OsdIdx > MAPI_OSD_MAX_CNT || pstAttr == NULL) {
        CVI_LOGE("MAPI_OSD_ATTR_S is error!\n");
        return MAPI_ERR_FAILURE;
    }

    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];
    pthread_mutex_lock(&pstOsdParam->mutexLock);

    /*   Check OSD Attrbute init or not   */
    if (!pstOsdParam->bInit) {
        pthread_mutex_unlock(&pstOsdParam->mutexLock);
        CVI_LOGE("OSD[%d] not init yet!\n", s32OsdIdx);
        return MAPI_ERR_FAILURE;
    }

    memcpy(pstAttr, &pstOsdParam->stAttr, sizeof(MAPI_OSD_ATTR_S));
    pthread_mutex_unlock(&pstOsdParam->mutexLock);
    return MAPI_SUCCESS;
}

int MAPI_OSD_Start(int s32OsdIdx)
{
    if(g_bOsdInitFlg != true || s32OsdIdx < 0 || s32OsdIdx > MAPI_OSD_MAX_CNT) {
        CVI_LOGE("MAPI_OSD_Start is error!\n");
        return MAPI_ERR_FAILURE;
    }
    int s32Ret;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];

    pthread_mutex_lock(&pstOsdParam->mutexLock);

    /*   Check OSD Attrbute init or not   */
    if (!pstOsdParam->bInit) {
        pthread_mutex_unlock(&pstOsdParam->mutexLock);
        CVI_LOGE("OSD[%d] not init yet!\n", s32OsdIdx);
        return MAPI_ERR_FAILURE;
    }

    /*   Check OSD stop or not   */
    if (pstOsdParam->bStart) {
        pthread_mutex_unlock(&pstOsdParam->mutexLock);
        CVI_LOGE("OSD[%d] has already started!\n", s32OsdIdx);
        return MAPI_SUCCESS;
    }

    s32Ret = OSD_Start(s32OsdIdx);
    pthread_mutex_unlock(&pstOsdParam->mutexLock);
    return s32Ret;
}

int MAPI_OSD_Stop(int s32OsdIdx)
{
    if(g_bOsdInitFlg != true || s32OsdIdx < 0 || s32OsdIdx > MAPI_OSD_MAX_CNT) {
        CVI_LOGE("MAPI_OSD_Stop is error!\n");
        return MAPI_ERR_FAILURE;
    }

    int s32Ret;
    OSD_PARAM_S *pstOsdParam = &g_astOSDParam[s32OsdIdx];

    pthread_mutex_lock(&pstOsdParam->mutexLock);

    /*   Check OSD Attrbute init or not   */
    if (!pstOsdParam->bInit) {
        pthread_mutex_unlock(&pstOsdParam->mutexLock);
        return MAPI_SUCCESS;
    }

    /*   Check OSD stop or not   */
    if (!pstOsdParam->bStart) {
        pthread_mutex_unlock(&pstOsdParam->mutexLock);
        return MAPI_SUCCESS;
    }

    s32Ret = OSD_Stop(s32OsdIdx);
    pstOsdParam->stMaxSize.u32Width = 0;
    pstOsdParam->stMaxSize.u32Height = 0;
    pthread_mutex_unlock(&pstOsdParam->mutexLock);
    return s32Ret;
}

int MAPI_OSD_Batch(uint32_t u32Batch, bool bShow)
{
    if(g_bOsdInitFlg != true) {
        CVI_LOGE("MAPI_OSD_Batch is error!\n");
        return MAPI_ERR_FAILURE;
    }
    int s32OsdIdx;
    uint32_t u32DispIdx;
    bool bUpdateContent;
    for (s32OsdIdx = 0; s32OsdIdx < MAPI_OSD_MAX_CNT; ++s32OsdIdx) {
        if (!g_astOSDParam[s32OsdIdx].bStart) {
            continue;
        }

        bUpdateContent = false;

        pthread_mutex_lock(&g_astOSDParam[s32OsdIdx].mutexLock);
        for (u32DispIdx = 0; u32DispIdx < g_astOSDParam[s32OsdIdx].stAttr.u32DispNum; ++u32DispIdx) {
            if (g_astOSDParam[s32OsdIdx].stAttr.astDispAttr[u32DispIdx].u32Batch == u32Batch) {
                bUpdateContent = true;
                g_astOSDParam[s32OsdIdx].stAttr.astDispAttr[u32DispIdx].bShow = bShow;
            }
        }

        /* nonentity u32Batch, no need update display */
        if ((bUpdateContent == true) && (g_astOSDParam[s32OsdIdx].bOsdcEnable  == false)) {
            OSD_UpdateDisplay(s32OsdIdx, &g_astOSDParam[s32OsdIdx].stAttr);
        }

        pthread_mutex_unlock(&g_astOSDParam[s32OsdIdx].mutexLock);
    }

    return MAPI_SUCCESS;
}

int MAPI_OSD_Show(int32_t s32OsdIdx, uint32_t u32DispIdx, bool bShow)
{
    if(g_bOsdInitFlg != true) {
        CVI_LOGE("MAPI_OSD_Batch is error!\n");
        return MAPI_ERR_FAILURE;
    }
    if (!g_astOSDParam[s32OsdIdx].bStart) {
        return MAPI_SUCCESS;
    }

    if (bShow == g_astOSDParam[s32OsdIdx].stAttr.astDispAttr[u32DispIdx].bShow) {
       return MAPI_SUCCESS;
    }

    pthread_mutex_lock(&g_astOSDParam[s32OsdIdx].mutexLock);

    g_astOSDParam[s32OsdIdx].stAttr.astDispAttr[u32DispIdx].bShow = bShow;
    if (g_astOSDParam[s32OsdIdx].bOsdcEnable == false) {
        OSD_UpdateDisplay(s32OsdIdx, &g_astOSDParam[s32OsdIdx].stAttr);
    }
    pthread_mutex_unlock(&g_astOSDParam[s32OsdIdx].mutexLock);

    return MAPI_SUCCESS;
}
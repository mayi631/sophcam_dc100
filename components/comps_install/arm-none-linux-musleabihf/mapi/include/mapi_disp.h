#ifndef __MAPI_DISP_H__
#define __MAPI_DISP_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#include "mapi_define.h"
#include "mapi_vproc.h"

#include "cvi_comm_video.h"

#include "cvi_comm_vo.h"
#include "cvi_math.h"
#include "mapi_sys.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define COLOR_10_RGB_RED RGB(0x3FF, 0, 0)
#define COLOR_10_RGB_GREEN RGB(0, 0x3FF, 0)
#define COLOR_10_RGB_BLUE RGB(0, 0, 0x3FF)
#define COLOR_10_RGB_BLACK RGB(0, 0, 0)
#define COLOR_10_RGB_YELLOW RGB(0x3FF, 0x3FF, 0)
#define COLOR_10_RGB_CYN RGB(0, 0x3FF, 0x3FF)
#define COLOR_10_RGB_WHITE RGB(0x3FF, 0x3FF, 0x3FF)

#define DISP_VIDEOLAYER_DISPRECT_X 0
#define DISP_VIDEOLAYER_DISPRECT_Y 0

#define DISP_MAX_WND_NUM    (16)

#define PDT_MEDIA_DISP_MAX_CNT    (1) /**<video display maximum count */

#define MAPI_CHECK_NULL_PTR(ptr)                                              \
        do {                                                                  \
                if (!(ptr)) {                                                 \
                        CVI_LOGE("Error: NULL pointer\n");                    \
                        return MAPI_ERR_INVALID;                          \
                }                                                             \
        } while (0)

typedef MAPI_HANDLE_T MAPI_DISP_HANDLE_T;
typedef MAPI_HANDLE_T MAPI_WND_HANDLE_T;

typedef enum _MAPI_DISP_MODE_E {
    DISP_MODE_1MUX,
    DISP_MODE_2MUX,
    DISP_MODE_4MUX,
    DISP_MODE_8MUX,
    DISP_MODE_9MUX,
    DISP_MODE_16MUX,
    DISP_MODE_25MUX,
    DISP_MODE_36MUX,
    DISP_MODE_49MUX,
    DISP_MODE_64MUX,
    DISP_MODE_2X4,
    DISP_MODE_BUTT
} MAPI_DISP_MODE_E;

typedef struct _MAPI_DISP_ATTR_S {
    uint32_t          width;
    uint32_t          height;
    int32_t           fps;
    PIXEL_FORMAT_E    pixel_format;
    ROTATION_E        rotate;  // 0 - 0, 1 - 90, 2 - 180, 3 - 270
    bool              window_mode;
    VO_PUB_ATTR_S     stPubAttr;
} MAPI_DISP_ATTR_T;

typedef struct _MAPI_DISP_VIDEOLAYER_ATTR_S {
    RECT_S stDispRect;
    SIZE_S stImageSize;
    uint32_t u32VLFrameRate;
    uint32_t u32BufLen;
    uint32_t u32PixelFmt;
} MAPI_DISP_VIDEOLAYER_ATTR_S;

typedef struct MAPI_WND_ATTR_S {
    uint32_t          wnd_x;
    uint32_t          wnd_y;
    uint32_t          wnd_w;
    uint32_t          wnd_h;
    uint32_t          u32Priority;
} MAPI_WND_ATTR_T;

/** video display configure */
typedef struct tagMEDIA_DispCfg {
    MAPI_DISP_HANDLE_T dispHdl;
    MAPI_DISP_ATTR_T dispAttr;
    MAPI_DISP_VIDEOLAYER_ATTR_S videoLayerAttr;
    //MEDIA_DispWndCfg    wndCfg[HI_PDT_MEDIA_DISP_WND_MAX_CNT];
} MEDIA_DispCfg;

/** video out configure */
typedef struct tagMEDIA_VideoOutCfg {
    MEDIA_DispCfg dispCfg[PDT_MEDIA_DISP_MAX_CNT];
} MEDIA_VideoOutCfg;


int MAPI_DISP_Init(MAPI_DISP_HANDLE_T *disp_hdl, int disp_id,
            MAPI_DISP_ATTR_T *attr);
int MAPI_DISP_Deinit(MAPI_DISP_HANDLE_T disp_hdl);
int MAPI_DISP_Start(MAPI_DISP_HANDLE_T disp_hdl,
            MAPI_DISP_VIDEOLAYER_ATTR_S *pstVideoLayerAttr);
int MAPI_DISP_Stop(MAPI_DISP_HANDLE_T disp_hdl);
int MAPI_DISP_BindVproc(MAPI_DISP_HANDLE_T disp_hdl,
        MAPI_VPROC_HANDLE_T vproc_hdl, int vproc_chn_idx);
int MAPI_DISP_UnBindVproc(MAPI_DISP_HANDLE_T disp_hdl,
        MAPI_VPROC_HANDLE_T vproc_hdl, int vproc_chn_idx);
int MAPI_DISP_SendFrame(MAPI_DISP_HANDLE_T disp_hdl,
        VIDEO_FRAME_INFO_S *frame);
int MAPI_DISP_ClearBuf(MAPI_DISP_HANDLE_T disp_hdl);
int MAPI_DISP_CreateWindow(int disp_id, CVI_STITCH_ATTR_S *pstStitchAttr);
int MAPI_DISP_DestroyWindow(int disp_id, CVI_STITCH_ATTR_S *pstStitchAttr);
int MAPI_DISP_WndBindVproc(MAPI_WND_HANDLE_T wnd_hdl,
        MAPI_VPROC_HANDLE_T vproc_hdl, int vproc_chn_idx);
int MAPI_DISP_SendWndFrame(MAPI_WND_HANDLE_T wnd_hdl,
        VIDEO_FRAME_INFO_S *frame);
int MAPI_DISP_GetWndAttr(MAPI_WND_HANDLE_T wnd_hdl, MAPI_WND_ATTR_T *attr);
int MAPI_DISP_SetWndAttr(MAPI_WND_HANDLE_T wnd_hdl, MAPI_WND_ATTR_T *attr);
int MAPI_DISP_ReleaseWndFrame(MAPI_WND_HANDLE_T wnd_hdl);
void MAPI_DISP_SetDumpStatus(bool en);
int MAPI_DISP_Pause(MAPI_DISP_HANDLE_T disp_hdl);
int MAPI_DISP_Resume(MAPI_DISP_HANDLE_T disp_hdl);
#ifdef __cplusplus
}
#endif

#endif

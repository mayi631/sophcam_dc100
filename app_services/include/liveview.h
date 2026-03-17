#ifndef __LIVEVIEW_H__
#define __LIVEVIEW_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mapi.h"
#include "sysutils_mq.h"

#ifndef CHECK_RET
#define CHECK_RET(express)                                                       \
    do {                                                                         \
        int32_t rc = express;                                                        \
        if (rc != 0) {                                                           \
            printf("\nFailed at %s: %d  (rc:0x%#x!)\n", __FILE__, __LINE__, rc); \
        }                                                                        \
    } while (0)
#endif

#define CMD_CLIENT_ID_LIVEVIEW (MQ_CLIENT_ID_SVC_1)

#define CMD_CHANNEL_ID_LIVEVIEW(liveview_id) (0x00 + (liveview_id))

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

typedef enum cmd_liveview_e {
    CMD_LIVEVIEW_INVALID = 0,
    CMD_LIVEVIEW_SHUTDOWN,
    CMD_LIVEVIEW_SWITCH,
    CMD_LIVEVIEW_MOVEUP,
    CMD_LIVEVIEW_MOVEDOWN,
    CMD_LIVEVIEW_MIRROR,
    CMD_LIVEVIEW_FILP,
    CMD_LIVEVIEW_ADJUSTFOCUS,
    CMD_LIVEVIEW_MAX
} cmd_liveview_t;

typedef struct LIVEVIEW_SERVICE_WNDATTR_S {
    bool        WndEnable;
    bool        UsedCrop;
    bool        SmallWndEnable;
    uint32_t    BindVprocId;
    uint32_t    BindVprocChnId;
    uint32_t    WndX;
    uint32_t    WndY;
    uint32_t    WndWidth;
    uint32_t    WndHeight;
    uint32_t    WndsWidth;
    uint32_t    WndsHeight;
    uint32_t    WndsX;
    uint32_t    WndsY;
    uint32_t    OneStep;
    uint32_t    WndMirror;
    uint32_t    WndFilp;
    int32_t     yStep;
    /* new */
    float ratio;
    uint32_t Yoffset;
    uint32_t Xoffset;
} LIVEVIEW_SERVICE_WNDATTR_S;

typedef struct _LIVEVIEW_SERVICE_ATTR_S {
    LIVEVIEW_SERVICE_WNDATTR_S  wnd_attr;
    MAPI_VPROC_HANDLE_T         vproc_hdl;
} LIVEVIEW_SERVICE_ATTR_S;

typedef struct _LIVEVIEW_SERVICE_PARAM_S {
    uint32_t    WndCnt;
    VPSS_GRP vproc_id;
    VB_POOL hVbPool;
    LIVEVIEW_SERVICE_ATTR_S LiveviewService[DISP_MAX_WND_NUM];
} LIVEVIEW_SERVICE_PARAM_S;

typedef void *LIVEVIEW_SERVICE_HANDLE_T;
typedef LIVEVIEW_SERVICE_PARAM_S lv_param_t, *lv_param_handle_t;

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t LIVEVIEW_SERVICE_Create(LIVEVIEW_SERVICE_HANDLE_T *hdl, LIVEVIEW_SERVICE_PARAM_S *params);
int32_t LIVEVIEW_SERVICE_Destroy(LIVEVIEW_SERVICE_HANDLE_T hdl);
int32_t LIVEVIEW_SERVICE_GetParam(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, LIVEVIEW_SERVICE_WNDATTR_S *WndParam);
int32_t LIVEVIEW_SERVICE_SetStepY(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId,int32_t step,int32_t* lastStep);
int32_t LIVEVIEW_SERVICE_GetStepY(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId,int32_t* lastStep);
int32_t LIVEVIEW_SERVICE_AddStepY(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, int32_t step,int32_t* lastStep);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
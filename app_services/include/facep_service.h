#ifndef __FACEP_SERVICE_H__
#define __FACEP_SERVICE_H__

#include "cvi_type.h"
#include "mapi.h"

#define MODEL_PATH_LEN  128
typedef CVI_S32 FACEP_SERVICE_HANDLE_T;
typedef CVI_S32 (*DrawRectFun)(CVI_U32 osd_id, CVI_U32 num, RECT_S* rects);
typedef CVI_VOID (*SmileFun)(CVI_VOID);

typedef struct _FACEP_SERVICE_PARAM_S{
    CVI_S32 fd_enable;
    CVI_S32 fattr_enable;
    CVI_S32 in_vpss_grp;
    CVI_S32 in_vpss_chn;
    CVI_U32 in_width;
    CVI_U32 in_height;
    CVI_U32 model_width;
    CVI_U32 model_height;
    CVI_S32 model_fd_id;
    CVI_S32 osd_id;
    CVI_U32 osd_mirror;
    char model_fd_path[MODEL_PATH_LEN];
    float model_fd_thres;
    CVI_S32 model_fattr_id;
    char model_fattr_path[MODEL_PATH_LEN];
}FACEP_SERVICE_PARAM_S;

typedef struct _FACEP_PARAM_S{
    CVI_S32 count;
    FACEP_SERVICE_PARAM_S sv_param;
}FACEP_PARAM_S;

typedef enum _FACEP_FUNC_ID_E{
    FACEP_FUNC_ID_FD = 0,
    FACEP_FUNC_ID_FATTR = 1,
    FACEP_FUNC_ID_BUTT,
}FACEP_FUNC_ID_E;

CVI_S32 FACEP_SERVICE_Create(FACEP_SERVICE_HANDLE_T* phandle, FACEP_SERVICE_PARAM_S* pparam);
CVI_S32 FACEP_SERVICE_Destory(FACEP_SERVICE_HANDLE_T handle);
CVI_S32 FACEP_SERVICE_Pause(CVI_VOID);
CVI_S32 FACEP_SERVICE_Resume(CVI_VOID);
CVI_S32 FACEP_SERVICE_Enable(FACEP_SERVICE_HANDLE_T handle, FACEP_FUNC_ID_E id);
CVI_S32 FACEP_SERVICE_Disable(FACEP_SERVICE_HANDLE_T handle, FACEP_FUNC_ID_E id);
CVI_S32 FACEP_SERVICE_Register_DrawRects_Callback(DrawRectFun pfun);
CVI_S32 FACEP_SERVICE_Register_Smile_Callback(SmileFun pfun);
CVI_S32 FACEP_SERVICE_Register_Smile_Pre_Callback(SmileFun pfun);
CVI_S32 FACEP_SERVICE_Unregister_Smile_Pre_Callback(CVI_VOID);
CVI_S32 FACEP_SERVICE_Register_Smile_Post_Callback(SmileFun pfun);
CVI_S32 FACEP_SERVICE_Unregister_Smile_Post_Callback(CVI_VOID);

#endif

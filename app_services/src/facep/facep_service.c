#include "facep_service.h"
#include "cvi_vpss.h"
#include <unistd.h>
#include <math.h>
#include "c_apis/tdl_sdk.h"

typedef CVI_S32 (*PfpFaceDetInferFunc)(TDLHandle handle, const TDLModel model_id, TDLImage image_handle, TDLFace *obj);
typedef CVI_S32(*PfpFaceAttrInferFunc)(TDLHandle handle, const TDLModel model_id, TDLImage image_handle, TDLFace *face_meta);

typedef struct _FACEP_CONTEXT_T{
    CVI_S32 handle;
    pthread_mutex_t mutex;
    CVI_S32 is_running;
    OSAL_TASK_HANDLE_S facep_task_handle;
    FACEP_SERVICE_PARAM_S facep_param;
    TDLHandle fp_tdl_handle;
    PfpFaceDetInferFunc fd_infer_func;
    PfpFaceAttrInferFunc fattr_infer_func;
}FACEP_CONTEXT_T;

#define MAX_FACEP_CNT MAX_CAMERA_INSTANCES
#define MAX_FACE_NUM 1
FACEP_CONTEXT_T g_facep_context[MAX_FACEP_CNT];
CVI_S32 g_facep_is_pause = 0;
CVI_S32 g_facep_is_pause_ok = 1;
RECT_S face_rects[MAX_FACE_NUM] = {0};
DrawRectFun g_draw_rect_func = NULL;
SmileFun g_smile_func = NULL;
SmileFun g_smile_pre_func = NULL;
SmileFun g_smile_post_func = NULL;
FACEP_SERVICE_HANDLE_T g_facep_handle_map[MAX_FACEP_CNT] = {0};

static CVI_S32 facep_get_model_info(CVI_CHAR *model_path, TDLModel *model_index){
    CVI_S32 ret = CVI_SUCCESS;
    if (strstr(model_path, "scrfd_det_face") != NULL) {
        *model_index = TDL_MODEL_SCRFD_DET_FACE;
    } else if (strstr(model_path, "cls_attribute_gender_age_glass_emotion") != NULL) {
        *model_index = TDL_MODEL_CLS_ATTRIBUTE_GENDER_AGE_GLASS_EMOTION;
    } else {
        ret = CVI_FAILURE;
    }
    return ret;
}

static void* facep_infer_func_get(TDLModel model_id){
    void* ret = NULL;
    switch (model_id)
    {
        case TDL_MODEL_SCRFD_DET_FACE:
            ret = TDL_FaceDetection;
            CVI_LOGI("SCRFD_DET_FACE");
        break;

        case TDL_MODEL_CLS_ATTRIBUTE_GENDER_AGE_GLASS_EMOTION:
            ret = TDL_FaceAttribute;
            CVI_LOGI("CLS_ATTRIBUTE_GENDER_AGE_GLASS_EMOTION");
        break;

        default:
            CVI_LOGE("model id (%d) invalid!", model_id);
            ret = NULL;
        break;
    }

    return ret;
}

const char *emotion_to_text(int code) {
    switch (code) {
      case 0:
        return "anger";
      case 1:
        return "disgust";
      case 2:
        return "fear";
      case 3:
        return "happy";
      case 4:
        return "neutral";
      case 5:
        return "sad";
      case 6:
        return "surprise";
      default:
        return "unknown";
    }
}

static CVI_S32 facep_draw_rect(CVI_U32 osd_id, CVI_U32 osd_w, CVI_U32 osd_h,
                                CVI_U32 osd_mirror, CVI_U32 osd_flip,
                                TDLFace face_meta) {
    CVI_U32 i = 0;
    CVI_S32 ret = CVI_SUCCESS;
    (void)osd_h;
    (void)osd_flip;

    memset(face_rects, 0, sizeof(RECT_S));

    for(i = 0; i < face_meta.size; i++){
        if(i >= MAX_FACE_NUM){
            break;
        }else{

            if(osd_mirror) {
                face_rects[i].s32X = osd_w - face_meta.info[i].box.x2;
                face_rects[i].s32Y = face_meta.info[i].box.y1;
                face_rects[i].u32Width = osd_w - face_meta.info[i].box.x1;
                face_rects[i].u32Height = face_meta.info[i].box.y2;
            } else {
                face_rects[i].s32X = face_meta.info[i].box.x1;
                face_rects[i].s32Y = face_meta.info[i].box.y1;
                face_rects[i].u32Width = face_meta.info[i].box.x2;
                face_rects[i].u32Height = face_meta.info[i].box.y2;
            }
            // CVI_LOGI("%d %d %d %d", face_rects[i].s32X, face_rects[i].s32Y, face_rects[i].u32Width, face_rects[i].u32Height);
        }
    }

    if(i <= MAX_FACE_NUM){
        if(g_draw_rect_func != NULL){
            ret = g_draw_rect_func(osd_id, i, face_rects);
        }
    }

    return ret;
}

static CVI_S32 facep_clear_rect(CVI_U32 osd_id){
    CVI_LOGI("in");
    TDLFace empty_face_meta;
    memset(&empty_face_meta, 0, sizeof(TDLFace));
    facep_draw_rect(osd_id, 0, 0, 0, 0, empty_face_meta);
    return CVI_SUCCESS;
}

static CVI_VOID facep_task_entry(CVI_VOID *arg){
    FACEP_CONTEXT_T* pcontext = (FACEP_CONTEXT_T*)arg;
    FACEP_SERVICE_PARAM_S* pparam = &pcontext->facep_param;
    VPSS_GRP in_vpss_grp = pparam->in_vpss_grp;
    VPSS_CHN in_vpss_chn = pparam->in_vpss_chn;
    VIDEO_FRAME_INFO_S in_frame = {0};
    TDLImage image_handle;
    TDLFace face_meta;
    CVI_S32 ret = CVI_SUCCESS;
    CVI_U32 smile_score = 0;
    CVI_U32 smile_time_circles = 0;
    CVI_S32 smile_time_interval_circles = 0;
    CVI_U32 fd_enable = 0;
    CVI_U32 fattr_enable = 0;
    CVI_U32 if_disable_need_remove_rect = 0;

    CVI_LOGI("is running: %d", pcontext->is_running);
    while(pcontext->is_running){
        if(g_facep_is_pause){
            g_facep_is_pause_ok = 1;
            sleep(1);
            continue;
        }else{
            g_facep_is_pause_ok = 0;
        }
        /* shoud lock ? */
        fd_enable = pparam->fd_enable;
        fattr_enable = pparam->fattr_enable;

        ret = CVI_VPSS_GetChnFrame(in_vpss_grp, in_vpss_chn, &in_frame, 3000);
        if (ret != CVI_SUCCESS){
            CVI_LOGE("Grp(%d)-Chn(%d) get frame failed with %#x\n", in_vpss_grp, in_vpss_chn, ret);
            usleep(100*1000);
            continue;
        }

        image_handle = TDL_WrapFrame((void*)&in_frame, false);
        memset(&face_meta, 0, sizeof(TDLFace));

        if(fd_enable || fattr_enable){
            if(pcontext->fd_infer_func != NULL){
                pcontext->fd_infer_func(pcontext->fp_tdl_handle, pparam->model_fd_id, image_handle, &face_meta);
            }else{
                CVI_LOGE("fd_infer_func is NULL");
            }
            // for (CVI_U32 i = 0; i < face_meta.size; i++) {
            //     CVI_LOGI("[%d] x1:%f, y1:%f, x2:%f, y2:%f], score:%f\n",
            //     i, face_meta.info[i].box.x1, face_meta.info[i].box.y1,
            //     face_meta.info[i].box.x2, face_meta.info[i].box.y2,
            //     face_meta.info[i].score);
            // }
        }

        if(smile_time_interval_circles > 0){
            smile_time_interval_circles--;
        }

        if(fattr_enable && face_meta.size > 0 && smile_time_interval_circles <= 0){
            if(pcontext->fattr_infer_func != NULL){
                pcontext->fattr_infer_func(pcontext->fp_tdl_handle, pparam->model_fattr_id, image_handle, &face_meta);
            }else{
                CVI_LOGE("fd_infer_func is NULL");
            }
            /* only use face0 */
            smile_score = (CVI_U32)face_meta.info[0].emotion_score;
            // CVI_LOGI("smile_score:%d smile_time_circles:%d", smile_score, smile_time_circles);
            if(smile_score == 3){
                ++smile_time_circles;
            }
            if(g_smile_func != NULL && smile_time_circles > 5){
                smile_time_circles = 0;
                smile_time_interval_circles = 20;
                if(g_smile_pre_func != NULL){
                    g_smile_pre_func();
                }
                g_smile_func();
                if(g_smile_post_func != NULL){
                    g_smile_post_func();
                }
            }
            // CVI_LOGI("Gender:%f Age:%d emotion:%s\n", face_meta.info[0].gender_score, (int)round(face_meta.info[0].age * 100.0), emotion_to_text(face_meta.info[0].emotion_score));
        }else{
            smile_time_circles = 0;
        }

        ret = CVI_VPSS_ReleaseChnFrame(in_vpss_grp, in_vpss_chn, &in_frame);
        if (ret != CVI_SUCCESS){
            CVI_LOGE("Grp(%d)-Chn(%d) release frame failed with %#x", in_vpss_grp, in_vpss_chn, ret);
        }
        TDL_DestroyImage(image_handle);

        if(fd_enable){
            if_disable_need_remove_rect = 1;
            facep_draw_rect(pparam->osd_id, pparam->in_width, pparam->in_height, pparam->osd_mirror, 0, face_meta);
        }else{
            /* remove rect */
            if(if_disable_need_remove_rect){
                CVI_LOGI("remove last rect");
                facep_clear_rect(pparam->osd_id);
                if_disable_need_remove_rect = 0;
            }
        }

        TDL_ReleaseFaceMeta(&face_meta);
        usleep(100*1000);
    }
    g_facep_is_pause_ok = 1;
    CVI_LOGI("is exit");
}

static void facep_param_dump(FACEP_SERVICE_PARAM_S* pparam){
    if(pparam != NULL){
        CVI_LOGI("in_vpss_grp:%d in_vpss_chn:%d\n",
            pparam->in_vpss_grp, pparam->in_vpss_chn);
        CVI_LOGI("fd_enable:%d model_fd_id:%d model_fd_thres:%f model_fd_path:%s\n",
            pparam->fd_enable, pparam->model_fd_id,
            pparam->model_fd_thres, pparam->model_fd_path);
        CVI_LOGI("fattr_enable:%d model_fattr_id:%d model_fattr_path:%s\n",
            pparam->fattr_enable, pparam->model_fattr_id, pparam->model_fattr_path);
    }
}

static CVI_S32 facep_proc_init(FACEP_CONTEXT_T* pcontext){
    CVI_LOGI("FP init ------------------> start");
    CVI_S32 ret = CVI_SUCCESS;
    FACEP_SERVICE_PARAM_S* pparam = &pcontext->facep_param;
    TDLModel mode_id = TDL_MODEL_INVALID;

    facep_get_model_info(pparam->model_fd_path, &mode_id);
    pparam->model_fd_id = (TDLModel)mode_id;

    facep_get_model_info(pparam->model_fattr_path, &mode_id);
    pparam->model_fattr_id = (TDLModel)mode_id;

    pcontext->fd_infer_func = (PfpFaceDetInferFunc)facep_infer_func_get(pparam->model_fd_id);
    if (pcontext->fd_infer_func == NULL){
        CVI_LOGE("unsupported model id: %d", pparam->model_fd_id);
        return CVI_FAILURE;
    }

    pcontext->fattr_infer_func = (PfpFaceAttrInferFunc)facep_infer_func_get(pparam->model_fattr_id);
    if (pcontext->fd_infer_func == NULL){
        CVI_LOGE("unsupported model id: %d", pparam->model_fattr_id);
        return CVI_FAILURE;
    }

    facep_param_dump(pparam);

    pcontext->fp_tdl_handle = TDL_CreateHandle(0);
    if(pcontext->fp_tdl_handle == NULL){
        CVI_LOGE("TDL_FD_CreateHandle failed");
        return CVI_FAILURE;
    }

    ret = TDL_OpenModel(pcontext->fp_tdl_handle, pparam->model_fd_id, pparam->model_fd_path, NULL, 0);
    if(ret != CVI_SUCCESS){
       CVI_LOGE("%s TDL_OpenModel failed with %#x!", pparam->model_fd_path, ret);
       return CVI_FAILURE;
    }

    ret = TDL_OpenModel(pcontext->fp_tdl_handle, pparam->model_fattr_id, pparam->model_fattr_path, NULL, 0);
    if(ret != CVI_SUCCESS){
       CVI_LOGE("%s TDL_OpenModel failed with %#x!", pparam->model_fattr_path, ret);
       return CVI_FAILURE;
    }

    ret = TDL_SetModelThreshold(pcontext->fp_tdl_handle,  pparam->model_fd_id, pparam->model_fd_thres);
    if (ret != CVI_SUCCESS){
        CVI_LOGE("%s TDL_SetModelThreshold failed with %#x!", pparam->model_fd_path, ret);
        return CVI_FAILURE;
    }

    CVI_LOGI("FP init ------------------> done");
    return CVI_SUCCESS;
}


static CVI_S32 facep_start(FACEP_CONTEXT_T* pcontext){
    CVI_S32 ret = CVI_SUCCESS;

    CVI_LOGI("in");

    if(pcontext->is_running){
        CVI_LOGE("FP has started");
        return CVI_SUCCESS;
    }

    ret = facep_proc_init(pcontext);
    if(ret != CVI_SUCCESS){
        CVI_LOGE("facep_proc_init failed with %#x", ret);
        return CVI_FAILURE;
    }

    pcontext->is_running = 1;
    g_facep_is_pause = 0;
    pthread_mutex_init(&pcontext->mutex, NULL);

    OSAL_TASK_ATTR_S facep_task;
    static char facep_name[16] = {0};
    snprintf(facep_name, sizeof(facep_name), "facep_%d", pcontext->handle);
    facep_task.name = facep_name;
    facep_task.entry = facep_task_entry;
    facep_task.param = (void*)pcontext;
    facep_task.priority = OSAL_TASK_PRI_RT_MID;
    facep_task.detached = CVI_FALSE;
    facep_task.stack_size = 256 * 1024;
    ret = OSAL_TASK_Create(&facep_task, &pcontext->facep_task_handle);
    if (ret != OSAL_SUCCESS) {
        CVI_LOGE("facep task create failed, %#x", ret);
        return CVI_FAILURE;
    }

    return CVI_SUCCESS;
}

static CVI_S32 facep_stop(FACEP_CONTEXT_T* pcontext){
    CVI_S32 ret = CVI_SUCCESS;
    FACEP_SERVICE_PARAM_S* pparam = NULL;

    CVI_LOGI("in");

    if(!pcontext->is_running){
        CVI_LOGE("FP has stoped");
        return CVI_SUCCESS;
    }

    pparam = &pcontext->facep_param;
    pcontext->is_running = 0;
    ret = OSAL_TASK_Join(pcontext->facep_task_handle);
    if (ret != OSAL_SUCCESS) {
        CVI_LOGE("phs_piv task join failed, %#x", ret);
        return CVI_FAILURE;
    }
    OSAL_TASK_Destroy(&pcontext->facep_task_handle);

    facep_clear_rect(pparam->osd_id);
    TDL_CloseModel(pcontext->fp_tdl_handle, pparam->model_fd_id);
    TDL_CloseModel(pcontext->fp_tdl_handle, pparam->model_fattr_id);
    ret = TDL_DestroyHandle(pcontext->fp_tdl_handle);
    if (ret != CVI_SUCCESS){
        CVI_LOGE("TDL_FD_DestroyHandle failed with 0x%x!\n", ret);
        return CVI_FAILURE;
    }
    pcontext->fp_tdl_handle = NULL;
    pthread_mutex_destroy(&pcontext->mutex);

    return ret;
}

static CVI_S32 facep_get_handle(FACEP_SERVICE_HANDLE_T* phandle){
    CVI_U32 i = 0;
    FACEP_SERVICE_HANDLE_T handle = -1;
    for(i = 0; i < MAX_FACEP_CNT; i++){
        if(g_facep_handle_map[i] == 0){
            g_facep_handle_map[i] = 1;
            handle = i;
            break;
        }
    }
    if(handle == -1){
        CVI_LOGE("no handle");
        return CVI_FAILURE;
    }else{
        *phandle = handle;
    }
    return CVI_SUCCESS;
}

static CVI_S32 facep_reset_handle(FACEP_SERVICE_HANDLE_T handle){
    g_facep_handle_map[handle] = 0;
    return CVI_SUCCESS;
}


CVI_S32 FACEP_SERVICE_Create(FACEP_SERVICE_HANDLE_T* phandle, FACEP_SERVICE_PARAM_S* pparam){
    CVI_S32 ret = CVI_SUCCESS;
    FACEP_CONTEXT_T* pcontext = NULL;

    ret = facep_get_handle(phandle);
    if(ret != CVI_SUCCESS){
        CVI_LOGE("get handle failed");
        return CVI_FAILURE;
    }else {
        pcontext = &g_facep_context[*phandle];
        pcontext->handle = *phandle;
        memcpy(&pcontext->facep_param, pparam, sizeof(FACEP_SERVICE_PARAM_S));
    }

    if(!pparam->fd_enable && !pparam->fattr_enable){
        CVI_LOGE("both fd(%d) and fattr(%d) disabled", pparam->fd_enable, pparam->fattr_enable);
        g_facep_is_pause_ok = 1;
        return CVI_SUCCESS;
    }else{
        ret = facep_start(pcontext);
    }

    return ret;
}

CVI_S32 FACEP_SERVICE_Destory(FACEP_SERVICE_HANDLE_T handle){
    CVI_S32 ret = CVI_SUCCESS;
    FACEP_CONTEXT_T* pcontext = NULL;

    if(handle < MAX_FACEP_CNT && handle >= 0){
        pcontext = &g_facep_context[handle];

        ret = facep_stop(pcontext);
        facep_reset_handle(handle);
    }else{
        CVI_LOGE("invalid handle:%d", handle);
        return CVI_FAILURE;
    }
    CVI_LOGI("handle %d finish.", handle);
    return ret;
}

CVI_S32 FACEP_SERVICE_Pause(CVI_VOID){
    g_facep_is_pause = 1;
    while(g_facep_is_pause_ok == 0){
        g_facep_is_pause = 1;
        usleep(1000);
    }
    return CVI_SUCCESS;
}

CVI_S32 FACEP_SERVICE_Resume(CVI_VOID){
    g_facep_is_pause = 0;
    return CVI_SUCCESS;
}

CVI_S32 FACEP_SERVICE_Register_DrawRects_Callback(DrawRectFun pfun){
    if(pfun != NULL){
        g_draw_rect_func = pfun;
        return CVI_SUCCESS;
    }
    return CVI_FAILURE;
}

CVI_S32 FACEP_SERVICE_Register_Smile_Callback(SmileFun pfun){
    if(pfun != NULL){
        g_smile_func = pfun;
        return CVI_SUCCESS;
    }
    return CVI_FAILURE;
}

CVI_S32 FACEP_SERVICE_Register_Smile_Pre_Callback(SmileFun pfun){
    if(pfun != NULL){
        g_smile_pre_func = pfun;
        return CVI_SUCCESS;
    }
    return CVI_FAILURE;
}

CVI_S32 FACEP_SERVICE_Unregister_Smile_Pre_Callback(CVI_VOID){
    g_smile_pre_func = NULL;
    return CVI_SUCCESS;
}

CVI_S32 FACEP_SERVICE_Register_Smile_Post_Callback(SmileFun pfun){
    if(pfun != NULL){
        g_smile_post_func = pfun;
        return CVI_SUCCESS;
    }
    return CVI_FAILURE;
}

CVI_S32 FACEP_SERVICE_Unregister_Smile_Post_Callback(CVI_VOID){
    g_smile_post_func = NULL;
    return CVI_SUCCESS;
}

CVI_S32 FACEP_SERVICE_Enable(FACEP_SERVICE_HANDLE_T handle, FACEP_FUNC_ID_E id){
    CVI_S32 ret = CVI_SUCCESS;
    FACEP_CONTEXT_T* pcontext = NULL;
    FACEP_SERVICE_PARAM_S* pparam = NULL;
    if(handle < MAX_FACEP_CNT && handle >= 0){
        pcontext = &g_facep_context[handle];
        pparam = &pcontext->facep_param;
    }else{
        CVI_LOGE("handle(%d)", handle);
        return CVI_FAILURE;
    }
    switch(id){
        case FACEP_FUNC_ID_FD:
            pparam->fd_enable = 1;
        break;

        case FACEP_FUNC_ID_FATTR:
            pparam->fattr_enable = 1;
        break;

        case FACEP_FUNC_ID_BUTT:
            pparam->fd_enable = 1;
            pparam->fattr_enable = 1;
        break;

        default:
            CVI_LOGE("unsupport id(%d)", id);
        break;
    }

    if(!pcontext->is_running){
        ret = facep_start(pcontext);
    }

    return ret;
}

CVI_S32 FACEP_SERVICE_Disable(FACEP_SERVICE_HANDLE_T handle, FACEP_FUNC_ID_E id){
    CVI_S32 ret = CVI_SUCCESS;
    FACEP_CONTEXT_T* pcontext = NULL;
    FACEP_SERVICE_PARAM_S* pparam = NULL;
    if(handle < MAX_FACEP_CNT && handle >= 0){
        pcontext = &g_facep_context[handle];
        pparam = &pcontext->facep_param;
    }else{
        CVI_LOGE("handle(%d)", handle);
        return CVI_FAILURE;
    }
    switch(id){
        case FACEP_FUNC_ID_FD:
            pparam->fd_enable = 0;
        break;

        case FACEP_FUNC_ID_FATTR:
            pparam->fattr_enable = 0;
        break;

        case FACEP_FUNC_ID_BUTT:
            pparam->fd_enable = 0;
            pparam->fattr_enable = 0;
        break;

        default:
            CVI_LOGE("unsupport id(%d)", id);
        break;
    }

    if(pparam->fd_enable == 0 && pparam->fattr_enable == 0 && pcontext->is_running){
        ret =  facep_stop(pcontext);
    }

    return ret;
}

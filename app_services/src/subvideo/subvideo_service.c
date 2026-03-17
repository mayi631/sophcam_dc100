#include "cvi_log.h"
#include "mapi.h"
#include "osal.h"
#include <pthread.h>
#include "subvideo_service.h"

typedef struct _vcap_get_frame_cb_list VCAP_GET_FRAME_CB_LIST;

struct _vcap_get_frame_cb_list {
    void *arg;
    void *callback;
    char name[64];
    VCAP_GET_FRAME_CB_LIST *next;
};

typedef struct {
    int8_t id;
    OSAL_TASK_HANDLE_S task;
    MAPI_VENC_HANDLE_T venc_handle;
    int32_t vpss_grp;
    int32_t chnid;
    volatile int8_t stop_venc;
    volatile int32_t references;
    pthread_mutex_t v_mutex;
    pthread_mutex_t v_cb_list_mutex;
    VCAP_GET_FRAME_CB_LIST *vcap_cb_list;
} VIDEO_CONTEXT;

// static pthread_mutex_t v_mutex = PTHREAD_MUTEX_INITIALIZER;
// static pthread_mutex_t v_cb_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static VIDEO_CONTEXT v_ctx[MAX_CAMERA_INSTANCES] = {0};

#define FRAME_STREAM_SEGMENT_MAX_NUM (8)

static void process_one_frame(int8_t id) {
    VENC_STREAM_S venc_stream = {0};

    if (v_ctx[id].venc_handle == NULL) {
        return ;
    }

    if (0 != MAPI_VENC_GetStreamTimeWait(v_ctx[id].venc_handle, &venc_stream, 1000)) {
        CVI_LOGE("[%p]: MAPI_VENC_GetStreamTimeWait failed", v_ctx[id].venc_handle);
        return ;
    }

    if (venc_stream.u32PackCount <= 0 || venc_stream.u32PackCount > FRAME_STREAM_SEGMENT_MAX_NUM) {
        MAPI_VENC_ReleaseStream(v_ctx[id].venc_handle, &venc_stream);
        return ;
    }

    pthread_mutex_lock(&v_ctx[id].v_cb_list_mutex);
    VCAP_GET_FRAME_CB_LIST *cur = v_ctx[id].vcap_cb_list;
    while (cur && v_ctx[id].references > 0) {
        VIDEO_SERVICR_VCAP_GET_FRAME_CALLBACK *callback = (VIDEO_SERVICR_VCAP_GET_FRAME_CALLBACK *)cur->callback;
        callback(&venc_stream, cur->arg);
        cur = cur->next;
    }
    pthread_mutex_unlock(&v_ctx[id].v_cb_list_mutex);

    MAPI_VENC_ReleaseStream(v_ctx[id].venc_handle, &venc_stream);
}

static void vcap_task(void *arg) {
    // UNUSED(arg);
    VIDEO_CONTEXT *v_tmp = (VIDEO_CONTEXT *)arg;
    int8_t id = v_tmp->id;
    while (v_ctx[id].references > 0) {
        if (v_ctx[id].vcap_cb_list == NULL) {
            if(v_ctx[id].stop_venc == 1) {
                MAPI_VENC_UnBindVproc(v_ctx[id].venc_handle, v_ctx[id].vpss_grp, v_ctx[id].chnid);
                MAPI_VENC_StopRecvFrame(v_ctx[id].venc_handle);
                v_ctx[id].stop_venc = 0;
            }
            // cvi_osal_task_sleep(10000);  // 10 ms
        } else {
            if(v_ctx[id].stop_venc == 0) {
                MAPI_VENC_BindVproc(v_ctx[id].venc_handle, v_ctx[id].vpss_grp, v_ctx[id].chnid);
                MAPI_VENC_StartRecvFrame(v_ctx[id].venc_handle, -1);
                v_ctx[id].stop_venc = 1;
            }
            process_one_frame(id);
        }
        OSAL_TASK_Sleep(10000);
    }
}

static int32_t set_callback(int8_t id, const char *name, void *cb, void *arg) {
    if (name == NULL || cb == NULL) {
        CVI_LOGE("Name and callback shouldn't be NULL");
        return -1;
    }

    VCAP_GET_FRAME_CB_LIST *head = (VCAP_GET_FRAME_CB_LIST *)malloc(sizeof(VCAP_GET_FRAME_CB_LIST));
    if (NULL == head) {
        CVI_LOGE("Failed to alloc of [%s] callback", name);
        return -1;
    }

    head->arg = arg;
    head->callback = cb;
    snprintf(head->name, sizeof(head->name), "%s", name);

    pthread_mutex_lock(&v_ctx[id].v_cb_list_mutex);

    head->next = v_ctx[id].vcap_cb_list;
    v_ctx[id].vcap_cb_list = head;

    pthread_mutex_unlock(&v_ctx[id].v_cb_list_mutex);

    return 0;
}

static int32_t unset_callback(int8_t id, const char *name) {
    if (!name) {
        CVI_LOGE("callback name is NULL");
        return -1;
    }

    pthread_mutex_lock(&v_ctx[id].v_cb_list_mutex);

    int32_t ret = -1;
    VCAP_GET_FRAME_CB_LIST *head = v_ctx[id].vcap_cb_list;
    VCAP_GET_FRAME_CB_LIST *cur = head;
    VCAP_GET_FRAME_CB_LIST *prev = NULL;

    if (!head) {
        ret = 0;
        goto END;
    }

    while (cur) {
        if (0 == strncmp(cur->name, name, sizeof(cur->name))) {
            if (!prev) {
                head = head->next;
            } else {
                prev->next = cur->next;
            }
            if (cur)
                free(cur);
            break;
        }

        prev = cur;
        cur = cur->next;
    }

     v_ctx[id].vcap_cb_list = head;

    ret = 0;
END:

    pthread_mutex_unlock(&v_ctx[id].v_cb_list_mutex);

    return ret;
}

int32_t VIDEO_SERVICR_CallbackSet(int8_t id, const char *name, VIDEO_SERVICR_VCAP_GET_FRAME_CALLBACK *cb, void *arg) {
    return set_callback(id, name, cb, arg);
}

int32_t VIDEO_SERVICR_CallbackUnSet(int8_t id, const char *name) {
    return unset_callback(id, name);
}

int32_t VIDEO_SERVICR_TaskStart(int8_t id, MAPI_VENC_HANDLE_T venc_handle, int32_t vpss_grp, int32_t chnid) {
    for (int i = 0; i < MAX_CAMERA_INSTANCES; ++i) {
        pthread_mutex_init(&v_ctx[i].v_mutex, NULL);
        pthread_mutex_init(&v_ctx[i].v_cb_list_mutex, NULL);
    }

    pthread_mutex_lock(&v_ctx[id].v_mutex);

    int32_t ret = -1;
    int32_t rc = OSAL_SUCCESS;
    v_ctx[id].id = id;
    OSAL_TASK_ATTR_S ta;

    v_ctx[id].references++;

    if (v_ctx[id].task != NULL) {
        ret = 0;
        goto END;
    }

    v_ctx[id].venc_handle = venc_handle;
    v_ctx[id].vpss_grp = vpss_grp;
    v_ctx[id].chnid = chnid;

    ta.name = "vstream_send";
    ta.entry = vcap_task;
    ta.param = (void *)&v_ctx[id];
    ta.priority = OSAL_TASK_PRI_RT_MID;
    ta.detached = false;

    if (OSAL_SUCCESS != (rc = OSAL_TASK_Create(&ta, &v_ctx[id].task))) {
        CVI_LOGE("vstream_send task create failed, %d", rc);
        v_ctx[id].references--;
        goto END;
    }

    ret = 0;
    CVI_LOGI("VIDEO_SERVICR_TaskStart ID:%d\n",id);

END:

    pthread_mutex_unlock(&v_ctx[id].v_mutex);

    return ret;
}

int32_t VIDEO_SERVICR_TaskStop(int8_t id) {
    pthread_mutex_lock(&v_ctx[id].v_mutex);

    // still has video service running
    if (--v_ctx[id].references > 0) {
        pthread_mutex_unlock(&v_ctx[id].v_mutex);
        return 0;
    }

    if (v_ctx[id].task == NULL) {
        pthread_mutex_unlock(&v_ctx[id].v_mutex);
        return -1;
    }

    int32_t rc;
    if (OSAL_SUCCESS != (rc = OSAL_TASK_Join(v_ctx[id].task))) {
        CVI_LOGE("vstream_send task join failed, %d", rc);
        pthread_mutex_unlock(&v_ctx[id].v_mutex);
        return -1;
    }
    OSAL_TASK_Destroy(&v_ctx[id].task);

    v_ctx[id].task = NULL;

    pthread_mutex_unlock(&v_ctx[id].v_mutex);

    for (int i = 0; i < MAX_CAMERA_INSTANCES; ++i) {
        pthread_mutex_destroy(&v_ctx[i].v_mutex);
        pthread_mutex_destroy(&v_ctx[i].v_cb_list_mutex);
    }

    memset(&v_ctx[id], 0, sizeof(VIDEO_CONTEXT));

    CVI_LOGI("VIDEO_SERVICR_TaskStop ID:%d\n",id);
    return 0;
}
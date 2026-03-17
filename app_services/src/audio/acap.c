#include <pthread.h>

#include "audio_service.h"
#include "cvi_log.h"
#include "mapi.h"
#include "mapi_acap.h"
#include "osal.h"

typedef struct _acap_get_frame_cb_list ACAP_GET_FRAME_CB_LIST;
struct _acap_get_frame_cb_list {
	void *arg;
	void *callback;
	char name[64];
	ACAP_GET_FRAME_CB_LIST *next;
};

typedef struct {
	OSAL_TASK_HANDLE_S task;
	MAPI_ACAP_HANDLE_T acap_handle;
	MAPI_AENC_HANDLE_T aenc_handle;
	volatile int32_t references;
	ACAP_GET_FRAME_CB_LIST *acap_cb_list;
	ACAP_GET_FRAME_CB_LIST *aac_cb_list;
} AUDIO_CONTEXT;

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t g_cb_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static AUDIO_CONTEXT g_ctx = {0};

static void process_aenc_frame(AUDIO_STREAM_S *stream)
{
	pthread_mutex_lock(&g_cb_list_mutex);

	ACAP_GET_FRAME_CB_LIST *cur = g_ctx.aac_cb_list;
	while (cur && g_ctx.references > 0) {
		AUDIO_SERVICR_ACAP_GET_AAC_FRAME_CALLBACK *callback = (AUDIO_SERVICR_ACAP_GET_AAC_FRAME_CALLBACK *)cur->callback;
		callback(stream, cur->arg);
		cur = cur->next;
	}

	pthread_mutex_unlock(&g_cb_list_mutex);
}

static inline bool is_aac_callback_exist()
{
	bool is_exist = false;

	pthread_mutex_lock(&g_cb_list_mutex);
	is_exist = (NULL != g_ctx.aac_cb_list);
	pthread_mutex_unlock(&g_cb_list_mutex);

	return is_exist;
}

static void process_one_frame()
{
	AUDIO_FRAME_S frame = {0};
	AEC_FRAME_S aec_frame = {0};

	if (g_ctx.acap_handle == NULL) {
		return;
	}

	if (0 != MAPI_ACAP_GetFrame(g_ctx.acap_handle, &frame, &aec_frame)) {
		CVI_LOGE("failed to MAPI_ACAP_GetFrame");
		return;
	}

	pthread_mutex_lock(&g_cb_list_mutex);
	ACAP_GET_FRAME_CB_LIST *cur = g_ctx.acap_cb_list;
	while (cur && g_ctx.references > 0) {
		AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK *callback = (AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK *)cur->callback;
		callback(&frame, &aec_frame, cur->arg);
		cur = cur->next;
	}
	pthread_mutex_unlock(&g_cb_list_mutex);

	/// TODO: check aenc is enabled
	if (is_aac_callback_exist()) {
		AUDIO_STREAM_S stream = {0};

		if (0 == MAPI_AENC_GetStream(g_ctx.aenc_handle, &stream, 0)) {
			if (0 < stream.u32Len) {
				process_aenc_frame(&stream);
			}
		} else {
			CVI_LOGE("MAPI_AENC_GetStream failed");
		}
	}

	MAPI_ACAP_ReleaseFrame(g_ctx.acap_handle, &frame, &aec_frame);
}

static void acap_task(void *arg)
{
	(void)arg;

	while (g_ctx.references > 0) {
		if (g_ctx.acap_cb_list == NULL && !is_aac_callback_exist()) {
			OSAL_TASK_Sleep(10000); // 10 ms
		} else {
			process_one_frame();
		}
		OSAL_TASK_Sleep(10000);
	}
}

static int32_t set_callback(const char *name, void *cb, void *arg, bool is_aac)
{
	if (name == NULL || cb == NULL) {
		CVI_LOGE("Name and callback shouldn't be NULL");
		return -1;
	}

	ACAP_GET_FRAME_CB_LIST *head = (ACAP_GET_FRAME_CB_LIST *)malloc(sizeof(ACAP_GET_FRAME_CB_LIST));
	if (NULL == head) {
		CVI_LOGE("Failed to alloc of [%s] callback", name);
		return -1;
	}

	head->arg = arg;
	head->callback = cb;
	snprintf(head->name, sizeof(head->name), "%s", name);

	pthread_mutex_lock(&g_cb_list_mutex);

	if (is_aac) {
		head->next = g_ctx.aac_cb_list;
		g_ctx.aac_cb_list = head;
	} else {
		head->next = g_ctx.acap_cb_list;
		g_ctx.acap_cb_list = head;
	}

	pthread_mutex_unlock(&g_cb_list_mutex);

	return 0;
}

static int32_t unset_callback(const char *name, bool is_aac)
{
	if (!name) {
		CVI_LOGE("callback name is NULL");
		return -1;
	}

	pthread_mutex_lock(&g_cb_list_mutex);

	int32_t ret = -1;
	ACAP_GET_FRAME_CB_LIST *head = (is_aac) ? g_ctx.aac_cb_list : g_ctx.acap_cb_list;
	ACAP_GET_FRAME_CB_LIST *cur = head;
	ACAP_GET_FRAME_CB_LIST *prev = NULL;

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

	if (is_aac) {
		g_ctx.aac_cb_list = head;
	} else {
		g_ctx.acap_cb_list = head;
	}

	ret = 0;
END:

	pthread_mutex_unlock(&g_cb_list_mutex);

	return ret;
}

int32_t AUDIO_SERVICR_ACAP_CallbackSet(const char *name, AUDIO_SERVICR_ACAP_GET_FRAME_CALLBACK *cb, void *arg)
{
	return set_callback(name, cb, arg, false);
}

int32_t AUDIO_SERVICR_ACAP_CallbackUnset(const char *name)
{
	return unset_callback(name, false);
}

int32_t AUDIO_SERVICR_ACAP_AacCallbackSet(const char *name, AUDIO_SERVICR_ACAP_GET_AAC_FRAME_CALLBACK *cb, void *arg)
{
	return set_callback(name, cb, arg, true);
}

int32_t AUDIO_SERVICR_ACAP_AacCallbackUnset(const char *name)
{
	return unset_callback(name, true);
}

int32_t AUDIO_SERVICR_ACAP_TaskStart(MAPI_ACAP_HANDLE_T acap_handle, MAPI_AENC_HANDLE_T aenc_handle)
{
	pthread_mutex_lock(&g_mutex);

	int32_t ret = -1;
	int32_t rc = OSAL_ERR_FAILURE;
	OSAL_TASK_ATTR_S ta;

	g_ctx.references++;

	if (g_ctx.task != NULL) {
		ret = 0;
		goto END;
	}

	g_ctx.acap_handle = acap_handle;
	g_ctx.aenc_handle = aenc_handle;

	ta.name = "audio_capture";
	ta.entry = acap_task;
	ta.param = NULL;
	ta.priority = OSAL_TASK_PRI_RT_MID;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	if (OSAL_SUCCESS != (rc = OSAL_TASK_Create(&ta, &g_ctx.task))) {
		CVI_LOGE("rs_video task create failed, %d", rc);
		g_ctx.references--;
		goto END;
	}

	ret = 0;

END:

	pthread_mutex_unlock(&g_mutex);

	return ret;
}

int32_t AUDIO_SERVICR_ACAP_TaskStop()
{
	pthread_mutex_lock(&g_mutex);

	// still has video service running
	if (--g_ctx.references > 0) {
		pthread_mutex_unlock(&g_mutex);
		return 0;
	}

	if (g_ctx.task == NULL) {
		pthread_mutex_unlock(&g_mutex);
		return -1;
	}

	int32_t rc;
	if (OSAL_SUCCESS != (rc = OSAL_TASK_Join(g_ctx.task))) {
		CVI_LOGE("rs_audio task join failed, %d", rc);
		pthread_mutex_unlock(&g_mutex);
		return -1;
	}
	OSAL_TASK_Destroy(&g_ctx.task);

	g_ctx.task = NULL;

	pthread_mutex_unlock(&g_mutex);

	return 0;
}

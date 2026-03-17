#ifndef __ADAS_SERVICE_H__
#define __ADAS_SERVICE_H__

#include <stdint.h>
#include "osal.h"
#include "mapi.h"
#include "cvi_tdl_app.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define GOTO_IF_FAILED(func, result, label)                                    \
	do {                                                                       \
		result = (func);                                                       \
		if (result != 0) {                                                     \
			printf("failed! ret=%#x, at %s:%d\n", result, __FILE__, __LINE__); \
			goto label;                                                        \
		}                                                                      \
	} while (0)

typedef int32_t (*ADAS_SERVICE_VOICE_CALLBACK)(int32_t index);
typedef int32_t (*ADAS_SERVICE_LABEL_CALLBACK)(int32_t camid, int32_t index, uint32_t count, char *coordinates);
typedef void *ADAS_SERVICE_HANDLE_T;

typedef struct _ADAS_CALLBACK_S {
	void *pfnVoiceCb;
	void *pfnLabelCb;
} ADAS_SERVICE_CALLBACK_S;

typedef struct _ADAS_SERVICE_VPROC_S {
	MAPI_VPROC_HANDLE_T vprocHandle;
	int32_t vprocId;
	uint32_t vprocChnId;
	// uint32_t                   isExtVproc;
} ADAS_SERVICE_VPROC_ATTR_S;

typedef struct _ADAS_SERVICE_MODEL_ATTR_S {
	float fps;
	int32_t width;
	int32_t height;
	char CarModelPath[128];
	char LaneModelPath[128];
} ADAS_SERVICE_MODEL_ATTR_S;

typedef struct _ADAS_SERVICE_ATTR_S {
	ADAS_SERVICE_HANDLE_T ADASHdl;
	ADAS_SERVICE_CALLBACK_S stADASCallback;
	ADAS_SERVICE_VPROC_ATTR_S stVprocAttr;
	ADAS_SERVICE_MODEL_ATTR_S stADASModelAttr;
} ADAS_SERVICE_ATTR_S;

typedef struct _ADAS_SERVICE_PARAM_S {
	int32_t camid;
	ADAS_SERVICE_MODEL_ATTR_S stADASModelParam;
	ADAS_SERVICE_VPROC_ATTR_S stVPSSParam;
	void *adas_voice_event_cb;
	void *adas_label_event_cb;
} ADAS_SERVICE_PARAM_S;

typedef struct _ADAS_SERVICE_CTX_S {
	int32_t id;
	int32_t state;
	void *attr;
	OSAL_TASK_HANDLE_S adas_task;
	pthread_mutex_t adas_mutex;
	cvitdl_app_handle_t app_handle;
	cvitdl_handle_t tdl_handle;
} ADAS_SERVICE_CTX_S, *ADAS_CONTEXT_HANDLE_S;

typedef enum _ADAS_SERVICE_CMD_E {
	ADAS_SERVICE_NORMAL = 0,
	ADAS_SERVICE_CAR_MOVING,
	ADAS_SERVICE_CAR_CLOSING,
	ADAS_SERVICE_CAR_COLLISION,
	ADAS_SERVICE_CAR_LANE,
	ADAS_SERVICE_LABEL_CAR,
	ADAS_SERVICE_LABEL_LANE,
	ADAS_SERVICE_BUTT
} ADAS_SERVICE_CMD_E;

int32_t ADAS_SERVICE_Destroy(ADAS_SERVICE_HANDLE_T hdl);
int32_t ADAS_SERVICE_Create(ADAS_SERVICE_HANDLE_T *hdl, ADAS_SERVICE_PARAM_S *ADASParam);
void ADAS_SERVICE_SetState(int32_t id, int32_t en);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __ADAS_AERVICE_H__ */
#ifndef __PHOTO_H__
#define __PHOTO_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mapi.h"
#include "osal.h"

#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PHS_SUCCESS ((int32_t)(0))
#define PHS_ERR_FAILURE ((int32_t)(-1001))
#define PHS_ERR_NOMEM ((int32_t)(-1002))
#define PHS_ERR_TIMEOUT ((int32_t)(-1003))
#define PHS_ERR_INVALID ((int32_t)(-1004))

typedef enum _PHOTO_EVENT_E {
	PHOTO_SERVICE_EVENT_OPEN_FILE_FAILED,
	PHOTO_SERVICE_EVENT_PIV_START,
	PHOTO_SERVICE_EVENT_PIV_END,
	PHOTO_SERVICE_EVENT_PIV_ERROR,
	PHOTO_SERVICE_EVENT_SYNC_DONE,
	PHOTO_SERVICE_EVENT_BUTT
} PHOTO_SERVICE_EVENT_E;

typedef int32_t (*PHOTO_SERVICE_EVENT_CALLBACK)(PHOTO_SERVICE_EVENT_E event_type, const char *filename, void *p);

typedef void *PHOTO_SERVICE_HANDLE_T;

/* photo callback set */
typedef struct _PHOTO_CALLBACK_S {
	void *pfnNormalPhsCb;
} PHOTO_SERVICE_CALLBACK_S;

typedef enum PHOTO_SERVICE_VENC_BIND_MODE_E {
	PHOTO_SERVICE_VENC_BIND_MODE_NONE,
	PHOTO_SERVICE_VENC_BIND_MODE_VPSS,
	PHOTO_SERVICE_VENC_BIND_MODE_VI, // not support yet
	PHOTO_SERVICE_VENC_BIND_MODE_BUTT
} PHOTO_SERVICE_VENC_BIND_MODE_E;

typedef enum _PHOTO_SERVICE_FLASH_LED_MODE_E {
	PHOTO_SERVICE_FLASH_LED_MODE_NC = 0,
	PHOTO_SERVICE_FLASH_LED_MODE_NP,
	PHOTO_SERVICE_FLASH_LED_MODE_AUTO
} PHOTO_SERVICE_FLASH_LED_MODE_E;

typedef struct PHOTO_HANDLES_S {
	MAPI_VENC_HANDLE_T photo_venc_hdl;
	uint32_t photo_bufsize;
	uint32_t enable_dump_raw;

	MAPI_VPROC_HANDLE_T thumbnail_vproc;
	int32_t vproc_chn_id_thumbnail;
	MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
	uint32_t thumbnail_bufsize;

	MAPI_VPROC_HANDLE_T sub_pic_vproc;
	int32_t vproc_chn_id_sub_pic;
	MAPI_VENC_HANDLE_T sub_pic_venc_hdl;
	uint32_t sub_pic_bufsize;

	MAPI_VPROC_HANDLE_T src_vproc;
	int32_t src_vproc_chn_id;

	MAPI_VCAP_SENSOR_HANDLE_T src_vcap;

	MAPI_VPROC_HANDLE_T scale_vproc;
	int32_t scale_vproc_chn_id;
} PHOTO_SERVICE_MAPI_HANDLES_S;

/* photo attribute param */
typedef struct PHOTO_ATTR_S {
	PHOTO_SERVICE_HANDLE_T phs;
	PHOTO_SERVICE_CALLBACK_S stCallback;
	int32_t s32SnapPresize;
	PHOTO_SERVICE_MAPI_HANDLES_S handles;
	uint32_t flash_led_gpio;
	uint32_t flash_led_pulse;
	int32_t flash_led_thres;
} PHOTO_SERVICE_ATTR_S;

#ifndef LOG_PHET
#define LOG_PHET(express)                                                           \
	do {                                                                            \
		int32_t ph = express;                                                       \
		if (ph != 0) {                                                              \
			printf("\nFailed at %s: %d  (photo:0x%#x!)\n", __FILE__, __LINE__, ph); \
		}                                                                           \
	} while (0)
#endif

enum _phs_state_bit_e {
	PHS_STATE_PHOTO_CREATE_BIT = 0,
	PHS_STATE_PHOTO_BIT,
	PHS_STATE_PIV_BIT,
	PHS_STATE_PERF_BIT,
	PHS_STATE_DEBUG_BIT,
	PHS_STATE_MUTE_BIT,
	RS_STATE_BIT_MAX
};

#define PHS_STATE_IDLE (0)
#define PHS_STATE_PHOTO_CREATE_EN (1 << PHS_STATE_PHOTO_CREATE_BIT)
#define PHS_STATE_PHOTO_EN (1 << PHS_STATE_PHOTO_BIT)
#define PHS_STATE_PIV_EN (1 << PHS_STATE_PIV_BIT)
#define PHS_STATE_PERF_EN (1 << PHS_STATE_PERF_BIT)
#define PHS_STATE_DEBUG_EN (1 << PHS_STATE_DEBUG_BIT)
#define PHS_STATE_MUTE_EN (1 << PHS_STATE_MUTE_BIT)

typedef struct __phs_context {
	int32_t id;
	void *attr;

	// state
	volatile uint32_t cur_state;
	volatile uint32_t new_state;
	pthread_mutex_t state_mutex;
	volatile int32_t shutdown;

	// video task
	OSAL_TASK_HANDLE_S state_task;
	OSAL_TASK_HANDLE_S piv_task;
	OSAL_TASK_HANDLE_S thumb_task;
	OSAL_TASK_HANDLE_S sub_pic_task;

	// piv
	volatile int32_t need_thumbnail;
	volatile int32_t need_sub_pic;
	uint32_t piv_prealloclen;

	char piv_filename[128];
	int32_t piv_finish;
	pthread_mutex_t piv_mutex;
	pthread_cond_t piv_cond;
	pthread_mutex_t thumbnail_mutex;
	pthread_mutex_t sub_pic_mutex;
} phs_context_t, *phs_context_handle_t;

static inline void phs_wait_state_change(phs_context_handle_t phs)
{
	// pooling at 1ms interval
	// TODO: use sem
	do {
		// cvi_osal_task_resched();
		OSAL_TASK_Sleep(10 * 1000);
	} while (phs->cur_state != phs->new_state);
}

static inline void phs_change_state(phs_context_handle_t phs, uint32_t new_state)
{
	// set new_state
	pthread_mutex_lock(&phs->state_mutex);
	phs->new_state = new_state;
	pthread_mutex_unlock(&phs->state_mutex);
	// wait new_state
	phs_wait_state_change(phs);
}

static inline void phs_enable_state(phs_context_handle_t phs, uint32_t enable_bits)
{
	// set new_state
	pthread_mutex_lock(&phs->state_mutex);
	phs->new_state = phs->cur_state | enable_bits;
	pthread_mutex_unlock(&phs->state_mutex);
	// wait new_state
	phs_wait_state_change(phs);
}

static inline void phs_disable_state(phs_context_handle_t phs, uint32_t disable_bits)
{
	// set new_state
	pthread_mutex_lock(&phs->state_mutex);
	phs->new_state = phs->cur_state & (~disable_bits);
	pthread_mutex_unlock(&phs->state_mutex);
	// wait new_state
	phs_wait_state_change(phs);
}

typedef struct _PHOTO_SERVICE_PARAM_S {
	int32_t photo_id;
	uint32_t prealloclen;
	uint32_t enable_dump_raw;

	MAPI_VENC_HANDLE_T photo_venc_hdl;
	uint32_t photo_bufsize;

	MAPI_VPROC_HANDLE_T thumbnail_vproc;
	int32_t vproc_chn_id_thumbnail;
	MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
	uint32_t thumbnail_bufsize;

	MAPI_VPROC_HANDLE_T sub_pic_vproc;
	int32_t vproc_chn_id_sub_pic;
	MAPI_VENC_HANDLE_T sub_pic_venc_hdl;
	uint32_t sub_pic_bufsize;

	MAPI_VPROC_HANDLE_T src_vproc;
	int32_t src_vproc_chn_id;

	uint32_t flash_led_gpio;
	uint32_t flash_led_pulse;
	int32_t flash_led_thres;

	MAPI_VCAP_SENSOR_HANDLE_T src_vcap;

	MAPI_VPROC_HANDLE_T scale_vproc;
	int32_t scale_vproc_chn_id;

	void *cont_photo_event_cb;
} PHOTO_SERVICE_PARAM_S;

#define MAX_CONTEXT_CNT 4

int32_t PHOTO_SERVICE_Create(PHOTO_SERVICE_HANDLE_T *hdl, PHOTO_SERVICE_PARAM_S *param);
int32_t PHOTO_SERVICE_Destroy(PHOTO_SERVICE_HANDLE_T hdl);
void PHOTO_SERVICE_WaitPivFinish(PHOTO_SERVICE_HANDLE_T hdl);
int32_t PHOTO_SERVICE_PivCapture(PHOTO_SERVICE_HANDLE_T hdl, char *file_name);
int32_t PHOTO_SERVICE_AdjustFocus(PHOTO_SERVICE_HANDLE_T hdl , char* ratio);
int32_t PHOTO_SERVICE_SetFlashLed(PHOTO_SERVICE_HANDLE_T hdl, PHOTO_SERVICE_FLASH_LED_MODE_E mode);
int32_t PHOTO_SERVICE_SetQuality(PHOTO_SERVICE_HANDLE_T hdl, uint32_t quality);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __PHOTP_H__

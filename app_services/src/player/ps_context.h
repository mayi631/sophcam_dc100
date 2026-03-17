#ifndef __PS_CONTEXT_H__
#define __PS_CONTEXT_H__

#include "cvi_log.h"
#include "osal.h"
#include "sysutils_mq.h"
#include "mapi.h"
#include "mapi_ao.h"
#include "player_service.h"
#include "player.h"
#include "file_recover.h"

typedef struct {
	PLAYER_SERVICE_PARAM_S param;
	volatile bool playing;
	volatile bool shutdown;
	MAPI_VCODEC_E codec_type;
	OSAL_MUTEX_HANDLE_S play_mutex;
	uint32_t screen_width;
	uint32_t screen_height;
	uint32_t vdec_max_buffer_size;
	bool send_vo_again;
	// handle
	MAPI_DISP_HANDLE_T disp;
	MAPI_VPROC_HANDLE_T vproc;
	MAPI_AO_HANDLE_T ao;
	int32_t ao_channel;
	MAPI_VDEC_HANDLE_T vdec;
	PLAYER_HANDLE_T player;
	FILE_RECOVER_HANDLE_T file_recover;
	// event task
	OSAL_TASK_HANDLE_S event_task;
	MQ_ENDPOINT_HANDLE_t mq_ep;
	// handler
	PLAYER_SERVICE_EVENT_HANDLER event_handler;
	// signal and slot
	PLAYER_SERVICE_SIGNALS_S signals;
	PLAYER_SERVICE_SLOTS_S slots;
} PS_CONTEXT_T;

typedef PS_CONTEXT_T *PS_CONTEXT_HANDLE;

#endif // __PS_CONTEXT_H__

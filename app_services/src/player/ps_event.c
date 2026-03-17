#include <inttypes.h>

#include "ps_event.h"
#include "player_service_command.h"
#include "cvi_log.h"
#include "osal.h"

static int32_t shutdown_command_cb(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);

	PLAYER_SERVICE_HANDLE_T ps = (PLAYER_SERVICE_HANDLE_T)userdata;
	return PLAYER_SERVICE_Destroy(&ps);
}

static int32_t play_command_cb(MQ_MSG_S *msg, void *userdata)
{
	CVI_LOGI("Player play, len = %d, arg = %d, str = %s",
			 msg->len, msg->arg2, msg->payload);

	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)userdata;
	if ((msg->len > (int32_t)MQ_MSG_HEADER_LEN) &&
		(strlen(msg->payload) > 0)) {
		if (PLAYER_SERVICE_SetInput(ps, msg->payload) != 0) {
			CVI_LOGE("Player set input failed");
			return -1;
		}
	}

	return PLAYER_SERVICE_Play(ps);
}

static int32_t stop_command_cb(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);

	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)userdata;
	return PLAYER_SERVICE_Stop(ps);
}

static int32_t pause_command_cb(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);

	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)userdata;
	return PLAYER_SERVICE_Pause(ps);
}

static int32_t seek_command_cb(MQ_MSG_S *msg, void *userdata)
{
	CVI_LOGI("Player seek to %d ms", msg->arg2);

	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)userdata;
	return PLAYER_SERVICE_Seek(ps, msg->arg2);
}

static int32_t reset_size_command_cb(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);

	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)userdata;
	PS_PARAM_HANDLE param = &ps->param;
	return PLAYER_SERVICE_Resize(ps, param->width, param->height);
}

static int32_t reset_pos_command_cb(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);

	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)userdata;
	PS_PARAM_HANDLE param = &ps->param;
	return PLAYER_SERVICE_MoveTo(ps, param->x, param->y);
}

typedef int32_t (*ps_cmd_cb_t)(MQ_MSG_S *msg, void *userdate);
static ps_cmd_cb_t command_table[] = {
	NULL,				   // 0x00     INVALID
	shutdown_command_cb,   // 0x01     SHUTDOWN
	NULL,				   // 0x02     DEBUG
	NULL,				   // 0x03     GET_STATUS
	play_command_cb,	   // 0x04     PLAY
	stop_command_cb,	   // 0x05     STOP
	pause_command_cb,	   // 0x06     PAUSE
	seek_command_cb,	   // 0x07     SEEK
	reset_size_command_cb, // 0x08     RESET_SIZE
	reset_pos_command_cb   // 0x09     RESET_POS
};

static int32_t mq_cb(MQ_ENDPOINT_HANDLE_t mq_handle, MQ_MSG_S *msg,
					 void *mq_arg)
{
	UNUSED(mq_handle);

	CVI_LOGD("Message rx, target_id = %08x, len = %d, mq_arg = %p",
			 msg->target_id, msg->len, mq_arg);
	CVI_LOGD("Message arg1 = 0x%08x, arg2 = 0x%08x",
			 msg->arg1, msg->arg2);
	CVI_LOGD("Message seq_no = 0x%04x, time = %" PRIu64 "",
			 msg->seq_no, msg->crete_time);

	if (msg->len > ((int32_t)MQ_MSG_HEADER_LEN + 4)) {
		CVI_LOGD("Message payload [%02x %02x %02x %02x]",
				 msg->payload[0], msg->payload[1], msg->payload[2], msg->payload[3]);
	}

	int32_t command_id = msg->arg1;
	CVI_LOG_ASSERT((command_id > CMD_PLAYER_INVALID) && (command_id < CMD_PLAYER_MAX),
				   "Command id %d out of range", command_id);
	if (command_table[command_id] == NULL) {
		CVI_LOGE("Command id %d not handled", command_id);
		return PS_FAILURE;
	}

	return command_table[command_id](msg, mq_arg);
}

static void ps_event_task_entry(void *arg)
{
	PS_CONTEXT_HANDLE ps = (PS_CONTEXT_HANDLE)arg;
	PS_PARAM_HANDLE param = &ps->param;
	// create mq end point
	MQ_ENDPOINT_CONFIG_S mq_config = {};
	mq_config.name = "player_service_mq_task";
	mq_config.id = MQ_ID(PLAYER_SERVICE_CLIENT_ID,
						 PLAYER_SERVICE_CHANNEL_ID(param->chn_id));
	mq_config.recv_cb = mq_cb;
	mq_config.recv_cb_arg = (void *)ps;
	if (MQ_CreateEndpoint(&mq_config, &ps->mq_ep) != OSAL_SUCCESS) {
		CVI_LOGE("MQ_CreateEndpoint failed");
	}

	while (!ps->shutdown) {
		OSAL_TASK_Sleep(10000); // 10 ms
	}

	// cleanup mq
	MQ_DestroyEndpoint(ps->mq_ep);
}

int32_t ps_start_event_task(PS_CONTEXT_HANDLE ps)
{
	OSAL_TASK_ATTR_S task_attr;
	task_attr.name = "Player service event task";
	task_attr.entry = ps_event_task_entry;
	task_attr.param = (void *)ps;
	task_attr.priority = OSAL_TASK_PRI_NORMAL;
	task_attr.detached = false;
	task_attr.stack_size = 256 * 1024;
	if (OSAL_TASK_Create(&task_attr, &ps->event_task) != OSAL_SUCCESS) {
		CVI_LOGE("Player service event task create failed");
		return PS_FAILURE;
	}

	return PS_SUCCESS;
}

int32_t ps_stop_event_task(PS_CONTEXT_HANDLE ps)
{
	if (OSAL_TASK_Join(ps->event_task) != OSAL_SUCCESS) {
		CVI_LOGE("Playr service task join failed");
		return PS_FAILURE;
	}
	OSAL_TASK_Destroy(&ps->event_task);

	return PS_SUCCESS;
}

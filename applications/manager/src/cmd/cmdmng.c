#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osal.h"
#include "sysutils_mq.h"

int32_t CMDMNG_SendMqCmd(int32_t client_id, int32_t chn_id,
						 int32_t cmd_id, int32_t arg_val)
{
	static int32_t i = 0;

	// TODO: implement ACK

	// test MQ_SendRAW
	MQ_MSG_S msg;
	msg.target_id = MQ_ID(client_id, chn_id);
	msg.arg1 = cmd_id;
	msg.arg2 = arg_val;
	msg.needack = 0;
	msg.seq_no = i++;
	msg.len = (int32_t)MQ_MSG_HEADER_LEN;
	uint64_t boot_time;
	OSAL_TIME_GetBootTimeUs(&boot_time);
	msg.crete_time = boot_time;
	return MQ_SendRAW(&msg);
}

int32_t CMDMNG_SendMqCmd_Str(int32_t client_id, int32_t chn_id,
							 int32_t cmd_id, int32_t arg_val, const char *str)
{
	static int32_t i = 0;

	// TODO: implement ACK

	// test MQ_SendRAW
	MQ_MSG_S msg;
	msg.target_id = MQ_ID(client_id, chn_id);
	msg.arg1 = cmd_id;
	msg.arg2 = arg_val;
	strncpy(msg.payload, str, MQ_MSG_PAYLOAD_LEN);
	msg.seq_no = i++;
	msg.needack = 0;
	msg.len = (int32_t)MQ_MSG_HEADER_LEN + (int32_t)strnlen(msg.payload, MQ_MSG_PAYLOAD_LEN) + 1;
	uint64_t boot_time;
	OSAL_TIME_GetBootTimeUs(&boot_time);
	msg.crete_time = boot_time;
	return MQ_SendRAW(&msg);
}
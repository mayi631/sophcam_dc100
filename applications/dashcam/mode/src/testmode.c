#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "modetest.h"
#include "mapi.h"
#include "osal.h"
#include "mode.h"
#include "param.h"
#include "media_dump.h"
#include "media_init.h"
#include "modeinner.h"
#include "media_sensor_test.h"
#include "filemng.h"
// #include "tkc/mem.h"

typedef struct __mt_context {
	mt_param_t param;
	volatile uint32_t shutdown;
	volatile bool curwndmode;
	volatile bool newwndmode;
	pthread_mutex_t mt_mutex;
	OSAL_TASK_HANDLE_S mt_task;

	// event task
	OSAL_TASK_HANDLE_S event_task;
	MQ_ENDPOINT_HANDLE_t mq_ep;
} mt_context_t, *mt_context_handle_t;

typedef int32_t (*mt_cmd_cb_t)(MQ_MSG_S *msg, void *userdate);

typedef struct _mt_cmd_desc {
	mt_cmd_cb_t cb;
	uint32_t flags;
} mt_cmd_desc_t;

uint32_t get_sd_card_avaible(void)
{
	return MODEMNG_GetCardState();
}

static int32_t mt_cmd_cb_start_rec(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	// tk_mem_dump();
	// return 0;
	if (get_sd_card_avaible()) {
		MESSAGE_S Msg = {0};
		Msg.topic = EVENT_MODEMNG_START_REC;
		MODEMNG_SendMessage(&Msg);
	} else {
		CVI_LOGI("the card is not exist\n");
	}
	return 0;
}

static int32_t mt_cmd_cb_stop_rec(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	uint32_t u32ModeState = 0;
	MODEMNG_GetModeState(&u32ModeState);
	if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
		(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
		MESSAGE_S Msg = {0};
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		MODEMNG_SendMessage(&Msg);
	} else {
		CVI_LOGI("the record is not working\n");
	}

	return 0;
}

static int32_t mt_cmd_cb_start_event_rec(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);

	uint32_t u32ModeState = 0;
	MODEMNG_GetModeState(&u32ModeState);
	if (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC) {
		if (get_sd_card_avaible()) {
			MESSAGE_S Msg = {0};
			Msg.topic = EVENT_MODEMNG_START_EMRREC;
			MODEMNG_SendMessage(&Msg);
		} else {
			CVI_LOGI("the card is not exist\n");
		}
	}

	return 0;
}

static int32_t mt_cmd_cb_start_piv(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	if (get_sd_card_avaible()) {
		if (msg->arg2 < 101 && msg->arg2 > 0) {
			for (int32_t i = 0; i < msg->arg2; i++) {
				MESSAGE_S Msg = {0};
				Msg.topic = EVENT_MODEMNG_START_PIV;
				MODEMNG_SendMessage(&Msg);
				OSAL_TASK_Sleep(500 * 1000);
			}
		} else {
			printf("error start pri parm\n");
		}
	} else {
		CVI_LOGI("the card is not exist\n");
	}

	return 0;
}

#define MAX_PATH 255
static int32_t mt_cmd_cb_remove_piv(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	char filename[FILEMNG_PATH_MAX_LEN];

	uint32_t totalfile = FILEMNG_GetDirFileCnt(0, FILEMNG_DIR_PHOTO);

	if (get_sd_card_avaible()) {
		if (msg->arg2 < 100 || msg->arg2 > 0) {
			while (totalfile > 0) {
				FILEMNG_GetFileNameByFileInx(0, FILEMNG_DIR_PHOTO, totalfile - 1, &filename, 1);
				CVI_LOGE("delete cur filename = %s\n", filename);
				FILEMNG_DelFile(0, filename);
				totalfile--;
			}
		}
	} else {
		CVI_LOGI("the card is not exist\n");
	}

	return 0;
}

static void set_option_itemcnt2videosize(uint32_t *viedosize, int32_t itemcnt)
{
	if (itemcnt == 0) {
		*viedosize = MEDIA_VIDEO_SIZE_2560X1440P25;
	} else if (itemcnt == 1) {
		*viedosize = MEDIA_VIDEO_SIZE_1920X1080P25;
	} else {
		CVI_LOGE("media video size faile !\n");
	}
}

static int32_t mt_cmd_cb_set_media_size(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	uint32_t videosize = 0;
	if (msg->arg2 > 2 || msg->arg2 < 0) {
		return 0;
	}
	set_option_itemcnt2videosize(&videosize, msg->arg2);
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_VIDEO_SIZE;
	if (0 == msg->arg2) {
		Msg.arg2 = MEDIA_VIDEO_SIZE_2560X1440P25;
	} else {
		Msg.arg2 = MEDIA_VIDEO_SIZE_1920X1080P25;
	}
	MODEMNG_SendMessage(&Msg);

	// CVI_MODE_SetMediaVideoSize(videosize);
	return 0;
}

static int32_t mt_cmd_cb_set_media_audio(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	MESSAGE_S Msg = {0};

	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
	Msg.arg2 = msg->arg2;
	MODEMNG_SendMessage(&Msg);

	return 0;
}

static int32_t mt_cmd_cb_dump_data(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
#ifdef ENABLE_ISP_PQ_TOOL
	if (CARD_STATE_AVAILABLE == MODEMNG_GetCardState()) {
		MEDIA_DUMP_SetDumpRawAttr(msg->arg2);
		MEDIA_DUMP_DumpRaw(msg->arg2);
		MEDIA_DUMP_DumpYuv(msg->arg2);
		OSAL_FS_System("sync");
	}
#else
	CVI_LOGE("PQ Tool Disable!\n");
#endif
	return 0;
}

static int32_t mt_cmd_cb_set_media_venc(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	MODEMNG_SetMediaVencFormat(msg->arg2);
	return 0;
}

static int32_t dec_flag = 0;
static int32_t mt_cmd_cb_start_dec(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	if (get_sd_card_avaible() && (dec_flag == 0)) {
		MESSAGE_S Msg;
		Msg.topic = EVENT_MODEMNG_MODESWITCH;
		Msg.arg1 = WORK_MODE_PLAYBACK;
		MODEMNG_SendMessage(&Msg);
		dec_flag = 1;
	}
	return 0;
}

static int32_t mt_cmd_cb_stop_dec(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	if (get_sd_card_avaible() && (dec_flag == 1)) {
		MESSAGE_S Msg;
		Msg.topic = EVENT_MODEMNG_MODESWITCH;
		Msg.arg1 = WORK_MODE_MOVIE;
		MODEMNG_SendMessage(&Msg);
		dec_flag = 0;
	}
	return 0;
}

static int32_t mt_cmd_cb_switch_mode(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	if (get_sd_card_avaible()) {
		MESSAGE_S event;
		event.topic = EVENT_MODEMNG_MODESWITCH;
		event.arg1 = msg->arg2;
		MODEMNG_SendMessage(&event);
	}
	return 0;
}

static int32_t mt_cmd_cb_start_record(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	EVENT_S stEvent;
	stEvent.topic = EVENT_MODETEST_START_RECORD;
	EVENTHUB_Publish(&stEvent);
	return 0;
}

static int32_t mt_cmd_cb_stop_record(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	EVENT_S stEvent;
	stEvent.topic = EVENT_MODETEST_STOP_RECORD;
	EVENTHUB_Publish(&stEvent);
	return 0;
}

static int32_t mt_cmd_cb_play_record(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	EVENT_S stEvent;
	stEvent.topic = EVENT_MODETEST_PLAY_RECORD;
	stEvent.arg1 = msg->arg2;
	EVENTHUB_Publish(&stEvent);
	return 0;
}

static int32_t mt_cmd_cb_dump_sendvo(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	UNUSED(userdata);
	MAPI_DISP_SetDumpStatus(true);

	return 0;
}

static int32_t mt_cmd_cb_start_sensor_test(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	CVI_LOGD("msg->payload: %s, msg->arg1 = 0x%x\n", msg->payload, msg->arg1);
	sensor_test(msg->payload);
	return 0;
}

static int32_t mt_cmd_cb_update_ota(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	if (MODEMNG_GetCurWorkMode() == WORK_MODE_UPDATE) {
		if (get_sd_card_avaible()) {
			MESSAGE_S event;
			event.topic = EVENT_MODEMNG_START_UPFILE;
			event.arg1 = msg->arg2;
			char mode_version[] = "generic_0.1.8";
			memcpy(event.aszPayload, mode_version, strlen(mode_version) + 1);
			MODEMNG_SendMessage(&event);
		} else {
			CVI_LOGE("the card is not exist\n");
		}
	} else {
		CVI_LOGE("please first switchmode to updatemode\n");
	}

	return 0;
}

static int32_t mt_cmd_cb_adjust_focus(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	if (MODEMNG_GetCurWorkMode() == WORK_MODE_MOVIE || MODEMNG_GetCurWorkMode() == WORK_MODE_PHOTO) {
		MESSAGE_S event;
		event.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
		event.arg1 = msg->arg2;
		snprintf((char *)event.aszPayload, sizeof(msg->payload), "%s", msg->payload);
		MODEMNG_SendMessage(&event);
	} else {
		CVI_LOGE("please first switchmode to movie or photo mode\n");
	}

	return 0;
}

static mt_cmd_desc_t mt_cmd_tbl[] = {
	{mt_cmd_cb_start_rec, 0},		  /* 0x00    START REC */
	{mt_cmd_cb_stop_rec, 0},		  /* 0x01    STOP REC */
	{mt_cmd_cb_start_event_rec, 0},	  /* 0x02    EVNET REC */
	{mt_cmd_cb_start_piv, 0},		  /* 0x03    SET PIV */
	{mt_cmd_cb_set_media_size, 0},	  /* 0x04    MEDIA SIZE */
	{mt_cmd_cb_set_media_audio, 0},	  /* 0x05    SET AUDO */
	{mt_cmd_cb_dump_data, 0},		  /* 0x06    DWMP DATA */
	{mt_cmd_cb_set_media_venc, 0},	  /* 0x06    SET VENC */
	{mt_cmd_cb_start_dec, 0},		  /* 0x07    START DEC */
	{mt_cmd_cb_stop_dec, 0},		  /* 0x08    STOP DEC */
	{mt_cmd_cb_remove_piv, 0},		  /* 0x09    REMOVE PHOTO*/
	{mt_cmd_cb_switch_mode, 0},		  /* 0x0A    SWITCH MODE*/
	{mt_cmd_cb_start_record, 0},	  /* 0x0B    START RECORD*/
	{mt_cmd_cb_stop_record, 0},		  /* 0x0C    STop RECORD*/
	{mt_cmd_cb_play_record, 0},		  /* 0x0D    PLAY RECORD*/
	{mt_cmd_cb_dump_sendvo, 0},		  /* 0x0E    DUMP SENDVO*/
	{mt_cmd_cb_start_sensor_test, 0}, /* 0x0F    SENSOR TEST*/
	{mt_cmd_cb_update_ota, 0},		  /* 0x11    start OTA*/
	{mt_cmd_cb_adjust_focus, 0},	  /* 0x12    adjust focus*/
};

static int32_t mt_mq_cb(MQ_ENDPOINT_HANDLE_t ep, MQ_MSG_S *msg, void *ep_arg)
{
	UNUSED(ep);

#if 1
	printf("mt_mq_cb: rx, target_id = %08x, len = %d, ep_arg = %p\n", msg->target_id, msg->len, ep_arg);
	printf("mt_mq_cb:     arg1 = 0x%08x, arg2 = 0x%08x\n", msg->arg1, msg->arg2);
	printf("mt_mq_cb:     seq_no = 0x%04x, time = %" PRIu64 "\n", msg->seq_no, msg->crete_time);
	if (msg->len > (int32_t)MQ_MSG_HEADER_LEN + 4) {
		printf("mt_mq_cb:     payload [%02x %02x %02x %02x]\n", msg->payload[0], msg->payload[1],
			   msg->payload[2], msg->payload[3]);
	}
#endif
	int32_t cmd_id = msg->arg1;
	CVI_LOG_ASSERT(cmd_id >= 0 && cmd_id < (int32_t)(sizeof(mt_cmd_tbl) / sizeof(mt_cmd_desc_t)),
				   "cmd_id %d out of range\n", cmd_id);

	if (mt_cmd_tbl[cmd_id].cb == NULL) {
		CVI_LOGE("cmd_id %d not handled\n", cmd_id);
		return -1;
	}

	return mt_cmd_tbl[cmd_id].cb(msg, ep_arg);
}

static void mt_event_task_entry(void *arg)
{
	mt_context_handle_t mt = (mt_context_handle_t)arg;
	UNUSED(mt);

	// start mq
	MQ_ENDPOINT_CONFIG_S mq_config;
	mq_config.name = "mt_mq";
	mq_config.id = MQ_ID(CMD_CLIENT_ID_MT_TOOL, CMD_CHANNEL_ID_MT(0));
	mq_config.recv_cb = mt_mq_cb;
	mq_config.recv_cb_arg = (void *)mt;
	int32_t rc = MQ_CreateEndpoint(&mq_config, &mt->mq_ep);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("MQ_CreateEndpoint failed\n");
		exit(-1);
	}
	while (!mt->shutdown) {
		OSAL_TASK_Sleep(10000); // 10 ms
	}
	// cleanup mq
	MQ_DestroyEndpoint(mt->mq_ep);
}

static int32_t mt_start_event_task(mt_context_handle_t mt)
{
	OSAL_TASK_ATTR_S ta;
	ta.name = "mt_event";
	ta.entry = mt_event_task_entry;
	ta.param = (void *)mt;
	ta.priority = OSAL_TASK_PRI_NORMAL;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	/*create event_task parm*/
	int32_t rc = OSAL_TASK_Create(&ta, &mt->event_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("lv_event task create failed, %d\n", rc);
		return -1;
	}
	return 0;
}

static int32_t mt_stop_event_task(mt_context_handle_t mt)
{
	int32_t rc = OSAL_TASK_Join(mt->event_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("lv_event task join failed, %d\n", rc);
		return -1;
	}
	OSAL_TASK_Destroy(&mt->event_task);
	return 0;
}

int32_t MODE_Test_Create(MT_HANDLE_T *hdl, MT_SERVICE_PARAM_T *params)
{
	mt_context_handle_t mt = (mt_context_handle_t)calloc(sizeof(mt_context_t), 1);
	mt->param = *params;
	mt->curwndmode = true;

	pthread_mutex_init(&mt->mt_mutex, NULL);
	mt_start_event_task(mt);
	*hdl = (MT_HANDLE_T)mt;
	return 0;
}

int32_t MODE_Test_Destroy(MT_HANDLE_T hdl)
{
	mt_context_handle_t mt = (mt_context_handle_t)hdl;
	// send shutdown to self
	mt->shutdown = 1;
	int32_t s32Ret = 0;

	// wait for exit
	while (!mt->shutdown) {
		OSAL_TASK_Sleep(20000);
	}

	CVI_LOGI("MODE_Test Service destroy\n");
	pthread_mutex_destroy(&mt->mt_mutex);

	s32Ret = mt_stop_event_task(mt);
	MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "mt_stop_event_task");

	free(mt);
	return 0;
}

int32_t MODEMNG_TEST_MAIN_Create(void)
{
	MT_HANDLE_T *pmtHdl = &MEDIA_GetCtx()->SysServices.MtHdl;
	MT_SERVICE_PARAM_T *pmtParams = &MEDIA_GetCtx()->SysServices.MtParam;
	int32_t s32Ret = 0;
	s32Ret = MODE_Test_Create(pmtHdl, pmtParams);
	MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MODE_Test_Create");
	return 0;
}

int32_t MODEMNG_TEST_MAIN_Destroy(void)
{
	int32_t s32Ret = 0;
	s32Ret = MODE_Test_Destroy(MEDIA_GetCtx()->SysServices.MtHdl);
	MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "MODE_Test_Destroy");
	return 0;
}
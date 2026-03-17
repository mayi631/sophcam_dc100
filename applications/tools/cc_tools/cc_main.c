#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"
#include "sysutils_mq.h"

#ifndef CHECK_RET
#define CHECK_RET(express)                              \
	do {                                                \
		int32_t rc = express;                           \
		if (rc != 0) {                                  \
			printf("\nFailed at %s: %d  (rc:0x%#x!)\n", \
				   __FILE__, __LINE__, rc);             \
			return rc;                                  \
		}                                               \
	} while (0)
#endif

#define CMD_CLIENT_ID_CC_TOOL (MQ_CLIENT_ID_USER_0)
#define CMD_CHANNEL_ID_CC(cc_id) (0x00 + (cc_id))

typedef enum cmd_cc_e {
	CMD_CC_StartRec = 0,
	CMD_CC_StopRec,
	CMD_CC_StartEventRec,
	CMD_CC_StartPiv,
	CMD_CC_SetMediaVideoSize,
	CMD_CC_SetMediaAudio,
	CMD_CC_Dumpdata,
	CMD_CC_SetVencFormat,
	CMD_CC_StartDec,
	CMD_CC_StopDec,
	CMD_CC_RemovePiv,
	CMD_CC_SwitchMode,
	CMD_CC_StartRecord,
	CMD_CC_StopRecord,
	CMD_CC_PlayRecord,
	CMD_CC_DumpSendVo,
	CMD_CC_StartSensorTest,
	CMD_CC_UpdateOTA,
	CMD_CC_AdjustFocus,
	CMD_BUT
} cmd_cc_t;

typedef enum video_size_e {
	VIDEO_SIZE_1440P = 0,
	VIDEO_SIZE_1080P,
	VIDEO_SIZE_BUT
} video_size_t;

typedef enum switch_e {
	SWITCH_ON = 0,
	SWITCH_OFF,
	SWITCH_BUT
} switch_t;

typedef enum venctype_e {
	VENCTYPE_H264 = 0,
	VENCTYPE_H265,
	VENCTYPE_BUT
} venctype_t;

char p_cmd_buf[CMD_BUT][32] = {"startrec", "stoprec", "warnrec", "takephoto", "videosize", "audio", "dump", "vencformat", "startdec",
								   "stopdec", "removephoto", "switchmode", "startrecord", "stoprecord", "playrecord", "dumpsendvo", "sensortest", "updateota", "adjustfocus"};

char *p_vedio_size_buf[] = {"1440P", "1080P"};
char *p_audio_switch_buf[] = {"off", "on"};
char *p_vencformat_buf[] = {"264", "265"};

static int32_t CMDMNG_SendMqCmd(int32_t client_id, int32_t chn_id,
								int32_t cmd_id, int32_t arg_val,
								int32_t argc, char *argv[])
{

	static int32_t i = 0;
	char param[256] = {0};
	// TODO: implement ACK
	// test MQ_SendRAW
	MQ_MSG_S msg;
	msg.target_id = MQ_ID(client_id, chn_id);
	if (cmd_id == CMD_CC_StartSensorTest) {
		if (argc != 7) {
			printf("input param error!, argc = %d\n", argc);
		} else {
			for (int32_t j = 2; j < argc; j++) {
				printf("argv[j] = %s\n", argv[j]);
				strcat(param, argv[j]);
				strcat(param, "-");
			}
			param[strlen(param) - 1] = '\0';
		}
	} else if (cmd_id == CMD_CC_AdjustFocus) {
		snprintf(param, MQ_MSG_PAYLOAD_LEN, "%s", argv[3]);
	}
	snprintf(msg.payload, MQ_MSG_PAYLOAD_LEN, "%s", param);
	printf("msg.payload: %s\n", msg.payload);
	msg.arg1 = cmd_id;
	msg.arg2 = arg_val;
	msg.needack = 0;
	msg.seq_no = i++;
	msg.len = (int32_t)MQ_MSG_HEADER_LEN + strlen(param);
	uint64_t boot_time;
	OSAL_TIME_GetBootTimeUs(&boot_time);
	msg.crete_time = boot_time;
	return MQ_SendRAW(&msg);
}

void usage(int32_t argc, char *argv[])
{

	UNUSED(argc);
	printf("   |-------------------------------------------------|\n");
	// TODO: print in auto gen
	printf("   |-----you should input :%s CmdStr---------|\n", argv[0]);
	printf("   |-----the CmdStr could select below---------------|\n");
	for (int32_t i = 0; i < CMD_BUT; i++) {
		printf("          |-----  %d.CmdStr : %s ----|\n", i, p_cmd_buf[i]);
	}
}

void usage_extra_cmd(int32_t argc, char *argv[], int32_t cmd_id)
{
	printf("   |-------------------------------------------------|\n");
	printf("   |------you should input complet cmd: %s %s option--------------|\n", argv[0], argv[1]);
	switch (cmd_id) {
	case CMD_CC_SetMediaVideoSize:
		printf("   |------the videosize option should select the string: 1600P, 1440P, 1080P -------|\n");
		break;
	case CMD_CC_SetMediaAudio:
		printf("   |------set audio option should select: on or off-------|\n");
		break;
	case CMD_CC_SetVencFormat:
		printf("   |------set vencformat option should select: 264 or 265-------|\n");
		break;
	case CMD_CC_StartPiv:
		printf("   |------set takephoto option should select: num(eg. 1 - 100)-------|\n");
		break;
	case CMD_CC_RemovePiv:
		printf("   |------set removephoto option should select: num(eg. 1 - 100)-------|\n");
		break;
	case CMD_CC_SwitchMode:
		printf("   |------set switch mode option should select: num(eg. 0:MOVIE 1:PHOTO 2:PLAYBACK 5:UPDATE)-------|\n");
		break;
	case CMD_CC_PlayRecord:
		printf("   |------set play record again option should select: num(eg. 0 - 100)-------|\n");
		break;
	case CMD_CC_StartSensorTest:
		printf("   |------set start sensor test  option should input five prams -------|\n");
		break;
	case CMD_CC_UpdateOTA:
		printf("   |------set ota option should select: num(eg. 0:via flash 1:via SD)-------|\n");
		break;
	case CMD_CC_Dumpdata:
		printf("   |------set ota option should select: num(eg. 0:cam 0 1:cam 1)-------|\n");
		break;
	case CMD_CC_AdjustFocus:
		printf("   |------adjust focus should select: window ratio(eg. Parm1(window): 0 or 1  Parm2(ratio) :1~4(support float) )-------|\n");
		break;
	default:
		printf("   |------please input corret parm-------|\n");
		break;
	}
	printf("   |-------------------------------------------------|\n");
}

int32_t get_cmd_arg(int32_t argc, char *argv[], char **buf, int32_t buf_size, uint32_t *arg, int32_t cmd_id)
{

	for (int32_t i = 0; i < buf_size; i++) {
		if (!strcmp(argv[2], buf[i])) {
			*arg = i;
			break;
		}
		if (buf_size == (i + 1)) {
			usage_extra_cmd(argc, argv, cmd_id);
			exit(-1);
		}
	}

	return 0;
}

int32_t set_photo_deal_arrange(int32_t argc, char *argv[], uint32_t *num, int32_t cmd_id)
{
	int32_t ret = atoi(argv[2]);
	if (ret < 0 || ret > 1000) {
		usage_extra_cmd(argc, argv, cmd_id);
	}
	*num = ret;

	return 0;
}

int32_t switch_mode_arrange(int32_t argc, char *argv[], uint32_t *num, int32_t cmd_id)
{
	int32_t ret = atoi(argv[2]);
	if (ret < 0 || ret > 5) {
		usage_extra_cmd(argc, argv, cmd_id);
	}
	*num = ret;

	return 0;
}

int32_t switch_ota_mode_arrange(int32_t argc, char *argv[], uint32_t *num, int32_t cmd_id)
{
	int32_t ret = atoi(argv[2]);
	if (ret < 0 || ret > 1) {
		usage_extra_cmd(argc, argv, cmd_id);
	}
	*num = ret;

	return 0;
}

int32_t start_record_deal_arrange(int32_t argc, char *argv[], uint32_t *num, int32_t cmd_id)
{
	printf("CMD_CC_StartRecord\n");
	int32_t ret = atoi(argv[2]);
	if (ret < 0 || ret > 300) {
		usage_extra_cmd(argc, argv, cmd_id);
	}
	*num = ret;

	return 0;
}

int32_t play_record_deal_arrange(int32_t argc, char *argv[], uint32_t *num, int32_t cmd_id)
{
	printf("CMD_CC_PlayRecord\n");
	int32_t ret = atoi(argv[2]);
	if (ret < 0) {
		usage_extra_cmd(argc, argv, cmd_id);
	}
	*num = ret;

	return 0;
}

int32_t Adjust_focus_arrange(int32_t argc, char *argv[], uint32_t *num, int32_t cmd_id)
{
	int32_t ret = atoi(argv[2]);
	float ratio = atof(argv[3]);
	if ((ret < 0 || ret > 2) && (ratio < 1 || ratio > 4)) {
		usage_extra_cmd(argc, argv, cmd_id);
	}
	*num = ret;

	return 0;
}

int32_t main(int32_t argc, char **argv)
{
	int32_t rc = 0;
	uint32_t i = 0;
	uint32_t arg_val = 0;
	uint32_t cmd_id = 0;

	printf("argc = %d, argv[1] = %s, CMD_BUT = %d\n", argc, argv[1], CMD_BUT);
	if ((argc != 3) && (argc != 4) && (argc != 2) && (argc != 5) && (argc != 7)) {
		usage(argc, argv);
		exit(-1);
	}

	for (i = 0; i < CMD_BUT; i++) {
		if (!strcmp(argv[1], p_cmd_buf[i])) {
			cmd_id = i;
			printf("cmd_id = %d\n", cmd_id);
			printf("argc = %d, argv[0] = %s, argv[2] = %s\n", argc, argv[0], argv[2]);
			/*deal with the audio and media size need three argc*/
			if (cmd_id == CMD_CC_SetMediaVideoSize || cmd_id == CMD_CC_SetMediaAudio ||
				cmd_id == CMD_CC_SetVencFormat || cmd_id == CMD_CC_StartPiv || cmd_id == CMD_CC_RemovePiv ||
				cmd_id == CMD_CC_SwitchMode || cmd_id == CMD_CC_PlayRecord || cmd_id == CMD_CC_StartSensorTest || cmd_id == CMD_CC_UpdateOTA || cmd_id == CMD_CC_AdjustFocus) {
				if ((argc != 3) && (argc != 7) && (argc != 4)) {
					usage_extra_cmd(argc, argv, cmd_id);
					exit(-1);
				} else {
					switch (cmd_id) {
					case CMD_CC_SetMediaVideoSize:
						get_cmd_arg(argc, argv, p_vedio_size_buf, VIDEO_SIZE_BUT, &arg_val, cmd_id);
						break;
					case CMD_CC_SetMediaAudio:
						get_cmd_arg(argc, argv, p_audio_switch_buf, SWITCH_BUT, &arg_val, cmd_id);
						break;
					case CMD_CC_SetVencFormat:
						get_cmd_arg(argc, argv, p_vencformat_buf, VENCTYPE_BUT, &arg_val, cmd_id);
						break;
					case CMD_CC_StartPiv:
						set_photo_deal_arrange(argc, argv, &arg_val, cmd_id);
						break;
					case CMD_CC_RemovePiv:
						set_photo_deal_arrange(argc, argv, &arg_val, cmd_id);
						break;
					case CMD_CC_SwitchMode:
						switch_mode_arrange(argc, argv, &arg_val, cmd_id);
						break;
					case CMD_CC_PlayRecord:
						play_record_deal_arrange(argc, argv, &arg_val, cmd_id);
						break;
					case CMD_CC_UpdateOTA:
						switch_ota_mode_arrange(argc, argv, &arg_val, cmd_id);
						break;
					case CMD_CC_Dumpdata:
						switch_ota_mode_arrange(argc, argv, &arg_val, cmd_id);
						break;
					case CMD_CC_AdjustFocus:
						Adjust_focus_arrange(argc, argv, &arg_val, cmd_id);
						break;
					default:
						break;
					}
				}
			}
			break;
		}

		// if (CMD_BUT == (i + 1)) {
		//     usage(argc, argv);
		//     exit(-1);
		// }
	}

	int32_t client_id = CMD_CLIENT_ID_CC_TOOL;
	int32_t chn_id = CMD_CHANNEL_ID_CC(0);
	rc = CMDMNG_SendMqCmd(client_id, chn_id, cmd_id, arg_val, argc, argv);
	return rc;
}

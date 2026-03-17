#include "ui_windowmng.h"

static widget_t *sd_image = NULL;

static ret_t ui_home_SetSdCardStatus(const idle_info_t *idle)
{
	(void)idle;
	if (sd_image == NULL) {
		CVI_LOGW("SD widget is null \n");
		return RET_OK;
	}

	switch (MODEMNG_GetCardState()) {
	case CARD_STATE_AVAILABLE:
		widget_set_visible(sd_image, TRUE, FALSE);
		break;
	default:
		widget_set_visible(sd_image, FALSE, FALSE);
		break;
	}

	return RET_OK;
}

static void Set_SdCard_Image(void)
{
	idle_queue(ui_home_SetSdCardStatus, NULL);
	return;
}

static bool ui_home_cardstatus(void)
{
	uint32_t type = 0x0;

	if (ui_winmng_getwinisshow(UI_HOME_PAGE) == false) {
		CVI_LOGW("ui_home_page no open !\n");
		return false;
	}

	switch (MODEMNG_GetCardState()) {
	case CARD_STATE_REMOVE:
		type = MSG_EVENT_ID_NO_CARD;
		break;
	case CARD_STATE_AVAILABLE:
		ui_winmng_finishwin(UI_WRNMSG_PAGE);
		Set_SdCard_Image();
		return true;
		break;
	case CARD_STATE_ERROR:
		type = MSG_EVENT_ID_SDCARD_ERROR;
		break;
	case CARD_STATE_FSERROR:
	case CARD_STATE_UNAVAILABLE:
		type = MSG_EVENT_ID_SDCARD_NEED_FORMAT;
		break;
	case CARD_STATE_SLOW:
		type = MSG_EVENT_ID_SDCARD_SLOW;
		break;
	case CARD_STATE_CHECKING:
		type = MSG_EVENT_ID_SDCARD_CHECKING;
		break;
	case CARD_STATE_READ_ONLY:
		type = MSG_EVENT_ID_SDCARD_READ_ONLY;
		break;
	case CARD_STATE_MOUNT_FAILED:
		type = MSG_EVENT_ID_SDCARD_MOUNT_FAILED;
		break;
	default:
		type = MSG_EVENT_ID_INVALID;
		CVI_LOGE("value is invalid\n");
		break;
	}
	Set_SdCard_Image();

	ui_wrnmsg_update_type(type);
	ui_winmng_startwin(UI_WRNMSG_PAGE, false);

	return false;
}

static int32_t UI_Home_OnReceiveMsgResult(EVENT_S *pstEvent)
{
	// int32_t  s32Ret = 0;
	(void)pstEvent;
	return 0;
}

ret_t on_systime_update(const timer_info_t *timer)
{
	static uint8_t u8status = 0;
	uint32_t u32ModeState = 0;
	widget_t *win = WIDGET(timer->ctx);
	widget_t *video_image = widget_lookup(win, "recstate_image", TRUE);
	widget_t *sos_image = widget_lookup(win, "sosstate_image", TRUE);
	MODEMNG_GetModeState(&u32ModeState);

	if (u32ModeState == MEDIA_MOVIE_STATE_VIEW) {
		widget_set_visible(video_image, FALSE, FALSE);
		widget_set_visible(sos_image, FALSE, FALSE);
		return RET_REPEAT;
	} else if (u32ModeState == MEDIA_MOVIE_STATE_REC ||
			   u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC) {
		if (MODEMNG_GetEmrState() == false) {
			widget_set_visible(sos_image, FALSE, FALSE);
		} else {
			widget_set_visible(sos_image, TRUE, FALSE);
		}
	}
	widget_set_visible(video_image, u8status, FALSE);
	u8status = 1 - u8status;

	return RET_REPEAT;
}

ret_t on_digit_clock_set(widget_t *digit_clock)
{
	ret_t ret = 0;
	static uint8_t foramt_id;
	bool_t time_format = false;

	time_format = ui_check_time_format_type();
	foramt_id = ui_settime_get_date_foramtid();
	CVI_LOGD("on_digit_clock_set foramt_id is %d", foramt_id);

	if (foramt_id == DATE_YY_MM_DD) {
		if (time_format)
			ret = digit_clock_set_format(digit_clock, "Y-MM-DD hh:mm:ss A");
		else
			ret = digit_clock_set_format(digit_clock, "Y-MM-DD hh:mm:ss");
	} else if (foramt_id == DATE_MM_DD_YY) {
		if (time_format)
			ret = digit_clock_set_format(digit_clock, "MM-DD-Y hh:mm:ss A");
		else
			ret = digit_clock_set_format(digit_clock, "MM-DD-Y hh:mm:ss");
	} else if (foramt_id == DATE_DD_MM_YY) {
		if (time_format)
			ret = digit_clock_set_format(digit_clock, "DD-MM-Y hh:mm:ss A");
		else
			ret = digit_clock_set_format(digit_clock, "DD-MM-Y hh:mm:ss");
	}
	if (ret != RET_OK) {
		CVI_LOGD("digit_clock_format_time failed \n");
		return ret;
	}

	return ret;
}

static ret_t on_home_page_low_battery(void *ctx, event_t *e)
{
	CVI_LOGD("on_home_page_low_battery \n");
	return RET_OK;
}

static ret_t on_photo_click(void)
{
	if (ui_home_cardstatus()) {
#ifndef ENABLE_ISP_PQ_TOOL
		MESSAGE_S Msg = {0};
		Msg.topic = EVENT_MODEMNG_START_PIV;
		MODEMNG_SendMessage(&Msg);
#else
		MEDIA_DUMP_SetDumpRawAttr(0);
		MEDIA_DUMP_DumpRaw(0);
		MEDIA_DUMP_DumpYuv(0);
		OSAL_FS_System("sync");
#endif
	}
	return RET_OK;
}

static void Set_LoopTime_Image(widget_t *widget_main)
{
	widget_t *widget_loop = widget_lookup(widget_main, "cyclic_rec_image", TRUE);

	char *loop_image_buf[] = {"ICON_CYCLIC_REC_1MIN_M", "ICON_CYCLIC_REC_3MIN_M", "ICON_CYCLIC_REC_5MIN_M"};
	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	image_base_set_image(widget_loop, loop_image_buf[param.Menu.VideoLoop.Current]);
}

static void Set_Res_Label(widget_t *widget_main)
{
	char *res_image_buf[] = {"STRID_1440P", "STRID_1080P", "STRID_720P"};

	widget_t *widget_reslabel = widget_lookup(widget_main, "resolution_label", TRUE);

	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	CVI_LOGD(" Set_Res_Label\n");
	widget_set_tr_text(widget_reslabel, res_image_buf[param.Menu.VideoSize.Current]);

	return;
}

static void Set_FatigueDrive_Image(widget_t *widget_main)
{
	widget_t *widget_image = widget_lookup(widget_main, "fatiguedrive_image", TRUE);
	char *res_image_buf[] = {"ICON_FATIGUE_M",
							 "ICON_FATIGUE_1H_M",
							 "ICON_FATIGUE_2H_M",
							 "ICON_FATIGUE_3H_M",
							 "ICON_FATIGUE_4H_M"};
	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	image_base_set_image(widget_image, res_image_buf[param.Menu.FatigueDirve.Current]);

	return;
}

static void Set_Cyclic_Rec_Image(widget_t *widget_main)
{
	widget_t *widget_image = widget_lookup(widget_main, "cyclic_rec_image", TRUE);
	char *res_image_buf[] = {"ICON_CYCLIC_REC_1MIN_M",
							 "ICON_CYCLIC_REC_3MIN_M",
							 "ICON_CYCLIC_REC_5MIN_M"};
	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	image_base_set_image(widget_image, res_image_buf[param.Menu.VideoLoop.Current]);

	return;
}

static void Set_BTM_Audio_Image(widget_t *widget_main)
{
	widget_t *widget_image = widget_lookup(widget_main, "audio_image", TRUE);
	char *res_image_buf[] = {"ICON_AUDIO_REC_OFF_M", "ICON_AUDIO_REC_ON_M"};

	PARAM_MENU_S menu_param;
	PARAM_GetMenuParam(&menu_param);
	image_base_set_image(widget_image, res_image_buf[menu_param.AudioEnable.Current]);

	return;
}

static void Set_GPS_Image(widget_t *widget_main)
{
	widget_t *widget_image = widget_lookup(widget_main, "gps_image", TRUE);
	char *res_image_buf[] = {"ICON_GPS_NS",
							 "ICON_GPS_OK",
							 "ICON_GPS_SERCH"};
	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	image_base_set_image(widget_image, res_image_buf[param.Menu.GPSStamp.Current]);

	return;
}

static void Set_Gsensor_Image(widget_t *widget_main)
{
	widget_t *widget_image = widget_lookup(widget_main, "gsensor_image", TRUE);
	char *res_image_buf[] = {"ICON_GSENSOR_C_M",
							 "ICON_GSENSOR_L_M",
							 "ICON_GSENSOR_M_M",
							 "ICON_GSENSOR_H_M"};
	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	CVI_LOGD("param.DevMng.Gsensor.enSensitity is %d\n", param.DevMng.Gsensor.enSensitity);
	image_base_set_image(widget_image, res_image_buf[param.DevMng.Gsensor.enSensitity]);

	return;
}

static void set_home_page_icons(widget_t *ctx)
{
	widget_t *widget_main = WIDGET(ctx);
	Set_LoopTime_Image(widget_main);
	Set_Res_Label(widget_main);
	Set_FatigueDrive_Image(widget_main);
	Set_Cyclic_Rec_Image(widget_main);
	Set_BTM_Audio_Image(widget_main);
	Set_GPS_Image(widget_main);
	Set_Gsensor_Image(widget_main);
}

static int32_t ui_setscreen_backlight(bool state)
{
	int32_t s32Ret = 0;

	MESSAGE_S stMsg = {};
	stMsg.topic = EVENT_MODEMNG_SCREEN_DORMANT;
	stMsg.arg1 = state;
	s32Ret = MODEMNG_SendMessage(&stMsg);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	return 0;
}

static ret_t init_widget(void *ctx, const void *iter)
{
	widget_t *widget = WIDGET(iter);
	// widget_t* win = widget_get_window(widget);
	(void)ctx;

	if (widget->name != NULL) {
		const char *name = widget->name;
		if (tk_str_eq(name, "recstate_image")) {
			widget_set_visible(widget, FALSE, FALSE);
		} else if (tk_str_eq(name, "sosstate_image")) {
			widget_set_visible(widget, FALSE, FALSE);
		}
	}

	return RET_OK;
}

static void init_children_widget(widget_t *widget)
{
	widget_foreach(widget, init_widget, widget);
}

static ret_t on_home_page_key_down(void *ctx, event_t *e)
{
	key_event_t *evt = (key_event_t *)e;
	uint32_t u32ModeState = 0;
	MESSAGE_S Msg = {0};
	HAL_SCREEN_STATE_E ScreenState = HAL_SCREEN_STATE_OFF;
	HAL_SCREEN_GetBackLightState(HAL_SCREEN_IDXS_0, &ScreenState);

	if (ui_winmng_getwinisshow(UI_WRNMSG_PAGE) == true && evt->key != UI_KEY_POWER) {
		ui_winmng_finishwin(UI_WRNMSG_PAGE);
		return 0;
	}

	if (ScreenState == HAL_SCREEN_STATE_OFF) {
		ui_setscreen_backlight(false);
		return 0;
	}
	if (evt->key == UI_KEY_POWER) {
		ui_setscreen_backlight(true);
	} else if (evt->key == UI_KEY_MENU) {
		MODEMNG_GetModeState(&u32ModeState);
		if (evt->alt == false) {
			CVI_LOGE("u32ModeState==============%d\n", u32ModeState);
			if (u32ModeState == MEDIA_MOVIE_STATE_REC) {
				Msg.topic = EVENT_MODEMNG_STOP_REC;
				Msg.arg1 = MEDIA_MOVIE_STATE_MENU;
				MODEMNG_SendMessage(&Msg);
			} else {
				MODEMNG_SetModeState(MEDIA_MOVIE_STATE_MENU);
			}
			ui_winmng_startwin(UI_SET_PAGE, true);
			return RET_STOP;
		} else {
			if (ui_home_cardstatus() && (u32ModeState != MEDIA_MOVIE_STATE_REC)) {
				Msg.topic = EVENT_MODEMNG_MODESWITCH;
				Msg.arg1 = WORK_MODE_PLAYBACK;
				MODEMNG_SendMessage(&Msg);
				return RET_STOP;
			}
		}
	} else if (evt->key == UI_KEY_UP) {
		if (ui_home_cardstatus() && MODEMNG_GetEmrState() == false) {
			Msg.topic = EVENT_MODEMNG_START_EMRREC;
			MODEMNG_SendMessage(&Msg);
		}
		return RET_STOP;
	} else if (evt->key == UI_KEY_DOWN) {
		if (evt->alt == false) {
			MODEMNG_GetModeState(&u32ModeState);
			if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
				(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
				Msg.topic = EVENT_MODEMNG_STOP_REC;
				MODEMNG_SendMessage(&Msg);
			} else {
				if (ui_home_cardstatus() == true) {
					Msg.topic = EVENT_MODEMNG_START_REC;
					MODEMNG_SendMessage(&Msg);
				}
				// else {}   //TODO, show sd card message
			}
		} else {
			on_photo_click();
		}
		return RET_STOP;
	}
	return RET_OK;
}

int32_t ui_homepage_eventcb(void *argv, EVENT_S *msg)
{
	MESSAGE_S Msg = {0};
	uint32_t u32ModeState = 0;
	MODEMNG_GetModeState(&u32ModeState);
	CVI_LOGD("u32ModeState == %d\n", u32ModeState);

	switch (msg->topic) {
	case EVENT_MODEMNG_MODEOPEN: {
		ui_winmng_startwin(UI_HOME_PAGE, false);
		break;
	}
	case EVENT_MODEMNG_MODECLOSE: {
		ui_winmng_closeallwin();
		break;
	}
	case EVENT_MODEMNG_CARD_FSERROR:
	case EVENT_MODEMNG_CARD_SLOW:
	case EVENT_MODEMNG_CARD_CHECKING:
	case EVENT_MODEMNG_CARD_UNAVAILABLE:
	case EVENT_MODEMNG_CARD_ERROR:
	case EVENT_MODEMNG_CARD_READ_ONLY:
	case EVENT_MODEMNG_CARD_MOUNT_FAILED:
	case EVENT_MODEMNG_CARD_REMOVE: {
		uint32_t type = ui_wrnmsg_get_type();
		if ((u32ModeState != MEDIA_MOVIE_STATE_MENU) ||
			(type == MSG_EVENT_ID_FORMAT_PROCESS)) {
			ui_home_cardstatus();
		}
		break;
	}
	case EVENT_MODEMNG_CARD_AVAILABLE:
	case EVENT_MODEMNG_RESET: {
		if (u32ModeState != MEDIA_MOVIE_STATE_MENU && ui_home_cardstatus() == true) {
			Msg.topic = EVENT_MODEMNG_START_REC;
			MODEMNG_SendMessage(&Msg);
		}
		break;
	}
	case EVENT_GSENSORMNG_COLLISION: {
		CVI_LOGD("EVENT_GSENSORMNG_COLLISION received! \n");
		if ((u32ModeState != MEDIA_MOVIE_STATE_MENU) &&
			ui_home_cardstatus() && MODEMNG_GetEmrState() == false) {
			Msg.topic = EVENT_MODEMNG_START_EMRREC;
			MODEMNG_SendMessage(&Msg);
		}
		break;
	}
	case EVENT_MODEMNG_RECODER_STARTPIVSTAUE: {
		if (msg->arg1 == 0) {
			VOICEPLAY_VOICE_S stVoice =
				{
					.au32VoiceIdx = {UI_VOICE_PHOTO_IDX},
					.u32VoiceCnt = 1,
					.bDroppable = false,
				};
			VOICEPLAY_Push(&stVoice, 0);
		}
		break;
	}
	case EVENT_MODEMNG_RECODER_STARTSTATU: {
		if (msg->arg1 == 0) {
			VOICEPLAY_VOICE_S stVoice =
				{
					.au32VoiceIdx = {UI_VOICE_REC_IDX},
					.u32VoiceCnt = 1,
					.bDroppable = false,
				};
			VOICEPLAY_Push(&stVoice, 0);
		}
		break;
	}
	case EVENT_MODEMNG_RECODER_STOPSTATU:
		CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
		break;
	case EVENT_MODEMNG_RECODER_SPLITREC:
		CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
		break;
	case EVENT_MODEMNG_RECODER_STARTEVENTSTAUE:
		CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
		break;
	case EVENT_MODEMNG_RECODER_STOPEVENTSTAUE: {
		CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
		break;
	}
	case EVENT_NETCTRL_APPCONNECT_SUCCESS: {
		if (u32ModeState == MEDIA_MOVIE_STATE_MENU) {
			ui_winmng_startwin(UI_HOME_PAGE, TRUE);
		}
		return 0;
	}
	case EVENT_MODEMNG_CARD_FORMAT: {
		ui_winmng_finishwin(UI_WRNMSG_PAGE);
		break;
	}
	case EVENT_NETCTRL_UIUPDATE: {
		widget_t *topwin = window_manager_get_top_window(window_manager());
		CVI_LOGD("ui set update===>>>>>>>> top window is %s \n", topwin->name);
		set_home_page_icons(topwin);
		break;
	}
	default:
		break;
	}
	return 0;
}

ret_t ui_home_page_open(void *ctx, event_t *e)
{
	(void)e;
	widget_t *win = WIDGET(ctx);
	widget_t *digit_clock = NULL;

	if (win) {
		widget_add_timer(win, on_systime_update, 1000);
		digit_clock = widget_lookup(win, "digit_clock", TRUE);
		// 数字时钟设置
		on_digit_clock_set(digit_clock);
		widget_on(window_manager(), EVT_LOW_BATTERY, on_home_page_low_battery, win);
		widget_on(window_manager(), EVT_UI_KEY_DOWN, on_home_page_key_down, win);
		init_children_widget(win);
		// 主界面图标设置
		set_home_page_icons(win);
		sd_image = widget_lookup(win, "sdcard_image", TRUE);
		if (ui_home_cardstatus() == true) {
			MESSAGE_S Msg = {0};
			Msg.topic = EVENT_MODEMNG_START_REC;
			MODEMNG_SendMessage(&Msg);
		} else {
			MODEMNG_SetModeState(MEDIA_MOVIE_STATE_VIEW);
		}
	}
	return RET_OK;
}

ret_t ui_home_page_close(void *ctx, event_t *e)
{
	(void)e;
	widget_t *win = WIDGET(ctx);
	if (win) {
		sd_image = NULL;
	}
	return RET_OK;
}

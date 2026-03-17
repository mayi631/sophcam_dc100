#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "ui_common.h"
#include "filemng.h"
#include "media_init.h"
#include "mode.h"
#ifdef SERVICES_PLAYER_ON
#include "playbackmng.h"
#include "player_service.h"
#endif
#include "ui_windowmng.h"

static int32_t s_cur_camid = 0;
static int32_t s_cur_dirid = 0;
static int32_t s_curfileinx = 0;
uint32_t s_totaltime = 0;


enum _MSG_WARN_ID {
	MSG_WARN_ID_Normal = 0,
	MSG_WARN_ID_Delete_File,
	MSG_WARN_ID_Buttom,
};

ret_t back_close_window(void)
{
#ifdef SERVICES_PLAYER_ON
	PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
	PLAYER_SERVICE_Stop(ps_handle);
	set_current_directory(s_cur_camid, s_cur_dirid);
	ui_winmng_finishwin(UI_PLAYBACK_PAGE);
	ui_winmng_startwin(UI_FILELIST_PAGE, false);
#endif
	return 0;
}

static bool playback_prompt_wrnmsg(int32_t warnmsg)
{
    uint32_t totalfile = FILEMNG_GetDirFileCnt(s_cur_camid, s_cur_dirid);

	if (warnmsg == MSG_WARN_ID_Delete_File) {
		if (totalfile == 0) {
			back_close_window();
			return true;
		} else {
			ui_wrnmsg_update_type(MSG_EVENT_ID_DELETE_FILE_CONFIRM);
			ui_winmng_startwin(UI_WRNMSG_PAGE, false);
			return true;
		}
	} else {
		if (totalfile == 0) {
			if (s_cur_dirid != FILEMNG_DIR_PHOTO) {
				ui_wrnmsg_update_type(MSG_EVENT_ID_NOVIDEO_FILE);
				ui_winmng_startwin(UI_WRNMSG_PAGE, false);
			} else {
				ui_wrnmsg_update_type(MSG_EVENT_ID_DELETE_FILE_CONFIRM);
				ui_winmng_startwin(UI_WRNMSG_PAGE, false);
			}
			return true;
		}
	}
	return false;
}

static void playback_update_time()
{
#ifdef SERVICES_PLAYER_ON
    widget_t* win = window_manager_get_top_window(window_manager());
    widget_t* time = widget_lookup(win, "time", TRUE);
    widget_t* slider = widget_lookup(win, "slider", TRUE);
    uint32_t curmin, cursec = 0;
    uint32_t _totaltime = s_totaltime;
    uint32_t min = _totaltime/60;
    uint32_t sec;
    static double last_time = 0;
    char totaltime[64] = {};

    if (min != 0) {
        sec = _totaltime - min*60;
    } else {
        sec = _totaltime;
    }

    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
    PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    PLAYER_PLAY_INFO info;
    if (PLAYER_SERVICE_GetPlayInfo(ps_handle, &info) != 0) {
        CVI_LOGE("Player get play info failed");
    }

    if (0.0 == info.duration_sec) {
        snprintf(totaltime, sizeof(totaltime), "%02d:%02d/%02d:%02d", min, sec, 0, 0);
        widget_set_text_utf8(time, totaltime);
        slider_set_max(slider, _totaltime);
        slider_set_value(slider, 0);

        return;
    } else if (((_totaltime-info.duration_sec) < 1.0) && ((info.duration_sec-last_time) == 0)) {
        curmin = _totaltime/60;
        if (curmin != 0) {
            cursec = _totaltime - curmin*60;
        } else {
            cursec = _totaltime;
        }
    } else {
        curmin = info.duration_sec/60;
        if (curmin != 0) {
            cursec = info.duration_sec - curmin*60;
        } else {
            cursec = info.duration_sec;
        }
    }
    last_time = info.duration_sec;

    snprintf(totaltime, sizeof(totaltime), "%02d:%02d/%02d:%02d", min, sec, curmin, cursec);
    widget_set_text_utf8(time, totaltime);
    slider_set_max(slider, _totaltime);
    slider_set_value(slider, info.duration_sec);
#endif
}

bool check_jpg_file_complete(const char *filename, uint32_t file_type)
{
	if (file_type == FILEMNG_DIR_PHOTO) {
		FILE *fp = fopen(filename, "rb");
		if (NULL == fp) {
			CVI_LOGF("fail to open\n");
			return false;
		}
		unsigned char read_temp_buf[8] = {0};
		uint32_t seek_loc = 0;
		fseek(fp, 0, SEEK_SET);
		fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
		seek_loc = (((read_temp_buf[4] << 8) | read_temp_buf[5]) + 4);
		fseek(fp, seek_loc, SEEK_SET);
		fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
		seek_loc = (read_temp_buf[4] << 24 | read_temp_buf[5] << 16 | read_temp_buf[6] << 8 | read_temp_buf[7] << 0) + seek_loc + 4;
		fseek(fp, seek_loc, SEEK_SET);
		fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
		if ((read_temp_buf[0] == 0xFF) && (read_temp_buf[1] == 0xD9) && (read_temp_buf[2] == 0xFF) && (read_temp_buf[3] == 0xD9)) {
			fclose(fp);
			return true;
		} else {
			fclose(fp);
			return false;
		}
	}
	return true;
}

static int32_t play_file()
{
	int32_t s32Ret = 0;
#ifdef SERVICES_PLAYER_ON
	char filename[FILEMNG_PATH_MAX_LEN];
	MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
	PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
	PLAYER_MEDIA_INFO_S info = {};
    s32Ret = FILEMNG_GetFileNameByFileInx(s_cur_camid, s_cur_dirid, s_curfileinx, &filename, 1);
	char filepath[FILEMNG_PATH_MAX_LEN];
    FILEMNG_GetFilePath(s_cur_camid, s_cur_dirid, filepath);
	int  file_length = FILEMNG_PATH_MAX_LEN *2;
    char fullfilepath[file_length];
    sprintf(fullfilepath, "%s/%s", filepath, filename);
	if (s32Ret != 0) {
		CVI_LOGE("error FILEMNG_GetFileByIndex\n");
		return s32Ret;
	}
	CVI_LOGI("fullfilepath = %s\n", fullfilepath);
	widget_t *win = window_manager_get_top_main_window(window_manager());
	widget_t *filenametitle = widget_lookup(win, "filename", TRUE);
	widget_t *filetime = widget_lookup(win, "time", TRUE);
	widget_t *slider = widget_lookup(win, "slider", TRUE);

	if (s_cur_dirid == FILEMNG_DIR_PHOTO) {
		widget_set_visible(slider, FALSE);
		widget_set_visible(filetime, FALSE);
	} else {
		widget_set_visible(slider, TRUE);
		widget_set_visible(filetime, TRUE);
	}
#ifdef SERVICES_PLAYER_SUBVIDEO
	PLAYER_SERVICE_SetPlaySubStreamFlag(ps_handle, TRUE);
#endif
	s32Ret = PLAYER_SERVICE_SetInput(ps_handle, fullfilepath);
	if (s32Ret != 0) {
		CVI_LOGE("Player set input %s failed", fullfilepath);
		return s32Ret;
	}

	s32Ret = PLAYER_SERVICE_GetMediaInfo(ps_handle, &info);
	if (s32Ret != 0) {
		CVI_LOGE("Player Get MediaInfo %s failed", fullfilepath);
		return s32Ret;
	}

	if (s_cur_dirid == FILEMNG_DIR_PHOTO) {
		bool jpg_flag = check_jpg_file_complete(fullfilepath, s_cur_dirid);
		if (false == jpg_flag) {
			remove(fullfilepath);
			continue_play_file(PLAY_SELETE_NEXT_FILE);
			s32Ret = -1;
			return s32Ret;
		}
	}

	if (strcmp(info.video_codec, "hevc")) {
		s_totaltime = (uint32_t)info.duration_sec;
		widget_set_text_utf8(filenametitle, strrchr(fullfilepath, '/') + 1);
		PLAYER_SERVICE_Play(ps_handle);
	} else {
		ui_wrnmsg_update_type(MSG_EVENT_ID_NOTSUPPORT_H265);
		ui_winmng_startwin(UI_WRNMSG_PAGE, false);
	}
#endif
	return s32Ret;
}

ret_t continue_play_file(int32_t switch_file_flag)
{
#ifdef SERVICES_PLAYER_ON
	bool ret = false;
	int32_t s32Ret = -1;
    int32_t totalfile = (int32_t)FILEMNG_GetDirFileCnt(s_cur_camid, s_cur_dirid);
	ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
	if (ret == true) {
		return RET_OK;
	}

	PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
	switch (switch_file_flag) {
	case PLAY_SELETE_CUR_FILE:
		PLAYER_SERVICE_Play(ps_handle);
		break;
	case PLAY_SELETE_NEXT_FILE:
		PLAYER_SERVICE_Stop(ps_handle);
		s_curfileinx--;
		if (s_curfileinx < 0) {
			s_curfileinx = totalfile - 1;
		}
		s32Ret = play_file();
		if (s32Ret != 0) {
			ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
			ui_winmng_startwin(UI_WRNMSG_PAGE, false);
		}
		playback_update_time();
		break;
	case PLAY_SELETE_PRE_FILE:
		PLAYER_SERVICE_Stop(ps_handle);
		s_curfileinx++;
		if (s_curfileinx > (totalfile - 1)) {
			s_curfileinx = 0;
		}
		s32Ret = play_file();
		if (s32Ret != 0) {
			ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
			ui_winmng_startwin(UI_WRNMSG_PAGE, false);
		}
		playback_update_time();
		break;
	default:
		CVI_LOGI("ERROR PARM\n");
		break;
	}
#endif
	return RET_OK;
}

void playback_add_time(void)
{
	playback_update_time();
}

void playback_reset_time(void)
{
	if (s_cur_dirid == FILEMNG_DIR_PHOTO) {
		return;
	}
	playback_update_time();
	continue_play_file(PLAY_SELETE_NEXT_FILE);
}

static ret_t on_startplay_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
	bool ret = false;
	ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
	if (ret == true) {
		return RET_OK;
	}
	continue_play_file(PLAY_SELETE_CUR_FILE);
	return RET_OK;
}

static ret_t on_stopplay_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
#ifdef SERVICES_PLAYER_ON
	bool ret = false;
	ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
	if (ret == true) {
		return RET_OK;
	}
	PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
	PLAYER_SERVICE_Stop(ps_handle);
#endif
	return RET_OK;
}

static ret_t on_preplay_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
	continue_play_file(PLAY_SELETE_PRE_FILE);
	return RET_OK;
}

static ret_t on_nextplay_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
	continue_play_file(PLAY_SELETE_NEXT_FILE);
	return RET_OK;
}

static ret_t on_back_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
	back_close_window();
	return RET_OK;
}

static ret_t on_pauseplay_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
#ifdef SERVICES_PLAYER_ON
	bool ret = false;
	ret = playback_prompt_wrnmsg(MSG_WARN_ID_Normal);
	if (ret == true) {
		return RET_OK;
	}
	PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
	PLAYER_SERVICE_Pause(ps_handle);
#endif
	return RET_OK;
}

static int32_t playback_update_fileinfo(void)
{
	int32_t s32Ret = 0;
	int32_t totalfile = (int32_t)FILEMNG_GetDirFileCnt(s_cur_camid, s_cur_dirid);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	if (s_curfileinx >= totalfile) {
		s_curfileinx = 0;
	}

	CVI_LOGD("CurGrpIdx:%d, GroupCnt:%d \n", s_curfileinx, totalfile);

	if (0 == totalfile) {
		CVI_LOGD("file count:%d \n", totalfile);
		return -1;
	}

	return 0;
}

static int32_t playback_delete_curfile(void)
{
	int32_t s32Ret = 0;
	char filename[FILEMNG_PATH_MAX_LEN];
	char filepath[FILEMNG_PATH_MAX_LEN];
    char fullfilepath[FILEMNG_PATH_MAX_LEN *2];
    int32_t totalfile = (int32_t)FILEMNG_GetDirFileCnt(s_cur_camid, s_cur_dirid);
    FILEMNG_GetFileNameByFileInx(s_cur_camid, s_cur_dirid, s_curfileinx, &filename, 1);
	FILEMNG_GetFilePath(s_cur_camid, s_cur_dirid, filepath);
	snprintf(fullfilepath, sizeof(fullfilepath), "%s/%s", filepath, filename);
	CVI_LOGI("delete cur filename = %s\n", fullfilepath);
	if (1 < totalfile) {
		s32Ret = FILEMNG_DelFile(s_cur_camid, fullfilepath);
		APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

		OSAL_FS_Async();

		totalfile--;
		if (s_curfileinx >= totalfile) {
			s_curfileinx = totalfile - 1;
		}

		s32Ret = playback_update_fileinfo();
		APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

		play_file();
	} else {
		totalfile = 0;
		s_curfileinx = 0;
		s32Ret = FILEMNG_DelFile(s_cur_camid, fullfilepath);
		APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

		OSAL_FS_Async();
		playback_prompt_wrnmsg(MSG_WARN_ID_Delete_File);
		return 0;
	}
	return 0;
}

static ret_t on_deletecurfile_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
	playback_prompt_wrnmsg(MSG_WARN_ID_Delete_File);
	return RET_OK;
}

ret_t delete_file_playback(void)
{
    MAPI_DISP_HANDLE_T disp_handle = MEDIA_GetCtx()->SysHandle.dispHdl;
    MAPI_DISP_ClearBuf(disp_handle);
    playback_delete_curfile();
    return 0;
}

static ret_t on_deleteallfile_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;

	return RET_OK;
}

static ret_t on_cancel_click(void *ctx, event_t *e)
{
	(void)e;
	widget_t *win = WIDGET(ctx);
	window_close(win);
	ui_winmng_finishwin(UI_PLAYBACK_PAGE);

	return RET_OK;
}

static ret_t init_listview_widget(void *ctx, const void *iter)
{
	widget_t *widget = WIDGET(iter);
	widget_t *win = widget_get_window(widget);
	(void)ctx;

	if (widget->name != NULL) {
		const char *name = widget->name;
		if (tk_str_eq(name, "list_item_deletecur")) {
			widget_on(widget, EVT_CLICK, on_deletecurfile_click, win);
		} else if (tk_str_eq(name, "list_item_deleteall")) {
			widget_on(widget, EVT_CLICK, on_deleteallfile_click, win);
		} else if (tk_str_eq(name, "list_item_cancel")) {
			widget_on(widget, EVT_CLICK, on_cancel_click, win);
		}
	}

	return RET_OK;
}

void init_children_listview(widget_t *widget)
{
	widget_foreach(widget, init_listview_widget, widget);
}

static ret_t on_delete_click(void *ctx, event_t *e)
{
	(void)e;
	(void)ctx;
#ifdef SERVICES_PLAYER_ON
	PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    PLAYER_SERVICE_Stop(ps_handle);
    bool ret = false;
    ret = playback_prompt_wrnmsg(MSG_WARN_ID_Delete_File);
    if (ret == true) {
        return RET_OK;
    }

    MAPI_DISP_HANDLE_T disp_handle = MEDIA_GetCtx()->SysHandle.dispHdl;
    MAPI_DISP_ClearBuf(disp_handle);
    playback_delete_curfile();
#endif
	return RET_OK;
}

static ret_t on_sliderprogress_changed(void *ctx, event_t *e)
{
#ifdef SERVICES_PLAYER_ON
	(void)e;
	widget_t *widget = WIDGET(ctx);
	uint32_t slider_value = widget_get_value(widget);

	MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
	PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
	if (PLAYER_SERVICE_Seek(ps_handle, slider_value * 1000) != 0) {
		CVI_LOGE("Player seek locate failed");
	}
#endif
	return RET_OK;
}

static ret_t init_widget(void *ctx, const void *iter)
{
	widget_t *widget = WIDGET(iter);
	widget_t *win = widget_get_window(widget);
	(void)ctx;

	if (widget->name != NULL) {
		const char *name = widget->name;
		if (tk_str_eq(name, "palyback_pre_button")) {
			widget_on(widget, EVT_CLICK, on_preplay_click, win);
		} else if (tk_str_eq(name, "palyback_start_button")) {
			widget_on(widget, EVT_CLICK, on_startplay_click, win);
		} else if (tk_str_eq(name, "palyback_stop_button")) {
			widget_on(widget, EVT_CLICK, on_stopplay_click, win);
		} else if (tk_str_eq(name, "palyback_next_button")) {
			widget_on(widget, EVT_CLICK, on_nextplay_click, win);
		} else if (tk_str_eq(name, "palyback_back_button")) {
			widget_on(widget, EVT_CLICK, on_back_click, win);
		} else if (tk_str_eq(name, "palyback_pause_button")) {
			widget_on(widget, EVT_CLICK, on_pauseplay_click, win);
		} else if (tk_str_eq(name, "slider")) {
			widget_on(widget, EVT_VALUE_CHANGING, on_sliderprogress_changed, widget);
		} else if (tk_str_eq(name, "palyback_delete_button")) {
			widget_on(widget, EVT_CLICK, on_delete_click, win);
		}
	}

	return RET_OK;
}

static void init_children_widget(widget_t *widget)
{
	widget_foreach(widget, init_widget, widget);
}

void set_info_playback_window(int32_t camid, int32_t dirid, uint64_t fileinx)
{
	s_curfileinx = fileinx;
	s_cur_dirid = dirid;
	s_cur_camid = camid;
	return;
}

ret_t open_playback_window(void *ctx, event_t *e)
{
	(void)e;
	int32_t s32Ret = -1;
	widget_t *win = WIDGET(ctx);
	if (win) {
		window_manager_get_top_window(window_manager());
		init_children_widget(win);
		s32Ret = play_file();
		if (s32Ret != 0) {
			ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
			ui_winmng_startwin(UI_WRNMSG_PAGE, false);
		}
		return RET_OK;
	}

	return RET_FAIL;
}

ret_t close_playback_window(void *ctx, event_t *e)
{
	(void)e;
	widget_t *win = WIDGET(ctx);
	if (win) {
	}
	s_curfileinx = 0;
	s_cur_dirid = 0;
	s_cur_camid = 0;
	return RET_OK;
}


int32_t  ui_playbackpage_eventcb(void *argv, EVENT_S *msg)
{
    switch(msg->topic) {
        case EVENT_MODEMNG_MODEOPEN:
        {
            ui_winmng_startwin(UI_DIR_PAGE, true);
            break;
        }
        case EVENT_MODEMNG_MODECLOSE:
        {
            ui_winmng_closeallwin();
            break;
        }
        case EVENT_MODEMNG_CARD_REMOVE:
        {
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_MODESWITCH;
            Msg.arg1 = WORK_MODE_MOVIE;
            MODEMNG_SendMessage(&Msg);
            break;
        }
        case EVENT_MODEMNG_PLAYBACK_PLAY:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PLAY\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_FINISHED:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_FINISHED\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PROGRESS:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PROGRESS\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PAUSE:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PAUSE\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_RESUME:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_RESUME\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_ABNORMAL:
        {
            CVI_LOGI("EVENT_MODEMNG_PLAYBACK_ABNORMAL\n");
            break;
        }
        default:
            break;
    }

    return 0;
}

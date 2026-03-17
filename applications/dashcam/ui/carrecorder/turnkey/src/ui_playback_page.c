#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "ui_windowmng.h"
#include "filemng_dtcf.h"
#include "media_init.h"
#include "mode.h"
#include "player_service.h"

#define PLAY_SELETE_CUR_FILE  0
#define PLAY_SELETE_NEXT_FILE 1
#define PLAY_SELETE_PRE_FILE 2

typedef struct _UI_PLAYBACK_INFO_ {
    uint8_t init_flag;
    int32_t  curfilenum;
    int32_t  totalfile;
    uint32_t curdirtype;
    uint32_t pu32FileObjCnt;
    uint32_t backforward;
    char curfilename[MAX_PATH];
} UI_PLAYBACK_INFO_S;

UI_PLAYBACK_INFO_S playback_info = {0};

static uint32_t s_totaltime = 0;

static bool check_jpg_file_complete(const char*filename, uint32_t file_type)
{
    if (file_type == DTCF_DIR_PHOTO_FRONT || file_type == DTCF_DIR_PHOTO_REAR) {
/*         FILE *fp = fopen(filename, "rb");
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
        seek_loc = (read_temp_buf[4]<<24 | read_temp_buf[5]<<16 | read_temp_buf[6]<<8 | read_temp_buf[7]<<0) + seek_loc + 4;
        fseek(fp, seek_loc, SEEK_SET);
        fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
        if ((read_temp_buf[0] == 0xFF) && (read_temp_buf[1] == 0xD9) && (read_temp_buf[2] == 0xFF) && (read_temp_buf[3] == 0xD9)) {
            fclose(fp);
            return true;
        } else {
            fclose(fp);
            return false;
        } */


        char *fileextern = NULL;

        if (NULL == filename) {
            return false;
        }

        fileextern = (strrchr(filename, '.') + 1);
        if (0 == (strcmp(fileextern, "JPG"))) {
            FILE *fp = fopen(filename, "rb");
            if (NULL == fp) {
                CVI_LOGF("fail to open\n");
                return false;
            }
            unsigned char read_temp_buf[3] = {0};
            fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
            if((read_temp_buf[0] == 0xFF) && (read_temp_buf[1] == 0xD8) &&(read_temp_buf[2] == 0xFF)) {
                fclose(fp);
                return true;
            } else {
                fclose(fp);
                return false;
            }
        } else {
            return false;
        }
    }
    return true;
}

static void show_wrnwin(void)
{
    ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    MODEMNG_SetModeState(MEDIA_PLAYBACK_STATE_ABNORMAL);
}

static void playback_update_time(void)
{
    widget_t* win = window_manager_get_top_window(window_manager());
    widget_t* time = widget_lookup(win, "time_title_label", TRUE);
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
        return;
    } else if (((_totaltime - info.duration_sec) < 1.0) && ((info.duration_sec - last_time) == 0)) {
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

    snprintf(totaltime, sizeof(totaltime), "%02d:%02d/%02d:%02d", curmin, cursec, min, sec);
    widget_set_text_utf8(time, totaltime);
}

static ret_t ui_playback_start_play(void)
{
    int32_t  s32Ret = 0;
    widget_t* win = window_manager_get_top_window(window_manager());
    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    widget_t* image = widget_lookup(win, "play_pause_image", TRUE);

    s32Ret = PLAYER_SERVICE_Play(ps_handle);
    if(s32Ret == 0) {
        image_base_set_image(image,"ICON_PLAY_PAUSE");
    } else {
        image_base_set_image(image,"");
    }

    return s32Ret;
}

static ret_t ui_playback_page_updatefile(void)
{
    int32_t  s32Ret = 0;
    //char filename[MAX_PATH];
    char totaltime[64] = {};
    char res[64] = {};
    uint32_t min = 0, sec = 0;

    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
    PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    PLAYER_MEDIA_INFO_S info = {0};

    memset(playback_info.curfilename, 0, MAX_PATH);
    s32Ret =  FILEMNG_GetFileByIndex(playback_info.curfilenum, playback_info.curfilename, MAX_PATH);
    if (s32Ret != 0) {
        CVI_LOGE("error FILEMNG_GetFileByIndex\n");
        return s32Ret;
    }

    widget_t* win = window_manager_get_top_window(window_manager());
    widget_t* filenametitle = widget_lookup(win, "file_name_title_label", TRUE);
    widget_t* filetime = widget_lookup(win, "time_title_label", TRUE);
    widget_t* image = widget_lookup(win, "play_pause_image", TRUE);
    widget_t* label = widget_lookup(win, "speeds", TRUE);
    widget_t* resolve_power_title_label = widget_lookup(win, "resolve_power_title_label", TRUE);

    if (playback_info.backforward != 2) {
        playback_info.backforward = 2;
        widget_set_text_utf8(label, "");
    }

    if (playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT || playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
        widget_set_visible(filetime, FALSE);
    } else {
        widget_set_visible(filetime, TRUE);
    }

    s32Ret = PLAYER_SERVICE_SetInput(ps_handle, playback_info.curfilename);
    if (s32Ret != 0) {
        CVI_LOGE("Player set input %s failed", playback_info.curfilename);
        return s32Ret;
    }

    s32Ret = PLAYER_SERVICE_GetMediaInfo(ps_handle, &info);
    if (s32Ret != 0) {
        CVI_LOGE("Player Get MediaInfo %s failed", playback_info.curfilename);
        return s32Ret;
    }

    if (playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT || playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
        bool jpg_flag = check_jpg_file_complete(playback_info.curfilename, playback_info.curdirtype);
        if (false == jpg_flag) {
            return -1;
        }
    }

    if (strcmp(info.video_codec, "hevc")) {
        s_totaltime = (uint32_t)info.duration_sec;
        widget_set_text_utf8(filenametitle, strrchr(playback_info.curfilename, '/') + 1);
    } else {
        ui_wrnmsg_update_type(MSG_EVENT_ID_NOTSUPPORT_H265);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
        return -1;
    }

    snprintf(res, sizeof(res), "%dP", info.height);
    widget_set_text_utf8(resolve_power_title_label, res);

    min = (uint32_t)info.duration_sec/60;
    sec = (uint32_t)info.duration_sec - min * 60;
    snprintf(totaltime, sizeof(totaltime), "%02d:%02d/%02d:%02d",0,0,min,sec);

    widget_set_text_utf8(filetime, totaltime);
    image_base_set_image(image,"ICON_PLAY_PLAY");

    return s32Ret;
}

ret_t continue_play_file(int32_t  switch_file_flag)
{
    int32_t  s32Ret = -1;

    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    switch (switch_file_flag) {
        case PLAY_SELETE_CUR_FILE:
            PLAYER_SERVICE_Play(ps_handle);
            break;
        case PLAY_SELETE_NEXT_FILE:
            PLAYER_SERVICE_Stop(ps_handle);
            playback_info.curfilenum--;
            if(playback_info.curfilenum < 0) {
                playback_info.curfilenum = playback_info.totalfile - 1;
            }
            s32Ret = ui_playback_page_updatefile();
            s32Ret |= ui_playback_start_play();
            if(s32Ret != 0){
                show_wrnwin();
            }
            break;
        case PLAY_SELETE_PRE_FILE:
            PLAYER_SERVICE_Stop(ps_handle);
            playback_info.curfilenum++;
            if(playback_info.curfilenum > (playback_info.totalfile - 1)) {
                playback_info.curfilenum = 0;
            }
            s32Ret = ui_playback_page_updatefile();
            s32Ret |= ui_playback_start_play();
            if(s32Ret != 0){
                show_wrnwin();
            }
            break;
        default:
            CVI_LOGI("ERROR PARM\n");
            break;
    }
    return RET_OK;
}

static ret_t on_startplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    continue_play_file(PLAY_SELETE_CUR_FILE);
    return RET_OK;
}

static ret_t on_stopplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    PLAYER_SERVICE_Stop(ps_handle);
    MODEMNG_SetModeState(MEDIA_PLAYBACK_STATE_VIEW);
    return RET_OK;
}

static ret_t on_preplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    continue_play_file(PLAY_SELETE_PRE_FILE);
    return RET_OK;
}

static ret_t on_nextplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    continue_play_file(PLAY_SELETE_NEXT_FILE);
    return RET_OK;
}

static ret_t on_pauseplay_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    PLAYER_SERVICE_Pause(ps_handle);

    return RET_OK;
}

static int32_t  playback_update_fileinfo(void)
{
    int32_t  s32Ret = 0;
    CVI_DTCF_DIR_E aenDirs = playback_info.curdirtype;
    uint32_t totalfile = 0;
    s32Ret = FILEMNG_SetSearchScope(&aenDirs, 1, &totalfile);
    APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
    playback_info.totalfile = totalfile;

    if (playback_info.curfilenum >= playback_info.totalfile) {
        playback_info.curfilenum = 0;
    }

    CVI_LOGI("curfilenum:%u, totalfile:%u \n", playback_info.curfilenum, playback_info.totalfile);

    if (0 == playback_info.totalfile) {
        CVI_LOGD("file count:%d \n", playback_info.totalfile);
        return -1;
    }

    return 0;
}

static int32_t  playback_delete_curfile(void)
{
    int32_t  s32Ret = 0;

    CVI_LOGE("delete cur filename = %s\n", playback_info.curfilename);
    if (1 < playback_info.totalfile) {
        s32Ret = FILEMNG_RemoveFile(playback_info.curfilename);
        APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        //cvi_async();

        playback_info.totalfile--;
        if (playback_info.curfilenum >= playback_info.totalfile) {
            playback_info.curfilenum = playback_info.totalfile - 1;
        }

        s32Ret = playback_update_fileinfo();
        APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
        FILEMNG_GetFileByIndex(playback_info.curfilenum, playback_info.curfilename, MAX_PATH);
    } else {
        playback_info.totalfile = 0;
        playback_info.curfilenum = 0;
        s32Ret = FILEMNG_RemoveFile(playback_info.curfilename);
        APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

        //cvi_async();
        memset(playback_info.curfilename, 0, MAX_PATH);
    }
    return 0;
}

ret_t delete_file_playback(void)
{
    MAPI_DISP_HANDLE_T disp_handle = MEDIA_GetCtx()->SysHandle.dispHdl;
    MAPI_DISP_ClearBuf(disp_handle);
    playback_delete_curfile();
    cvi_async();
    return 0;
}

ret_t delete_all_file_playback(void)
{
    MAPI_DISP_HANDLE_T disp_handle = MEDIA_GetCtx()->SysHandle.dispHdl;
    MAPI_DISP_ClearBuf(disp_handle);

    int32_t  tmp = playback_info.totalfile;  //total will change after deleting one file.
    for(int32_t  i = 0; i < tmp; i++) {
        playback_delete_curfile();
    }
    cvi_async();
    return 0;
}

static void clear_disp_buf(void)
{
    MAPI_DISP_HANDLE_T disp_handle = MEDIA_GetCtx()->SysHandle.dispHdl;
    MAPI_DISP_ClearBuf(disp_handle);
}

static ret_t ui_playback_addtime(const idle_info_t* idle)
{
    playback_update_time();
    return RET_OK;
}

static ret_t ui_playback_reset(const idle_info_t* idle)
{
    if (playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT || playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
        return RET_OK;
    }
    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;

    PLAYER_SERVICE_Stop(ps_handle);
    playback_update_time();
    ui_playback_page_updatefile();
    //continue_play_file(PLAY_SELETE_NEXT_FILE);
    return RET_OK;
}

ret_t ui_playback_page_updatedir(uint32_t id)
{
    playback_info.curdirtype = id;

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
            playback_info.init_flag = 0;
            playback_info.backforward = 2;
            ui_dir_page_reset();
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
            idle_queue(ui_playback_reset, NULL);
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_FINISHED\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PROGRESS:
            idle_queue(ui_playback_addtime, NULL);
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PROGRESS\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_PAUSE:
            if (playback_info.backforward != 2) {
                idle_queue(ui_playback_reset, NULL);
            }
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_PAUSE\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_RESUME:
            CVI_LOGD("EVENT_MODEMNG_PLAYBACK_RESUME\n");
            break;
        case EVENT_MODEMNG_PLAYBACK_ABNORMAL:
        {
            ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            CVI_LOGI("EVENT_MODEMNG_PLAYBACK_ABNORMAL\n");
            break;
        }
        default:
            break;
    }

    return 0;
}

static ret_t on_playback_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    widget_t* win = WIDGET(ctx);
    uint32_t CurModeState = 0;
    static int32_t  speeds = 1;
    MODEMNG_GetModeState(&CurModeState);
    CVI_LOGD("on_playback_page_key_down\n");

    if (ui_winmng_getwinisshow(UI_WRNMSG_PAGE) == true) {
        MODEMNG_SetModeState(MEDIA_PLAYBACK_STATE_ABNORMAL);
        ui_winmng_finishwin(UI_WRNMSG_PAGE);
        if(0 == playback_info.totalfile) {
            playback_info.init_flag = 0;
            ui_winmng_startwin(UI_DIR_PAGE, true);
            return RET_STOP;
        }
        return RET_STOP;
    }

    if (evt->key == UI_KEY_POWER) {
        if(evt->alt == false) {
            if (MEDIA_PLAYBACK_STATE_ABNORMAL != CurModeState) {
                if (playback_info.backforward == 0 || playback_info.backforward == 1) {
                    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
                    PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
                    widget_t* label = widget_lookup(win, "speeds", TRUE);

                    speeds = 1;
                    PLAYER_SERVICE_PlayerSeep(ps_handle, speeds);
                    playback_info.backforward = 2;
                    widget_set_text_utf8(label, "");

                    return RET_STOP;
                }

                widget_t* image = widget_lookup(win, "play_pause_image", TRUE);
                if (MEDIA_PLAYBACK_STATE_VIEW == CurModeState || MEDIA_PLAYBACK_STATE_PAUSE == CurModeState) {
                    on_startplay_click(ctx, e);
                    image_base_set_image(image,"ICON_PLAY_PAUSE");
                } else {
                    on_pauseplay_click(ctx, e);
                    image_base_set_image(image,"ICON_PLAY_PLAY");
                }
            }
        }
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
        if(evt->alt == false) {
            if (MEDIA_PLAYBACK_STATE_PLAY != CurModeState) {
                on_stopplay_click(ctx, e);
                ui_winmng_startwin(UI_PLAYBACK_MENU_PAGE, true);
                return RET_STOP;
            } else {
                if (MEDIA_PLAYBACK_STATE_ABNORMAL != CurModeState) {
                    on_stopplay_click(ctx, e);
                    ui_playback_page_updatefile();
                    return RET_STOP;
                }
            }
        } else {
            playback_info.init_flag = 0;
            on_stopplay_click(ctx, e);
            clear_disp_buf();
            ui_winmng_startwin(UI_DIR_PAGE, true);
            return RET_STOP;
        }
    } else if (evt->key == UI_KEY_UP) {
        if(evt->alt == false) {
            if (MEDIA_PLAYBACK_STATE_VIEW == CurModeState || MEDIA_PLAYBACK_STATE_PAUSE == CurModeState ||
                MEDIA_PLAYBACK_STATE_ABNORMAL == CurModeState || playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT ||
                playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
                clear_disp_buf();
                on_preplay_click(ctx, e);
            } else {
                MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
                PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
                widget_t* label = widget_lookup(win, "speeds", TRUE);

                if (playback_info.backforward == 2) {
                    playback_info.backforward = 1;
                    speeds = 2;
                    widget_set_text_utf8(label, "-2X");
                    PLAYER_SERVICE_PlayerSeepBack(ps_handle, speeds);
                } else if (playback_info.backforward == 1) {
                    char text[8] = {0};

                    speeds = MIN((speeds << 1), 8);
                    snprintf(text, sizeof(text), "-%dX", speeds);
                    widget_set_text_utf8(label, text);
                    PLAYER_SERVICE_PlayerSeepBack(ps_handle, speeds);
                } else if (playback_info.backforward == 0) {
                    char text[8] = {0};

                    speeds = MAX((speeds >> 1), 1);
                    if (speeds == 1) {
                        playback_info.backforward = 2;
                        widget_set_text_utf8(label, "");
                        PLAYER_SERVICE_PlayerSeep(ps_handle, speeds);
                    } else {
                        snprintf(text, sizeof(text), "%dX", speeds);
                        widget_set_text_utf8(label, text);
                        PLAYER_SERVICE_PlayerSeep(ps_handle, speeds);
                    }
                }
            }
        }
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        if(evt->alt == false) {
            if (MEDIA_PLAYBACK_STATE_VIEW == CurModeState || MEDIA_PLAYBACK_STATE_PAUSE == CurModeState ||
                MEDIA_PLAYBACK_STATE_ABNORMAL == CurModeState || playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT ||
                playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
                clear_disp_buf();
                on_nextplay_click(ctx, e);
            } else {
                MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
                PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
                widget_t* label = widget_lookup(win, "speeds", TRUE);

                if (playback_info.backforward == 2) {
                    playback_info.backforward = 0;
                    speeds = 2;
                    widget_set_text_utf8(label, "2X");
                    PLAYER_SERVICE_PlayerSeep(ps_handle, speeds);
                } else if (playback_info.backforward == 1) {
                    char text[8] = {0};

                    speeds = MAX((speeds >> 1), 1);
                    if (speeds == 1) {
                        playback_info.backforward = 2;
                        widget_set_text_utf8(label, "");
                        PLAYER_SERVICE_PlayerSeep(ps_handle, speeds);
                    } else {
                        snprintf(text, sizeof(text), "-%dX", speeds);
                        widget_set_text_utf8(label, text);
                        PLAYER_SERVICE_PlayerSeepBack(ps_handle, speeds);
                    }
                } else if (playback_info.backforward == 0) {
                    char text[8] = {0};
                    speeds = MIN((speeds << 1), 8);
                    snprintf(text, sizeof(text), "%dX", speeds);
                    widget_set_text_utf8(label, text);
                    PLAYER_SERVICE_PlayerSeep(ps_handle, speeds);
                }
            }
        }
        return RET_STOP;
    }
    return RET_OK;
}

static ret_t hide_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    bool en = *((bool *)ctx);
    if (en == true) {
        if (widget->name != NULL) {
            const char* name = widget->name;
            if (tk_str_eq(name, "set_leftkey")) {
                widget_set_visible(widget, FALSE, FALSE);
            } else if (tk_str_eq(name, "time_title_label")) {
                widget_set_visible(widget, FALSE, FALSE);
            } else if (tk_str_eq(name, "play_video_image")) {
                widget_set_visible(widget, FALSE, FALSE);
            }
        }
    } else {
        if (widget->name != NULL) {
            const char* name = widget->name;
            if (tk_str_eq(name, "set_leftkey")) {
                widget_set_visible(widget, FALSE, FALSE);
            } else if (tk_str_eq(name, "top_view")) {
                widget_set_visible(widget, FALSE, FALSE);
            } else if (tk_str_eq(name, "resolve_power_title_label")) {
                widget_set_visible(widget, FALSE, FALSE);
            } else if (tk_str_eq(name, "speeds")) {
                widget_set_visible(widget, FALSE, FALSE);
            }
        }
    }

    return RET_OK;
}

static ret_t ui_playback_hidewidget(widget_t* widget, bool isphoto)
{
    widget_foreach(widget, hide_widget, (void *)&isphoto);
    return RET_OK;
}

static ret_t ui_playback_page_init(widget_t* widget)
{
    int32_t  s32Ret = 0;

    if (0 == playback_info.init_flag) {
        FILEMNG_SetSearchScope(&playback_info.curdirtype, 1, &playback_info.pu32FileObjCnt);
        playback_info.curfilenum = playback_info.pu32FileObjCnt - 1;
        playback_info.totalfile = playback_info.pu32FileObjCnt;
    }
    CVI_LOGD("Debug: tpye: %d, curfilenum: %d, total: %d\n",playback_info.curdirtype,playback_info.curfilenum,playback_info.totalfile);
    if (playback_info.totalfile == 0) {
        if (playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT || playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
            ui_wrnmsg_update_type(MSG_EVENT_ID_NOPHOTO_FILE);
        } else {
            ui_wrnmsg_update_type(MSG_EVENT_ID_NOVIDEO_FILE);
        }
        ui_playback_hidewidget(widget, false);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
        return RET_OK;
    }
    s32Ret = ui_playback_page_updatefile();
    if(s32Ret != 0){
        ui_wrnmsg_update_type(MSG_EVENT_ID_FILE_ABNORMAL);
        ui_winmng_startwin(UI_WRNMSG_PAGE, false);
        return RET_OK;
    }

    if (playback_info.curdirtype == DTCF_DIR_PHOTO_FRONT || playback_info.curdirtype == DTCF_DIR_PHOTO_REAR) {
        ui_playback_hidewidget(widget, true);
    }

    /*start play*/
    s32Ret = ui_playback_start_play();
    if(s32Ret != 0) {
        show_wrnwin();
        return RET_OK;
    }

    return RET_OK;
}

ret_t ui_playback_page_open(void* ctx, event_t* e)
{
    (void)e;

    widget_t* win = WIDGET(ctx);
    win = WIDGET(ctx);
    if (win) {
        ui_playback_page_init(win);
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_playback_page_key_down, win);
        playback_info.init_flag = 1;
        playback_info.backforward = 2;
    }

    return RET_OK;
}

ret_t ui_playback_page_close(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;

    return RET_OK;
}

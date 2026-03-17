#ifndef __UI_WINDOWMNG_H__
#define __UI_WINDOWMNG_H__
#include "ui_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef ret_t (*UIFunc_CallBack)(void* ctx, event_t* e);

typedef enum UIWINDOWMNG_ID {
    UI_HOME_PAGE,
    UI_DIR_PAGE,
    UI_PLAYBACK_PAGE,
    UI_OPTION_PAGE,
    UI_CLOSE_MACHINE,
    UI_DELETEFILE_PAGE,
    UI_FILELIST_PAGE,
    UI_FORMAT_PAGE,
    UI_SETTIME_PAGE,
    UI_SET_PAGE,
    UI_STORAGE_PAGE, // 10
    UI_UVC_PAGE,
    UI_WRNMSG_PAGE,
    UI_HOME_PHOTO_PAGE,
    UI_PHOTO_SET_PAGE,
    UI_WINLIST_SIZE,
} UIWINDOWMNG_ID_E;

static const char s_WinName[UI_WINLIST_SIZE][64] = {
    "ui_home_page",
    "ui_dir_page",
    "ui_playback_page",
    "ui_option_page",
    "ui_close_machine",
    "ui_deletefile_page",
    "ui_filelist_page",
    "ui_format_page",
    "ui_settime_page",
    "ui_setting_page",
    "ui_storage_page",
    "ui_uvc_page",
    "ui_wrnmsg_page",
    "ui_home_photo_page",
    "ui_photo_set_page"
};

typedef struct tagUI_WINDOW_CALLBACK_S {
    UIFunc_CallBack ui_open;
    UIFunc_CallBack ui_close;
}UI_WINDOW_CALLBACK_S;

static const UI_WINDOW_CALLBACK_S s_astWinCallBack[UI_WINLIST_SIZE] = {
    {ui_home_open, ui_home_close},
    {open_dir_window, close_dir_window},
    {open_playback_window, close_playback_window},
    {open_option_window, close_option_window},
    {NULL, NULL},
    {NULL, NULL},
    {open_filelist_window, close_filelist_window},
    {open_format_window, close_format_window},
    {open_settime_window, close_settime_window},
    {open_menu_window, close_menu_window},
    {ui_open_storage, NULL},
    {ui_open_uvc, NULL},
    {open_wrnmsg_window, close_wrnmsg_window},
    {ui_home_photo_open, ui_home_photo_close},
    {open_menu_window, close_menu_window},
};

int32_t  ui_winmng_init(void);
int32_t  ui_winmng_deinit(void);
int32_t  ui_winmng_startwin(int32_t  WinHdl, bool bCloseCurWin);
int32_t  ui_winmng_finishwin(int32_t  WinHdl);
int32_t  ui_winmng_closeallwin(void);
bool ui_winmng_getwinisshow(int32_t  WinHdl);
int32_t ui_winmng_receivemsgresult(EVENT_S* pstEvent);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
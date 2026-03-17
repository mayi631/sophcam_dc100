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
    UI_OPTION_PAGE,
    UI_IDE_OPTION_PAGE,
    UI_IDENTICAL_PAGE,
    UI_DATETIME_PAGE,
    UI_DIR_PAGE,
    UI_PLAYBACK_PAGE,
    UI_PLAYBACK_MENU_PAGE,
    UI_SET_PAGE,
    UI_WRNMSG_PAGE,
    UI_WINLIST_SIZE
} UIWINDOWMNG_ID_E;

static const char s_WinName[UI_WINLIST_SIZE][64] = {
    "ui_home_page",
    "ui_option_page",
    "ui_optioniden_page",
    "ui_identical_page",
    "ui_datetime_page",
    "ui_dir_page",
    "ui_playback_page",
    "ui_playback_menu_page",
    "ui_set_page",
    "ui_wrnmsg_page"
};

typedef struct tagUI_WINDOW_CALLBACK_S {
    UIFunc_CallBack ui_open;
    UIFunc_CallBack ui_close;
}UI_WINDOW_CALLBACK_S;

static const UI_WINDOW_CALLBACK_S s_astWinCallBack[UI_WINLIST_SIZE] = {
    {ui_home_page_open, ui_home_page_close},
    {ui_option_page_open, ui_option_page_close},
    {ui_ide_option_page_open, ui_ide_option_page_close},
    {ui_identical_page_open, ui_identical_page_close},
    {ui_datetime_page_open, ui_datetime_page_close},
    {ui_dir_page_open, ui_dir_page_close},
    {ui_playback_page_open, ui_playback_page_close},
    {ui_playback_menu_page_open, ui_playback_menu_page_close},
    {ui_set_page_open, ui_set_page_close},
    {ui_wrnmsg_page_open, ui_wrnmsg_page_close},
};

int32_t  ui_winmng_init(void);
int32_t  ui_winmng_deinit(void);
int32_t  ui_winmng_startwin(int32_t  WinHdl, bool bCloseCurWin);
int32_t  ui_winmng_finishwin(int32_t  WinHdl);
int32_t  ui_winmng_closeallwin(void);
bool ui_winmng_getwinisshow(int32_t  WinHdl);
bool ui_format_cardstatus(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

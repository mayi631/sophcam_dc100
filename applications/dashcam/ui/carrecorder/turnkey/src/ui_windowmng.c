#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#include "ui_windowmng.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct tagUI_WINDOW_S {
    char WinName[64];
    uint8_t u8WinId;
    bool bWinIsShow;
    bool bCloseCurWin;
    UI_WINDOW_CALLBACK_S ui_callback;
}UI_WINDOW_S;

static pthread_mutex_t s_UIWinMngMutex = PTHREAD_MUTEX_INITIALIZER;
static UI_WINDOW_S s_astUIWin[UI_WINLIST_SIZE] = {0};
static widget_t* s_curWin = NULL;
static uint8_t s_u8CurWinId = UI_WINLIST_SIZE;

static void ui_windowmng_dumplist(void)
{
    for (int32_t  i = 0; i < UI_WINLIST_SIZE; i++) {
        if (s_astUIWin[i].bWinIsShow == true) {
            CVI_LOGD("=====>>>>>>>>>>>>>%s window is opend\n", s_astUIWin[i].WinName);
        }
    }
    widget_t* topwin = window_manager_get_top_window(window_manager());
    CVI_LOGD("===>>>>>>>> top window is %s \n", topwin->name);
}

static ret_t ui_open_page(const idle_info_t* idle)
{
    UI_WINDOW_S *UiWin = (UI_WINDOW_S *)(idle->ctx);
    widget_t* win = NULL;

    if ((s_curWin) && (strcmp(UiWin->WinName, s_curWin->name) == 0)) {
        CVI_LOGD("%s is opend !!!!!!!\n", UiWin->WinName);
        return RET_OK;
    }

    if (UiWin->bCloseCurWin == false) {
        if (s_curWin != NULL) {
            window_manager_dispatch_window_event(s_curWin, EVT_WINDOW_TO_BACKGROUND);
        }
        win = window_open(UiWin->WinName);
    } else {
        if (s_curWin != NULL) {
            window_close_force(s_curWin);
            for (int32_t  i = 0; i < UI_WINLIST_SIZE; i++) {
                if (strcmp(s_astUIWin[i].WinName, s_curWin->name) == 0) {
                    s_astUIWin[i].bWinIsShow = false;
                    break;
                }
            }
        }
        win = window_open(UiWin->WinName);
    }
    if (win) {
        if (UiWin->ui_callback.ui_open != NULL && UiWin->ui_callback.ui_close != NULL) {
            widget_on(win, EVT_WINDOW_OPEN, UiWin->ui_callback.ui_open, win);
            widget_on(win, EVT_WINDOW_CLOSE, UiWin->ui_callback.ui_close, win);
        } else {
            CVI_LOGE("%s windown no callback \n", __func__);
        }
    }

    s_curWin = win;
    s_u8CurWinId = UiWin->u8WinId;
    UiWin->bWinIsShow = true;
    ui_windowmng_dumplist();
    return RET_OK;
}

static ret_t ui_close_page(const idle_info_t* idle)
{
    UI_WINDOW_S *UiWin = (UI_WINDOW_S *)(idle->ctx);
    if (UiWin->bWinIsShow == true) {
        widget_t* win = widget_lookup(window_manager(), UiWin->WinName, false);
        if (win) {
            window_close_force(win);
        }
        UiWin->bWinIsShow = false;
        widget_t* topwin = window_manager_get_top_window(window_manager());
        if (topwin != NULL) {
            for (int32_t  i = 0; i < UI_WINLIST_SIZE; i++) {
                if (strcmp(s_astUIWin[i].WinName, topwin->name) == 0) {
                    s_curWin = topwin;
                    s_u8CurWinId = s_astUIWin[i].u8WinId;
                    break;
                }
            }
            window_manager_dispatch_window_event(s_curWin, EVT_WINDOW_TO_FOREGROUND);
        } else {
            s_curWin = NULL;
            s_u8CurWinId = UI_WINLIST_SIZE;
            CVI_LOGE("%s windown is null\n", __func__);
        }

    } else {
        CVI_LOGE("%s %s windown no open\n", __func__, UiWin->WinName);
    }
    ui_windowmng_dumplist();
    return RET_OK;
}

static ret_t ui_closeall_page(const idle_info_t* idle)
{
    (void)idle;
    for (int32_t  i = 0; i < UI_WINLIST_SIZE; i++) {
        if (s_astUIWin[i].bWinIsShow == true) {
            widget_t* win = widget_lookup(window_manager(), s_astUIWin[i].WinName, false);
            if (win) {
                window_close_force(win);
                s_astUIWin[i].bWinIsShow = false;
            }
        }
    }
    s_curWin = NULL;
    s_u8CurWinId = UI_WINLIST_SIZE;
    return RET_OK;
}

int32_t  ui_winmng_init(void)
{
    int32_t  i = 0;

    MUTEX_LOCK(s_UIWinMngMutex);

    while(i<UI_WINLIST_SIZE)
    {
        snprintf(s_astUIWin[i].WinName, sizeof(s_astUIWin[i].WinName), "%s", s_WinName[i]);
        s_astUIWin[i].u8WinId = i;
        s_astUIWin[i].ui_callback.ui_open = s_astWinCallBack[i].ui_open;
        s_astUIWin[i].ui_callback.ui_close = s_astWinCallBack[i].ui_close;
        i++;
    }

    MUTEX_UNLOCK(s_UIWinMngMutex);

    return 0;
}

int32_t  ui_winmng_deinit(void)
{
    int32_t  i = 0;

    MUTEX_LOCK(s_UIWinMngMutex);

    while(i<UI_WINLIST_SIZE)
    {
        snprintf(s_astUIWin[i].WinName, sizeof(s_astUIWin[i].WinName), " ");
        s_astUIWin[i].u8WinId = 0;
        s_astUIWin[i].bWinIsShow = false;
        s_astUIWin[i].ui_callback.ui_open = NULL;
        s_astUIWin[i].ui_callback.ui_close = NULL;
        i++;
    }

    MUTEX_UNLOCK(s_UIWinMngMutex);

    return 0;
}

int32_t  ui_winmng_startwin(int32_t  WinHdl, bool bCloseCurWin)
{
    MUTEX_LOCK(s_UIWinMngMutex);
    ui_lock();
    s_astUIWin[WinHdl].bCloseCurWin = bCloseCurWin;
    if (WinHdl == UI_WRNMSG_PAGE) {
        if (s_u8CurWinId == UI_WRNMSG_PAGE) {
            idle_queue(ui_wrnmsg_update_msglael, NULL);
            ui_unlock();
            MUTEX_UNLOCK(s_UIWinMngMutex);
            return 0;
        }
    }
    idle_queue(ui_open_page, (void *)&s_astUIWin[WinHdl]);
    ui_unlock();
    MUTEX_UNLOCK(s_UIWinMngMutex);
    return 0;
}

int32_t  ui_winmng_finishwin(int32_t  WinHdl)
{
    MUTEX_LOCK(s_UIWinMngMutex);
    ui_lock();
    idle_queue(ui_close_page, (void *)&s_astUIWin[WinHdl]);
    ui_unlock();
    MUTEX_UNLOCK(s_UIWinMngMutex);
    return 0;
}

int32_t  ui_winmng_closeallwin(void)
{
    MUTEX_LOCK(s_UIWinMngMutex);
    ui_lock();
    idle_queue(ui_closeall_page, NULL);
    ui_unlock();
    MUTEX_UNLOCK(s_UIWinMngMutex);
    return 0;
}

bool ui_winmng_getwinisshow(int32_t  WinHdl)
{
    return s_astUIWin[WinHdl].bWinIsShow;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
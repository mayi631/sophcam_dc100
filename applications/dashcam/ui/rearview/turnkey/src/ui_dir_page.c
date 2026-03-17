#include "ui_windowmng.h"
#include "filemng.h"
#ifdef SERVICES_PLAYER_ON
#include "playbackmng.h"
#endif

#include "mode.h"

static int32_t s_dir_remap[MAX_CAMERA_INSTANCES][FILEMNG_DIR_BUTT];

static ret_t on_diritem_click(void *ctx, event_t *e)
{
	(void)e;
	if (MODEMNG_GetInProgress() == true) {
		return RET_OK;
	}

	widget_t *list_item = WIDGET(ctx);
	int32_t index = widget_index_of(list_item);
    int32_t camid = -1, dir_id = -1;

	for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
		for (int32_t j = 0; j < FILEMNG_DIR_BUTT; j++) {
            if(s_dir_remap[i][j] == 1){
                if(index == 0){
                    camid = i;
                    dir_id = j;
                    break;
                }
                index--;
            }
		}
		if (camid != -1) {
			break;
		}
	}
    if(camid == -1){
        return RET_OK;
    }

	set_current_directory(camid, dir_id);
	ui_winmng_startwin(UI_FILELIST_PAGE, false);

	return RET_OK;
}

static ret_t on_dirback_click(void *ctx, event_t *e)
{
	(void)ctx;
	(void)e;
	ui_winmng_finishwin(UI_DIR_PAGE);

    MESSAGE_S Msg;
    Msg.topic = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1 = WORK_MODE_MOVIE;
    MODEMNG_SendMessage(&Msg);

	return RET_OK;
}

ret_t open_dir_window(void *ctx, event_t *e)
{
	widget_t *win = WIDGET(ctx);

	if (win) {
		widget_t *scroll_view = widget_lookup(win, "scroll_view", TRUE);
		widget_t *button = widget_lookup(win, "back_button", TRUE);
		widget_on(button, EVT_CLICK, on_dirback_click, win);

		for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
			for (int32_t j = 0; j < FILEMNG_DIR_BUTT; j++) {
				char dir_name[FILEMNG_PATH_MAX_LEN] = {0};
				FILEMNG_GetDirName(i, j, dir_name, FILEMNG_PATH_MAX_LEN);
				s_dir_remap[i][j] = 0;
				if (strlen(dir_name) > 0) {
					s_dir_remap[i][j] = 1;
					widget_t *list_item = list_item_create(scroll_view, 0, 0, 0, 0);
					widget_set_text_utf8(list_item, dir_name);
					widget_on(list_item, EVT_CLICK, on_diritem_click, list_item);
				}
			}
		}

		return RET_OK;
	}

	return RET_FAIL;
}

ret_t close_dir_window(void *ctx, event_t *e)
{
	(void)e;
	(void)ctx;
	return RET_OK;
}
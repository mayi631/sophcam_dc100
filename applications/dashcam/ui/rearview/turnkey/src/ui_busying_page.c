#include "ui_common.h"

static bool s_busyingOpen = false;
static widget_t* busying_win = NULL;

ret_t close_busying_page(void)
{
    if (s_busyingOpen == true) {
        s_busyingOpen = false;
        widget_t* gif_ctrl = widget_lookup(busying_win, "gif", TRUE);
        widget_destroy(gif_ctrl);
        usleep(20000);
        window_close(busying_win);
        busying_win = NULL;
    }

    return RET_STOP;
}

ret_t open_busying_page(void)
{
    if (s_busyingOpen == false) {
        busying_win = window_open("ui_busying_page");
        if (busying_win != NULL) {
            s_busyingOpen = true;
        }
        return RET_OK;
    } else {

        return RET_OK;
    }
    return RET_FAIL;
}
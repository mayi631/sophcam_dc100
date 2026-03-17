
#ifndef __PAGE_ALBUMPIC_H_
#define __PAGE_ALBUMPIC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern bool is_album_pic;
extern lv_obj_t* obj_AibumPic_s;

const char* get_curr_pic_path(void);
void setup_scr_screen_PhotoAlbumPic(lv_ui_t* ui);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

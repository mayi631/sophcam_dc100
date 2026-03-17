#ifndef __PAGE_ALBUM_VIDEO_H_
#define __PAGE_ALBUM_VIDEO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
#include "lvgl/lvgl.h"  // 确保包含LVGL主头文件

extern lv_obj_t *obj_AibumVid_s;
extern char current_video_path[256];

void setup_scr_screen_PhotoAlbumVid(lv_ui_t *ui);
void video_playback_task(lv_timer_t *timer);
void video_play_pause();
void video_seek_to(int position);

#ifdef __cplusplus
}
#endif
#endif /* __PAGE_ALBUM_VIDEO_H_ */
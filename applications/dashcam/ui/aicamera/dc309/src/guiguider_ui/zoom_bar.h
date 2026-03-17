
#ifndef __ZOOM_BAR_H_
#define __ZOOM_BAR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void delete_zoombar_timer_handler(void);
void create_zoom_bar(lv_obj_t *parent);
void update_zoom_bar(uint32_t level);
void hide_zoom_bar(void);
uint32_t get_zoom_level();
void set_zoom_level(uint32_t level);
void delete_zoom_bar(void);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

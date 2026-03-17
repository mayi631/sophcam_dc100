
#ifndef __AIPROCESS_PUP_H_
#define __AIPROCESS_PUP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void delete_aiselect_scroll(void);
lv_obj_t *get_aiselete_scroll_handl(void);
void photoAISelect_listCreat(lv_obj_t *parent,lv_event_cb_t event_cb);
void AISelect_prev(void);
void AISelect_next(void);
void set_currIndex_focus(int index);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

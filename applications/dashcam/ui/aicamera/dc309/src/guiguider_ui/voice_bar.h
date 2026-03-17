
#ifndef __VOICE_BAR_H
#define __VOICE_BAR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"


lv_obj_t *get_arc_handel(void);
void voice_setting_arc_create(void);
void voice_arc_delete(void);

void volume_add(void);
void volume_reduce(void);

int32_t do_zoomin(int32_t key_value);
int32_t do_zoomout(int32_t key_value);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */


#ifndef __PAGE_STALIGHT_H_
#define __PAGE_STALIGHT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_sysMenu_statuslight_s; //底层窗口

void sysMenu_stlight(lv_ui_t *ui);
void setsysMenu_stlight_Label(char* plabel);
void setsysMenu_stlight_Index(uint8_t index);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

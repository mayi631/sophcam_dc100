
#ifndef __PAGE_SYS_PWOERDOWN_H_
#define __PAGE_SYS_PWOERDOWN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_sysMenu_PowerDown_s;
void sysMenu_PowerDown(lv_ui_t *ui);
void setpoweroff_Index(uint8_t index);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

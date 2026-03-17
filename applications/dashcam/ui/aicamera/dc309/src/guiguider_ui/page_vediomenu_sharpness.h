
#ifndef __PAGE_SHARPNESS_H
#define __PAGE_SHARPNESS_H
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
extern lv_obj_t *obj_Vedio_Sharpness_s; //底层窗口
uint8_t getSharness_index(void);
void vedioMenu_Sharpness(lv_ui_t *ui);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

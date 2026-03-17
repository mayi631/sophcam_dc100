
#ifndef __PAGE_SETTINGEXPOSURE_H_
#define __PAGE_SETTINGEXPOSURE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void photoMenu_Exposure(lv_ui_t *ui);

uint8_t get_EV_Level(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

#ifndef __PAGE_SYS_LIGHTFREQ_H_
#define __PAGE_SYS_LIGHTFREQ_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_sysMenu_LightFreq_s;

uint8_t getLightFreq_Index(void);
void SetLightFreq_Index(int32_t index);
void sysMenu_LightFreq(lv_ui_t *ui);
void SetLightFreq_Label(const char* plabel);
char* GetLightFreq_Label(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

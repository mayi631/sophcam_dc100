
#ifndef __PAGE_SYS_VOLUME_H_
#define __PAGE_SYS_VOLUME_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

#define SYSTEM_DEFAULT_VOLUME 5

extern lv_obj_t *obj_sysMenu_Volume_s;

uint8_t getaction_audio_Index(void);
void setaction_audio_Index(uint8_t index);
void sysMenu_Volume(lv_ui_t *ui);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */


#ifndef __PAGE_SETTING_FACEDEC_H_
#define __PAGE_SETTING_FACEDEC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void photoMenu_FaceDectection(lv_ui_t *ui);
uint8_t getface_Index(void);
void setface_Index(uint8_t index);
void setface_Label(const char* plabel);
char* getface_Label(void);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

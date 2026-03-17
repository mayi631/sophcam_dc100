
#ifndef __PAGE_SETTING_SMILEDEC_H_
#define __PAGE_SETTING_SMILEDEC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
void photoMenu_SmileDectection(lv_ui_t *ui);
uint8_t getsmile_Index(void);
void setsmile_Index(uint8_t index);
void setsmile_Label(const char* plabel);
char* getsmile_Label(void);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

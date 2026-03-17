
#ifndef __PAGE_SETTING_PICQUAL_H_
#define __PAGE_SETTING_PICQUAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void photoMenu_PictureQuality(lv_ui_t *ui);
void photo_setQuality_Index(uint8_t index);
uint8_t photo_getQuality_Index(void);
void photo_setQuality_Label(const char* plabel);
char* photo_getQuality_Label(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

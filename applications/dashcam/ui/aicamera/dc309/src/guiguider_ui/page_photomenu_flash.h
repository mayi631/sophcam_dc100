
#ifndef __PAGE_PHOEO_FLASH_H_
#define __PAGE_PHOEO_FLASH_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *photomenu_flash_s;

void photoMenu_Flash(lv_ui_t *ui);
void photo_setFlash_Index(uint8_t index);
uint8_t photo_getFlash_Index(void);
void photo_setFlash_Label(const char* plabel);
char* photo_getFlash_Label(void);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

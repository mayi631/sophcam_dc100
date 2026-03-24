#ifndef __PAGE_PHOTOMENU_CURSOR_H_
#define __PAGE_PHOTOMENU_CURSOR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
extern lv_obj_t *obj_sysMenu_cursor_s;

uint8_t get_curr_cursor(void);
void cursor_Index(uint8_t index);
void cursor_Label(char* plabel);
void photoMenu_Cursor(lv_ui_t *ui);

#ifdef __cplusplus
}
#endif
#endif /* __PAGE_PHOTOMENU_CURSOR_H_ */

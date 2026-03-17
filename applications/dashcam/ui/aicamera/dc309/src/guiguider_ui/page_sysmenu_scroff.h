
#ifndef __PAGE_SCREEN_OFF_H_
#define __PAGE_SCREEN_OFF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
    UI_SET_SCREEN_ON = 0,
    UI_SET_SCREEN_OFF,
} sysmenu_screen_status_e;


extern lv_obj_t *obj_sysMenu_screenoff_s; //底层窗口
void sysMenu_screenoff(lv_ui_t *ui);
uint8_t getoff_Index(void);
void setsysMenu_ScrOff_Index(uint8_t index);
void setsysMenu_ScrOff_Label(char* plabel);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

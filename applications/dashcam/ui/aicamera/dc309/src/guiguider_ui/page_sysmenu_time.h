
#ifndef __SYSMENU_TIME_H_
#define __SYSMENU_TIME_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_sysMenu_Time_s;

typedef enum {
    TIME_FLAG_ON = 0,
    TIME_FLAG_OFF,
    TIME_SETTING,
} sysmenu_time_flag_e;

uint8_t getSelect_Index(void);
void sysMenu_Time(lv_ui_t *ui);
void SettimeSelect_Index(int32_t index);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

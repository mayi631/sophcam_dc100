
#ifndef __PAGE_VEDIOMENU_SETTING_H_
#define __PAGE_VEDIOMENU_SETTING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_vedioMenu_s;
extern bool screen_vedioSetting_del;

typedef enum {
    VEDIO_EFFECT = 0,
    VEDIO_RES,
    // VEDIO_BEAUTY,
    VEDIO_GRAPHY,
    // VEDIO_LAPSE_TIME,
    VEDIO_ISO,
    VEDIO_EXPOSE,
    VEDIO_WHITE_BLA,
    VEDIO_SHARPNESS,
    VEDIO_CURSOR,
} Vedio_Select_Item_e;

void vedioMenu_Setting(lv_ui_t *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

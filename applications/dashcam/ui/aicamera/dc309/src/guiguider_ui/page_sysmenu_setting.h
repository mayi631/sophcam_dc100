
#ifndef __PAGE_SYS_SETTING_H_
#define __PAGE_SYS_SETTING_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
typedef enum {
    SYSMENU_WIFILIST = 0,
    SYSMENU_LANGUAGE,
    SYSMENU_TIME,
    SYSMENU_POWERDOWN,
    SYSMENU_SCREEN_OFF,
    SYSMENU_VOLUME,
    SYSMENU_LIGHTFREQ,
    SYSMENU_LIGHTBRIGHT,
    SYSMENU_FORMAT,
    SYSMENU_FACTORYSET,
    SYSMENU_VERSION,
    SYSMENU_STALIGHT,
} SyeMenu_Select_Item_e;

extern lv_obj_t *obj_sysMenu_Setting_s; //底层窗口
void sysMenu_Setting(lv_ui_t *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

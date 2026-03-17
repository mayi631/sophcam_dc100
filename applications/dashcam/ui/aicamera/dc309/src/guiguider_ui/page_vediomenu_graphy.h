
#ifndef __PAGE_VEDIO_GRAPHY_H_
#define __PAGE_VEDIO_GRAPHY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_Vedio_Graphy_s; //底层窗口

uint8_t getgraphy_mode_Index(void);//获取当前设置的模式
void setgraphy_mode_Index(uint8_t index);//设置当前的模式
void vedioMenu_Graphy(lv_ui_t *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

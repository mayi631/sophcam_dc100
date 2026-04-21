
#ifndef __PAGE_HOME_H_
#define __PAGE_HOME_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t *obj_home_s;
void setup_scr_home1(lv_ui_t *ui);
// 设置是否更新文字颜色
void set_update_text_color(bool enable);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

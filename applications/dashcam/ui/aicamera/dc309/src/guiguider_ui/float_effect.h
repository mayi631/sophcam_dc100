
#ifndef __FLOAT_EFFECT_H_
#define __FLOAT_EFFECT_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "gui_guider.h"
void delete_all_handle(void);
bool get_is_effect_exist(void);
void create_gradually_hide_anim(lv_anim_completed_cb_t completed_cb,uint32_t time);
void float_effect_creat(lv_obj_t *img_handel,lv_obj_t *parent);
void effect_Select_prev(void);
void effect_AISelect_next(void);
void set_effect_ok(void);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

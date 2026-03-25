
#ifndef __VOICE_STYLE_H_
#define __VOICE_STYLE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
void close_voice_style_modal(void);
void create_voice_style_modal(void);
void set_checkbox_index(uint8_t index);
void make_sure_ok(void);
void voice_style_management(void);
uint8_t get_currindex(void);
void set_voice_style_icon(lv_obj_t *handler);
bool get_play_switch(void);
//
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

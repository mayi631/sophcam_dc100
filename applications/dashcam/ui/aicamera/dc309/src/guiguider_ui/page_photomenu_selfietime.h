
#ifndef __PAGE_SETTING_SELFTIME_H_
#define __PAGE_SETTING_SELFTIME_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
    PHOTO_DELAY_NONE = 0,
    PHOTO_DELAY_5S,
    PHOTO_DELAY_7S,
    PHOTO_DELAY_10S,
} Photo_Delay_e;

void photoMenu_SelfieTime(lv_ui_t *ui);

uint8_t get_self_delay_time(void);

uint8_t get_self_index(void);

void set_self_index(uint8_t index);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

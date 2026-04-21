
#ifndef __PAGE_SETTINGEFFECT_H_
#define __PAGE_SETTINGEFFECT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
    EFFECT_NONE = 0, // 普通
    EFFECT_BLACKWHITE,
    EFFECT_OLDPIC,
    EFFECT_RED,
    EFFECT_GREEN,
    EFFECT_SUNSET,
    EFFECT_WARN,
    EFFECT_COOL,
    EFFECT_OVEREXP,
    EFFECT_INFRARD,
    EFFECT_TWOVAL,
    EFFECT_HIGHSTA,
    EFFECT_LOWSTA,
} Effect_Select_e;

void photoMenu_SettEffect(lv_ui_t *ui);

uint8_t geteffect_index(void);

void seteffect_index(uint8_t index);
void reset_effect(void);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

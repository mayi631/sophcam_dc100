
#ifndef __PAGE_AIMODE_H_
#define __PAGE_AIMODE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"
extern lv_obj_t *obj_Photo_AiMode_s; //底层窗口

typedef enum {
    AI_NONE = 0,
    AI_SCENE_CHANGE,
    AI_BG_CHANGE,
    // AI_AGE_CHANGE,
    AI_BEAUTY,
    AI_VOICE_CUSTOM,
} Photo_AIMode_e;

void photoMenu_AIMode(lv_ui_t *ui);
uint8_t AIModeSelect_GetMode(void); //获取设置的模式
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

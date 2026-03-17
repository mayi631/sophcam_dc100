
#ifndef __PAGE_SYS_LANGUAGE_H_
#define __PAGE_SYS_LANGUAGE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
LANGUAGE_ZH_CN  ,// 简体中文
LANGUAGE_ZH_TW  ,// 繁体中文  
LANGUAGE_JA     ,// 日语
LANGUAGE_EN     ,// 英语
LANGUAGE_PL     ,// 波兰语
LANGUAGE_NL     ,// 荷兰语
LANGUAGE_DE     ,// 德语
LANGUAGE_FR     ,// 法语
LANGUAGE_ES     ,// 西班牙语
LANGUAGE_IT     ,// 意大利语
LANGUAGE_TR     ,// 土耳其语
LANGUAGE_RU     ,// 俄语
LANGUAGE_PT     ,// 葡萄牙语
} sysMenu_language;

extern lv_obj_t *obj_sysMenu_Language_s;
void sysMenu_Language(lv_ui_t *ui);
uint8_t get_curr_language(void);
void setsysMenu_Language_Index(uint8_t index);
void setsysMenu_Language_Label(char* plabel);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

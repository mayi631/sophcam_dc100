
#ifndef __PAGE_SETTINGRES_H_
#define __PAGE_SETTINGRES_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

/* 如果要新增分辨率，注意同时更改 btn_labels 和 g_photo_size 变量*/
typedef enum {
    PHOTO_RES_64M = 0,
    PHOTO_RES_48M,
    PHOTO_RES_32M,
    PHOTO_RES_24M,
    PHOTO_RES_16M,
    PHOTO_RES_12M,
    PHOTO_RES_8M,
    PHOTO_RES_5M,
    PHOTO_RES_2M,
} photo_res_e;

void menuSetting_Resolution(lv_ui_t *ui);

uint8_t photo_getRes_Index(void);
void photo_setRes_Index(uint8_t index);
void photo_setRes_Label(const char* plabel);
char* photo_getRes_Label(void);
char* photo_getRes_Icon(void);
int get_photo_res_icon_index_by_width(int width);
const char* photo_getRes_IconByIndex(uint8_t index);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

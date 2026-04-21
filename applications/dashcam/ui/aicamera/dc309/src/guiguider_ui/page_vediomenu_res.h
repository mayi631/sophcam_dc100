
#ifndef __PAGE_RES_H
#define __PAGE_RES_H
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
    VIDEO_RES_4K = 0,
    VIDEO_RES_2_7K,
    VIDEO_RES_FULL,
    VIDEO_RES_HD,
} video_res_e;

extern lv_obj_t *obj_vedio_Res_s;

uint8_t video_getRes_Index(void);
void video_setRes_Index(uint8_t index);
void video_setRes_Label(const char* plabel);
char* video_getRes_Label(void);
char* video_getRes_Icon(void);
const char* video_getRes_IconByIndex(uint8_t index);
void vedioMenuSetting_Resolution(lv_ui_t *ui);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

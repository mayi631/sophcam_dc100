
#ifndef __PAGE_AI_TAKEPHOTO_H_
#define __PAGE_AI_TAKEPHOTO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_obj_t* page_ai_camera_s;   // AI拍照页面
void return_to_preview_with_image(const char *image_path);

void create_ai_camera_screen(lv_ui_t *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

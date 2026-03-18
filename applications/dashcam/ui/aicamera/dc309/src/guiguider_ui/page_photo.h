
#ifndef __PAGE_PHOTO_H_
#define __PAGE_PHOTO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

void Home_Photo(lv_ui_t *ui);

bool get_is_photo_back(void);

void set_is_photo_back(bool is_back);
void set_batter_image_index(int32_t index);

const char *get_ai_process_result_img_data(bool is_aiprocess);
// 显示取景框
void display_viewfinder(void);
// 取消取景框
void cancel_viewfinder(void);
void set_viewfinder_color(uint32_t color);

void hide_all_widgets(lv_obj_t *parent);
void restore_all_widgets(void);
#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

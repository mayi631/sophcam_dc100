
#ifndef __PAGE_VEDIO_TIMELAPSE_H_
#define __PAGE_VEDIO_TIMELAPSE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

// 按钮结构体定义
typedef struct {
    uint8_t index;
    lv_obj_t *btn;
    lv_obj_t *left_label;
    lv_obj_t *right_label;
    bool is_checkbox;
} timeLapseButton_t;

typedef enum {
    TIMED_COUNT_MODE = 0,   //拍照张数计算方式
    TIMED_PHOTO_GRAPHY, //定时开始/结束方式
} timeLapse_mode;

extern lv_obj_t *obj_Vedio_TimeLapse_s; //底层窗口

void vedioMenuSetting_Lapse(lv_ui_t *ui);

void check_and_start_timelapse(void);//到时间之后开始缩时录像

void set_use_scheduled_mode(timeLapse_mode mode);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

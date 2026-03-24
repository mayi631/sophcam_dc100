
#ifndef __PAGE_SWTTINGAUTOFOCUS_H_
#define __PAGE_SWTTINGAUTOFOCUS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

typedef enum {
    PHOTO_EFFECT = 0,    // 摄影效果
    PHOTO_RES,   // 分辨率
    PHOTO_WHI,       // 白平衡
    PHOTO_ISO,       // 感光度
    PHOTO_EXPOSE,    // 曝光设置
    // PHOTO_AIMODE,    // AI设置
    // PHOTO_PICMODE,   // 场景模式
    PHOTO_DELAY,     // 自拍时间
    PHOTO_SHOOTMODE, // 拍摄模式
    PHOTO_PICQUAL,   // 画质
    PHOTO_CURSOR,   // 光标
    // PHOTO_ANTISHAKE, // 防抖
    // PHOTO_AUTOFOCUS, // 自动对焦功能
    // PHOTO_FACEDEC,   // 人脸侦测功能
    // PHOTO_SMILEDEC,  // 笑脸拍照
    // PHOTO_BEAUTY,    // 美颜功能
    // PHOTO_FLASH, // 闪光灯
} Photo_Select_Item_e;

void photoMenu_Setting(lv_ui_t *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

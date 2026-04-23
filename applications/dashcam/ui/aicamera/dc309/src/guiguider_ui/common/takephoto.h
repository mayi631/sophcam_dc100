#ifndef TAKEPHOTO_H
#define TAKEPHOTO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdint.h>

typedef enum {
    VEDIO_STOP = 0,
    VEDIO_START,
} Vedio_Mode_e;

// 分辨率对应图标
typedef struct _res_to_icon_t{
    int32_t w;
    int32_t h;
    char *icon_c;
}res_to_icon_t;

// 最大16级zoom radio
#define ZOOM_RADIO_MAX 20

// 拍照后处理回调函数类型定义
typedef void (*takephoto_callback_t)(void);

// 拍照按键处理回调函数
void takephoto_key_handler(int key_code, int key_value);

// 延时拍照(自拍时间)时拍照按键处理函数
void takephoto_delay_handler(int key_code, int key_value);

// 注册拍照后处理回调函数
void takephoto_register_callback(takephoto_callback_t callback);

// 取消注册拍照后处理回调函数
void takephoto_unregister_callback(void);

// 显示SD卡未就绪提示标签
void sd_card_status_label(void);

// 计算剩余可拍照片数量, 根据最大分辨率估算
uint32_t photo_CalculateRemainingPhotoCount(void);
// 计算剩余可录像时间, 根据最大分辨率估算
char *  video_Calculateremainingvideo(void);

// 获取录像状态
void video_get_status(uint8_t* status);

//拍照之前回调注册
void takephoto_register_before_callback(takephoto_callback_t callback);
//取消注册
void takephoto_unregister_before_callback(void);

// 取消对焦
void takephoto_cancel_focus(void);

// ==================== 取景框相关函数 ====================
// 创建取景框
void create_viewfinder(lv_obj_t *parent);
// 显示取景框
void display_viewfinder(void);
// 设置取景框颜色（对焦成功时变为绿色）
void set_viewfinder_color(uint32_t color);
// 取消/隐藏取景框
void cancel_viewfinder(void);
//删除取景框
void delete_viewfinder(void);
// 打开focus功能
void enable_focus(void);
// 关闭focus功能
void disable_focus(void);
#ifdef __cplusplus
}
#endif

#endif /* TAKEPHOTO_H */

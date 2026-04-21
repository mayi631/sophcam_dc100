#ifndef __ICON_SELECT_POPUP_H_
#define __ICON_SELECT_POPUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "gui_guider.h"

// 选择项结构体
typedef struct {
    const char *icon;     // 图标名称
    const char *label;    // 标签文本
} icon_select_item_t;

// 选择类型枚举
typedef enum {
    ICON_SELECT_RESOLUTION,       // 拍照分辨率
    ICON_SELECT_REDLIGHT,         // 红外灯亮度
    ICON_SELECT_BRIGHTNESS,       // 屏幕亮度
    ICON_SELECT_SHOOTMODE,        // 连拍模式
    ICON_SELECT_VIDEO_RESOLUTION, // 录像分辨率
} icon_select_type_t;

// 删除弹窗
void delete_icon_select_popup(void);

// 检查弹窗是否已存在
bool is_icon_select_popup_exists(void);

// 重置透明度动画
void reset_icon_select_opa_anim(void);

// 创建通用图标选择弹窗
// parent: 父对象
// type: 选择类型
// items: 选项数组
// item_count: 选项数量
// current_index: 当前选中索引
// on_select_cb: 选择回调 (index, user_data)
void create_icon_select_popup(lv_obj_t *parent, icon_select_type_t type, 
                              const icon_select_item_t *items, uint32_t item_count,
                              uint32_t current_index,
                              void (*on_select_cb)(uint32_t index, void *user_data),
                              void *user_data);

// 选择上一个
void icon_select_prev(void);

// 选择下一个
void icon_select_next(void);

// 确认选择
void icon_select_confirm(void);

#ifdef __cplusplus
}
#endif

#endif /* __ICON_SELECT_POPUP_H_ */

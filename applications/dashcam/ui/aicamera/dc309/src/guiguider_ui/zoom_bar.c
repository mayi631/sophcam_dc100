
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"
#include "ui_common.h"
#include <time.h>
#include "indev.h"
#include "common/takephoto.h"
#include "indev.h"

typedef struct zoom_bar {
    lv_obj_t *cont;         // 缩放等级容器
    lv_obj_t *label;        // 缩放等级标签
    lv_obj_t *bar;          // 缩放等级进度条
    lv_timer_t *timer;      // 缩放隐藏定时器
    uint32_t level;         // 当前缩放等级
} zoom_bar_t;

static zoom_bar_t zoom_bar = {
    .cont = NULL,
    .label = NULL,
    .bar = NULL,
    .timer = NULL,
    .level = 1,
};

void hide_zoom_bar(void)
{
    // 隐藏缩放等级显示
    if (lv_obj_is_valid(zoom_bar.cont))
        lv_obj_add_flag(zoom_bar.cont, LV_OBJ_FLAG_HIDDEN);
}

uint32_t get_zoom_level()
{
    return zoom_bar.level;
}

void set_zoom_level(uint32_t level)
{
    zoom_bar.level = level;
}

// 缩放隐藏定时器回调
static void zoom_hide_timer_cb(lv_timer_t *timer)
{
    // 隐藏缩放等级显示
    lv_obj_add_flag(zoom_bar.cont, LV_OBJ_FLAG_HIDDEN);
    // 删除定时器
    lv_timer_del(zoom_bar.timer);
    zoom_bar.timer = NULL;
}

void delete_zoombar_timer_handler(void)
{
    if(zoom_bar.timer) {
        lv_timer_del(zoom_bar.timer);
        zoom_bar.timer = NULL;
    }
}

/*
 * zoom_bar.cont 的生存周期跟随父对象， zoom_bar.cont 会在父对象销毁时一起销毁
 * zoom_bar.timer 的生存周期则是独立于父对象的，会自动在执行完一次后销毁。
 */
void create_zoom_bar(lv_obj_t *parent)
{
    // 初始化缩放等级为1
    zoom_bar.level = 1;

    if(zoom_bar.cont != NULL && lv_obj_is_valid(zoom_bar.cont)) {
        lv_obj_del(zoom_bar.cont);
        zoom_bar.cont = NULL;
        MLOG_WARN("zoom_bar有效，删除后重新创建");
    }

    // 创建容器
    zoom_bar.cont = lv_obj_create(parent);
    lv_obj_set_size(zoom_bar.cont, 50, 200); // 宽度50，高度200（竖向）
    lv_obj_align(zoom_bar.cont, LV_ALIGN_RIGHT_MID, -20, 0); // 右侧居中，距离右边20像素
    lv_obj_set_style_bg_opa(zoom_bar.cont, LV_OPA_20, 0);
    lv_obj_set_style_bg_color(zoom_bar.cont, lv_color_hex(0x808080), 0);
    lv_obj_set_style_border_width(zoom_bar.cont, 0, 0);
    lv_obj_set_style_radius(zoom_bar.cont, 10, 0);
    lv_obj_add_flag(zoom_bar.cont, LV_OBJ_FLAG_HIDDEN); // 初始隐藏
    lv_obj_set_style_pad_all(zoom_bar.cont, 0, 0);
    // 无边框
    lv_obj_set_style_border_width(zoom_bar.cont, 0, 0);
    // 禁用滚动
    lv_obj_set_scrollbar_mode(zoom_bar.cont, LV_SCROLLBAR_MODE_OFF);
    //添加阴影效果
    lv_obj_set_style_shadow_width(zoom_bar.cont, 10, 0);
    lv_obj_set_style_shadow_color(zoom_bar.cont, lv_color_white(), 0);
    lv_obj_set_style_shadow_opa(zoom_bar.cont, LV_OPA_30, 0);

    // 创建进度条（竖向）
    zoom_bar.bar = lv_bar_create(zoom_bar.cont);
    lv_obj_set_size(zoom_bar.bar, 10, 180); // 宽度10，高度180（竖向）
    lv_bar_set_mode(zoom_bar.bar, LV_ARC_MODE_NORMAL); // 设置为垂直方向
    lv_obj_align(zoom_bar.bar, LV_ALIGN_CENTER, 0, 0); // 在容器中居中
    lv_bar_set_range(zoom_bar.bar, 1, 16);
    lv_bar_set_value(zoom_bar.bar, zoom_bar.level, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(zoom_bar.bar, lv_color_hex(0x444444), LV_PART_MAIN);
    lv_obj_set_style_bg_color(zoom_bar.bar, lv_color_hex(0x0080FF), LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(zoom_bar.bar, 0, 0);

    // 创建标签
    zoom_bar.label = lv_label_create(zoom_bar.cont);
    lv_label_set_text_fmt(zoom_bar.label, "%dx", zoom_bar.level);
    lv_obj_set_style_text_font(zoom_bar.label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);
    lv_obj_set_style_text_color(zoom_bar.label, lv_color_white(), 0);
    lv_obj_set_style_pad_all(zoom_bar.label, 0, 0);
    lv_obj_align(zoom_bar.label, LV_ALIGN_CENTER, 0,0); // 在容器底部居中
}


// 更新缩放等级显示
void update_zoom_bar(uint32_t level)
{
    zoom_bar.level = level;
    // 更新进度条值
    lv_bar_set_value(zoom_bar.bar, zoom_bar.level, LV_ANIM_OFF);
    // 更新标签文本
    lv_label_set_text_fmt(zoom_bar.label, "%dx", zoom_bar.level);
    // 显示容器
    lv_obj_clear_flag(zoom_bar.cont, LV_OBJ_FLAG_HIDDEN);

    // 重置隐藏定时器
    if(zoom_bar.timer) {
        lv_timer_del(zoom_bar.timer);
        zoom_bar.timer = NULL;
    }
    // 创建新的3秒定时器
    zoom_bar.timer = lv_timer_create(zoom_hide_timer_cb, 3000, NULL);
}

void delete_zoom_bar(void)
{
    if(zoom_bar.cont != NULL && lv_obj_is_valid(zoom_bar.cont)) {
        lv_obj_del(zoom_bar.cont);
        zoom_bar.cont = NULL;
    }
}
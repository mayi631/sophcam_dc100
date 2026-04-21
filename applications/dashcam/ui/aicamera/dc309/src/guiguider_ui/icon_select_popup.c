#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"
#include "ui_common.h"
#include "icon_select_popup.h"
#include "indev.h"

extern lv_style_t ttf_font_16;
extern bool is_video_mode;

// 弹窗容器
static lv_obj_t *icon_select_cont_s = NULL;
static lv_anim_t anim_icon_select_hide_s;  // 动画渐隐句柄
static uint8_t icon_select_opa_s = 0;

// 选择状态
static uint32_t g_current_select_index = 0;
static uint32_t g_item_count = 0;
static icon_select_type_t g_select_type = ICON_SELECT_RESOLUTION;

// 回调函数
static void (*g_on_select_callback)(uint32_t index, void *user_data) = NULL;
static void *g_user_data = NULL;

// 选项数组
static const icon_select_item_t *g_items = NULL;

// 选项点击回调
static void icon_select_item_click_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn_clicked = lv_event_get_target(e);
        lv_obj_t *parent = lv_obj_get_parent(btn_clicked);
        
        uint32_t clicked_index = 0;
        for (uint32_t i = 0; i < g_item_count; i++) {
            if (btn_clicked == lv_obj_get_child(parent, i)) {
                clicked_index = i;
                break;
            }
        }
        
        // 更新选中状态
        for (uint32_t i = 0; i < g_item_count; i++) {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if (i == clicked_index) {
                lv_obj_add_state(child, LV_STATE_CHECKED);
                lv_obj_set_style_border_width(child, 2, LV_STATE_CHECKED);
                lv_obj_set_style_border_color(child, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
            } else {
                lv_obj_clear_state(child, LV_STATE_CHECKED);
                lv_obj_set_style_border_width(child, 0, LV_PART_MAIN);
            }
        }
        
        g_current_select_index = clicked_index;
        
        // 触发回调
        if (g_on_select_callback != NULL) {
            g_on_select_callback(clicked_index, g_user_data);
        }
        
        // 关闭弹窗
        delete_icon_select_popup();
    }
}

// 滑动事件处理
static void icon_select_scroll_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SCROLL_BEGIN || code == LV_EVENT_SCROLL || code == LV_EVENT_SCROLL_END) {
        // 检查容器是否有效
        if (icon_select_cont_s == NULL || !lv_obj_is_valid(icon_select_cont_s)) {
            return;
        }
        // 重置透明度
        lv_obj_set_style_opa(icon_select_cont_s, icon_select_opa_s, 0);
        // 停止当前动画
        lv_anim_del(anim_icon_select_hide_s.var, anim_icon_select_hide_s.exec_cb);
        // 重置并启动新动画
        anim_icon_select_hide_s.act_time = 0;
        lv_anim_set_delay(&anim_icon_select_hide_s, 5000);
        lv_anim_start(&anim_icon_select_hide_s);
    }
}

// 动画回调
static void anim_icon_select_set_opa(void *var, int32_t v)
{
    lv_obj_set_style_opa(var, v, 0);
}

// 动画完成回调
static void anim_icon_select_complete_cb(lv_anim_t *a)
{
    delete_icon_select_popup();
}

// 删除弹窗
void delete_icon_select_popup(void)
{
    if (icon_select_cont_s != NULL && lv_obj_is_valid(icon_select_cont_s)) {
        lv_obj_del(icon_select_cont_s);
        icon_select_cont_s = NULL;
    }
    lv_anim_del(anim_icon_select_hide_s.var, anim_icon_select_hide_s.exec_cb);
    memset(&anim_icon_select_hide_s, 0, sizeof(lv_anim_t));
    g_on_select_callback = NULL;
    g_user_data = NULL;
}

// 检查弹窗是否存在
bool is_icon_select_popup_exists(void)
{
    return (icon_select_cont_s != NULL && lv_obj_is_valid(icon_select_cont_s));
}

// 重置透明度动画
void reset_icon_select_opa_anim(void)
{
    if (icon_select_cont_s == NULL || !lv_obj_is_valid(icon_select_cont_s)) {
        return;
    }
    lv_obj_set_style_opa(icon_select_cont_s, icon_select_opa_s, 0);
    lv_anim_del(anim_icon_select_hide_s.var, anim_icon_select_hide_s.exec_cb);
    anim_icon_select_hide_s.act_time = 0;
    lv_anim_set_delay(&anim_icon_select_hide_s, 5000);
    lv_anim_start(&anim_icon_select_hide_s);
}

// 创建通用图标选择弹窗
void create_icon_select_popup(lv_obj_t *parent, icon_select_type_t type,
                              const icon_select_item_t *items, uint32_t item_count,
                              uint32_t current_index,
                              void (*on_select_cb)(uint32_t index, void *user_data),
                              void *user_data)
{
    // 如果已存在，先删除
    if (is_icon_select_popup_exists()) {
        delete_icon_select_popup();
    }
    
    // 保存参数
    g_select_type = type;
    g_items = items;
    g_item_count = item_count;
    g_current_select_index = current_index;
    g_on_select_callback = on_select_cb;
    g_user_data = user_data;
    
    // 创建弹窗容器
    icon_select_cont_s = lv_obj_create(parent);
    lv_obj_set_size(icon_select_cont_s, 440, 90);
    lv_obj_align(icon_select_cont_s, LV_ALIGN_BOTTOM_MID, 0, -58);
    lv_obj_set_style_bg_opa(icon_select_cont_s, LV_OPA_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(icon_select_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(icon_select_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(icon_select_cont_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(icon_select_cont_s, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(icon_select_cont_s, 0, 0);
    
    // 添加滚动事件
    lv_obj_add_event_cb(icon_select_cont_s, icon_select_scroll_cb,
                        LV_EVENT_SCROLL_BEGIN | LV_EVENT_SCROLL | LV_EVENT_SCROLL_END, NULL);
    
    // 设置横向滚动
    lv_obj_set_scroll_dir(icon_select_cont_s, LV_DIR_HOR);
    
    // 计算按钮宽度（自适应数量）
    uint32_t btn_width = 76;
    uint32_t spacing = 6;
    
    // 创建选项按钮
    for (uint32_t i = 0; i < item_count; i++) {
        lv_obj_t *btn = lv_btn_create(icon_select_cont_s);
        lv_obj_set_size(btn, btn_width, 90);
        lv_obj_align(btn, LV_ALIGN_LEFT_MID, (btn_width + spacing) * i, 0);
        lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN);
        lv_obj_add_event_cb(btn, icon_select_item_click_cb, LV_EVENT_ALL, NULL);
        
        // 设置选中状态
        if (i == current_index) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_border_width(btn, 2, LV_STATE_CHECKED);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_STATE_CHECKED);
            lv_obj_scroll_to_view(btn, LV_ANIM_ON);
        }
        
        // 创建图标
        lv_obj_t *img = lv_img_create(btn);
        lv_obj_set_size(img, btn_width, 68);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_pad_all(img, 0, LV_STATE_DEFAULT);
        if (items[i].icon != NULL) {
            show_image(img, items[i].icon);
        }
        
        // 创建标签
        lv_obj_t *label = lv_label_create(btn);
        lv_obj_add_style(label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 70);
        if (items[i].label != NULL) {
            lv_label_set_text(label, items[i].label);
        } else {
            lv_label_set_text(label, "");
        }
        lv_obj_set_size(label, LV_PCT(100), 20);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    
    // 创建渐隐动画
    icon_select_opa_s = lv_obj_get_style_opa(icon_select_cont_s, LV_PART_MAIN);
    lv_anim_init(&anim_icon_select_hide_s);
    lv_anim_set_var(&anim_icon_select_hide_s, icon_select_cont_s);
    lv_anim_set_values(&anim_icon_select_hide_s, LV_OPA_30, LV_OPA_10);
    lv_anim_set_time(&anim_icon_select_hide_s, 2000);
    lv_anim_set_exec_cb(&anim_icon_select_hide_s, anim_icon_select_set_opa);
    lv_anim_set_path_cb(&anim_icon_select_hide_s, lv_anim_path_ease_out);
    lv_anim_set_completed_cb(&anim_icon_select_hide_s, anim_icon_select_complete_cb);
    lv_anim_set_delay(&anim_icon_select_hide_s, 5000);
    lv_anim_start(&anim_icon_select_hide_s);
}

// 选择上一个
void icon_select_prev(void)
{
    if (icon_select_cont_s == NULL || !lv_obj_is_valid(icon_select_cont_s)) {
        return;
    }
    
    uint32_t btn_count = lv_obj_get_child_cnt(icon_select_cont_s);
    if (btn_count == 0) return;
    
    // 清除上一个按钮的选中样式
    lv_obj_t *prev_btn = lv_obj_get_child(icon_select_cont_s, g_current_select_index);
    if (prev_btn) {
        lv_obj_clear_state(prev_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(prev_btn, 0, LV_PART_MAIN);
    }
    
    // 计算新索引
    if (g_current_select_index == 0) {
        g_current_select_index = btn_count - 1;
    } else {
        g_current_select_index--;
    }
    
    // 设置新按钮的选中样式
    lv_obj_t *new_btn = lv_obj_get_child(icon_select_cont_s, g_current_select_index);
    if (new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
    
    reset_icon_select_opa_anim();
}

// 选择下一个
void icon_select_next(void)
{
    if (icon_select_cont_s == NULL || !lv_obj_is_valid(icon_select_cont_s)) {
        return;
    }
    
    uint32_t btn_count = lv_obj_get_child_cnt(icon_select_cont_s);
    if (btn_count == 0) return;
    
    // 清除上一个按钮的选中样式
    lv_obj_t *prev_btn = lv_obj_get_child(icon_select_cont_s, g_current_select_index);
    if (prev_btn) {
        lv_obj_clear_state(prev_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(prev_btn, 0, LV_PART_MAIN);
    }
    
    // 计算新索引
    g_current_select_index++;
    if (g_current_select_index >= btn_count) {
        g_current_select_index = 0;
    }
    
    // 设置新按钮的选中样式
    lv_obj_t *new_btn = lv_obj_get_child(icon_select_cont_s, g_current_select_index);
    if (new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
    
    reset_icon_select_opa_anim();
}

// 确认选择
void icon_select_confirm(void)
{
    if (g_on_select_callback != NULL) {
        g_on_select_callback(g_current_select_index, g_user_data);
    }
    delete_icon_select_popup();
}

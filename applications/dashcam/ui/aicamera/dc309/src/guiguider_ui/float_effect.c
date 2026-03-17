#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
// #include "events_init.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"
#include <time.h>
#include "ui_common.h"
#include "common/takephoto.h"
#include "indev.h"

static lv_obj_t *effect_scroll_cont_s;  // 特效滚动控件
extern const char *effect_style[];      // 特效图片数组
extern const char *effect_style_small[];

static uint8_t float_effect_opa_s = 0;  // 初始透明度
static lv_anim_t anim_gradually_hide_s; // 动画渐隐句柄
// static lv_obj_t *img_effect_s = NULL;   // 特效图标
extern lv_style_t ttf_font_16;
extern bool is_video_mode;
extern char g_button_labelPho[32];
uint32_t g_current_effect_index = 0;   //焦点

static void set_effect_currIndex_focus(int index);
// 滑动时间
static void scrolling_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // MLOG_DBG("%s...\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_SCROLL_BEGIN:
        case LV_EVENT_SCROLL:
        case LV_EVENT_SCROLL_END: {
            // 重置透明度
            lv_obj_set_style_opa(effect_scroll_cont_s, float_effect_opa_s, 0);
            // 停止当前动画
            lv_anim_del(anim_gradually_hide_s.var, anim_gradually_hide_s.exec_cb);
            // 重置并启动新动画
            anim_gradually_hide_s.act_time = 0;
            // 延迟启动
            lv_anim_set_delay(&anim_gradually_hide_s, 8000);
            lv_anim_start(&anim_gradually_hide_s);
        }; break;

        default: break;
    }
}

// 动画回调,设置透明度
void anim_objSet_Opa(void *var, int32_t v)
{
    lv_obj_set_style_opa(var, v, 0);
}

bool get_is_effect_exist(void)
{
    if(effect_scroll_cont_s != NULL && lv_obj_is_valid(effect_scroll_cont_s)) {
        return true;
    }
    return false;
}

void reset_opa_anim(void)
{
    // 重置透明度
    lv_obj_set_style_opa(effect_scroll_cont_s, float_effect_opa_s, 0);
    // 停止当前动画
    lv_anim_del(anim_gradually_hide_s.var, anim_gradually_hide_s.exec_cb);
    // 重置并启动新动画
    anim_gradually_hide_s.act_time = 0;
    // 延迟启动
    lv_anim_set_delay(&anim_gradually_hide_s, 8000);
    lv_anim_start(&anim_gradually_hide_s);
}

// 创建渐隐动画
void create_gradually_hide_anim(lv_anim_completed_cb_t completed_cb, uint32_t time)
{
    // 创建透明度动画
    lv_anim_init(&anim_gradually_hide_s);
    lv_anim_set_var(&anim_gradually_hide_s, effect_scroll_cont_s);
    float_effect_opa_s = lv_obj_get_style_opa(effect_scroll_cont_s, LV_PART_MAIN);
    lv_anim_set_values(&anim_gradually_hide_s, lv_obj_get_style_opa(effect_scroll_cont_s, LV_PART_MAIN), LV_OPA_10);

    lv_anim_set_time(&anim_gradually_hide_s, time);

    lv_anim_set_exec_cb(&anim_gradually_hide_s, anim_objSet_Opa);
    lv_anim_set_path_cb(&anim_gradually_hide_s, lv_anim_path_ease_out);

    // 设置动画完成回调（销毁对象）
    lv_anim_set_completed_cb(&anim_gradually_hide_s, completed_cb);

    // 启动动画
    lv_anim_set_delay(&anim_gradually_hide_s, 8000);
    lv_anim_start(&anim_gradually_hide_s);
}

void delete_all_handle(void)
{
    if(effect_scroll_cont_s != NULL && lv_obj_is_valid(effect_scroll_cont_s)) {
        lv_obj_del(effect_scroll_cont_s);
        effect_scroll_cont_s = NULL;
    }

    if(!lv_obj_is_valid(effect_scroll_cont_s) &&
       effect_scroll_cont_s != NULL) { // 意外状况，界面先销毁，子控件已经不存在了，但是控件指针不为空
        effect_scroll_cont_s = NULL;
    }

    lv_anim_del(anim_gradually_hide_s.var, anim_gradually_hide_s.exec_cb);
    memset(&anim_gradually_hide_s, 0, sizeof(lv_anim_t));
}

// 特效选择回调
static void effect_select_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // MLOG_DBG("%s...\n",lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);
            lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); // 获取发生点击事件的父控件

            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                if(btn_clicked == lv_obj_get_child(parent, i)) {
                    MESSAGE_S event = {0};
                    event.topic     = EVENT_MODEMNG_SETTING;
                    if(is_video_mode == false) {
                        event.arg1 = PARAM_MENU_PHOTO_EFFECT;
                    } else if(is_video_mode == true) {
                        event.arg1 = PARAM_MENU_VIDEO_EFFECT;
                    }
                    event.arg2 = i;
                    MODEMNG_SendMessage(&event);
                    seteffect_index(i);//和菜单关联
                    // show_image(img_effect_s, effect_style_small[geteffect_index()]);//设置图片
                    set_effect_currIndex_focus(i);
                    strncpy(g_button_labelPho, lv_label_get_text(lv_obj_get_child(btn_clicked,1)), sizeof(g_button_labelPho));
                }
            }
            reset_opa_anim();
        }; break;

        default: break;
    }
}

// 创建特效选择小窗
void float_effect_creat(lv_obj_t *img_handel, lv_obj_t *parent)
{
    // img_effect_s = img_handel;

    effect_scroll_cont_s = lv_obj_create(parent);
    lv_obj_set_size(effect_scroll_cont_s, 440, 90);
    lv_obj_align(effect_scroll_cont_s, LV_ALIGN_BOTTOM_MID, 0, -58);
    lv_obj_set_style_bg_opa(effect_scroll_cont_s, LV_OPA_20, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_shadow_width(effect_scroll_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(effect_scroll_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(effect_scroll_cont_s, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条，默认 LV_SCROLLBAR_MODE_AUTO
    lv_obj_set_style_bg_color(effect_scroll_cont_s, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(effect_scroll_cont_s, 0, 0);
    lv_obj_add_event_cb(effect_scroll_cont_s, scrolling_event_cb,
                        LV_EVENT_SCROLL_BEGIN | LV_EVENT_SCROLL | LV_EVENT_SCROLL_END, NULL);

    // 设置滚动方向为横向（左右）
    lv_obj_set_scroll_dir(effect_scroll_cont_s, LV_DIR_HOR);

    // // 创建功能按钮
    const char* btn_labels[] = {
        str_language_normal[get_curr_language()],
        str_language_black_and_white[get_curr_language()],
        str_language_film[get_curr_language()],
        str_language_vivid[get_curr_language()],
        str_language_green[get_curr_language()],
        str_language_infrared[get_curr_language()],
        str_language_high_saturation[get_curr_language()],
        str_language_low_saturation[get_curr_language()],
    };

    // 在容器内部创建多个对象，使其宽度超出容器
    for(uint32_t i = 0; i < sizeof(btn_labels) / sizeof(btn_labels[0]); i++) {
        lv_obj_t *btn = lv_btn_create(effect_scroll_cont_s);
        lv_obj_set_size(btn, 76, 90);
        lv_obj_align(btn, LV_ALIGN_LEFT_MID, (76 + 6) * i, 0);
        lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(btn, effect_select_event_cb, LV_EVENT_ALL, NULL);

        if(i == g_current_effect_index) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_border_width(btn, 2, LV_STATE_CHECKED);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_STATE_CHECKED);
            lv_obj_scroll_to_view(btn, LV_ANIM_ON);
        }

        lv_obj_t *img = lv_img_create(btn);
        lv_obj_set_size(img, 76, 68);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_pad_all(img, 0, LV_STATE_DEFAULT);
        show_image(img, effect_style[i]);

        lv_obj_t *label = lv_label_create(btn);
        lv_obj_add_style(label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 70);
        lv_label_set_text_fmt(label, "%s", btn_labels[i]);
        lv_obj_set_size(label, LV_PCT(100), 20);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL); // 过长显示省略号
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}


/**
 * @brief 选择上一个按钮（向左选择）
 */
void effect_Select_prev(void)
{
    if(effect_scroll_cont_s == NULL || !lv_obj_is_valid(effect_scroll_cont_s)) {
        return;
    }
    
    int btn_count = lv_obj_get_child_cnt(effect_scroll_cont_s);
    if(btn_count == 0) return;
    
    // 清除上一个按钮的选中样式
    lv_obj_t *prev_btn = lv_obj_get_child(effect_scroll_cont_s, g_current_effect_index);
    if(prev_btn) {
        lv_obj_clear_state(prev_btn, LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(prev_btn, LV_OPA_0, LV_PART_MAIN);
    }
    
    // 计算新索引
    g_current_effect_index--;
    if(g_current_effect_index < 0) {
        g_current_effect_index = btn_count - 1;
    }
    
    // 设置新按钮的选中样式
    lv_obj_t *new_btn = lv_obj_get_child(effect_scroll_cont_s, g_current_effect_index);
    if(new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(new_btn, LV_OPA_0, LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
    reset_opa_anim();
}

/**
 * @brief 选择下一个按钮（向右选择）
 */
void effect_AISelect_next(void)
{
    if(effect_scroll_cont_s == NULL || !lv_obj_is_valid(effect_scroll_cont_s)) {
        return;
    }
    
    uint32_t btn_count = lv_obj_get_child_cnt(effect_scroll_cont_s);
    if(btn_count == 0) return;
    
    // 清除上一个按钮的选中样式
    lv_obj_t *prev_btn = lv_obj_get_child(effect_scroll_cont_s, g_current_effect_index);
    if(prev_btn) {
        lv_obj_clear_state(prev_btn, LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(prev_btn, LV_OPA_0, LV_PART_MAIN);
    }
    
    // 计算新索引
    g_current_effect_index++;
    if(g_current_effect_index >= btn_count) {
        g_current_effect_index = 0;
    }
    
    // 设置新按钮的选中样式
    lv_obj_t *new_btn = lv_obj_get_child(effect_scroll_cont_s, g_current_effect_index);
    if(new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(new_btn, LV_OPA_0, LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }

    reset_opa_anim();
}

static void set_effect_currIndex_focus(int index)
{
    if(effect_scroll_cont_s == NULL || !lv_obj_is_valid(effect_scroll_cont_s)) {
        return;
    }
    g_current_effect_index = index;
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(effect_scroll_cont_s); i++) {
        lv_obj_t *btn = lv_obj_get_child(effect_scroll_cont_s, i);
        if(btn != NULL) lv_obj_clear_state(btn, LV_STATE_CHECKED);
    }
    lv_obj_t *new_btn = lv_obj_get_child(effect_scroll_cont_s, g_current_effect_index);
    if(new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(new_btn, LV_OPA_0, LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
}

void set_effect_ok(void)
{
    MESSAGE_S event = {0};
    event.topic     = EVENT_MODEMNG_SETTING;
    if(is_video_mode == false) {
        event.arg1 = PARAM_MENU_PHOTO_EFFECT;
    } else if(is_video_mode == true) {
        event.arg1 = PARAM_MENU_VIDEO_EFFECT;
    }
    event.arg2 = g_current_effect_index;
    MODEMNG_SendMessage(&event);
    seteffect_index(g_current_effect_index);                                              // 和菜单关联
    // show_image(img_effect_s, effect_style_small[geteffect_index()]); // 设置图片

    reset_opa_anim();
}
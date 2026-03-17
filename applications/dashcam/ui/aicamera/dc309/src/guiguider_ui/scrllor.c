#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "page_all.h"
// #include "events_init.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"

// 屏幕尺寸定义
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

// 全局变量
lv_obj_t *content;    // 内容容器
lv_obj_t *page_label; // 页码标签
int current_page = 0; // 当前页码
int total_pages  = 4; // 总页数

// 翻页动画回调
static void scroll_anim_cb(void *var, int32_t value)
{
    // 更新内容容器的Y位置
    lv_obj_set_y(var, value);
}

// 翻页函数
static void scroll_to_page(int page_index)
{
    // 边界检查（确保页码有效）
    if(page_index < 0) page_index = 0;
    if(page_index >= total_pages) page_index = total_pages - 1;

    // 跳过当前页面
    if(page_index == current_page) return;

    // 计算目标Y坐标（每页高度为屏幕高度）
    int32_t target_y = -page_index * SCREEN_HEIGHT;

    // 创建并配置动画
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, content);
    lv_anim_set_values(&anim, lv_obj_get_y(content), target_y);
    lv_anim_set_time(&anim, 400);                      // 动画时长400ms
    lv_anim_set_playback_time(&anim, 0);               // 禁用回放
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out); // 缓出动画曲线
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)scroll_anim_cb);

    // 启动动画
    lv_anim_start(&anim);

    // 更新当前页码
    current_page = page_index;

    // 更新页码标签
    lv_label_set_text_fmt(page_label, "Page %d/%d", current_page + 1, total_pages);
}

// 创建单个页面
lv_obj_t *create_page(lv_obj_t *parent, int page_num, lv_color_t color)
{
    // 创建页面容器
    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_set_size(page, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(page, color, 0);
    lv_obj_set_style_border_width(page, 0, 0);
    lv_obj_set_style_pad_all(page, 0, 0);

    // 创建居中文本标签
    lv_obj_t *label = lv_label_create(page);
    lv_label_set_text_fmt(label, "Page %d\n\nLVGL v9.3 Page Flip Demo", page_num + 1);
    lv_obj_center(label);

    // 创建图片占位符
    lv_obj_t *img = lv_img_create(page);
    lv_obj_set_size(img, 200, 150);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 50);

    // 图片样式（无实际图片，用背景色代替）
    static lv_style_t img_style;
    lv_style_init(&img_style);
    lv_style_set_bg_color(&img_style, lv_color_darken(color, 100));
    lv_style_set_bg_opa(&img_style, LV_OPA_100);
    lv_style_set_radius(&img_style, 8);
    lv_obj_add_style(img, &img_style, 0);

    // 创建图片文字标签
    lv_obj_t *img_label = lv_label_create(img);
    lv_label_set_text(img_label, "Image\nPlaceholder");
    lv_obj_center(img_label);

    return page;
}

// 事件回调函数
static void event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj        = lv_event_get_current_target(e);
    const char *txt      = lv_label_get_text(lv_obj_get_child(obj, 0));

    if(code == LV_EVENT_CLICKED) {
        if(strstr(txt, "Previous")) {
            scroll_to_page(current_page - 1);
        } else if(strstr(txt, "Next")) {
            scroll_to_page(current_page + 1);
        }
    }
}

// 创建控制按钮
void create_control_buttons(lv_obj_t *parent)
{
    // 创建控制按钮容器
    lv_obj_t *btn_container = lv_obj_create(parent);
    lv_obj_set_size(btn_container, SCREEN_WIDTH, 60);
    lv_obj_set_flex_flow(btn_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_container, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_color(btn_container, lv_color_hex(0x2c2c2c), 0);
    lv_obj_set_style_border_width(btn_container, 0, 0);
    lv_obj_align(btn_container, LV_ALIGN_BOTTOM_MID, 0, 0);

    // 创建上一页按钮
    lv_obj_t *btn_prev = lv_btn_create(btn_container);
    lv_obj_set_size(btn_prev, 150, 40);
    lv_obj_add_event_cb(btn_prev, event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_prev_label = lv_label_create(btn_prev);
    lv_label_set_text(btn_prev_label, LV_SYMBOL_LEFT " Previous");

    // 创建页码标签
    page_label = lv_label_create(btn_container);
    lv_label_set_text_fmt(page_label, "Page %d/%d", current_page + 1, total_pages);
    lv_obj_set_style_text_font(page_label, &lv_font_montserrat_20, 0);

    // 创建下一页按钮
    lv_obj_t *btn_next = lv_btn_create(btn_container);
    lv_obj_set_size(btn_next, 150, 40);
    lv_obj_add_event_cb(btn_next, event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_next_label = lv_label_create(btn_next);
    lv_label_set_text(btn_next_label, "Next " LV_SYMBOL_RIGHT);

    // 按钮样式
    static lv_style_t btn_style;
    lv_style_init(&btn_style);
    lv_style_set_bg_color(&btn_style, lv_color_hex(0x476e91));
    lv_style_set_text_color(&btn_style, lv_color_white());
    lv_style_set_border_width(&btn_style, 0);
    lv_style_set_radius(&btn_style, 20);

    lv_obj_add_style(btn_prev, &btn_style, 0);
    lv_obj_add_style(btn_next, &btn_style, 0);
}

// 添加手势支持
static void screen_TakePhotoSetting_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        if(dir == LV_DIR_BOTTOM) {
            // 上滑：下一页
            scroll_to_page(current_page + 1);
        } else if(dir == LV_DIR_TOP) {
            // 下滑：上一页
            scroll_to_page(current_page - 1);
        }
    }
}

// 主函数
void lvgl_page_flip_demo()
{
    // 创建主容器
    lv_obj_t *main_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_container, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_align(main_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(main_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(main_container, lv_color_hex(0x121212), 0);
    lv_obj_set_style_pad_all(main_container, 0, 0);
    lv_obj_set_style_border_width(main_container, 0, 0);

    // 创建标题栏
    lv_obj_t *header = lv_obj_create(main_container);
    lv_obj_set_size(header, SCREEN_WIDTH, 60);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x476e91), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_t *header_label = lv_label_create(header);
    lv_label_set_text(header_label, "LVGL 9.3 Page Flip Demo");
    lv_obj_set_style_text_color(header_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(header_label, &lv_font_montserrat_24, 0);
    lv_obj_center(header_label);

    // 创建滚动区域容器
    lv_obj_t *scroll_area = lv_obj_create(main_container);
    lv_obj_set_size(scroll_area, SCREEN_WIDTH, SCREEN_HEIGHT - 120);
    lv_obj_set_style_border_width(scroll_area, 0, 0);
    lv_obj_set_style_pad_all(scroll_area, 0, 0);
    lv_obj_set_scrollbar_mode(scroll_area, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(scroll_area, LV_DIR_NONE); // 禁用原生滚动

    // 创建内容容器
    content = lv_obj_create(scroll_area);
    lv_obj_set_size(content, SCREEN_WIDTH, (SCREEN_HEIGHT - 120) * total_pages);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_align(content, LV_ALIGN_TOP_LEFT, 0, 0);

    // 创建页面
    static lv_color_t page_colors[4] = {0};

    page_colors[1] = lv_color_hex(0x476e91), page_colors[0] = lv_color_hex(0x4CAF50),
    page_colors[2] = lv_color_hex(0xFF9800), page_colors[3] = lv_color_hex(0x9C27B0);
    for(int i = 0; i < total_pages; i++) {
        create_page(content, i, page_colors[i]);
        lv_obj_align_to(lv_obj_get_child(content, i), content, LV_ALIGN_TOP_LEFT, 0, i * (SCREEN_HEIGHT - 120));
    }

    // 创建控制按钮
    create_control_buttons(main_container);

    lv_obj_add_event_cb(scroll_area, screen_TakePhotoSetting_btn_event_handler, LV_EVENT_GESTURE, NULL);
}

/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"
#include "config.h"
#include "img2img/img2img.h"
#include "common/extract_thumbnail.h"
#include "mlog.h"

// --- 图标点击事件回调 ---
static lv_obj_t *style_icon_btns[9]   = {NULL};
static lv_obj_t *style_icon_labels[9] = {NULL};
static lv_obj_t *style_icon_lines[9]  = {NULL};
static int style_icon_selected        = 0;
extern char pic_filepath[128];
static char pic_thumbnail[128] = {0};

static AIEffectPic_t *AIEffectPic;
// 字体样式对象
static lv_style_t label_result_style;

static void update_style_icon_selected(int idx)
{
    for(int i = 0; i < 9; ++i) {
        // 图片按钮样式
        if(i == idx) {
            // 选中状态 - 添加边框和阴影
            lv_obj_set_style_border_width(style_icon_btns[i], 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(style_icon_btns[i], lv_color_hex(0x00BFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(style_icon_btns[i], 15, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_color(style_icon_btns[i], lv_color_hex(0x00BFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_opa(style_icon_btns[i], 100, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_ofs_y(style_icon_btns[i], 3, LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            // 未选中状态 - 清除边框和阴影
            lv_obj_set_style_border_width(style_icon_btns[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_shadow_width(style_icon_btns[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
        // label颜色
        if(style_icon_labels[i])
            lv_obj_set_style_text_color(style_icon_labels[i],
                                        i == idx ? lv_color_hex(0x00BFFF) : lv_color_hex(0xCCCCCC), 0);
        // 下划线
        if(style_icon_lines[i])
            lv_obj_add_flag(style_icon_lines[i], LV_OBJ_FLAG_HIDDEN);
    }
    // 只显示当前选中的下划线
    if(style_icon_lines[idx])
        lv_obj_clear_flag(style_icon_lines[idx], LV_OBJ_FLAG_HIDDEN);
    style_icon_selected = idx;
    MLOG_DBG("style_icon_selected: %d\n", style_icon_selected);
}

static void style_icon_event_cb(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    int idx = (int)lv_event_get_user_data(e);
    update_style_icon_selected(idx);

    // 获取当前选中的风格类型
    const char *style_names[9] = {"无特效",   "网红日漫", "吉卜力", "印象画派", "粘土",
                                  "迪士尼3D", "新海诚",   "动漫",   "像素艺术"};

    // // 根据选中的风格类型映射到AIdemo支持的风格名称
    // const char *aigc_style_names[9] = {
    //     "", "RIMAN", "CUTE_CLAY", "MONET_GARDEN", "CUTE_CLAY", "CHINESE_INK", "MONET_GARDEN", "ANIME", "CUTE_DOLL"};

    // 检查是否有选中的风格
    if(style_icon_selected >= 0 && style_icon_selected < 9) {
        const char *selected_style = style_names[style_icon_selected];

        // 如果是"无特效"，直接返回
        if(strncmp(selected_style, "无特效", 4) == 0) {
            // 显示原图
            MLOG_DBG("选择了无特效，无需处理\n");
            lv_img_set_src(AIEffectPic->img_main, AI_EFFECT_TMP_IMAGE_PATH);
            return;
        }

        // 立即显示"正在处理"状态
        if(AIEffectPic->label_result != NULL) {
            MLOG_DBG("正在处理...\n");
            lv_obj_clear_flag(AIEffectPic->label_result, LV_OBJ_FLAG_HIDDEN); // 显示结果label
            lv_label_set_text(AIEffectPic->label_result, "正在处理...");
            lv_obj_set_style_text_color(AIEffectPic->label_result, lv_color_hex(0xFFFF00),
                                        LV_PART_MAIN | LV_STATE_DEFAULT); // 黄色表示正在处理
            // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
            lv_obj_invalidate(AIEffectPic->label_result);
            lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
            MLOG_DBG("强制UI刷新完成\n");
        }

        char style_prompt[1024] = {0};
        snprintf(style_prompt, sizeof(style_prompt), "将这张照片重绘成 %s 风格", selected_style);

        MLOG_DBG("开始处理图像风格转换，提升词: %s\n", style_prompt);
        // 调用AIdemo的aigc图像风格转换接口
        img2img_processor_t *processor = img2img_create(IMAGE_TO_IMAGE_ACCESS_KEY, IMAGE_TO_IMAGE_SECRET_KEY);
        // 设置处理参数
        img2img_params_t params = img2img_default_params();
        params.prompt           = style_prompt;
        // 宽和高必须指定，否则服务端会报错
        params.width  = 640;
        params.height = 480;
        // 处理图片
        int ret = img2img_process_file(processor, pic_thumbnail, &params, "/tmp/output.jpg");
        if(ret != 0) {
            MLOG_DBG("图像风格转换失败: %s\n", img2img_get_error_string(ret));
            // 显示错误信息
            if(AIEffectPic->label_result != NULL) {
                lv_label_set_text(AIEffectPic->label_result, "处理失败");
                lv_obj_set_style_text_color(AIEffectPic->label_result, lv_color_hex(0xFF0000),
                                            LV_PART_MAIN | LV_STATE_DEFAULT); // 红色表示失败
            }
        } else {
            MLOG_DBG("图像风格转换成功！\n");
            // 隐藏结果label
            lv_obj_add_flag(AIEffectPic->label_result, LV_OBJ_FLAG_HIDDEN);
            // 显示成功信息
            // if(AIEffectPic->label_result != NULL) {
            //     lv_label_set_text(AIEffectPic->label_result, "处理成功");
            //     lv_obj_set_style_text_color(AIEffectPic->label_result, lv_color_hex(0x00FF00),
            //                                 LV_PART_MAIN | LV_STATE_DEFAULT); // 绿色表示成功
            // }
        }
        img2img_destroy(processor);
    }

    if(AI_EFFECT_OUT_IMAGE_PATH) {
        // 检查图片文件是否存在
        const char *real_path = strchr(AI_EFFECT_OUT_IMAGE_PATH, '/');
        FILE *file            = fopen(real_path, "r");
        if(file != NULL) {
            fclose(file);
            MLOG_DBG("AI_EFFECT_OUT_IMAGE_PATH 文件存在: %s\n", AI_EFFECT_OUT_IMAGE_PATH);
            lv_img_set_src(AIEffectPic->img_main, AI_EFFECT_OUT_IMAGE_PATH);
        } else {
            MLOG_DBG("AI_EFFECT_OUT_IMAGE_PATH 文件不存在: %s\n", AI_EFFECT_OUT_IMAGE_PATH);
        }
    } else {
        MLOG_DBG("AI_EFFECT_OUT_IMAGE_PATH is not exist\n");
    }
}

// --- 返回按钮点击事件回调 ---
static void btn_back_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    ui_load_scr_animation(ui, &ui->page_aieffect.scr, ui->screen_AIEffect_del, &ui->screen_AIEffectPic_del,
                          setup_scr_screen_AIEffect, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

// --- 原图/特效切换按钮按下事件回调 ---
static void imgbtn_switch_pressed_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    MLOG_DBG("原图/特效切换按钮被按下 - 显示原图\n");
}

// --- 原图/特效切换按钮松开事件回调 ---
static void imgbtn_switch_released_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    MLOG_DBG("原图/特效切换按钮被松开 - 显示特效图\n");
}

// --- 确认按钮点击事件回调 ---
static void btn_ok_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    // lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    MLOG_DBG("确认按钮被点击\n");
    // 这里可以添加确认按钮的其他逻辑，比如保存处理后的图片等
}

// --- 取消按钮点击事件回调 ---
static void btn_cancel_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    // lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    MLOG_DBG("取消按钮被点击\n");
}

void setup_scr_screen_AIEffectPic(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIEffect...\n");

    AIEffectPic      = &ui->page_aieffectpic;
    AIEffectPic->del = true;

    // 创建主页面1 容器
    if(AIEffectPic->scr != NULL) {
        if(lv_obj_is_valid(AIEffectPic->scr)) {
            MLOG_DBG("page_AIEffectPic->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIEffectPic->scr);
        } else {
            MLOG_DBG("page_AIEffectPic->scr 已被自动销毁，仅重置指针\n");
        }
        AIEffectPic->scr = NULL;
    }

    // 初始化字体样式对象
    lv_style_init(&label_result_style);
    lv_style_set_opa(&label_result_style, 255);
    lv_style_set_text_color(&label_result_style, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&label_result_style, 255);
    lv_style_set_text_align(&label_result_style, LV_TEXT_ALIGN_CENTER);

    // The custom code of screen_AIEffectPic.
    // Write codes screen_AIEffectPic
    AIEffectPic->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIEffectPic->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIEffectPic->scr, LV_SCROLLBAR_MODE_OFF);

    // 显示图片
    AIEffectPic->img_main = lv_img_create(AIEffectPic->scr);
    lv_obj_set_pos(AIEffectPic->img_main, 0, 60);
    lv_obj_set_size(AIEffectPic->img_main, 640, 360);

    MLOG_DBG("show pic: %s\n", pic_filepath);
    char thumbnail_path_small[256];
    char thumbnail_path_large[256];
    char *filename = strrchr(pic_filepath, '/');
    if(strstr(filename, ".jpg")) {
        // 提取小缩略图到 PHOTO_ALBUM_IMAGE_PATH_S 目录下，图片名不变
        snprintf(thumbnail_path_small, sizeof(thumbnail_path_small), "%s%s", PHOTO_ALBUM_IMAGE_PATH_S, filename);
        // 提取大缩略图到 PHOTO_ALBUM_IMAGE_PATH_L目录下，图片名不变
        snprintf(thumbnail_path_large, sizeof(thumbnail_path_large), "%s%s", PHOTO_ALBUM_IMAGE_PATH_L, filename);
        char *real_path       = strchr(pic_filepath, '/');
        char *real_path_small = strchr(thumbnail_path_small, '/');
        char *real_path_large = strchr(thumbnail_path_large, '/');
        if(fopen(real_path_small, "rb") == NULL || fopen(real_path_large, "rb") == NULL) {
            extract_thumbnail(real_path, real_path_small, real_path_large);
        }
        strncpy(pic_thumbnail, real_path_large, sizeof(pic_thumbnail));
    }

    // 检查图片文件是否存在
    FILE *file = fopen(pic_thumbnail, "r");
    if(file != NULL) {
        fclose(file);
        char pic_path_forshow[128];
        snprintf(pic_path_forshow, sizeof(pic_path_forshow), "A:%s", pic_thumbnail);
        lv_img_set_src(AIEffectPic->img_main, pic_path_forshow);
    } else {
        MLOG_DBG("pic_thumbnail 文件不存在: %s\n", pic_thumbnail);
    }

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    // lv_obj_set_style_bg_opa(AIEffectPic->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(AIEffectPic->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont (底部边栏)
    AIEffectPic->bottom_cont = lv_obj_create(AIEffectPic->scr);
    lv_obj_set_pos(AIEffectPic->bottom_cont, 0, 400);
    lv_obj_set_size(AIEffectPic->bottom_cont, 640, 80);
    lv_obj_set_scrollbar_mode(AIEffectPic->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(AIEffectPic->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AIEffectPic->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(AIEffectPic->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIEffectPic->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffectPic->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffectPic->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffectPic->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont (顶部边栏)
    AIEffectPic->top_cont = lv_obj_create(AIEffectPic->scr);
    lv_obj_set_pos(AIEffectPic->top_cont, 0, 0);
    lv_obj_set_size(AIEffectPic->top_cont, 640, 40);
    lv_obj_set_scrollbar_mode(AIEffectPic->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIEffectPic->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffectPic->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIEffectPic->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffectPic->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (顶部返回按钮)
    AIEffectPic->btn_back = lv_btn_create(AIEffectPic->top_cont);
    lv_obj_set_pos(AIEffectPic->btn_back, 0, 2);
    lv_obj_set_size(AIEffectPic->btn_back, 50, 36);
    AIEffectPic->label_back = lv_label_create(AIEffectPic->btn_back);
    lv_label_set_text(AIEffectPic->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIEffectPic->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffectPic->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffectPic->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffectPic->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIEffectPic->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffectPic->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffectPic->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffectPic->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIEffectPic->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffectPic->label_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffectPic->label_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffectPic->label_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffectPic->label_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加返回按钮点击事件
    lv_obj_add_event_cb(AIEffectPic->btn_back, btn_back_event_cb, LV_EVENT_CLICKED, ui);

    // Write codes scr 风格图标与名称
    const char *style_names[9] = {"无特效",   "网红日漫", "吉卜力", "印象画派", "粘土风",
                                  "迪士尼3D", "新海诚",   "动漫风", "像素艺术"};
    int selected_index         = 0; // 默认选中第一个
    lv_obj_t *screen_AIEffectPic_icon[9];
    lv_obj_t *screen_AIEffectPic_label[9];
    lv_obj_t *screen_AIEffectPic_line[9];

    // 计算每个图标的间距，确保9个图标均匀分布且不重叠
    int icon_count      = 9;
    int icon_area_width = 640 - 2 * 40; // 左右各留40px边距
    int icon_spacing    = icon_area_width / (icon_count - 1);
    int start_x         = 40;
    // 定义图片数组
    const void *style_images[9] = {
        &_effect_none_RGB565A8_44x44,    // 无特效
        &_effect_web_RGB565A8_44x44,     // 网红日漫
        &_effect_child_RGB565A8_44x44,   // 儿童绘画
        &_effect_impress_RGB565A8_44x44, // 印象画派
        &_effect_clay_RGB565A8_44x44,    // 粘土风
        &_effect_ink_RGB565A8_44x44,     // 水墨画
        &_effect_fan_RGB565A8_44x44,     // 梵高风格 (暂时用相同图片)
        &_effect_anime_RGB565A8_44x44,   // 动漫风
        &_effect_cartoon_RGB565A8_44x44  // 卡通风
    };

    for(int i = 0; i < icon_count; ++i) {
        // 创建图片按钮
        screen_AIEffectPic_icon[i] = lv_imagebutton_create(AIEffectPic->bottom_cont);
        style_icon_btns[i]         = screen_AIEffectPic_icon[i];
        lv_obj_set_size(screen_AIEffectPic_icon[i], 44, 44);

        // 设置图片按钮的图片源
        lv_imagebutton_set_src(screen_AIEffectPic_icon[i], LV_IMAGEBUTTON_STATE_RELEASED, NULL, style_images[i], NULL);
        lv_imagebutton_set_src(screen_AIEffectPic_icon[i], LV_IMAGEBUTTON_STATE_PRESSED, NULL, style_images[i], NULL);
        lv_imagebutton_set_src(screen_AIEffectPic_icon[i], LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL, style_images[i],
                               NULL);
        lv_imagebutton_set_src(screen_AIEffectPic_icon[i], LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL, style_images[i],
                               NULL);

        // 设置图片按钮样式
        lv_obj_set_style_bg_opa(screen_AIEffectPic_icon[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(screen_AIEffectPic_icon[i], 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(screen_AIEffectPic_icon[i], 26, LV_PART_MAIN | LV_STATE_DEFAULT);

        // 均匀分布9个图标
        int x_pos = start_x + i * icon_spacing - 26; // 图标中心对齐
        lv_obj_set_pos(screen_AIEffectPic_icon[i], x_pos, 10);
        // 添加点击事件
        lv_obj_add_event_cb(screen_AIEffectPic_icon[i], style_icon_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        screen_AIEffectPic_label[i] = lv_label_create(AIEffectPic->bottom_cont);
        style_icon_labels[i]        = screen_AIEffectPic_label[i];
        lv_label_set_text(screen_AIEffectPic_label[i], style_names[i]);
        // 设置字体样式
        set_chs_fonts(ALI_PUHUITI_FONTPATH, 16, &label_result_style);
        lv_obj_add_style(screen_AIEffectPic_label[i], &label_result_style, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align_to(screen_AIEffectPic_label[i], screen_AIEffectPic_icon[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
        // 下划线对象
        screen_AIEffectPic_line[i] = lv_obj_create(AIEffectPic->bottom_cont);
        style_icon_lines[i]        = screen_AIEffectPic_line[i];
        lv_obj_set_size(screen_AIEffectPic_line[i], 28, 3);
        lv_obj_set_style_bg_color(screen_AIEffectPic_line[i], lv_color_hex(0x00BFFF), 0);
        lv_obj_set_style_radius(screen_AIEffectPic_line[i], 2, 0);
        lv_obj_align_to(screen_AIEffectPic_line[i], screen_AIEffectPic_label[i], LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    }
    // 初始化选中状态
    update_style_icon_selected(selected_index);

    // Write codes btn_switch (左下角原图/特效切换按钮)
    AIEffectPic->imgbtn_switch = lv_imagebutton_create(AIEffectPic->scr);
    lv_obj_set_pos(AIEffectPic->imgbtn_switch, 10, 400 - 54);
    lv_obj_set_size(AIEffectPic->imgbtn_switch, 50, 50);
    lv_imagebutton_set_src(AIEffectPic->imgbtn_switch, LV_IMAGEBUTTON_STATE_RELEASED, NULL,
                           &_comparison_yellow_RGB565A8_50x50, NULL);
    lv_imagebutton_set_src(AIEffectPic->imgbtn_switch, LV_IMAGEBUTTON_STATE_PRESSED, NULL,
                           &_comparison_yellow_RGB565A8_50x50, NULL);
    lv_imagebutton_set_src(AIEffectPic->imgbtn_switch, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_comparison_yellow_RGB565A8_50x50, NULL);
    lv_imagebutton_set_src(AIEffectPic->imgbtn_switch, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_comparison_yellow_RGB565A8_50x50, NULL);
    lv_obj_set_style_image_opa(AIEffectPic->imgbtn_switch, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮样式 - 默认状态
    lv_obj_set_style_bg_opa(AIEffectPic->imgbtn_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffectPic->imgbtn_switch, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffectPic->imgbtn_switch, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置按钮样式 - 按压状态（半透明浅灰色阴影）
    lv_obj_set_style_bg_opa(AIEffectPic->imgbtn_switch, 128, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(AIEffectPic->imgbtn_switch, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_width(AIEffectPic->imgbtn_switch, 0, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_radius(AIEffectPic->imgbtn_switch, 25, LV_PART_MAIN | LV_STATE_PRESSED);
    // 添加切换按钮按下和松开事件
    lv_obj_add_event_cb(AIEffectPic->imgbtn_switch, imgbtn_switch_pressed_cb, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(AIEffectPic->imgbtn_switch, imgbtn_switch_released_cb, LV_EVENT_RELEASED, ui);

    // Write codes btn_ok (右下角确认按钮)
    AIEffectPic->btn_ok = lv_btn_create(AIEffectPic->scr);
    lv_obj_set_pos(AIEffectPic->btn_ok, 640 - 120, 400 - 54);
    lv_obj_set_size(AIEffectPic->btn_ok, 48, 48);
    AIEffectPic->label_ok = lv_label_create(AIEffectPic->btn_ok);
    lv_label_set_text(AIEffectPic->label_ok, LV_SYMBOL_OK);
    lv_label_set_long_mode(AIEffectPic->label_ok, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffectPic->label_ok, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffectPic->btn_ok, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffectPic->label_ok, LV_PCT(100));
    lv_obj_set_style_bg_opa(AIEffectPic->btn_ok, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffectPic->btn_ok, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffectPic->btn_ok, lv_color_hex(0x00BFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffectPic->btn_ok, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffectPic->label_ok, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffectPic->label_ok, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffectPic->label_ok, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffectPic->label_ok, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加确认按钮点击事件
    lv_obj_add_event_cb(AIEffectPic->btn_ok, btn_ok_event_cb, LV_EVENT_CLICKED, ui);

    // Write codes btn_cancel (右下角取消按钮)
    AIEffectPic->btn_cancel = lv_btn_create(AIEffectPic->scr);
    lv_obj_set_pos(AIEffectPic->btn_cancel, 640 - 60, 400 - 54);
    lv_obj_set_size(AIEffectPic->btn_cancel, 48, 48);
    AIEffectPic->label_cancel = lv_label_create(AIEffectPic->btn_cancel);
    lv_label_set_text(AIEffectPic->label_cancel, LV_SYMBOL_CLOSE);
    lv_label_set_long_mode(AIEffectPic->label_cancel, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffectPic->label_cancel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIEffectPic->btn_cancel, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIEffectPic->label_cancel, LV_PCT(100));
    lv_obj_set_style_bg_opa(AIEffectPic->btn_cancel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIEffectPic->btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIEffectPic->btn_cancel, lv_color_hex(0xFF4444), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIEffectPic->btn_cancel, 24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIEffectPic->label_cancel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIEffectPic->label_cancel, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIEffectPic->label_cancel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIEffectPic->label_cancel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加取消按钮点击事件
    lv_obj_add_event_cb(AIEffectPic->btn_cancel, btn_cancel_event_cb, LV_EVENT_CLICKED, ui);

    // 创建AI处理结果显示标签
    AIEffectPic->label_result = lv_label_create(AIEffectPic->scr);
    lv_label_set_text(AIEffectPic->label_result, "AI处理中");
    lv_label_set_long_mode(AIEffectPic->label_result, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIEffectPic->label_result, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(AIEffectPic->label_result, 410);
    // AIEffectPic->label_result
    lv_obj_add_style(AIEffectPic->label_result, &label_result_style, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 初始时隐藏结果标签
    lv_obj_add_flag(AIEffectPic->label_result, LV_OBJ_FLAG_HIDDEN);

    // Update current screen layout.
    lv_obj_update_layout(AIEffectPic->scr);
}

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
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"
#include "config.h"
#include "ui_common.h"

// 添加image_recognize头文件
// #include "image_recognize.h"
#include "image_recognize/image_recognize.h"
#include "common/extract_thumbnail.h"

// 添加硬件按键相关头文件
#include <linux/input.h>
#include "indev.h"

extern char pic_filepath[128];
static char pic_thumbnail[128] = {0};
static AIPhoto_t *AIPhoto;

static void recognize_image(const char *image_path)
{
    char result[4096];
    int ret;
    image_recognizer_t *recognizer;

    recognizer = image_recognizer_create(IMAGE_RECOGNIZE_MODEL_NAME, IMAGE_RECOGNIZE_API_KEY, IMAGE_RECOGNIZE_BASE_URL);

    ret = image_recognizer_from_file(recognizer, image_path, NULL, result, sizeof(result));
    if(ret == 0) {
        MLOG_DBG("图像识别成功，结果: %s\n", result);
        if(AIPhoto->label_result != NULL) {
            lv_label_set_text(AIPhoto->label_result, result);
            lv_obj_set_style_text_color(AIPhoto->label_result, lv_color_hex(0x00FF00),
                                        LV_PART_MAIN | LV_STATE_DEFAULT); // 绿色表示成功
        }
    } else {
        MLOG_ERR("图像识别失败: %s\n", image_recognizer_get_error_string(ret));
        // 显示错误信息
        if(AIPhoto->label_result != NULL) {
            lv_label_set_text(AIPhoto->label_result, "识别失败");
            lv_obj_set_style_text_color(AIPhoto->label_result, lv_color_hex(0xFF0000),
                                        LV_PART_MAIN | LV_STATE_DEFAULT); // 红色表示失败
        }
    }

    image_recognizer_destroy(recognizer);
}

// 事件处理函数
static void page_aiphoto_btn_ai_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            MLOG_DBG("AIphoto按钮被点击，开始拍照和图像识别...\n");

            // 立即显示"正在识别"状态
            if(AIPhoto->label_result != NULL) {
                MLOG_DBG("正在识别...\n");
                lv_obj_clear_flag(AIPhoto->label_result, LV_OBJ_FLAG_HIDDEN); // 显示结果label
                lv_label_set_text(AIPhoto->label_result, "正在识别...");
                lv_obj_set_style_text_color(AIPhoto->label_result, lv_color_hex(0xFFFF00),
                                            LV_PART_MAIN | LV_STATE_DEFAULT); // 黄色表示正在处理
                // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
                lv_obj_invalidate(AIPhoto->label_result);
                lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
                MLOG_DBG("强制UI刷新完成\n");
            }

            // 2. 识别图片
            recognize_image(pic_thumbnail);
            break;
        }
        default: break;
    }
}

static void page_aiphoto_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aitakephoto.scr, g_ui.screen_AITakePhoto_del,
                                  &g_ui.screen_AIPhoto_del, setup_scr_screen_AITakePhoto, LV_SCR_LOAD_ANIM_NONE, 200,
                                  200, false, true);
            break;
        }
        default: break;
    }
}

// 硬件拍照按键读取回调函数
static void aiphoto_key_handler(int key_code, int key_value)
{
    button_type_t button_type = BUTTON_TYPE_NONE;
    if(key_value == 1) {
        if(key_code == KEY_PLAY) {
            button_type = BUTTON_TYPE_AI; // AI识别
        }
    }

    bool pressed = (button_type == BUTTON_TYPE_AI);

    // 检测按键按下
    if(pressed) {
        MLOG_DBG("Hardware AI识别按键被按下\n");

        // 直接执行AI识别业务逻辑，与UI按钮功能一致
        // 立即显示"正在识别"状态
        if(AIPhoto->label_result != NULL) {
            MLOG_DBG("正在识别...\n");
            lv_obj_clear_flag(AIPhoto->label_result, LV_OBJ_FLAG_HIDDEN); // 显示结果label
            lv_label_set_text(AIPhoto->label_result, "正在识别...");
            lv_obj_set_style_text_color(AIPhoto->label_result, lv_color_hex(0xFFFF00),
                                        LV_PART_MAIN | LV_STATE_DEFAULT); // 黄色表示正在处理
            // 强制UI刷新 - 刷新整个屏幕，否则由于后续处理阻塞，无法显示提示信息
            lv_obj_invalidate(AIPhoto->label_result);
            lv_refr_now(lv_disp_get_default()); // 刷新默认显示器
            MLOG_DBG("强制UI刷新完成\n");
        }

        // 2. 识别图片
        recognize_image(pic_thumbnail);
    }
}

void events_init_screen_AIPhoto(lv_ui_t *ui)
{
    // 添加事件处理
    lv_obj_add_event_cb(AIPhoto->btn_back, page_aiphoto_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(AIPhoto->btn_ai, page_aiphoto_btn_ai_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_AIPhoto(lv_ui_t *ui)
{
    MLOG_DBG("loading page_AIPhoto...\n");

    AIPhoto      = &ui->page_aiphoto;
    AIPhoto->del = true;

    // 创建主页面1 容器
    if(AIPhoto->scr != NULL) {
        if(lv_obj_is_valid(AIPhoto->scr)) {
            MLOG_DBG("pageAiTakePhoto->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIPhoto->scr);
        } else {
            MLOG_DBG("pageAiTakePhoto->scr 已被自动销毁，仅重置指针\n");
        }
        AIPhoto->scr = NULL;
    }

    // Write codes screen_AIPhoto
    AIPhoto->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIPhoto->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIPhoto->scr, LV_SCROLLBAR_MODE_OFF);

    // 显示图片
    AIPhoto->img_main = lv_img_create(AIPhoto->scr);
    lv_obj_set_pos(AIPhoto->img_main, 0, 60);
    lv_obj_set_size(AIPhoto->img_main, 640, 360);

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
        MLOG_DBG("pic_thumbnail 文件存在: %s\n", pic_thumbnail);
        char pic_path_forshow[128];
        snprintf(pic_path_forshow, sizeof(pic_path_forshow), "A:%s", pic_thumbnail);
        lv_img_set_src(AIPhoto->img_main, pic_path_forshow);
    } else {
        MLOG_DBG("pic_thumbnail 文件不存在: %s\n", pic_thumbnail);
    }

    usleep(500000); // 500ms

    // Write style for screen_AIPhoto, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    // lv_obj_set_style_bg_opa(AIPhoto->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(AIPhoto->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes bottom_cont (底部边栏)
    AIPhoto->bottom_cont = lv_obj_create(AIPhoto->scr);
    lv_obj_set_pos(AIPhoto->bottom_cont, 0, 420);
    lv_obj_set_size(AIPhoto->bottom_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIPhoto->bottom_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for bottom_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(AIPhoto->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(AIPhoto->bottom_cont, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(AIPhoto->bottom_cont, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIPhoto->bottom_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhoto->bottom_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhoto->bottom_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhoto->bottom_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont (顶部边栏)
    AIPhoto->top_cont = lv_obj_create(AIPhoto->scr);
    lv_obj_set_pos(AIPhoto->top_cont, 0, 0);
    lv_obj_set_size(AIPhoto->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIPhoto->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIPhoto->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhoto->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhoto->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhoto->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (顶部返回按钮)
    AIPhoto->btn_back = lv_btn_create(AIPhoto->top_cont);
    lv_obj_set_pos(AIPhoto->btn_back, 0, 4);
    lv_obj_set_size(AIPhoto->btn_back, 60, 50);
    AIPhoto->label_back = lv_label_create(AIPhoto->btn_back);
    lv_label_set_text(AIPhoto->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIPhoto->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhoto->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhoto->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhoto->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIPhoto->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhoto->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIPhoto->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhoto->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhoto->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIPhoto->label_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIPhoto->label_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIPhoto->label_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIPhoto->label_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_ai (AI按钮)
    AIPhoto->btn_ai = lv_btn_create(AIPhoto->scr);
    lv_obj_set_pos(AIPhoto->btn_ai, 6, 203);
    lv_obj_set_size(AIPhoto->btn_ai, 77, 56);
    AIPhoto->label_ai = lv_label_create(AIPhoto->btn_ai);
    lv_label_set_text(AIPhoto->label_ai, "AI");
    lv_label_set_long_mode(AIPhoto->label_ai, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhoto->label_ai, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIPhoto->btn_ai, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIPhoto->label_ai, LV_PCT(100));

    // Write style for btn_ai, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIPhoto->btn_ai, 63, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIPhoto->btn_ai, lv_color_hex(0x7b797b), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIPhoto->btn_ai, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIPhoto->btn_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIPhoto->btn_ai, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIPhoto->btn_ai, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIPhoto->label_ai, lv_color_hex(0xfff290), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIPhoto->label_ai, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIPhoto->label_ai, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIPhoto->label_ai, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建AI识别结果显示标签
    AIPhoto->label_result = lv_label_create(AIPhoto->scr);
    lv_label_set_text(AIPhoto->label_result, "AI识别中");
    lv_label_set_long_mode(AIPhoto->label_result, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIPhoto->label_result, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(AIPhoto->label_result, 410);
    // AIPhoto->label_result
    static lv_style_t label_result_style;
    if(label_result_style.prop_cnt > 1)
        lv_style_reset(&label_result_style);
    else
        lv_style_init(&label_result_style);
    lv_style_set_opa(&label_result_style, 255);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 36, &label_result_style);
    lv_style_set_text_color(&label_result_style, lv_color_hex(0xFFFFFF));
    lv_style_set_text_opa(&label_result_style, 255);
    lv_style_set_text_align(&label_result_style, LV_TEXT_ALIGN_CENTER);
    lv_obj_add_style(AIPhoto->label_result, &label_result_style, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 初始时隐藏结果标签
    lv_obj_add_flag(AIPhoto->label_result, LV_OBJ_FLAG_HIDDEN);

    /* 设置当前页面和按键处理回调 */
    set_current_page_handler(aiphoto_key_handler);

    // Update current screen layout.
    lv_obj_update_layout(AIPhoto->scr);

    // 添加事件处理
    events_init_screen_AIPhoto(ui);
}

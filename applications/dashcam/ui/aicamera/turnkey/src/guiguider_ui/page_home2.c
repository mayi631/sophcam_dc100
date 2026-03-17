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
#include <stdlib.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"
#include "config.h"

static void screenHome2_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_GESTURE: {
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
            switch(dir) {
                case LV_DIR_RIGHT: {
                    lv_indev_wait_release(lv_indev_active());
                    ui_load_scr_animation(&g_ui, &g_ui.page_home1.scr, g_ui.screenHome1_del, &g_ui.screenHome2_del,
                                          setup_scr_home1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 200, 0, false, true);
                    break;
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

static void screenHome2_btn_album_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_photoalbum.scr, g_ui.screen_PhotoAlbum_del, &g_ui.screenHome2_del,
                                  setup_scr_screen_PhotoAlbum, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screenHome2_btn_setting_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_settings.scr, g_ui.screen_Settings_del, &g_ui.screenHome2_del,
                                  setup_scr_screen_Settings, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screenHome2_btn_chinese_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_TakeChinese, g_ui.screen_TakeChinese_del, &g_ui.screenHome2_del,
                                  setup_scr_screen_TakeChinese, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screenHome2_btn_english_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.screen_TakeEnglish, g_ui.screen_TakeEnglish_del, &g_ui.screenHome2_del,
                                  setup_scr_screen_TakeEnglish, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void events_init_screenHome2(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_home2.scr, screenHome2_event_handler, LV_EVENT_GESTURE, ui);
    lv_obj_add_event_cb(ui->page_home2.imgbtn_album, screenHome2_btn_album_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_home2.imgbtn_setting, screenHome2_btn_setting_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_home2.imgbtn_chinese, screenHome2_btn_chinese_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_home2.imgbtn_english, screenHome2_btn_english_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screenHome2(lv_ui_t *ui)
{
    MLOG_DBG("loading screenHome2...\n");

    PageHome2_t *home2 = &ui->page_home2;
    home2->del         = true;

    // 创建主页面2 容器
    if(home2->scr != NULL) {
        if(lv_obj_is_valid(home2->scr)) {
            MLOG_DBG("page_home2->scr 仍然有效，删除旧对象\n");
            lv_obj_del(home2->scr);
        } else {
            MLOG_DBG("page_home2->scr 已被自动销毁，仅重置指针\n");
        }
        home2->scr = NULL;
    }

    // 创建主页面2
    home2->scr = lv_obj_create(NULL);
    lv_obj_set_size(home2->scr, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(home2->scr, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条，默认 LV_SCROLLBAR_MODE_AUTO

    // Write style for page_home2->scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    MLOG_DBG("home2->scr lv_obj_get_scrollbar_mode: %d\n", lv_obj_get_scrollbar_mode(home2->scr));
    lv_obj_set_style_bg_opa(home2->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明度 0~255
    lv_obj_set_style_bg_color(home2->scr, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 背景颜色
    lv_obj_set_style_bg_grad_dir(home2->scr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);    // 无渐变色

    // 主页面上增加按钮

    // 1. 相册按钮
    home2->imgbtn_album = lv_imagebutton_create(home2->scr);
    lv_obj_set_pos(home2->imgbtn_album, 350, 70);
    lv_obj_set_size(home2->imgbtn_album, 163, 156);
    lv_imagebutton_set_src(home2->imgbtn_album, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_photoalbum_2_RGB565A8_163x156,
                           NULL);
    lv_imagebutton_set_src(home2->imgbtn_album, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_photoalbum_2_RGB565A8_163x156,
                           NULL);
    lv_imagebutton_set_src(home2->imgbtn_album, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_photoalbum_2_RGB565A8_163x156, NULL);
    lv_imagebutton_set_src(home2->imgbtn_album, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_photoalbum_2_RGB565A8_163x156, NULL);
    lv_obj_set_style_pad_all(home2->imgbtn_album, 0, LV_STATE_DEFAULT);

    // Write style for screenHome2_btn_album, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home2->imgbtn_album, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home2->imgbtn_album, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home2->imgbtn_album, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 2. 设置按钮
    home2->imgbtn_setting = lv_imagebutton_create(home2->scr);
    lv_obj_set_pos(home2->imgbtn_setting, 113, 79);
    lv_obj_set_size(home2->imgbtn_setting, 156, 145);
    lv_imagebutton_set_src(home2->imgbtn_setting, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_setting_RGB565A8_156x145,
                           NULL);
    lv_imagebutton_set_src(home2->imgbtn_setting, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_setting_RGB565A8_156x145, NULL);
    lv_imagebutton_set_src(home2->imgbtn_setting, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_setting_RGB565A8_156x145, NULL);
    lv_imagebutton_set_src(home2->imgbtn_setting, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_setting_RGB565A8_156x145, NULL);
    lv_obj_set_style_pad_all(home2->imgbtn_setting, 0, LV_STATE_DEFAULT);

    // Write style for screenHome2_btn_setting, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home2->imgbtn_setting, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home2->imgbtn_setting, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home2->imgbtn_setting, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 3. 拍汉字按钮
    home2->imgbtn_chinese = lv_imagebutton_create(home2->scr);
    lv_obj_set_pos(home2->imgbtn_chinese, 113, 253);
    lv_obj_set_size(home2->imgbtn_chinese, 165, 159);
    lv_imagebutton_set_src(home2->imgbtn_chinese, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_chinese_2_RGB565A8_165x159,
                           NULL);
    lv_imagebutton_set_src(home2->imgbtn_chinese, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_chinese_2_RGB565A8_165x159,
                           NULL);
    lv_imagebutton_set_src(home2->imgbtn_chinese, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_chinese_2_RGB565A8_165x159, NULL);
    lv_imagebutton_set_src(home2->imgbtn_chinese, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_chinese_2_RGB565A8_165x159, NULL);
    lv_obj_set_style_pad_all(home2->imgbtn_chinese, 0, LV_STATE_DEFAULT);

    // Write style for screenHome2_btn_chinese, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home2->imgbtn_chinese, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home2->imgbtn_chinese, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home2->imgbtn_chinese, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 4. 拍英文按钮
    home2->imgbtn_english = lv_imagebutton_create(home2->scr);
    lv_obj_set_pos(home2->imgbtn_english, 350, 253);
    lv_obj_set_size(home2->imgbtn_english, 165, 163);
    lv_imagebutton_set_src(home2->imgbtn_english, LV_IMAGEBUTTON_STATE_RELEASED, NULL, &_english_RGB565A8_165x163,
                           NULL);
    lv_imagebutton_set_src(home2->imgbtn_english, LV_IMAGEBUTTON_STATE_PRESSED, NULL, &_english_RGB565A8_165x163, NULL);
    lv_imagebutton_set_src(home2->imgbtn_english, LV_IMAGEBUTTON_STATE_CHECKED_RELEASED, NULL,
                           &_english_RGB565A8_165x163, NULL);
    lv_imagebutton_set_src(home2->imgbtn_english, LV_IMAGEBUTTON_STATE_CHECKED_PRESSED, NULL,
                           &_english_RGB565A8_165x163, NULL);
    lv_obj_set_style_pad_all(home2->imgbtn_english, 0, LV_STATE_DEFAULT);

    // Write style for screenHome2_btn_english, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    //  lv_obj_set_style_bg_opa(home2->imgbtn_english, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_bg_color(home2->imgbtn_english, lv_color_hex(0x2A2A2A), LV_PART_MAIN|LV_STATE_DEFAULT);
    //  lv_obj_set_style_border_width(home2->imgbtn_english, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 创建页面指示器容器
    home2->dots_cont = lv_obj_create(home2->scr);
    lv_obj_set_size(home2->dots_cont, 40, 20);                                     // 缩小容器宽度
    lv_obj_set_pos(home2->dots_cont, 300, 450);                                    // 640/2-40/2=300，底部居中
    lv_obj_set_style_bg_opa(home2->dots_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_border_width(home2->dots_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(home2->dots_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建第一个圆点
    home2->dot_home1 = lv_obj_create(home2->dots_cont);
    lv_obj_set_size(home2->dot_home1, 12, 12);
    lv_obj_set_pos(home2->dot_home1, 2, 4);                                                               // 靠左
    lv_obj_set_style_radius(home2->dot_home1, 6, LV_PART_MAIN | LV_STATE_DEFAULT);                        // 圆形
    lv_obj_set_style_bg_color(home2->dot_home1, lv_color_hex(0x666666), LV_PART_MAIN | LV_STATE_DEFAULT); // 灰色
    lv_obj_set_style_border_width(home2->dot_home1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建第二个圆点（当前页面）
    home2->dot_home2 = lv_obj_create(home2->dots_cont);
    lv_obj_set_size(home2->dot_home2, 12, 12);
    lv_obj_set_pos(home2->dot_home2, 22, 4);                                       // 靠右
    lv_obj_set_style_radius(home2->dot_home2, 6, LV_PART_MAIN | LV_STATE_DEFAULT); // 圆形
    lv_obj_set_style_bg_color(home2->dot_home2, lv_color_hex(0xFFD400),
                              LV_PART_MAIN | LV_STATE_DEFAULT); // 黄色（当前页面）
    lv_obj_set_style_border_width(home2->dot_home2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout. 不需要，LVGL 会自动更新。
    // lv_obj_update_layout(home2->scr);

    // Init events for screen.
    events_init_screenHome2(ui);
}

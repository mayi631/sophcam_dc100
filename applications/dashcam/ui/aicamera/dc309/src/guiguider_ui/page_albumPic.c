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
#include <unistd.h>
#include "events_init.h"
#include "custom.h"
#include "page_all.h"
#include "gui_guider.h"
#include "filemng.h"
#include "linux/input.h"
#include "indev.h"
#include "image_process.h"
#include "common/extract_thumbnail.h"
#include "rtt.h"

extern char current_image_path[256]; // 当前选中的图片路径，用于传递给图片查看页面
extern int g_total_media_files;
extern char **g_all_filenames;
extern int g_current_page;  // 当前页面索引
extern int g_total_pages;
extern int g_images_per_page;  // 每页显示的图片数量
extern int g_cam_id; // 当前相机ID，第一路
extern int g_album_image_index; // 当前焦点图片在所有图片中的索引

lv_obj_t *obj_AibumPic_s; //底层容器
static lv_obj_t *cont_top_s; // 顶部容器
static lv_obj_t *btn_delete_s; // 删除按钮
static lv_obj_t *btn_delete_label_s; // 删除按钮标签
static lv_obj_t *btn_back_s; // 返回按钮
static lv_obj_t *btn_back_label_s; // 返回按钮标签
static lv_obj_t *label_name_s; // 图片名称标签
static lv_obj_t *img_s; // 图片对象

int g_return_page_index = 0;    // 返回时需要跳转的页面索引
int g_return_focus_index = 0;  // 返回时的焦点索引

bool is_album_pic = false;
extern bool ai_custom_is_confire;

static void ord_delete_after_cb(void);

static int set_current_photo_album_image_path(int index)
{
    MLOG_DBG("当前索引 %d\n", index);
    if(index < 0 || index >= g_total_media_files) {
        MLOG_ERR("索引 %d 超出范围 (0 - %d)\n", index, g_total_media_files - 1);
        return -1;
    }
    if(g_all_filenames[index] == NULL) {
        MLOG_ERR("索引 %d 处的文件名为空\n", index);
        return -1;
    }
    get_thumbnail_path(g_all_filenames[index], current_image_path, sizeof(current_image_path), PHOTO_LARGE_PATH);
    MLOG_DBG("设置当前图片路径: %s\n", current_image_path);
    return 0;
}

const char *get_curr_pic_path(void)
{
    return current_image_path;
}

static void screen_PhotoAlbumPic_btn_delete_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));

    if(code == LV_EVENT_CLICKED) {
        create_simple_delete_dialog(current_image_path); // 创建确认浮窗
        sure_delete_register_callback(ord_delete_after_cb);
    }
}

static void screen_PhotoAlbumPic_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
                is_album_pic = false;
                // 保存返回时需要的信息
                g_return_page_index = g_album_image_index / g_images_per_page;
                g_return_focus_index = g_album_image_index % g_images_per_page;

                ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album_from_Pic, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                      false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_PhotoAlbumPic(lv_ui_t *ui)
{
    lv_obj_add_event_cb(btn_delete_s, screen_PhotoAlbumPic_btn_delete_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_back_s, screen_PhotoAlbumPic_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void photo_delete_anim_complete(lv_anim_t *a)
{
    // 非选择模式：跳转到图片查看页面
    lv_img_set_src(img_s, current_image_path);
    char *last_slash = strrchr(current_image_path, '/');
    // 如果找到了'/'，返回其后面的字符串；否则返回原字符串
    char *filename;
    if(last_slash != NULL) {
        filename = last_slash + 1;
    } else {
        filename = current_image_path; // 如果没有找到'/'，则使用原字符串
    }
    lv_label_set_text(label_name_s, filename);
    lv_anim_del(a, a->exec_cb);
}

static void albumpic_menu_callback(int key_code, int key_value)
{
    if(key_code == KEY_MENU && key_value == 1) {
        MLOG_DBG("albumpic_menu_callback\n");
            is_album_pic = false;
            ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album_from_Pic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                  true);
    } else if(key_code == KEY_PLAY && key_value == 1) {

    } else if((key_code == KEY_LEFT || key_code == KEY_RIGHT) && key_value == 1) {
        if(key_code == KEY_LEFT) // 左键
        {
            g_album_image_index++; // 左
            if(g_album_image_index >= g_total_media_files) {
                g_album_image_index = g_total_media_files - 1;
            }
        } else if(key_code == KEY_RIGHT) {
            g_album_image_index--;
            if(g_album_image_index < 0) {
                g_album_image_index = 0;
            }
        }
        g_return_page_index  = g_album_image_index / g_images_per_page; // 焦点
        g_return_focus_index = g_album_image_index % g_images_per_page; // 页面
        set_current_photo_album_image_path(g_album_image_index);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_values(&anim, 0, 1);
        lv_anim_set_time(&anim, 50);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
        lv_anim_set_completed_cb(&anim, photo_delete_anim_complete);
        lv_anim_start(&anim);
    } else if((key_code == KEY_OK) && key_value == 1) {
    } else if((key_code == KEY_DOWN) && key_value == 1) {
        lv_obj_send_event(btn_delete_s, LV_EVENT_CLICKED, NULL);
    }
}

static void gesture_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_GESTURE: {
            // 获取手势方向，需要 TP 驱动支持
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
            switch(dir) {
                case LV_DIR_LEFT: {
                    lv_indev_wait_release(lv_indev_active());
                    g_album_image_index++; // 左滑
                    if(g_album_image_index >= g_total_media_files) {
                        g_album_image_index = g_total_media_files - 1;
                    }
                    set_current_photo_album_image_path(g_album_image_index);
                    break;
                }
                case LV_DIR_RIGHT: {
                    lv_indev_wait_release(lv_indev_active());
                    g_album_image_index--; // 右滑
                    if(g_album_image_index < 0) {
                        g_album_image_index = 0;
                    }
                    set_current_photo_album_image_path(g_album_image_index);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
    g_return_page_index = g_album_image_index / g_images_per_page;
    g_return_focus_index = g_album_image_index % g_images_per_page;
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_values(&anim, 0, 1);
    lv_anim_set_time(&anim, 50);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
    lv_anim_set_completed_cb(&anim, photo_delete_anim_complete);
    lv_anim_start(&anim);
}

void setup_scr_screen_PhotoAlbumPic(lv_ui_t *ui)
{
    MLOG_DBG("loading page_PhotoAlbumPic...\n");
    is_album_pic = true;
    // Write codes screen_PhotoAlbumPic
    obj_AibumPic_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_AibumPic_s , H_RES, V_RES);
    lv_obj_add_style(obj_AibumPic_s, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(obj_AibumPic_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // 创建图片
    MLOG_DBG("current_image_path: %s\n", current_image_path);
    img_s = lv_img_create(obj_AibumPic_s);
    lv_obj_set_pos(img_s, 0, 0);
    lv_obj_set_size(img_s, H_RES, V_RES);
    lv_img_set_src(img_s, current_image_path);

    // Write codes screen_PhotoAlbumPic_cont_top
    cont_top_s = lv_obj_create(obj_AibumPic_s);
    lv_obj_set_pos(cont_top_s, 0, 0);
    lv_obj_set_size(cont_top_s, H_RES, 60);
    lv_obj_set_scrollbar_mode(cont_top_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top_s, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_btn_delete
    btn_delete_s = lv_button_create(obj_AibumPic_s);
    lv_obj_align(btn_delete_s, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    lv_obj_set_size(btn_delete_s, 64, 60);
    lv_obj_set_style_bg_opa(btn_delete_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    btn_delete_label_s = lv_label_create(btn_delete_s);
    lv_obj_set_style_text_color(btn_delete_label_s, lv_color_hex(0XFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(btn_delete_label_s, " " LV_SYMBOL_TRASH " ");
    lv_label_set_long_mode(btn_delete_label_s, LV_LABEL_LONG_WRAP);
    lv_obj_align(btn_delete_label_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_delete_s, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(btn_delete_label_s, LV_PCT(100));
    lv_obj_set_style_text_font(btn_delete_label_s, &lv_font_montserrat_42, 0);

    // Write style for screen_PhotoAlbumPic_btn_delete, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(btn_delete_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_delete_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_delete_s, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_delete_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_delete_s, lv_color_hex(0x171717),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(btn_delete_s, &lv_font_SourceHanSerifSC_Regular_30,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_delete_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_delete_s, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_btn_back
    btn_back_s = lv_button_create(cont_top_s);
    lv_obj_set_pos(btn_back_s, 4, 2);
    lv_obj_set_size(btn_back_s, 60, 50);
    lv_obj_add_style(btn_back_s, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    btn_back_label_s = lv_label_create(btn_back_s);
    lv_label_set_text(btn_back_label_s, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(btn_back_label_s, LV_LABEL_LONG_WRAP);
    lv_obj_align(btn_back_label_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(btn_back_label_s, LV_PCT(100));
    lv_obj_add_style(btn_back_label_s, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_label_name
    char* last_slash = strrchr(current_image_path, '/');
    // 如果找到了'/'，返回其后面的字符串；否则返回原字符串
    char* filename;
    if (last_slash != NULL) {
        filename = last_slash + 1;
    } else {
        filename = current_image_path; // 如果没有找到'/'，则使用原字符串
    }
    label_name_s = lv_label_create(obj_AibumPic_s);
    lv_obj_set_pos(label_name_s, 189, 20);
    lv_obj_set_size(label_name_s, 300, 40);
    lv_label_set_text(label_name_s, filename);
    lv_label_set_long_mode(label_name_s, LV_LABEL_LONG_SCROLL);
    // Write style for screen_PhotoAlbumPic_label_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_name_s, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_name_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(label_name_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label_name_s, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(label_name_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置当前页面的按键处理器
    set_current_page_handler(albumpic_menu_callback);
    // Update current screen layout.
    lv_obj_update_layout(obj_AibumPic_s);

    // Init events for screen.
    events_init_screen_PhotoAlbumPic(ui);

}

static void ord_delete_after_cb(void)
{
    char *last_slash = strrchr(current_image_path, '/');
    char *filename;
    if(last_slash != NULL) {
        filename = last_slash + 1;
    } else {
        filename = current_image_path;
    }
    int j;
    for(j = 0; j < g_total_media_files; j++) {
        if(g_all_filenames[j] != NULL && strcmp(g_all_filenames[j], filename) == 0) {
            free(g_all_filenames[j]);
            g_all_filenames[j] = NULL;
            // 将后面的文件名前移
            int k;
            for(k = j; k < g_total_media_files - 1; k++) {
                g_all_filenames[k] = g_all_filenames[k + 1];
            }
            if(((j + 1) % g_images_per_page == 1) && (j == g_total_media_files - 1)) {
                g_current_page--;
                g_total_pages--;
                if(g_current_page < 0) {
                    g_current_page = 0;
                }
            }
            g_total_media_files--;
            // 调整返回页面和焦点索引
            if(j <= g_album_image_index) {
                // 如果删除的是当前或之前的图片，需要调整索引
                g_album_image_index--;
                if(g_album_image_index < 0) g_album_image_index = 0;

                // 重新计算返回页面和焦点
                g_return_page_index  = g_album_image_index / g_images_per_page;
                g_return_focus_index = g_album_image_index % g_images_per_page;
            }

            break;
        }
    }

    is_album_pic = 0;
    ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album_from_Pic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

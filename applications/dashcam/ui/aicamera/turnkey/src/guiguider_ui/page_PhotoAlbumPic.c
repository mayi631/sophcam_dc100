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

extern char current_image_path[256]; // 当前选中的图片路径，用于传递给图片查看页面

static void screen_PhotoAlbumPic_btn_delete_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {

            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbumPic_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_photoalbum.scr, g_ui.page_photoalbum.del,
                                  &g_ui.page_photoalbumpic.del, setup_scr_screen_PhotoAlbum, LV_SCR_LOAD_ANIM_NONE, 0,
                                  0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_PhotoAlbumPic(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_photoalbumpic.btn_delete, screen_PhotoAlbumPic_btn_delete_event_handler,
                        LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_photoalbumpic.btn_back, screen_PhotoAlbumPic_btn_back_event_handler, LV_EVENT_CLICKED,
                        ui);
}

void setup_scr_screen_PhotoAlbumPic(lv_ui_t *ui)
{
    MLOG_DBG("loading page_PhotoAlbumPic...\n");

    PhotoAlbumPic_t *PhotoAlbumPic = &ui->page_photoalbumpic;
    PhotoAlbumPic->del             = true;

    // 创建主页面1 容器
    if(PhotoAlbumPic->scr != NULL) {
        if(lv_obj_is_valid(PhotoAlbumPic->scr)) {
            MLOG_DBG("page_PhotoAlbumPic->scr 仍然有效，删除旧对象\n");
            lv_obj_del(PhotoAlbumPic->scr);
        } else {
            MLOG_DBG("page_PhotoAlbumPic->scr 已被自动销毁，仅重置指针\n");
        }
        PhotoAlbumPic->scr = NULL;
    }

    // Write codes screen_PhotoAlbumPic
    PhotoAlbumPic->scr = lv_obj_create(NULL);
    lv_obj_set_size(PhotoAlbumPic->scr, 640, 480);
    lv_obj_set_scrollbar_mode(PhotoAlbumPic->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_PhotoAlbumPic, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    // lv_obj_set_style_bg_opa(PhotoAlbumPic->scr, LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(PhotoAlbumPic->scr, LV_OPA_0, LV_PART_MAIN);

    // Write codes screen_PhotoAlbumPic_cont_top
    PhotoAlbumPic->cont_top = lv_obj_create(PhotoAlbumPic->scr);
    lv_obj_set_pos(PhotoAlbumPic->cont_top, 0, 0);
    lv_obj_set_size(PhotoAlbumPic->cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(PhotoAlbumPic->cont_top, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_PhotoAlbumPic_cont_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(PhotoAlbumPic->cont_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbumPic->cont_top, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(PhotoAlbumPic->cont_top, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbumPic->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_btn_delete
    PhotoAlbumPic->btn_delete = lv_button_create(PhotoAlbumPic->cont_top);
    lv_obj_set_pos(PhotoAlbumPic->btn_delete, 547, 12);
    lv_obj_set_size(PhotoAlbumPic->btn_delete, 64, 38);
    PhotoAlbumPic->btn_delete_label = lv_label_create(PhotoAlbumPic->btn_delete);
    lv_label_set_text(PhotoAlbumPic->btn_delete_label, " " LV_SYMBOL_TRASH " ");
    lv_label_set_long_mode(PhotoAlbumPic->btn_delete_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoAlbumPic->btn_delete_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(PhotoAlbumPic->btn_delete, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(PhotoAlbumPic->btn_delete_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumPic_btn_delete, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbumPic->btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbumPic->btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbumPic->btn_delete, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbumPic->btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbumPic->btn_delete, lv_color_hex(0xFFD600),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbumPic->btn_delete, &lv_font_SourceHanSerifSC_Regular_30,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbumPic->btn_delete, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbumPic->btn_delete, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_btn_back
    PhotoAlbumPic->btn_back = lv_button_create(PhotoAlbumPic->cont_top);
    lv_obj_set_pos(PhotoAlbumPic->btn_back, 0, 4);
    lv_obj_set_size(PhotoAlbumPic->btn_back, 60, 50);
    PhotoAlbumPic->btn_back_label = lv_label_create(PhotoAlbumPic->btn_back);
    lv_label_set_text(PhotoAlbumPic->btn_back_label, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(PhotoAlbumPic->btn_back_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoAlbumPic->btn_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(PhotoAlbumPic->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(PhotoAlbumPic->btn_back_label, LV_PCT(100));

    // Write style for screen_PhotoAlbumPic_btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbumPic->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbumPic->btn_back, lv_color_hex(0xFFD600),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbumPic->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbumPic->btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbumPic->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbumPic->btn_back, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbumPic->btn_back, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbumPic->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbumPic->btn_back, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_label_name
    PhotoAlbumPic->label_name = lv_label_create(PhotoAlbumPic->scr);
    lv_obj_set_pos(PhotoAlbumPic->label_name, 269, 23);
    lv_obj_set_size(PhotoAlbumPic->label_name, 101, 18);
    lv_label_set_text(PhotoAlbumPic->label_name, "文件名");
    lv_label_set_long_mode(PhotoAlbumPic->label_name, LV_LABEL_LONG_WRAP);

    // Write style for screen_PhotoAlbumPic_label_name, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbumPic->label_name, lv_color_hex(0xffffff),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbumPic->label_name, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbumPic->label_name, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbumPic->label_name, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbumPic->label_name, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_PhotoAlbumPic_cont_bottom
    PhotoAlbumPic->cont_bottom = lv_obj_create(PhotoAlbumPic->scr);
    lv_obj_set_pos(PhotoAlbumPic->cont_bottom, 0, 420);
    lv_obj_set_size(PhotoAlbumPic->cont_bottom, 640, 60);
    lv_obj_set_scrollbar_mode(PhotoAlbumPic->cont_bottom, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_PhotoAlbumPic_cont_bottom, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(PhotoAlbumPic->cont_bottom, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbumPic->cont_bottom, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(PhotoAlbumPic->cont_bottom, LV_GRAD_DIR_NONE,
                                 LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbumPic->cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建图片
    MLOG_DBG("current_image_path: %s\n", current_image_path);
    PhotoAlbumPic->img = lv_img_create(PhotoAlbumPic->scr);
    lv_obj_set_pos(PhotoAlbumPic->img, 0, 60);
    lv_obj_set_size(PhotoAlbumPic->img, 640, 360);
    lv_img_set_src(PhotoAlbumPic->img, current_image_path);

    // Update current screen layout.
    lv_obj_update_layout(PhotoAlbumPic->scr);

    // Init events for screen.
    events_init_screen_PhotoAlbumPic(ui);
}

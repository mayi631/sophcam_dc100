/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"

void setup_scr_screen_Photo(lv_ui_t *ui)
{
    // Write codes screen_Photo
    ui->screen_Photo = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_Photo, 640, 480);
    lv_obj_set_scrollbar_mode(ui->screen_Photo, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_Photo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_Photo, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_Photo_canvas_1
    LV_DRAW_BUF_DEFINE_STATIC(screen_Photo_canvas_1_draw_buf, 640, 480, LV_COLOR_FORMAT_ARGB8888);
    LV_DRAW_BUF_INIT_STATIC(screen_Photo_canvas_1_draw_buf);
    ui->screen_Photo_canvas_1 = lv_canvas_create(ui->screen_Photo);
    lv_obj_set_pos(ui->screen_Photo_canvas_1, 0, 0);
    lv_obj_set_size(ui->screen_Photo_canvas_1, 640, 480);
    lv_obj_set_scrollbar_mode(ui->screen_Photo_canvas_1, LV_SCROLLBAR_MODE_OFF);
    lv_canvas_set_draw_buf(ui->screen_Photo_canvas_1, &screen_Photo_canvas_1_draw_buf);
    lv_canvas_fill_bg(ui->screen_Photo_canvas_1, lv_color_hex(0xffffff), 0);

    lv_layer_t layer_screen_Photo_canvas_1;
    lv_canvas_init_layer(ui->screen_Photo_canvas_1, &layer_screen_Photo_canvas_1);
    lv_canvas_finish_layer(ui->screen_Photo_canvas_1, &layer_screen_Photo_canvas_1);

    // Write codes screen_Photo_btn_1
    ui->screen_Photo_btn_1 = lv_button_create(ui->screen_Photo);
    lv_obj_set_pos(ui->screen_Photo_btn_1, 0, 0);
    lv_obj_set_size(ui->screen_Photo_btn_1, 77, 56);
    ui->screen_Photo_btn_1_label = lv_label_create(ui->screen_Photo_btn_1);
    lv_label_set_text(ui->screen_Photo_btn_1_label, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(ui->screen_Photo_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_Photo_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_Photo_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_Photo_btn_1_label, LV_PCT(100));

    // Write style for screen_Photo_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_Photo_btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_Photo_btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_Photo_btn_1, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_Photo_btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_Photo_btn_1, lv_color_hex(0xfff290), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_Photo_btn_1, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_Photo_btn_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_Photo_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of screen_Photo.

    // Update current screen layout.
    lv_obj_update_layout(ui->screen_Photo);

    // Init events for screen.
    events_init_screen_Photo(ui);
}

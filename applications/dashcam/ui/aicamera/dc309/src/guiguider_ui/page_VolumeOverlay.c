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
// #include "events_init.h"

#include "custom.h"
int volume_level = 50;

void setup_scr_screen_VolumeOverlay(lv_ui_t *ui)
{
    // Write codes screen_VolumeOverlay
    ui->screen_VolumeOverlay = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_VolumeOverlay, 640, 480);
    lv_obj_set_scrollbar_mode(ui->screen_VolumeOverlay, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_VolumeOverlay, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_VolumeOverlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_VolumeOverlay_label
    ui->screen_VolumeOverlay_label = lv_label_create(ui->screen_VolumeOverlay);
    lv_obj_set_pos(ui->screen_VolumeOverlay_label, 261, 103);
    lv_obj_set_size(ui->screen_VolumeOverlay_label, 100, 32);
    lv_label_set_text(ui->screen_VolumeOverlay_label, "Label");
    lv_label_set_long_mode(ui->screen_VolumeOverlay_label, LV_LABEL_LONG_WRAP);

    // Write style for screen_VolumeOverlay_label, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_VolumeOverlay_label, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_VolumeOverlay_label, &lv_font_montserratMedium_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_VolumeOverlay_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_VolumeOverlay_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_VolumeOverlay_label, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_VolumeOverlay_bar
    ui->screen_VolumeOverlay_bar = lv_bar_create(ui->screen_VolumeOverlay);
    lv_obj_set_pos(ui->screen_VolumeOverlay_bar, 275, 230);
    lv_obj_set_size(ui->screen_VolumeOverlay_bar, 90, 20);
    lv_obj_set_style_anim_duration(ui->screen_VolumeOverlay_bar, 1000, 0);
    lv_bar_set_mode(ui->screen_VolumeOverlay_bar, LV_BAR_MODE_NORMAL);
    lv_bar_set_range(ui->screen_VolumeOverlay_bar, 0, 100);
    lv_bar_set_value(ui->screen_VolumeOverlay_bar, 50, LV_ANIM_OFF);

    // Write style for screen_VolumeOverlay_bar, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_VolumeOverlay_bar, 60, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_VolumeOverlay_bar, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_VolumeOverlay_bar, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_VolumeOverlay_bar, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_VolumeOverlay_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style for screen_VolumeOverlay_bar, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_VolumeOverlay_bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_VolumeOverlay_bar, lv_color_hex(0x2195f6),
                              LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_VolumeOverlay_bar, LV_GRAD_DIR_NONE, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_VolumeOverlay_bar, 10, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    // The custom code of screen_VolumeOverlay.
    // Write codes screen_VolumeOverlay
    // Check if the object already exists (possibly created by Gui Guider) and delete it
    if(ui && ui->screen_VolumeOverlay) {
        lv_obj_del(ui->screen_VolumeOverlay);
        ui->screen_VolumeOverlay = NULL; // Set pointer to NULL after deletion
    }

    // Create the overlay object on the top layer
    ui->screen_VolumeOverlay = lv_obj_create(lv_layer_top()); // Create on top layer
    lv_obj_set_size(ui->screen_VolumeOverlay, 640, 480);      // Full screen size for overlay
    lv_obj_set_scrollbar_mode(ui->screen_VolumeOverlay, LV_SCROLLBAR_MODE_OFF);

    // Set overlay background to transparent
    lv_obj_set_style_bg_opa(ui->screen_VolumeOverlay, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    // No background color needed if fully transparent
    // lv_obj_set_style_bg_color(ui->screen_VolumeOverlay, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_flag(ui->screen_VolumeOverlay, LV_OBJ_FLAG_EVENT_BUBBLE, true);
    // Hide the overlay by default
    lv_obj_add_flag(ui->screen_VolumeOverlay, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(
        ui->screen_VolumeOverlay,
        LV_OBJ_FLAG_CLICKABLE); //若父对象覆盖在其他可点击对象上方，移除其CLICKABLE标志后，点击事件会穿透到下层对象
    // Create Volume Label (child of the overlay object)
    ui->screen_VolumeOverlay_label = lv_label_create(ui->screen_VolumeOverlay);
    lv_label_set_text(ui->screen_VolumeOverlay_label, "音量: 50%");
    lv_obj_set_style_text_color(ui->screen_VolumeOverlay_label, lv_color_hex(0xFFD700), LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_VolumeOverlay_label, &lv_font_SourceHanSerifSC_Regular_16, LV_STATE_DEFAULT);
    lv_obj_align(ui->screen_VolumeOverlay_label, LV_ALIGN_LEFT_MID, 20, -120); // Position above the bar

    // Create Volume Bar (child of the overlay object)
    ui->screen_VolumeOverlay_bar = lv_bar_create(ui->screen_VolumeOverlay);
    lv_obj_set_size(ui->screen_VolumeOverlay_bar, 20, 200);               // Set size for vertical bar
    lv_obj_align(ui->screen_VolumeOverlay_bar, LV_ALIGN_LEFT_MID, 20, 0); // Align to left-middle with offset
    lv_bar_set_range(ui->screen_VolumeOverlay_bar, 0, 100);
    lv_bar_set_value(ui->screen_VolumeOverlay_bar, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(ui->screen_VolumeOverlay_bar, lv_color_hex(0x555555), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_VolumeOverlay_bar, lv_color_hex(0xFFD700),
                              LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_VolumeOverlay_bar, LV_RADIUS_CIRCLE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_VolumeOverlay_bar, LV_RADIUS_CIRCLE, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(ui->screen_VolumeOverlay);
}

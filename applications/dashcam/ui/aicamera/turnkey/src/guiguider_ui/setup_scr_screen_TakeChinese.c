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
#include <inttypes.h>
#include "gui_guider.h"
#include "events_init.h"

#include "custom.h"
// 底部倍数按钮点击事件回调
static lv_obj_t *takech_btns[3] = {NULL};
static void takech_btn_event_cb(lv_event_t *e)
{
    int idx = (int)lv_event_get_user_data(e);
    for(int i = 0; i < 3; ++i) {
        lv_obj_set_style_bg_color(takech_btns[i], i == idx ? lv_color_hex(0xFFD600) : lv_color_hex(0xCCCCCC), 0);
    }
}

// 闪光灯按钮点击事件回调（三态：自动、打开、关闭）
static int flash_mode = 0; // 0=自动, 1=打开, 2=关闭
static void takech_flash_btn_event_cb(lv_event_t *e)
{
    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    flash_mode  = (flash_mode + 1) % 3;
    if(flash_mode == 0) { // 自动
        lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_flash, lv_color_hex(0xCCCCCC), 0);
        lv_label_set_text(ui->screen_TakeChinese_btn_flash_label, LV_SYMBOL_CHARGE "A");
        lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_flash_label, lv_color_hex(0xFFD600), 0);
    } else if(flash_mode == 1) { // 打开
        lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_flash, lv_color_hex(0xFFD600), 0);
        lv_label_set_text(ui->screen_TakeChinese_btn_flash_label, LV_SYMBOL_CHARGE);
        lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_flash_label, lv_color_hex(0x1A1A1A), 0);
    } else { // 关闭
        lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_flash, lv_color_hex(0xCCCCCC), 0);
        lv_label_set_text(ui->screen_TakeChinese_btn_flash_label, LV_SYMBOL_CHARGE);
        lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_flash_label, lv_color_hex(0x888888), 0);
    }
}

void setup_scr_screen_TakeChinese(lv_ui_t *ui)
{
    // Write codes screen_TakeChinese
    ui->screen_TakeChinese = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_TakeChinese, 640, 480);
    lv_obj_set_scrollbar_mode(ui->screen_TakeChinese, LV_SCROLLBAR_MODE_OFF);

    // Write style for screen_TakeChinese, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_TakeChinese, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui->screen_TakeChinese, LV_OPA_0, LV_PART_MAIN);

    // Write codes screen_TakeChinese_img_box
    ui->screen_TakeChinese_img_box = lv_image_create(ui->screen_TakeChinese);
    lv_obj_set_pos(ui->screen_TakeChinese_img_box, 245, 164);
    lv_obj_set_size(ui->screen_TakeChinese_img_box, 150, 150);
    lv_obj_add_flag(ui->screen_TakeChinese_img_box, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_TakeChinese_img_box, &_mizige_yellow_RGB565A8_150x150);
    lv_image_set_pivot(ui->screen_TakeChinese_img_box, 50, 50);
    lv_image_set_rotation(ui->screen_TakeChinese_img_box, 0);

    // Write style for screen_TakeChinese_img_box, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_TakeChinese_img_box, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_TakeChinese_img_box, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // The custom code of screen_TakeChinese.

    // 顶部栏
    ui->screen_TakeChinese_cont_top = lv_obj_create(ui->screen_TakeChinese);
    lv_obj_set_pos(ui->screen_TakeChinese_cont_top, 0, 0);
    lv_obj_set_size(ui->screen_TakeChinese_cont_top, 640, 50);
    lv_obj_set_scrollbar_mode(ui->screen_TakeChinese_cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_TakeChinese_cont_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_TakeChinese_cont_top, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_TakeChinese_cont_top, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_TakeChinese_cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 顶部返回按钮
    ui->screen_TakeChinese_btn_back = lv_btn_create(ui->screen_TakeChinese_cont_top);
    lv_obj_set_pos(ui->screen_TakeChinese_btn_back, 8, 5);
    lv_obj_set_size(ui->screen_TakeChinese_btn_back, 40, 40);
    lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_TakeChinese_btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_TakeChinese_btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui->screen_TakeChinese_btn_back_label = lv_label_create(ui->screen_TakeChinese_btn_back);
    lv_label_set_text(ui->screen_TakeChinese_btn_back_label, LV_SYMBOL_LEFT);
    lv_obj_align(ui->screen_TakeChinese_btn_back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_back_label, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_TakeChinese_btn_back_label, &lv_font_SourceHanSerifSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // // 顶部右侧拍照按钮
    // // ui->screen_TakeChinese_btn_shutter = lv_btn_create(ui->screen_TakeChinese_cont_top);
    // lv_obj_set_pos(ui->screen_TakeChinese_btn_shutter, 640-56, 5);
    // lv_obj_set_size(ui->screen_TakeChinese_btn_shutter, 40, 40);
    // lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_shutter, lv_color_hex(0xFFD600),
    // LV_PART_MAIN|LV_STATE_DEFAULT); lv_obj_set_style_radius(ui->screen_TakeChinese_btn_shutter, 20,
    // LV_PART_MAIN|LV_STATE_DEFAULT); lv_obj_set_style_shadow_width(ui->screen_TakeChinese_btn_shutter, 0,
    // LV_PART_MAIN|LV_STATE_DEFAULT);
    // // ui->screen_TakeChinese_btn_shutter_label = lv_label_create(ui->screen_TakeChinese_btn_shutter);
    // lv_label_set_text(ui->screen_TakeChinese_btn_shutter_label, LV_SYMBOL_IMAGE);
    // lv_obj_align(ui->screen_TakeChinese_btn_shutter_label, LV_ALIGN_CENTER, 0, 0);
    // lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_shutter_label, lv_color_hex(0x1A1A1A),
    // LV_PART_MAIN|LV_STATE_DEFAULT); lv_obj_set_style_text_font(ui->screen_TakeChinese_btn_shutter_label,
    // &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN|LV_STATE_DEFAULT);

    // 底部栏
    ui->screen_TakeChinese_cont_bottom = lv_obj_create(ui->screen_TakeChinese);
    lv_obj_set_pos(ui->screen_TakeChinese_cont_bottom, 0, 410);
    lv_obj_set_size(ui->screen_TakeChinese_cont_bottom, 640, 70);
    lv_obj_set_scrollbar_mode(ui->screen_TakeChinese_cont_bottom, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_border_width(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_TakeChinese_cont_bottom, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_TakeChinese_cont_bottom, lv_color_hex(0x2A2A2A),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_TakeChinese_cont_bottom, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_TakeChinese_cont_bottom, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 底部左侧历史按钮
    ui->screen_TakeChinese_btn_history = lv_btn_create(ui->screen_TakeChinese_cont_bottom);
    lv_obj_set_pos(ui->screen_TakeChinese_btn_history, 8, 11);
    lv_obj_set_size(ui->screen_TakeChinese_btn_history, 48, 48);
    lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_history, lv_color_hex(0xFFD600),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_TakeChinese_btn_history, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_TakeChinese_btn_history, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    ui->screen_TakeChinese_btn_history_label = lv_label_create(ui->screen_TakeChinese_btn_history);
    lv_label_set_text(ui->screen_TakeChinese_btn_history_label, LV_SYMBOL_LIST);
    lv_obj_align(ui->screen_TakeChinese_btn_history_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_history_label, lv_color_hex(0x1A1A1A),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_TakeChinese_btn_history_label, &lv_font_SourceHanSerifSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // 底部bar内的圆形按钮
    const char *btn_texts[3] = {"1x", "2x", "4x"};
    int selected_idx         = 0; // 4x为选中
    int btn_count            = 3;
    int btn_size             = 48;
    int btn_spacing          = 32;
    int total_width          = btn_count * btn_size + (btn_count - 1) * btn_spacing;
    int start_x              = (640 - total_width) / 2;
    for(int i = 0; i < btn_count; ++i) {
        lv_obj_t *btn  = lv_btn_create(ui->screen_TakeChinese_cont_bottom);
        takech_btns[i] = btn;
        lv_obj_set_size(btn, btn_size, btn_size);
        lv_obj_set_style_radius(btn, btn_size / 2, 0);
        lv_obj_set_style_bg_color(btn, i == selected_idx ? lv_color_hex(0xFFD600) : lv_color_hex(0xCCCCCC), 0);
        lv_obj_set_style_bg_opa(btn, 255, 0);
        lv_obj_set_style_shadow_width(btn, 0, 0);
        lv_obj_set_pos(btn, start_x + i * (btn_size + btn_spacing), 11);
        lv_obj_add_event_cb(btn, takech_btn_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, btn_texts[i]);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x1A1A1A), 0);
        lv_obj_set_style_text_font(label, &lv_font_SourceHanSerifSC_Regular_20, 0);
    }

    // 右下角闪光灯按钮
    ui->screen_TakeChinese_btn_flash = lv_btn_create(ui->screen_TakeChinese_cont_bottom);
    lv_obj_set_size(ui->screen_TakeChinese_btn_flash, 48, 48);
    lv_obj_set_style_radius(ui->screen_TakeChinese_btn_flash, 24, 0);
    lv_obj_set_style_bg_color(ui->screen_TakeChinese_btn_flash, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_bg_opa(ui->screen_TakeChinese_btn_flash, 255, 0);
    lv_obj_set_style_shadow_width(ui->screen_TakeChinese_btn_flash, 0, 0);
    lv_obj_set_pos(ui->screen_TakeChinese_btn_flash, 640 - 56, 11);
    ui->screen_TakeChinese_btn_flash_label = lv_label_create(ui->screen_TakeChinese_btn_flash);
    lv_label_set_text(ui->screen_TakeChinese_btn_flash_label, LV_SYMBOL_CHARGE "A");
    lv_obj_align(ui->screen_TakeChinese_btn_flash_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(ui->screen_TakeChinese_btn_flash_label, lv_color_hex(0xFFD600), 0);
    lv_obj_set_style_text_font(ui->screen_TakeChinese_btn_flash_label, &lv_font_SourceHanSerifSC_Regular_20, 0);
    lv_obj_add_event_cb(ui->screen_TakeChinese_btn_flash, takech_flash_btn_event_cb, LV_EVENT_CLICKED, ui);

    // Update current screen layout.
    lv_obj_update_layout(ui->screen_TakeChinese);

    // Init events for screen.
    events_init_screen_TakeChinese(ui);
}

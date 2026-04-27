/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
#define DEBUG
#include "config.h"
#include "custom.h"
#include "gui_guider.h"
#include "indev.h"
#include "lvgl.h"
#include "page_all.h"
#include "style_common.h"
#include "ui_common.h"
#include <stdio.h>

#define GRID_COLS 1
#define GRID_ROWS 15
#define GRID_MAX_OBJECTS GRID_ROWS * GRID_COLS
static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];

extern char g_button_labelPho[32];
extern bool is_video_mode;
static uint8_t effect_Current_Index_s = 0;

const char* effect_style[] = {
    "滤镜_普通.png", "滤镜_黑白.png", "滤镜_胶片.png", "滤镜_鲜艳.png",
    "滤镜_绿色.png", "滤镜_红外.png", "滤镜_高饱和度.png", "滤镜_低饱和度.png"
};

const char* effect_style_small[] = {
    "滤镜_普通_small.png", "滤镜_黑白_small.png", "滤镜_胶片_small.png", "滤镜_鲜艳_small.png",
    "滤镜_绿色_small.png", "滤镜_红外_small.png", "滤镜_高饱和度_small.png", "滤镜_低饱和度_small.png"
};

uint8_t geteffect_index(void)
{
    return effect_Current_Index_s / 2;
}

void seteffect_index(uint8_t index)
{
    effect_Current_Index_s = index * 2;
}

void reset_effect(void)
{
    extern uint32_t g_current_effect_index;
    g_current_effect_index = 0;
    seteffect_index(0);
    strncpy(g_button_labelPho, str_language_normal[get_curr_language()], sizeof(g_button_labelPho));
}

static void effect_Del_Complete_anim_cb(lv_anim_t *a)
{
    if(ui_Get_PreScreen() == g_ui.page_photoMenu_Setting.menuscr) {
        ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
                                &g_ui.screen_SettingPhotoEffect_del, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                false, true);
    } else if(ui_Get_PreScreen() == obj_vedioMenu_s) {
        ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, &g_ui.screen_SettingPhotoEffect_del, vedioMenu_Setting,
                                LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
    }
}

static void effect_win_Delete_anim(void)
{
    lv_anim_t Delete_anim; //动画渐隐句柄
    // 创建透明度动画
    lv_anim_init(&Delete_anim);
    lv_anim_set_values(&Delete_anim, 0, 1);

    lv_anim_set_time(&Delete_anim, 6);

    // lv_anim_set_exec_cb(&Delete_anim, AIanim_objSet_Opa);
    lv_anim_set_path_cb(&Delete_anim, lv_anim_path_ease_out);
    // 设置动画完成回调（销毁对象）
    lv_anim_set_completed_cb(&Delete_anim, effect_Del_Complete_anim_cb);

    lv_anim_start(&Delete_anim);
}

static void screen_SettingPhotoEffect_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {

            if(ui_Get_PreScreen() == g_ui.page_photoMenu_Setting.menuscr) {
                ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
                                      &g_ui.screen_SettingPhotoEffect_del, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0,
                                      0, false, true);
            } else if(ui_Get_PreScreen() == obj_vedioMenu_s) {
                ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, &g_ui.screen_SettingPhotoEffect_del,
                                      vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            }

            break;
        }
        default: break;
    }
}

void events_init_screen_SettingPhotoEffect(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_photoeffect.btn_back, screen_SettingPhotoEffect_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

void photoEffect_SelectFocus_OK(lv_event_t *e, lv_obj_t *obj)
{
    lv_obj_t *btn_clicked = NULL;
    if(obj == NULL) {
        btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
    } else {
        btn_clicked = obj;
    }
    lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
    //获取焦点控件
    lv_obj_t *chlid = lv_obj_get_child(parent, effect_Current_Index_s);

    for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
        if(i == effect_Current_Index_s) {
            //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
            lv_group_focus_obj(chlid);
            lv_obj_add_state(chlid, LV_STATE_FOCUS_KEY);
            //设置焦点渐变
            // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
            //设置焦点BG
            // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            //设置焦点标签颜色
            lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20),
                                        LV_PART_MAIN | LV_STATE_DEFAULT);
            // lv_obj_set_style_text_color(lv_obj_get_child(chlid,1), lv_color_hex(0xFFFFFF), LV_PART_MAIN |
            // LV_STATE_DEFAULT);

        } else {
            // lv_obj_t *other_child = lv_obj_get_child(parent, i);
            // //设置焦点渐变
            // lv_set_obj_grad_style(other_child, LV_GRAD_DIR_VER, lv_color_hex(0), lv_color_hex(0));
            // //设置焦点BG
            // lv_obj_set_style_bg_color(other_child, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        if((btn_clicked == lv_obj_get_child(parent, i))) {
            if(lv_obj_get_child(btn_clicked, 1) == NULL) {
                lv_obj_t *label1 = lv_label_create(btn_clicked);
                lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
            }
        } else {
            lv_obj_t *child = lv_obj_get_child(parent, i);
            if(lv_obj_get_child(child, 1) != NULL) {
                lv_obj_del(lv_obj_get_child(child, 1));
            }
        }
    }
}

// 添加按钮事件处理函数
static void screen_SettingPhotoEffect_btn_event_handler(lv_event_t *e)
{

    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
            lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {

                if(lv_obj_get_child(parent, i) == btn_clicked) {
                    effect_Current_Index_s = i;
                    photoEffect_SelectFocus_OK(e, NULL);
                    // 获取按钮标签文本
                    lv_obj_t *label = lv_obj_get_child(lv_obj_get_child(parent, i), 1);
                    if(label && lv_obj_check_type(label, &lv_label_class)) {
                        const char *txt = lv_label_get_text(label);
                        if(txt) strncpy(g_button_labelPho, txt, sizeof(g_button_labelPho));

                        MLOG_DBG("switch to %d:%s\n", i, g_button_labelPho);
                        MESSAGE_S event = {0};
                        event.topic = EVENT_MODEMNG_SETTING;
                        if(is_video_mode == false) {
                            event.arg1 = PARAM_MENU_PHOTO_EFFECT;
                        } else if(is_video_mode == true) {
                            event.arg1 = PARAM_MENU_VIDEO_EFFECT;
                        }
                        event.arg2 = effect_Current_Index_s / 2; // 偶数
                        extern uint32_t g_current_effect_index;
                        g_current_effect_index = event.arg2;
                        MODEMNG_SendMessage(&event);
                    }
                    lv_obj_add_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xFF0000), LV_PART_MAIN);
                } else {
                    lv_obj_clear_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
                    lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                }
            }
            effect_win_Delete_anim();
        }; break;
        default: {
            // MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
        } break;
    }
}

static void photoEffect_click_callback(lv_obj_t *obj)
{
    MLOG_DBG("photoEffect_click_callback\n");
    lv_obj_t *parent      = lv_obj_get_parent(obj); //获取发生点击事件的父控件
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {

        if(lv_obj_get_child(parent, i) == obj) {
            effect_Current_Index_s = i;
            photoEffect_SelectFocus_OK(NULL, obj);
            // 获取按钮标签文本
            lv_obj_t *label = lv_obj_get_child(lv_obj_get_child(parent, i), 1);
            if(label && lv_obj_check_type(label, &lv_label_class)) {
                const char *txt = lv_label_get_text(label);
                if(txt) strncpy(g_button_labelPho, txt, sizeof(g_button_labelPho));

                MLOG_DBG("switch to %d:%s\n", i, g_button_labelPho);
                MESSAGE_S event = {0};
                event.topic = EVENT_MODEMNG_SETTING;
                if(is_video_mode == false) {
                    event.arg1 = PARAM_MENU_PHOTO_EFFECT;
                } else if(is_video_mode == true) {
                    event.arg1 = PARAM_MENU_VIDEO_EFFECT;
                }
                event.arg2 = effect_Current_Index_s / 2; // 偶数
                MODEMNG_SendMessage(&event);
            }
            lv_obj_add_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
            lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xFF0000), LV_PART_MAIN);
        } else {
            lv_obj_clear_state(lv_obj_get_child(parent, i), LV_STATE_PRESSED);
            lv_obj_set_style_border_color(lv_obj_get_child(parent, i), lv_color_hex(0xCCCCCC), LV_PART_MAIN);
        }
    }
    effect_win_Delete_anim();
}

static void photoEffect_menu_callback(void)
{
    MLOG_DBG("photoEffect_menu_callback\n");
    if(ui_Get_PreScreen() == g_ui.page_photoMenu_Setting.menuscr) {
        ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
                              &g_ui.screen_SettingPhotoEffect_del, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0,
                              0, false, true);
    } else if(ui_Get_PreScreen() == obj_vedioMenu_s) {
        ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, &g_ui.screen_SettingPhotoEffect_del,
                              vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
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
                case LV_DIR_RIGHT: {
                    if(ui_Get_PreScreen() == g_ui.page_photoMenu_Setting.menuscr) {
                        ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr,
                                              g_ui.screenPhotoMenuSetting_del, &g_ui.screen_SettingPhotoEffect_del,
                                              photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
                    } else if(ui_Get_PreScreen() == obj_vedioMenu_s) {
                        ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, &g_ui.screen_SettingPhotoEffect_del,
                                              vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
                    }
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}
void photoMenu_SettEffect(lv_ui_t *ui)
{
    MLOG_DBG("loading page_PhotoEffect...\n");

    SettingPhotoEffect_t *PhotoEffect = &ui->page_photoeffect;
    PhotoEffect->effect_del           = true;

    // 创建主页面1 容器
    if(PhotoEffect->effect_scr != NULL) {
        if(lv_obj_is_valid(PhotoEffect->effect_scr)) {
            MLOG_DBG("page_PhotoEffect->effect_scr 仍然有效，删除旧对象\n");
            lv_obj_del(PhotoEffect->effect_scr);
        } else {
            MLOG_DBG("page_PhotoEffect->effect_scr 已被自动销毁，仅重置指针\n");
        }
        PhotoEffect->effect_scr = NULL;
    }

    // Write codes scr
    PhotoEffect->effect_scr = lv_obj_create(NULL);
    lv_obj_set_size(PhotoEffect->effect_scr, H_RES, V_RES);
    lv_obj_add_style(PhotoEffect->effect_scr, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(PhotoEffect->effect_scr, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // Write codes scr_cont_top (顶部栏)
    PhotoEffect->cont_top = lv_obj_create(PhotoEffect->effect_scr);
    lv_obj_set_pos(PhotoEffect->cont_top, 0, 0);
    lv_obj_set_size(PhotoEffect->cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(PhotoEffect->cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(PhotoEffect->cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    PhotoEffect->btn_back = lv_button_create(PhotoEffect->cont_top);
    lv_obj_set_pos(PhotoEffect->btn_back, 4, 4);
    lv_obj_set_size(PhotoEffect->btn_back, 60, 52);
    lv_obj_add_style(PhotoEffect->btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    PhotoEffect->label_back = lv_label_create(PhotoEffect->btn_back);
    lv_label_set_text(PhotoEffect->label_back, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(PhotoEffect->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoEffect->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(PhotoEffect->label_back, LV_PCT(100));
    lv_obj_add_style(PhotoEffect->label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes screen_SettingPhotoEffect_label_title (标题)
    PhotoEffect->title = lv_label_create(PhotoEffect->cont_top);
    lv_label_set_text(PhotoEffect->title, str_language_photography_effect[get_curr_language()]);
    lv_label_set_long_mode(PhotoEffect->title, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(PhotoEffect->title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoEffect->title, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(PhotoEffect->title, LV_ALIGN_CENTER, 0, 0);
    // Write codes screen_SettingPhotoEffect_cont_settings (设置选项容器)
    PhotoEffect->cont_settings = lv_obj_create(PhotoEffect->effect_scr);
    lv_obj_set_size(PhotoEffect->cont_settings, 600, MENU_CONT_SIZE);
    lv_obj_align(PhotoEffect->cont_settings, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_bg_opa(PhotoEffect->cont_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoEffect->cont_settings, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(PhotoEffect->cont_settings, 10, 0);

    // 创建功能按钮
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

    int btn_count = sizeof(btn_labels) / sizeof(btn_labels[0]);
    static lv_point_precise_t line_points_pool[sizeof(btn_labels) / sizeof(btn_labels[0])][2];
    for(int i = 0; i < btn_count; i++) {
        lv_obj_t *btn = lv_button_create(PhotoEffect->cont_settings);
        if(!btn) continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, 60);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (60 + 10) * i);
        lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(btn, lv_color_hex(0xCCCCCC), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);

        lv_obj_t *img = lv_img_create(btn);
        lv_obj_set_size(img, 40, 40);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0,0);
        // lv_obj_align(img, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_set_style_pad_all(img, 0, LV_STATE_DEFAULT);
        // lv_img_set_src(img, effect_style[i]);
        show_image(img,effect_style_small[i]);

        lv_obj_t *label = lv_label_create(btn);
        if(!label) continue; // 如果标签创建失败则跳过

        lv_label_set_text(label, btn_labels[i]);
        lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 60, 0);
        // 添加事件处理器，传入容器对象作为用户数据
        lv_obj_add_event_cb(btn, screen_SettingPhotoEffect_btn_event_handler, LV_EVENT_ALL, PhotoEffect->cont_settings);

        if(i == effect_Current_Index_s / 2) {
            {
                lv_obj_t *label1 = lv_label_create(btn);
                lv_obj_set_style_text_color(label1, lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_label_set_text(label1, "" LV_SYMBOL_OK " ");
                lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
                lv_obj_align(label1, LV_ALIGN_RIGHT_MID, 0, 0);
            }
        }

        lv_obj_t *line = lv_line_create(PhotoEffect->cont_settings);
        int y_position = (60 + 10) * (i + 1) - 4; // 计算y坐标  //横线在下方,且第一个btn不用画线
        // 使用点数组池中的第i组
        line_points_pool[i][0].x = 10;
        line_points_pool[i][0].y = y_position;
        line_points_pool[i][1].x = 570;
        line_points_pool[i][1].y = y_position;
        lv_line_set_points(line, line_points_pool[i], 2);
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, lv_color_hex(0x5F5F5F), 0);
    }

    //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
    lv_group_focus_obj(lv_obj_get_child(PhotoEffect->cont_settings, effect_Current_Index_s));
    lv_obj_add_state(lv_obj_get_child(PhotoEffect->cont_settings, effect_Current_Index_s), LV_STATE_FOCUS_KEY);
    lv_obj_scroll_to_y(PhotoEffect->cont_settings, ((effect_Current_Index_s / 2) * 60), LV_ANIM_OFF);
    //获取焦点控件
    lv_obj_t *chlid = lv_obj_get_child(PhotoEffect->cont_settings, effect_Current_Index_s);
    //设置焦点渐变
    // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
    //设置焦点BG
    // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    //设置焦点标签颜色
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_color(lv_obj_get_child(chlid,1), lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(PhotoEffect->effect_scr);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *target_obj = lv_obj_get_child(PhotoEffect->cont_settings, effect_Current_Index_s);
    // 初始化焦点组
    init_focus_group(PhotoEffect->cont_settings, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
                     photoEffect_click_callback, target_obj);
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(photoEffect_menu_callback);

    // Update current screen layout.
    lv_obj_update_layout(PhotoEffect->effect_scr);

    // Init events for screen.
    events_init_screen_SettingPhotoEffect(ui);
}

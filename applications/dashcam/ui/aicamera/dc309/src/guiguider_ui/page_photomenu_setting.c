#define DEBUG
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
#include "page_all.h"
// #include "events_init.h"
#include "config.h"
#include "custom.h"
#include "indev.h"
#include "page_all.h"
#include "style_common.h"

#define GRID_COLS 1
#define GRID_ROWS 8
#define GRID_MAX_OBJECTS GRID_ROWS * GRID_COLS
static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];

char g_button_labelWhi[32]     = "自动";
char g_button_labelPho[32]     = "普通";
char g_button_labelExp[32]     = "EV0";
char g_button_labelAIMode[32]  = "普通模式";
char g_button_labelPicMode[32] = "自动";
char g_button_labelSel[32]     = "关闭";//延迟
char g_button_labelSho[32]     = "关闭";//连拍
char g_button_labelPicQual[32] = "普通";
char g_button_labelSen[32]     = "自动";
char g_button_labelAnti[32]    = "开启";
char g_button_labelAuto[32]    = "触控对焦";
// char g_button_labelBeau1[32]   = "";
// char g_button_labelBeau2[32]   = "美白";
char g_button_labelflash[32]   = "关闭";
char g_button_labelFace[32] = {0};
char g_button_labelSmile[32] = {0};
extern char g_button_labelRes[32];

lv_obj_t *photoSettings_cont_s;
lv_obj_t *arrowpage;
static int current_page = 0; // 当前页码
static int total_pages  = 5; // 总页数

static int32_t curr_photoScroll_Index_s = 0; // 设置焦点控件

static const char* get_localized_string(uint8_t index) {
    // 获取当前语言
    int lang = get_curr_language();

    // 修复1: 定义正确的二维指针数组类型
    const char *(*settting_str[])[NUM_LANGUAGES] = {&str_language_auto,
                                                    &str_language_normal,
                                                    &str_language_normal_mode,
                                                    &str_language_off,
                                                    &str_language_burst_shot,
                                                    &str_language_on,
                                                    &str_language_auto_focus,
                                                    &str_language_sunny,
                                                    &str_language_cloudy,
                                                    &str_language_incandescent_light,
                                                    &str_language_fluorescent,
                                                    &str_language_black_and_white,
                                                    &str_language_film,
                                                    &str_language_vivid,
                                                    &str_language_old_photo,
                                                    &str_language_red,
                                                    &str_language_green,
                                                    &str_language_sunset,
                                                    &str_language_warm_color,
                                                    &str_language_cool_color,
                                                    &str_language_overexposed,
                                                    &str_language_infrared,
                                                    &str_language_binary,
                                                    &str_language_high_saturation,
                                                    &str_language_low_saturation,
                                                    &str_language_normal_mode,
                                                    &str_language_style_transformation,
                                                    &str_language_background_replacement,
                                                    &str_language_age_transformation,
                                                    &str_language_ai_beauty,
                                                    &str_language_auto,
                                                    &str_language_beach,
                                                    &str_language_night_scene,
                                                    &str_language_landscape,
                                                    &str_language_portrait,
                                                    &str_language_sports,
                                                    &str_language_backlight,
                                                    &str_language_party,
                                                    &str_language_off,
                                                    &str_language_timer_5s,
                                                    &str_language_timer_7s,
                                                    &str_language_timer_10s,
                                                    &str_language_burst_3,
                                                    &str_language_burst_5,
                                                    &str_language_burst_7,
                                                    &str_language_touch_focus,
                                                    &str_language_other_methods,
                                                    &str_language_ultra_high_quality,
                                                    &str_language_high_quality,
                                                    &str_language_normal

    };

    char* g_button_n[] = {
        g_button_labelPho,
        g_button_labelRes,
        g_button_labelWhi,
        g_button_labelSen,
        g_button_labelExp,
        g_button_labelSel,
        g_button_labelSho,
        g_button_labelPicQual,
    };

    // 检查索引是否在有效范围内
    uint8_t array_size = sizeof(settting_str) / sizeof(settting_str[0]);
    if (index >= array_size) {
        return "";
    }

    for(uint8_t i=0;i<array_size;i++)
    {
        for(uint8_t j=0;j<NUM_LANGUAGES;j++)
        {
            // MLOG_DBG("%s %s\n", g_button_n[index], (*settting_str[i])[j]);
            if(strcmp(g_button_n[index], (*settting_str[i])[j]) == 0) {
                // 找到匹配的语言，然后返回对应的多国语言字符串
                return (*settting_str[i])[lang];
            }
        }
    }

    if(index == PHOTO_ISO) {
        return g_button_labelSen;
    }

    return "";
}


// 翻页动画回调
static void photoMenu_Scroll_anim_cb(void *var, int32_t value)
{
    // 更新内容容器的Y位置
    lv_obj_set_y(var, value);
}

// 翻页函数
void photoMenu_Scroll_to_Page(int page_index)
{
    // 边界检查（确保页码有效）
    if(page_index < 0) {
        page_index = 0;
    }
    if(page_index >= total_pages) {
        page_index = total_pages - 1;
    }

    // 跳过当前页面
    if(page_index == current_page) return;

    // 计算目标Y坐标（每页高度为屏幕高度）
    int32_t target_y = page_index * 100;

    MLOG_DBG("%d \n", target_y);

    // 创建并配置动画
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, photoSettings_cont_s);
    lv_obj_scroll_to_y(photoSettings_cont_s, target_y, LV_ANIM_ON);
    lv_anim_set_time(&anim, 400);                      // 动画时长400ms
    lv_anim_set_playback_time(&anim, 0);               // 禁用回放
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out); // 缓出动画曲线

    // 启动动画
    lv_anim_start(&anim);

    // 更新当前页码
    current_page = page_index;

    // 更新页码标签
    // lv_label_set_text_fmt(page_label, "Page %d/%d", current_page + 1, total_pages);
    lv_label_set_text_fmt(arrowpage, "%d", current_page + 1);
}

static void photoMenu_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {

            // curr_photoScroll_Index_s = 0; // 设置焦点控件归0
            ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del,
                                  &g_ui.screenPhotoMenuSetting_del, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                  true);
            break;
        }
        default: break;
    }
}

// 事件回调函数
static void left_arr_event_cb(lv_event_t *e)
{
    uint16_t btn_id = lv_buttonmatrix_get_selected_button(lv_event_get_target(e));

    if(btn_id == 0) {
        // "Text1"被点击
        photoMenu_Scroll_to_Page(current_page - 1);
    }
}
// 事件回调函数
static void right_arr_event_cb(lv_event_t *e)
{
    uint16_t btn_id = lv_buttonmatrix_get_selected_button(lv_event_get_target(e));

    if(btn_id == 0) {
        // "Text1"被点击
        photoMenu_Scroll_to_Page(current_page + 1);
    }
}

// 事件回调函数
static void vedioModeChange_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // ui_load_win_animation(label_arrowPage_s,vedioMenu_Setting,LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
            ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, &g_ui.screenPhotoMenuSetting_del, sysMenu_Setting,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
        } break;
        default: break;
    }
}

static void screen_TakePhotoSetting_All_btn_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
            lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                if(btn_clicked == lv_obj_get_child(parent, i)) {
                    curr_photoScroll_Index_s = i;
                    lv_set_obj_grad_style(lv_obj_get_child(photoSettings_cont_s, curr_photoScroll_Index_s),
                                          LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));

                    switch(i / 2) // photoSettings_cont_s 窗口多了横线控件.每两个是一个btn
                    {
                        case PHOTO_RES: //分辨率
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_resolution.resscr,
                                                  g_ui.screen_SettingResolution_del, &g_ui.screenPhotoMenuSetting_del,
                                                  menuSetting_Resolution, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        case PHOTO_WHI: //白平衡
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_whitebalance.balanscr,
                                                  g_ui.screen_SettingWhiteBalance_del, &g_ui.screenPhotoMenuSetting_del,
                                                  menuSetting_WhiteBalance, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        case PHOTO_EFFECT: //摄影效果
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_photoeffect.effect_scr,
                                                  g_ui.screen_SettingPhotoEffect_del, &g_ui.screenPhotoMenuSetting_del,
                                                  photoMenu_SettEffect, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        case PHOTO_EXPOSE: //曝光设置
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_exposure.expos_scr, g_ui.screen_SettingExposure_del,
                                                  &g_ui.screenPhotoMenuSetting_del, photoMenu_Exposure,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        // case PHOTO_AIMODE: // AI设置
                        // {
                        //     ui_load_scr_animation(&g_ui, &obj_Photo_AiMode_s, 1, &g_ui.screenPhotoMenuSetting_del,
                        //                           photoMenu_AIMode, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        // }; break;
                        // case PHOTO_PICMODE: //场景模式
                        // {
                        //     ui_load_scr_animation(&g_ui, &g_ui.page_picturemode.picmod_scr,
                        //                           g_ui.screen_SettingPictureMode_del, &g_ui.screenPhotoMenuSetting_del,
                        //                           photoMenu_PictureMode, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        // }; break;
                        case PHOTO_PICQUAL: //画质
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_picturequality.qual_scr,
                                                  g_ui.screen_SettingPictureQuality_del,
                                                  &g_ui.screenPhotoMenuSetting_del, photoMenu_PictureQuality,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        case PHOTO_ISO: //感光度
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_sensitivity.iso_scr,
                                                  g_ui.screen_SettingSensitivity_del, &g_ui.screenPhotoMenuSetting_del,
                                                  photoMenu_Sensitivity, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        case PHOTO_DELAY: // 延时
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_selfietime.self_scr, 1, NULL, photoMenu_SelfieTime, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                        case PHOTO_SHOOTMODE: // 连拍
                        {
                            ui_load_scr_animation(&g_ui, &g_ui.page_shootingmode.shoot_scr, 1, NULL, photoMenu_ShootingMode, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        }; break;
                            // case PHOTO_AUTOFOCUS: //自动对焦
                            // {
                            //     ui_load_scr_animation(&g_ui, &g_ui.page_autofocus.auto_scr,
                            //                           g_ui.screen_SettingAutofocus_del, &g_ui.screenPhotoMenuSetting_del,
                            //                           photoMenu_Autofocus, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            // }; break;
                            // case PHOTO_FACEDEC: //人脸侦测功能
                            // {
                            //     ui_load_scr_animation(&g_ui, &g_ui.page_facedectection.facedec_scr,
                            //                           g_ui.screen_SettingFaceDectection_del,
                            //                           &g_ui.screenPhotoMenuSetting_del, photoMenu_FaceDectection,
                            //                           LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            // }; break;
                            // case PHOTO_SMILEDEC: // 笑脸拍照
                            // {
                            //     ui_load_scr_animation(&g_ui, &g_ui.page_smiledectection.smile_scr,
                            //                           g_ui.screen_SettingSmileDectection_del,
                            //                           &g_ui.screenPhotoMenuSetting_del, photoMenu_SmileDectection,
                            //                           LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            // }; break;
                        }
                } else {
                    lv_set_obj_grad_style(lv_obj_get_child(photoSettings_cont_s, i), LV_GRAD_DIR_VER,
                                          lv_color_hex(0x171717), lv_color_hex(0x171717));
                }
            }

            break;
        }
        default: break;
    }
}

void events_init_menuSetting(lv_ui_t *ui)
{
    // 为返回按钮添加事件处理
    lv_obj_add_event_cb(ui->page_photoMenu_Setting.btn_menusettingback, photoMenu_btn_back_event_handler,
                        LV_EVENT_CLICKED, ui);
}

static void photomenu_setting_click_callback(lv_obj_t *obj)
{
    MLOG_DBG("photomenu_setting_click_callback\n");
    lv_obj_t *parent      = lv_obj_get_parent(obj); //获取发生点击事件的父控件
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
        if(obj == lv_obj_get_child(parent, i)) {
            curr_photoScroll_Index_s = i;
            lv_set_obj_grad_style(lv_obj_get_child(photoSettings_cont_s, curr_photoScroll_Index_s),
                                    LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));

            switch(i / 2) // photoSettings_cont_s 窗口多了横线控件.每两个是一个btn
            {
                case PHOTO_RES: //分辨率
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_resolution.resscr,
                                            g_ui.screen_SettingResolution_del, &g_ui.screenPhotoMenuSetting_del,
                                            menuSetting_Resolution, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                case PHOTO_WHI: //白平衡
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_whitebalance.balanscr,
                                            g_ui.screen_SettingWhiteBalance_del, &g_ui.screenPhotoMenuSetting_del,
                                            menuSetting_WhiteBalance, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                case PHOTO_EFFECT: //摄影效果
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_photoeffect.effect_scr,
                                            g_ui.screen_SettingPhotoEffect_del, &g_ui.screenPhotoMenuSetting_del,
                                            photoMenu_SettEffect, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                case PHOTO_EXPOSE: //曝光设置
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_exposure.expos_scr, g_ui.screen_SettingExposure_del,
                                            &g_ui.screenPhotoMenuSetting_del, photoMenu_Exposure,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                // case PHOTO_AIMODE: // AI设置
                // {
                //     ui_load_scr_animation(&g_ui, &obj_Photo_AiMode_s, 1, &g_ui.screenPhotoMenuSetting_del,
                //                             photoMenu_AIMode, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                // }; break;
                // case PHOTO_PICMODE: //场景模式
                // {
                //     ui_load_scr_animation(&g_ui, &g_ui.page_picturemode.picmod_scr,
                //                             g_ui.screen_SettingPictureMode_del, &g_ui.screenPhotoMenuSetting_del,
                //                             photoMenu_PictureMode, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                // }; break;
                case PHOTO_PICQUAL: //画质
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_picturequality.qual_scr,
                                            g_ui.screen_SettingPictureQuality_del,
                                            &g_ui.screenPhotoMenuSetting_del, photoMenu_PictureQuality,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                case PHOTO_ISO: //感光度
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_sensitivity.iso_scr,
                                            g_ui.screen_SettingSensitivity_del, &g_ui.screenPhotoMenuSetting_del,
                                            photoMenu_Sensitivity, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                case PHOTO_DELAY: // 延时
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_selfietime.self_scr, 1, NULL, photoMenu_SelfieTime, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                case PHOTO_SHOOTMODE: // 连拍
                {
                    ui_load_scr_animation(&g_ui, &g_ui.page_shootingmode.shoot_scr, 1, NULL, photoMenu_ShootingMode, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                }; break;
                // case PHOTO_AUTOFOCUS: //自动对焦
                // {
                //     ui_load_scr_animation(&g_ui, &g_ui.page_autofocus.auto_scr,
                //                             g_ui.screen_SettingAutofocus_del, &g_ui.screenPhotoMenuSetting_del,
                //                             photoMenu_Autofocus, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                // }; break;
                // case PHOTO_FACEDEC: //人脸侦测功能
                // {
                //     ui_load_scr_animation(&g_ui, &g_ui.page_facedectection.facedec_scr,
                //                             g_ui.screen_SettingFaceDectection_del,
                //                             &g_ui.screenPhotoMenuSetting_del, photoMenu_FaceDectection,
                //                             LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                // }; break;
                // case PHOTO_SMILEDEC: // 笑脸拍照
                // {
                //     ui_load_scr_animation(&g_ui, &g_ui.page_smiledectection.smile_scr,
                //                             g_ui.screen_SettingSmileDectection_del,
                //                             &g_ui.screenPhotoMenuSetting_del, photoMenu_SmileDectection,
                //                             LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                // }; break;

            }
        } else {
            lv_set_obj_grad_style(lv_obj_get_child(photoSettings_cont_s, i), LV_GRAD_DIR_VER,
                                    lv_color_hex(0x171717), lv_color_hex(0x171717));
        }
    }
}

// 菜单按键处理回调函数
static void photomenu_setting_menu_callback(void)
{
    MLOG_DBG("photomenu_setting_menu_callback\n");
    // curr_photoScroll_Index_s = 0; // 设置焦点控件归0
    ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del,
                            &g_ui.screenPhotoMenuSetting_del, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                            true);
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
                    // curr_photoScroll_Index_s = 0; // 设置焦点控件归0
                    ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del,
                                          &g_ui.screenPhotoMenuSetting_del, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                          false, true);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

void photoMenu_Setting(lv_ui_t *ui)
{

    MLOG_DBG("loading page_takephotosetting...\n");

    MenuSetting_t *MenuSetting = &ui->page_photoMenu_Setting;
    MenuSetting->menudel       = true;

    set_is_photo_back(false);

    // 创建主页面1 容器
    if(MenuSetting->menuscr != NULL) {
        if(lv_obj_is_valid(MenuSetting->menuscr)) {
            MLOG_DBG("page_TakePhotoSetting->menuscr 仍然有效，删除旧对象\n");
            lv_obj_del(MenuSetting->menuscr);
        } else {
            MLOG_DBG("page_TakePhotoSetting->menuscr 已被自动销毁，仅重置指针\n");
        }
        MenuSetting->menuscr = NULL;
    }

    // Write codes menuscr
    MenuSetting->menuscr = lv_obj_create(NULL);
    lv_obj_set_size(MenuSetting->menuscr, H_RES, V_RES);
    lv_obj_add_style(MenuSetting->menuscr, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(MenuSetting->menuscr, gesture_event_handler, LV_EVENT_GESTURE, ui);
    // Write codes cont_top (顶部栏)
    MenuSetting->cont_top = lv_obj_create(MenuSetting->menuscr);
    lv_obj_set_pos(MenuSetting->cont_top, 0, 0);
    lv_obj_set_size(MenuSetting->cont_top, 640, 60);
    lv_obj_set_style_bg_color(MenuSetting->cont_top, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(MenuSetting->cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(MenuSetting->cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_menusettingback (返回按钮)
    MenuSetting->btn_menusettingback = lv_button_create(MenuSetting->cont_top);
    lv_obj_set_pos(MenuSetting->btn_menusettingback, 4, 4);
    lv_obj_set_size(MenuSetting->btn_menusettingback, 60, 52);
    lv_obj_add_style(MenuSetting->btn_menusettingback, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    MenuSetting->label_menusettingback = lv_label_create(MenuSetting->btn_menusettingback);
    lv_label_set_text(MenuSetting->label_menusettingback, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(MenuSetting->label_menusettingback, LV_LABEL_LONG_WRAP);
    lv_obj_align(MenuSetting->label_menusettingback, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(MenuSetting->label_menusettingback, LV_PCT(100));
    lv_obj_add_style(MenuSetting->label_menusettingback, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *modephoto_btn_bg = lv_button_create(MenuSetting->cont_top);
    lv_obj_align(modephoto_btn_bg, LV_ALIGN_LEFT_MID, 90, 0);
    lv_obj_set_size(modephoto_btn_bg, 30, 30);
    lv_obj_set_style_bg_color(modephoto_btn_bg, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(modephoto_btn_bg, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(modephoto_btn_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(modephoto_btn_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(modephoto_btn_bg, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(modephoto_btn_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(modephoto_btn_bg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    show_image(modephoto_btn_bg, "拍照.png");

    // 创建设置选项容器 - 使用flex布局
    photoSettings_cont_s = lv_obj_create(MenuSetting->menuscr);
    lv_obj_set_size(photoSettings_cont_s, 600, MENU_CONT_SIZE);
    lv_obj_align(photoSettings_cont_s, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_bg_opa(photoSettings_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_shadow_width(photoSettings_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(photoSettings_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(photoSettings_cont_s, 10, 0);
    // 创建所有设置按钮
    const char *btn_labels[] = {
                                str_language_color_effect[get_curr_language()],
                                str_language_resolution[get_curr_language()],
                                str_language_white_balance[get_curr_language()],
                                str_language_iso[get_curr_language()],
                                str_language_exposure_settings[get_curr_language()],
                                str_language_delay[get_curr_language()],
                                str_language_burst_shot[get_curr_language()],
                                str_language_quality[get_curr_language()],
                                };

    const char* btn_img[] = { 
        "颜色特效_menu.png",
        "分辨率.png",
        "白平衡.png",
        "iso_菜单.png",
        "曝光.png",
        "延时_menu.png",
        "连拍_menu.png",
        "画质.png", };

    static lv_point_precise_t line_points_pool[sizeof(btn_labels) / sizeof(btn_labels[0])][2];

    for(uint32_t i = 0; i < sizeof(btn_labels) / sizeof(btn_labels[0]); i++) {

        lv_obj_t *btn = lv_button_create(photoSettings_cont_s);
        if(!btn) continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(btn, 255, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
        lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(btn, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_add_event_cb(btn, screen_TakePhotoSetting_All_btn_event_handler, LV_EVENT_CLICKED, ui);

        lv_obj_t *label = lv_label_create(btn);
        if(!label) continue; // 如果标签创建失败则跳过

        lv_obj_set_size(label, 280, MENU_BTN_SIZE - 10); // 固定宽度
        lv_label_set_text(label, btn_labels[i]);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 38, 5);
        // lv_obj_set_style_text_font(label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL); // 过长滚动

        // 创建右侧值标签
        lv_obj_t *value_label = lv_label_create(btn);
        lv_obj_set_size(value_label, 180, MENU_BTN_SIZE - 10); // 固定宽度
        lv_obj_align(value_label, LV_ALIGN_RIGHT_MID, -10, 5);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        // lv_obj_set_style_text_font(value_label, &lv_font_SourceHanSerifSC_Regular_16, LV_PART_MAIN |
        // LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(value_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL); // 过长滚动
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *line = lv_line_create(photoSettings_cont_s);
        int y_position = (MENU_BTN_SIZE + 10) * (i + 1) - 4; // 计算y坐标  //横线在下方,且第一个btn不用画线
        // 使用点数组池中的第i组
        line_points_pool[i][0].x = 10;
        line_points_pool[i][0].y = y_position;
        line_points_pool[i][1].x = 570;
        line_points_pool[i][1].y = y_position;
        lv_line_set_points(line, line_points_pool[i], 2);
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, lv_color_hex(0x5F5F5F), 0);


        lv_obj_t * img = lv_image_create(btn);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_size(img, 32,32);
        show_image(img, btn_img[i]);


        // 设置对应的g_button_label值
        switch(i) {
            case PHOTO_RES: // 分辨率
                lv_label_set_text(value_label, photo_getRes_Label());
                break;
            case PHOTO_WHI: // 白平衡
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case PHOTO_EFFECT: // 摄影效果
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case PHOTO_ISO: // 感光度
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case PHOTO_EXPOSE: // 曝光设置
                lv_label_set_text(value_label, g_button_labelExp);
                break;
            case PHOTO_SHOOTMODE: // 连拍
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case PHOTO_DELAY: // 延时
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case PHOTO_PICQUAL: // 画质
                lv_label_set_text(value_label, get_localized_string(i));
                break;
        }
    }
    //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
    lv_group_focus_obj(lv_obj_get_child(photoSettings_cont_s, curr_photoScroll_Index_s));
    lv_obj_add_state(lv_obj_get_child(photoSettings_cont_s, curr_photoScroll_Index_s), LV_STATE_FOCUS_KEY);
    lv_obj_scroll_to_y(photoSettings_cont_s, ((curr_photoScroll_Index_s / 2) * 40), LV_ANIM_OFF);
    //获取焦点控件
    lv_obj_t *chlid = lv_obj_get_child(photoSettings_cont_s, curr_photoScroll_Index_s);
    //设置焦点渐变
    // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
    //设置焦点BG
    // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    //设置焦点标签颜色
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 1), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(MenuSetting->menuscr);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *target_obj = lv_obj_get_child(photoSettings_cont_s, curr_photoScroll_Index_s);
    // 初始化焦点组
    init_focus_group(photoSettings_cont_s, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS,
                     photomenu_setting_click_callback, target_obj);
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(photomenu_setting_menu_callback);

    // Update current screen layout.
    lv_obj_update_layout(MenuSetting->menuscr);
    events_init_menuSetting(ui);
}

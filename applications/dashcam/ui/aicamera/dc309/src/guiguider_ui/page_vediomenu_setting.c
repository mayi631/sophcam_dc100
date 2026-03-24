/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
// #define DEBUG
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

lv_obj_t *label_arrowPage_s;

lv_obj_t *obj_vedioMenu_s;
lv_obj_t *vedioSettings_cont_s;
static int current_page = 0; // 当前页码
static int total_pages  = 2; // 总页数

static int32_t curr_vedioScroll_Index_s = 0; // 设置焦点控件

char g_vediobtn_labelGraphy[32]    = "普通"; //摄影
char g_vediobtn_labelTimelapse[32] = "0";    //缩时录影
extern char g_button_labelExp[32];           //曝光
extern char g_button_labelWhi[32];           //白平衡
extern char g_button_labelPho[32];           //滤镜特效
char g_vediobtn_labelSharpness[32] = "普通"; //锐度
extern char g_vediobtn_labelRes[32];         //分辨率
extern char g_button_labelSen[32];           //感光度
extern char g_sysbtn_labelcursor[32];       //光标
struct button_setting {
    const char *button_name;//名称 
    const char *button_label;//右侧标签
    char *button_img;//图片
};

struct button_setting vedio_button_settings[] = {
    {"颜色特效", g_button_labelPho, "颜色特效_menu.png" },
    { "分辨率", g_vediobtn_labelRes, "分辨率.png" },
    {"摄影模式", g_vediobtn_labelGraphy, "摄影.png" },
    // {"缩时录像", "", "索时摄影.png" },
    {"感光度", g_button_labelSen, "iso_菜单.png" },
    { "曝光", g_button_labelExp, "曝光.png" },
    { "白平衡", g_button_labelWhi, "白平衡.png" },
    { "锐度", g_vediobtn_labelSharpness, "锐化.png" },
    {"光标", g_sysbtn_labelcursor, "光标.png" },
};

#define VEDIO_BUTTON_COUNT (sizeof(vedio_button_settings) / sizeof(vedio_button_settings[0]))

void video_language_setting(void)
{
    vedio_button_settings[VEDIO_EFFECT].button_name = str_language_color_effect[get_curr_language()];
    vedio_button_settings[VEDIO_RES].button_name = str_language_resolution[get_curr_language()];
    vedio_button_settings[VEDIO_GRAPHY].button_name = str_language_photography[get_curr_language()];
    // vedio_button_settings[VEDIO_LAPSE_TIME].button_name = str_language_timelapse_photography[get_curr_language()];
    vedio_button_settings[VEDIO_ISO].button_name = str_language_iso[get_curr_language()];
    vedio_button_settings[VEDIO_EXPOSE].button_name = str_language_exposure[get_curr_language()];
    vedio_button_settings[VEDIO_WHITE_BLA].button_name = str_language_white_balance[get_curr_language()];
    vedio_button_settings[VEDIO_SHARPNESS].button_name = str_language_sharpness[get_curr_language()];
    vedio_button_settings[VEDIO_CURSOR].button_name = str_language_cursor[get_curr_language()];
}

static const char* get_localized_string(uint8_t index) {
    // 获取当前语言
    int lang = get_curr_language();
    
    // 修复1: 定义正确的二维指针数组类型
    const char *(*settting_str[])[NUM_LANGUAGES] = {&str_language_auto,
                                                    &str_language_normal,
                                                    &str_language_off,
                                                    &str_language_on,
                                                    &str_language_timelapse_photography,
                                                    &str_language_timed_photo ,
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
                                                    &str_language_sharp,
                                                    &str_language_normal,
                                                    &str_language_soft
    };

    char* g_button_n[] = {
        g_button_labelPho,
        g_vediobtn_labelRes,
        g_vediobtn_labelGraphy, 
        // g_vediobtn_labelTimelapse,
        g_button_labelSen,
        g_button_labelExp,
        g_button_labelWhi,
        g_vediobtn_labelSharpness,
        g_sysbtn_labelcursor,
    };

    // 检查索引是否在有效范围内
    uint8_t array_size = sizeof(settting_str) / sizeof(settting_str[0]);
    if (index >= array_size) {
        return NULL;
    }

    for(uint8_t i=0;i<array_size;i++)
    {
        for(uint8_t j=0;j<NUM_LANGUAGES;j++)
        {
            if(strcmp(g_button_n[index], (*settting_str[i])[j]) == 0) {
                // 找到匹配的语言，然后返回对应的多国语言字符串
                return (*settting_str[i])[lang];
            }
        }
    }
    if(index == VEDIO_ISO) {
        return g_button_labelSen;
    }

    return NULL;
}

// 事件回调函数
static void photoModeChange_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &obj_sysMenu_Setting_s, 1, NULL, sysMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                  false, true);
        } break;
        default: break;
    }
}

static void vedioMenu_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, NULL, Home_Vedio, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

// 翻页函数
void vedioMenu_Scroll_to_Page(int page_index)
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
    lv_anim_set_var(&anim, vedioSettings_cont_s);
    lv_obj_scroll_to_y(vedioSettings_cont_s, target_y, LV_ANIM_ON);
    lv_anim_set_time(&anim, 400);                      // 动画时长400ms
    lv_anim_set_playback_time(&anim, 0);               // 禁用回放
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_out); // 缓出动画曲线

    // 启动动画
    lv_anim_start(&anim);

    // 更新当前页码
    current_page = page_index;

    // 更新页码标签
    // lv_label_set_text_fmt(page_label, "Page %d/%d", current_page + 1, total_pages);
    lv_label_set_text_fmt(label_arrowPage_s, "%d", current_page + 1);
}

// 事件回调函数
static void left_arr_event_cb(lv_event_t *e)
{
    uint16_t btn_id = lv_buttonmatrix_get_selected_button(lv_event_get_target(e));

    if(btn_id == 0) {
        // "Text1"被点击
        vedioMenu_Scroll_to_Page(current_page - 1);
    }
}
// 事件回调函数
static void right_arr_event_cb(lv_event_t *e)
{
    uint16_t btn_id = lv_buttonmatrix_get_selected_button(lv_event_get_target(e));

    if(btn_id == 0) {
        // "Text1"被点击
        vedioMenu_Scroll_to_Page(current_page + 1);
    }
}

static void ALL_Select_Item_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
            lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                if(btn_clicked == lv_obj_get_child(parent, i)) {
                    curr_vedioScroll_Index_s = i;
                    lv_set_obj_grad_style(lv_obj_get_child(parent, curr_vedioScroll_Index_s), LV_GRAD_DIR_VER,
                                          lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
                    switch(i / 2) {
                        case VEDIO_RES: // 分辨率
                            ui_load_scr_animation(&g_ui, &obj_vedio_Res_s, 1, NULL, vedioMenuSetting_Resolution,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case VEDIO_GRAPHY: // 摄影
                            ui_load_scr_animation(&g_ui, &obj_Vedio_Graphy_s, 1, NULL, vedioMenu_Graphy,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);

                            break;
                        case VEDIO_EXPOSE: // 曝光
                            ui_load_scr_animation(&g_ui, &g_ui.page_exposure.expos_scr, g_ui.screen_SettingExposure_del,
                                                  NULL, photoMenu_Exposure, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case VEDIO_WHITE_BLA: // 白平衡
                            ui_load_scr_animation(&g_ui, &g_ui.page_whitebalance.balanscr,
                                                  g_ui.screen_SettingWhiteBalance_del, NULL, menuSetting_WhiteBalance,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case VEDIO_SHARPNESS: // 锐度

                            ui_load_scr_animation(&g_ui, &obj_Vedio_Sharpness_s, 1, NULL, vedioMenu_Sharpness,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);

                            break;
                        case VEDIO_EFFECT: // 滤镜选择
                            ui_load_scr_animation(&g_ui, &g_ui.page_photoeffect.effect_scr,
                                                  g_ui.screen_SettingPhotoEffect_del, NULL, photoMenu_SettEffect,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case VEDIO_ISO: // 感光度
                            ui_load_scr_animation(&g_ui, &g_ui.page_sensitivity.iso_scr,
                                g_ui.screen_SettingSensitivity_del, NULL,photoMenu_Sensitivity, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case VEDIO_CURSOR: // 光标
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_cursor_s, 1, NULL, photoMenu_Cursor, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                    }
                }
            }
        }; break;
        default: break;
    }
}

static void vedioMenu_setting_click_callback(lv_obj_t *obj)
{
    MLOG_DBG("vedioMenu_setting_click_callback\n");
    lv_obj_t *parent      = lv_obj_get_parent(obj); //获取发生点击事件的父控件
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
        if(obj == lv_obj_get_child(parent, i)) {
            curr_vedioScroll_Index_s = i;
            lv_set_obj_grad_style(lv_obj_get_child(parent, curr_vedioScroll_Index_s), LV_GRAD_DIR_VER,
                                  lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
            switch(i / 2) {
                case VEDIO_RES: // 分辨率
                    ui_load_scr_animation(&g_ui, &obj_vedio_Res_s, 1, NULL, vedioMenuSetting_Resolution,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case VEDIO_GRAPHY: // 摄影
                    ui_load_scr_animation(&g_ui, &obj_Vedio_Graphy_s, 1, NULL, vedioMenu_Graphy,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);

                    break;

                case VEDIO_EXPOSE: // 曝光
                    ui_load_scr_animation(&g_ui, &g_ui.page_exposure.expos_scr, g_ui.screen_SettingExposure_del,
                                          NULL, photoMenu_Exposure, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case VEDIO_WHITE_BLA: // 白平衡
                    ui_load_scr_animation(&g_ui, &g_ui.page_whitebalance.balanscr,
                                          g_ui.screen_SettingWhiteBalance_del, NULL, menuSetting_WhiteBalance,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case VEDIO_SHARPNESS: // 锐度

                    ui_load_scr_animation(&g_ui, &obj_Vedio_Sharpness_s, 1, NULL, vedioMenu_Sharpness,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);

                    break;
                case VEDIO_EFFECT: // 滤镜选择
                    ui_load_scr_animation(&g_ui, &g_ui.page_photoeffect.effect_scr,
                                          g_ui.screen_SettingPhotoEffect_del, NULL, photoMenu_SettEffect,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case VEDIO_ISO: // 感光度
                    ui_load_scr_animation(&g_ui, &g_ui.page_sensitivity.iso_scr,
                        g_ui.screen_SettingSensitivity_del, NULL,photoMenu_Sensitivity, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case VEDIO_CURSOR: // 光标
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_cursor_s, 1, NULL, photoMenu_Cursor, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                }
        }
    }
}

// 菜单按键处理回调函数
static void vedioMenu_setting_menu_callback(void)
{
    MLOG_DBG("vedioMenu_setting_menu_callback\n");
    ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, NULL, Home_Vedio, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
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
                    ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, NULL, Home_Vedio, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                          true);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

void vedioMenu_Setting(lv_ui_t *ui)
{
    // Write codes menuscr
    video_language_setting();

    obj_vedioMenu_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_vedioMenu_s, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(obj_vedioMenu_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj_vedioMenu_s, 0, 0);
    // Write style for menuscr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_add_style(obj_vedioMenu_s, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(obj_vedioMenu_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // Write codes cont_top (顶部栏)
    lv_obj_t *cont_top = lv_obj_create(obj_vedioMenu_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_menuback (返回按钮)
    lv_obj_t* btn_menuback = lv_button_create(cont_top);
    lv_obj_set_pos(btn_menuback, 4, 4);
    lv_obj_set_size(btn_menuback, 60, 52);
    lv_obj_add_style(btn_menuback, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 为返回按钮添加事件处理
    lv_obj_add_event_cb(btn_menuback, vedioMenu_btn_back_event_handler, LV_EVENT_CLICKED, ui);

    lv_obj_t* label_menuback = lv_label_create(btn_menuback);
    lv_label_set_text(label_menuback, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_menuback, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_menuback, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(label_menuback, LV_PCT(100));
    lv_obj_add_style(label_menuback, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 录像模式
    lv_obj_t* modephoto_btn_bg = lv_button_create(cont_top);
    lv_obj_align(modephoto_btn_bg, LV_ALIGN_LEFT_MID, 90, 0);
    lv_obj_set_size(modephoto_btn_bg, 36, 36);
    lv_obj_set_style_pad_all(modephoto_btn_bg, 0, LV_STATE_DEFAULT);
    // Write style for btn_menusettingback, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(modephoto_btn_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(modephoto_btn_bg, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(modephoto_btn_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(modephoto_btn_bg, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(modephoto_btn_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(modephoto_btn_bg, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(modephoto_btn_bg, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(modephoto_btn_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(modephoto_btn_bg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    show_image(modephoto_btn_bg, "录像.png");

    // 创建设置选项容器 - 使用flex布局
    vedioSettings_cont_s = lv_obj_create(obj_vedioMenu_s);
    lv_obj_set_size(vedioSettings_cont_s, 600, MENU_CONT_SIZE);
    lv_obj_align(vedioSettings_cont_s, LV_ALIGN_TOP_MID, 0, 64);
    lv_obj_set_style_bg_opa(vedioSettings_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_shadow_width(vedioSettings_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(vedioSettings_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_flex_flow(vedioSettings_cont_s, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(vedioSettings_cont_s, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(vedioSettings_cont_s, 10, 0);
    
   static lv_point_precise_t line_points_pool[VEDIO_BUTTON_COUNT][2];
    for(uint8_t i = 0; i < VEDIO_BUTTON_COUNT ; i++) {
        lv_obj_t* btn = lv_button_create(vedioSettings_cont_s);
        if (!btn)
            continue;

        lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(btn, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(btn, ALL_Select_Item_event_handler, LV_EVENT_CLICKED, (void*)VEDIO_RES);

        lv_obj_t* label = lv_label_create(btn);
        if (!label)
            continue;

        // 使用结构体中的按钮名称
        lv_label_set_text(label, vedio_button_settings[i].button_name);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 38, 5);
        lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_size(label, 280, MENU_BTN_SIZE - 10);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);

        // 创建右侧值标签
        lv_obj_t* value_label = lv_label_create(btn);
        lv_obj_align(value_label, LV_ALIGN_RIGHT_MID, -10, 5);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(value_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE),
            LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_size(value_label, 180, MENU_BTN_SIZE - 10);
        lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t* line = lv_line_create(vedioSettings_cont_s);
        int y_position = (MENU_BTN_SIZE + 10) * (i + 1) - 4;
        line_points_pool[i][0].x = 10;
        line_points_pool[i][0].y = y_position;
        line_points_pool[i][1].x = 570;
        line_points_pool[i][1].y = y_position;
        lv_line_set_points(line, line_points_pool[i], 2);
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, lv_color_hex(0x5F5F5F), 0);

        lv_obj_t* img = lv_image_create(btn);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_size(img, 32, 32);
        // 使用结构体中的图片名称
        show_image(img, vedio_button_settings[i].button_img);

        // 设置对应的g_button_label值
        switch(i) {
            case VEDIO_RES: // 分辨率
                lv_label_set_text(value_label, video_getRes_Label());
                break;
            case VEDIO_GRAPHY: // 摄影
                lv_label_set_text(value_label,  get_localized_string(i));
                break;
            case VEDIO_ISO: // 曝光
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case VEDIO_EXPOSE: // 曝光
                lv_label_set_text(value_label, g_button_labelExp);
                break;
            case VEDIO_WHITE_BLA: // 白平衡
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case VEDIO_SHARPNESS: // 锐度
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case VEDIO_EFFECT: // 特效选择
                lv_label_set_text(value_label, get_localized_string(i));
                break;
            case VEDIO_CURSOR: // 光标
                lv_label_set_text(value_label, g_sysbtn_labelcursor);
                break;
        }
    }
    //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
    lv_group_focus_obj(lv_obj_get_child(vedioSettings_cont_s, curr_vedioScroll_Index_s));
    lv_obj_add_state(lv_obj_get_child(vedioSettings_cont_s, curr_vedioScroll_Index_s), LV_STATE_FOCUS_KEY);
    lv_obj_scroll_to_y(vedioSettings_cont_s, ((curr_vedioScroll_Index_s / 2) * MENU_BTN_SIZE), LV_ANIM_OFF);
    //获取焦点控件
    lv_obj_t *chlid = lv_obj_get_child(vedioSettings_cont_s, curr_vedioScroll_Index_s);
    //设置焦点渐变
    // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
    // //设置焦点BG
    // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    //设置焦点标签颜色
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 1), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(obj_vedioMenu_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    // // 在左侧方方添加一条分割线
    // lv_obj_t *left_line = lv_line_create(obj_vedioMenu_s);
    // static lv_point_precise_t points_line1[] = { {10, 60}, {10, 460} };
    // lv_line_set_points(left_line, points_line1, 2);
    // lv_obj_set_style_line_width(left_line, 2, 0);
    // lv_obj_set_style_line_color(left_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *target_obj = lv_obj_get_child(vedioSettings_cont_s, curr_vedioScroll_Index_s);

    static lv_obj_t* focusable_objects[GRID_COLS * VEDIO_BUTTON_COUNT];

    init_focus_group(vedioSettings_cont_s, GRID_COLS, VEDIO_BUTTON_COUNT, focusable_objects,
        GRID_COLS * VEDIO_BUTTON_COUNT, vedioMenu_setting_click_callback, target_obj);

    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(vedioMenu_setting_menu_callback);

    // Update current screen layout.
    lv_obj_update_layout(obj_vedioMenu_s);
}
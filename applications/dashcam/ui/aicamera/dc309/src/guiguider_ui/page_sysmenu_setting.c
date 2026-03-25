#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"

#include "config.h"
#include "custom.h"
#include "indev.h"
#include "page_all.h"
#include "style_common.h"

#define GRID_COLS 1
#define GRID_ROWS 12
#define GRID_MAX_OBJECTS GRID_ROWS * GRID_COLS
static lv_obj_t *focusable_objects[GRID_MAX_OBJECTS];

// static lv_obj_t *syslabel_arrowPage_s;
static lv_obj_t *sysSettings_cont_s;
lv_obj_t *obj_sysMenu_Setting_s; //底层窗口

static int32_t curr_sysMenuScroll_Index_s = 0; // 设置焦点控件

char g_sysbtn_labelLanguage[32] = "简体中文"; // 语言
char g_sysbtn_labelTime[32] = "开启"; // 时间
char g_sysbtn_labelPowerdown[32] = "关闭"; // 自动关机
char g_sysbtn_labelVolume[32] = "开启"; // 动作音
char g_sysbtn_labelLightfreq[32] = "50HZ"; // 光频
char g_sysbtn_labelscreen_off[32] = "开启"; // 自动息屏
char g_sysbtn_labellight_level[32] = "level 4"; // 亮度等级
char g_sysbtn_labelstatuslight[32] = "关闭"; // 状态灯
static const char* get_localized_string(uint8_t index) {
    // 获取当前语言
    int lang = get_curr_language();
    
    // 修复1: 定义正确的二维指针数组类型
    const char *(*settting_str[])[NUM_LANGUAGES] = {&str_language_on,
                                                    &str_language_off,
                                                    &str_language_3_minutes,
                                                    &str_language_5_minutes,
                                                    &str_language_10_minutes,
    };

   char *current_values[] = {
        g_sysbtn_labelTime,      // 索引0
        g_sysbtn_labelPowerdown, // 索引1
        g_sysbtn_labelscreen_off,
        g_sysbtn_labelVolume,    // 索引2  
        g_sysbtn_labelLightfreq, // 索引3
        g_sysbtn_labelstatuslight,
    };

    // 检查索引是否在有效范围内
    uint8_t array_size = sizeof(settting_str) / sizeof(settting_str[0]);

    for(uint8_t i=0;i<array_size;i++)
    {
        for(uint8_t j=0;j<NUM_LANGUAGES;j++)
        {
            if(strcmp(current_values[index], (*settting_str[i])[j]) == 0) {
                // 找到匹配的语言，然后返回对应的多国语言字符串
                return (*settting_str[i])[lang];
            }
        }
    }
    return current_values[index];
}


static void sysMenu_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // if(homeMode_Get() == PHOTO_MODE) {
            //     ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
            //                           NULL, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            // } else if(homeMode_Get() == VEDIO_MODE) {
            //     ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
            //                           false, true);
            // };
            ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1,
                                  LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void sysMenuALL_Select_Item_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *btn_clicked = lv_event_get_target(e);         //获取发生点击事件的控件
            lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件
            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                if(btn_clicked == lv_obj_get_child(parent, i)) {
                    curr_sysMenuScroll_Index_s = i;
                    lv_set_obj_grad_style(lv_obj_get_child(parent, curr_sysMenuScroll_Index_s), LV_GRAD_DIR_VER,
                                          lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));

                    switch(i / 2) {
                        case SYSMENU_LANGUAGE: // 语言
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_Language_s, 1, NULL, sysMenu_Language,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_TIME: //时间和日期
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_Time_s, 1, NULL, sysMenu_Time,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_POWERDOWN: // 自动关闭
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_PowerDown_s, 1, NULL, sysMenu_PowerDown,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_SCREEN_OFF: // 自动关闭
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_screenoff_s, 1, NULL, sysMenu_screenoff,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_VOLUME: // 动作音
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_Volume_s, 1, NULL, sysMenu_Volume,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_LIGHTFREQ: // 光频率
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_LightFreq_s, 1, NULL, sysMenu_LightFreq,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_FORMAT: // 格式化
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_Format_s, 1, NULL, sysMenu_Format,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_FACTORYSET: // 出厂设置
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_Factory_s, 1, NULL, sysMenu_Factory,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_WIFILIST: // 出厂设置
                            ui_load_scr_animation(&g_ui, &sysMenu_WifiList_s, 1, NULL, sysMenu_WifiList,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_VERSION: // 版本信息
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_version_s, 1, NULL, version_info_page,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_LIGHTBRIGHT: // 版本信息
                            ui_load_scr_animation(&g_ui, &brightness_scr, 1, NULL, sysMenu_brightness,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;
                        case SYSMENU_STALIGHT: // 状态灯
                            ui_load_scr_animation(&g_ui, &obj_sysMenu_statuslight_s, 1, NULL, sysMenu_stlight,
                                                  LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                            break;

                    }
                }
            }
        }; break;
        default: break;
    }
}

static void sysmenu_click_callback(lv_obj_t *obj)
{
    MLOG_DBG("sysmenu_click_callback\n");
    lv_obj_t *parent      = lv_obj_get_parent(obj); //获取发生点击事件的父控件
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
        if(obj == lv_obj_get_child(parent, i)) {
            curr_sysMenuScroll_Index_s = i;
            lv_set_obj_grad_style(lv_obj_get_child(parent, curr_sysMenuScroll_Index_s), LV_GRAD_DIR_VER,
                                    lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));

            switch(i / 2) {
                case SYSMENU_LANGUAGE: // 语言
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_Language_s, 1, NULL, sysMenu_Language,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_TIME: //时间和日期
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_Time_s, 1, NULL, sysMenu_Time,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_POWERDOWN: // 自动关闭
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_PowerDown_s, 1, NULL, sysMenu_PowerDown,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_SCREEN_OFF: // 自动关闭
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_screenoff_s, 1, NULL, sysMenu_screenoff,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_VOLUME: // 动作音
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_Volume_s, 1, NULL, sysMenu_Volume,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_LIGHTFREQ: // 光频率
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_LightFreq_s, 1, NULL, sysMenu_LightFreq,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_FORMAT: // 格式化
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_Format_s, 1, NULL, sysMenu_Format,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_FACTORYSET: // 出厂设置
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_Factory_s, 1, NULL, sysMenu_Factory,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_WIFILIST: // 出厂设置
                    ui_load_scr_animation(&g_ui, &sysMenu_WifiList_s, 1, NULL, sysMenu_WifiList,
                                            LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_VERSION: // 版本信息
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_version_s, 1, NULL, version_info_page,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_LIGHTBRIGHT: // 版本信息
                    ui_load_scr_animation(&g_ui, &brightness_scr, 1, NULL, sysMenu_brightness,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;
                case SYSMENU_STALIGHT: // 状态灯
                    ui_load_scr_animation(&g_ui, &obj_sysMenu_statuslight_s, 1, NULL, sysMenu_stlight,
                                          LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                    break;

            }
        }
    }
}

static void sysmenu_menu_callback(void)
{
    MLOG_DBG("sysmenu_menu_callback\n");
    ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1,
        LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
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
                    ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                          false, true);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

void sysMenu_Setting(lv_ui_t *ui)
{
    // Write codes menuscr
    obj_sysMenu_Setting_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_sysMenu_Setting_s, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(obj_sysMenu_Setting_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(obj_sysMenu_Setting_s, 0, 0);
    // Write style for menuscr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(obj_sysMenu_Setting_s, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj_sysMenu_Setting_s, LV_OPA_100, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj_sysMenu_Setting_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(obj_sysMenu_Setting_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // Write codes cont_top (顶部栏)
    lv_obj_t *cont_top = lv_obj_create(obj_sysMenu_Setting_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, 640, 60);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(cont_top, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_menuback (返回按钮)
    lv_obj_t* btn_menuback = lv_button_create(cont_top);
    lv_obj_set_pos(btn_menuback, 4, 4);
    lv_obj_set_size(btn_menuback, 60, 52);
    lv_obj_add_style(btn_menuback, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 为返回按钮添加事件处理
    lv_obj_add_event_cb(btn_menuback, sysMenu_btn_back_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_menuback = lv_label_create(btn_menuback);
    lv_label_set_text(label_menuback, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_menuback, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_menuback, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(label_menuback, LV_PCT(100));
    lv_obj_add_style(label_menuback, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    //系统设置
    // 设置模式
    lv_obj_t *modebtn_bg = lv_button_create(cont_top);
    lv_obj_align(modebtn_bg, LV_ALIGN_LEFT_MID, 90, 0);
    lv_obj_set_size(modebtn_bg, 36, 36);
    lv_obj_set_style_bg_color(modebtn_bg, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(modebtn_bg, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(modebtn_bg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(modebtn_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(modebtn_bg, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(modebtn_bg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(modebtn_bg, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    show_image(modebtn_bg, "设置_menu.png");

    // 创建设置选项容器 - 使用flex布局
    sysSettings_cont_s = lv_obj_create(obj_sysMenu_Setting_s);
    lv_obj_add_style(sysSettings_cont_s, &style_common_cont_setting, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(sysSettings_cont_s, 600, MENU_CONT_SIZE);
    lv_obj_align(sysSettings_cont_s, LV_ALIGN_TOP_MID, 0, 64);

    const char* btn_labels[] = { 
        str_language_network_switch[get_curr_language()],
        str_language_language[get_curr_language()],
        str_language_time_and_date[get_curr_language()],
        str_language_auto_shutdown[get_curr_language()],
        str_language_auto_screen_off[get_curr_language()],
        str_language_action_sound[get_curr_language()],
        str_language_power_frequency[get_curr_language()],
        str_language_display_brightness[get_curr_language()],
        str_language_format[get_curr_language()],
        str_language_factory_settings[get_curr_language()],
        str_language_version_info[get_curr_language()],
        str_language_status_light[get_curr_language()],
    };

    const char* btn_img[] = {
        "WiFi设置.png", "语言设置.png", "时间和日期.png", "自动关机.png", "息屏.png",
        "音量设置.png", "光频.png", "显示亮度.png", "格式化.png", "恢复出厂设置.png", "版本信息.png",
        "状态灯.png"
    };

    static lv_point_precise_t line_points_pool[sizeof(btn_labels) / sizeof(btn_labels[0])][2];

    for(uint8_t i = 0; i < sizeof(btn_labels) / sizeof(btn_labels[0]); i++) {
        lv_obj_t *btn = lv_button_create(sysSettings_cont_s);
        if(!btn) continue; // 如果按钮创建失败则跳过

        lv_obj_set_size(btn, 560, MENU_BTN_SIZE);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (MENU_BTN_SIZE + 10) * i);
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(btn, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(btn, sysMenuALL_Select_Item_event_handler, LV_EVENT_CLICKED, (void *)SYSMENU_LANGUAGE);

        lv_obj_t *label = lv_label_create(btn);
        if(!label) continue; // 如果标签创建失败则跳过

        lv_obj_set_size(label, 280, MENU_BTN_SIZE - 10); // 固定宽度
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 42, 5);  // 图标右侧
        lv_label_set_text(label, btn_labels[i]);
        lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL); // 过长显示省略号

        // 右侧值标签 - 固定宽度，右对齐
        lv_obj_t *value_label = lv_label_create(btn);
        lv_obj_set_size(value_label, 180, MENU_BTN_SIZE - 10); // 固定宽度
        lv_obj_align(value_label, LV_ALIGN_RIGHT_MID, -10, 5);
        lv_obj_set_style_text_color(value_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(value_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_long_mode(value_label, LV_LABEL_LONG_SCROLL); // 过长显示省略号
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *line = lv_line_create(sysSettings_cont_s);
        int y_position = (MENU_BTN_SIZE + 10) * (i + 1) - 4; // 计算y坐标  //横线在下方,且第一个btn不用画线
        // 使用点数组池中的第i组
        line_points_pool[i][0].x = 10;
        line_points_pool[i][0].y = y_position;
        line_points_pool[i][1].x = 570;
        line_points_pool[i][1].y = y_position;
        lv_line_set_points(line, line_points_pool[i], 2);
        lv_obj_set_style_line_width(line, 2, 0);
        lv_obj_set_style_line_color(line, lv_color_hex(0x5F5F5F), 0);

         // 图标
        lv_obj_t *img = lv_image_create(btn);
        lv_obj_set_size(img, 32, 32);
        lv_obj_align(img, LV_ALIGN_LEFT_MID, 5, 0);
        show_image(img, btn_img[i]);

        // 设置对应的g_button_label值
        switch(i) {
            case SYSMENU_LANGUAGE: // 语言
                lv_label_set_text(value_label, g_sysbtn_labelLanguage);
                break;
            case SYSMENU_TIME: // 时间和日期
                lv_label_set_text(value_label, get_localized_string(0)); // 对应 str_language_on/off
                break;
            case SYSMENU_POWERDOWN: // 自动关闭  
                lv_label_set_text(value_label, get_localized_string(1)); // 对应 3/5/10分钟
                break;
            case SYSMENU_SCREEN_OFF: // 自动关闭  
                lv_label_set_text(value_label, get_localized_string(2)); // 对应 3/5/10分钟
                break;
            case SYSMENU_VOLUME: // 动作音
                lv_label_set_text(value_label, get_localized_string(3)); // 对应 str_language_on/off
                break;
            case SYSMENU_LIGHTFREQ: // 光频率
                lv_label_set_text(value_label, get_localized_string(4)); // 对应 str_language_on/off
                break;
            case SYSMENU_FORMAT: // 格式化
                lv_label_set_text(value_label, "");
                break;
            case SYSMENU_FACTORYSET: // 出厂设置
                lv_label_set_text(value_label, "");
                break;
            case SYSMENU_WIFILIST: // WIFI
                lv_label_set_text(value_label, "");
                break;
            case SYSMENU_VERSION: // 版本信息
                lv_label_set_text(value_label, "");
                break;
            case SYSMENU_LIGHTBRIGHT: //亮度等级
                lv_label_set_text(value_label,g_sysbtn_labellight_level);
                break;
            case SYSMENU_STALIGHT: //亮度等级
                lv_label_set_text(value_label,get_localized_string(5));
                break;

        }
    }

    //先设置焦点控件,再进行滚动,否则会直接滚动到最下,不知什么原因.
    lv_group_focus_obj(lv_obj_get_child(sysSettings_cont_s, curr_sysMenuScroll_Index_s));
    lv_obj_add_state(lv_obj_get_child(sysSettings_cont_s, curr_sysMenuScroll_Index_s), LV_STATE_FOCUS_KEY);
    lv_obj_scroll_to_y(sysSettings_cont_s, ((curr_sysMenuScroll_Index_s / 2) * 58), LV_ANIM_OFF);
    //获取焦点控件
    lv_obj_t *chlid = lv_obj_get_child(sysSettings_cont_s, curr_sysMenuScroll_Index_s);
    //设置焦点渐变
    // lv_set_obj_grad_style(chlid, LV_GRAD_DIR_VER, lv_color_hex(0xFBDEBD), lv_color_hex(0xF09F20));
    //设置焦点BG
    // lv_obj_set_style_bg_color(chlid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    //设置焦点标签颜色
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 0), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(lv_obj_get_child(chlid, 1), lv_color_hex(0xF09F20), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(obj_sysMenu_Setting_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    lv_obj_t *target_obj = lv_obj_get_child(sysSettings_cont_s, curr_sysMenuScroll_Index_s);
    // 初始化焦点组
    init_focus_group(sysSettings_cont_s, GRID_COLS, GRID_ROWS, focusable_objects, GRID_MAX_OBJECTS, sysmenu_click_callback, target_obj);
    // 设置当前页面的按键处理器
    set_current_page_handler(handle_grid_navigation);
    takephoto_register_menu_callback(sysmenu_menu_callback);

    // Update current screen layout.
    lv_obj_update_layout(obj_sysMenu_Setting_s);
}

/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
// #include "images_declare.h"
#include "../lvgl/src/misc/lv_timer_private.h"

// 定义界面管理器结构
typedef struct
{
    lv_obj_t *current_screen; // 当前显示的屏幕
    lv_obj_t *prev_screen;    // 上一个屏幕（上一级界面）
    lv_obj_t *nav_bar;        // 可选的导航栏（可选）
} UIScreenManager_t;

// 当前的模式，上一级模式
typedef enum {
    PHOTO_MODE = 0,
    VEDIO_MODE,
    ALBUM_MODE,
} HomeMode_t;

typedef struct
{
    lv_obj_t *scr; /* 页面容器：带4个按钮 + 底部页面标识 */
    bool del;
    lv_obj_t *imgbtn_ai_photo;
    lv_obj_t *imgbtn_photo;
    lv_obj_t *imgbtn_ai_dialog;
    lv_obj_t *imgbtn_ai_effect;

    lv_obj_t *dots_cont; /* 底部页面标识：容器带两个点 */
    lv_obj_t *dot_home1;
    lv_obj_t *dot_home2;
} PageHome1_t;

typedef struct
{
    lv_obj_t *photoscr; /* */
    bool photodel;

    lv_obj_t *img_mode;
    lv_obj_t *img_sdonline;
    lv_obj_t *redlight_level;
    lv_obj_t *img_ev;
    lv_obj_t *img_batter;
    lv_obj_t *label_batter;
    lv_obj_t *img_exit;
    lv_obj_t *img_menu;
    lv_obj_t *img_facedec;
    lv_obj_t *img_smiledec;
    lv_obj_t *img_antishake;
    lv_obj_t *label_datatime;
    lv_obj_t *label_numphoto;
    lv_obj_t *img_iso;
    lv_obj_t *img_album;
} HomePhoto_t;

/* 拍照设置页面 */
typedef struct
{
    /* 拍照设置页面 */
    lv_obj_t *menuscr;
    bool menudel;

    lv_obj_t *cont_top;
    lv_obj_t *settings_cont;
    lv_obj_t *cont_bottom;
    lv_obj_t *btn_menusettingback;
    lv_obj_t *label_menusettingback;
    lv_obj_t *btn_modephoto;
    lv_obj_t *btn_setting;
    lv_obj_t *btn_modeplayback;

    lv_obj_t *btn_resluation;
    lv_obj_t *btn_whitebalance;
    lv_obj_t *btn_effect;
    lv_obj_t *btn_exposure;
    lv_obj_t *btn_picmode;
    lv_obj_t *btn_timelapse;
    lv_obj_t *btn_shootingmode;
    lv_obj_t *btn_quality;
    lv_obj_t *btn_isosensitivity;
    lv_obj_t *btn_Antishake;
    lv_obj_t *btn_autofocus;
    lv_obj_t *btn_facedec;
    lv_obj_t *btn_slimefacedec;
    lv_obj_t *btn_beauty;

} MenuSetting_t;

/* 拍照设置页面 子页面分辨率设置*/
typedef struct
{

    lv_obj_t *resscr; /* 拍照设置页面 -- 子页面分辨率设置  5M.7M.12M.16M.24M.36M.48M.64M*/
    bool resdel;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *title;
    lv_obj_t *cont_12;
    lv_obj_t *btn_back0;
    lv_obj_t *label_back0;
    lv_obj_t *btn_9;
    lv_obj_t *label_9;
    lv_obj_t *btn_8;
    lv_obj_t *label_8;
    lv_obj_t *btn_7;
    lv_obj_t *label_7;
    lv_obj_t *btn_6;
    lv_obj_t *label_6;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
} SettingResolution_t;

/* 拍照设置页面 -- 白平衡设置*/
typedef struct
{

    lv_obj_t *balanscr; /* 拍照设置页面 -- 白平衡设置 自动.晴天.阴天.白炽灯.荧光*/
    bool balandel;
    lv_obj_t *cont_3;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *btn_6;
    lv_obj_t *label_6;
    lv_obj_t *btn_7;
    lv_obj_t *label_7;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} SettingWhiteBalance_t;

/* 拍照设置页面 -- 特效（滤镜）设置*/
typedef struct
{

    lv_obj_t *effect_scr; /*拍照设置页面--特效设置容器
                             普通.黑白.老照片.红色.绿色.日落.暖色.冷色.过曝.红外线.二值.高饱和.低饱和*/
    bool effect_del;
    lv_obj_t *cont_settings;
    lv_obj_t *btn_back0;
    lv_obj_t *label_back0;
    lv_obj_t *btn_9;
    lv_obj_t *label_9;
    lv_obj_t *btn_8;
    lv_obj_t *label_8;
    lv_obj_t *btn_7;
    lv_obj_t *label_7;
    lv_obj_t *btn_6;
    lv_obj_t *label_6;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *btn_back1;
    lv_obj_t *label_back1;
    lv_obj_t *btn_back2;
    lv_obj_t *label_back2;
    lv_obj_t *btn_back3;
    lv_obj_t *label_back3;
    lv_obj_t *btn_back4;
    lv_obj_t *label_back4;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} SettingPhotoEffect_t;

/* 拍照设置页面 -- 曝光设置*/
typedef struct
{

    lv_obj_t *expos_scr; //曝光页面容器, ev{"+3", "+2", "+1", "0", "-1", "-2", "-3"};
    bool expos_del;
    lv_obj_t *canvas_1;
    lv_obj_t *cont_top;
    lv_obj_t *btn_9;
    lv_obj_t *label_9;
    lv_obj_t *btn_8;
    lv_obj_t *label_8;
    lv_obj_t *btn_7;
    lv_obj_t *label_7;
    lv_obj_t *btn_6;
    lv_obj_t *label_6;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_30;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} SettingExposure_t;

/* 拍照设置页面 -- 自动对焦设置*/
typedef struct
{
    lv_obj_t *auto_scr; //自动对焦设置，{"触控对焦", "其他方式"};
    bool auto_del;
    lv_obj_t *cont_settings;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingAutofocus_t;

/* 拍照设置页面 -- 自拍时间*/
typedef struct
{
    lv_obj_t *self_scr; //自拍时长设置 {"定时2s", "定时5s", "定时10s"};
    bool self_del;
    lv_obj_t *cont_34;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingSelfieTime_t;

/* 拍照设置页面 -- 场景模式设置*/
typedef struct
{
    lv_obj_t *picmod_scr; //画面设置页面容器，{"自动", "海滩", "夜景", "风景", "人物", "运动", "背光", "派对"}
    bool picmod_del;
    lv_obj_t *cont_32;
    lv_obj_t *btn_8;
    lv_obj_t *label_8;
    lv_obj_t *btn_7;
    lv_obj_t *label_7;
    lv_obj_t *btn_6;
    lv_obj_t *label_6;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *btn_9;
    lv_obj_t *label_9;
    lv_obj_t *cont_30;
    lv_obj_t *cont_top;
    lv_obj_t *cont_bottom;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} SettingPictureMode_t;

/* 拍照设置页面 -- 拍摄模式设置（单张，连拍）*/
typedef struct
{

    lv_obj_t *shoot_scr; /*拍摄模式页面容器（单张，连拍）*/
    bool shoot_del;
    lv_obj_t *cont_35;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingShootingMode_t;

/* 拍照设置页面 -- 画质设置*/
typedef struct
{

    lv_obj_t *qual_scr; /*画质设置页面容器 {"超高画质", "高画质", "普通"};*/
    bool qual_del;
    lv_obj_t *cont_36;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingPictureQuality_t;

/* 拍照设置页面 -- ISO感光度设置*/
typedef struct
{

    lv_obj_t *iso_scr; //感光度页面容器，{"自动", "iso100", "isp200", "isp400"}
    bool iso_del;
    lv_obj_t *cont_3;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *btn_5;
    lv_obj_t *label_5;
    lv_obj_t *cont_2;
    lv_obj_t *cont_top;
    lv_obj_t *cont_bottom;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingSensitivity_t;

/* 拍照设置页面 -- 防抖开关设置*/
typedef struct
{

    lv_obj_t *shake_scr; //防抖开关页面容器，{"开启", "关闭"};
    bool shake_del;
    lv_obj_t *cont_settings;
    lv_obj_t *btn_4;
    lv_obj_t *label_4;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingAntiShake_t;

/* 拍照设置页面 -- 人脸侦测*/
typedef struct
{
    lv_obj_t *facedec_scr; //人脸侦测页面容器，{"开启", "关闭"};
    bool facedec_del;
    lv_obj_t *cont_settings;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingFaceDectection_t;

/* 拍照设置页面 -- 笑脸侦测*/
typedef struct
{

    lv_obj_t *smile_scr; //笑脸侦测页面容器，{"开启", "关闭"};
    bool smile_del;
    lv_obj_t *cont_3;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingSmileDectection_t;

/* 拍照设置页面 -- 美颜*/
typedef struct
{
    lv_obj_t *beauty_scr; //美颜页面容器，{"磨皮", "美白"};
    bool beauty_del;
    lv_obj_t *cont_settings;
    lv_obj_t *btn_3;
    lv_obj_t *label_3;
    lv_obj_t *btn_2;
    lv_obj_t *label_2;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingBeauty_t;

typedef struct
{
    /* 展示音量的弹窗 */
    lv_obj_t *screen_VolumeOverlay;
    bool screen_VolumeOverlay_del;
    lv_obj_t *screen_VolumeOverlay_label;
    lv_obj_t *screen_VolumeOverlay_bar;
    lv_obj_t *g_kb_top_layer;

    /* 顶部状态栏 */
    lv_obj_t *status_bar;
    lv_obj_t *img_4g;
    lv_obj_t *img_wifi;
    lv_obj_t *img_power;
    lv_obj_t *date_text;
    lv_obj_t *digital_clock;

    /* 主界面1 */
    PageHome1_t page_home1;
    bool screenHome1_del;

    /* 主界面1 */
    HomePhoto_t page_photo;
    bool screenHomePhoto_del;

    /* 拍照设置 */
    MenuSetting_t page_photoMenu_Setting;
    bool screenPhotoMenuSetting_del;

    //拍照设置页面--拍照分辨率设置
    SettingResolution_t page_resolution;
    bool screen_SettingResolution_del;

    /* 拍照设置页面 -- 白平衡设置*/
    SettingWhiteBalance_t page_whitebalance;
    bool screen_SettingWhiteBalance_del;

    /* 拍照设置页面 -- 特效（滤镜）设置*/
    SettingPhotoEffect_t page_photoeffect;
    bool screen_SettingPhotoEffect_del;

    /* 拍照设置页面 -- 曝光设置*/
    SettingExposure_t page_exposure;
    bool screen_SettingExposure_del;

    /* 拍照设置页面 -- 自动对焦设置*/
    SettingAutofocus_t page_autofocus;
    bool screen_SettingAutofocus_del;

    /* 拍照设置页面 -- 场景模式设置*/
    SettingPictureMode_t page_picturemode;
    bool screen_SettingPictureMode_del;

    /* 拍照设置页面 -- 自拍时间设置*/
    SettingSelfieTime_t page_selfietime;
    bool screen_SettingSelfieTime_del;

    /* 拍照设置页面 -- 拍摄模式设置（单张，连拍）*/
    SettingShootingMode_t page_shootingmode;
    bool screen_SettingShootingMode_del;

    /* 拍照设置页面 -- 画质设置*/
    SettingPictureQuality_t page_picturequality;
    bool screen_SettingPictureQuality_del;

    /* 拍照设置页面 -- ISO感光度设置*/
    SettingSensitivity_t page_sensitivity;
    bool screen_SettingSensitivity_del;

    /* 拍照设置页面 -- 防抖开关设置*/
    SettingAntiShake_t page_antishake;
    bool screen_SettingAntiShake_del;

    /* 拍照设置页面 -- 人脸侦测*/
    SettingFaceDectection_t page_facedectection;
    bool screen_SettingFaceDectection_del;

    /* 拍照设置页面 -- 笑脸侦测*/
    SettingSmileDectection_t page_smiledectection;
    bool screen_SettingSmileDectection_del;

    /* 拍照设置页面 -- 美颜*/
    SettingBeauty_t page_beauty;
    bool screen_SettingBeauty_del;

} lv_ui_t;

typedef void (*ui_setup_scr_t)(lv_ui_t *ui);

void homeMode_Set(uint8_t mode);
uint8_t homeMode_Get(void);

// typedef void (*ui_new_wind_scr_t)(void);

void ui_init_style(lv_style_t *style);
void ui_load_win_animation(lv_obj_t *new_scr, ui_setup_scr_t new_win, lv_screen_load_anim_t anim_type, uint32_t time,
                           uint32_t delay, bool auto_del);

void ui_load_scr_animation(lv_ui_t *ui, lv_obj_t **new_scr, bool new_scr_del, bool *old_scr_del,
                           ui_setup_scr_t setup_scr, lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay,
                           bool is_clean, bool auto_del);

void ui_animation(void *var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value,
                  lv_anim_path_cb_t path_cb, uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time,
                  uint32_t playback_delay, lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb,
                  lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);

void init_scr_del_flag(lv_ui_t *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui_t *ui);

void init_keyboard(lv_ui_t *ui);

extern lv_ui_t g_ui;

void setup_scr_screen_VolumeOverlay(lv_ui_t *ui);
void setup_scr_home1(lv_ui_t *ui);
//禁用触摸事件
void disable_touch_events(void);
//使能触摸
void enable_touch_events(void);

int enable_hardware_input_device(int device_id);
int disable_hardware_input_device(int device_id);

lv_obj_t *ui_Get_PreScreen(void);

LV_IMAGE_DECLARE(_none_RGB565A8_20x28);
LV_IMAGE_DECLARE(_wifi_1_RGB565A8_24x23);
LV_IMAGE_DECLARE(_wifi_2_RGB565A8_24x23);
LV_IMAGE_DECLARE(_wifi_3_RGB565A8_24x23);
LV_IMAGE_DECLARE(_wifi_4_RGB565A8_24x23);
LV_IMAGE_DECLARE(_none_RGB565A8_37x33);
LV_IMAGE_DECLARE(_4g_4_RGB565A8_24x17);
LV_IMAGE_DECLARE(_4g_3_RGB565A8_24x17);
LV_IMAGE_DECLARE(_4g_2_RGB565A8_24x17);
LV_IMAGE_DECLARE(_4g_1_RGB565A8_24x17);
LV_IMAGE_DECLARE(_power_3_RGB565A8_33x28);
LV_IMAGE_DECLARE(_power_2_RGB565A8_33x28);
LV_IMAGE_DECLARE(_power_1_RGB565A8_33x28);
LV_IMAGE_DECLARE(_power0_red_RGB565A8_33x28);
LV_IMAGE_DECLARE(_power_charging_RGB565A8_33x28);
LV_IMAGE_DECLARE(_sun_RGB565A8_100x100);
LV_IMAGE_DECLARE(_video_icon_RGB565A8_35x29);
LV_IMAGE_DECLARE(_power_3_RGB565A8_416x440);
LV_IMAGE_DECLARE(_power_3_RGB565A8_385x358);
LV_IMAGE_DECLARE(_power_3_RGB565A8_332x319);
LV_IMAGE_DECLARE(_mizige_yellow_RGB565A8_150x150);
LV_IMAGE_DECLARE(_fangkuan_RGB565A8_100x100);
LV_IMAGE_DECLARE(_AIPhoto_RGB565A8_135x194);
LV_IMAGE_DECLARE(_TakePhoto_RGB565A8_135x194);
LV_IMAGE_DECLARE(_AItalk_RGB565A8_135x194);
LV_IMAGE_DECLARE(_AIeffect_RGB565A8_135x194);
LV_IMAGE_DECLARE(_AIPhoto_2_RGB565A8_167x161);
LV_IMAGE_DECLARE(_TakePhoto_2_RGB565A8_141x133);
LV_IMAGE_DECLARE(_AItalk_2_RGB565A8_158x149);
LV_IMAGE_DECLARE(_AIeffect_2_RGB565A8_168x167);
LV_IMAGE_DECLARE(_photoalbum_2_RGB565A8_163x156);
LV_IMAGE_DECLARE(_setting_RGB565A8_156x145);
LV_IMAGE_DECLARE(_chinese_2_RGB565A8_165x159);
LV_IMAGE_DECLARE(_english_RGB565A8_165x163);
LV_IMAGE_DECLARE(icon_card_err);
LV_IMAGE_DECLARE(Desktop);
LV_IMAGE_DECLARE(airecognition);

LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_16)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_montserratMedium_17)
LV_FONT_DECLARE(lv_font_montserratMedium_13)
LV_FONT_DECLARE(lv_font_montserratMedium_45)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_13)
LV_FONT_DECLARE(lv_font_montserratMedium_35)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_20)
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_30)
LV_FONT_DECLARE(lv_font_montserratMedium_34)
LV_FONT_DECLARE(lv_font_custom)
LV_FONT_DECLARE(import_symbol_lock)
#ifdef __cplusplus
}
#endif
#endif

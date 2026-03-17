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
#include "../lvgl/src/misc/lv_timer_private.h"

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
    lv_obj_t *scr; /* 页面容器：带4个按钮 + 底部页面标识 */
    bool del;
    lv_obj_t *imgbtn_album;
    lv_obj_t *imgbtn_setting;
    lv_obj_t *imgbtn_chinese;
    lv_obj_t *imgbtn_english;

    lv_obj_t *dots_cont; /* 底部页面标识：容器带两个点 */
    lv_obj_t *dot_home1;
    lv_obj_t *dot_home2;
} PageHome2_t;

// AI拍照结构体
typedef struct
{
    lv_obj_t *scr; /* 页面容器：底部4个缩放按钮+顶部返回按钮 */
    bool del;

    lv_obj_t *bottom_cont; //底部显示容器
    lv_obj_t *btn_size05x;
    lv_obj_t *label_size05x;
    lv_obj_t *btn_size1x;
    lv_obj_t *label_size1x;
    lv_obj_t *btn_size2x;
    lv_obj_t *label_size2x;
    lv_obj_t *btn_size3x;
    lv_obj_t *label_size3x;

    lv_obj_t *top_cont; //顶部显示容器
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} AiTakePhoto_t;

//传统拍照结构体
typedef struct
{
    /* 传统拍照页面 */
    lv_obj_t *scr; /* 拍照页面容器 底部4个缩放按钮 + 模式选择按钮，顶部返回和菜单按钮 ，右侧点击拍照按钮*/
    bool del;

    lv_obj_t *bottom_cont; //底部容器
    lv_obj_t *cont_2;
    //拍照缩放
    lv_obj_t *btn_size05x;
    lv_obj_t *label_size05x;
    lv_obj_t *btn_size1x;
    lv_obj_t *label_size1x;
    lv_obj_t *btn_size2x;
    lv_obj_t *label_size2x;
    lv_obj_t *btn_size3x;
    lv_obj_t *label_size3x;

    //拍照模式
    lv_obj_t *btn_video;
    lv_obj_t *label_video;
    lv_obj_t *btn_loopvideo;
    lv_obj_t *label_loopvideo;
    lv_obj_t *btn_photo;
    lv_obj_t *label_photo;
    lv_obj_t *btn_conti_photo;
    lv_obj_t *label_conti_photo;
    lv_obj_t *btn_timed_photo;
    lv_obj_t *label_timed_photo;
    lv_obj_t *btn_time_lapse_video;
    lv_obj_t *label_time_lapse_video;

    lv_obj_t *top_cont; //顶部容器
    lv_obj_t *btn_setting;
    lv_obj_t *label_setting;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

    lv_obj_t *label_timer; //计时
    lv_timer_t *take_photo_timer;

    lv_obj_t *video_circle_container; // 外部白色圆圈容器
    lv_obj_t *label_video_timer; // 录像计时器标签
    lv_timer_t *recording_timer; // 录像计时器

    lv_obj_t *btn_temp_photo; //点击拍照
    lv_obj_t *label_temp_photo;

} TakePhoto_t;

/* AI对话页面 */
typedef struct
{

    lv_obj_t *scr; /* AI对话页面容器 顶部两个按钮，底部一个按钮，中间三个选择项 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *cont_dr;
    lv_obj_t *cont_nezha;
    lv_obj_t *cont_deepseek;
    lv_obj_t *btn_dr;
    lv_obj_t *label_dr;
    lv_obj_t *btn_nezha;
    lv_obj_t *label_nezha;
    lv_obj_t *btn_deepseek;
    lv_obj_t *label_deepseek;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_talk;
    lv_obj_t *label_talk;
    lv_obj_t *title;

} AIDialog_t;

/* AI对话页面 -- 子页面，百科博士 */
typedef struct
{

    lv_obj_t *scr; /* AI对话页面 -- 子页面，百科博士容器，顶部一个返回按钮，底部一个确认按钮，中心一个按钮 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_dr;
    lv_obj_t *label_dr;
    lv_obj_t *btn_talk;
    lv_obj_t *label_talk;
    lv_obj_t *title;
} AIDialogDr_t;

/* AI对话页面 -- 子页面，DeepSeek  */
typedef struct
{

    lv_obj_t *scr; /* AI对话页面 -- 子页面，DeepSeek 容器，顶部一个返回按钮，底部一个确认按钮，中心一个按钮*/
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *btn_talk;
    lv_obj_t *label_talk;
    lv_obj_t *btn_deepseek;
    lv_obj_t *label_deepseek;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} AIDialogDS_t;

/* AI对话页面 */
typedef struct
{

    lv_obj_t *scr; /* AI对话页面 -- 子页面，哪吒 容器，顶部一个返回按钮，底部一个确认按钮，中心一个按钮*/
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *btn_talk;
    lv_obj_t *label_talk;
    lv_obj_t *btn_nezha;
    lv_obj_t *label_nezha;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
} AIDialogNZ_t;

/* AI特效页面（图像风格转换） */
typedef struct
{

    lv_obj_t *scr; /* AI特效页面（图像风格转换）容器，底部四个缩放按钮，顶部一个按钮，加一个模拟拍照按钮 */
    bool del;
    lv_obj_t *bottom_cont;
    lv_obj_t *top_cont;
    lv_obj_t *btn_size3x;
    lv_obj_t *label_size3x;
    lv_obj_t *btn_size2x;
    lv_obj_t *label_size2x;
    lv_obj_t *btn_size1x;
    lv_obj_t *label_size1x;
    lv_obj_t *btn_size05x;
    lv_obj_t *label_size05x;

    lv_obj_t *btn_back;
    lv_obj_t *label_back;

    lv_obj_t *btn_takephoto;
    lv_obj_t *label_takephoto;
} AIEffect_t;

/* AI特效页面,--子页面，模拟ai拍照 */
typedef struct
{

    lv_obj_t *scr; /* AI特效页面,--子页面，模拟ai拍照容器，下面一个大的容器按钮+三个开关按钮，上面一个返回按钮 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *bottom_cont;
    lv_obj_t *imgbtn_switch;
    lv_obj_t *btn_cancel;
    lv_obj_t *label_cancel;
    lv_obj_t *btn_ok;
    lv_obj_t *label_ok;
    lv_obj_t *img_main; /* 拍照结果图片 */
    lv_obj_t *label_result; /* AI处理结果显示标签 */
} AIEffectPic_t;

/* AI拍照页面 */
typedef struct
{
    lv_obj_t *scr; /* AI拍照页面容器 */
    bool del;
    lv_obj_t *top_cont; //顶部显示容器
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *bottom_cont; //底部显示容器
    lv_obj_t *btn_ai;
    lv_obj_t *label_ai;
    lv_obj_t *img_main; /* AI拍照结果图片 */
    lv_obj_t *label_result; /* AI识别结果显示标签 */
} AIPhoto_t;

/* AI拍照结果页面 */
typedef struct
{
    lv_obj_t *scr; /* AI拍照结果页面容器 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *bottom_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_ai;
    lv_obj_t *label_ai;
    lv_obj_t *btn_science;
    lv_obj_t *label_science;
    lv_obj_t *btn_chinese;
    lv_obj_t *label_chinese;
    lv_obj_t *btn_english;
    lv_obj_t *label_english;
    lv_obj_t *btn_history;
    lv_obj_t *label_history;
    lv_obj_t *result_cont;    /* AI识别结果容器 */
    lv_obj_t *result_label;   /* AI识别结果文本标签 */
    lv_obj_t *img_main;       /* AI拍照结果图片 */
} AIPhotoResult_t;

/* AI拍照结果页面1 */
typedef struct
{
    lv_obj_t *scr; /* AI拍照结果页面1容器 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *bottom_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_func;
    lv_obj_t *label_func;
    lv_obj_t *img_main; /* AI拍照结果图片 */
} AIPhotoResult1_t;

/* AI拍照结果页面2 */
typedef struct
{
    lv_obj_t *scr; /* AI拍照结果页面2容器 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *bottom_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_func;
    lv_obj_t *label_func;
    lv_obj_t *img_main; /* AI拍照结果图片 */
} AIPhotoResult2_t;

/* AI拍照结果页面3 */
typedef struct
{
    lv_obj_t *scr; /* AI拍照结果页面3容器 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *bottom_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_func;
    lv_obj_t *label_func;
    lv_obj_t *img_main; /* AI拍照结果图片 */
} AIPhotoResult3_t;

/* AI拍照结果页面4 */
typedef struct
{
    lv_obj_t *scr; /* AI拍照结果页面4容器 */
    bool del;
    lv_obj_t *top_cont;
    lv_obj_t *bottom_cont;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_func;
    lv_obj_t *label_func;
    lv_obj_t *img_main; /* AI拍照结果图片 */
} AIPhotoResult4_t;
/* 设置页面 */
typedef struct
{
    lv_obj_t *scr; /* 设置页面 */
    bool del;
    lv_obj_t *btn_ai;
    lv_obj_t *label_ai;
    lv_obj_t *btn_sys;
    lv_obj_t *label_sys;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *title;
} Settings_t;

/* 设置页面 -- 子页面，系统设置 */
typedef struct
{

    lv_obj_t *scr;
    bool del;
    lv_obj_t *cont_main;
    lv_obj_t *btn_wifi;
    lv_obj_t *label_wifi;
    lv_obj_t *btn_volumeg;
    lv_obj_t *label_volumeg;
    lv_obj_t *btn_volume;
    lv_obj_t *label_volume;
    lv_obj_t *btn_luma;
    lv_obj_t *label_luma;
    lv_obj_t *btn_backlight;
    lv_obj_t *label_backlight;
    lv_obj_t *btn_about;
    lv_obj_t *label_about;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *title;
    lv_obj_t *btn_volume_down;
    lv_obj_t *label_volume_down;
    lv_obj_t *btn_volume_up;
    lv_obj_t *label_volume_up;
} SettingsSys_t;

/* 设置页面 -- 系统设置 -- WIFI设置*/
typedef struct
{

    lv_obj_t *scr; /* 设置页面 -- 系统设置 -- WIFI设置容器 上面一个返回按钮 加wifi开关按钮 下面为wifi列表加进度条*/
    bool del;
    lv_obj_t *title;
    lv_obj_t *list;
    lv_obj_t *list_item0;
    lv_obj_t *btn_back;
    lv_obj_t *topbar_wifi_cont;   // 顶部wifi开关栏
    lv_obj_t *topbar_wifi_label;  // 顶部wifi开关栏文字
    lv_obj_t *topbar_wifi_switch; // 顶部wifi开关栏开关
    lv_obj_t *label_refresh;      // 刷新按钮
    lv_obj_t *label_scan;         // 提示正在扫描wifi的标签
    lv_obj_t *label_back_btn;
    lv_obj_t *label_my_network;

} SettingsSysWifi_t;

/* 设置页面 -- 系统设置 -- 输入WIFI密码*/
typedef struct
{
    lv_obj_t *scr; //输入WIFI密码页面容器 ，返回按钮加连接按钮+显示密码的按钮 +输入秘密键盘
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_connect;
    lv_obj_t *label_connect;
    lv_obj_t *password_ta; //密码输入区域
    lv_obj_t *btn_toggle;  //是否显示密码按钮
    lv_obj_t *label_toggle;
    lv_obj_t *toggle_icon; //是否显示密码图标
    lv_obj_t *keyboard;    // 键盘对象
} SettingsSysWifiCode_t;

/* 设置页面 -- 系统设置 -- 移动网络*/
typedef struct
{
    lv_obj_t *scr; //移动网络页面容器 ，返回按钮加连接按钮+移动网络开关
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingsSys4G_t;
/* 设置页面 -- 系统设置 -- 音量设置*/
typedef struct
{
    lv_obj_t *scr; //音量设置页面容器 ，返回按钮+半圆音量进度条
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *arc;        //半圆进度条
    lv_obj_t *label_icon; //中间音量符号
    lv_obj_t *btn_minus;
    lv_obj_t *label_minus;
    lv_obj_t *btn_plus;
    lv_obj_t *label_plus;

} SettingsSysVolume_t;

/* 设置页面 -- 系统设置 -- 亮度设置*/
typedef struct
{
    lv_obj_t *scr; //亮度设置页面容器 ，返回按钮+半圆音量进度条
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *arc; //半圆进度条
    lv_obj_t *label_icon;
    lv_obj_t *btn_minus;
    lv_obj_t *label_minus;
    lv_obj_t *btn_plus;
    lv_obj_t *label_plus;
    lv_obj_t *img_sun;

} SettingsSysLuma_t;

/* 设置页面 -- 系统设置 -- 亮屏时长*/
typedef struct
{

    lv_obj_t *scr; //亮屏时长页面容器 ，返回按钮+时长选择列表
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_minus;
    lv_obj_t *label_minus;
    lv_obj_t *btn_plus;
    lv_obj_t *label_plus;
    lv_obj_t *list;

} SettingsSysBlTime_t;

/* 设置页面 -- 系统设置 -- 关于*/
typedef struct
{

    lv_obj_t *scr; //关于页面容器,一个返回按钮+五个新页面按钮
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_update;
    lv_obj_t *label_update;
    lv_obj_t *btn_info;
    lv_obj_t *label_info;
    lv_obj_t *btn_log;
    lv_obj_t *label_log;
    lv_obj_t *btn_service;
    lv_obj_t *label_service;
    lv_obj_t *btn_privacy;
    lv_obj_t *label_privacy;
    lv_obj_t *btn_1;
    lv_obj_t *label_1;

} SettingsSysAbout_t;

/* 设置页面 -- 系统设置 -- 关于--系统更新*/
typedef struct
{
    lv_obj_t *scr; //一个返回按钮+标题
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingsSysUpdate_t;

/* 设置页面 -- 系统设置 -- 关于--本机信息*/
typedef struct
{
    lv_obj_t *scr; //一个返回按钮+标题
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingsSysInfo_t;

/* 设置页面 -- 系统设置 -- 关于--日志上传*/
typedef struct
{
    lv_obj_t *scr; //一个返回按钮+标题
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingsSysLog_t;

/* 设置页面 -- 系统设置 -- 关于--服务协议*/
typedef struct
{
    lv_obj_t *scr; //一个返回按钮+标题
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingsSysService_t;

/* 设置页面 -- 系统设置 -- 关于--隐私政策*/
typedef struct
{
    lv_obj_t *scr; //一个返回按钮+标题
    bool del;
    lv_obj_t *title;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;

} SettingsSysPrivacy_t;

/* 设置页面 -- 子页面，AI设置 */
typedef struct
{

    lv_obj_t *scr; // ai设置页面容器 一个返回按钮+两个设置选择按钮
    bool del;
    lv_obj_t *btn_voice;
    lv_obj_t *label_voice;
    lv_obj_t *btn_content;
    lv_obj_t *label_content;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *title;
} SettingsAI_t;

/* 相册页面（照片、视频） */
typedef struct
{

    lv_obj_t *scr; /* 相册页面容器  */
    bool del;
    lv_obj_t *cont_top;
    lv_obj_t *btn_delete;
    lv_obj_t *label_delete;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *btn_choose;
    lv_obj_t *label_choose;
    lv_obj_t *btn_cancel;
    lv_obj_t *label_cancel;
    lv_obj_t *cont_album_grid;
    lv_obj_t *scrollbar_slider;
} PhotoAlbum_t;

/* 相册页面（照片、视频） -- 子页面，照片 */
typedef struct
{
    lv_obj_t *scr;
    bool del;
    lv_obj_t *cont_top;
    lv_obj_t *btn_delete;
    lv_obj_t *btn_delete_label;
    lv_obj_t *btn_back;
    lv_obj_t *btn_back_label;
    lv_obj_t *label_name;
    lv_obj_t *cont_bottom;
    lv_obj_t *img;
} PhotoAlbumPic_t;

/* 拍照设置页面 */
typedef struct
{
    /* 拍照设置页面 */
    lv_obj_t *scr;
    bool del;
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
    lv_obj_t *btn_8;
    lv_obj_t *label_8;
    lv_obj_t *btn_9;
    lv_obj_t *label_9;
    lv_obj_t *btn_10;
    lv_obj_t *label_10;
    lv_obj_t *btn_11;
    lv_obj_t *label_11;
    lv_obj_t *btn_12;
    lv_obj_t *label_12;
    lv_obj_t *btn_13;
    lv_obj_t *label_13;
    lv_obj_t *btn_14;
    lv_obj_t *label_14;
    lv_obj_t *btn_15;
    lv_obj_t *label_15;
    lv_obj_t *btn_16;
    lv_obj_t *label_16;
    lv_obj_t *cont_top;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *cont_bottom;
} TakePhotoSetting_t;

/* 拍照设置页面 */
typedef struct
{

    lv_obj_t *scr;
    bool del;
    lv_obj_t *cont_bottom;
    lv_obj_t *cont_top;
    lv_obj_t *btn_back;
    lv_obj_t *label_back;
    lv_obj_t *label_title;
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

    /* 主界面2 */
    PageHome2_t page_home2;

    bool screenHome1_del;
    bool screenHome2_del;

    /* AI拍照（拍万物） */
    AiTakePhoto_t page_aitakephoto;
    bool screen_AITakePhoto_del;

    /* 传统拍照页面 */
    TakePhoto_t page_takephoto;
    bool screen_TakePhoto_del;

    /* AI对话页面 */
    AIDialog_t page_aidialog;
    bool screen_AIDialog_del;

    /* AI对话页面 -- 子页面，百科博士 */
    AIDialogDr_t page_aidialogdr;
    bool screen_AIDialogDr_del;

    /* AI对话页面 -- 子页面，哪吒 */
    AIDialogNZ_t page_aidialognz;
    bool screen_AIDialogNZ_del;

    /* AI对话页面 -- 子页面，DeepSeek */
    AIDialogDS_t page_aidialogds;
    bool screen_AIDialogDS_del;

    /* AI特效页面（图像风格转换） */
    AIEffect_t page_aieffect;
    bool screen_AIEffect_del;

    /* AI特效页面--子页面 模拟拍照*/
    AIEffectPic_t page_aieffectpic;
    bool screen_AIEffectPic_del;

    /* AI拍照照片页面 */
    AIPhoto_t page_aiphoto;
    bool screen_AIPhoto_del;

    /* AI拍照识别结果页面 */
    AIPhotoResult_t page_aiphotoresult;
    bool screen_AIPhotoResult_del;

    /* AI拍照识别结果页面1 */
    AIPhotoResult1_t page_aiphotoresult1;
    bool screen_AIPhotoResult1_del;

    /* AI拍照识别结果页面2 */
    AIPhotoResult2_t page_aiphotoresult2;
    bool screen_AIPhotoResult2_del;

    /* AI拍照识别结果页面3 */
    AIPhotoResult3_t page_aiphotoresult3;
    bool screen_AIPhotoResult3_del;

    /* AI拍照识别结果页面4 */
    AIPhotoResult4_t page_aiphotoresult4;
    bool screen_AIPhotoResult4_del;

    /* 设置页面 */
    Settings_t page_settings;
    bool screen_Settings_del;

    /* 设置页面 -- 子页面，系统设置 */
    SettingsSys_t page_settingssys;
    bool screen_SettingsSys_del;

    /* 设置页面 -- 系统设置 -- WIFI设置*/
    SettingsSysWifi_t page_syswifi;
    bool screen_SettingsSysWifi_del;

    /* 设置页面 -- 系统设置 -- 输入WIFI密码*/
    SettingsSysWifiCode_t page_syswificode;
    bool screen_SettingsSysWifiCode_del;

    /* 设置页面 -- 系统设置 -- 4G*/
    SettingsSys4G_t page_sys4g;
    bool screen_SettingsSys4G_del;

    /* 设置页面 -- 系统设置 -- 音量*/
    SettingsSysVolume_t page_sysvolume;
    bool screen_SettingsSysVolume_del;

    /* 设置页面 -- 系统设置 -- 亮度*/
    SettingsSysLuma_t page_sysluma;
    bool screen_SettingsSysLuma_del;

    /* 设置页面 -- 系统设置 -- 亮屏时长*/
    SettingsSysBlTime_t page_sysbltim;
    bool screen_SettingsSysBl_del;

    /* 设置页面 -- 系统设置 -- 关于*/
    SettingsSysAbout_t page_sysabout;
    bool screen_SettingsSysAbout_del;

    /* 设置页面 -- 系统设置 -- 关于--系统更新*/
    SettingsSysUpdate_t page_sysupdate;
    bool screen_SettingsSysUpdate_del;

    /* 设置页面 -- 系统设置 -- 关于--本机信息*/
    SettingsSysInfo_t page_sysinfo;
    bool screen_SettingsSysInfo_del;

    /* 设置页面 -- 系统设置 -- 关于--日志上传*/
    SettingsSysLog_t page_syslog;
    bool screen_SettingsSysLog_del;

    /* 设置页面 -- 系统设置 -- 关于--服务协议*/
    SettingsSysService_t page_sysservice;
    bool screen_SettingsSysService_del;

    /* 设置页面 -- 系统设置 -- 关于--隐私政策*/
    SettingsSysPrivacy_t page_sysprivacy;
    bool screen_SettingsSysPrivacy_del;

    /* 设置页面 -- 子页面，AI设置 */
    SettingsAI_t page_settingsai;
    bool screen_SettingsAI_del;

    /* 相册页面（照片、视频） */
    PhotoAlbum_t page_photoalbum;
    bool screen_PhotoAlbum_del;

    /* 相册页面（照片、视频） -- 子页面，照片 */
    PhotoAlbumPic_t page_photoalbumpic;
    bool screen_PhotoAlbumPic_del;

    lv_obj_t *screen_PhotoAlbumVid;
    bool screen_PhotoAlbumVid_del;
    lv_obj_t *screen_PhotoAlbumVid_btn_stop;
    lv_obj_t *screen_PhotoAlbumVid_btn_stop_label;
    lv_obj_t *screen_PhotoAlbumVid_btn_play;
    lv_obj_t *screen_PhotoAlbumVid_btn_play_label;
    lv_obj_t *screen_PhotoAlbumVid_slider;
    lv_obj_t *screen_PhotoAlbumVid_cont_bottom;
    lv_obj_t *screen_PhotoAlbumVid_btn_15x;
    lv_obj_t *screen_PhotoAlbumVid_btn_15x_label;
    lv_obj_t *screen_PhotoAlbumVid_btn_1x;
    lv_obj_t *screen_PhotoAlbumVid_btn_1x_label;
    lv_obj_t *screen_PhotoAlbumVid_btn_075x;
    lv_obj_t *screen_PhotoAlbumVid_btn_075x_label;
    lv_obj_t *screen_PhotoAlbumVid_cont_top;
    lv_obj_t *screen_PhotoAlbumVid_btn_delete;
    lv_obj_t *screen_PhotoAlbumVid_btn_delete_label;
    lv_obj_t *screen_PhotoAlbumVid_btn_back;
    lv_obj_t *screen_PhotoAlbumVid_btn_back_label;
    lv_obj_t *screen_PhotoAlbumVid_label_name;

    lv_obj_t *screen_Photo;
    bool screen_Photo_del;
    lv_obj_t *screen_Photo_canvas_1;
    lv_obj_t *screen_Photo_btn_1;
    lv_obj_t *screen_Photo_btn_1_label;

    /* 拍照设置页面 */
    TakePhotoSetting_t page_takephotosetting;
    bool screen_TakePhotoSetting_del;

    lv_obj_t *screen_SettingResolution;
    bool screen_SettingResolution_del;
    lv_obj_t *screen_SettingResolution_cont_bottom;
    lv_obj_t *screen_SettingResolution_cont_top;
    lv_obj_t *screen_SettingResolution_btn_back;
    lv_obj_t *screen_SettingResolution_btn_back_label;
    lv_obj_t *screen_SettingResolution_label_title;
    lv_obj_t *screen_SettingResolution_cont_12;
    lv_obj_t *screen_SettingResolution_btn_back0;
    lv_obj_t *screen_SettingResolution_btn_back0_label;
    lv_obj_t *screen_SettingResolution_btn_9;
    lv_obj_t *screen_SettingResolution_btn_9_label;
    lv_obj_t *screen_SettingResolution_btn_8;
    lv_obj_t *screen_SettingResolution_btn_8_label;
    lv_obj_t *screen_SettingResolution_btn_7;
    lv_obj_t *screen_SettingResolution_btn_7_label;
    lv_obj_t *screen_SettingResolution_btn_6;
    lv_obj_t *screen_SettingResolution_btn_6_label;
    lv_obj_t *screen_SettingResolution_btn_5;
    lv_obj_t *screen_SettingResolution_btn_5_label;
    lv_obj_t *screen_SettingResolution_btn_4;
    lv_obj_t *screen_SettingResolution_btn_4_label;
    lv_obj_t *screen_SettingResolution_btn_3;
    lv_obj_t *screen_SettingResolution_btn_3_label;
    lv_obj_t *screen_SettingResolution_btn_2;
    lv_obj_t *screen_SettingResolution_btn_2_label;

    /* 拍照设置页面 -- 白平衡设置*/
    lv_obj_t *screen_SettingWhiteBalance;
    bool screen_SettingWhiteBalance_del;
    lv_obj_t *screen_SettingWhiteBalance_cont_3;
    lv_obj_t *screen_SettingWhiteBalance_btn_3;
    lv_obj_t *screen_SettingWhiteBalance_btn_3_label;
    lv_obj_t *screen_SettingWhiteBalance_btn_4;
    lv_obj_t *screen_SettingWhiteBalance_btn_4_label;
    lv_obj_t *screen_SettingWhiteBalance_btn_5;
    lv_obj_t *screen_SettingWhiteBalance_btn_5_label;
    lv_obj_t *screen_SettingWhiteBalance_btn_6;
    lv_obj_t *screen_SettingWhiteBalance_btn_6_label;
    lv_obj_t *screen_SettingWhiteBalance_btn_7;
    lv_obj_t *screen_SettingWhiteBalance_btn_7_label;
    lv_obj_t *screen_SettingWhiteBalance_cont_bottom;
    lv_obj_t *screen_SettingWhiteBalance_cont_top;
    lv_obj_t *screen_SettingWhiteBalance_label_title;
    lv_obj_t *screen_SettingWhiteBalance_btn_back;
    lv_obj_t *screen_SettingWhiteBalance_btn_back_label;

    /* 拍照设置页面 -- 特效（滤镜）设置*/
    lv_obj_t *screen_SettingPhotoEffect;
    bool screen_SettingPhotoEffect_del;
    lv_obj_t *screen_SettingPhotoEffect_cont_settings;
    lv_obj_t *screen_SettingPhotoEffect_btn_back0;
    lv_obj_t *screen_SettingPhotoEffect_btn_back0_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_9;
    lv_obj_t *screen_SettingPhotoEffect_btn_9_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_8;
    lv_obj_t *screen_SettingPhotoEffect_btn_8_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_7;
    lv_obj_t *screen_SettingPhotoEffect_btn_7_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_6;
    lv_obj_t *screen_SettingPhotoEffect_btn_6_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_5;
    lv_obj_t *screen_SettingPhotoEffect_btn_5_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_4;
    lv_obj_t *screen_SettingPhotoEffect_btn_4_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_3;
    lv_obj_t *screen_SettingPhotoEffect_btn_3_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_2;
    lv_obj_t *screen_SettingPhotoEffect_btn_2_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_back1;
    lv_obj_t *screen_SettingPhotoEffect_btn_back1_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_back2;
    lv_obj_t *screen_SettingPhotoEffect_btn_back2_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_back3;
    lv_obj_t *screen_SettingPhotoEffect_btn_back3_label;
    lv_obj_t *screen_SettingPhotoEffect_btn_back4;
    lv_obj_t *screen_SettingPhotoEffect_btn_back4_label;
    lv_obj_t *screen_SettingPhotoEffect_cont_bottom;
    lv_obj_t *screen_SettingPhotoEffect_cont_top;
    lv_obj_t *screen_SettingPhotoEffect_label_title;
    lv_obj_t *screen_SettingPhotoEffect_btn_back;
    lv_obj_t *screen_SettingPhotoEffect_btn_back_label;

    /* 拍照设置页面 -- 曝光设置*/
    lv_obj_t *screen_SettingExposure;
    bool screen_SettingExposure_del;
    lv_obj_t *screen_SettingExposure_canvas_1;
    lv_obj_t *screen_SettingExposure_cont_top;
    lv_obj_t *screen_SettingExposure_btn_9;
    lv_obj_t *screen_SettingExposure_btn_9_label;
    lv_obj_t *screen_SettingExposure_btn_8;
    lv_obj_t *screen_SettingExposure_btn_8_label;
    lv_obj_t *screen_SettingExposure_btn_7;
    lv_obj_t *screen_SettingExposure_btn_7_label;
    lv_obj_t *screen_SettingExposure_btn_6;
    lv_obj_t *screen_SettingExposure_btn_6_label;
    lv_obj_t *screen_SettingExposure_btn_5;
    lv_obj_t *screen_SettingExposure_btn_5_label;
    lv_obj_t *screen_SettingExposure_btn_4;
    lv_obj_t *screen_SettingExposure_btn_4_label;
    lv_obj_t *screen_SettingExposure_btn_3;
    lv_obj_t *screen_SettingExposure_btn_3_label;
    lv_obj_t *screen_SettingExposure_cont_bottom;
    lv_obj_t *screen_SettingExposure_cont_30;
    lv_obj_t *screen_SettingExposure_label_title;
    lv_obj_t *screen_SettingExposure_btn_back;
    lv_obj_t *screen_SettingExposure_btn_back_label;

    /* 拍照设置页面 -- 画面模式设置*/
    lv_obj_t *screen_SettingPictureMode;
    bool screen_SettingPictureMode_del;
    lv_obj_t *screen_SettingPictureMode_cont_32;
    lv_obj_t *screen_SettingPictureMode_btn_8;
    lv_obj_t *screen_SettingPictureMode_btn_8_label;
    lv_obj_t *screen_SettingPictureMode_btn_7;
    lv_obj_t *screen_SettingPictureMode_btn_7_label;
    lv_obj_t *screen_SettingPictureMode_btn_6;
    lv_obj_t *screen_SettingPictureMode_btn_6_label;
    lv_obj_t *screen_SettingPictureMode_btn_5;
    lv_obj_t *screen_SettingPictureMode_btn_5_label;
    lv_obj_t *screen_SettingPictureMode_btn_4;
    lv_obj_t *screen_SettingPictureMode_btn_4_label;
    lv_obj_t *screen_SettingPictureMode_btn_3;
    lv_obj_t *screen_SettingPictureMode_btn_3_label;
    lv_obj_t *screen_SettingPictureMode_btn_2;
    lv_obj_t *screen_SettingPictureMode_btn_2_label;
    lv_obj_t *screen_SettingPictureMode_btn_9;
    lv_obj_t *screen_SettingPictureMode_btn_9_label;
    lv_obj_t *screen_SettingPictureMode_cont_30;
    lv_obj_t *screen_SettingPictureMode_cont_top;
    lv_obj_t *screen_SettingPictureMode_cont_bottom;
    lv_obj_t *screen_SettingPictureMode_label_title;
    lv_obj_t *screen_SettingPictureMode_btn_back;
    lv_obj_t *screen_SettingPictureMode_btn_back_label;

    /* 拍照设置页面 -- 自拍时间设置*/
    lv_obj_t *screen_SettingSelfieTime;
    bool screen_SettingSelfieTime_del;
    lv_obj_t *screen_SettingSelfieTime_cont_34;
    lv_obj_t *screen_SettingSelfieTime_btn_5;
    lv_obj_t *screen_SettingSelfieTime_btn_5_label;
    lv_obj_t *screen_SettingSelfieTime_btn_4;
    lv_obj_t *screen_SettingSelfieTime_btn_4_label;
    lv_obj_t *screen_SettingSelfieTime_btn_3;
    lv_obj_t *screen_SettingSelfieTime_btn_3_label;
    lv_obj_t *screen_SettingSelfieTime_cont_bottom;
    lv_obj_t *screen_SettingSelfieTime_cont_top;
    lv_obj_t *screen_SettingSelfieTime_label_title;
    lv_obj_t *screen_SettingSelfieTime_btn_back;
    lv_obj_t *screen_SettingSelfieTime_btn_back_label;

    /* 拍照设置页面 -- 拍摄模式设置（单张，连拍）*/
    lv_obj_t *screen_SettingShootingMode;
    bool screen_SettingShootingMode_del;
    lv_obj_t *screen_SettingShootingMode_cont_35;
    lv_obj_t *screen_SettingShootingMode_btn_3;
    lv_obj_t *screen_SettingShootingMode_btn_3_label;
    lv_obj_t *screen_SettingShootingMode_btn_2;
    lv_obj_t *screen_SettingShootingMode_btn_2_label;
    lv_obj_t *screen_SettingShootingMode_cont_bottom;
    lv_obj_t *screen_SettingShootingMode_cont_top;
    lv_obj_t *screen_SettingShootingMode_label_title;
    lv_obj_t *screen_SettingShootingMode_btn_back;
    lv_obj_t *screen_SettingShootingMode_btn_back_label;

    /* 拍照设置页面 -- 画质设置*/
    lv_obj_t *screen_SettingPictureQuality;
    bool screen_SettingPictureQuality_del;
    lv_obj_t *screen_SettingPictureQuality_cont_36;
    lv_obj_t *screen_SettingPictureQuality_btn_4;
    lv_obj_t *screen_SettingPictureQuality_btn_4_label;
    lv_obj_t *screen_SettingPictureQuality_btn_3;
    lv_obj_t *screen_SettingPictureQuality_btn_3_label;
    lv_obj_t *screen_SettingPictureQuality_btn_2;
    lv_obj_t *screen_SettingPictureQuality_btn_2_label;
    lv_obj_t *screen_SettingPictureQuality_cont_bottom;
    lv_obj_t *screen_SettingPictureQuality_cont_top;
    lv_obj_t *screen_SettingPictureQuality_label_title;
    lv_obj_t *screen_SettingPictureQuality_btn_back;
    lv_obj_t *screen_SettingPictureQuality_btn_back_label;

    /* 拍照设置页面 -- ISO感光度设置*/
    lv_obj_t *screen_SettingSensitivity;
    bool screen_SettingSensitivity_del;
    lv_obj_t *screen_SettingSensitivity_cont_3;
    lv_obj_t *screen_SettingSensitivity_btn_4;
    lv_obj_t *screen_SettingSensitivity_btn_4_label;
    lv_obj_t *screen_SettingSensitivity_btn_3;
    lv_obj_t *screen_SettingSensitivity_btn_3_label;
    lv_obj_t *screen_SettingSensitivity_btn_2;
    lv_obj_t *screen_SettingSensitivity_btn_2_label;
    lv_obj_t *screen_SettingSensitivity_btn_5;
    lv_obj_t *screen_SettingSensitivity_btn_5_label;
    lv_obj_t *screen_SettingSensitivity_cont_2;
    lv_obj_t *screen_SettingSensitivity_cont_top;
    lv_obj_t *screen_SettingSensitivity_cont_bottom;
    lv_obj_t *screen_SettingSensitivity_label_title;
    lv_obj_t *screen_SettingSensitivity_btn_back;
    lv_obj_t *screen_SettingSensitivity_btn_back_label;

    /* 拍照设置页面 -- 防抖开关设置*/
    lv_obj_t *screen_SettingAntiShake;
    bool screen_SettingAntiShake_del;
    lv_obj_t *screen_SettingAntiShake_cont_settings;
    lv_obj_t *screen_SettingAntiShake_btn_4;
    lv_obj_t *screen_SettingAntiShake_btn_4_label;
    lv_obj_t *screen_SettingAntiShake_btn_3;
    lv_obj_t *screen_SettingAntiShake_btn_3_label;
    lv_obj_t *screen_SettingAntiShake_cont_bottom;
    lv_obj_t *screen_SettingAntiShake_cont_top;
    lv_obj_t *screen_SettingAntiShake_label_title;
    lv_obj_t *screen_SettingAntiShake_btn_back;
    lv_obj_t *screen_SettingAntiShake_btn_back_label;

    /* 拍照设置页面 -- 自动对焦设置*/
    lv_obj_t *screen_SettingAutofocus;
    bool screen_SettingAutofocus_del;
    lv_obj_t *screen_SettingAutofocus_cont_settings;
    lv_obj_t *screen_SettingAutofocus_btn_3;
    lv_obj_t *screen_SettingAutofocus_btn_3_label;
    lv_obj_t *screen_SettingAutofocus_btn_2;
    lv_obj_t *screen_SettingAutofocus_btn_2_label;
    lv_obj_t *screen_SettingAutofocus_cont_bottom;
    lv_obj_t *screen_SettingAutofocus_cont_top;
    lv_obj_t *screen_SettingAutofocus_label_title;
    lv_obj_t *screen_SettingAutofocus_btn_back;
    lv_obj_t *screen_SettingAutofocus_btn_back_label;

    /* 拍照设置页面 -- 人脸侦测*/
    lv_obj_t *screen_SettingFaceDectection;
    bool screen_SettingFaceDectection_del;
    lv_obj_t *screen_SettingFaceDectection_cont_settings;
    lv_obj_t *screen_SettingFaceDectection_btn_3;
    lv_obj_t *screen_SettingFaceDectection_btn_3_label;
    lv_obj_t *screen_SettingFaceDectection_btn_2;
    lv_obj_t *screen_SettingFaceDectection_btn_2_label;
    lv_obj_t *screen_SettingFaceDectection_cont_bottom;
    lv_obj_t *screen_SettingFaceDectection_cont_top;
    lv_obj_t *screen_SettingFaceDectection_label_title;
    lv_obj_t *screen_SettingFaceDectection_btn_back;
    lv_obj_t *screen_SettingFaceDectection_btn_back_label;

    /* 拍照设置页面 -- 笑脸侦测*/
    lv_obj_t *screen_SettingSmileDectection;
    bool screen_SettingSmileDectection_del;
    lv_obj_t *screen_SettingSmileDectection_cont_3;
    lv_obj_t *screen_SettingSmileDectection_btn_3;
    lv_obj_t *screen_SettingSmileDectection_btn_3_label;
    lv_obj_t *screen_SettingSmileDectection_btn_2;
    lv_obj_t *screen_SettingSmileDectection_btn_2_label;
    lv_obj_t *screen_SettingSmileDectection_cont_bottom;
    lv_obj_t *screen_SettingSmileDectection_cont_top;
    lv_obj_t *screen_SettingSmileDectection_label_title;
    lv_obj_t *screen_SettingSmileDectection_btn_back;
    lv_obj_t *screen_SettingSmileDectection_btn_back_label;

    /* 拍照设置页面 -- 美颜*/
    lv_obj_t *screen_SettingBeauty;
    bool screen_SettingBeauty_del;
    lv_obj_t *screen_SettingBeauty_cont_settings;
    lv_obj_t *screen_SettingBeauty_btn_3;
    lv_obj_t *screen_SettingBeauty_btn_3_label;
    lv_obj_t *screen_SettingBeauty_btn_2;
    lv_obj_t *screen_SettingBeauty_btn_2_label;
    lv_obj_t *screen_SettingBeauty_cont_bottom;
    lv_obj_t *screen_SettingBeauty_cont_top;
    lv_obj_t *screen_SettingBeauty_label_title;
    lv_obj_t *screen_SettingBeauty_btn_back;
    lv_obj_t *screen_SettingBeauty_btn_back_label;

    /* 拍汉字 */
    lv_obj_t *screen_TakeChinese;
    bool screen_TakeChinese_del;
    lv_obj_t *screen_TakeChinese_cont_bottom;
    lv_obj_t *screen_TakeChinese_btn_flash;
    lv_obj_t *screen_TakeChinese_btn_flash_label;
    lv_obj_t *screen_TakeChinese_btn_history;
    lv_obj_t *screen_TakeChinese_btn_history_label;
    lv_obj_t *screen_TakeChinese_cont_top;
    lv_obj_t *screen_TakeChinese_btn_back;
    lv_obj_t *screen_TakeChinese_btn_back_label;
    lv_obj_t *screen_TakeChinese_img_box;

    /* 拍英文 */
    lv_obj_t *screen_TakeEnglish;
    bool screen_TakeEnglish_del;
    lv_obj_t *screen_TakeEnglish_cont_bottom;
    lv_obj_t *screen_TakeEnglish_btn_flash;
    lv_obj_t *screen_TakeEnglish_btn_flash_label;
    lv_obj_t *screen_TakeEnglish_btn_history;
    lv_obj_t *screen_TakeEnglish_btn_history_label;
    lv_obj_t *screen_TakeEnglish_cont_top;
    lv_obj_t *screen_TakeEnglish_btn_back;
    lv_obj_t *screen_TakeEnglish_btn_back_label;
    lv_obj_t *screen_TakeEnglish_img_box;

} lv_ui_t;

typedef void (*ui_setup_scr_t)(lv_ui_t *ui);

void ui_init_style(lv_style_t *style);

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
void setup_scr_screenHome2(lv_ui_t *ui);
void setup_scr_screen_AITakePhoto(lv_ui_t *ui);
void setup_scr_screen_TakePhoto(lv_ui_t *ui);
void setup_scr_screen_AIDialog(lv_ui_t *ui);
void setup_scr_screen_AIDialogDr(lv_ui_t *ui);
void setup_scr_screen_AIDialogNZ(lv_ui_t *ui);
void setup_scr_screen_AIDialogDS(lv_ui_t *ui);
void setup_scr_screen_AIEffect(lv_ui_t *ui);
void setup_scr_screen_AIEffectPic(lv_ui_t *ui);
void setup_scr_screen_Settings(lv_ui_t *ui);
void setup_scr_screen_SettingsSys(lv_ui_t *ui);
void setup_scr_screen_SettingsSysWifi(lv_ui_t *ui);
void setup_scr_screen_SettingsSysWifiCode(lv_ui_t *ui);
void setup_scr_screen_SettingsSys4G(lv_ui_t *ui);
void setup_scr_screen_SettingsSysVolume(lv_ui_t *ui);
void setup_scr_screen_SettingsSysLuma(lv_ui_t *ui);
void setup_scr_screen_SettingsSysBl(lv_ui_t *ui);
void setup_scr_screen_SettingsSysAbout(lv_ui_t *ui);
void setup_scr_screen_SettingsSysUpdate(lv_ui_t *ui);
void setup_scr_screen_SettingsSysInfo(lv_ui_t *ui);
void setup_scr_screen_SettingsSysLog(lv_ui_t *ui);
void setup_scr_screen_SettingsSysService(lv_ui_t *ui);
void setup_scr_screen_SettingsSysPrivacy(lv_ui_t *ui);
void setup_scr_screen_SettingsAI(lv_ui_t *ui);
void setup_scr_screen_PhotoAlbum(lv_ui_t *ui);
void setup_scr_screen_PhotoAlbumPic(lv_ui_t *ui);
void setup_scr_screen_PhotoAlbumVid(lv_ui_t *ui);
void setup_scr_screen_Photo(lv_ui_t *ui);
void setup_scr_screen_AIPhoto(lv_ui_t *ui);
void setup_scr_screen_AIPhotoResult(lv_ui_t *ui);
void update_ai_result_text(const char *result_text);
void clear_ai_result_text(void);
void setup_scr_screen_AIPhotoResult1(lv_ui_t *ui);
void setup_scr_screen_AIPhotoResult2(lv_ui_t *ui);
void setup_scr_screen_AIPhotoResult3(lv_ui_t *ui);
void setup_scr_screen_AIPhotoResult4(lv_ui_t *ui);
void setup_scr_screen_TakePhotoSetting(lv_ui_t *ui);
void setup_scr_screen_SettingResolution(lv_ui_t *ui);
void setup_scr_screen_SettingWhiteBalance(lv_ui_t *ui);
void setup_scr_screen_SettingPhotoEffect(lv_ui_t *ui);
void setup_scr_screen_SettingExposure(lv_ui_t *ui);
void setup_scr_screen_SettingPictureMode(lv_ui_t *ui);
void setup_scr_screen_SettingSelfieTime(lv_ui_t *ui);
void setup_scr_screen_SettingShootingMode(lv_ui_t *ui);
void setup_scr_screen_SettingPictureQuality(lv_ui_t *ui);
void setup_scr_screen_SettingSensitivity(lv_ui_t *ui);
void setup_scr_screen_SettingAntiShake(lv_ui_t *ui);
void setup_scr_screen_SettingAutofocus(lv_ui_t *ui);
void setup_scr_screen_SettingFaceDectection(lv_ui_t *ui);
void setup_scr_screen_SettingSmileDectection(lv_ui_t *ui);
void setup_scr_screen_SettingBeauty(lv_ui_t *ui);
void setup_scr_screen_TakeChinese(lv_ui_t *ui);
void setup_scr_screen_TakeEnglish(lv_ui_t *ui);
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
LV_IMAGE_DECLARE(_comparison_yellow_RGB565A8_50x50);
LV_IMAGE_DECLARE(_effect_anime_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_cartoon_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_child_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_clay_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_ink_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_fan_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_impress_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_none_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_web_RGB565A8_52x52);
LV_IMAGE_DECLARE(_effect_anime_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_cartoon_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_child_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_clay_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_ink_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_fan_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_impress_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_none_RGB565A8_44x44);
LV_IMAGE_DECLARE(_effect_web_RGB565A8_44x44);
// LV_IMAGE_DECLARE(_dialog_dr_RGB565A8_136x197);
// LV_IMAGE_DECLARE(_dialog_nezha_RGB565A8_136x197);
// LV_IMAGE_DECLARE(_dialog_deepseek_RGB565A8_136x197);

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

#ifdef __cplusplus
}
#endif
#endif

#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
// #include "events_init.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"
#include <time.h>
#include "ui_common.h"
#include "common/takephoto.h"
#include "indev.h"

lv_obj_t *label_Vedio_Durtime_s; //录像时长
lv_obj_t *obj_vedio_s;           //底层窗口
lv_obj_t *dot_red_s;              //闪烁红点
lv_obj_t *video_red_level_s;         //red_light图标
lv_obj_t *img_sdonline_s = NULL;
lv_obj_t *img_batter_s = NULL;
lv_obj_t *label_available_video_s = NULL;//剩余录像时长
static lv_obj_t *img_effect_s = NULL;  //特效图标
static uint8_t g_flash_led_index = 0;

// lv_obj_t *label_datatime_s = NULL;
extern lv_style_t ttf_font_24;
extern lv_style_t ttf_font_16;
static lv_timer_t *date_timer_s = NULL; //时间更新定时器

extern const char *effect_style_small[];//特效图片数组

extern uint8_t is_start_video;     //录像状态
extern bool is_video_mode;
extern char *batter_image_big[];
extern char *red_light_image_level[];
extern int32_t g_batter_image_index;

static lv_timer_t *g_zoom_longpress_timer = NULL;  // 长按定时器
static int g_zoom_longpress_dir = 0;               // 长按方向: 0=无, 1=缩小, 2=放大
static bool g_zoom_longpress_active = false;       // 是否正在长按

static void video_zoomin_key_cb(void);//w按键回调
static void video_zoomout_key_cb(void);//t按键回调
// left按键处理回调函数
static void video_left_callback(void);
// right按键处理回调函数
static void video_right_callback(void);

static void photo_zoom_event_cb(lv_event_t* e);

// 清理视频页面资源的通用函数
static void cleanup_vedio_page_resources(void)
{
    takephoto_unregister_all_callback();//取消所有按键回调
    if(is_start_video == VEDIO_START) {
        is_start_video = VEDIO_STOP;
        MESSAGE_S Msg  = {0};
        Msg.topic      = EVENT_MODEMNG_STOP_REC;
        MODEMNG_SendMessage(&Msg);
        enable_touch_events(); // 停止录像，恢复TP事件
    }

    // 清理定时器和动画，避免在页面切换时访问已销毁的对象
    if(date_timer_s != NULL)
    {
        lv_timer_delete(date_timer_s);
        date_timer_s = NULL;
    }
    //删除关于特效的资源
    delete_all_handle();
    delete_viewfinder();//销毁取景框
    delete_zoom_bar();//销毁zoombar
    // 释放缩放相关资源
    delete_zoombar_timer_handler();
}

//参数动态更新回调
static void video_var_dynamic_update(lv_timer_t *timer)
{
    // lv_ui_t *ui  = (lv_ui_t *)lv_timer_get_user_data(timer);
    time_t now   = time(NULL);
    struct tm *t = localtime(&now);
    static uint32_t total_seconds = 0;
    // MLOG_DBG("%s[%d]   %d...\n",__func__,__LINE__,lv_obj_is_valid(obj_vedio_s));
    {
        // 检查对象有效性，避免在页面切换时访问已销毁的对象
        if (!lv_obj_is_valid(obj_vedio_s)) {
            return;
        }

        // // 更新日期
        // lv_label_set_text_fmt(label_datatime_s, "%04d/%02d/%02d", t->tm_year + 1900, t->tm_mon + 1,
        //                       t->tm_mday);
        if(is_start_video == VEDIO_STOP)
        {
            if(getSelect_Index() == TIME_FLAG_OFF && !lv_obj_has_flag(label_Vedio_Durtime_s,LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(label_Vedio_Durtime_s, LV_OBJ_FLAG_HIDDEN);
            }
            // 更新时间
            lv_label_set_text_fmt(label_Vedio_Durtime_s, "%04d/%02d/%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1,
                          t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
            lv_obj_set_style_text_color(label_Vedio_Durtime_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        //电池等级更新
        show_image(img_batter_s, batter_image_big[g_batter_image_index]);
        lv_label_set_text_fmt(label_available_video_s, "%s", video_Calculateremainingvideo());
        if(ui_common_cardstatus()) {
            show_image(img_sdonline_s, "icon_card_online.png");
        } else {
            show_image(img_sdonline_s, "icon_card_offline.png");
        }
        //录像状态
        switch(is_start_video)
        {
            case VEDIO_STOP:
            {
                if(!lv_obj_has_flag(dot_red_s,LV_OBJ_FLAG_HIDDEN)&&lv_obj_is_valid(obj_vedio_s))
                {
                    lv_obj_add_flag(dot_red_s, LV_OBJ_FLAG_HIDDEN);
                }
                total_seconds = 0;
            }  ;break;
            case VEDIO_START:
            {
                if(!lv_obj_has_flag(dot_red_s,LV_OBJ_FLAG_HIDDEN)&&lv_obj_is_valid(obj_vedio_s))
                {
                    lv_obj_add_flag(dot_red_s, LV_OBJ_FLAG_HIDDEN);
                }
                else if(lv_obj_has_flag(dot_red_s,LV_OBJ_FLAG_HIDDEN)&&lv_obj_is_valid(obj_vedio_s))
                {
                    lv_obj_remove_flag(dot_red_s, LV_OBJ_FLAG_HIDDEN);
                }
                uint8_t hour = (total_seconds / 3600);
                uint8_t min = (total_seconds%3600)/60;
                uint8_t sec = total_seconds%60;
                total_seconds++;

                lv_label_set_text_fmt(label_Vedio_Durtime_s, "%02d:%02d:%02d",hour, min, sec);
                lv_obj_set_style_text_color(label_Vedio_Durtime_s, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);

            };break;
        }
    }
}

//渐隐动画完成回调
void animCompleted_objDel_cb(lv_anim_t *a)
{
    //移除标志
    if(getSelect_Index() == TIME_FLAG_ON) {
        lv_obj_remove_flag(label_Vedio_Durtime_s, LV_OBJ_FLAG_HIDDEN);
    }
    lv_timer_resume(date_timer_s);
    delete_all_handle();
}

// ok按键处理回调函数
static void video_sesor_switch_completed_callback(void)
{
    enable_touch_events(); // 恢复触摸
    enable_hardware_input_device(0);
    enable_hardware_input_device(1);
}

static void buttonVedio_All_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int Click_index = (int)lv_event_get_user_data(e);
    MLOG_DBG("code:%d, Click_index:%d\n", code, Click_index);

    switch(code) {
        case LV_EVENT_CLICKED: {
            // 清理视频页面资源
            cleanup_vedio_page_resources();
            if(Click_index == 1) {
                ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                      false, true);
            } else if(Click_index == 2) {
                // 进入拍照模式
                MESSAGE_S Msg = {0};
                Msg.topic = EVENT_MODEMNG_MODESWITCH;
                Msg.arg1 = WORK_MODE_PHOTO;
                MODEMNG_SendMessage(&Msg);
                // 复位缩放
                set_zoom_level(1);
                // 使能对焦
                enable_focus();
                is_video_mode = false;
                ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del, NULL, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                      false, true);
            } else if(Click_index == 3) {
                MESSAGE_S Msg = {0};
                takephoto_cancel_focus();
                // 通知mode关闭时要关闭sensor
                Msg.topic     = EVENT_MODEMNG_SENSOR_STATE;
                Msg.arg1      = 1;
                MODEMNG_SendMessage(&Msg);
                memset(&Msg, 0, sizeof(MESSAGE_S));
                // 进入BOOT模式
                Msg.topic = EVENT_MODEMNG_MODESWITCH;
                Msg.arg1  = WORK_MODE_BOOT;
                MODEMNG_SendMessage(&Msg);
                // 复位缩放
                set_zoom_level(1);
                is_video_mode = false;
                ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                      true);
            } else if(Click_index == 4) {
                // 进入录像模式
                MESSAGE_S Msg = { 0 };
                Msg.topic = EVENT_MODEMNG_MODESWITCH;
                Msg.arg1 = WORK_MODE_PLAYBACK;
                MODEMNG_SendMessage(&Msg);
                // 复位缩放
                set_zoom_level(1);
                is_video_mode = false;
                ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            }
            break;
        }
        default: break;
    }
}

void video_effect_scr_delete(void)
{
    delete_all_handle();
    lv_timer_ready(date_timer_s);
    lv_timer_resume(date_timer_s);
}

//特效选择回调
static void vedioEffect_Select_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED:
        {
            if(get_is_effect_exist() == true) {
                delete_all_handle();
                lv_timer_resume(date_timer_s);
            } else {
                // 添加隐藏时间标志
                lv_timer_pause(date_timer_s);
                // 创建滚动列表
                float_effect_creat(img_effect_s,obj_vedio_s);
                // 创建控件并启动渐渐隐藏动画
                create_gradually_hide_anim(animCompleted_objDel_cb,8000);
            }
            // Update current screen layout.
            lv_obj_update_layout(obj_vedio_s);
        } break;
        default: break;
    }
}

// 菜单按键处理回调函数
static void key_takephoto_menu_callback(void)
{
    MLOG_DBG("进入录像模式设置页面\n");

    // 清理视频页面资源
    cleanup_vedio_page_resources();

    ui_load_scr_animation(&g_ui, &obj_vedioMenu_s, 1, NULL, vedioMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0,
        false, true);
}

// UP按键处理回调函数
static void video_up_callback(void)
{
    g_flash_led_index = !g_flash_led_index;
    MESSAGE_S Msg = {0};
    Msg.topic     = EVENT_MODEMNG_SETTING;
    Msg.arg1      = PARAM_MENU_FLASH_LED;
    Msg.arg2      = g_flash_led_index;
    MODEMNG_SendMessage(&Msg);

}

// 模式切换按键处理回调函数
static void key_takephoto_mode_callback(void)
{
    MLOG_DBG("模式切换，进入拍照模式\n");

    // 清理视频页面资源
    cleanup_vedio_page_resources();

    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1 = WORK_MODE_PHOTO;
    MODEMNG_SendMessage(&Msg);
    // 复位缩放
    set_zoom_level(1);
    // 使能对焦
    enable_focus();
    is_video_mode = false;
    ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del, NULL, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                            false, true);
}

static void vieo_key_down_callback(void)
{
    create_simple_delete_dialog(NULL);//创建确认浮窗
}

// 长按菜单按键处理回调函数
static void video_long_menu_callback(void)
{
    cleanup_vedio_page_resources();
    MESSAGE_S Msg = {0};
    takephoto_cancel_focus();
    // 通知mode关闭时要关闭sensor
    Msg.topic     = EVENT_MODEMNG_SENSOR_STATE;
    Msg.arg1      = 1;
    MODEMNG_SendMessage(&Msg);
    memset(&Msg, 0, sizeof(MESSAGE_S));
    // 进入BOOT模式
    Msg.topic     = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1      = WORK_MODE_BOOT;
    MODEMNG_SendMessage(&Msg);
    // 复位缩放
    set_zoom_level(1);
    is_video_mode = false;
    ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

// ok按键处理回调函数
static void key_takephoto_ok_callback(void)
{
    if(get_is_effect_exist() == false) {
    } else {
        set_effect_ok();
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
                    MESSAGE_S Msg = {0};
                    takephoto_cancel_focus();
                    // 通知mode关闭时要关闭sensor
                    Msg.topic     = EVENT_MODEMNG_SENSOR_STATE;
                    Msg.arg1      = 1;
                    MODEMNG_SendMessage(&Msg);
                    memset(&Msg, 0, sizeof(MESSAGE_S));
                    // 进入BOOT模式
                    Msg.topic     = EVENT_MODEMNG_MODESWITCH;
                    Msg.arg1      = WORK_MODE_BOOT;
                    MODEMNG_SendMessage(&Msg);
                    // 复位缩放
                    set_zoom_level(1);
                    is_video_mode = false;
                    ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                          false, true);
                    cleanup_vedio_page_resources();
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}


static void video_redlight_callback(void)
{
    if (brightness_level > 6) {
        show_image(video_red_level_s, red_light_image_level[6]);
    } else {
        show_image(video_red_level_s, red_light_image_level[brightness_level]);
    }
}


void Home_Vedio(lv_ui_t *ui)
{
    MLOG_DBG("loading obj_vedio_s...\n");
    // 创建主页面1 容器
    if(obj_vedio_s != NULL) {
        if(lv_obj_is_valid(obj_vedio_s)) {
            MLOG_DBG("obj_vedio_s 仍然有效，删除旧对象\n");
            lv_obj_del(obj_vedio_s);
        } else {
            MLOG_DBG("obj_vedio_s 已被自动销毁，仅重置指针\n");
        }
        obj_vedio_s = NULL;
    }
    obj_vedio_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_vedio_s, H_RES, V_RES);

    lv_obj_set_scrollbar_mode(obj_vedio_s, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条，默认 LV_SCROLLBAR_MODE_AUTO
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj_vedio_s, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(obj_vedio_s, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 背景颜色
    lv_obj_set_style_bg_grad_dir(obj_vedio_s, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);    // 无渐变色
    lv_obj_add_event_cb(obj_vedio_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // 录像模式
    lv_obj_t *img_mode = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(img_mode, LV_ALIGN_TOP_LEFT, 6, 0);
    lv_obj_set_size(img_mode, 74, 47);
    show_image(img_mode, "shexiangmoshi.png");
    lv_obj_add_event_cb(img_mode, buttonVedio_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)2);

    // res
    lv_obj_t *img_res = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(img_res, LV_ALIGN_TOP_LEFT, 72+14, 4);
    lv_obj_set_size(img_res, 38, 32);
    show_image(img_res, video_getRes_Icon());
    lv_obj_update_layout(img_res);

    //闪光灯
    video_red_level_s = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(video_red_level_s, LV_ALIGN_TOP_LEFT, 116+14, 4);
    lv_obj_set_size(video_red_level_s, 38, 32);
    show_image(video_red_level_s, red_light_image_level[brightness_level]);


    lv_obj_t *iso_level = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(iso_level, LV_ALIGN_TOP_LEFT, 160+14, 4);
    lv_obj_set_size(iso_level, 38, 32);
    char* iso_buf[] = {
        "ISO.png",
        "ISO 100.png",
        "ISO 200.png",
        "ISO 400.png",
        "ISO 800.png",
        "ISO 1600.png",
        "ISO 3200.png",
        "ISO 6400.png",
    };

    show_image(iso_level, iso_buf[get_iso_index()]);

    lv_obj_t *screenbrightness_level = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(screenbrightness_level, LV_ALIGN_TOP_LEFT, 204+14, 4);
    lv_obj_set_size(screenbrightness_level, 38, 32);
    char* brightness_buf[] = { "1.png", "2.png", "3.png", "4.png", "5.png", "6.png", "7.png" };
    show_image(screenbrightness_level, brightness_buf[get_curr_brightness()]);

    // // 白平衡
    // const char *whitebalanc[] = {"WAB AUTo.png", "白平衡太阳.png", "白平衡阴天.png", "白平衡灯泡.png",
    //                              "白平衡日光灯.png"};
    // lv_obj_t *img_whitebalanc = lv_imagebutton_create(obj_vedio_s);
    // lv_obj_align(img_whitebalanc, LV_ALIGN_TOP_LEFT, 194, 0);
    // if(getwhiBlance_Index() != 0)
    //     lv_obj_set_size(img_whitebalanc, 58, 58);
    // else
    //     lv_obj_set_size(img_whitebalanc, 0, 58);
    // show_image(img_whitebalanc, whitebalanc[getwhiBlance_Index()]);
    // lv_obj_set_style_image_recolor(img_whitebalanc, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // lv_obj_set_style_image_recolor_opa(img_whitebalanc, LV_OPA_COVER, LV_PART_MAIN);
    // lv_obj_set_style_image_recolor(img_whitebalanc, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // lv_obj_update_layout(img_whitebalanc);
    // // 曝光
    // static const char *photo_EV_s[] = {
    //     "ev3.png", "EV2.png", "EV1.png", "EV00.png", "EV11.png", "EV22.png", "EV33.png",
    // };
    // lv_obj_t *img_ev = lv_imagebutton_create(obj_vedio_s);
    // lv_obj_align(img_ev, LV_ALIGN_TOP_LEFT, lv_obj_get_x(img_whitebalanc) + lv_obj_get_width(img_whitebalanc) - 4, 0);
    // if(get_EV_Level() != 3)
    //     lv_obj_set_size(img_ev, 58, 58);
    // else
    //     lv_obj_set_size(img_ev, 0, 58);
    // show_image(img_ev, photo_EV_s[get_EV_Level()]);
    // lv_obj_update_layout(img_ev);

    // 剩余录像时间
    label_available_video_s = lv_label_create(obj_vedio_s);
    lv_obj_add_style(label_available_video_s, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_available_video_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(label_available_video_s, "%s", video_Calculateremainingvideo());
    lv_label_set_long_mode(label_available_video_s, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_available_video_s, LV_ALIGN_TOP_RIGHT, -104, 6);

    // sd
    img_sdonline_s = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(img_sdonline_s, LV_ALIGN_TOP_RIGHT, -58, 0);
    lv_obj_set_size(img_sdonline_s, 38, 32);
    if(ui_common_cardstatus()) {
        show_image(img_sdonline_s, "icon_card_online.png");
    } else {
        show_image(img_sdonline_s, "icon_card_offline.png");
    }

    // batter
    img_batter_s = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(img_batter_s, LV_ALIGN_TOP_RIGHT, -8, 2);
    lv_obj_set_size(img_batter_s, 38, 2);
    show_image(img_batter_s,"充电.png");


    //缩放
    lv_obj_t *imgbtn_zoomout = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(imgbtn_zoomout, LV_ALIGN_LEFT_MID, 12, -42);
    lv_obj_set_size(imgbtn_zoomout, 38, 38);
    show_image(imgbtn_zoomout, "T.png");
    lv_obj_add_event_cb(imgbtn_zoomout, photo_zoom_event_cb, LV_EVENT_ALL, (void *)(intptr_t)2);

    lv_obj_t *imgbtn_zoomin = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(imgbtn_zoomin, LV_ALIGN_LEFT_MID, 12, 42);
    lv_obj_set_size(imgbtn_zoomin, 38, 38);
    show_image(imgbtn_zoomin, "W.png");
    lv_obj_add_event_cb(imgbtn_zoomin, photo_zoom_event_cb, LV_EVENT_ALL, (void *)(intptr_t)1);


    // menu
    lv_obj_t *img_menu = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(img_menu, LV_ALIGN_BOTTOM_LEFT, 6, 0);
    lv_obj_set_size(img_menu, 40, 40);
    show_image(img_menu, "menu.png");
    lv_obj_add_event_cb(img_menu, buttonVedio_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)1);
    
    // 滤镜
    lv_obj_t *btn_effect = lv_button_create(obj_vedio_s);
    lv_obj_align(btn_effect, LV_ALIGN_BOTTOM_LEFT, 60,0);
    lv_obj_set_size(btn_effect, 40, 40);
    lv_obj_set_style_bg_opa(btn_effect, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_pad_all(btn_effect, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_effect, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_effect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_effect, vedioEffect_Select_event_cb, LV_EVENT_CLICKED, NULL);

    img_effect_s = lv_img_create(btn_effect);
    lv_obj_set_size(img_effect_s, 40, 40);
    lv_obj_align(img_effect_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(img_effect_s, 0, LV_STATE_DEFAULT);
    show_image(img_effect_s, "颜色特效.png");


    label_Vedio_Durtime_s = lv_label_create(obj_vedio_s);
    lv_obj_add_style(label_Vedio_Durtime_s, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_Vedio_Durtime_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(label_Vedio_Durtime_s, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(label_Vedio_Durtime_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_Vedio_Durtime_s, LV_ALIGN_BOTTOM_MID, 0, -12);
    if(getSelect_Index() == TIME_FLAG_OFF) {
        lv_obj_add_flag(label_Vedio_Durtime_s, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_t *album = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(album, LV_ALIGN_BOTTOM_RIGHT, -80, 0);
    lv_obj_set_size(album, 40, 40);
    show_image(album, "photo_album.png");
    lv_obj_add_event_cb(album, buttonVedio_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)4);


    // 退出按钮
    lv_obj_t *img_exit = lv_imagebutton_create(obj_vedio_s);
    lv_obj_align(img_exit, LV_ALIGN_BOTTOM_RIGHT, -6, -6);
    lv_obj_set_size(img_exit, 41, 40);
    show_image(img_exit, "exit.png");
    lv_obj_add_event_cb(img_exit, buttonVedio_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)3);
    
    //创建缩放UI
    create_zoom_bar(obj_vedio_s);

    // 闪烁圆点
    dot_red_s = lv_obj_create(obj_vedio_s);
    lv_obj_set_size(dot_red_s, 24, 24);
    lv_obj_align(dot_red_s, LV_ALIGN_BOTTOM_MID, -80, -14);
    lv_obj_set_style_radius(dot_red_s, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dot_red_s, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(dot_red_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(dot_red_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(dot_red_s, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_pad_all(dot_red_s, 0, LV_STATE_DEFAULT);

    create_viewfinder(obj_vedio_s);

    /* 设置当前页面按键处理回调 */
    set_current_page_handler(takephoto_key_handler);
    takephoto_register_up_callback(video_redlight_callback);
    takephoto_register_down_callback(video_redlight_callback);
    takephoto_register_menu_callback(key_takephoto_menu_callback);
    takephoto_register_mode_callback(key_takephoto_mode_callback);
    takephoto_register_ok_callback(key_takephoto_ok_callback);
    takephoto_register_long_menu_callback(video_long_menu_callback);
    // takephoto_register_long_mode_callback(video_long_mode_callback);
    takephoto_register_zoomin_callback(video_zoomin_key_cb);
    takephoto_register_zoomout_callback(video_zoomout_key_cb);
    takephoto_register_left_callback(video_left_callback);
    takephoto_register_right_callback(video_right_callback);
    //创建时间更新定时器
    if(date_timer_s == NULL) {
        date_timer_s = lv_timer_create(video_var_dynamic_update, 1000, ui);
    }
    // 立即执行一次更新
    lv_timer_ready(date_timer_s);
    lv_obj_update_layout(obj_vedio_s);

}

// 缩放按键事件处理
static void video_zoomin_key_cb(void)
{
    uint32_t new_level = get_zoom_level();
    // 设置放大比例
    MESSAGE_S Msg = {0};
    Msg.topic     = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
    Msg.arg1      = 0;
    snprintf((char *)Msg.aszPayload, 3, "%d", new_level);
    MODEMNG_SendMessage(&Msg);
    // 更新UI
    update_zoom_bar(new_level);
}

// 缩放按键事件处理
static void video_zoomout_key_cb(void)
{
    uint32_t new_level = get_zoom_level();
    // 设置放大比例
    MESSAGE_S Msg = {0};
    Msg.topic     = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
    Msg.arg1      = 0;
    snprintf((char *)Msg.aszPayload, 3, "%d", new_level);
    MODEMNG_SendMessage(&Msg);
    // 更新UI
    update_zoom_bar(new_level);
}

// left按键处理回调函数
static void video_left_callback(void)
{
    if(get_is_effect_exist() == true) {
        effect_Select_prev();
    }
}

// right按键处理回调函数
static void video_right_callback(void)
{
    if(get_is_effect_exist() == true) {
        effect_AISelect_next();
    }
}

// 长按定时器回调函数
static void zoom_longpress_timer_cb(lv_timer_t *timer)
{

    if (obj_vedio_s == NULL || !lv_obj_is_valid(obj_vedio_s)) {
        g_zoom_longpress_active = false;
        if (g_zoom_longpress_timer != NULL) {
            lv_timer_del(g_zoom_longpress_timer);
            g_zoom_longpress_timer = NULL;
        }
        return;
    }

    if (!g_zoom_longpress_active) {
        lv_timer_del(timer);
        g_zoom_longpress_timer = NULL;
        return;
    }
    
    uint32_t new_level = get_zoom_level();
    MESSAGE_S Msg = {0};
    bool can_continue = false;
    
    switch (g_zoom_longpress_dir) {
        case 1: // 缩小
            if (new_level > 1) {
                new_level--;
                can_continue = true;
            }
            break;
            
        case 2: // 放大
            if (new_level < ZOOM_RADIO_MAX) {
                new_level++;
                can_continue = true;
            }
            break;
    }
    
    if (can_continue) {
        set_zoom_level(new_level);
        new_level = get_zoom_level();
        
        Msg.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
        Msg.arg1 = 0;
        snprintf((char*)Msg.aszPayload, 3, "%d", new_level);
        MODEMNG_SendMessage(&Msg);
        update_zoom_bar(new_level);
        
        MLOG_DBG("长按缩放: 方向=%d, 等级=%d\n", g_zoom_longpress_dir, new_level);
    } else {
        // 达到边界，停止长按
        g_zoom_longpress_active = false;
        lv_timer_del(timer);
        g_zoom_longpress_timer = NULL;
    }
}

static void photo_zoom_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int click_index = (int)lv_event_get_user_data(e);
    static uint32_t last_click_time = 0;
    
    if (obj_vedio_s == NULL || !lv_obj_is_valid(obj_vedio_s)) {
        g_zoom_longpress_active = false;
        if (g_zoom_longpress_timer != NULL) {
            lv_timer_del(g_zoom_longpress_timer);
            g_zoom_longpress_timer = NULL;
        }
        return;
    }

    switch(code) {
        case LV_EVENT_PRESSED: {
            // 立即执行一次缩放
            uint32_t new_level = get_zoom_level();
            MESSAGE_S msg = {0};
            bool zoom_performed = false;
            
            if (click_index == 1 && new_level > 1) { // 缩小
                new_level--;
                zoom_performed = true;
            } else if (click_index == 2 && new_level < ZOOM_RADIO_MAX) { // 放大
                new_level++;
                zoom_performed = true;
            }
            
            if (zoom_performed) {
                set_zoom_level(new_level);
                new_level = get_zoom_level();
                
                msg.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
                msg.arg1 = 0;
                snprintf((char*)msg.aszPayload, 3, "%d", new_level);
                MODEMNG_SendMessage(&msg);
                update_zoom_bar(new_level);
                
                MLOG_DBG("缩放按钮按下: 方向=%d, 新等级=%d\n", click_index, new_level);
            }
            
            // 记录按下时间
            last_click_time = lv_tick_get();
            
            // 启动长按定时器
            g_zoom_longpress_dir = click_index;
            g_zoom_longpress_active = true;
            if (g_zoom_longpress_timer == NULL) {
                g_zoom_longpress_timer = lv_timer_create(zoom_longpress_timer_cb, 300, NULL);
            }
            break;
        }
        
        case LV_EVENT_RELEASED: {
            // 停止长按
            g_zoom_longpress_active = false;
            if (g_zoom_longpress_timer != NULL) {
                lv_timer_del(g_zoom_longpress_timer);
                g_zoom_longpress_timer = NULL;
            }
            
            // 计算按下时间
            uint32_t press_duration = lv_tick_get() - last_click_time;
            
            // 如果是短按（小于300ms），不执行额外操作
            if (press_duration < 300) {
                MLOG_DBG("短按释放: 持续时间=%dms\n", press_duration);
            } else {
                MLOG_DBG("长按释放: 持续时间=%dms\n", press_duration);
            }
            break;
        }
        default:
        break;
    }
}
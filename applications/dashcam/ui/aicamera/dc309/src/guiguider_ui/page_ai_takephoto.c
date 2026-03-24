#define DEBUG
#include "common/extract_thumbnail.h"
#include "common/takephoto.h"
#include "config.h"
#include "custom.h"
#include "gui_guider.h"
#include "image_recognize/image_recognize.h"
#include "indev.h"
#include "lvgl.h"
#include "mapi_ao.h"
#include "mlog.h"
#include "page_all.h"
#include "ttp.h"
#include "ui_common.h"
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 全局变量声明 */
char pic_thumbnail[128] = { 0 };
char recognize_result[4096] = { 0 };
// 底层页面控件
lv_obj_t* page_ai_camera_s = NULL;
// 识别物体预览控件
static lv_obj_t* camera_preview_s = NULL;

// 识别结果文本
static const char* result_text = NULL;
// 结果页面描述标签
static lv_obj_t* result_desc_label = NULL;
// 识别结果更新定时器
static lv_timer_t* update_result_timer = NULL;
static lv_timer_t* read_status_timer = NULL;

static lv_obj_t* text_cont = NULL;
static lv_obj_t* corner_deco = NULL;

static lv_obj_t* voice_man = NULL;
//重新朗读文本
static lv_obj_t* s_reread_btn = NULL;
static lv_obj_t* s_reread_label = NULL;
//是否正在朗读
static bool s_is_read = false;
/* 函数声明 */
// 显示识别结果页面
static void show_recognition_result(void);
// 识别结果更新定时器回调函数
static void update_recognition_result_cb(lv_timer_t *timer);
// 结果返回按钮回调函数
static void back_btn_result_cb(lv_event_t *e);
// 底层返回按钮回调函数
static void back_cb(lv_event_t *e);
// 菜单按键处理回调函数
static void aiphoto_menu_callback(void);
//结果页面按键处理
static void aiphoto_result_key_handler(int key_code, int key_value);

static void voice_value_btn_event_cb(lv_event_t *e);
static void wifi_return_to_ai_camera(void *user_data);

static void voice_style_btn_event_cb(lv_event_t *e);

//快捷删除按键处理回调函数
static void photo_key_down_callback(void);
// 模式切换按键处理回调函数
static void photo_mode_callback(void);

// UP按键处理回调函数
static void photo_up_callback(void);

// ok按键处理回调函数
static void photo_ok_callback(void);
//重读
static void reread_btn_event_cb(lv_event_t* e);
static void get_read_status_cb(lv_timer_t* timer);

/* 识别图片
 * @param image_path 图片路径
 * @return 识别结果
 */
static char *recognize_image(const char *image_path)
{
    int ret;
    image_recognizer_t *recognizer;

    char prompt[200] = {0};
    extern char g_sysbtn_labelLanguage[32];
    snprintf(prompt, sizeof(prompt), "用50字左右的%s语言介绍一下这张图,注意抓关键信息\n",g_sysbtn_labelLanguage);
    recognizer = image_recognizer_create(IMAGE_RECOGNIZE_MODEL_NAME, IMAGE_RECOGNIZE_API_KEY, IMAGE_RECOGNIZE_BASE_URL);
    ret = image_recognizer_from_file(recognizer, image_path, prompt,recognize_result, sizeof(recognize_result));
    if(ret != 0) MLOG_ERR("图像识别失败: %s\n", image_recognizer_get_error_string(ret));
    image_recognizer_destroy(recognizer);

    return ret == 0 ? recognize_result : NULL;
}

static void ttp_start_cb(lv_timer_t *timer)
{
    char text[4096] = { 0 };
    strncpy(text, result_text, sizeof(text) - 1);
    s_is_read = true;
    ttp_play(text);
    if (get_play_switch()) {
        lv_label_set_text(s_reread_label, str_language_pause_reading[get_curr_language()]);
        lv_obj_clear_flag(s_reread_btn, LV_OBJ_FLAG_HIDDEN);
        if (read_status_timer == NULL)
            read_status_timer = lv_timer_create(get_read_status_cb, 1000, NULL);
    }
    lv_timer_del(timer);
}

/* 安全删除控件函数 */
static void safe_delete_obj(lv_obj_t **obj_ptr)
{
    if(obj_ptr && *obj_ptr) {
        if(lv_obj_is_valid(*obj_ptr)) {
            MLOG_DBG("Deleting object at %p\n", *obj_ptr);
            lv_obj_del(*obj_ptr);
        }
        *obj_ptr = NULL;
    }
}

/* 安全删除定时器 */
static void safe_delete_timer(lv_timer_t **timer_ptr)
{
    if(timer_ptr && *timer_ptr) {
        MLOG_DBG("Deleting timer at %p\n", *timer_ptr);
        lv_timer_del(*timer_ptr);
        *timer_ptr = NULL;
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
                    /* 取消注册拍照后处理回调函数 */
                    takephoto_unregister_callback();
                    /* 取消拍照按键处理回调函数 */
                    set_current_page_handler(NULL);

                    MESSAGE_S Msg = {0};
                    // 通知mode关闭时要关闭sensor
                    Msg.topic     = EVENT_MODEMNG_SENSOR_STATE;
                    Msg.arg1      = 1;
                    MODEMNG_SendMessage(&Msg);
                    memset(&Msg, 0, sizeof(MESSAGE_S));
                    // 进入BOOT模式
                    Msg.topic     = EVENT_MODEMNG_MODESWITCH;
                    Msg.arg1      = WORK_MODE_BOOT;
                    MODEMNG_SendMessage(&Msg);
                    voice_arc_delete();
                    delete_viewfinder();
                    delete_zoom_bar();
                    delete_zoombar_timer_handler();
                    takephoto_cancel_focus();
                    /* 清除播放线程 */
                    ttp_deinit();
                    /* 返回主页 */
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

// 放大按键事件处理
static void zoomin_key_cb(void)
{
    if(get_arc_handel() != NULL) {
        voice_arc_delete();
        voice_setting_arc_create();
        volume_add();
    } else {
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
}

// 缩小按键事件处理
static void zoomout_key_cb(void)
{
    if(get_arc_handel() != NULL) {
        voice_arc_delete();
        voice_setting_arc_create();
        volume_reduce();
    } else {
        uint32_t new_level = get_zoom_level();;
        // 设置放大比例
        MESSAGE_S Msg = {0};
        Msg.topic     = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
        Msg.arg1      = 0;
        snprintf((char *)Msg.aszPayload, 3, "%d", new_level);
        MODEMNG_SendMessage(&Msg);
        // 更新UI
        update_zoom_bar(new_level);
    }
}


/* 创建AI拍照页面 */
void create_ai_camera_screen(lv_ui_t *ui)
{
    MLOG_DBG("[AI Camera] Creating AI camera screen\n");
    set_exit_completed(false);
    // 复位缩放
    set_zoom_level(1);
    /* 创建页面容器 */
    page_ai_camera_s = lv_obj_create(NULL);
    if(!page_ai_camera_s) {
        MLOG_ERR("[AI Camera] Failed to create screen container!\n");
        return;
    }

    lv_obj_remove_style_all(page_ai_camera_s);
    lv_obj_set_size(page_ai_camera_s, H_RES, V_RES);
    lv_obj_set_style_bg_color(page_ai_camera_s, lv_color_hex(0x020524), 0);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(page_ai_camera_s, LV_OPA_TRANSP,
                            LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明度 0~255，255 完全不透明
    lv_obj_add_event_cb(page_ai_camera_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    /* 创建顶部标题栏 */
    lv_obj_t *header = lv_obj_create(page_ai_camera_s);
    lv_obj_set_size(header, LV_PCT(100), 60);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x020524), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 创建返回按钮 */
    lv_obj_t *back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 60, 52);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x020524), 0);
    lv_obj_set_style_radius(back_btn, 20, 0);
    lv_obj_set_style_shadow_width(back_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 4, 4);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_center(back_label);
    lv_obj_set_style_text_font(back_label, &lv_font_SourceHanSerifSC_Regular_30,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    /* 添加返回事件 */
    lv_obj_add_event_cb(back_btn, back_cb, LV_EVENT_CLICKED, NULL);

    /* 创建标题 */
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, str_language_ai_photo_recognition[get_curr_language()]);
    lv_obj_set_style_text_font(title, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_center(title);
    lv_obj_set_width(title,H_RES - 128);
    lv_label_set_long_mode(title, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 音色选择按钮（右侧）
    lv_obj_t *voice_style_btn = lv_btn_create(header);
    lv_obj_set_size(voice_style_btn, 60, 52);
    lv_obj_set_pos(voice_style_btn, H_RES - 64, 4); // 右侧位置
    lv_obj_set_style_pad_all(voice_style_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(voice_style_btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(voice_style_btn, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(voice_style_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *voice_style_label = lv_label_create(voice_style_btn);
    lv_obj_set_style_text_align(voice_style_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(voice_style_label, LV_SYMBOL_VOLUME_MAX); // 使用音量图标
    lv_obj_align(voice_style_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(voice_style_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加设置音量
    lv_obj_add_event_cb(voice_style_btn, voice_value_btn_event_cb, LV_EVENT_CLICKED, NULL);

    voice_man = lv_imagebutton_create(header);
    lv_obj_align(voice_man, LV_ALIGN_RIGHT_MID, -88, 0);
    lv_obj_set_size(voice_man, 55, 48);
    set_voice_style_icon(voice_man);
    if(get_currindex()) {
        show_image(voice_man, "女声.png");
    } else {
        show_image(voice_man, "男声.png");
    }
    lv_obj_set_style_pad_all(voice_man, 0, LV_STATE_DEFAULT);
    lv_obj_add_event_cb(voice_man, voice_style_btn_event_cb, LV_EVENT_CLICKED, NULL);
    /* 创建相机预览区域 */
    camera_preview_s = lv_img_create(page_ai_camera_s);
    lv_obj_set_size(camera_preview_s, H_RES, V_RES - 100);
    lv_obj_align(camera_preview_s, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_opa(camera_preview_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明度 0~255，255 完全不透明

    create_viewfinder(page_ai_camera_s);
    create_zoom_bar(page_ai_camera_s);
    /* 拍照按键处理回调函数 */
    set_current_page_handler(takephoto_key_handler);
    takephoto_register_menu_callback(aiphoto_menu_callback);
    takephoto_register_zoomin_callback(zoomin_key_cb);
    takephoto_register_zoomout_callback(zoomout_key_cb);
    takephoto_register_down_callback(photo_key_down_callback);
    takephoto_register_up_callback(photo_up_callback);
    takephoto_register_mode_callback(photo_mode_callback);
    takephoto_register_ok_callback(photo_ok_callback);
    /* 注册拍照后处理回调函数 */
    takephoto_register_callback(show_recognition_result);

    /* 检查WiFi连接状态 */
    wifi_check_and_show_dialog(page_ai_camera_s, wifi_return_to_ai_camera, ui);

    /* 创建ttp线程 */
    ttp_init();

    MLOG_DBG("[AI Camera] AI camera screen created successfully\n");
}

void takephoto_result_resources(void) // 要先删除结果页面在加载拍照页面
{
    /* 清理结果页面相关资源 */
    safe_delete_timer(&update_result_timer);
    result_desc_label = NULL;
    memset(&result_text, 0, sizeof(result_text));
    voice_arc_delete();
    safe_delete_timer(&read_status_timer);
    /* 安全删除结果页面 */
    lv_obj_t* current_scr = lv_scr_act();
    if (current_scr && lv_obj_is_valid(current_scr) && current_scr != page_ai_camera_s) {
        safe_delete_obj(&current_scr);
    }
}

static void gesture_result_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_GESTURE: {
            // 获取手势方向，需要 TP 驱动支持
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
            switch(dir) {
                case LV_DIR_RIGHT: {
                    /* tts+player复位 */
                    ttp_reset();
                    takephoto_result_resources();
                    /* 确保拍照页面存在 */
                    if(!page_ai_camera_s || !lv_obj_is_valid(page_ai_camera_s)) {
                        MLOG_WARN("Camera screen not valid, recreating...\n");
                        create_ai_camera_screen(NULL);
                    } else {
                        /* 设置拍照按键处理回调函数 */
                        set_current_page_handler(takephoto_key_handler);
                    }
                    /* 切换回拍照页面 */
                    lv_scr_load(page_ai_camera_s);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

/* 显示识别结果 */
static void show_recognition_result(void)
{
    voice_arc_delete();
    delete_zoombar_timer_handler();//删除自动隐藏缩放UI timer
    set_zoom_level(1);
    delete_viewfinder();
    hide_zoom_bar();

    /* 取消拍照按键处理回调函数 */
    set_current_page_handler(NULL);

    /* 创建结果页面容器 */
    lv_obj_t *result_page = lv_obj_create(NULL);
    if(!result_page) {
        MLOG_ERR("[AI Result] Failed to create result page!\n");
        return;
    }

    lv_obj_remove_style_all(result_page);
    lv_obj_set_size( result_page, H_RES, V_RES);
    lv_obj_add_style( result_page, &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(result_page, gesture_result_event_handler, LV_EVENT_GESTURE, NULL);

    /* 创建顶部标题栏 */
    lv_obj_t *header = lv_obj_create(result_page);
    lv_obj_set_size(header, LV_PCT(100), 60);
    lv_obj_add_style(header, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *back_btn = lv_btn_create(header);
    if(back_btn) {
        lv_obj_set_size(back_btn, 60, 52);
        lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 4, 4);
        lv_obj_add_style(back_btn, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t *back_label = lv_label_create(back_btn);
        if(back_label) {
            lv_label_set_text(back_label, LV_SYMBOL_LEFT);
            lv_obj_add_style(back_label, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_center(back_label);
            lv_obj_set_style_text_font(back_label, &lv_font_SourceHanSerifSC_Regular_30,
                            LV_PART_MAIN | LV_STATE_DEFAULT);
        }

        /* 添加返回事件 */
        lv_obj_add_event_cb(back_btn, back_btn_result_cb, LV_EVENT_CLICKED, NULL);
    }

    /* 创建标题文本 */
    lv_obj_t *title_label = lv_label_create(header);
    if(title_label) {
        lv_label_set_text(title_label, str_language_recognition_result[get_curr_language()]);
        lv_obj_set_style_text_font(title_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), 0);
        lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
        lv_obj_center(title_label);
    }

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(result_page);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    /* 创建主体内容容器 */
    lv_obj_t *content = lv_obj_create(result_page);
    lv_obj_set_size(content, LV_PCT(100), V_RES - 62);
    lv_obj_align(content, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x020524), 0);
    lv_obj_set_style_pad_all(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(content, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(content, 0, 0);

    MLOG_DBG("show pic: %s\n", pic_filepath);
    char path_small[100] = {0};
    char path_large[100] = {0};
    // 使用统一的路径获取函数
    get_thumbnail_path(pic_filepath, path_small, sizeof(path_small), PHOTO_SMALL_PATH);
    get_thumbnail_path(pic_filepath, path_large, sizeof(path_large), PHOTO_LARGE_PATH);

    char *rel_large = strchr(path_large, '/');
    strncpy(pic_thumbnail, rel_large, sizeof(pic_thumbnail));
    /* 识别图片 */
    lv_obj_t* result_img = lv_img_create(content);
    if (result_img) {
        lv_obj_set_size(result_img, 200,140);
        lv_obj_align(result_img, LV_ALIGN_LEFT_MID, 40, 0);
        lv_obj_set_style_border_width(result_img, 1, 0);
        lv_obj_set_style_border_color(result_img, lv_color_hex(0x4da6ff), 0);
        lv_obj_set_style_radius(result_img, 10, 0);
        lv_obj_set_style_pad_all(result_img, 0, 0);
        lv_img_set_src(result_img, path_small);
    }

    MLOG_DBG("BUG调试 %d\n",get_play_switch());
    if (get_play_switch()) {
        s_reread_btn = lv_btn_create(content);
        lv_obj_set_size(s_reread_btn, 120, 40);
        lv_obj_align_to(s_reread_btn, result_img, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
        lv_obj_set_style_bg_color(s_reread_btn, lv_color_hex(0x333333), 0);
        lv_obj_set_style_radius(s_reread_btn, 20, 0);
        lv_obj_set_style_shadow_width(s_reread_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(s_reread_btn, LV_OBJ_FLAG_HIDDEN);
        /* 添加重读事件回调 */
        lv_obj_add_event_cb(s_reread_btn, reread_btn_event_cb, LV_EVENT_CLICKED, NULL);

        s_reread_label = lv_label_create(s_reread_btn);
        lv_obj_center(s_reread_label);
        lv_obj_add_style(s_reread_label, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(s_reread_label, lv_color_white(), 0);
        lv_label_set_long_mode(s_reread_label, LV_LABEL_LONG_SCROLL);
    }
    /* 右侧文本容器 - 现代化信纸风格 */
    text_cont = lv_obj_create(content);
    lv_obj_set_size(text_cont, 300, LV_PCT(90));
    lv_obj_align(text_cont, LV_ALIGN_LEFT_MID, 280, 0);

    // 信纸背景样式
    lv_obj_set_style_bg_color(text_cont, lv_color_hex(0xF8F5E6), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(text_cont, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(text_cont, lv_color_hex(0xD3C9A1), LV_PART_MAIN);
    lv_obj_set_style_border_width(text_cont, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(text_cont, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(text_cont, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(text_cont, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(text_cont, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(text_cont, 2, LV_PART_MAIN); // 修正：使用正确的函数名
    lv_obj_set_style_pad_all(text_cont, 8, LV_PART_MAIN);

    /* 识别结果描述 - 现代化信纸文字 */
    result_desc_label = lv_label_create(text_cont);
    if(result_desc_label) {
        lv_label_set_text(result_desc_label, str_language_recognizing[get_curr_language()]);
        lv_obj_set_width(result_desc_label, 270);
        lv_obj_align(result_desc_label, LV_ALIGN_TOP_LEFT, 0, 0);

        // 信纸文字样式
        lv_obj_set_style_text_font(result_desc_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 22), 0);
        const lv_font_t *font1 = lv_obj_get_style_text_font(result_desc_label, LV_PART_MAIN);
        lv_coord_t line_height1 = lv_font_get_line_height(font1);
        lv_obj_set_style_text_color(result_desc_label, lv_color_hex(0x5C4B37), 0);
        lv_obj_set_style_text_align(result_desc_label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_style_text_line_space(result_desc_label, line_height1, 0); // 设置行高与线条间距一致

        // 确保文本在线条上方
        lv_obj_move_foreground(result_desc_label);
    }
    // 添加信纸线条 - 创建足够多的线条覆盖整个容器
    const lv_font_t *font = lv_obj_get_style_text_font(result_desc_label, LV_PART_MAIN);
    lv_coord_t line_height = lv_font_get_line_height(font);
    int container_height = lv_obj_get_height(text_cont);
    if(container_height == 0) container_height = 300; // 默认高度
    int num_lines = container_height / line_height + 2; // 多创建一条确保覆盖

    for(int i = 0; i < num_lines; i++) {
        lv_obj_t *line = lv_line_create(text_cont);
        /* 设置线的点：从(0,0)到(270,0) */
        static lv_point_precise_t points[] = {{0, 0}, {270, 0}};
        lv_line_set_points(line, points, 2);
        lv_obj_set_style_line_color(line, lv_color_hex(0xE8E0C7), LV_PART_MAIN);
        lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
        lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, i * line_height);
    }

    // 添加信纸角装饰
    corner_deco = lv_img_create(text_cont);
    show_image(corner_deco, "羽毛笔.png");
    lv_obj_set_size(corner_deco, 48, 48);
    lv_obj_align(corner_deco, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    lv_obj_set_style_img_opa(corner_deco, LV_OPA_50, LV_PART_MAIN);

    // 音色选择按钮（右侧）
    lv_obj_t *voice_style_btn = lv_btn_create(header);
    lv_obj_set_size(voice_style_btn, 60, 52);
    lv_obj_set_pos(voice_style_btn, H_RES - 64, 4); // 右侧位置
    lv_obj_set_style_pad_all(voice_style_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(voice_style_btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(voice_style_btn, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(voice_style_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *voice_style_label = lv_label_create(voice_style_btn);
    lv_obj_set_style_text_align(voice_style_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(voice_style_label, LV_SYMBOL_VOLUME_MAX); // 使用音量图标
    lv_obj_align(voice_style_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(voice_style_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加设置音量
    lv_obj_add_event_cb(voice_style_btn, voice_value_btn_event_cb, LV_EVENT_CLICKED, NULL);
    /* 加载结果页面 */
    lv_scr_load(result_page);

    /* 启动识别结果更新定时器 */
    safe_delete_timer(&update_result_timer);
    update_result_timer = lv_timer_create(update_recognition_result_cb, 500, NULL);
    //注册按键事件
    set_current_page_handler(aiphoto_result_key_handler);

    if (page_ai_camera_s != NULL && lv_obj_is_valid(page_ai_camera_s)) {
        MLOG_DBG("删除拍照界面\n");
        lv_obj_del(page_ai_camera_s);
        page_ai_camera_s = NULL;
    }
}

/* 延迟线条调整函数 */
static void delayed_line_adjustment(lv_timer_t *timer)
{
    if(!result_desc_label || !lv_obj_is_valid(result_desc_label)) {
        lv_timer_del(timer);
        return;
    }

    /* 获取字体和高度信息 */
    const lv_font_t *font = lv_obj_get_style_text_font(result_desc_label, LV_PART_MAIN);
    lv_coord_t height = lv_obj_get_height(result_desc_label);
    lv_coord_t line_height = lv_font_get_line_height(font);

    MLOG_DBG("Delayed adjustment - height:%d line_height:%d height/line_height:%d\n", height, line_height,
             height / line_height);

    /* 创建新的线条 */
    if(line_height > 0) {
        int line_count = (height + line_height - 1) / line_height; // 向上取整
        MLOG_DBG("Line count: %d\n", line_count);

    /* 如果高度超过300，重新设置羽毛笔图标的位置 */
    if(height > 300) {
        // 确保羽毛笔图标存在
        if(corner_deco && lv_obj_is_valid(corner_deco)) {
            // 获取文本容器的尺寸
            lv_coord_t container_width = lv_obj_get_width(text_cont);
            lv_coord_t container_height = lv_obj_get_height(text_cont);

            // 获取羽毛笔图标的尺寸
            lv_coord_t icon_width = lv_obj_get_width(corner_deco);
            lv_coord_t icon_height = lv_obj_get_height(corner_deco);

            // 计算右下角位置（考虑边距）
            lv_coord_t target_x = container_width - icon_width - 5; // 右边距5px
            lv_coord_t target_y = container_height - icon_height - 5; // 底边距5px

            // 设置羽毛笔图标的位置
            lv_obj_set_pos(corner_deco, 0, target_y+50);
            // lv_obj_align(corner_deco, LV_ALIGN_BOTTOM_RIGHT, -5, -5);

            MLOG_DBG("Feather icon positioned at (%d, %d), container size: %dx%d\n", target_x, target_y,
                     container_width, container_height);
        }
    }

        for(int i = 0; i < line_count; i++) {
            lv_obj_t *line = lv_line_create(text_cont);
            /* 设置线的点：从(0,0)到(270,0) */
            static lv_point_precise_t points[] = {{0, 0}, {270, 0}};
            lv_line_set_points(line, points, 2);
            lv_obj_set_style_line_color(line, lv_color_hex(0xE8E0C7), LV_PART_MAIN);
            lv_obj_set_style_line_width(line, 1, LV_PART_MAIN);
            lv_obj_align(line, LV_ALIGN_TOP_LEFT, 0, i * line_height);
            MLOG_DBG("Created line at y=%d\n", i * line_height);
        }
    }

    /* 删除定时器 */
    lv_timer_del(timer);

    if(get_play_switch()) {
        /* 通知ttp播放 */
        lv_timer_t *tts_timer = lv_timer_create(ttp_start_cb, 5, NULL);
        lv_timer_set_repeat_count(tts_timer, 1); // 只执行一次
    }
}

/* 识别结果更新定时器回调 */
static void update_recognition_result_cb(lv_timer_t *timer)
{
     /* 获取识别结果 */
    result_text = recognize_image(pic_thumbnail);

    if(result_text != NULL && result_desc_label && lv_obj_is_valid(result_desc_label)) {
        /* 识别成功，更新显示内容 */
        lv_label_set_text(result_desc_label, result_text);
        /* 强制布局更新 */
        lv_obj_update_layout(result_desc_label);

        /* 延迟获取高度，确保布局完成 */
        lv_timer_t *layout_timer = lv_timer_create(delayed_line_adjustment, 50, NULL);
        lv_timer_set_repeat_count(layout_timer, 1); // 只执行一次

        MLOG_DBG("Recognition success - updated result\n");
    } else {
        /* 识别失败，显示失败信息 */
        if(result_desc_label && lv_obj_is_valid(result_desc_label)) {
            lv_label_set_text(result_desc_label, str_language_recognition_failed_please_check_network[get_curr_language()]);
        }
        MLOG_DBG("Recognition failed - show failure message\n");
    }

    /* 删除定时器 */
    safe_delete_timer(&update_result_timer);
}

/* 返回按钮回调函数 */
static void back_btn_result_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        /* tts+player复位 */
        ttp_reset();
        takephoto_result_resources();
        /* 确保拍照页面存在 */
        if(!page_ai_camera_s || !lv_obj_is_valid(page_ai_camera_s)) {
            MLOG_WARN("Camera screen not valid, recreating...\n");
            create_ai_camera_screen(NULL);
        } else {
            /* 设置拍照按键处理回调函数 */
            set_current_page_handler(takephoto_key_handler);
        }
        /* 切换回拍照页面 */
        lv_scr_load(page_ai_camera_s);
    }
}

/* 返回按钮回调 */
static void back_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        /* 关闭WiFi提示窗口 */
        wifi_check_dialog_close();
        delete_viewfinder();
        delete_zoom_bar();

        /* 取消注册拍照后处理回调函数 */
        takephoto_unregister_callback();
        /* 取消拍照按键处理回调函数 */
        set_current_page_handler(NULL);

        MESSAGE_S Msg = {0};
        // 通知mode关闭时要关闭sensor
        Msg.topic     = EVENT_MODEMNG_SENSOR_STATE;
        Msg.arg1      = 1;
        MODEMNG_SendMessage(&Msg);
        memset(&Msg, 0, sizeof(MESSAGE_S));
        // 进入BOOT模式
        Msg.topic     = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1      = WORK_MODE_BOOT;
        MODEMNG_SendMessage(&Msg);
        voice_arc_delete();
        delete_zoombar_timer_handler();
        takephoto_cancel_focus();
        /* 清除播放线程 */
        ttp_deinit();
        /* 返回主页 */
        ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
    }
}

// 菜单按键处理回调函数
static void aiphoto_menu_callback(void)
{
    /* 取消注册拍照后处理回调函数 */
    takephoto_unregister_callback();
    /* 取消拍照按键处理回调函数 */
    set_current_page_handler(NULL);
    takephoto_unregister_menu_callback();

    MESSAGE_S Msg = {0};
    // 通知mode关闭时要关闭sensor
    Msg.topic     = EVENT_MODEMNG_SENSOR_STATE;
    Msg.arg1      = 1;
    MODEMNG_SendMessage(&Msg);
    memset(&Msg, 0, sizeof(MESSAGE_S));
    // 进入BOOT模式
    Msg.topic     = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1      = WORK_MODE_BOOT;
    MODEMNG_SendMessage(&Msg);
    voice_arc_delete();
    delete_viewfinder();
    delete_zoom_bar();

    delete_zoombar_timer_handler();
    takephoto_cancel_focus();
    /* 清除播放线程 */
    ttp_deinit();
    /* 返回主页 */
    ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

static void aiphoto_result_key_handler(int key_code, int key_value)
{
    switch(key_code) {
        case KEY_CAMERA:
        case KEY_MENU:
            /* tts+player复位 */
            if(!key_value) return;
            ttp_reset();
            takephoto_result_resources();
            /* 确保拍照页面存在 */
            if(!page_ai_camera_s || !lv_obj_is_valid(page_ai_camera_s)) {
                MLOG_WARN("Camera screen not valid, recreating...\n");
                create_ai_camera_screen(NULL);
            } else {
                /* 设置拍照按键处理回调函数 */
                set_current_page_handler(takephoto_key_handler);
            }
            /* 切换回拍照页面 */
            lv_scr_load(page_ai_camera_s);
            break;
        case KEY_ZOOMIN: { // in
            do_zoomin(key_value);
        }; break;
        case KEY_ZOOMOUT: { // out
            do_zoomout(key_value);
        }; break;
        case KEY_OK: {
            if (get_play_switch()) {
                lv_obj_send_event(s_reread_btn, LV_EVENT_CLICKED, NULL);
            }
        }; break;
        }
}

// 音色选择按钮事件回调
static void voice_value_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 创建音量设置
        if(get_arc_handel() == NULL) {
            hide_zoom_bar();
            voice_setting_arc_create();
        } else {
            voice_arc_delete();
        }
    }
}

static void wifi_return_to_ai_camera(void *user_data)
{
    lv_ui_t *ui = (lv_ui_t *)user_data;
    if(ui == NULL) {
        MLOG_ERR("UI is NULL, using default UI...\n");
        ui = &g_ui;
    }
    ui_load_scr_animation(ui, &page_ai_camera_s, 1, NULL, create_ai_camera_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

static void voice_style_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    switch(code) {
        case LV_EVENT_CLICKED: {
            // 创建音色选择模态框
            create_voice_style_modal();
        }; break;
        default: break;
    }
}
static uint8_t selected_index = 0;
//快捷删除按键处理回调函数
static void photo_key_down_callback(void)
{
    selected_index = (selected_index < 1) ? 1 : 0;
}

// 模式切换按键处理回调函数
static void photo_mode_callback(void)
{
    voice_style_management();
}

// UP按键处理回调函数
static void photo_up_callback(void)
{
    selected_index = (selected_index > 0) ? 0 : 1;
}

// ok按键处理回调函数
static void photo_ok_callback(void)
{
    make_sure_ok();
}

/* 重读按钮回调函数 */
static void reread_btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    ttp_status_s ttp_status = { 0 };
    if (code == LV_EVENT_CLICKED) {
        /* 检查是否有识别结果文本 */
        static uint32_t last_reread_click_time = 0;
        if (result_text != NULL && strlen(result_text) > 0) {
            // 获取当前时间
            uint32_t current_time = lv_tick_get();
            // 检查是否在点击间隔内
            if (current_time - last_reread_click_time < 1000) { //每1000ms内不允许再次点击
                MLOG_WARN("点击过于频繁，忽略此次点击\n");
                return;
            }
            // 更新上次点击时间
            last_reread_click_time = current_time;

            ttp_get_status(&ttp_status);
            /* 重新播放识别结果 */
            if (s_is_read) { // 暂停朗读
                /* TTS过程不支持断 */
                ttp_reset();
                s_is_read = false;
                lv_label_set_text(s_reread_label, str_language_resume_reading[get_curr_language()]);
            } else { // 开始朗读
                if (ttp_status.tts_status == TTP_STATUS_TTS_IDLE && ttp_status.player_status == TTP_STATUS_PLAYER_IDLE) {
                    ttp_play(result_text);
                    s_is_read = true;
                    lv_label_set_text(s_reread_label, str_language_pause_reading[get_curr_language()]);
                } else {
                    MLOG_WARN("wait ttp idle:%d %d\n", ttp_status.tts_status, ttp_status.player_status);
                }
            }
            if (read_status_timer != NULL) {
                lv_timer_reset(read_status_timer);
            }
        } else {
            MLOG_WARN("No result text available for rereading\n");
        }
    }
}

static void get_read_status_cb(lv_timer_t* timer)
{
    ttp_status_s ttp_status = { 0 };
    ttp_get_status(&ttp_status);
    if (ttp_status.player_status == TTP_STATUS_PLAYER_IDLE && ttp_status.tts_status == TTP_STATUS_TTS_IDLE) {
        s_is_read = false;
        lv_label_set_text(s_reread_label, str_language_resume_reading[get_curr_language()]);
    }
}
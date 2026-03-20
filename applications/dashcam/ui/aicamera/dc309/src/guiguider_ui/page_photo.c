#define DEBUG

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"
#include "ui_common.h"
#include <time.h>
#include "indev.h"
#include "common/takephoto.h"
#include "img2img/img2img.h"
#include "indev.h"
#include "common/extract_thumbnail.h"
#include "face_beautifier/face_beautifier.h"
#include "image_process.h"
#include "jpegp.h"
#include "cvi_comm_vo.h"
#include "cvi_vo.h"
#include "facep_service.h"
#include "linux/input.h"
#include "rtt.h"

// 控件状态结构体
typedef struct {
    lv_obj_t *obj;
    bool hidden;
} widget_state_t;

// 全局变量用于存储控件状态
static widget_state_t *g_widget_states = NULL;
static int g_widget_count = 0;
static int g_max_widgets = 0;

extern lv_style_t ttf_font_24;
static char pic_thumbnail[128] = { 0 };

#define AI_OUT_IMG_WIDTH 960
#define AI_OUT_IMG_HEIGHT 720
#define SUBPIC_WIDTH 640
#define SUBPIC_HEIGHT 480
#define THUMBNAIL_WIDTH 200
#define THUMBNAIL_HEIGHT 140

// 全局变量声明
static lv_timer_t *date_timer_s    = NULL;    // 更新定时器
static uint8_t limit_key_flag_s    = true;    // 限制自拍时间内的连续按键按下标志
static lv_obj_t *continuous_count_label_s;           // 连续拍照拍照次数
static bool continue_limit_key_flag_s = true; // 限制连续拍照时间内的连续按键按下标志
static lv_obj_t *label_delay_time_s = NULL;     //延时拍摄倒计时文本
static lv_timer_t *continue_take_photo_timer = NULL;//连续拍照定时器
// 延时拍照，用于跟踪上一次的整秒值
static int last_second = -1;

extern bool is_video_mode;
extern const char *effect_style_small[];//特效图片数组
static lv_obj_t *img_effect_s = NULL;  //特效图标
// i是否返回photo页面
static bool is_photo_back = true;

const char *batter_image_big[] = {"充电.png", "电池1.png", "电池2.png", "电池满.png"};
char *red_light_image_level[] = {"IR  1.png", "IR 2.png", "IR 3.png", "IR 4.png","IR 5.png", "IR 6.png", "IR 7.png"};
static lv_obj_t *g_top_controls[7];  // 存储顶部控件对象

static lv_timer_t *g_zoom_longpress_timer = NULL;  // 长按定时器
static int g_zoom_longpress_dir = 0;               // 长按方向: 0=无, 1=缩小, 2=放大
static bool g_zoom_longpress_active = false;       // 是否正在长按

// 资源释放函数声明
static void release_HomePhoto_resources(lv_ui_t *ui);

static void photo_zoom_event_cb(lv_event_t* e);

void photo_process_ai_beauty(void); //美颜处理
static void photoScroll_del_cb(void);//删除浮窗

static void zoomin_key_cb(void);//t按键回调
static void zoomout_key_cb(void);//w按键回调
void hide_all_widgets(lv_obj_t *parent);
void restore_all_widgets(void);
void register_all_key(void);                                     // 注册按键

static void photoEffect_Select_event_cb(lv_event_t *e);
void continue_take_photo(void);

bool get_is_photo_back(void)
{
    return is_photo_back;
}

void set_is_photo_back(bool is_back)
{
    is_photo_back = is_back;
}

// 简洁的顶部控件布局更新
static void update_top_controls_simple(void)
{
    lv_ui_t *ui = &g_ui;
    int x_pos = 6;  // 起始X坐标
    
    // 1. 相机模式按钮
    lv_obj_set_pos(ui->page_photo.img_mode, x_pos, 0);
    x_pos += 74 + 14;
    
    // 2. 分辨率按钮
    lv_obj_set_pos(g_top_controls[0], x_pos, 4);
    x_pos += 38 + 14;
    
    // 3. 红光亮级按钮
    if (brightness_level > 0) {
        lv_obj_clear_flag(ui->page_photo.redlight_level, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(ui->page_photo.redlight_level, x_pos, 4);
        x_pos += 38 + 14;
    } else {
        lv_obj_add_flag(ui->page_photo.redlight_level, LV_OBJ_FLAG_HIDDEN);
    }
    
    // 4. ISO级别按钮
    lv_obj_set_pos(g_top_controls[1], x_pos, 4);
    x_pos += 38 + 14;
    
    // 5. 屏幕亮度按钮
    lv_obj_set_pos(g_top_controls[2], x_pos, 4);
    x_pos += 38 + 14;
    
    // 6. 连拍按钮
    lv_obj_set_pos(g_top_controls[3], x_pos, 4);
    x_pos += 38 + 14;
    
    // 7. 延时拍摄按钮
    lv_obj_set_pos(g_top_controls[4], x_pos, 4);
}

const char *get_ai_process_result_img_data(bool is_aiprocess)
{
    MLOG_DBG("show pic: %s\n", pic_filepath);
    char thumbnail_path_small[256];
    char thumbnail_path_large[256];
    char *filename = strrchr(pic_filepath, '/');
    if(strstr(filename, ".jpg")) {
        get_thumbnail_path(pic_filepath, thumbnail_path_small, sizeof(thumbnail_path_small), PHOTO_SMALL_PATH);
        get_thumbnail_path(pic_filepath, thumbnail_path_large, sizeof(thumbnail_path_large), PHOTO_LARGE_PATH);

        strncpy(pic_thumbnail, thumbnail_path_large, sizeof(pic_thumbnail));
        if(is_aiprocess) {
            char *real_large = strchr(pic_thumbnail, '/');
            return real_large;
        }
    }
    return pic_thumbnail;
}

// 参数动态更新回调
int32_t g_batter_image_index = 3;

void set_batter_image_index(int32_t index)
{
    if(index < 0 || index > 3) {
        g_batter_image_index = 3; // 默认满电
        return;
    }

    switch (index) {
    case 0:
        g_batter_image_index = 3;
        break; // 电量在70%以上
    case 1:
        g_batter_image_index = 2;
        break; // 电量在25%-70%
    case 2:
        g_batter_image_index = 1;
        break; // 电量在0%-25%
    case 3:
        g_batter_image_index = 0;
        break; // 充电
    default:
        g_batter_image_index = 0;
        break;
    }

    MLOG_DBG("batter index:%d, g_batter_image_index:%d\n", index, g_batter_image_index);
}

static void photo_var_dynamic_update(lv_timer_t *timer)
{
    lv_ui_t *ui      = (lv_ui_t *)lv_timer_get_user_data(timer);
    time_t now       = time(NULL);
    struct tm *t     = localtime(&now);

    lv_obj_t *page_photo = ui->page_photo.photoscr;

    // 检查对象有效性，避免在页面切换时访问已销毁的对象
    if(page_photo == NULL && !lv_obj_is_valid(page_photo)) {
        if(date_timer_s != NULL) {
            lv_timer_del(date_timer_s);
            date_timer_s = NULL;
        }
        return;
    }

    if(ui->page_photo.label_datatime == NULL || !lv_obj_is_valid(ui->page_photo.label_datatime)) return;
    // 时间更新
    // MLOG_DBG("BUG调试 ui->page_photo.label_datatime%p %d\n",ui->page_photo.label_datatime,lv_obj_is_valid(ui->page_photo.label_datatime));
    lv_label_set_text_fmt(ui->page_photo.label_datatime, "%04d/%02d/%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1,
                          t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec);
    show_image(ui->page_photo.img_batter, batter_image_big[g_batter_image_index]);
    lv_label_set_text_fmt(ui->page_photo.label_numphoto, "%02d", photo_CalculateRemainingPhotoCount());

    if(ui_common_cardstatus()) {
        show_image(ui->page_photo.img_sdonline, "icon_card_online.png");
    } else {
        show_image(ui->page_photo.img_sdonline, "icon_card_offline.png");
    }
}

// 拍照完成
void photo_resume_anim_complete(lv_anim_t *a)
{
    if(AIModeSelect_GetMode() == AI_NONE) {
        restore_all_widgets();
        lv_timer_resume(date_timer_s);
    }
    CVI_VO_ResumeChn(0,0);
    lv_anim_del(a, a->exec_cb);
}

//延时拍计数动画
void photo_delay_anim(void *var, int32_t v)
{
    // 计算当前剩余的整秒数（向下取整）
    int current_second = get_self_delay_time() - (int)v;

    // 只有当整秒数变化时才更新标签
    if(current_second != last_second) {
        // 确保标签可见
        if(lv_obj_has_flag(label_delay_time_s, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(label_delay_time_s, LV_OBJ_FLAG_HIDDEN);
        }
        // 更新标签文本
        lv_label_set_text_fmt(label_delay_time_s, "%02d", current_second);
        // 更新记录的最后整秒值
        last_second = current_second;

        if (current_second != get_self_delay_time()) {
            // 触发倒计时声音
            EVENT_S stEvent = { 0 };
            stEvent.topic = EVENT_UI_TOUCH;
            EVENTHUB_Publish(&stEvent);
        }
        // 调试信息
        MLOG_DBG("倒计时更新: %d\n", current_second);
    }
}
//延时拍计数动画完成
void photo_delay_anim_complete(lv_anim_t *a)
{
    limit_key_flag_s = true;
    if(!lv_obj_has_flag(label_delay_time_s, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(label_delay_time_s, LV_OBJ_FLAG_HIDDEN);
    }
    if(get_shootmode(1) == 0) {
        hide_all_widgets(g_ui.page_photo.photoscr);
        lv_timer_pause(date_timer_s);//暂停动画更新
        CVI_VO_PauseChn(0, 0);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_values(&anim, 0, 1);
        lv_anim_set_time(&anim, 1000);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
        lv_anim_set_completed_cb(&anim, photo_resume_anim_complete);
        lv_anim_start(&anim);

        MESSAGE_S Msg = {0};
        Msg.topic     = EVENT_MODEMNG_START_PIV;
        MODEMNG_SendMessage(&Msg);
        ui_common_wait_piv_end();
        enable_touch_events();//倒计时结束，没有连拍，恢复触摸
        enable_hardware_input_device(0);
        enable_hardware_input_device(1);
    } else {//倒计时结束，开始连拍
        if(continue_limit_key_flag_s) {
            hide_all_widgets(g_ui.page_photo.photoscr);
            lv_timer_pause(date_timer_s);
            if(lv_obj_has_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN);
            }
            // 开始连拍
            continue_take_photo();
            continue_limit_key_flag_s = false;
        }
    }
    lv_anim_del(a, a->exec_cb);
}
//拍照之前回调
static void key_takephoto_before_exec(void* user_data)
{
    //如果发生连续点击，先恢复原有的状态再隐藏//防止将原有的显示状态覆盖掉
    if(!lv_obj_has_flag(g_ui.page_photo.img_sdonline, LV_OBJ_FLAG_HIDDEN) && (AIModeSelect_GetMode() == AI_NONE)) {
        hide_all_widgets(g_ui.page_photo.photoscr);
    } else if(AIModeSelect_GetMode() == AI_NONE) {
        restore_all_widgets();
        lv_timer_resume(date_timer_s);
        hide_all_widgets(g_ui.page_photo.photoscr);
        lv_timer_pause(date_timer_s);
    }
    CVI_VO_PauseChn(0, 0);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_values(&anim, 0, 1);
    lv_anim_set_time(&anim, 1000);
    lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
    lv_anim_set_completed_cb(&anim, photo_resume_anim_complete);
    lv_anim_start(&anim);
}

//拍照之前回调
static void key_takephoto_before_callback(void)
{
    lv_async_call(key_takephoto_before_exec, NULL);
}
//按键执行回调
static void key_takephoto_callback_exec(void* user_data)
{
    set_defalt_retval();
    //如果有倒计时，且ai为普通模式，则进入延时拍
    if(get_self_delay_time() != 0 && AIModeSelect_GetMode() == AI_NONE) {
        if(limit_key_flag_s) {
            limit_key_flag_s = false;
            // 重置整秒记录
            last_second = -1;
            // 立即显示初始值
            lv_obj_clear_flag(label_delay_time_s, LV_OBJ_FLAG_HIDDEN);
            lv_label_set_text_fmt(label_delay_time_s, "%02d", get_self_delay_time());
            // 调试信息
            MLOG_DBG("次数：get_self_time():%d 总时长:%d\n", get_self_delay_time(), get_self_delay_time() * 1000);
            cancel_viewfinder();
            disable_touch_events();//延时倒计时，禁用触摸
            disable_hardware_input_device(0);
            disable_hardware_input_device(1);
            // 创建动画
            lv_anim_t anim;
            lv_anim_init(&anim);
            lv_anim_set_values(&anim, 0, get_self_delay_time());
            lv_anim_set_time(&anim, get_self_delay_time() * 1000);
            lv_anim_set_exec_cb(&anim, photo_delay_anim);
            lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
            lv_anim_set_completed_cb(&anim, photo_delay_anim_complete);
            lv_anim_start(&anim);
        }
    } else {
        // 如果有连拍和ai为普通模式，则进入连拍
        if(get_shootmode(1) != 0 && AIModeSelect_GetMode() == AI_NONE) {
            if(continue_limit_key_flag_s) {
                hide_all_widgets(g_ui.page_photo.photoscr);
                lv_timer_pause(date_timer_s);
                if(lv_obj_has_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_clear_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN);
                }
                disable_touch_events();//连拍中，禁用触摸
                disable_hardware_input_device(0);
                disable_hardware_input_device(1);
                // 创建连续拍照动画
                continue_take_photo();
                continue_limit_key_flag_s = false;
            }
        }
    }
}

//按键执行回调
static void key_takephoto_callback(void)
{
    lv_async_call(key_takephoto_callback_exec, NULL);
}

// 删除浮窗
static void photoScroll_del_cb(void)
{
    delete_aiselect_scroll();
}

// ok按键处理回调函数
static void photo_sesor_switch_completed_callback(void)
{
    enable_touch_events(); // 连拍结束，恢复触摸
    enable_hardware_input_device(0);
    enable_hardware_input_device(1);
}

// 所有事件回调处理
static void buttonPhoto_All_event_handler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int Click_index      = (int)lv_event_get_user_data(e);
    MLOG_DBG("code:%d, Click_index:%d\n", code, Click_index);

    if(code == LV_EVENT_CLICKED) {
        lv_ui_t *ui   = &g_ui;
        MESSAGE_S Msg = {0};

        switch(Click_index) {
            case 0: // 跳转系统菜单
                release_HomePhoto_resources(ui);
                ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
                                      &g_ui.screenHomePhoto_del, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                      true);
                break;

            case 1: // 切换到视频模式
                release_HomePhoto_resources(ui);
                homeMode_Set(VEDIO_MODE);
                // 进入录像模式
                Msg.topic = EVENT_MODEMNG_MODESWITCH;
                Msg.arg1  = WORK_MODE_MOVIE;
                MODEMNG_SendMessage(&Msg);
                // 复位缩放
                set_zoom_level(1);
                // 关闭对焦
                disable_focus();
                is_video_mode = true;
                ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, &g_ui.screenHomePhoto_del, Home_Vedio,
                                      LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
                break;


            case 9: // 返回主页
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
                release_HomePhoto_resources(ui);
                ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                      true);
                break;
            case 3: //相册
                release_HomePhoto_resources(ui);
                // 复位缩放
                set_zoom_level(1);
                // 关闭对焦
                disable_focus();
                // 进入录像模式
                MESSAGE_S Msg = { 0 };
                Msg.topic = EVENT_MODEMNG_MODESWITCH;
                Msg.arg1 = WORK_MODE_PLAYBACK;
                MODEMNG_SendMessage(&Msg);
                ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
                break;
            default: break;
        }
    }
}

// 初始化事件
static void events_init_HomePhoto(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_photo.img_mode, buttonPhoto_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)1);
    lv_obj_add_event_cb(ui->page_photo.img_exit, buttonPhoto_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)9);
    lv_obj_add_event_cb(ui->page_photo.img_album, buttonPhoto_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)3);
    lv_obj_add_event_cb(ui->page_photo.img_menu, buttonPhoto_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)0);
}

// 资源释放函数
static void release_HomePhoto_resources(lv_ui_t *ui)
{
    // 删除AI选择滚动容器
    delete_aiselect_scroll(); // ai选择弹窗
    delete_all_handle(); // 特效选择弹窗
    delete_viewfinder(); // 取景框
    delete_zoom_bar(); // 放大缩小
    wifi_check_dialog_close(); // wifi是否开启检查对话框销毁
    delete_batter_tips_mbox(); // 低电量不允许开wifi弹窗销毁
    destroy_voice_input_popup(); // ai语音自定义弹窗销毁

    if(date_timer_s != NULL) {
        lv_timer_del(date_timer_s);
        date_timer_s = NULL;
    }

    if(continue_take_photo_timer != NULL) {
        lv_timer_del(continue_take_photo_timer);
        continue_take_photo_timer = NULL;
    }

    // 释放缩放相关资源
    delete_zoombar_timer_handler();
    // 删除当前页面按键处理回调
    takephoto_unregister_all_callback();
    FACEP_SERVICE_Unregister_Smile_Pre_Callback();
    FACEP_SERVICE_Unregister_Smile_Post_Callback();
    set_current_page_handler(NULL);
}

// 菜单按键处理回调函数
static void photo_menu_callback(void)
{
    MLOG_DBG("进入拍照模式设置页面\n");
    release_HomePhoto_resources(&g_ui);
    ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
        &g_ui.screenHomePhoto_del, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
        true);
}

// UP按键处理回调函数
static void photo_redlight_callback(void)
{
    lv_ui_t *ui = &g_ui;
    // 更新红光亮级图片
    if (brightness_level > 6) {
        show_image(ui->page_photo.redlight_level, red_light_image_level[6]);
    } else if (brightness_level > 0) {
        show_image(ui->page_photo.redlight_level, red_light_image_level[brightness_level-1]);
    }
    // 更新布局
    update_top_controls_simple();
}

// 模式切换按键处理回调函数
static void photo_mode_callback(void)
{
    MLOG_DBG("AI按键,进入AI模式切换页面\n");
    MLOG_DBG("模式切换，进入视频模式\n");
    MESSAGE_S Msg = {0};
    release_HomePhoto_resources(&g_ui);
    homeMode_Set(VEDIO_MODE);
    // 进入录像模式
    Msg.topic = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1  = WORK_MODE_MOVIE;
    MODEMNG_SendMessage(&Msg);
    // 复位缩放
    set_zoom_level(1);
    // 关闭对焦
    disable_focus();
    is_video_mode = true;
    ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, &g_ui.screenHomePhoto_del, Home_Vedio, LV_SCR_LOAD_ANIM_NONE,
                            0, 0, false, true);
}

// AI按键处理回调函数
static void photo_play_callback(void)
{

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
                        Msg.topic = EVENT_MODEMNG_SENSOR_STATE;
                        Msg.arg1  = 1;
                        MODEMNG_SendMessage(&Msg);
                        memset(&Msg, 0, sizeof(MESSAGE_S));
                        // 进入BOOT模式
                        Msg.topic = EVENT_MODEMNG_MODESWITCH;
                        Msg.arg1  = WORK_MODE_BOOT;
                        MODEMNG_SendMessage(&Msg);
                        // 复位缩放
                        set_zoom_level(1);
                        release_HomePhoto_resources(&g_ui);
                        ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                              false, true);

                        if(date_timer_s != NULL) {
                            lv_timer_del(date_timer_s);
                            date_timer_s = NULL;
                        }
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

// 创建主页面
void Home_Photo(lv_ui_t *ui)
{
    MLOG_DBG("loading page_home1...\n");
    HomePhoto_t *Home_Photo_Item = &ui->page_photo;
    is_photo_back = true;
    extern uint8_t g_last_scr_mode;
    g_last_scr_mode = 1;
    set_exit_completed(false);
    
    // 创建新页面
    Home_Photo_Item->photoscr = lv_obj_create(NULL);
    lv_obj_set_size(Home_Photo_Item->photoscr, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(Home_Photo_Item->photoscr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(Home_Photo_Item->photoscr, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(Home_Photo_Item->photoscr, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(Home_Photo_Item->photoscr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(Home_Photo_Item->photoscr, gesture_event_handler, LV_EVENT_GESTURE, ui);
    
    // 相机模式按钮
    Home_Photo_Item->img_mode = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(Home_Photo_Item->img_mode, 74, 47);
    show_image(Home_Photo_Item->img_mode, "paizhaompshi.png");
    
    // 分辨率按钮
    g_top_controls[0] = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(g_top_controls[0], 38, 32);
    show_image(g_top_controls[0], photo_getRes_Icon());
    
    // 红光亮级按钮
    Home_Photo_Item->redlight_level = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(Home_Photo_Item->redlight_level, 38, 32);
    
    // ISO级别按钮
    g_top_controls[1] = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(g_top_controls[1], 38, 32);
    char* iso_buf[] = {
        "ISO.png", "ISO 100.png", "ISO 200.png", "ISO 400.png",
        "ISO 800.png", "ISO 1600.png", "ISO 3200.png", "ISO 6400.png",
    };
    show_image(g_top_controls[1], iso_buf[get_iso_index()]);
    
    // 屏幕亮度按钮
    g_top_controls[2] = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(g_top_controls[2], 38, 32);
    char* brightness_buf[] = { "1.png", "2.png", "3.png", "4.png", "5.png", "6.png", "7.png" };
    show_image(g_top_controls[2], brightness_buf[get_curr_brightness()]);
    
    // 连拍按钮
    g_top_controls[3] = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(g_top_controls[3], 38, 33);
    char* continue_buf[] = { "连拍关闭.png", "连拍3.png", "连拍5.png", "连拍7.png" };
    show_image(g_top_controls[3], continue_buf[get_shootmode(0)]);
    
    // 延时拍摄按钮
    g_top_controls[4] = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_set_size(g_top_controls[4], 38, 33);
    char* delay_buf[] = { "延时 关闭.png", "延时5.png", "延时7.png", "延时10.png" };
    show_image(g_top_controls[4], delay_buf[get_self_index()]);
    
    // 初始设置红光亮级图片
    if (brightness_level > 6) {
        show_image(Home_Photo_Item->redlight_level, red_light_image_level[6]);
    } else if (brightness_level > 0) {
        show_image(Home_Photo_Item->redlight_level, red_light_image_level[brightness_level-1]);
    }
    
    // 初始布局更新
    update_top_controls_simple();

    // 剩余拍照数量
    Home_Photo_Item->label_numphoto = lv_label_create(Home_Photo_Item->photoscr);
    lv_label_set_text_fmt(Home_Photo_Item->label_numphoto, "%02d", photo_CalculateRemainingPhotoCount());
    lv_label_set_long_mode(Home_Photo_Item->label_numphoto, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(Home_Photo_Item->label_numphoto, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(Home_Photo_Item->label_numphoto, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(Home_Photo_Item->label_numphoto, LV_ALIGN_TOP_RIGHT, -104, 6);

    // SD卡状态
    Home_Photo_Item->img_sdonline = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(Home_Photo_Item->img_sdonline, LV_ALIGN_TOP_RIGHT, -58, 0);
    lv_obj_set_size(Home_Photo_Item->img_sdonline, 38, 32);
    if(ui_common_cardstatus()) {
        show_image(Home_Photo_Item->img_sdonline, "icon_card_online.png");
    } else {
        show_image(Home_Photo_Item->img_sdonline, "icon_card_offline.png");
    }

    // 电池状态
    Home_Photo_Item->img_batter = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(Home_Photo_Item->img_batter, LV_ALIGN_TOP_RIGHT, -8, 2);
    lv_obj_set_size(Home_Photo_Item->img_batter, 38, 32);
    show_image(Home_Photo_Item->img_batter,"充电.png");

    //缩放
    lv_obj_t *imgbtn_zoomout = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(imgbtn_zoomout, LV_ALIGN_LEFT_MID, 12, -42);
    lv_obj_set_size(imgbtn_zoomout, 38, 38);
    show_image(imgbtn_zoomout, "T.png");
    lv_obj_add_event_cb(imgbtn_zoomout, photo_zoom_event_cb, LV_EVENT_ALL, (void *)(intptr_t)2);

    lv_obj_t *imgbtn_zoomin = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(imgbtn_zoomin, LV_ALIGN_LEFT_MID, 12, 42);
    lv_obj_set_size(imgbtn_zoomin, 38, 38);
    show_image(imgbtn_zoomin, "W.png");
    lv_obj_add_event_cb(imgbtn_zoomin, photo_zoom_event_cb, LV_EVENT_ALL, (void *)(intptr_t)1);


    // 菜单按钮
    Home_Photo_Item->img_menu = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(Home_Photo_Item->img_menu, LV_ALIGN_BOTTOM_LEFT, 6, 0);
    lv_obj_set_size(Home_Photo_Item->img_menu, 40, 40);
    show_image(Home_Photo_Item->img_menu, "menu.png");

    // 滤镜
    lv_obj_t *btn_effect = lv_button_create(Home_Photo_Item->photoscr);
    lv_obj_align(btn_effect, LV_ALIGN_BOTTOM_LEFT, 60,0);
    lv_obj_set_size(btn_effect, 40, 40);
    lv_obj_set_style_bg_opa(btn_effect, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
    lv_obj_set_style_pad_all(btn_effect, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_effect, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_effect, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(btn_effect, photoEffect_Select_event_cb, LV_EVENT_CLICKED, NULL);

    img_effect_s = lv_img_create(btn_effect);
    lv_obj_set_size(img_effect_s, 40, 40);
    lv_obj_align(img_effect_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(img_effect_s, 0, LV_STATE_DEFAULT);
    show_image(img_effect_s, "颜色特效.png");

    time_t now       = time(NULL);
    struct tm *t     = localtime(&now);

    // 时间显示
    Home_Photo_Item->label_datatime = lv_label_create(Home_Photo_Item->photoscr);
    lv_obj_set_pos(Home_Photo_Item->label_datatime, 220, 420);
    lv_label_set_text_fmt(Home_Photo_Item->label_datatime, "%04d/%02d/%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1,
                          t->tm_mday,t->tm_hour, t->tm_min, t->tm_sec);
    lv_label_set_long_mode(Home_Photo_Item->label_datatime, LV_LABEL_LONG_WRAP);
    lv_obj_add_style(Home_Photo_Item->label_datatime, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(Home_Photo_Item->label_datatime, lv_color_hex(0xFFFFFF),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(Home_Photo_Item->label_datatime, LV_ALIGN_BOTTOM_MID, 0, -12);
    if(getSelect_Index() == TIME_FLAG_OFF) {
        lv_obj_add_flag(Home_Photo_Item->label_datatime, LV_OBJ_FLAG_HIDDEN);
    }
    lv_obj_update_layout(Home_Photo_Item->photoscr);


    //相册
    Home_Photo_Item->img_album = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(Home_Photo_Item->img_album, LV_ALIGN_BOTTOM_RIGHT, -80, 0);
    lv_obj_set_size(Home_Photo_Item->img_album, 40, 40);
    show_image(Home_Photo_Item->img_album, "photo_album.png");

    // 退出按钮
    Home_Photo_Item->img_exit = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(Home_Photo_Item->img_exit, LV_ALIGN_BOTTOM_RIGHT, -6, -6);
    lv_obj_set_size(Home_Photo_Item->img_exit, 41, 40);
    show_image(Home_Photo_Item->img_exit, "exit.png");

    // 时间
    label_delay_time_s = lv_label_create(Home_Photo_Item->photoscr);
    lv_label_set_text(label_delay_time_s, "10");
    lv_obj_set_style_text_font(label_delay_time_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 80),
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_delay_time_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label_delay_time_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(label_delay_time_s, LV_OBJ_FLAG_HIDDEN);

    //连续拍照次数文本
    continuous_count_label_s = lv_label_create(ui->page_photo.photoscr);
    lv_obj_add_style(continuous_count_label_s, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(continuous_count_label_s, lv_color_hex(0x00FF00),
                            LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(continuous_count_label_s, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN);

    // 创建缩放等级显示
    create_zoom_bar(ui->page_photo.photoscr);

    /* 设置当前页面按键处理回调 */
    register_all_key();

    // 创建更新定时器
    if(date_timer_s == NULL) {
        date_timer_s = lv_timer_create(photo_var_dynamic_update, 1000, ui);
        lv_timer_ready(date_timer_s);
    }
    events_init_HomePhoto(ui);
}


// 放大按键事件处理
static void zoomin_key_cb(void)
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

// 缩小按键事件处理
static void zoomout_key_cb(void)
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

// 隐藏所有控件并保存状态
void hide_all_widgets(lv_obj_t *parent)
{
    // 首先计算需要存储的控件数量
    int child_count = lv_obj_get_child_count(parent);
    // 分配内存存储控件状态
    if (g_widget_states == NULL || g_max_widgets < child_count) {
        g_max_widgets = child_count + 10; // 额外分配一些空间
        g_widget_states = realloc(g_widget_states, g_max_widgets * sizeof(widget_state_t));
        if (g_widget_states == NULL) {
            MLOG_ERR("Failed to allocate memory for widget states\n");
            return;
        }
    }

    g_widget_count = 0;

    // 遍历所有子控件，保存状态并隐藏
    lv_obj_t *child = lv_obj_get_child(parent, 0);
    while (child != NULL) {
        // 保存控件状态
        g_widget_states[g_widget_count].obj = child;
        g_widget_states[g_widget_count].hidden = lv_obj_has_flag(child, LV_OBJ_FLAG_HIDDEN);

        // 隐藏控件（除非是取景框）
        lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);

        g_widget_count++;
        child = lv_obj_get_child(parent, g_widget_count);
    }

    MLOG_DBG("Hidden %d widgets\n", g_widget_count);
}

// 恢复所有控件的显示状态
void restore_all_widgets(void)
{
    if (g_widget_states == NULL) {
        MLOG_ERR("No widget states to restore\n");
        return;
    }

    for (int i = 0; i < g_widget_count; i++) {
        if (lv_obj_is_valid(g_widget_states[i].obj)) {
            if (g_widget_states[i].hidden) {
                lv_obj_add_flag(g_widget_states[i].obj, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_clear_flag(g_widget_states[i].obj, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    MLOG_DBG("Restored %d widgets\n", g_widget_count);

    // 清理状态存储
    free(g_widget_states);
    g_widget_states = NULL;
    g_widget_count = 0;
    g_max_widgets = 0;
}

static void wifi_return_to_home_photo(void *user_data)
{
    lv_ui_t *ui = (lv_ui_t *)user_data;
    if(ui == NULL) {
        MLOG_ERR("UI is NULL, using default UI...\n");
        ui = &g_ui;
    }
    ui_load_scr_animation(ui, &ui->page_photo.photoscr, ui->screenHomePhoto_del, NULL, Home_Photo,
                          LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

static void key_takephoto_power_callback(void)
{
    static bool power_key_count = false;
    power_key_count = !power_key_count;
    switch (power_key_count) {
    case false:
        restore_all_widgets();
        break;
    case true:
        hide_all_widgets(g_ui.page_photo.photoscr);
        break;
    }
}

void register_all_key(void)
{
    if((get_self_delay_time() != 0 && AIModeSelect_GetMode() == AI_NONE) ||
       ((get_shootmode(1) != 0 && AIModeSelect_GetMode() == AI_NONE))) {
        set_current_page_handler(takephoto_delay_handler);
    } else {
        set_current_page_handler(takephoto_key_handler);
    }
    takephoto_register_callback(key_takephoto_callback);
    takephoto_register_menu_callback(photo_menu_callback);
    takephoto_register_mode_callback(photo_mode_callback);
    takephoto_register_play_callback(photo_play_callback);
    takephoto_register_up_callback(photo_redlight_callback);
    takephoto_register_down_callback(photo_redlight_callback);
    takephoto_register_zoomin_callback(zoomin_key_cb);
    takephoto_register_zoomout_callback(zoomout_key_cb);
    takephoto_register_before_callback(key_takephoto_before_callback);
    takephoto_power_callback(key_takephoto_power_callback);
}


// 渐隐动画完成回调
void photoanimCompleted_objDel_cb(lv_anim_t *a)
{
    // 移除标志
    lv_timer_resume(date_timer_s);
    delete_all_handle();
}

// 特效选择回调
static void photoEffect_Select_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            if(get_is_effect_exist() == true) {
                delete_all_handle();
                // 移除标志
                lv_timer_resume(date_timer_s);
            } else {
                lv_timer_pause(date_timer_s);
                // 创建滚动列表
                float_effect_creat(img_effect_s, g_ui.page_photo.photoscr);
                // 创建控件并启动渐渐隐藏动画
                create_gradually_hide_anim(photoanimCompleted_objDel_cb,8000);
            }
            // Update current screen layout.
            lv_obj_update_layout(g_ui.page_photo.photoscr);
        } break;
        default: break;
    }
}

static void continue_take_photo_timer_cb(lv_timer_t* timer)
{
    static uint8_t take_photo_num = 0;
    if (get_shootmode(1) - take_photo_num != 0) // 没有拍照完成
    {
        CVI_VO_PauseChn(0, 0);
        MESSAGE_S Msg = { 0 };
        Msg.topic = EVENT_MODEMNG_START_PIV;
        MODEMNG_SendMessage(&Msg);
        ui_common_wait_piv_end();
        CVI_VO_ResumeChn(0, 0);
        MLOG_DBG("第%d次拍照\n", take_photo_num+1);
        lv_label_set_text_fmt(continuous_count_label_s, "%d/%d",take_photo_num + 1, get_shootmode(1));
        take_photo_num++;
    } else {
        restore_all_widgets();
        lv_timer_resume(date_timer_s);

        // 隐藏连续拍照次数
        if (!lv_obj_has_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(continuous_count_label_s, LV_OBJ_FLAG_HIDDEN);
        }
        enable_touch_events(); // 连拍结束，恢复触摸
        enable_hardware_input_device(0);
        enable_hardware_input_device(1);
        continue_limit_key_flag_s = true;
        take_photo_num = 0;
        lv_timer_del(continue_take_photo_timer);
        continue_take_photo_timer = NULL;
    }
}

void continue_take_photo(void)
{
    if(continue_take_photo_timer == NULL) {
        continue_take_photo_timer = lv_timer_create(continue_take_photo_timer_cb, 100, NULL);
        lv_timer_ready(continue_take_photo_timer);
    }
}


// 长按定时器回调函数
static void zoom_longpress_timer_cb(lv_timer_t *timer)
{

    if (g_ui.page_photo.photoscr == NULL || !lv_obj_is_valid(g_ui.page_photo.photoscr)) {
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
    
    if (g_ui.page_photo.photoscr == NULL || !lv_obj_is_valid(g_ui.page_photo.photoscr)) {
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
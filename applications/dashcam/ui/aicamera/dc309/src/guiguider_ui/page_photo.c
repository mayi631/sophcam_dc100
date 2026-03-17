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
extern bool ai_custom_is_confire;

//AI处理相关变量
static lv_obj_t *ai_process_cont_s = NULL; // AI处理容器
static lv_timer_t *get_aiprocess_result_timer = NULL;  ;//获取结果定时器
static lv_timer_t *rtt_get_text_timer = NULL;  ;//获取结果定时器
static pthread_t aiprocess_thread = 0;                 //AI处理任务线程
static bool is_processing = false; // 限制重复点击AI处理
//是否是AI处理页面进入
static bool is_aiprocess_entry_aiset_back = false;

// static const char *photo_EV_s[] = {
//     "ev3.png", "EV2.png", "EV1.png", "EV00.png", "EV11.png", "EV22.png", "EV33.png",
// };

const char *batter_image_big[] = {"充电.png", "电池1.png", "电池2.png", "电池满.png"};
char *red_light_image_level[] = {"IR  1.png", "IR 2.png", "IR 3.png", "IR 4.png","IR 5.png", "IR 6.png", "IR 7.png"};
void ai_takephoto_process_res_create(lv_obj_t *parent);
// 资源释放函数声明
static void release_HomePhoto_resources(lv_ui_t *ui);
static void aiprocessing_ui_update(lv_timer_t *timer);

static void photo_zoom_event_cb(lv_event_t* e);

void photo_process_ai_beauty(void); //美颜处理
static void ai_select_event_cb(lv_event_t *e);//选择事件
static void photoScroll_del_cb(void);//删除浮窗

static void zoomin_key_cb(void);//t按键回调
static void zoomout_key_cb(void);//w按键回调
void hide_all_widgets(lv_obj_t *parent);
void restore_all_widgets(void);
void register_all_key(void);                                     // 注册按键
static void aiprocess_key_callback(int key_code, int key_value); // 结果界面key回调
void start_aiprocess(const int index);                           // 开始处理

static void ai_result_delete_after_cb(void);

static void delete_ai_result_cb(lv_event_t *e);
static void wifi_return_to_home_photo(void *user_data);

static void photoEffect_Select_event_cb(lv_event_t *e);
static void rtt_get_text_timer_cb(lv_timer_t *timer);
void continue_take_photo(void);

bool get_is_photo_back(void)
{
    return is_photo_back;
}

void set_is_photo_back(bool is_back)
{
    is_photo_back = is_back;
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


void aiprocess_result_get_img(void)
{
    if(ai_process_cont_s == NULL || !lv_obj_is_valid(ai_process_cont_s)) {
        MLOG_ERR("ai_process_cont_s is invalid");
        return;
    }

    // 获取容器中的图片和标签对象
    lv_obj_t *img     = lv_obj_get_child(ai_process_cont_s, 0);
    lv_obj_t *label   = lv_obj_get_child(ai_process_cont_s, 1);
    lv_obj_t *spinner = lv_obj_get_child(ai_process_cont_s, 2);
    if(img == NULL || label == NULL) {
        MLOG_ERR("Failed to get child objects");
        return;
    }

    // 获取AI处理结果图片数据
    const char *result_img = get_ai_process_result_img_data(0);
    MLOG_DBG("result_img pic: %s\n", result_img);

    if(result_img != NULL) {
        lv_image_set_src(img, result_img);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    }
}

// 动画完成回调
void aiprocess_result_get_img_complete(lv_anim_t *a)
{
    // 使用正确的API显示容器
    if(!lv_obj_has_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN);
        register_all_key();
    }
    // 删除
    lv_anim_del(a, a->exec_cb);
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

static void ai_select_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn        = lv_obj_get_child(get_aiselete_scroll_handl(), 0);
    lv_obj_t *label      = lv_obj_get_child(btn, 1);
    switch(code) {
        case LV_EVENT_PRESSED: // 获取语音自定义的输入语言
        {
            if(AIModeSelect_GetMode() == AI_VOICE_CUSTOM) {
                lv_label_set_text(label, str_language_listening[get_curr_language()]);
                // 创建语音输入弹框
                create_voice_input_popup();
                // 开始录音
                rtt_start();
                // 启动定时器获取文本
                if(rtt_get_text_timer == NULL) {
                    rtt_get_text_timer = lv_timer_create(rtt_get_text_timer_cb, 300, NULL);
                    lv_timer_ready(rtt_get_text_timer);
                }
                return;
            }
        }; break;
        case LV_EVENT_CLICKED:
        {
            if(AIModeSelect_GetMode() == AI_VOICE_CUSTOM) {//语言输入中
               return;
            }
            lv_obj_t *btn_clicked = lv_event_get_target(e);
            lv_obj_t *parent      = lv_obj_get_parent(btn_clicked); //获取发生点击事件的父控件，列表

            for(uint8_t i = 0; i < lv_obj_get_child_cnt(parent); i++) {
                if(btn_clicked == lv_obj_get_child(parent, i)) {
                    set_currIndex_focus(i);
                    start_aiprocess(i);
                }
            }
        }; break;
        case LV_EVENT_RELEASED:
        case LV_EVENT_CANCEL://播放
        {
            if(AIModeSelect_GetMode() == AI_VOICE_CUSTOM) {
                // 停止录音
                rtt_stop();
                lv_label_set_text(label, str_language_hold_to_speak[get_curr_language()]);
                // destroy_voice_input_popup();
                voice_display_button();
            }
        }; break;
        default: break;
    }
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
                is_aiprocess_entry_aiset_back = false;
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
                is_aiprocess_entry_aiset_back = false;
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
                is_aiprocess_entry_aiset_back = false;
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
    if(ai_process_cont_s != NULL && lv_obj_is_valid(ai_process_cont_s)) {
        lv_obj_del(ai_process_cont_s);
        ai_process_cont_s = NULL;
    }

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
    // 删除获取结果定时器
    if(get_aiprocess_result_timer != NULL) {
        lv_timer_del(get_aiprocess_result_timer);
        get_aiprocess_result_timer = NULL;
    }
    // 销毁AI处理线程
    if(aiprocess_thread) {
        pthread_cancel(aiprocess_thread);
        pthread_join(aiprocess_thread, NULL);
        aiprocess_thread = 0;
    }

    // 删除当前页面按键处理回调
    takephoto_unregister_all_callback();
    FACEP_SERVICE_Unregister_Smile_Pre_Callback();
    FACEP_SERVICE_Unregister_Smile_Post_Callback();
    set_current_page_handler(NULL);
}

// 菜单按键处理回调函数
static void photo_menu_callback(void)
{
    //m没有在ai处理中
    if(!is_processing) {
        if(!lv_obj_has_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN);
            register_all_key();
        } else {

            if (wifi_check_dialog_close() == 1)
                return;
            MLOG_DBG("进入拍照模式设置页面\n");
            release_HomePhoto_resources(&g_ui);
            is_aiprocess_entry_aiset_back = false;
            ui_load_scr_animation(&g_ui, &g_ui.page_photoMenu_Setting.menuscr, g_ui.screenPhotoMenuSetting_del,
                                  &g_ui.screenHomePhoto_del, photoMenu_Setting, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
                                  true);
        }
    } else {
        lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
        lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
    }
}

// UP按键处理回调函数
static void photo_redlight_callback(void)
{
    if (brightness_level > 6) {
        show_image(g_ui.page_photo.redlight_level, red_light_image_level[6]);
    } else {
        show_image(g_ui.page_photo.redlight_level, red_light_image_level[brightness_level]);
    }
}

// left按键处理回调函数
static void photo_left_callback(void)
{

}

// right按键处理回调函数
static void photo_right_callback(void)
{
}

//快捷删除按键处理回调函数
static void photo_key_down_callback(void)
{
    //没有在ai处理中
    if(!is_processing) {
        if(!lv_obj_has_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_t *delete_btn = lv_obj_get_child(ai_process_cont_s, 5);
            lv_obj_send_event(delete_btn, LV_EVENT_CLICKED, NULL);
        } else {
            create_simple_delete_dialog(NULL); // 创建确认浮窗
        }
    } else {
        lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
        lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
    }
}

// 模式切换按键处理回调函数
static void photo_mode_callback(void)
{
    if(!is_processing) {
        if(!lv_obj_has_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN)) {
            is_aiprocess_entry_aiset_back = true;
            release_HomePhoto_resources(&g_ui);
            ui_load_scr_animation(&g_ui, &obj_Photo_AiMode_s, 1, NULL, photoMenu_AIMode, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                  false, true);
        } else {
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
            is_aiprocess_entry_aiset_back = false;
        }
    } else {
        lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
        lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
    }
}

// ok按键处理回调函数
static void photo_ok_callback(void)
{
    if(!is_processing) {
        if(get_is_effect_exist() == false) {
            MLOG_DBG("ok按键, 切换前后摄像头\n");
            MESSAGE_S Msg = {0};
            Msg.topic     = EVENT_MODEMNG_SETTING;
            Msg.arg1      = PARAM_MENU_SENSOR_SWITCH;
            MODEMNG_SendMessage(&Msg);

            disable_touch_events();//延时倒计时，禁用触摸
            disable_hardware_input_device(0);
            disable_hardware_input_device(1);

            completed_register_cb(photo_sesor_switch_completed_callback);
            // 复位缩放
            set_zoom_level(1);
        } else {
            set_effect_ok();
        }
    }
}

// 长按菜单按键处理回调函数
static void photo_long_menu_callback(void)
{
    if(!is_processing) {
        MLOG_DBG("长按菜单按键，返回主页\n");
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
        release_HomePhoto_resources(&g_ui);
        is_aiprocess_entry_aiset_back = false;
        ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
    } else {
        lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
        lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
    }
}

// 长按模式按键处理回调函数
static void photo_long_mode_callback(void)
{
    if(!is_processing) {
        MLOG_DBG("长按模式按键，切换前后摄像头\n");
        MESSAGE_S Msg = {0};
        Msg.topic     = EVENT_MODEMNG_SETTING;
        Msg.arg1      = PARAM_MENU_SENSOR_SWITCH;
        MODEMNG_SendMessage(&Msg);
        // 复位缩放
        set_zoom_level(1);
    }
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
                        is_aiprocess_entry_aiset_back = false;
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
    is_photo_back                = true;
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
    create_viewfinder(Home_Photo_Item->photoscr);

    // 相机模式按钮
    Home_Photo_Item->img_mode = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(Home_Photo_Item->img_mode, LV_ALIGN_TOP_LEFT, 6, 0);
    lv_obj_set_size(Home_Photo_Item->img_mode, 74, 47);
    show_image(Home_Photo_Item->img_mode, "paizhaompshi.png");

    // 分辨率按钮
    lv_obj_t *res = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(res, LV_ALIGN_TOP_LEFT, 72+14, 4);
    lv_obj_set_size(res, 38, 32);
    show_image(res, photo_getRes_Icon());


    Home_Photo_Item->redlight_level = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align( Home_Photo_Item->redlight_level, LV_ALIGN_TOP_LEFT, 116+14, 4);
    lv_obj_set_size( Home_Photo_Item->redlight_level, 38, 32);
    show_image( Home_Photo_Item->redlight_level, red_light_image_level[brightness_level]);

    lv_obj_t *iso_level = lv_imagebutton_create(Home_Photo_Item->photoscr);
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

    lv_obj_t *screenbrightness_level = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(screenbrightness_level, LV_ALIGN_TOP_LEFT, 204+14, 4);
    lv_obj_set_size(screenbrightness_level, 38, 32);
    char* brightness_buf[] = { "1.png", "2.png", "3.png", "4.png", "5.png", "6.png", "7.png" };
    show_image(screenbrightness_level, brightness_buf[get_curr_brightness()]);

    lv_obj_t *continue_photo = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(continue_photo, LV_ALIGN_TOP_LEFT, 268, 4);
    lv_obj_set_size(continue_photo, 38, 33);
    char* continue_buf[] = { "连拍关闭.png", "连拍3.png", "连拍5.png", "连拍7.png" };
    show_image(continue_photo, continue_buf[get_shootmode(0)]);

    lv_obj_t *delay_photo = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(delay_photo, LV_ALIGN_TOP_LEFT, 312, 4);
    lv_obj_set_size(delay_photo, 38, 33);
    char* delay_buf[] = { "延时 关闭.png", "延时5.png", "延时7.png", "延时10.png" };
    show_image(delay_photo, delay_buf[get_self_index()]);

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
    lv_obj_add_event_cb(imgbtn_zoomout, photo_zoom_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)1);

    lv_obj_t *imgbtn_zoomin = lv_imagebutton_create(Home_Photo_Item->photoscr);
    lv_obj_align(imgbtn_zoomin, LV_ALIGN_LEFT_MID, 12, 42);
    lv_obj_set_size(imgbtn_zoomin, 38, 38);
    show_image(imgbtn_zoomin, "W.png");
    lv_obj_add_event_cb(imgbtn_zoomin, photo_zoom_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)2);


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

void ai_takephoto_process_res_create(lv_obj_t *parent)
{

    if(parent == NULL) {
        MLOG_ERR("Parent is NULL in ai_takephoto_process_res_create\n");
        return;
    }

    // 创建AI处理容器
    ai_process_cont_s = lv_obj_create(parent);
    if(ai_process_cont_s == NULL) {
        MLOG_ERR("Failed to create AI process container\n");
        return;
    }

    lv_obj_set_size(ai_process_cont_s, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(ai_process_cont_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(ai_process_cont_s, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(ai_process_cont_s, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(ai_process_cont_s, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(ai_process_cont_s, 0, 0);
    lv_obj_set_style_pad_all(ai_process_cont_s, 0, 0);
    lv_obj_set_style_shadow_width(ai_process_cont_s, 0, 0);

    // 创建结果图片
    lv_obj_t *ai_process_result_img = lv_img_create(ai_process_cont_s);
    if(ai_process_result_img == NULL) {
        MLOG_ERR("Failed to create result image\n");
        return;
    }

    lv_obj_set_size(ai_process_result_img, H_RES, V_RES); // 留出底部空间
    lv_obj_align(ai_process_result_img, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_flag(ai_process_result_img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(ai_process_result_img, lv_color_hex(0x000000), 0);
    // 初始隐藏处理容器
    if(!is_aiprocess_entry_aiset_back || AIModeSelect_GetMode() == AI_NONE) {
        lv_obj_add_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN);
        register_all_key();
    } else {
        const char *result_img = get_ai_process_result_img_data(0);
        lv_image_set_src(ai_process_result_img, result_img);
        set_current_page_handler(NULL);
        set_current_page_handler(aiprocess_key_callback);
    }
    // 绑定点击事件
    // lv_obj_add_event_cb(ai_process_result_img, full_screen_img_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建处理中标签
    lv_obj_t *label_processing = lv_label_create(ai_process_cont_s);
    if(label_processing == NULL) {
        MLOG_ERR("Failed to create processing label\n");
        return;
    }

    lv_label_set_text(label_processing, "处理中...");
    lv_obj_set_style_text_font(label_processing, get_usr_fonts(ALI_PUHUITI_FONTPATH, 24), 0);
    lv_obj_set_style_text_color(label_processing, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_processing, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(label_processing, LV_OBJ_FLAG_HIDDEN);

    // 创建进度指示器
    lv_obj_t *spinner = lv_spinner_create(ai_process_cont_s);
    if(spinner) {
        lv_obj_set_size(spinner, 60, 60);
        lv_obj_set_style_arc_width(spinner, 6, 0);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0x0080FF), 0);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 60);
        lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    }

    // AI按钮
    lv_obj_t *ai_btn = lv_imagebutton_create(ai_process_cont_s);
    lv_obj_align(ai_btn, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_size(ai_btn, 58, 58);
    // lv_obj_add_event_cb(ai_btn, ai_process_select_event_handler, LV_EVENT_CLICKED, NULL);
    const char *mode_icon[] = {"aimoshi.png", "风格_1.png", "AI背景替换_1.png","美颜_1.png","语音_自定义_1.png"};
    show_image(ai_btn, mode_icon[AIModeSelect_GetMode()]);
    if(AIModeSelect_GetMode() == AI_NONE) {
        lv_obj_add_flag(ai_btn, LV_OBJ_FLAG_HIDDEN);
    }

    // 文本模式按钮
    lv_obj_t *text_mode_btn = lv_btn_create(ai_process_cont_s);
    lv_obj_set_size(text_mode_btn, 58, 58);
    lv_obj_align(text_mode_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_radius(text_mode_btn, lv_obj_get_style_radius(ai_btn, 0), 0);
    lv_obj_set_style_bg_color(text_mode_btn, lv_obj_get_style_bg_color(ai_btn, 0), 0);
    lv_obj_set_style_bg_opa(text_mode_btn, lv_obj_get_style_bg_opa(ai_btn, 0), 0);
    lv_obj_set_style_border_width(text_mode_btn, lv_obj_get_style_border_width(ai_btn, 0), 0);
    lv_obj_set_style_shadow_width(text_mode_btn, lv_obj_get_style_shadow_width(ai_btn, 0), 0);
    lv_obj_set_style_pad_all(text_mode_btn, 0, 0);

    // 创建标签并设置字体图标
    lv_obj_t *label = lv_label_create(text_mode_btn);
    if(label != NULL) {
        lv_label_set_text(label, LV_SYMBOL_LIST);
        lv_obj_center(label);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_42, 0);
        lv_obj_set_style_text_color(label, lv_color_hex(0x1296db), 0);
    }
    lv_obj_add_event_cb(text_mode_btn, buttonPhoto_All_event_handler, LV_EVENT_CLICKED, (void *)(intptr_t)2);

    // 如果AI按钮隐藏，文本模式按钮也隐藏
    if(lv_obj_has_flag(ai_btn, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(text_mode_btn, LV_OBJ_FLAG_HIDDEN);
    }

    // === 新增：创建删除按钮（右下角）===
    lv_obj_t *delete_btn = lv_btn_create(ai_process_cont_s);
    lv_obj_set_size(delete_btn, 64, 60);
    lv_obj_align(delete_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10); // 右下角位置
    lv_obj_set_style_radius(delete_btn, 5, 0); // 圆形按钮
    lv_obj_set_style_bg_color(delete_btn, lv_color_hex(0x171717), 0); // 红色背景
    lv_obj_set_style_bg_opa(delete_btn, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(delete_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加删除图标
    lv_obj_t *delete_label = lv_label_create(delete_btn);
    lv_label_set_text(delete_label, LV_SYMBOL_TRASH);
    lv_obj_center(delete_label);
    lv_obj_set_style_text_font(delete_label, &lv_font_montserrat_42, 0);
    lv_obj_set_style_text_color(delete_label, lv_color_white(), 0);

    // 添加点击事件
    lv_obj_add_event_cb(delete_btn, delete_ai_result_cb, LV_EVENT_CLICKED, delete_btn);

    // === 删除按钮创建结束 ===

    if(is_aiprocess_entry_aiset_back) {
        if(AIModeSelect_GetMode() == AI_BEAUTY) {
            photo_process_ai_beauty();
        } else {
            if(get_aiselete_scroll_handl() == NULL) {
                if(AI_NONE != AIModeSelect_GetMode()) {
                    photoAISelect_listCreat(ai_process_cont_s, ai_select_event_cb);
                }
            } else {
                photoScroll_del_cb();
            }
        }
    }

    if(AIModeSelect_GetMode() == AI_VOICE_CUSTOM) {
        rtt_init();
    } else {
        rtt_deinit();
    }
}

void photo_process_ai_beauty(void)
{
    set_defalt_retval();
    if (!check_battery_for_wifi(g_ui.page_photo.photoscr)) {
        return;
    }
    if(get_aiprocess_result_timer == NULL) {
        get_aiprocess_result_timer = lv_timer_create(aiprocessing_ui_update, 100, NULL);
        lv_timer_ready(get_aiprocess_result_timer);
    }
    if(!is_processing) {
        ai_process_state_set(AI_PROCESS_START);
        is_processing = true;
    } else {
        MLOG_DBG("重复点击处理，此次点击忽略");
    }
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

static void aiprocessing_ui_update(lv_timer_t *timer)
{
    // 获取容器中的各个对象
    lv_obj_t *img = lv_obj_get_child(ai_process_cont_s, 0);
    lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
    lv_obj_t *spinner = lv_obj_get_child(ai_process_cont_s, 2);


    if(lv_obj_has_flag(label, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    }
    if(lv_obj_has_flag(spinner, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    }

    static uint8_t tips_tim = 0;

    // 处理结果成功或失败
    if(get_retval() != 0 && get_retval() != DEFALT_RETVAL) {
        tips_tim++;
        lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);
        if (get_retval() == -2) {
            lv_label_set_text_fmt(label, "%s %s",
                str_language_network_not_connected[get_curr_language()],
                str_language_please_try_again[get_curr_language()]);
        } else if(AIModeSelect_GetMode() == AI_BEAUTY && (get_retval() == -7 || get_retval() == -3)) {
            lv_label_set_text(label, str_language_no_face_detected[get_curr_language()]);
        } else {
            lv_label_set_text_fmt(label, "处理失败,错误码：%d 请重试", get_retval());
        }
        lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);
        // 清除处理结果路径
        aiprocess_clean_cache();
        is_processing = false;
        if (tips_tim >= 20) {
            lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
            tips_tim = 0;
            if(get_aiprocess_result_timer != NULL) {
                lv_timer_del(get_aiprocess_result_timer);
                get_aiprocess_result_timer = NULL;
            }
        }
    } else if (get_retval() == 0) {
        char display_path[256];
        snprintf(display_path, sizeof(display_path), "%s", process_result_get_thumbnail());
        lv_image_set_src(img, display_path);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(spinner, LV_OBJ_FLAG_HIDDEN);

        is_processing = false;
        if(get_aiprocess_result_timer != NULL) {
            lv_timer_del(get_aiprocess_result_timer);
            get_aiprocess_result_timer = NULL;
        }
    } else {
        tips_tim = 0;
        static uint8_t leave_tips = 0;
        const char *text = lv_label_get_text(label);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

        if(strcmp(text, str_language_processing_please_do_not_leave[get_curr_language()]) == 0) {
            leave_tips++;
            if(leave_tips >= 10) {
                lv_label_set_text(label, "处理中...");
                leave_tips = 0;
            }
        } else {
            lv_label_set_text(label, "处理中...");
        }
    }
}

// 隐藏所有控件并保存状态
void hide_all_widgets(lv_obj_t *parent)
{
    // 首先计算需要存储的控件数量
    int child_count = lv_obj_get_child_count(parent);
    lv_obj_t *viewfinder = lv_obj_get_child(g_ui.page_photo.photoscr, 0);
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
        if (child != viewfinder) { // 假设viewfinder是取景框的全局变量
            lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);
        }

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
// 删除按钮点击事件处理函数
static void delete_ai_result_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        MLOG_DBG("用户点击删除AI处理结果按钮, get_retval():%d  is_processing:%d\n", get_retval(), is_processing);
        // 获取AI处理结果图片路径
        char *result_path = NULL;
        if(get_retval() == 0) {
            result_path = process_result_get();
            create_simple_delete_dialog(result_path); // 创建确认浮窗
            goto DISPLAY;
        } else if(get_retval() != 0 && is_processing == false) {
            result_path = pic_filepath;
        } else if(is_processing == true) {
            lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
            lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
            return;
        }
        if(result_path == NULL || strlen(result_path) == 0) {
            MLOG_DBG("无AI处理结果可删除\n");
            return;
        }

        // 规范化路径，移除多余的斜杠
        normalize_path(result_path);
        MLOG_DBG("规范化后的路径: %s\n", result_path);

        // 检查文件是否存在
        char *real_path = strchr(result_path, '/');
        if(real_path == NULL) {
            real_path = result_path;
        }

        // 提取文件名
        char *filename = strrchr(real_path, '/');
        if(filename == NULL) {
            filename = (char *)real_path;
        } else {
            filename++; // 跳过 '/'
        }

        MLOG_DBG("待删除的主文件: %s\n", real_path);
        MLOG_DBG("文件名: %s\n", filename);

        // 构建缩略图路径
        char thumbnail_path_small[100] = {0};
        char thumbnail_path_large[100] = {0};

        // 小缩略图路径
        get_thumbnail_path(filename, thumbnail_path_small, sizeof(thumbnail_path_small), PHOTO_SMALL_PATH);
        // 大缩略图路径
        get_thumbnail_path(filename, thumbnail_path_large, sizeof(thumbnail_path_large), PHOTO_LARGE_PATH);

        // 处理真实路径（移除"A:"前缀）
        char *real_path_small = strchr(thumbnail_path_small, '/');
        char *real_path_large = strchr(thumbnail_path_large, '/');

        if(real_path_small == NULL) real_path_small = thumbnail_path_small;
        if(real_path_large == NULL) real_path_large = thumbnail_path_large;

        // 删除所有相关文件
        char cmd[512]    = {0};
        int delete_count = 0;
        int total_files  = 0;

        // 检查并删除小缩略图
        if(access(real_path_small, F_OK) == 0) {
            total_files++;
            snprintf(cmd, sizeof(cmd), "rm -f \"%s\"", real_path_small);
            MLOG_DBG("执行删除命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("成功删除小缩略图: %s\n", real_path_small);
                delete_count++;
            } else {
                MLOG_ERR("删除小缩略图失败: %s\n", real_path_small);
            }
        } else {
            MLOG_DBG("小缩略图不存在: %s\n", real_path_small);
        }

        // 检查并删除大缩略图
        if(access(real_path_large, F_OK) == 0) {
            total_files++;
            snprintf(cmd, sizeof(cmd), "rm -f \"%s\"", real_path_large);
            MLOG_DBG("执行删除命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("成功删除大缩略图: %s\n", real_path_large);
                delete_count++;
            } else {
                MLOG_ERR("删除大缩略图失败: %s\n", real_path_large);
            }
        } else {
            MLOG_DBG("大缩略图不存在: %s\n", real_path_large);
        }

        // 检查并删除主文件
        if(access(real_path, F_OK) == 0) {
            total_files++;
            snprintf(cmd, sizeof(cmd), "rm -f \"%s\"", real_path);
            MLOG_DBG("执行删除命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("成功删除主文件: %s\n", real_path);
                delete_count++;
            } else {
                MLOG_ERR("删除主文件失败: %s\n", real_path);
            }

            // 从文件管理器中移除文件记录
            FILEMNG_DelFile(0, real_path);
        } else {
            MLOG_DBG("主文件不存在: %s\n", real_path);
        }
        MLOG_DBG("总共尝试删除 %d 个文件，成功删除 %d 个\n", total_files, delete_count);

    DISPLAY:
        if(get_retval() == 0) {
            sure_delete_register_callback(ai_result_delete_after_cb);
        } else if(ai_process_cont_s != NULL && lv_obj_is_valid(ai_process_cont_s)) {
            is_aiprocess_entry_aiset_back = false;
            lv_obj_add_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN);
            register_all_key();
        }
    }
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
    takephoto_register_ok_callback(photo_ok_callback);
    takephoto_register_long_menu_callback(photo_long_menu_callback);
    takephoto_register_play_callback(photo_play_callback);
    takephoto_register_up_callback(photo_redlight_callback);
    takephoto_register_down_callback(photo_redlight_callback);
    takephoto_register_zoomin_callback(zoomin_key_cb);
    takephoto_register_zoomout_callback(zoomout_key_cb);
    takephoto_register_left_callback(photo_left_callback);
    takephoto_register_right_callback(photo_right_callback);
    takephoto_register_before_callback(key_takephoto_before_callback);
    takephoto_power_callback(key_takephoto_power_callback);
}

static void aiprocess_key_callback(int key_code, int key_value)
{
    if(key_code == KEY_MENU && key_value == 1) {
        photo_menu_callback();
    } else if(key_code == KEY_PLAY && key_value == 1) {
        photo_play_callback();
    } else if((key_code == KEY_LEFT || key_code == KEY_RIGHT) && key_value == 1) {
        if(key_code == KEY_RIGHT) {
            AISelect_next();
        } else {
            AISelect_prev();
        }
    } else if(key_code == KEY_DOWN && key_value == 1) {
        photo_key_down_callback();
    } else if(key_code == KEY_OK && key_value == 1) {
        extern int g_current_selected_index;
        start_aiprocess(g_current_selected_index);
    } else if (key_code == KEY_CAMERA && key_value == 1) {
        if (!lv_obj_has_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN)) {
            if (!is_processing) {
                lv_obj_add_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN);
                register_all_key();
            } else {
                lv_obj_t* label = lv_obj_get_child(ai_process_cont_s, 1);
                lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
            }
        }
    }
}

// 风格提示词数组，与scene_btn_labels顺序对应
const char* style_prompts[] = {
    "将这张照片重绘成 3D 风格",
    "将这张照片重绘成 写实风 风格",
    "将这张照片重绘成 天使风 风格",
    "将这张照片重绘成 动漫风 风格",
    "将这张照片重绘成 日漫风 风格",
    "将这张照片重绘成 公主风 风格",
    "将这张照片重绘成 梦幻风 风格",
    "将这张照片重绘成 水墨风 风格",
    "将这张照片重绘成 新莫奈花园风 风格",
    "将这张照片重绘成 水彩风 风格",
    "将这张照片重绘成 莫奈花园风 风格",
    "将这张照片重绘成 精致美漫 风格",
    "将这张照片重绘成 赛博机械 风格",
    "将这张照片重绘成 精致韩漫 风格",
    "将这张照片重绘成 国风-水墨 风格",
    "将这张照片重绘成 浪漫光影 风格",
    "将这张照片重绘成 瓷娃娃 风格",
    "将这张照片重绘成 中国红 风格",
    "将这张照片重绘成 丑萌粘土 风格",
    "将这张照片重绘成 可爱玩偶 风格",
    "将这张照片重绘成 3D游戏Z世代风 风格",
    "将这张照片重绘成 动画电影 风格",
    "将这张照片重绘成 玩偶 风格",
    "将这张照片重绘成 青年15岁 风格，年轻，稚嫩",
    "将这张照片重绘成 中年35岁 风格，成熟，稳重",
    "将这张照片重绘成 老年80岁 风格，白发，皱纹",
};

// 背景提示词数组，与scene_btn_labels顺序对应
const char* bg_prompts[] = {
    "保证图片的主体不变 将这张照片的背景换成 长城落日",
    "保证图片的主体不变 将这张照片的背景换成 山谷",
    "保证图片的主体不变 将这张照片的背景换成 秋季落叶",
    "保证图片的主体不变 将这张照片的背景换成 冬季雪景",
    "保证图片的主体不变 将这张照片的背景换成 操场",
    "保证图片的主体不变 将这张照片的背景换成 沙滩",
};

void start_aiprocess(const int index)
{
    if (!check_battery_for_wifi(g_ui.page_photo.photoscr)) {
        return;
    }
    // 获取容器中的图片和标签对象
    lv_obj_t *img          = lv_obj_get_child(ai_process_cont_s, 0);
    lv_obj_t *label        = lv_obj_get_child(ai_process_cont_s, 1);
    lv_obj_t *spinner      = lv_obj_get_child(ai_process_cont_s, 2);
    const char *result_img = get_ai_process_result_img_data(0);

    lv_image_set_src(img, result_img); // 每次点击，都需要先显示一下原图
    lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    set_defalt_retval();
    if (AIModeSelect_GetMode() == AI_SCENE_CHANGE) {
        MLOG_DBG("开始处理图像风格转换，提示词: %s\n", style_prompts[index]);
        aiprocess_set_prompt(style_prompts[index]);
    } else if (AIModeSelect_GetMode() == AI_BG_CHANGE) {
        MLOG_DBG("开始处理图像背景替换，提示词: %s\n", bg_prompts[index]);
        aiprocess_set_prompt(bg_prompts[index]);
    }

    if (get_aiprocess_result_timer == NULL) {
        get_aiprocess_result_timer = lv_timer_create(aiprocessing_ui_update, 100, NULL);
        lv_timer_ready(get_aiprocess_result_timer);
    }
    if(!is_processing) {
        ai_process_state_set(AI_PROCESS_START);
        is_processing = true;
    } else {
        MLOG_DBG("重复点击处理，此次点击忽略");
    }
}

static void album_start_ai_custom_process(const char* text)
{
    if(text == NULL) {
        return;
    }
    if (!check_battery_for_wifi(g_ui.page_photo.photoscr)) {
        return;
    }

    // 获取容器中的图片和标签对象
    lv_obj_t *img          = lv_obj_get_child(ai_process_cont_s, 0);
    lv_obj_t *label        = lv_obj_get_child(ai_process_cont_s, 1);
    lv_obj_t *spinner      = lv_obj_get_child(ai_process_cont_s, 2);
    const char *result_img = get_ai_process_result_img_data(0);

    lv_image_set_src(img, result_img); // 每次点击，都需要先显示一下原图
    lv_obj_remove_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(spinner, LV_OBJ_FLAG_HIDDEN);
    set_defalt_retval();

    if ((AIModeSelect_GetMode() == AI_VOICE_CUSTOM)) {
        MLOG_DBG("自定义提示词: %s\n", text);
        aiprocess_set_prompt(text);
    }

    if (get_aiprocess_result_timer == NULL) {
        get_aiprocess_result_timer = lv_timer_create(aiprocessing_ui_update, 100, NULL);
        lv_timer_ready(get_aiprocess_result_timer);
    }
    if(!is_processing) {
        ai_process_state_set(AI_PROCESS_START);
        is_processing = true;
    } else {
        MLOG_DBG("重复点击处理，此次点击忽略");
    }
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

static void ai_result_delete_after_cb(void)
{
    // 获取UI对象
    lv_obj_t *img = lv_obj_get_child(ai_process_cont_s, 0);
    // 更新UI
    if(img != NULL && lv_obj_is_valid(img)) {
        // 切换到原图显示
        const char *original_img = get_ai_process_result_img_data(0);
        lv_image_set_src(img, original_img);
        MLOG_DBG("已切换到原图显示\n");
    }
    // 清除处理结果路径
    aiprocess_clean_cache();
    // 重置处理状态
    is_processing = false;
    set_defalt_retval();
}

static void rtt_get_text_timer_cb(lv_timer_t *timer)
{
    char* custom_voice_text = NULL;
    int32_t ret = 0;
    ret = rtt_get_text(&custom_voice_text);
    if(ret != RTT_SUCCESS) {
        MLOG_INFO("text: %s\n", custom_voice_text);
        if(rtt_get_text_timer) {
            lv_timer_del(rtt_get_text_timer);
            rtt_get_text_timer = NULL;
        }
        rtt_reset();
        return;
    }

    if(custom_voice_text != NULL) {
        MLOG_INFO("text: %s\n", custom_voice_text);
        if(strlen(custom_voice_text) > 0) {
            voice_text_set(custom_voice_text);
        }
    }

    if((rtt_is_finial() && ai_custom_is_confire)) {
        if(rtt_get_text_timer) {
            lv_timer_del(rtt_get_text_timer);
            rtt_get_text_timer = NULL;
        }
        album_start_ai_custom_process(custom_voice_text);
    } else {
        lv_timer_reset(timer);
    }
}

static void ai_process_float_back_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            if(!lv_obj_has_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN)) {
                if(!is_processing) {
                    lv_obj_add_flag(ai_process_cont_s, LV_OBJ_FLAG_HIDDEN);
                    register_all_key();
                } else {
                    lv_obj_t *label = lv_obj_get_child(ai_process_cont_s, 1);
                    lv_label_set_text(label, str_language_processing_please_do_not_leave[get_curr_language()]);
                }
            }
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


static void photo_zoom_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int Click_index      = (int)lv_event_get_user_data(e);
    uint32_t new_level = get_zoom_level();
    MESSAGE_S Msg = {0};
    if(code == LV_EVENT_CLICKED) {
        switch(Click_index) {
            case 1: // T
                if (new_level < ZOOM_RADIO_MAX) {
                    new_level++;
                    set_zoom_level(new_level);
                    new_level = get_zoom_level();
                    // 设置放大比例
                    Msg.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
                    Msg.arg1 = 0;
                    snprintf((char*)Msg.aszPayload, 3, "%d", new_level);
                    MODEMNG_SendMessage(&Msg);
                    // 更新UI
                    update_zoom_bar(new_level);
                }

                break;
            case 2: // w
                if (new_level > 1) {
                    new_level--;
                    set_zoom_level(new_level);
                    new_level = get_zoom_level();
                    // 设置放大比例
                    Msg.topic = EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS;
                    Msg.arg1 = 0;
                    snprintf((char*)Msg.aszPayload, 3, "%d", new_level);
                    MODEMNG_SendMessage(&Msg);
                    // 更新UI
                    update_zoom_bar(new_level);
                }
                break;
            default: break;
        }
    }
}
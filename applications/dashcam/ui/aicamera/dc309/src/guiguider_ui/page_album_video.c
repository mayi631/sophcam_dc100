#define DEBUG
#include "page_album_video.h"
#include "common/extract_thumbnail.h"
#include "custom.h"
#include "events_init.h"
#include "filemng.h"
#include "gui_guider.h"
#include "indev.h"
#include "linux/input.h"
#include "lvgl.h"
#include "page_all.h"
#include "power.h"
#include "ui_common.h"
#include "volmng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char current_image_path[256];
extern int g_total_media_files;
extern char **g_all_filenames;
extern int g_current_page;  // 当前页面索引
extern int g_total_pages;
extern int g_images_per_page;  // 每页显示的图片数量
extern int g_cam_id; // 当前相机ID，第一路
extern int g_album_image_index; // 当前视频在所有视频中的索引
extern int g_return_page_index ;    // 返回时需要跳转的页面索引
extern int g_return_focus_index ;  // 返回时的焦点索引

// 必要的全局变量
lv_obj_t *obj_AibumVid_s = NULL;
static bool g_video_playing = false;//播放状态标志
static int g_video_duration = 120; // 总时长
static int g_video_position = 0;   //当前时长
static lv_timer_t *g_playback_task = NULL;//播放定时器

// 保留播放按钮和时间标签为全局变量
static lv_obj_t *g_progress_bar = NULL; //进度条
static lv_obj_t *g_play_btn_big = NULL; //大播放按钮
static lv_obj_t *g_play_btn = NULL;     //小播放按钮
static lv_obj_t *g_time_label = NULL;   //时间标签
static lv_obj_t *btn_delete_vid = NULL;

void video_playback_task(lv_timer_t *timer);
static void voice_style_btn_event_cb(lv_event_t *e);
static void ord_delete_after_cb(void);
static int set_current_photo_album_image_path(int index)
{
    MLOG_ERR("当前索引 %d\n", index);
    if(index < 0 || index >= g_total_media_files) {
        MLOG_ERR("索引 %d 超出范围 (0 - %d)\n", index, g_total_media_files - 1);
        return -1;
    }
    if(g_all_filenames[index] == NULL) {
        MLOG_ERR("索引 %d 处的文件名为空\n", index);
        return -1;
    }
    // 构建完整的图片路径：PHOTO_ALBUM_IMAGE_PATH_L + 文件名，查看的是大缩略图
    char *real_path_small = strchr(PHOTO_ALBUM_MOVIE_PATH, '/');
    snprintf(current_image_path, sizeof(current_image_path), "%s%s", real_path_small,
             g_all_filenames[index]);
    MLOG_DBG("设置当前图片路径: %s\n", current_image_path);
    return 0;
}

static void play_button_event_handler(lv_event_t *e)
{
    video_play_pause();
}

static void progress_bar_event_handler(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *bar = lv_event_get_target(e);
        int position = lv_bar_get_value(bar);
        video_seek_to(position);
    }
}

static void screen_PhotoAlbumVid_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED:
            // 停止播放任务
            if(g_playback_task) {
                lv_timer_del(g_playback_task);
                g_playback_task = NULL;
            }
            // 返回相册页面
            PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
            PLAYER_SERVICE_Stop(ps_handle);
            voice_arc_delete();
            g_return_page_index = g_album_image_index / g_images_per_page; // 焦点
            g_return_focus_index = g_album_image_index % g_images_per_page; // 页面
            ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL,
                Home_Album_from_Pic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        default:
            break;
        }
}

static void screen_PhotoAlbumVid_btn_delete_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 停止播放任务
            if(g_playback_task) {
                lv_timer_del(g_playback_task);
                g_playback_task = NULL;
            }
            g_video_playing = false;
            if(g_play_btn) lv_label_set_text(g_play_btn, LV_SYMBOL_PLAY);
            if(g_play_btn_big) lv_label_set_text(g_play_btn_big, LV_SYMBOL_PLAY);

            create_simple_delete_dialog(current_image_path); // 创建确认浮窗
            sure_delete_register_callback(ord_delete_after_cb);
            break;
        }
        default: break;
    }
}

void events_init_screen_PhotoAlbumVid(lv_ui_t *ui, lv_obj_t *btn_back, lv_obj_t *btn_delete)
{
    lv_obj_add_event_cb(btn_back, screen_PhotoAlbumVid_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_delete, screen_PhotoAlbumVid_btn_delete_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(g_progress_bar, progress_bar_event_handler, LV_EVENT_VALUE_CHANGED, ui);
}

static int32_t play_file()
{
    int32_t s32Ret = 0;
    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
    PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    MAPI_AO_Unmute(MEDIA_GetCtx()->SysHandle.aohdl);

#ifdef SERVICES_PLAYER_SUBVIDEO
    PLAYER_SERVICE_SetPlaySubStreamFlag(ps_handle, true);
#endif
    s32Ret = PLAYER_SERVICE_SetInput(ps_handle, current_image_path);
    if (s32Ret != 0) {
        CVI_LOGE("Player set input %s failed", current_image_path);
        return s32Ret;
    }

    PLAYER_SERVICE_Play(ps_handle);

    return s32Ret;
}

static int32_t prepare_play_file()
{
    int32_t s32Ret = 0;
    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();
    PLAYER_SERVICE_HANDLE_T ps_handle = MediaParams->SysServices.PsHdl;
    MAPI_AO_Unmute(MEDIA_GetCtx()->SysHandle.aohdl);

    #ifdef SERVICES_PLAYER_SUBVIDEO
    PLAYER_SERVICE_SetPlaySubStreamFlag(ps_handle, true);
    #endif
    s32Ret = PLAYER_SERVICE_SetInput(ps_handle, current_image_path);
    if (s32Ret != 0) {
        CVI_LOGE("Player set input %s failed", current_image_path);
        return s32Ret;
    }

    PLAYER_SERVICE_Play(ps_handle);
    PLAYER_SERVICE_TouchSeekPause(ps_handle, 0);

    return s32Ret;
}

void video_play_pause()
{
    g_video_playing = !g_video_playing;

    MLOG_DBG("video sta: %d\n", g_video_playing);

    if(g_video_playing) {
        // 更新按钮图标为暂停
        if (g_play_btn)
            lv_label_set_text(g_play_btn, LV_SYMBOL_PAUSE);
        if (g_play_btn_big)
            lv_label_set_text(g_play_btn_big, LV_SYMBOL_PAUSE);

        // 视频播放时阻止息屏
        update_last_activity_time();

        // 隐藏大播放按钮的父对象
        if (g_play_btn_big) {
            lv_obj_t* parent = lv_obj_get_parent(g_play_btn_big);
            if(parent && !lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(parent, LV_OBJ_FLAG_HIDDEN);
            }
        }

        // 启动播放任务
        if(!g_playback_task) {
            g_playback_task = lv_timer_create(video_playback_task, 1000, NULL);
        }

        if (!g_video_position) {
            // 如果是从头开始播放，调用播放函数
            play_file();
        } else {
            // 继续播放
            MAPI_AO_Unmute(MEDIA_GetCtx()->SysHandle.aohdl);
            PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
            PLAYER_SERVICE_Play(ps_handle);
        }
    } else {
        // 更新按钮图标为播放
        if(g_play_btn) lv_label_set_text(g_play_btn, LV_SYMBOL_PLAY);
        if(g_play_btn_big) lv_label_set_text(g_play_btn_big, LV_SYMBOL_PLAY);

        // 显示大播放按钮的父对象
        if(g_play_btn_big) {
            lv_obj_t *parent = lv_obj_get_parent(g_play_btn_big);
            if(parent && lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
            }
        }

        // 停止播放任务
        if(g_playback_task) {
            lv_timer_del(g_playback_task);
            g_playback_task = NULL;
        }

        PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
        PLAYER_SERVICE_Pause(ps_handle);
    }
}

void video_seek_to(int position)
{
    g_video_position = position;
    if(g_progress_bar) {
        lv_bar_set_value(g_progress_bar, position, LV_ANIM_ON);
    }

    if(g_time_label) {
        char time_str[20];
        snprintf(time_str, sizeof(time_str), "%02d:%02d/%02d:%02d",
                 g_video_position / 60, g_video_position % 60,
                 g_video_duration / 60, g_video_duration % 60);
        lv_label_set_text(g_time_label, time_str);
    }
}

void video_playback_task(lv_timer_t *timer)
{
    if (!g_video_playing)
        return;

    // 视频播放过程中持续更新活动时间，防止息屏
    update_last_activity_time();

    // 更新播放位置
    if (g_video_position < g_video_duration) {
        g_video_position++;
    } else {
        // 播放结束
        g_video_playing = false;
        if(g_play_btn) lv_label_set_text(g_play_btn, LV_SYMBOL_PLAY);
        if(g_play_btn_big) lv_label_set_text(g_play_btn_big, LV_SYMBOL_PLAY);

        // 显示大播放按钮的父对象
        if(g_play_btn_big) {
            lv_obj_t *parent = lv_obj_get_parent(g_play_btn_big);
            if(parent && lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
            }
        }

        g_video_position = 0;
    }

    // 更新进度条
    if(g_progress_bar) {
        lv_bar_set_value(g_progress_bar, g_video_position, LV_ANIM_OFF);
    }

    // 更新时间显示
    if(g_time_label) {
        char time_str[20];
        snprintf(time_str, sizeof(time_str), "%02d:%02d/%02d:%02d",
                 g_video_position / 60, g_video_position % 60,
                 g_video_duration / 60, g_video_duration % 60);
        lv_label_set_text(g_time_label, time_str);
    }
}

void video_delete_anim_complete(lv_anim_t *a)
{
    g_video_position = 0;
    g_return_page_index = g_album_image_index / g_images_per_page;//焦点
    g_return_focus_index = g_album_image_index % g_images_per_page;//页面
    // 停止当前播放
    if(g_playback_task) {
        lv_timer_del(g_playback_task);
        g_playback_task = NULL;
    }
    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    PLAYER_SERVICE_Stop(ps_handle);
    voice_arc_delete();
    // 视频页刷新
    ui_load_scr_animation(&g_ui, &obj_AibumVid_s, 1, NULL, setup_scr_screen_PhotoAlbumVid, LV_SCR_LOAD_ANIM_NONE, 0, 0,
        false, true);
    lv_anim_del(a, a->exec_cb);
}


void albumvid_menu_callback(int key_code, int key_value)
{
    if(key_code == KEY_MENU && key_value == 1)
    {
            MLOG_DBG("albumvid_menu_callback\n");
            // 停止播放任务
            if(g_playback_task) {
                lv_timer_del(g_playback_task);
                g_playback_task = NULL;
            }
            // 返回相册页面
            PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
            PLAYER_SERVICE_Stop(ps_handle);
            voice_arc_delete();
            g_return_page_index = g_album_image_index / g_images_per_page; // 焦点
            g_return_focus_index = g_album_image_index % g_images_per_page; // 页面
            ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL,
                Home_Album_from_Pic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
    } else if (key_code == KEY_OK && key_value == 1) {
        video_play_pause();
    } else if ((key_code == KEY_LEFT || key_code == KEY_RIGHT) && key_value == 1) {
        if(key_code == KEY_LEFT) // 左键
        {
            g_album_image_index++; // 左
            if(g_album_image_index >= g_total_media_files) {
                g_album_image_index = g_total_media_files - 1;
            }
        } else if(key_code == KEY_RIGHT) {
            g_album_image_index--;
            if(g_album_image_index < 0) {
                g_album_image_index = 0;
            }
        }
        g_return_page_index = g_album_image_index / g_images_per_page;//焦点
        g_return_focus_index = g_album_image_index % g_images_per_page;//页面
        set_current_photo_album_image_path(g_album_image_index);
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_values(&anim, 0, 1);
        lv_anim_set_time(&anim, 50);
        lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
        lv_anim_set_completed_cb(&anim, video_delete_anim_complete);
        lv_anim_start(&anim);
    } else if ((key_code == KEY_ZOOMIN || key_code == KEY_ZOOMOUT)) {
        if(key_code == KEY_ZOOMIN) // T键
        {
            do_zoomin(key_value);
        } else if(key_code == KEY_ZOOMOUT) {
            do_zoomout(key_value);
        }
    } else if ((key_code == KEY_DOWN) && key_value == 1) {
        lv_obj_send_event(btn_delete_vid, LV_EVENT_CLICKED, NULL);
    }
}
// 上一视频
static void prev_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            g_album_image_index--;
            if(g_album_image_index < 0) {
                g_album_image_index = 0;
            }
            set_current_photo_album_image_path(g_album_image_index);
            lv_anim_t anim;
            lv_anim_init(&anim);
            lv_anim_set_values(&anim, 0, 1);
            lv_anim_set_time(&anim, 50);
            lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
            lv_anim_set_completed_cb(&anim, video_delete_anim_complete);
            lv_anim_start(&anim);

        }; break;
        default: break;
    }
}
// 下一视频
static void next_button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code) {
        case LV_EVENT_CLICKED: {
            g_album_image_index++;
            if(g_album_image_index >= g_total_media_files) {
                g_album_image_index = g_total_media_files - 1;
            }
            set_current_photo_album_image_path(g_album_image_index);
            lv_anim_t anim;
            lv_anim_init(&anim);
            lv_anim_set_values(&anim, 0, 1);
            lv_anim_set_time(&anim, 50);
            lv_anim_set_path_cb(&anim, lv_anim_path_linear); // 使用线性路径
            lv_anim_set_completed_cb(&anim, video_delete_anim_complete);
            lv_anim_start(&anim);
        }; break;
        default: break;
    }
}

void setup_scr_screen_PhotoAlbumVid(lv_ui_t* ui)
{
    // 创建主容器
    obj_AibumVid_s = lv_obj_create(NULL);
    lv_obj_set_size(obj_AibumVid_s, 640, 360);
    lv_obj_set_scrollbar_mode(obj_AibumVid_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(obj_AibumVid_s, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(obj_AibumVid_s, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj_AibumVid_s, LV_OPA_0, LV_PART_MAIN);

    // 顶部容器（局部变量）
    lv_obj_t *cont_top_vid = lv_obj_create(obj_AibumVid_s);
    lv_obj_set_pos(cont_top_vid, 0, 0);
    lv_obj_set_size(cont_top_vid, 640, 60);
    lv_obj_set_scrollbar_mode(cont_top_vid, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top_vid, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 返回按钮（局部变量）
    lv_obj_t* btn_back_vid = lv_button_create(cont_top_vid);
    lv_obj_set_pos(btn_back_vid, 4, 2);
    lv_obj_set_size(btn_back_vid, 60, 48);
    lv_obj_add_style(btn_back_vid, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label_back = lv_label_create(btn_back_vid);
    lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);
    // 删除按钮（局部变量）
    btn_delete_vid = lv_button_create(cont_top_vid);
    lv_obj_set_pos(btn_delete_vid, 556, 2);
    lv_obj_set_size(btn_delete_vid, 64, 48);
    lv_obj_set_style_bg_color(btn_delete_vid, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_delete_vid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *label_delete = lv_label_create(btn_delete_vid);
    lv_label_set_text(label_delete, " " LV_SYMBOL_TRASH " ");
    lv_obj_align(label_delete, LV_ALIGN_CENTER, 0, 0);
    MLOG_ERR("当前路径 %s\n", current_image_path);
    // 视频名称标签（局部变量）
    char* filename = strrchr(current_image_path, '/');
    filename = filename ? filename + 1 : current_image_path;
    lv_obj_t *label_name_vid = lv_label_create(cont_top_vid);
    lv_obj_set_size(label_name_vid, 200, 40);
    lv_label_set_text(label_name_vid, filename);
    lv_obj_align(label_name_vid, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_long_mode(label_name_vid, LV_LABEL_LONG_SCROLL);
    lv_obj_set_style_text_color(label_name_vid, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(label_name_vid, get_usr_fonts(ALI_PUHUITI_FONTPATH, 24), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(label_name_vid, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 视频播放容器（局部变量）
    lv_obj_t *video_container = lv_obj_create(obj_AibumVid_s);
    lv_obj_set_pos(video_container, 0, 60);  // 保持顶部不变
    lv_obj_set_size(video_container, 640, 240);  // 高度从320改为240
    lv_obj_set_style_bg_color(video_container, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(video_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(video_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(video_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(video_container, 0, LV_STATE_DEFAULT);

    lv_obj_set_style_bg_opa(video_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(video_container, LV_OPA_0, LV_PART_MAIN);

    // 添加视频播放器占位符
    lv_obj_t *video_cover = lv_img_create(video_container);
    lv_obj_align(video_cover, LV_ALIGN_CENTER, 0, 0);

    // 添加播放按钮（居中）
    lv_obj_t *play_btn_overlay = lv_button_create(video_container);
    lv_obj_set_size(play_btn_overlay, 80, 80);
    lv_obj_align(play_btn_overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_opa(play_btn_overlay, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_radius(play_btn_overlay, 40, LV_PART_MAIN);
    lv_obj_set_style_pad_all(play_btn_overlay, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(play_btn_overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(play_btn_overlay, play_button_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_set_style_bg_color(play_btn_overlay, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(play_btn_overlay, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 大播放按钮标签（全局变量）
    g_play_btn_big = lv_label_create(play_btn_overlay);
    lv_label_set_text(g_play_btn_big, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(g_play_btn_big, &lv_font_montserrat_48, 0);
    lv_obj_align(g_play_btn_big, LV_ALIGN_CENTER, 0, 0);

    // 底部控制栏（局部变量）
    lv_obj_t *cont_bottom_vid = lv_obj_create(obj_AibumVid_s);
    lv_obj_set_pos(cont_bottom_vid, 0, 300);  // 从380改为300 (60+240=300)
    lv_obj_set_size(cont_bottom_vid, 640, 60);  // 高度从100改为60
    lv_obj_set_style_bg_opa(cont_bottom_vid, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont_bottom_vid, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(cont_bottom_vid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont_bottom_vid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(cont_bottom_vid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(cont_bottom_vid, 0, LV_STATE_DEFAULT);

    g_video_duration =  get_video_duration(current_image_path);
    g_video_position = 0;   // 初始化为0
    g_video_playing = false;//播放状态标志
    MLOG_DBG("视频总时长: %d秒\n", g_video_duration);

    // 进度条（全局变量）
    g_progress_bar = lv_bar_create(cont_bottom_vid);
    lv_obj_set_size(g_progress_bar, 300, 6);
    lv_obj_align(g_progress_bar, LV_ALIGN_BOTTOM_MID, -20, -10);
    lv_bar_set_range(g_progress_bar, 0, g_video_duration);
    lv_bar_set_value(g_progress_bar, 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(g_progress_bar, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_progress_bar, LV_OPA_COVER, LV_PART_MAIN);
    // 设置指示器颜色（填充部分）- 默认状态为深灰色（未播放状态）
    lv_obj_set_style_bg_color(g_progress_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);

    // 时间显示（全局变量）
    char time_str[20];
    snprintf(time_str, sizeof(time_str), "00:00/%02d:%02d",
             g_video_duration / 60, g_video_duration % 60);
    g_time_label = lv_label_create(cont_bottom_vid);
    lv_label_set_text(g_time_label, time_str);
    lv_obj_align(g_time_label, LV_ALIGN_BOTTOM_RIGHT, -90, -5);  // 从-20改为-5
    lv_obj_set_style_text_color(g_time_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(g_time_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 16), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加播放/暂停按钮（局部变量）
    lv_obj_t *play_btn = lv_btn_create(cont_bottom_vid);
    lv_obj_set_size(play_btn, 48, 48);  // 从58改为48
    lv_obj_set_style_radius(play_btn, 20, 0);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(play_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(play_btn, LV_ALIGN_BOTTOM_LEFT, 6, 0);

    // 小播放按钮标签（全局变量）
    g_play_btn = lv_label_create(play_btn);
    lv_label_set_text(g_play_btn, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(g_play_btn, &lv_font_montserrat_24, 0);  // 从30改为24
    lv_obj_center(g_play_btn);
    lv_obj_add_event_cb(play_btn, play_button_event_handler, LV_EVENT_CLICKED, ui);

    // 音色选择按钮（右侧）
    lv_obj_t *voice_style_btn = lv_btn_create(cont_bottom_vid);
    lv_obj_set_size(voice_style_btn, 48, 48);  // 从58改为48
    lv_obj_align(voice_style_btn, LV_ALIGN_BOTTOM_RIGHT, -10, 0);
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
    lv_obj_add_event_cb(voice_style_btn, voice_style_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 添加上方分割线（局部变量）
    lv_obj_t *up_line = lv_line_create(obj_AibumVid_s);
    static lv_point_precise_t points_line[] = {{0, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);

    // 设置当前页面的按键处理器
    set_current_page_handler(albumvid_menu_callback);

    // 初始化事件
    events_init_screen_PhotoAlbumVid(ui, btn_back_vid, btn_delete_vid);

    // 刚开始处于暂停状态
    // 更新按钮图标为暂停
    if(g_play_btn) lv_label_set_text(g_play_btn, LV_SYMBOL_PLAY);
    if(g_play_btn_big) lv_label_set_text(g_play_btn_big, LV_SYMBOL_PLAY);

    // 显示大播放按钮的父对象
    if(g_play_btn_big) {
        lv_obj_t *parent = lv_obj_get_parent(g_play_btn_big);
        if(parent && lv_obj_has_flag(parent, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(parent, LV_OBJ_FLAG_HIDDEN);
        }
    }
    // 启动播放任务
    if(!g_playback_task) {
        g_playback_task = lv_timer_create(video_playback_task, 1000, NULL);
    }

    /* 准备播放文件并卡住第一帧 */
    prepare_play_file();
}
// 音色选择按钮事件回调
static void voice_style_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 创建音量设置
        if(get_arc_handel() == NULL) {
            voice_setting_arc_create();
        } else {
            voice_arc_delete();
        }
    }
}

static void ord_delete_after_cb(void)
{
    PLAYER_SERVICE_HANDLE_T ps_handle = MEDIA_GetCtx()->SysServices.PsHdl;
    PLAYER_SERVICE_Stop(ps_handle);
    char* last_slash = strrchr(current_image_path, '/');
    char* filename = last_slash ? last_slash + 1 : current_image_path;

    MLOG_DBG("删除视频: %s\n", filename);
    // 从全局文件名数组中移除已删除的文件
    for(int j = 0; j < g_total_media_files; j++) {
        if(g_all_filenames[j] && strcmp(g_all_filenames[j], filename) == 0) {
            free(g_all_filenames[j]);
            g_all_filenames[j] = NULL;
            // 将后面的文件名前移
            for(int k = j; k < g_total_media_files - 1; k++) {
                g_all_filenames[k] = g_all_filenames[k + 1];
            }
            if(((j + 1) % g_images_per_page == 1) && (j == g_total_media_files - 1)) {
                g_current_page--;
                g_total_pages--;
                if(g_current_page < 0) {
                    g_current_page = 0;
                }
            }
            g_total_media_files--;
            // 调整返回页面和焦点索引
            if(j <= g_album_image_index) {
                // 如果删除的是当前或之前的视频，需要调整索引
                g_album_image_index--;
                if(g_album_image_index < 0) g_album_image_index = 0;

                // 重新计算返回页面和焦点
                g_return_page_index  = g_album_image_index / g_images_per_page;
                g_return_focus_index = g_album_image_index % g_images_per_page;
            }
            break;
        }
    }
    // 返回相册页面
    voice_arc_delete();
    ui_load_scr_animation(&g_ui, &obj_Aibum_s, 1, NULL, Home_Album_from_Pic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}


#define DEBUG
/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include "page_all.h"
#include "filemng.h"
#include "common/extract_thumbnail.h"
#include "ui_common.h"

// 删除确认弹窗相关变量
lv_obj_t *simple_dialog             = NULL;
char delete_filename[100]           = {0};
sure_delete_callback_t make_sure_cb = NULL;
extern lv_style_t ttf_font_18;
extern lv_style_t ttf_font_16;
extern bool is_video_mode;

#define ALBUM_DIR_PHOTO_ID FILEMNG_DIR_PHOTO
#define ALBUM_DIR_MOVIE_ID FILEMNG_DIR_NORMAL

// 获取所有文件名
void get_all_filenames(char ***filenames, int *total_files)
{
    // 初始化输出参数
    *filenames   = NULL;
    *total_files = 0;

    if(filenames == NULL || total_files == NULL) {
        MLOG_ERR("参数不能为NULL\n");
        return;
    }

    if(ui_common_cardstatus()) {
        if(!is_video_mode) {
            *total_files = FILEMNG_GetDirFileCnt(0, ALBUM_DIR_PHOTO_ID);
        } else {
            *total_files = FILEMNG_GetDirFileCnt(0, ALBUM_DIR_MOVIE_ID);
        }

        if(*total_files <= 0) {
            MLOG_DBG("目录中没有文件\n");
            return;
        }

        // 分配文件名数组
        *filenames = (char **)malloc(sizeof(char *) * (*total_files));
        if(*filenames == NULL) {
            MLOG_ERR("内存分配失败\n");
            *total_files = 0;
            return;
        }
        memset(*filenames, 0, sizeof(char *) * (*total_files));

        // 获取所有文件名
        for(int i = 0; i < *total_files; i++) {
            char temp_filename[FILEMNG_PATH_MAX_LEN];
            int ret = -1;

            if(!is_video_mode) {
                ret = FILEMNG_GetFileNameByFileInx(0, ALBUM_DIR_PHOTO_ID, i, &temp_filename, 1);
            } else {
                ret = FILEMNG_GetFileNameByFileInx(0, ALBUM_DIR_MOVIE_ID, i, &temp_filename, 1);
            }

            if(ret == 0) {
                (*filenames)[i] = strdup(temp_filename);
                if((*filenames)[i] == NULL) {
                    MLOG_ERR("字符串复制失败\n");
                    // 如果分配失败，清理已分配的内存并返回
                    for(int j = 0; j < i; j++) {
                        free((*filenames)[j]);
                    }
                    free(*filenames);
                    *filenames   = NULL;
                    *total_files = 0;
                    return;
                }
            } else {
                MLOG_ERR("获取文件名失败，索引: %d\n", i);
                (*filenames)[i] = NULL;
            }
        }
    } else {
        MLOG_DBG("存储卡未就绪\n");
    }
}

// 清理分配的内存
void clean_all_malloc(char **filenames, int total_files)
{
    if(filenames == NULL) {
        MLOG_DBG("文件名数组为空，无需清理\n");
        return;
    }

    for(int i = 0; i < total_files; i++) {
        if(filenames[i] != NULL) {
            free(filenames[i]);
            filenames[i] = NULL;
        }
    }
    free(filenames);

    MLOG_DBG("清理了 %d 个文件名\n", total_files);
}

// 执行照片删除操作
static void perform_photo_deletion(void)
{
    // 提取文件名
    const char *filename = delete_filename;
    char *dot = NULL;
    MLOG_DBG("开始删除照片: %s\n", filename);
    dot       = strrchr(filename, '.');
    // 构建缩略图路径
    char thumbnail_path_small[100] = {0};
    char thumbnail_path_large[100] = {0};
    char thumbnail_path_real[100]  = {0};

    get_thumbnail_path(filename, thumbnail_path_small, sizeof(thumbnail_path_small),
                       strcmp(dot, ".jpg") ? VIDEO_SMALL_PATH : PHOTO_SMALL_PATH);

    get_thumbnail_path(filename, thumbnail_path_large, sizeof(thumbnail_path_large),
                       strcmp(dot, ".jpg") ? VIDEO_LARGE_PATH : PHOTO_LARGE_PATH);

    get_thumbnail_path(filename, thumbnail_path_real, sizeof(thumbnail_path_real),
                       strcmp(dot, ".jpg") ? VIDEO_REL_PATH : PHOTO_REL_PATH);

    char *real_path_small = strchr(thumbnail_path_small, '/');
    char *real_path_large = strchr(thumbnail_path_large, '/');
    char *real_path_real  = strchr(thumbnail_path_real, '/');

    if(real_path_small == NULL) real_path_small = thumbnail_path_small;
    if(real_path_large == NULL) real_path_large = thumbnail_path_large;
    if(real_path_real == NULL) real_path_real = thumbnail_path_real;

    // 删除所有相关文件
    int delete_count = 0;
    // MLOG_DBG("小缩略图: %s\n", real_path_small);
    // MLOG_DBG("大缩略图: %s\n", real_path_large);
    // 删除小缩略图
    if(access(real_path_small, F_OK) == 0) {
        if(remove(real_path_small) == 0) {
            MLOG_DBG("删除小缩略图成功: %s\n", real_path_small);
            delete_count++;
        } else {
            MLOG_ERR("删除小缩略图失败: %s\n", real_path_small);
        }
    }

    // 删除大缩略图
    if(access(real_path_large, F_OK) == 0) {
        if(remove(real_path_large) == 0) {
            MLOG_DBG("删除大缩略图成功: %s\n", real_path_large);
            delete_count++;
        } else {
            MLOG_ERR("删除大缩略图失败: %s\n", real_path_large);
        }
    }

    // 删除主文件
    if(access(real_path_real, F_OK) == 0) {
        if(remove(real_path_real) == 0) {
            MLOG_DBG("删除主文件成功: %s\n", real_path_real);
            delete_count++;
            // 从文件管理器中移除文件记录
            FILEMNG_DelFile(0, real_path_real);

        } else {
            MLOG_ERR("删除主文件失败: %s\n", real_path_real);
        }
    }
    // 清理内存
    MLOG_DBG("删除操作完成，成功删除 %d 个文件\n", delete_count);
}

// 确认删除回调
static void simple_confirm_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("确认删除照片\n");

    if(code == LV_EVENT_CLICKED) {
        perform_photo_deletion(); // 删除
        // 关闭弹窗
        if(simple_dialog != NULL && lv_obj_is_valid(simple_dialog)) {
            lv_obj_del(simple_dialog);
            simple_dialog = NULL;
            if(make_sure_cb != NULL) {
                make_sure_cb();
            }
        }
    }
}

// 取消删除回调
static void simple_cancel_cb(lv_event_t *e)
{
    MLOG_DBG("取消删除操作\n");
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        // 关闭弹窗
        if(simple_dialog != NULL && lv_obj_is_valid(simple_dialog)) {
            lv_obj_del(simple_dialog);
            simple_dialog = NULL;
            make_sure_cb  = false;
        }
    }
}

// 添加事件回调到 simple_dialog
static void simple_dialog_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    // 拦截右滑等手势事件，阻止其冒泡到父控件
    if(code == LV_EVENT_GESTURE) {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        // 如果是右滑事件，停止传播
        if(dir == LV_DIR_RIGHT) {
            if(simple_dialog != NULL && lv_obj_is_valid(simple_dialog)) {
                lv_obj_del(simple_dialog);
                simple_dialog = NULL;
                make_sure_cb  = false;
            }
        }
    }
    return;
}

// 创建简单删除确认弹窗
void create_simple_delete_dialog(const char *file_name)
{
    make_sure_cb                          = NULL;
    static char current_display_path[100] = {0};
    char *dot = NULL;

    // 如果弹窗已存在，先删除
    if(simple_dialog != NULL && lv_obj_is_valid(simple_dialog)) {
        lv_obj_del(simple_dialog);
        simple_dialog = NULL;
    }
    if(file_name != NULL) {
        dot       = strrchr(file_name, '.');
        get_thumbnail_path(file_name, current_display_path, sizeof(current_display_path),
                           strcmp(dot, ".jpg") ? VIDEO_LARGE_PATH : PHOTO_LARGE_PATH);
        strncpy(delete_filename, file_name, sizeof(delete_filename));
    } else {
        static char **local_filenames = NULL;
        static int local_total_files  = 0;
        char *dot = NULL;
        // 获取文件名
        get_all_filenames(&local_filenames, &local_total_files);
        if(local_total_files == 0) return;
        dot       = strrchr(local_filenames[0], '.');
        strncpy(delete_filename, local_filenames[0], sizeof(delete_filename));
        get_thumbnail_path(local_filenames[0], current_display_path, sizeof(current_display_path),
                           strcmp(dot, ".jpg") ? VIDEO_LARGE_PATH : PHOTO_LARGE_PATH);
        // 使用完毕后清理内存
        clean_all_malloc(local_filenames, local_total_files);
    }
    MLOG_DBG("显示路径: %s   delete_filename:%s\n", current_display_path, delete_filename);

    // 创建弹窗容器
    simple_dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(simple_dialog, 640, 360);
    lv_obj_set_style_bg_opa(simple_dialog, 255, LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明度 0~255，255 完全不透明
    lv_obj_set_style_bg_color(simple_dialog, lv_color_hex(0), 0);
    lv_obj_set_style_radius(simple_dialog, 0, 0);
    lv_obj_set_style_pad_all(simple_dialog, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_src(simple_dialog, current_display_path, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 关键设置：确保对话框可以接收所有事件
    lv_obj_clear_flag(simple_dialog, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(simple_dialog, simple_dialog_event_cb, LV_EVENT_ALL, NULL);

    // 创建弹窗容器
    lv_obj_t *simple_dialog_scrll = lv_obj_create(simple_dialog);
    lv_obj_set_size(simple_dialog_scrll, 280, 150);
    lv_obj_center(simple_dialog_scrll);
    lv_obj_set_style_bg_color(simple_dialog_scrll, lv_color_hex(0x2D2D2D), 0);
    lv_obj_set_style_radius(simple_dialog_scrll, 10, 0);
    lv_obj_set_style_pad_all(simple_dialog_scrll, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 提示文本
    lv_obj_t *label = lv_label_create(simple_dialog_scrll);
    if (!is_video_mode)
        lv_label_set_text(label, str_language_confirm_delete_recent_photo[get_curr_language()]);
    else
        lv_label_set_text(label, str_language_confirm_delete_recent_video[get_curr_language()]);
    if (file_name != NULL) {
        lv_label_set_text(label, strcmp(dot, ".mov") ? str_language_confirm_delete_this_photo[get_curr_language()] : str_language_confirm_delete_this_video[get_curr_language()]);
    }

    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_add_style(label, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);

    // 确认按钮
    lv_obj_t *confirm_btn = lv_btn_create(simple_dialog_scrll);
    lv_obj_set_size(confirm_btn, 100, 35);
    lv_obj_align(confirm_btn, LV_ALIGN_BOTTOM_LEFT, 30, -20);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0xFF5555), 0);
    lv_obj_set_style_pad_all(confirm_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, str_language_confirm[get_curr_language()]);
    lv_obj_add_style(confirm_label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(confirm_label);
    lv_obj_add_event_cb(confirm_btn, simple_confirm_cb, LV_EVENT_CLICKED, NULL);

    // 取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(simple_dialog_scrll);
    lv_obj_set_size(cancel_btn, 100, 35);
    lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -30, -20);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0x555555), 0);
    lv_obj_set_style_pad_all(cancel_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, str_language_cancel[get_curr_language()]);
    lv_obj_add_style(cancel_label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(cancel_label);
    lv_obj_add_event_cb(cancel_btn, simple_cancel_cb, LV_EVENT_CLICKED, NULL);
}

void sure_delete_register_callback(sure_delete_callback_t callback)
{
    make_sure_cb = callback;
    MLOG_DBG("注册确认删除回调\n");
}
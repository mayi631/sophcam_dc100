/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "page_all.h"
#include "custom.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "config.h"
#include "ui_common.h"
#include "indev.h"
#include "linux/input.h"
#include "common/extract_thumbnail.h"
#include "filemng.h"

// 从AI预览返回函数声明
extern void return_to_preview_with_image(const char *image_path);

lv_obj_t *obj_Aibum_s; //底层容器

extern int g_return_page_index ;    // 返回时需要跳转的页面索引
extern int g_return_focus_index;  // 返回时的焦点索引
// 从AI预览进入相册标志
static bool g_from_ai_preview = false;
// 设置从AI预览进入标志
void set_from_ai_preview(bool from_ai);

static lv_obj_t *btn_delete_s;
static lv_obj_t *btn_delete_all_s;
static lv_obj_t *btn_choose_s;
static lv_obj_t *btn_cancel_s;
static lv_obj_t *btn_phalbum_s;//相册按钮
static lv_obj_t *btn_vialbum_s;//视频按钮

static lv_obj_t *cont_album_grid_s;
static lv_obj_t *scrollbar_slider_s;

uint8_t g_last_scr_mode = 0;

#define GRID_ITEM_WIDTH 200
#define GRID_ITEM_HEIGHT 140

#define THUMBNAIL_WIDTH 640
#define THUMBNAIL_HEIGHT 360

#define ALBUM_DIR_PHOTO_ID FILEMNG_DIR_PHOTO
#define ALBUM_DIR_MOVIE_ID FILEMNG_DIR_NORMAL

static bool g_is_photo_mode = true;//是否是照片模式

char current_image_path[256];
// 全局变量用于分页加载
static int g_current_loaded_count = 0;
int g_total_media_files = 0;
char **g_all_filenames = NULL;
static lv_obj_t *g_current_parent = NULL;
static lv_ui_t *g_current_ui = NULL;
int g_current_page = 0;  // 当前页面索引
int g_images_per_page = 6;  // 每页显示的图片数量
int g_total_pages = 0;
int g_cam_id = 0; // 当前相机ID,第一路
int g_album_image_index = 0; // 当前焦点图片在所有图片中的索引
static int g_curr_page_max_index   = 0; // 当前页面的最大id数
static uint8_t g_album_focus_index = 0; // 当前页面的焦点id
// 全局变量用于记录上一次滚动位置
static lv_coord_t g_last_scroll_y = 0;
// 防止重复触发的标志
static bool g_is_loading_page = false;

// static lv_obj_t *g_slider = NULL;
static int g_slider_height = 0;

static lv_point_t g_touch_start_point = {0, 0};  // 触摸起始点
static lv_point_t g_touch_end_point = {0, 0};    // 触摸结束点
static uint32_t g_touch_start_time = 0;          // 触摸开始时间
static bool g_is_touching = false;                // 是否正在触摸
static bool g_is_scrolling = false;               // 是否正在滚动
#define TOUCH_THRESHOLD 10                        // 触摸移动阈值（像素）
#define TOUCH_TIME_THRESHOLD 200                  // 触摸时间阈值（毫秒）

void create_album_grid(lv_obj_t *parent, lv_ui_t *ui);
void key_ok_enter(void);
static void touch_event_handler(lv_event_t *e);
static bool is_valid_click(void);
void album_return_cb(void);
static void screen_PhotoAlbum_btn_delete_all_event_handler(lv_event_t *e);
// 2. 为容器con2中的照片添加可选状态
void mark_photos_selectable(lv_obj_t *parent)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    MLOG_DBG("mark_photos_selectable: 找到 %d 个子对象\n", child_cnt);

    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        // 现在检查的是容器，不是图片控件
        if(lv_obj_check_type(child, &lv_obj_class)) {
            MLOG_DBG("设置图片容器 %d 为可选状态\n", i);

            // 先清除可能存在的选中状态
            lv_obj_clear_state(child, LV_STATE_CHECKED);

            // 设置选中样式（红色边框）- 使用内联样式设置
            lv_obj_set_style_border_color(child, lv_color_hex(0x035edb), LV_STATE_CHECKED);
            lv_obj_set_style_border_width(child, 3, LV_STATE_CHECKED);
            lv_obj_set_style_border_side(child, LV_BORDER_SIDE_FULL, LV_STATE_CHECKED);

            // 将容器标记为可选（使用用户数据，但不覆盖文件名）
            // 注意：这里不设置用户数据，因为需要保留文件名信息
            // 可选状态通过其他方式（如样式）来标识
            if(!lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_t *select_img = lv_obj_get_child(child, 1);
                if(select_img != NULL && lv_obj_has_flag(select_img, LV_OBJ_FLAG_HIDDEN))
                    lv_obj_clear_flag(select_img, LV_OBJ_FLAG_HIDDEN);
            }

            MLOG_DBG("图片容器 %d 样式设置完成，初始状态：未选中\n", i);
        }
    }
    MLOG_DBG("mark_photos_selectable: 完成设置\n");
}
void mark_photos_unselectable(lv_obj_t *parent)
{
    uint32_t child_cnt = lv_obj_get_child_cnt(parent);
    MLOG_DBG("mark_photos_unselectable: 找到 %d 个子对象\n", child_cnt);

    for(uint32_t i = 0; i < child_cnt; i++) {
        lv_obj_t *child = lv_obj_get_child(parent, i);
        // 现在检查的是容器，不是图片控件
        if(lv_obj_check_type(child, &lv_obj_class)) {
            MLOG_DBG("清除图片容器 %d 的可选状态\n", i);
            // 1. 移除可选标志
            // lv_obj_clear_flag(child, LV_OBJ_FLAG_CLICKABLE);
            // lv_obj_clear_flag(child, LV_OBJ_FLAG_CHECKABLE);

            // 2. 清除选中状态
            lv_obj_clear_state(child, LV_STATE_CHECKED);

            // 3. 移除选中样式
            lv_obj_remove_style(child, NULL, LV_STATE_CHECKED);

            if(!lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_t *select_img = lv_obj_get_child(child, 1);
                if(select_img != NULL && !lv_obj_has_flag(select_img, LV_OBJ_FLAG_HIDDEN)) {
                    // 确保边框回到默认状态
                    lv_obj_set_style_border_color(child, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
                    lv_obj_set_style_border_width(child, 1, LV_PART_MAIN);
                    lv_obj_add_flag(select_img, LV_OBJ_FLAG_HIDDEN);
                    show_image(select_img, "未选中.png");
                }
            }
            // 4. 保持原始用户数据（文件名）- 不修改用户数据，保持文件名信息
            // 注意：用户数据包含文件名信息，不应该被清除或修改
        }
    }
    MLOG_DBG("mark_photos_unselectable: 完成清除\n");
}

// 1. 定义媒体类型检测函数
bool is_media_file(const char *filename)
{
    const char *ext = strrchr(filename, '.');
    if(!ext) return false;

    return (strcmp(ext, ".jpg") == 0) || (strcmp(ext, ".png") == 0) || (strcmp(ext, ".mov") == 0);
}

// 文件系统回调函数
static void *fs_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    // printf("打开文件: %s\n", path);
    FILE *f = fopen(path, mode == LV_FS_MODE_RD ? "rb" : "wb");
    if(f == NULL) {
        printf("文件打开失败: %s\n", path);
        return NULL;
    }
    return f;
}

static lv_fs_res_t fs_close_cb(lv_fs_drv_t *drv, void *file_p)
{
    // printf("关闭文件\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    fclose(f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    // printf("读取文件\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    *br = fread(buf, 1, btr, f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_write_cb(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    // printf("写入文件\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    *bw = fwrite(buf, 1, btw, f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    // printf("文件定位\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    int w;
    switch(whence) {
        case LV_FS_SEEK_SET: w = SEEK_SET; break;
        case LV_FS_SEEK_CUR: w = SEEK_CUR; break;
        case LV_FS_SEEK_END: w = SEEK_END; break;
        default: return LV_FS_RES_INV_PARAM;
    }

    fseek(f, pos, w);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    // printf("获取文件位置\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    *pos_p = ftell(f);
    return LV_FS_RES_OK;
}

// 目录操作回调函数
static void *fs_dir_open_cb(lv_fs_drv_t *drv, const char *path)
{
    // printf("打开目录: %s\n", path);
    DIR *dir = opendir(path);
    if(dir == NULL) {
        printf("目录打开失败: %s\n", path);
        return NULL;
    }
    return dir;
}

static lv_fs_res_t fs_dir_read_cb(lv_fs_drv_t *drv, void *dir_p, char *fn, uint32_t fn_len)
{
    // printf("读取目录\n");
    DIR *dir = (DIR *)dir_p;
    if(dir == NULL) return LV_FS_RES_INV_PARAM;

    struct dirent *entry = readdir(dir);
    if(entry == NULL) {
        return LV_FS_RES_NOT_EX; // 没有更多文件
    }

    strncpy(fn, entry->d_name, fn_len - 1);
    fn[fn_len - 1] = '\0'; // 确保字符串以null结尾
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_dir_close_cb(lv_fs_drv_t *drv, void *dir_p)
{
    // printf("关闭目录\n");
    DIR *dir = (DIR *)dir_p;
    if(dir == NULL) return LV_FS_RES_INV_PARAM;

    closedir(dir);
    return LV_FS_RES_OK;
}

// 清理全局变量
static void cleanup_global_vars()
{
    if(g_all_filenames) {
        for(int i = 0; i < g_total_media_files; i++) {
            if(g_all_filenames[i]) {
                free(g_all_filenames[i]);
            }
        }
        free(g_all_filenames);
        g_all_filenames = NULL;
    }
    g_total_media_files = 0;
    g_current_loaded_count = 0;
    g_current_page = 0;
    g_current_parent = NULL;
    g_current_ui = NULL;
    g_last_scroll_y = 0;
    g_is_loading_page = false;
    g_total_pages = 0;
    g_slider_height = 0;
    g_cam_id = 0;
    g_album_image_index = 0;
}

// 视频点击事件处理函数
static void childvideo_event_handler(lv_event_t *e)
{
    // 防误触检查
    if(!is_valid_click()) {
        MLOG_DBG("无效点击，忽略视频点击事件\n");
        return;
    }

    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    lv_obj_t *container = lv_event_get_target(e);

    // 检查ui指针是否有效
    if(!ui) {
        MLOG_DBG("UI指针无效, 忽略视频点击事件\n");
        return;
    }

    // 检查视频容器是否仍然有效
    if(!container || !lv_obj_is_valid(container)) {
        MLOG_DBG("视频容器无效，忽略点击事件\n");
        return;
    }

    // 检查btn_delete是否存在且有效
    if(!btn_delete_s || !lv_obj_is_valid(btn_delete_s)) {
        MLOG_DBG("btn_delete无效, 忽略视频点击事件\n");
        return;
    }

    // 检查是否在选择模式下（btn_delete可见表示在选择模式）
    if(lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
        MLOG_DBG("非选择模式：跳转到视频播放页面\n");

        // 获取视频文件名并设置到全局变量中
        char *filename = NULL;
        void *user_data = NULL;

        // 安全地获取用户数据
        user_data = lv_obj_get_user_data(container);
        if(user_data) {
            filename = (char *)user_data;
            if(filename && strlen(filename) > 0) {
                // 构建完整的路径：PHOTO_ALBUM_MOVIE_PATH + 文件名，查看的是源文件
                char *real_path = strchr(PHOTO_ALBUM_MOVIE_PATH, '/');
                snprintf(current_image_path, sizeof(current_image_path), "%s%s", real_path, filename);
                for(int i = 0; i < g_total_media_files; i++) {
                    if(g_all_filenames[i] && strcmp(g_all_filenames[i], filename) == 0) {
                        MLOG_DBG("找到当前图片在文件列表中的索引: %d\n", i);
                        g_album_image_index = i;
                        break;
                    }
                }
                MLOG_DBG("设置当前源文件路径: %s\n", current_image_path);
            } else {
                MLOG_DBG("警告：用户数据中的文件名为空\n");
                strcpy(current_image_path, "");
            }
        } else {
            MLOG_DBG("警告：无法获取设置当前源文件名\n");
            strcpy(current_image_path, "");
            return;
        }
        // 获取当前点击的容器在父容器中的索引
        uint32_t child_count = lv_obj_get_child_count(g_current_parent);
        for(uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(g_current_parent, i);
            if(child == container) {
                g_album_focus_index = i;
                break;
            }
        }
        // 非选择模式：跳转到视频播放页面
        ui_load_scr_animation(ui, &obj_AibumVid_s, 1, NULL, setup_scr_screen_PhotoAlbumVid, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                              false, true);
    } else {
        // 检查是否从AI预览进入
        if (g_from_ai_preview) {
            // AI模式进入，视频文件不支持识别，忽略或提示
            MLOG_DBG("AI预览只支持照片识别，忽略视频文件\n");
            return;
        }
        
        // 选择模式：切换选中状态
        lv_obj_t *select_img = lv_obj_get_child(container,1);
        if(lv_obj_has_state(container, LV_STATE_CHECKED)) {
            lv_obj_clear_state(container, LV_STATE_CHECKED);
            MLOG_DBG("视频取消选中\n");
            show_image(select_img,"未选中.png");
        } else {
            lv_obj_add_state(container, LV_STATE_CHECKED);
            MLOG_DBG("视频选中\n");
            show_image(select_img,"选中.png");
        }
    }
}

// 图片点击事件处理函数
static void child_event_handler(lv_event_t *e)
{
    // 防误触检查
    if(!is_valid_click()) {
        MLOG_DBG("无效点击，忽略图片点击事件\n");
        return;
    }
    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    lv_obj_t *container = lv_event_get_target(e);

    MLOG_DBG("图片点击事件触发\n");

    // 检查ui指针是否有效
    if(!ui) {
        MLOG_DBG("UI指针无效, 忽略点击事件\n");
        return;
    }

    // 检查容器是否仍然有效
    if(!container || !lv_obj_is_valid(container)) {
        MLOG_DBG("图片容器无效，忽略点击事件\n");
        return;
    }

    // 检查控件类型是否正确（现在是容器）
    if(!lv_obj_check_type(container, &lv_obj_class)) {
        MLOG_DBG("控件类型不是容器，忽略点击事件\n");
        return;
    }

    // 检查是否在选择模式下（btn_delete可见表示在选择模式）
    if(lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
        MLOG_DBG("非选择模式：跳转到图片查看页面\n");

        // 获取图片文件名并设置到全局变量中
        char *filename = NULL;
        void *user_data = NULL;

        // 安全地获取用户数据
        user_data = lv_obj_get_user_data(container);
        if(user_data) {
            filename = (char *)user_data;
            if(filename && strlen(filename) > 0) {
                // 构建完整的图片路径：PHOTO_ALBUM_IMAGE_PATH_L + 文件名，查看的是大缩略图
                snprintf(current_image_path, sizeof(current_image_path), "%s%s", PHOTO_ALBUM_IMAGE_PATH_L, filename);
                for(int i = 0; i < g_total_media_files; i++) {
                    if(g_all_filenames[i] && strcmp(g_all_filenames[i], filename) == 0) {
                        MLOG_DBG("找到当前图片在文件列表中的索引: %d\n", i);
                        g_album_image_index = i;
                        break;
                    }
                }
                MLOG_DBG("设置当前图片路径: %s\n", current_image_path);
            } else {
                MLOG_DBG("警告：用户数据中的文件名为空\n");
                strcpy(current_image_path, "");
            }
        } else {
            MLOG_DBG("警告：无法获取图片文件名\n");
            strcpy(current_image_path, "");
            return;
        }
        // 获取当前点击的容器在父容器中的索引
        uint32_t child_count = lv_obj_get_child_count(g_current_parent);
        for(uint32_t i = 0; i < child_count; i++) {
            lv_obj_t *child = lv_obj_get_child(g_current_parent, i);
            if(child == container) {
                g_album_focus_index = i;
                break;
            }
        }

        // 检查是否从AI预览进入
        if (g_from_ai_preview) {
            // AI模式进入，点击选择后立即返回
            // 获取文件名
            char* filename = NULL;
            void* user_data = NULL;
            user_data = lv_obj_get_user_data(container);
            if (user_data) {
                filename = (char*)user_data;
                if (filename && strlen(filename) > 0) {
                    // 只支持照片识别
                    if (strstr(filename, ".jpg") || strstr(filename, ".png")) {
                        // 构建大缩略图路径（带A:前缀）
                        g_from_ai_preview = false;
                        MESSAGE_S Msg = { 0 };
                        Msg.topic = EVENT_MODEMNG_MODESWITCH;
                        Msg.arg1 = WORK_MODE_PHOTO;
                        MODEMNG_SendMessage(&Msg);

                        char large_thumb_path[256];
                        snprintf(large_thumb_path, sizeof(large_thumb_path), "%s%s", PHOTO_ALBUM_IMAGE_PATH_L, filename);
                        MLOG_DBG("从AI预览点击选择图片: %s\n", large_thumb_path);
                        // 返回预览界面并更新图片
                        return_to_preview_with_image(large_thumb_path);
                        return;
                    } else {
                        MLOG_DBG("AI预览只支持照片识别，忽略视频文件\n");
                    }
                }
            }
            return;
        }

        // 非选择模式：跳转到图片查看页面
        ui_load_scr_animation(ui, &obj_AibumPic_s, 1, NULL,
                              setup_scr_screen_PhotoAlbumPic, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
    } else {
        MLOG_DBG("选择模式：检查图片是否可选\n");

        // 检查图片是否被标记为可选
        // 在选择模式下，所有图片都是可选的，不需要特殊标记
        lv_obj_t *select_img = lv_obj_get_child(container,1);
        if(lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE)) {
            MLOG_DBG("图片可选，切换选中状态\n");
            // 选择模式：切换选中状态
            if(lv_obj_has_state(container, LV_STATE_CHECKED)) {
                lv_obj_clear_state(container, LV_STATE_CHECKED);
                MLOG_DBG("图片取消选中\n");
                show_image(select_img,"未选中.png");
            } else {
                lv_obj_add_state(container, LV_STATE_CHECKED);
                MLOG_DBG("图片选中\n");
                show_image(select_img,"选中.png");
            }

            // 强制刷新容器显示
            lv_obj_invalidate(container);

            // 调试：检查当前状态
            MLOG_DBG("图片当前状态: %s\n", lv_obj_has_state(container, LV_STATE_CHECKED) ? "已选中" : "未选中");
        } else {
            MLOG_DBG("图片不可选，跳转到图片查看页面\n");
            // 如果图片不可选，跳转到图片查看页面
            ui_load_scr_animation(ui, &obj_AibumPic_s, 1, NULL,
                              setup_scr_screen_PhotoAlbumPic, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
        }
    }
}

// 加载指定页面的图片
static void load_page(int page_index)
{
    if(!g_current_parent || !g_current_ui || !g_all_filenames) {
        return;
    }

    // 计算指定页面的起始和结束索引
    int start_index = page_index * g_images_per_page;
    int end_index = start_index + g_images_per_page;

    // 确保不超过总文件数
    if(end_index > g_total_media_files) {
        end_index = g_total_media_files;
    }

    // 如果起始索引超出范围或为负数，重新开始
    if(start_index >= g_total_media_files || start_index < 0) {
        page_index = 0;
        start_index = 0;
        end_index = g_images_per_page;
        if(end_index > g_total_media_files) {
            end_index = g_total_media_files;
        }
    }

    // 清除当前页面
    lv_obj_clean(g_current_parent);

    // 加载指定页面的图片
    int load_count = 0;
    for(int i = start_index; i < end_index; i++) {
        char *filename = g_all_filenames[i];
        // 跳过已删除的文件（NULL指针）
        if(!filename) {
            continue;
        }

        if(is_media_file(filename)) {
            MLOG_DBG("创建图片容器: %s\n", filename);
            // 创建图片容器
            lv_obj_t *img_container = lv_obj_create(g_current_parent);
            lv_obj_set_size(img_container, GRID_ITEM_WIDTH, GRID_ITEM_HEIGHT);

            // 设置容器样式
            lv_obj_set_style_bg_opa(img_container, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_color(img_container, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
            lv_obj_set_style_border_width(img_container, 1, LV_PART_MAIN);
            lv_obj_set_style_border_width(img_container, 1, LV_PART_MAIN);
            lv_obj_set_style_pad_all(img_container, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(img_container, 0, LV_PART_MAIN);
            if(i-start_index == g_album_focus_index)
            {
                lv_obj_set_style_border_color(img_container, lv_color_hex(0x1afa29), LV_STATE_CHECKED);
                lv_obj_set_style_border_width(img_container, 3, LV_STATE_CHECKED);
                lv_obj_set_style_border_side(img_container, LV_BORDER_SIDE_FULL, LV_STATE_CHECKED);
                lv_obj_add_state(img_container, LV_STATE_CHECKED);
            }
            // 确保容器是可点击的
            lv_obj_add_flag(img_container, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(img_container, LV_OBJ_FLAG_SCROLLABLE);

            // 创建图片控件
            lv_obj_t *img = lv_img_create(img_container);
            lv_obj_set_size(img, GRID_ITEM_WIDTH, GRID_ITEM_HEIGHT);
            lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);


            lv_obj_t *select_img = lv_img_create(img_container);
            lv_obj_set_size(select_img, 32,32);
            lv_obj_set_style_bg_opa(select_img, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_align(select_img, LV_ALIGN_TOP_LEFT, 2, 0);
            show_image(select_img,"未选中.png");
            lv_obj_set_style_image_recolor(select_img, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_image_recolor_opa(select_img, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_add_flag(select_img, LV_OBJ_FLAG_HIDDEN);

            // 设置内容
            char fullpath[128];
            snprintf(fullpath, sizeof(fullpath), PHOTO_ALBUM_IMAGE_PATH "%s", filename);

            if(strstr(filename, ".mov")) {
                // 使用统一的路径获取函数，并复制到本地缓冲区
                char small_thumb_path[100];
                char real_thumb_path[100];
                get_thumbnail_path(filename, small_thumb_path, sizeof(small_thumb_path), VIDEO_SMALL_PATH);
                get_thumbnail_path(filename, real_thumb_path, sizeof(real_thumb_path), VIDEO_REL_PATH);

                // 直接使用小缩略图在列表中显示（get_display_path内部已处理缩略图提取）
                lv_img_set_src(img, small_thumb_path);

                // 添加视频时长标签
                lv_obj_t *duration_label = lv_label_create(img_container);
                // 获取实际视频时长
                char *rel_path = strchr(real_thumb_path, '/');
                int seconds    = get_video_duration(rel_path);
                // 计算时分秒
                int hours   = seconds / 3600;        // 1小时 = 3600秒
                int minutes = (seconds % 3600) / 60; // 剩余秒数转换分钟
                int secs    = seconds % 60;          // 最终剩余秒数
                lv_label_set_text_fmt(duration_label, "%02d:%02d:%02d", hours, minutes, secs);

                lv_obj_set_style_text_color(duration_label, lv_color_white(), 0);
                lv_obj_set_style_bg_opa(duration_label, LV_OPA_50, 0);
                lv_obj_set_style_bg_color(duration_label, lv_color_black(), 0);
                lv_obj_set_style_pad_all(duration_label, 2, 0);
                lv_obj_align(duration_label, LV_ALIGN_BOTTOM_LEFT, 5, -5);

                // 保存大缩略图路径到用户数据，用于详情视图
                char *large_thumb_copy = strdup(filename);
                lv_obj_set_user_data(img_container, large_thumb_copy);

                // 添加视频点击事件
                lv_obj_add_event_cb(img_container, touch_event_handler, LV_EVENT_ALL, g_current_ui);
            } else {
                // 使用统一的路径获取函数，并复制到本地缓冲区
                char thumbnail_path_small[100];
                get_thumbnail_path(filename, thumbnail_path_small, sizeof(thumbnail_path_small), PHOTO_SMALL_PATH);

                if(strstr(filename, ".jpg")) {
                    // 使用小缩略图在列表中显示（get_display_path内部已处理缩略图提取）
                    lv_img_set_src(img, thumbnail_path_small);
                } else {
                    lv_img_set_src(img, fullpath);
                }

                // 将文件名存储到容器的用户数据中
                char *filename_copy = strdup(filename);
                lv_obj_set_user_data(img_container, filename_copy);

                // 添加图片点击事件到容器
                lv_obj_add_event_cb(img_container, touch_event_handler, LV_EVENT_ALL, g_current_ui);
            }

            load_count++;
        }
    }

    // 更新当前页面索引
    g_current_page = page_index;
    g_current_loaded_count = end_index;

    // 更新滑动条位置（添加安全检查）
    if(scrollbar_slider_s && lv_obj_is_valid(scrollbar_slider_s)) {
        lv_obj_set_pos(scrollbar_slider_s, 625, 60 + g_slider_height * g_current_page);
    }

    // 设置加载标志，避免触发滚动事件
    g_is_loading_page = true;

    // 如果是第一页，确保滚动位置在顶部
    if(page_index == 0) {
        lv_obj_scroll_to(g_current_parent, 0, 0, LV_ANIM_OFF);
    }

    // 如果是最后一页，确保滚动位置在顶部
    if(page_index == g_total_pages - 1) {
        lv_obj_scroll_to(g_current_parent, 0, 0, LV_ANIM_OFF);

        // 如果最后一页内容不足，添加空白区域以确保可以滚动
        int content_height = lv_obj_get_content_height(g_current_parent);
        int page_height = lv_obj_get_height(g_current_parent);
        if(content_height < page_height && load_count < g_images_per_page) {
            // 计算需要添加的空白高度，确保有足够的滚动空间
            // 对于只有一张照片的情况，添加更多的空白
            int min_needed_height = page_height + 300; // 确保至少有足够的滚动空间
            int needed_height = (min_needed_height > content_height) ? (min_needed_height - content_height) : 200;

            // 添加多个空白容器来确保有足够的滚动空间
            int spacer_count = (needed_height + GRID_ITEM_HEIGHT - 1) / GRID_ITEM_HEIGHT;
            for(int i = 0; i < spacer_count; i++) {
                lv_obj_t *spacer = lv_obj_create(g_current_parent);
                lv_obj_set_size(spacer, GRID_ITEM_WIDTH, GRID_ITEM_HEIGHT);
                lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, LV_PART_MAIN);
                lv_obj_set_style_border_width(spacer, 0, LV_PART_MAIN);
                lv_obj_set_style_pad_all(spacer, 0, LV_PART_MAIN);
            }
            MLOG_DBG("最后一页内容不足，添加 %d 个空白区域以支持滚动 (content_height=%d, page_height=%d)\n",
                   spacer_count, content_height, page_height);
        }
    }
    // 计算当前页的实际项目数
    int items_in_current_page = end_index - start_index;
    // 计算当前页的最大索引（0-based）
    g_curr_page_max_index = items_in_current_page;
    if(g_curr_page_max_index < 0) {
        g_curr_page_max_index = 0;
    }
    MLOG_DBG("当前页项目数: %d, 最大索引: %d\n", items_in_current_page, g_curr_page_max_index);
    // 重置加载标志
    g_is_loading_page = false;

    MLOG_DBG("已加载第 %d 页，共 %d 张图片，总共 %d 张\n", g_current_page, load_count, g_total_media_files);
}

// 加载上一页的图片
static void load_prev_page()
{
    // 计算上一页的索引
    int prev_page = g_current_page - 1;

    load_page(prev_page);

    if(!lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
        mark_photos_selectable(cont_album_grid_s);
    }
}

// 加载下一页的图片
static void load_next_page()
{
    // 计算下一页的索引
    int next_page = g_current_page + 1;

    load_page(next_page);

    if(!lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
        mark_photos_selectable(cont_album_grid_s);
    }
}

// 滚动事件处理函数
static void scroll_event_handler(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_SCROLL) {
        // 如果正在加载页面，忽略滚动事件
        if(g_is_loading_page) {
            return;
        }

        // 获取当前滚动位置
        lv_coord_t current_scroll_y = lv_obj_get_scroll_y(obj);

        // 设置滚动检测阈值
        int scroll_threshold = 1;

        // 防止在第一页时向下滚动显示空白区域
        if(g_current_page == 0 && current_scroll_y < 0) {
            // 使用无动画的滚动重置
            lv_obj_scroll_to(obj, 0, 0, LV_ANIM_OFF);
            g_last_scroll_y = 0;
            return;
        }

        // 防止在最后一页时向上滚动显示空白区域
        if(g_current_page == g_total_pages - 1 && current_scroll_y > 0) {
            // 使用无动画的滚动重置
            lv_obj_scroll_to(obj, 0, 0, LV_ANIM_OFF);
            g_last_scroll_y = 0;
            return;
        }

        // 计算滚动方向
        lv_coord_t scroll_delta = current_scroll_y - g_last_scroll_y;
        MLOG_DBG("current_scroll_y: %d, current_page: %d/%d\n",
                current_scroll_y, g_current_page + 1, g_total_pages);
        MLOG_DBG("load_count: %d\n", g_current_loaded_count);
        MLOG_DBG("g_last_scroll_y: %d\n", g_last_scroll_y);
        MLOG_DBG("scroll_delta: %d, threshold: %d\n", scroll_delta, scroll_threshold);

        // 如果向下滚动（正值）且不是最后一页
        if(scroll_delta > scroll_threshold && g_current_loaded_count < g_total_media_files) {
            // 检查是否是最后一页
            if(g_current_page < g_total_pages - 1) {
                // 设置加载标志
                g_is_loading_page = true;
                // 加载下一页的图片
                load_next_page();
                // 重置加载标志
                g_is_loading_page = false;
                // 更新上一次滚动位置，避免重复触发
                g_last_scroll_y = 0; // 页面加载后重置为0
            }
            else if(scroll_delta > scroll_threshold && g_current_page == g_total_pages - 1) {
                // 忽略向上滚动，保持在第一页
                MLOG_DBG("已在最后一页，忽略向下滚动\n");
            }
        }
        // 如果向上滚动（负值）且不是第一页
        else if(scroll_delta < -scroll_threshold && g_current_page > 0) {
            // 设置加载标志
            g_is_loading_page = true;
            // 加载上一页的图片
            load_prev_page();
            // 重置加载标志
            g_is_loading_page = false;
            // 更新上一次滚动位置，避免重复触发
            g_last_scroll_y = 0; // 页面加载后重置为0
        }
        // 如果向上滚动但已经是第一页，忽略
        else if(scroll_delta < -scroll_threshold && g_current_page == 0) {
            // 忽略向上滚动，保持在第一页
            MLOG_DBG("已在第一页，忽略向上滚动\n");
        }
        else {
            // 更新上一次滚动位置（仅在非页面切换的情况下）
            g_last_scroll_y = current_scroll_y;
        }
    }
}

void init_global_vars(lv_obj_t *parent, lv_ui_t *ui)
{
    // 如果是从AI预览进入，强制为照片模式
    if (g_from_ai_preview) {
        g_is_photo_mode = true;
    }
    
    if(ui_common_cardstatus()) {
        if(g_is_photo_mode) {
            g_total_media_files = FILEMNG_GetDirFileCnt(g_cam_id, ALBUM_DIR_PHOTO_ID);
        } else {
            g_total_media_files = FILEMNG_GetDirFileCnt(g_cam_id, ALBUM_DIR_MOVIE_ID);
        }

        // 分配文件名数组
        g_all_filenames = (char **)malloc(sizeof(char *) * g_total_media_files);
        if(g_all_filenames == NULL) {
            MLOG_DBG("内存分配失败\n");
            return;
        }
        memset(g_all_filenames, 0, sizeof(char *) * g_total_media_files);

        // 获取所有文件名
        for(int i = 0; i < g_total_media_files; i++) {
            char temp_filename[FILEMNG_PATH_MAX_LEN];
            if(g_is_photo_mode) {
                if(FILEMNG_GetFileNameByFileInx(g_cam_id, ALBUM_DIR_PHOTO_ID, i, &temp_filename, 1) == 0) {
                    g_all_filenames[i] = strdup(temp_filename);
                }
            } else {
                if(FILEMNG_GetFileNameByFileInx(g_cam_id, ALBUM_DIR_MOVIE_ID, i, &temp_filename, 1) == 0) {
                    g_all_filenames[i] = strdup(temp_filename);
                }
            }
        }

        g_total_pages = (g_total_media_files + g_images_per_page - 1) / g_images_per_page;

        MLOG_DBG("找到 %d 个媒体文件\n", g_total_media_files);
        MLOG_DBG("总页数: %d\n", g_total_pages);
    } else {
        g_total_media_files = 0;
        MLOG_DBG("PHOTO_ALBUM_IMAGE_PATH 路径不存在\n");
    }
}

static void album_key_callback(int key_code, int key_value)
{
    // 计算当前页的最大有效索引（从0开始）
    int max_valid_index = g_curr_page_max_index - 1;
    if (max_valid_index < 0) max_valid_index = 0;

    // MLOG_DBG(" 最大:%d, 当前页:%d/%d  当前焦点:%d\n", max_valid_index, g_current_page + 1, g_total_pages,g_album_focus_index);

    if(key_code == KEY_MENU && key_value == 1)
    {
        if(!lv_obj_has_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN)) {
            mark_photos_unselectable(cont_album_grid_s);
            lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_cancel_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_choose_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_choose_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_delete_all_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);
        } else {
            album_return_cb();
        }

    }
    else if(key_code == KEY_MODE && key_value == 1) {
        // 如果是AI模式进入，禁用模式切换
        if (g_from_ai_preview) {
            MLOG_DBG("AI模式进入，禁止切换视频模式\n");
            return;
        }
        // 切换照片/视频模式
        MLOG_DBG("当前图片相册模式%d\n", g_is_photo_mode);
        if(!g_is_photo_mode) {
            g_is_photo_mode = true;
            lv_ui_t *ui     = &g_ui;
            create_album_grid(cont_album_grid_s, ui);
            g_album_focus_index = 0; // 重置焦点索引
            lv_obj_set_style_border_width(btn_phalbum_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(btn_vialbum_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        } else if(g_is_photo_mode) {
            g_is_photo_mode = false;
            lv_ui_t *ui     = &g_ui;
            create_album_grid(cont_album_grid_s, ui);
            g_album_focus_index = 0; // 重置焦点索引
            lv_obj_set_style_border_width(btn_phalbum_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(btn_vialbum_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
    else if(key_code == KEY_UP && key_value == 1) {
        // 上键逻辑
        if(g_album_focus_index >= 3) {
            g_album_focus_index -= 3;
        } else if(g_current_page > 0) {
            // 需要翻到上一页
            load_prev_page();
            // 翻页后焦点设置为上一页的最后一个项目
            g_album_focus_index = (g_curr_page_max_index - 1) / 3 * 3;
            if(g_album_focus_index < 0) g_album_focus_index = 0;
        } else {
            // 已经是第一页，保持在最前面
            g_album_focus_index = 0;
        }
    }
    else if(key_code == KEY_DOWN && key_value == 1) {
        // 下键逻辑
        if(g_album_focus_index + 3 <= max_valid_index) {
            g_album_focus_index += 3;
        } else if(g_current_page < g_total_pages - 1) {
            // 需要翻到下一页
            load_next_page();
            // 翻页后焦点设置为下一页的第一个项目
            g_album_focus_index = 0;
        } else {
            // 已经是最后一页，保持在最后面
            g_album_focus_index = max_valid_index;
        }
    }
    else if(key_code == KEY_LEFT && key_value == 1) {
        // 左键逻辑
        if(g_album_focus_index > 0) {
            g_album_focus_index--;
        } else if(g_current_page > 0) {
            // 需要翻到上一页
            load_prev_page();
            // 翻页后焦点设置为上一页的最后一个项目
            g_album_focus_index = g_curr_page_max_index - 1;
            if(g_album_focus_index < 0) g_album_focus_index = 0;
        } else {
            // 已经是第一页，保持在最前面
            g_album_focus_index = 0;
        }
    }
    else if(key_code == KEY_RIGHT && key_value == 1) {
        // 右键逻辑
        if(g_album_focus_index < max_valid_index) {
            g_album_focus_index++;
        } else if(g_current_page < g_total_pages - 1) {
            // 需要翻到下一页
            load_next_page();
            // 翻页后焦点设置为下一页的第一个项目
            g_album_focus_index = 0;
        } else {
            // 已经是最后一页，保持在最后面
            g_album_focus_index = max_valid_index;
        }
    } else if(key_code == KEY_OK && key_value == 1) { // 确认键逻辑
        key_ok_enter();
        return;//跳转就直接不执行状态添加
    }

    // 更新选中状态
    if(key_value == 1 &&
       (key_code == KEY_UP || key_code == KEY_DOWN || key_code == KEY_LEFT || key_code == KEY_RIGHT)) {
        mark_photos_selectable(cont_album_grid_s);

        // 确保焦点索引在有效范围内
        if(g_album_focus_index < 0) g_album_focus_index = 0;
        if(g_album_focus_index >= g_curr_page_max_index)
            g_album_focus_index = g_curr_page_max_index - 1;

        for(uint8_t i = 0; i < g_curr_page_max_index; i++) {
            lv_obj_t *container = lv_obj_get_child(g_current_parent, i);
            if(i == g_album_focus_index) {
                lv_obj_add_state(container, LV_STATE_CHECKED);
                MLOG_DBG("选中索引 %d\n", i);
            } else {
                if(lv_obj_has_state(container, LV_STATE_CHECKED)) {
                    lv_obj_clear_state(container, LV_STATE_CHECKED);
                }
            }
            lv_obj_invalidate(container);
        }
    }
}

void init_album_cont(lv_obj_t *parent, lv_ui_t *ui)
{
    g_current_parent = parent;
    g_current_ui = ui;

    // 容器样式设置（保持与原代码一致）
    lv_obj_set_style_pad_all(parent, 1, 0);
    lv_obj_set_style_base_dir(parent, LV_BASE_DIR_LTR, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    // 设置Flex布局间距
    lv_obj_set_style_pad_row(parent, 2, 0);     // 行间距5像素
    lv_obj_set_style_pad_column(parent, 2, 0);  // 列间距5像素
    lv_obj_set_style_pad_left(parent, 14, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置滚动边界，防止过度滚动
    lv_obj_set_scroll_snap_y(parent, LV_SCROLL_SNAP_START);

    // 设置滑动条样式，确保可见
    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, LV_PART_SCROLLBAR);

    // 更新滑动条大小和位置（添加安全检查）
    g_slider_height = 300 / g_total_pages;
    if(scrollbar_slider_s && lv_obj_is_valid(scrollbar_slider_s)) {
        lv_obj_set_size(scrollbar_slider_s, 12, g_slider_height);
        lv_obj_set_pos(scrollbar_slider_s, 625, 60 + g_slider_height * g_current_page);
    }

    // 设置当前页面的按键处理器
    set_current_page_handler(album_key_callback);
}

// 2. 动态创建相册格子, 适用于从主界面到相册界面
void create_album_grid(lv_obj_t *parent, lv_ui_t *ui)
{
    // 清理之前的全局变量
    cleanup_global_vars();

    // 设置全局变量
    init_global_vars(parent, ui);
    init_album_cont(parent, ui);

    // 初始加载第一页图片
    load_page(0);

    // 添加滚动事件监听
    lv_obj_add_event_cb(parent, scroll_event_handler, LV_EVENT_SCROLL, NULL);
    // 强制启用滚动，即使内容不足
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(parent, LV_SCROLL_SNAP_START);
}

// 2. 动态创建相册格子, 适用于从图片返回到相册界面
void update_album_grid(lv_obj_t *parent, lv_ui_t *ui, int return_page, int return_focus)
{
     // 保存当前页面和焦点索引
    g_current_page = return_page;
    g_album_focus_index = return_focus;

    // 确保页面索引在有效范围内
    if (g_current_page >= g_total_pages) {
        g_current_page = g_total_pages - 1;
    }
    if (g_current_page < 0) {
        g_current_page = 0;
    }

    // 确保焦点索引在有效范围内
    if (g_album_focus_index >= g_images_per_page) {
        g_album_focus_index = g_images_per_page - 1;
    }
    if (g_album_focus_index < 0) {
        g_album_focus_index = 0;
    }
    // 保存当前页面和焦点索引
    int saved_current_page = g_current_page;
    int saved_focus_index = g_album_focus_index;

    // 清理之前的全局变量并重新初始化
    cleanup_global_vars();
    init_global_vars(parent, ui);

    // 恢复页面和焦点索引
    g_current_page = saved_current_page;
    g_album_focus_index = saved_focus_index;

    // 确保页面索引在有效范围内
    if (g_current_page >= g_total_pages) {
        g_current_page = g_total_pages - 1;
    }
    if (g_current_page < 0) {
        g_current_page = 0;
    }
    init_album_cont(parent, ui);

    // 加载保存的页面
    load_page(g_current_page);

    // 添加滚动事件监听
    lv_obj_add_event_cb(parent, scroll_event_handler, LV_EVENT_SCROLL, NULL);
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(parent, LV_SCROLL_SNAP_START);
}

static void screen_PhotoAlbum_btn_delete_s_event_handler(lv_event_t *e)
{
    // 如果是AI模式进入，禁用删除功能
    if (g_from_ai_preview) {
        MLOG_DBG("AI模式进入，禁止删除操作\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *parent   = cont_album_grid_s;
            uint32_t child_cnt = lv_obj_get_child_cnt(parent);

            lv_obj_t *selected_photos[child_cnt]; // 存储选中对象指针
            const char *selected_filenames[child_cnt]; // 存储选中文件名
            uint32_t selected_count = 0;

            // 保存删除前的状态
            int saved_current_page = g_current_page;
            int saved_focus_index = g_album_focus_index;
            MLOG_DBG("删除前状态: 页面=%d, 焦点索引=%d\n", saved_current_page, saved_focus_index);

            // 第一次遍历：收集选中项和文件名
            for(uint32_t i = 0; i < child_cnt; i++) {
                lv_obj_t *child = lv_obj_get_child(parent, i);
                if(lv_obj_has_state(child, LV_STATE_CHECKED)) {
                    selected_photos[selected_count] = child;

                    // 从用户数据中获取文件名（适用于图片和视频）
                    void *user_data = lv_obj_get_user_data(child);
                    if(user_data) {
                        selected_filenames[selected_count] = (char *)user_data;
                        MLOG_DBG("选中文件: %s\n", selected_filenames[selected_count]);
                    } else {
                        selected_filenames[selected_count] = NULL;
                        MLOG_DBG("警告：无法从用户数据获取文件名\n");
                    }
                    selected_count++;
                }
            }

            // 如果没有选中任何文件，直接返回
            if(selected_count == 0) {
                MLOG_DBG("没有选中任何文件，无需删除\n");
                return;
            }

            // 第二次遍历：批量删除（避免索引错乱）
            for(uint32_t i = 0; i < selected_count; i++) {
                // 删除文件系统中的文件
                if(selected_filenames[i]) {
                    char cmd[512];

                    if(g_is_photo_mode) {
                        // 删除小缩略图文件 - 移除 "A:" 前缀
                        const char *real_path_small = strchr(PHOTO_ALBUM_IMAGE_PATH_S, '/');
                        if(real_path_small) {
                            snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path_small, selected_filenames[i]);
                            MLOG_DBG("执行命令: %s\n", cmd);
                            system(cmd);
                        }

                        // 删除大缩略图文件 - 移除 "A:" 前缀
                        const char *real_path_large = strchr(PHOTO_ALBUM_IMAGE_PATH_L, '/');
                        if(real_path_large) {
                            snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path_large, selected_filenames[i]);
                            MLOG_DBG("执行命令: %s\n", cmd);
                            system(cmd);
                        }

                        // 删除原图文件 - 移除 "A:" 前缀
                        const char *real_path = strchr(PHOTO_ALBUM_IMAGE_PATH, '/');
                        if(real_path) {
                            char fullfilepath[FILEMNG_PATH_MAX_LEN];
                            snprintf(fullfilepath, sizeof(fullfilepath), "%s%s", real_path, selected_filenames[i]);
                            snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path, selected_filenames[i]);
                            MLOG_DBG("执行命令: %s\n", cmd);
                            system(cmd);
                            FILEMNG_DelFile(g_cam_id, fullfilepath);
                        }
                    } else if(!g_is_photo_mode) {
                        // 分割文件名和扩展名
                        char *dot = strrchr(selected_filenames[i], '.');
                        if(!dot) {
                            MLOG_ERR("No extension in filename: %s\n", selected_filenames[i]);
                            return;
                        }
                        char new_filename[256] = {0};
                        snprintf(new_filename, sizeof(new_filename), "%.*s", (int)(dot - selected_filenames[i]),
                                 selected_filenames[i]);

                        // 删除小缩略图文件 - 移除 "A:" 前缀
                        const char *real_movie_path_small = strchr(PHOTO_ALBUM_VIDEO_THUMB_PATH_S, '/');
                        if(real_movie_path_small) {
                            snprintf(cmd, sizeof(cmd), "rm -f %s%s.jpg", real_movie_path_small, new_filename);
                            MLOG_DBG("执行命令: %s\n", cmd);
                            system(cmd);
                        }

                        // 删除大缩略图文件 - 移除 "A:" 前缀
                        const char *real_movie_path_large = strchr(PHOTO_ALBUM_VIDEO_THUMB_PATH_L, '/');
                        if(real_movie_path_large) {
                            snprintf(cmd, sizeof(cmd), "rm -f %s%s.jpg", real_movie_path_large, new_filename);
                            MLOG_DBG("执行命令: %s\n", cmd);
                            system(cmd);
                        }

                        // 删除视频原文件 - 移除 "A:" 前缀
                        const char *movie0_path = strchr(PHOTO_ALBUM_MOVIE_PATH, '/');
                        if(movie0_path) {
                            char movie0filepath[FILEMNG_PATH_MAX_LEN];
                            snprintf(movie0filepath, sizeof(movie0filepath), "%s%s", movie0_path,
                                     selected_filenames[i]);
                            snprintf(cmd, sizeof(cmd), "rm -f %s%s.mov", movie0_path, new_filename);
                            MLOG_DBG("执行命令: %s\n", cmd);
                            system(cmd);
                            FILEMNG_DelFile(g_cam_id, movie0filepath);
                        }
                    }

                    // 从全局文件名数组中移除已删除的文件
                    for(int j = 0; j < g_total_media_files; j++) {
                        if(g_all_filenames[j] && strcmp(g_all_filenames[j], selected_filenames[i]) == 0) {
                            free(g_all_filenames[j]);
                            g_all_filenames[j] = NULL;
                            // 将后面的文件名前移
                            for(int k = j; k < g_total_media_files - 1; k++) {
                                g_all_filenames[k] = g_all_filenames[k + 1];
                            }
                            g_total_media_files--;
                            break;
                        }
                    }
                }

                // 删除UI控件
                if(selected_photos[i] && lv_obj_is_valid(selected_photos[i])) {
                    // 释放用户数据中的内存
                    void *user_data = lv_obj_get_user_data(selected_photos[i]);
                    if(user_data) {
                        free(user_data);
                    }
                    lv_obj_del(selected_photos[i]);
                }
            }

            // 重新计算总页数
            g_total_pages = (g_total_media_files + g_images_per_page - 1) / g_images_per_page;
            // MLOG_DBG("删除后总文件数: %d, 总页数: %d\n", g_total_media_files, g_total_pages);

            // 调整当前页面索引，确保不越界
            if (g_current_page >= g_total_pages) {
                g_current_page = (g_total_pages > 0) ? g_total_pages - 1 : 0;
                // MLOG_DBG("调整当前页面索引为: %d\n", g_current_page);
            }

            // 调整焦点索引，确保不越界
            int current_page_item_count = g_total_media_files - (g_current_page * g_images_per_page);
            if (current_page_item_count > g_images_per_page) {
                current_page_item_count = g_images_per_page;
            }
            if (g_album_focus_index >= current_page_item_count) {
                g_album_focus_index = (current_page_item_count > 0) ? current_page_item_count - 1 : 0;
                // MLOG_DBG("调整焦点索引为: %d\n", g_album_focus_index);
            }

            // 重新加载当前页面（而不是第0页）
            lv_obj_clean(parent);
            load_page(g_current_page);

            // 恢复焦点位置
            if (current_page_item_count > 0) {
                // 确保焦点索引在有效范围内
                if (g_album_focus_index < 0) g_album_focus_index = 0;
                if (g_album_focus_index >= current_page_item_count) {
                    g_album_focus_index = current_page_item_count - 1;
                }
                
                // 设置焦点状态
                for(uint8_t i = 0; i < current_page_item_count; i++) {
                    lv_obj_t *container = lv_obj_get_child(parent, i);
                    if(i == g_album_focus_index) {
                        lv_obj_add_state(container, LV_STATE_CHECKED);
                        // MLOG_DBG("恢复焦点到索引 %d\n", i);
                    } else {
                        if(lv_obj_has_state(container, LV_STATE_CHECKED)) {
                            lv_obj_clear_state(container, LV_STATE_CHECKED);
                        }
                    }
                }
            }

            // 如果当前页面没有项目了，且不是第0页，则自动跳到上一页
            if (current_page_item_count == 0 && g_current_page > 0) {
                g_current_page--;
                // MLOG_DBG("当前页无项目，自动跳到上一页: %d\n", g_current_page);
                lv_obj_clean(parent);
                load_page(g_current_page);
                
                // 设置焦点到新页面的最后一个项目
                int new_page_item_count = g_total_media_files - (g_current_page * g_images_per_page);
                if (new_page_item_count > g_images_per_page) {
                    new_page_item_count = g_images_per_page;
                }
                if (new_page_item_count > 0) {
                    g_album_focus_index = new_page_item_count - 1;
                    for(uint8_t i = 0; i < new_page_item_count; i++) {
                        lv_obj_t *container = lv_obj_get_child(parent, i);
                        if(i == g_album_focus_index) {
                            lv_obj_add_state(container, LV_STATE_CHECKED);
                            // MLOG_DBG("设置焦点到新页面最后一个项目: %d\n", i);
                        }
                    }
                }
            }

            if(g_total_pages > 0) {
                // 重新计算滑动条高度（确保均匀分布）
                g_slider_height = 420 / g_total_pages; // 总高度420像素除以总页数

                // 确保滑动条高度不会太小
                if(g_slider_height < 20) {
                    g_slider_height = 20; // 最小高度
                }
                if(g_slider_height > 420) {
                    g_slider_height = 420; // 最大高度
                }

                MLOG_DBG("重新计算滑动条: 总页数=%d, 滑动条高度=%d, 当前页=%d\n", g_total_pages, g_slider_height,
                         g_current_page);
            } else {
                g_slider_height = 0; // 没有页面时高度为0
            }

            // 更新滑动条大小和位置
            if(scrollbar_slider_s && lv_obj_is_valid(scrollbar_slider_s)) {
                // 首先更新滑动条大小
                lv_obj_set_size(scrollbar_slider_s, 12, g_slider_height);

                // 然后计算并设置滑动条位置
                int slider_y_pos = 60 + (g_slider_height * g_current_page);

                // 确保位置不会超出边界
                if(slider_y_pos < 60) {
                    slider_y_pos = 60;
                }
                if(slider_y_pos > 480) { // 480是屏幕高度
                    slider_y_pos = 480 - g_slider_height;
                }

                lv_obj_set_pos(scrollbar_slider_s, 625, slider_y_pos);

                MLOG_DBG("滑动条位置更新: x=625, y=%d, 高度=%d\n", slider_y_pos, g_slider_height);
            }

            // 退出选择模式
            lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_cancel_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_choose_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_choose_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_delete_all_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbum_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            // 重置触摸状态
            g_is_touching = false;
            g_is_scrolling = false;
            
            album_return_cb();
        }; break;
        default: break;
    }
}

static void screen_PhotoAlbum_btn_choose_event_handler(lv_event_t *e)
{
    // 如果是AI模式进入，已经自动进入多选模式，不允许再次点击
    if (g_from_ai_preview) {
        MLOG_DBG("AI模式进入，已自动进入多选模式\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_remove_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_choose_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_choose_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_CLICKABLE);
            mark_photos_selectable(cont_album_grid_s);
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbum_btn_cancel_event_handler(lv_event_t *e)
{
    // 如果是AI模式进入，不允许取消选择模式
    if (g_from_ai_preview) {
        MLOG_DBG("AI模式进入，不允许取消选择模式\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_cancel_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(btn_choose_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(btn_choose_s, LV_OBJ_FLAG_CLICKABLE);
            mark_photos_unselectable(cont_album_grid_s);

            lv_obj_remove_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_delete_all_s, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);

            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbum_btn_phalbum_event_handler(lv_event_t *e)
{
    // 如果是AI模式进入，禁用模式切换
    if (g_from_ai_preview) {
        MLOG_DBG("AI模式进入，禁止切换模式\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            if(!g_is_photo_mode) {
                g_is_photo_mode = true;
                lv_ui_t *ui = &g_ui;
                g_album_focus_index = 0;//模式切换焦点重置
                create_album_grid(cont_album_grid_s, ui);
                lv_obj_set_style_border_width(btn_phalbum_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(btn_vialbum_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbum_btn_vialbum_event_handler(lv_event_t *e)
{
    // 如果是AI模式进入，禁用模式切换
    if (g_from_ai_preview) {
        MLOG_DBG("AI模式进入，禁止切换模式\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            if(g_is_photo_mode) {
                g_is_photo_mode = false;
                lv_ui_t *ui = &g_ui;
                g_album_focus_index = 0;//模式切换焦点重置
                create_album_grid(cont_album_grid_s, ui);
                lv_obj_set_style_border_width(btn_phalbum_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(btn_vialbum_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            break;
        }
        default: break;
    }
}

void events_init_screen_PhotoAlbum(lv_ui_t *ui)
{
    lv_obj_add_event_cb(btn_delete_s, screen_PhotoAlbum_btn_delete_s_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_choose_s, screen_PhotoAlbum_btn_choose_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_cancel_s, screen_PhotoAlbum_btn_cancel_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_phalbum_s, screen_PhotoAlbum_btn_phalbum_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_vialbum_s, screen_PhotoAlbum_btn_vialbum_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(btn_delete_all_s, screen_PhotoAlbum_btn_delete_all_event_handler, LV_EVENT_CLICKED, ui);
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
                    album_return_cb();
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

static void Home_Album_obj_Create(lv_ui_t *ui)
{
    MLOG_DBG("loading page_PhotoAlbum...\n");

    // Write codes scr
    obj_Aibum_s = lv_obj_create(NULL);
    lv_obj_set_size( obj_Aibum_s , H_RES, V_RES);
    lv_obj_add_style( obj_Aibum_s , &style_common_main_bg, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(obj_Aibum_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(obj_Aibum_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_top
    lv_obj_t *cont_top = lv_obj_create(obj_Aibum_s);
    lv_obj_set_pos(cont_top, 0, 0);
    lv_obj_set_size(cont_top, H_RES, 52);
    lv_obj_set_scrollbar_mode(cont_top, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(cont_top, &style_common_cont_top, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_delete_s
    btn_delete_s = lv_button_create(cont_top);
    lv_obj_set_pos(btn_delete_s, 500, 2);
    lv_obj_set_size(btn_delete_s, 56, 48);
    lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *label_delete = lv_label_create(btn_delete_s);
    lv_label_set_text(label_delete, " " LV_SYMBOL_TRASH " ");
    lv_label_set_long_mode(label_delete, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_delete, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_delete_s, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(label_delete, LV_PCT(100));

    // Write style for btn_delete_s, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(btn_delete_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_delete_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_delete_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_delete_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_delete_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_delete_s, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(btn_delete_s, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_delete_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_delete_s, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    btn_delete_all_s = lv_button_create(cont_top);
    lv_obj_set_pos(btn_delete_all_s, 432, 2);
    lv_obj_set_size(btn_delete_all_s, 56, 48);
    // lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_opa(btn_delete_all_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(btn_delete_all_s, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_delete_all_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_delete_all_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_delete_all_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    show_image(btn_delete_all_s,"删除全部.png");
    lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);

    // Write codes btn_back
    lv_obj_t *btn_back = lv_button_create(cont_top);
    lv_obj_set_pos(btn_back, 4, 2);
    lv_obj_set_size(btn_back, 60, 48);
    lv_obj_add_event_cb(btn_back, screen_PhotoAlbum_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_style(btn_back, &style_common_btn_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "" LV_SYMBOL_LEFT "");
    lv_label_set_long_mode(label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(label_back, &style_common_label_back, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_choose_s
    btn_choose_s = lv_button_create(cont_top);
    lv_obj_set_pos(btn_choose_s, 566, 2);
    lv_obj_set_size(btn_choose_s, 60, 48);
    lv_obj_t *label_choose = lv_label_create(btn_choose_s);
    lv_label_set_text(label_choose, str_language_delete[get_curr_language()]);
    lv_obj_set_style_text_font(label_choose, get_usr_fonts(ALI_PUHUITI_FONTPATH, 22), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(label_choose, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_choose, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_choose_s, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(label_choose, LV_PCT(100));

    // Write style for btn_choose_s, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(btn_choose_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_choose_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_choose_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_choose_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_choose_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_choose_s, lv_color_hex(0XFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(btn_choose_s, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_choose_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_choose_s, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_cancel_s
    btn_cancel_s = lv_button_create(cont_top);
    lv_obj_set_pos(btn_cancel_s, 566, 2);
    lv_obj_set_size(btn_cancel_s, 60, 48);
    lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
    lv_obj_t *label_cancel = lv_label_create(btn_cancel_s);
    lv_label_set_text(label_cancel, str_language_cancel[get_curr_language()]);
    lv_label_set_long_mode(label_cancel, LV_LABEL_LONG_WRAP);
    lv_obj_align(label_cancel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(btn_cancel_s, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(label_cancel, LV_PCT(100));
    lv_obj_set_style_text_font(label_cancel, get_usr_fonts(ALI_PUHUITI_FONTPATH, 22), LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write style for btn_cancel_s, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(btn_cancel_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(btn_cancel_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(btn_cancel_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(btn_cancel_s, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(btn_cancel_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(btn_cancel_s, lv_color_hex(0XFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(btn_cancel_s, &lv_font_SourceHanSerifSC_Regular_20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(btn_cancel_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(btn_cancel_s, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 照片
    // 照片按钮
    btn_phalbum_s = lv_imagebutton_create(cont_top);
    lv_obj_align(btn_phalbum_s, LV_ALIGN_CENTER, -48, 0);
    lv_obj_set_size(btn_phalbum_s, 48, 48);
    show_image(btn_phalbum_s, "相册_1.png");
    // 设置默认状态下的边框样式
    lv_obj_set_style_border_color(btn_phalbum_s, lv_color_hex(0x1296db), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置焦点状态下的边框样式（保持原有）
    lv_obj_set_style_border_color(btn_phalbum_s, lv_color_hex(0x1296db), LV_PART_MAIN | LV_STATE_FOCUSED);

    // 视频按钮
    btn_vialbum_s = lv_imagebutton_create(cont_top);
    lv_obj_align(btn_vialbum_s, LV_ALIGN_CENTER, 48, 0);
    lv_obj_set_size(btn_vialbum_s, 48, 48);
    show_image(btn_vialbum_s, "视频_2.png");

    // 设置默认状态下的边框样式（无边框）
    lv_obj_set_style_border_width(btn_vialbum_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(btn_vialbum_s, lv_color_hex(0x1296db), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置焦点状态下的边框样式
    lv_obj_set_style_border_color(btn_vialbum_s, lv_color_hex(0x1296db), LV_PART_MAIN | LV_STATE_FOCUSED);

    if(g_is_photo_mode)
        lv_obj_set_style_border_width(btn_phalbum_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    else
        lv_obj_set_style_border_width(btn_vialbum_s, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(obj_Aibum_s);
    static lv_point_precise_t points_line[] = {{10, 52}, {640, 52}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);


    // Write codes cont_album_grid_s
    cont_album_grid_s = lv_obj_create(obj_Aibum_s);
    lv_obj_set_pos(cont_album_grid_s, 0, 56);
    lv_obj_set_size(cont_album_grid_s, 640, 304);
    lv_obj_set_scrollbar_mode(cont_album_grid_s, LV_SCROLLBAR_MODE_ON);

    // Write style for cont_album_grid_s, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(cont_album_grid_s, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(cont_album_grid_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(cont_album_grid_s, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(cont_album_grid_s, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(cont_album_grid_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(cont_album_grid_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(cont_album_grid_s, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(cont_album_grid_s, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(cont_album_grid_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(cont_album_grid_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(cont_album_grid_s, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(cont_album_grid_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(cont_album_grid_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建滚动条滑块
    scrollbar_slider_s = lv_obj_create(obj_Aibum_s);
    lv_obj_set_size(scrollbar_slider_s, 12, 60);
    lv_obj_set_pos(scrollbar_slider_s, 625, 60);
    lv_obj_set_scrollbar_mode(scrollbar_slider_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(scrollbar_slider_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scrollbar_slider_s, 1, LV_PART_MAIN | LV_STATE_DEFAULT);  // 添加边框
    lv_obj_set_style_border_color(scrollbar_slider_s, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scrollbar_slider_s, 10, LV_PART_MAIN | LV_STATE_DEFAULT);  // 圆角

    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);
    fs_drv.letter       = 'A';
    fs_drv.open_cb      = fs_open_cb;
    fs_drv.close_cb     = fs_close_cb;
    fs_drv.read_cb      = fs_read_cb;
    fs_drv.write_cb     = fs_write_cb;
    fs_drv.seek_cb      = fs_seek_cb;
    fs_drv.tell_cb      = fs_tell_cb;
    fs_drv.dir_open_cb  = fs_dir_open_cb;
    fs_drv.dir_read_cb  = fs_dir_read_cb;
    fs_drv.dir_close_cb = fs_dir_close_cb;
    /* 配置具体文件系统驱动 */
    lv_fs_drv_register(&fs_drv);

}

void Home_Album(lv_ui_t *ui)
{
    set_exit_completed(false);
    Home_Album_obj_Create(ui);

    create_album_grid(cont_album_grid_s, ui);

    // Update current screen layout.
    lv_obj_update_layout(obj_Aibum_s);

    // Init events for screen.
    events_init_screen_PhotoAlbum(ui);
    
    // 如果是从AI预览进入，应用AI模式设置
    if (g_from_ai_preview) {
        set_from_ai_preview(true);
    }
}

void Home_Album_from_Pic(lv_ui_t *ui)
{
    Home_Album_obj_Create(ui);

    update_album_grid(cont_album_grid_s, ui, g_return_page_index, g_return_focus_index);

    // Update current screen layout.
    lv_obj_update_layout(obj_Aibum_s);

    // Init events for screen.
    events_init_screen_PhotoAlbum(ui);
    
    // 如果是从AI预览进入，应用AI模式设置
    if (g_from_ai_preview) {
        set_from_ai_preview(true);
    }
}

void key_ok_enter(void)
{
    // 确认键逻辑
    MLOG_DBG("确认键按下，当前焦点索引: %d\n", g_album_focus_index);
    // 获取当前焦点对象
    if(g_album_focus_index < g_curr_page_max_index) {
        lv_obj_t *focused_container = lv_obj_get_child(g_current_parent, g_album_focus_index);
        if(focused_container && lv_obj_is_valid(focused_container)) {
            // 获取文件名
            char *filename = (char *)lv_obj_get_user_data(focused_container);
            if(filename && strlen(filename) > 0) {
                MLOG_DBG("确认键处理文件: %s\n", filename);
                
                // 检查是否从AI预览进入
                if (g_from_ai_preview) {
                    // 只支持照片识别
                    if(strstr(filename, ".jpg") || strstr(filename, ".png")) {
                        // 构建大缩略图路径（带A:前缀）
                        g_from_ai_preview = false;
                        MESSAGE_S Msg = { 0 };
                        Msg.topic = EVENT_MODEMNG_MODESWITCH;
                        Msg.arg1 = WORK_MODE_PHOTO;
                        MODEMNG_SendMessage(&Msg);
                        char large_thumb_path[256];
                        snprintf(large_thumb_path, sizeof(large_thumb_path), "%s%s", PHOTO_ALBUM_IMAGE_PATH_L, filename);
                        MLOG_DBG("从AI预览选择图片: %s\n", large_thumb_path);
                        // 返回预览界面并更新图片
                        return_to_preview_with_image(large_thumb_path);
                        return;
                    } else {
                        MLOG_DBG("AI预览只支持照片识别，忽略视频文件\n");
                    }
                    return;
                }

                // 根据文件扩展名判断是照片还是视频
                if(strstr(filename, ".jpg") || strstr(filename, ".png")) {
                    // 照片文件
                    if(g_is_photo_mode) {
                        // 构建完整的图片路径：PHOTO_ALBUM_IMAGE_PATH_L + 文件名，查看的是大缩略图
                        snprintf(current_image_path, sizeof(current_image_path), "%s%s", PHOTO_ALBUM_IMAGE_PATH_L,
                                 filename);
                        for(int i = 0; i < g_total_media_files; i++) {
                            if(g_all_filenames[i] && strcmp(g_all_filenames[i], filename) == 0) {
                                MLOG_DBG("找到当前图片在文件列表中的索引: %d\n", i);
                                g_album_image_index = i;
                                break;
                            }
                        }
                        MLOG_DBG("跳转到照片查看页面: %s\n", current_image_path);

                        // 跳转到照片查看页面
                        ui_load_scr_animation(g_current_ui, &obj_AibumPic_s, 1, NULL, setup_scr_screen_PhotoAlbumPic,
                                              LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
                        return;
                    } else {
                        MLOG_DBG("警告：照片文件但在视频模式下\n");
                    }
                } else if(strstr(filename, ".mov")) {
                    // 视频文件
                    if(!g_is_photo_mode) {
                        // 构建完整的视频路径：PHOTO_ALBUM_MOVIE_PATH + 文件名，查看的是源文件
                        char *real_path = strchr(PHOTO_ALBUM_MOVIE_PATH, '/');
                        snprintf(current_image_path, sizeof(current_image_path), "%s%s", real_path, filename);
                        MLOG_DBG("跳转到视频播放页面: %s\n", current_image_path);

                        // 跳转到视频播放页面
                        ui_load_scr_animation(g_current_ui, &obj_AibumVid_s, 1, NULL, setup_scr_screen_PhotoAlbumVid,
                                              LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
                        return;
                    }
                }
            }
        }
    }
}

// 删除所有照片文件
static void delete_all_photo_files(void)
{
    MLOG_DBG("开始删除所有照片文件\n");

    int deleted_count = 0;

    // 遍历所有照片文件
    for(int i = 0; i < g_total_media_files; i++) {
        if(g_all_filenames[i] == NULL) continue;

        char *filename = g_all_filenames[i];
        MLOG_DBG("删除照片文件: %s\n", filename);

        char cmd[512];

        // 删除小缩略图文件
        const char *real_path_small = strchr(PHOTO_ALBUM_IMAGE_PATH_S, '/');
        if(real_path_small) {
            snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path_small, filename);
            MLOG_DBG("执行命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("小缩略图删除成功\n");
            }
        }

        // 删除大缩略图文件
        const char *real_path_large = strchr(PHOTO_ALBUM_IMAGE_PATH_L, '/');
        if(real_path_large) {
            snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path_large, filename);
            MLOG_DBG("执行命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("大缩略图删除成功\n");
            }
        }

        // 删除原图文件
        const char *real_path = strchr(PHOTO_ALBUM_IMAGE_PATH, '/');
        if(real_path) {
            char fullfilepath[FILEMNG_PATH_MAX_LEN];
            snprintf(fullfilepath, sizeof(fullfilepath), "%s%s", real_path, filename);
            snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path, filename);
            MLOG_DBG("执行命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("原图文件删除成功\n");
                // 从文件管理系统中删除记录
                FILEMNG_DelFile(g_cam_id, fullfilepath);
            }
        }

        // 释放文件名内存
        free(g_all_filenames[i]);
        g_all_filenames[i] = NULL;
        deleted_count++;
    }

    MLOG_DBG("照片文件删除完成，共删除 %d 个文件\n", deleted_count);
}

// 删除所有视频文件
static void delete_all_video_files(void)
{
    MLOG_DBG("开始删除所有视频文件\n");

    int deleted_count = 0;

    // 遍历所有视频文件
    for(int i = 0; i < g_total_media_files; i++) {
        if(g_all_filenames[i] == NULL) continue;

        char *filename = g_all_filenames[i];
        MLOG_DBG("删除视频文件: %s\n", filename);

        // 分割文件名和扩展名
        char *dot = strrchr(filename, '.');
        if(!dot) {
            MLOG_ERR("文件名格式错误: %s\n", filename);
            continue;
        }

        char new_filename[256] = {0};
        snprintf(new_filename, sizeof(new_filename), "%.*s", (int)(dot - filename), filename);

        char cmd[512];

        // 删除小缩略图文件
        const char *real_movie_path_small = strchr(PHOTO_ALBUM_VIDEO_THUMB_PATH_S, '/');
        if(real_movie_path_small) {
            snprintf(cmd, sizeof(cmd), "rm -f %s%s.jpg", real_movie_path_small, new_filename);
            MLOG_DBG("执行命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("视频小缩略图删除成功\n");
            }
        }

        // 删除大缩略图文件
        const char *real_movie_path_large = strchr(PHOTO_ALBUM_VIDEO_THUMB_PATH_L, '/');
        if(real_movie_path_large) {
            snprintf(cmd, sizeof(cmd), "rm -f %s%s.jpg", real_movie_path_large, new_filename);
            MLOG_DBG("执行命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("视频大缩略图删除成功\n");
            }
        }

        // 删除视频原文件
        const char *movie0_path = strchr(PHOTO_ALBUM_MOVIE_PATH, '/');
        if(movie0_path) {
            char movie0filepath[FILEMNG_PATH_MAX_LEN];
            snprintf(movie0filepath, sizeof(movie0filepath), "%s%s", movie0_path, filename);
            snprintf(cmd, sizeof(cmd), "rm -f %s%s.mov", movie0_path, new_filename);
            MLOG_DBG("执行命令: %s\n", cmd);
            if(system(cmd) == 0) {
                MLOG_DBG("视频原文件删除成功\n");
                // 从文件管理系统中删除记录
                FILEMNG_DelFile(g_cam_id, movie0filepath);
            }
        }

        // 释放文件名内存
        free(g_all_filenames[i]);
        g_all_filenames[i] = NULL;
        deleted_count++;
    }

    MLOG_DBG("视频文件删除完成，共删除 %d 个文件\n", deleted_count);
}

// 删除所有文件后更新UI
static void update_ui_after_delete_all(void)
{
    MLOG_DBG("更新UI状态\n");

    // 重置全局变量
    g_total_media_files = 0;
    g_current_page = 0;
    g_album_focus_index = 0;
    g_total_pages = 0;

    // 清理文件名数组
    if(g_all_filenames) {
        free(g_all_filenames);
        g_all_filenames = NULL;
    }

    // 清空相册网格容器
    lv_obj_clean(cont_album_grid_s);

    // 退出选择模式
    lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btn_cancel_s, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(btn_choose_s, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(btn_choose_s, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_remove_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(btn_delete_all_s, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);

    // 标记照片为不可选状态
    mark_photos_unselectable(cont_album_grid_s);

    // 更新滑动条
    if(scrollbar_slider_s && lv_obj_is_valid(scrollbar_slider_s)) {
        lv_obj_set_size(scrollbar_slider_s, 12, 0);
        lv_obj_set_pos(scrollbar_slider_s, 625, 60);
    }

    MLOG_DBG("UI更新完成，所有文件已删除\n");
}

// 创建删除进度提示对话框
static lv_obj_t *create_delete_progress_dialog(void)
{
    // 创建模态背景
    lv_obj_t *modal_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(modal_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(modal_bg, LV_OPA_50, 0);
    lv_obj_set_style_border_width(modal_bg, 0, 0);
    lv_obj_set_style_radius(modal_bg, 0, 0);

    // 创建对话框
    lv_obj_t *dialog = lv_obj_create(modal_bg);
    lv_obj_set_size(dialog, 300, 150);
    lv_obj_center(dialog);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x2D2D2D), 0);
    lv_obj_set_style_radius(dialog, 10, 0);
    lv_obj_set_style_border_width(dialog, 2, 0);
    lv_obj_set_style_border_color(dialog, lv_color_hex(0x444444), 0);

    // 添加提示文本
    lv_obj_t *label = lv_label_create(dialog);
    lv_label_set_text(label, "正在删除文件...\n请稍候");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    return modal_bg;
}

// 关闭删除进度对话框
static void close_delete_progress_dialog(lv_obj_t *dialog)
{
    lv_obj_del(dialog);
}

// 确认删除回调
static void confirm_delete_all_cb(lv_event_t *e)
{
   lv_obj_t *confirm_modal_bg = lv_event_get_user_data(e);

    // 关闭确认对话框
    lv_obj_del(confirm_modal_bg);

    // 创建删除进度提示对话框
    lv_obj_t *progress_dialog = create_delete_progress_dialog();

    // 强制刷新UI，确保对话框显示
    lv_refr_now(NULL);

    // 执行删除操作
    if(g_is_photo_mode) {
        delete_all_photo_files();
    } else {
        delete_all_video_files();
    }

    // 关闭进度对话框
    close_delete_progress_dialog(progress_dialog);

    // 更新UI状态
    update_ui_after_delete_all();
}

// 取消删除回调
static void cancel_delete_cb(lv_event_t *e)
{
    lv_obj_t *modal_bg = lv_event_get_user_data(e);
    lv_obj_del(modal_bg);
}

// 点击模态背景关闭对话框
static void modal_background_click_cb(lv_event_t *e)
{
    lv_obj_t *modal_bg = lv_event_get_target(e);
    lv_obj_del(modal_bg);
}

// 创建删除确认对话框
static void create_delete_confirmation_dialog(int total_files)
{
    // 创建模态背景
    lv_obj_t *modal_bg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(modal_bg, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(modal_bg, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(modal_bg, LV_OPA_50, 0);
    lv_obj_set_style_border_width(modal_bg, 0, 0);
    lv_obj_set_style_radius(modal_bg, 0, 0);

    // 创建对话框容器
    lv_obj_t *dialog = lv_obj_create(modal_bg);
    lv_obj_set_size(dialog, 400, 200);
    lv_obj_center(dialog);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x2D2D2D), 0);
    lv_obj_set_style_radius(dialog, 10, 0);
    lv_obj_set_style_border_width(dialog, 2, 0);
    lv_obj_set_style_border_color(dialog, lv_color_hex(0x444444), 0);

    // 添加提示文本
    lv_obj_t *label = lv_label_create(dialog);
    lv_label_set_text_fmt(label, "确定要删除所有%d个%s吗？",
                         total_files,
                         g_is_photo_mode ? "照片" : "视频");
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    // 创建确认按钮
    lv_obj_t *confirm_btn = lv_btn_create(dialog);
    lv_obj_set_size(confirm_btn, 120, 40);
    lv_obj_align(confirm_btn, LV_ALIGN_BOTTOM_LEFT, 40, -30);
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "确认");
    lv_obj_center(confirm_label);
    lv_obj_set_style_text_font(confirm_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0xFF3B30), 0);

    // 创建取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(dialog);
    lv_obj_set_size(cancel_btn, 120, 40);
    lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -40, -30);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_center(cancel_label);
    lv_obj_set_style_text_font(cancel_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0x4CD964), 0);

    // 添加按钮事件
    lv_obj_add_event_cb(confirm_btn, confirm_delete_all_cb, LV_EVENT_CLICKED, modal_bg);
    lv_obj_add_event_cb(cancel_btn, cancel_delete_cb, LV_EVENT_CLICKED, modal_bg);
    lv_obj_add_event_cb(modal_bg, modal_background_click_cb, LV_EVENT_CLICKED, modal_bg);
}

static void screen_PhotoAlbum_btn_delete_all_event_handler(lv_event_t *e)
{
    // 如果是AI模式进入，禁用删除功能
    if (g_from_ai_preview) {
        MLOG_DBG("AI模式进入，禁止删除全部操作\n");
        return;
    }
    
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));

    switch(code) {
        case LV_EVENT_CLICKED: {
            // 获取当前模式下的文件总数
            int total_files = g_total_media_files;
            if(total_files <= 0) {
                MLOG_DBG("当前模式下没有文件可删除\n");
                return;
            }

            // 创建确认对话框
            create_delete_confirmation_dialog(total_files);
            break;
        }
        default: break;
    }
}


// 触摸事件处理函数
static void touch_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    switch(code) {
        case LV_EVENT_PRESSED: {
            // 记录触摸开始信息
            lv_indev_get_point(lv_indev_active(), &g_touch_start_point);
            g_touch_start_time = lv_tick_get();
            g_is_touching = true;
            g_is_scrolling = false;
            MLOG_DBG("触摸开始: (%d, %d)\n", g_touch_start_point.x, g_touch_start_point.y);
            break;
        }
        case LV_EVENT_PRESSING: {
            if(g_is_touching) {
                lv_point_t current_point;
                lv_indev_get_point(lv_indev_active(), &current_point);
                
                // 计算移动距离
                int16_t dx = abs(current_point.x - g_touch_start_point.x);
                int16_t dy = abs(current_point.y - g_touch_start_point.y);
                
                // 如果移动距离超过阈值，认为是滑动
                if(dx > TOUCH_THRESHOLD || dy > TOUCH_THRESHOLD) {
                    g_is_scrolling = true;
                    MLOG_DBG("检测到滑动，距离: dx=%d, dy=%d\n", dx, dy);
                }
            }
            break;
        }
        case LV_EVENT_RELEASED: {
            if(g_is_touching) {
                lv_indev_get_point(lv_indev_active(), &g_touch_end_point);
                if (g_is_photo_mode) {
                    child_event_handler(e);
                } else {
                    childvideo_event_handler(e);
                }
                g_is_touching = false;
                MLOG_DBG("触摸结束: (%d, %d)\n", g_touch_end_point.x, g_touch_end_point.y);
            }
            break;
        }
        default:
            break;
    }
}

// 检查是否为有效点击
static bool is_valid_click(void)
{
    uint32_t touch_duration = lv_tick_elaps(g_touch_start_time);
    int16_t dx = abs(g_touch_end_point.x - g_touch_start_point.x);
    int16_t dy = abs(g_touch_end_point.y - g_touch_start_point.y);
    
    // 判断条件：短时间、小距离、非滑动状态
    bool is_valid = (touch_duration < TOUCH_TIME_THRESHOLD) && 
                   (dx < TOUCH_THRESHOLD) && 
                   (dy < TOUCH_THRESHOLD) && 
                   !g_is_scrolling;
    
    MLOG_DBG("点击有效性检查: 时间=%dms, 距离=(%d,%d), 滑动=%d, 有效=%d\n",
            touch_duration, dx, dy, g_is_scrolling, is_valid);
    
    return is_valid;
}

void album_return_cb(void)
{
    // 检查是否从AI预览进入
    if (g_from_ai_preview) {
        MLOG_DBG("从AI预览返回预览界面\n");
        g_from_ai_preview = false;
        MESSAGE_S Msg = { 0 };
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_PHOTO;
        MODEMNG_SendMessage(&Msg);
        return_to_preview_with_image(NULL);
        return;
    }
    
    // 返回主菜单
    if (g_last_scr_mode == 1) {
        MESSAGE_S Msg = { 0 };
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_PHOTO;
        MODEMNG_SendMessage(&Msg);
        ui_load_scr_animation(&g_ui, &g_ui.page_photo.photoscr, g_ui.screenHomePhoto_del, NULL, Home_Photo, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
    } else if (g_last_scr_mode == 2) {
        MESSAGE_S Msg = { 0 };
        homeMode_Set(VEDIO_MODE);
        // 进入录像模式
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_MOVIE;
        MODEMNG_SendMessage(&Msg);
        extern bool is_video_mode;
        is_video_mode = true;
        ui_load_scr_animation(&g_ui, &obj_vedio_s, 1, &g_ui.screenHomePhoto_del, Home_Vedio, LV_SCR_LOAD_ANIM_NONE,
            0, 0, false, true);
    } else if(g_last_scr_mode == 0){
        MLOG_DBG("album_menu_callback\n");
        MESSAGE_S Msg = { 0 };
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_BOOT;
        MODEMNG_SendMessage(&Msg);
        ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0, false,
            true);
    }
}

// 设置从AI预览进入标志
void set_from_ai_preview(bool from_ai)
{
    g_from_ai_preview = from_ai;
    MLOG_DBG("设置从AI预览进入标志: %d\n", from_ai);
    
    if (from_ai) {
        // AI模式进入，自动进入多选模式
        MLOG_DBG("AI模式进入，自动进入多选模式\n");
        
        // 确保为照片模式
        g_is_photo_mode = true;
        
        // 自动进入多选模式：设置按钮状态
        // 隐藏选择按钮（已经进入多选模式）
        if (btn_choose_s && lv_obj_is_valid(btn_choose_s)) {
            lv_obj_add_flag(btn_choose_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_choose_s, LV_OBJ_FLAG_CLICKABLE);
        }
        
        // 隐藏取消按钮（AI模式下不允许取消）
        if (btn_cancel_s && lv_obj_is_valid(btn_cancel_s)) {
            lv_obj_add_flag(btn_cancel_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_cancel_s, LV_OBJ_FLAG_CLICKABLE);
        }
        
        // 隐藏删除按钮（AI模式下不允许删除）
        if (btn_delete_s && lv_obj_is_valid(btn_delete_s)) {
            lv_obj_add_flag(btn_delete_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_delete_s, LV_OBJ_FLAG_CLICKABLE);
        }
        if (btn_delete_all_s && lv_obj_is_valid(btn_delete_all_s)) {
            lv_obj_add_flag(btn_delete_all_s, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(btn_delete_all_s, LV_OBJ_FLAG_CLICKABLE);
        }
        
        // 隐藏视频切换按钮，禁用模式切换
        if (btn_vialbum_s && lv_obj_is_valid(btn_vialbum_s)) {
            lv_obj_add_flag(btn_vialbum_s, LV_OBJ_FLAG_HIDDEN);
        }
        if (btn_phalbum_s && lv_obj_is_valid(btn_phalbum_s)) {
            lv_obj_add_flag(btn_phalbum_s, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 标记照片为可选状态
        if (cont_album_grid_s && lv_obj_is_valid(cont_album_grid_s)) {
            mark_photos_selectable(cont_album_grid_s);
        }
    } else {
        // 恢复普通模式：显示切换按钮
        if (btn_vialbum_s && lv_obj_is_valid(btn_vialbum_s)) {
            lv_obj_clear_flag(btn_vialbum_s, LV_OBJ_FLAG_HIDDEN);
        }
        if (btn_phalbum_s && lv_obj_is_valid(btn_phalbum_s)) {
            lv_obj_clear_flag(btn_phalbum_s, LV_OBJ_FLAG_HIDDEN);
        }
        
        // 注意：按钮的显示/隐藏状态由相册的正常逻辑管理
        // 我们只恢复模式切换按钮，其他按钮状态由相册逻辑处理
    }
}
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
#include "events_init.h"

#include "custom.h"
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include "config.h"

#include "common/extract_thumbnail.h"

#define GRID_ITEM_WIDTH 200
#define GRID_ITEM_HEIGHT 140

#define THUMBNAIL_WIDTH 640
#define THUMBNAIL_HEIGHT 480

char current_image_path[256];
// 全局变量用于分页加载
static int g_current_loaded_count = 0;
static int g_total_media_files = 0;
static char **g_all_filenames = NULL;
static lv_obj_t *g_current_parent = NULL;
static lv_ui_t *g_current_ui = NULL;
static int g_current_page = 0;  // 当前页面索引
static int g_images_per_page = 9;  // 每页显示的图片数量
static int g_total_pages = 0;

// 全局变量用于记录上一次滚动位置
static lv_coord_t g_last_scroll_y = 0;
// 防止重复触发的标志
static bool g_is_loading_page = false;

// static lv_obj_t *g_slider = NULL;
static int g_slider_height = 0;

static void load_next_page();
static void load_prev_page();
static void load_page(int page_index);

// 为容器cont_album_grid中的照片添加可选状态
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
            lv_obj_set_style_border_color(child, lv_color_hex(0xFF0000), LV_STATE_CHECKED);
            lv_obj_set_style_border_width(child, 3, LV_STATE_CHECKED);
            lv_obj_set_style_border_side(child, LV_BORDER_SIDE_FULL, LV_STATE_CHECKED);

            // 将容器标记为可选（使用用户数据，但不覆盖文件名）
            // 注意：这里不设置用户数据，因为需要保留文件名信息
            // 可选状态通过其他方式（如样式）来标识

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
            // 确保边框回到默认状态
            lv_obj_set_style_border_color(child, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
            lv_obj_set_style_border_width(child, 2, LV_PART_MAIN);

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

    return (strcmp(ext, ".jpg") == 0) || (strcmp(ext, ".png") == 0) || (strcmp(ext, ".mp4") == 0);
}

// 文件系统回调函数
static void *fs_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    // MLOG_DBG("打开文件: %s\n", path);
    FILE *f = fopen(path, mode == LV_FS_MODE_RD ? "rb" : "wb");
    if(f == NULL) {
        MLOG_DBG("文件打开失败: %s\n", path);
        return NULL;
    }
    return f;
}

static lv_fs_res_t fs_close_cb(lv_fs_drv_t *drv, void *file_p)
{
    // MLOG_DBG("关闭文件\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    fclose(f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    // MLOG_DBG("读取文件\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    *br = fread(buf, 1, btr, f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_write_cb(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    // MLOG_DBG("写入文件\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    *bw = fwrite(buf, 1, btw, f);
    return LV_FS_RES_OK;
}

static lv_fs_res_t fs_seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    // MLOG_DBG("文件定位\n");
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
    // MLOG_DBG("获取文件位置\n");
    FILE *f = (FILE *)file_p;
    if(f == NULL) return LV_FS_RES_INV_PARAM;

    *pos_p = ftell(f);
    return LV_FS_RES_OK;
}

// 目录操作回调函数
static void *fs_dir_open_cb(lv_fs_drv_t *drv, const char *path)
{
    // MLOG_DBG("打开目录: %s\n", path);
    DIR *dir = opendir(path);
    if(dir == NULL) {
        MLOG_DBG("目录打开失败: %s\n", path);
        return NULL;
    }
    return dir;
}

static lv_fs_res_t fs_dir_read_cb(lv_fs_drv_t *drv, void *dir_p, char *fn, uint32_t fn_len)
{
    // MLOG_DBG("读取目录\n");
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
    // MLOG_DBG("关闭目录\n");
    DIR *dir = (DIR *)dir_p;
    if(dir == NULL) return LV_FS_RES_INV_PARAM;

    closedir(dir);
    return LV_FS_RES_OK;
}

// 图片点击事件处理函数
static void child_event_handler(lv_event_t *e)
{
    // 获取传入的ui指针
    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    lv_obj_t *container = lv_event_get_target(e);

    MLOG_DBG("图片点击事件触发\n");

    // 检查ui指针是否有效
    if(!ui) {
        MLOG_DBG("UI指针无效，忽略点击事件\n");
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
    if(lv_obj_has_flag(ui->page_photoalbum.btn_delete, LV_OBJ_FLAG_HIDDEN)) {
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

        // 非选择模式：跳转到图片查看页面
        ui_load_scr_animation(ui, &ui->page_photoalbumpic.scr, ui->screen_PhotoAlbumPic_del, &ui->screen_PhotoAlbum_del,
                              setup_scr_screen_PhotoAlbumPic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
    } else {
        MLOG_DBG("选择模式：检查图片是否可选\n");

        // 检查图片是否被标记为可选
        // 在选择模式下，所有图片都是可选的，不需要特殊标记
        if(lv_obj_has_flag(ui->page_photoalbum.btn_delete, LV_OBJ_FLAG_CLICKABLE)) {
            MLOG_DBG("图片可选，切换选中状态\n");
            // 选择模式：切换选中状态
            if(lv_obj_has_state(container, LV_STATE_CHECKED)) {
                lv_obj_clear_state(container, LV_STATE_CHECKED);
                MLOG_DBG("图片取消选中\n");
            } else {
                lv_obj_add_state(container, LV_STATE_CHECKED);
                MLOG_DBG("图片选中\n");
            }

            // 强制刷新容器显示
            lv_obj_invalidate(container);

            // 调试：检查当前状态
            MLOG_DBG("图片当前状态: %s\n", lv_obj_has_state(container, LV_STATE_CHECKED) ? "已选中" : "未选中");
        } else {
            MLOG_DBG("图片不可选，跳转到图片查看页面\n");
            // 如果图片不可选，跳转到图片查看页面
            ui_load_scr_animation(ui, &ui->page_photoalbumpic.scr, ui->page_photoalbumpic.del, &ui->page_photoalbum.del,
                                  setup_scr_screen_PhotoAlbumPic, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
        }
    }
}

// 视频点击事件处理函数
static void childvideo_event_handler(lv_event_t *e)
{
    // 获取传入的ui指针
    lv_ui_t *ui = (lv_ui_t *)lv_event_get_user_data(e);
    lv_obj_t *container = lv_event_get_target(e);

    // 检查ui指针是否有效
    if(!ui) {
        MLOG_DBG("UI指针无效，忽略视频点击事件\n");
        return;
    }

    // 检查视频容器是否仍然有效
    if(!container || !lv_obj_is_valid(container)) {
        MLOG_DBG("视频容器无效，忽略点击事件\n");
        return;
    }

    // 检查btn_delete是否存在且有效
    if(!ui->page_photoalbum.btn_delete || !lv_obj_is_valid(ui->page_photoalbum.btn_delete)) {
        MLOG_DBG("btn_delete无效，忽略视频点击事件\n");
        return;
    }

    // 检查是否在选择模式下（btn_delete可见表示在选择模式）
    if(lv_obj_has_flag(ui->page_photoalbum.btn_delete, LV_OBJ_FLAG_HIDDEN)) {
        MLOG_DBG("非选择模式：跳转到视频播放页面\n");

        // 获取视频文件名并设置到全局变量中
        char *filename = NULL;
        void *user_data = NULL;

        // 安全地获取用户数据
        user_data = lv_obj_get_user_data(container);
        if(user_data) {
            filename = (char *)user_data;
            MLOG_DBG("获取到视频文件名: %s\n", filename);
            strcpy(current_image_path, filename);
        } else {
            MLOG_DBG("警告：无法获取视频文件名\n");
            strcpy(current_image_path, "");
            return;
        }

        // 非选择模式：跳转到视频播放页面
        ui_load_scr_animation(ui, &ui->screen_PhotoAlbumVid, ui->screen_PhotoAlbumVid_del, &ui->screen_PhotoAlbum_del,
                              setup_scr_screen_PhotoAlbumVid, LV_SCR_LOAD_ANIM_NONE, 200, 200, false, true);
    } else {
        // 选择模式：切换选中状态
        if(lv_obj_has_state(container, LV_STATE_CHECKED)) {
            lv_obj_clear_state(container, LV_STATE_CHECKED);
            MLOG_DBG("视频取消选中\n");
        } else {
            lv_obj_add_state(container, LV_STATE_CHECKED);
            MLOG_DBG("视频选中\n");
        }
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
            // 创建图片容器
            lv_obj_t *img_container = lv_obj_create(g_current_parent);
            lv_obj_set_size(img_container, GRID_ITEM_WIDTH, GRID_ITEM_HEIGHT);

            // 设置容器样式
            lv_obj_set_style_bg_opa(img_container, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_border_color(img_container, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
            lv_obj_set_style_border_width(img_container, 2, LV_PART_MAIN);
            lv_obj_set_style_pad_all(img_container, 0, LV_PART_MAIN);
            lv_obj_set_style_radius(img_container, 0, LV_PART_MAIN);

            // 确保容器是可点击的
            lv_obj_add_flag(img_container, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_clear_flag(img_container, LV_OBJ_FLAG_SCROLLABLE);

            // 创建图片控件
            lv_obj_t *img = lv_img_create(img_container);
            lv_obj_set_size(img, GRID_ITEM_WIDTH, GRID_ITEM_HEIGHT);
            lv_obj_set_style_bg_opa(img, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

            // 设置内容
            char fullpath[128];
            snprintf(fullpath, sizeof(fullpath), PHOTO_ALBUM_IMAGE_PATH "%s", filename);

            if(strstr(filename, ".mp4")) {
                lv_img_set_src(img, PHOTO_ALBUM_IMAGE_PATH "sun.png");
                // 添加视频时长标签（左下方）
                lv_obj_t *duration_label = lv_label_create(img_container);
                lv_label_set_text(duration_label, "02:30"); // 示例时长，实际应从视频元数据获取
                lv_obj_set_style_text_color(duration_label, lv_color_white(), 0);
                lv_obj_set_style_bg_opa(duration_label, LV_OPA_50, 0);
                lv_obj_set_style_bg_color(duration_label, lv_color_black(), 0);
                lv_obj_set_style_pad_all(duration_label, 2, 0);
                lv_obj_align(duration_label, LV_ALIGN_BOTTOM_LEFT, 5, -5);

                // 将文件名存储到容器的用户数据中
                char *filename_copy = strdup(filename);
                lv_obj_set_user_data(img_container, filename_copy);

                // 添加视频点击事件到容器
                lv_obj_add_event_cb(img_container, childvideo_event_handler, LV_EVENT_CLICKED, g_current_ui);
            } else {
                char thumbnail_path_small[256];
                char thumbnail_path_large[256];
                if(strstr(filename, ".jpg")) {
                    // 提取小缩略图到PHOTO_ALBUM_IMAGE_PATH_S目录下，图片名不变
                    snprintf(thumbnail_path_small, sizeof(thumbnail_path_small), "%s%s", PHOTO_ALBUM_IMAGE_PATH_S, filename);
                    // 提取大缩略图到PHOTO_ALBUM_IMAGE_PATH_L目录下，图片名不变
                    snprintf(thumbnail_path_large, sizeof(thumbnail_path_large), "%s%s", PHOTO_ALBUM_IMAGE_PATH_L, filename);
                    char *real_path = strchr(fullpath, '/');
                    char *real_path_small = strchr(thumbnail_path_small, '/');
                    char *real_path_large = strchr(thumbnail_path_large, '/');
                    if(fopen(real_path_small, "rb") == NULL || fopen(real_path_large, "rb") == NULL) {
                        extract_thumbnail(real_path, real_path_small, real_path_large);
                    }
                    lv_img_set_src(img, thumbnail_path_small);
                } else {
                    lv_img_set_src(img, fullpath);
                }

                // 将文件名存储到容器的用户数据中
                char *filename_copy = strdup(filename);
                lv_obj_set_user_data(img_container, filename_copy);

                // 添加图片点击事件到容器
                lv_obj_add_event_cb(img_container, child_event_handler, LV_EVENT_CLICKED, g_current_ui);
            }

            load_count++;
        }
    }

    // 更新当前页面索引
    g_current_page = page_index;
    g_current_loaded_count = end_index;

    // 更新滑动条位置（添加安全检查）
    if(g_current_ui && g_current_ui->page_photoalbum.scrollbar_slider &&
       lv_obj_is_valid(g_current_ui->page_photoalbum.scrollbar_slider)) {
        lv_obj_set_pos(g_current_ui->page_photoalbum.scrollbar_slider, 625, 40 + g_slider_height * g_current_page);
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
}

// 加载下一页的图片
static void load_next_page()
{
    // 计算下一页的索引
    int next_page = g_current_page + 1;

    load_page(next_page);
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
}

// 2. 动态创建相册格子
void create_album_grid(lv_obj_t *parent, lv_ui_t *ui)
{
    // 清理之前的全局变量
    cleanup_global_vars();

    // 设置全局变量
    g_current_parent = parent;
    g_current_ui = ui;

    // 容器样式设置
    lv_obj_set_style_pad_all(parent, 5, 0);
    lv_obj_set_style_base_dir(parent, LV_BASE_DIR_LTR, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW_WRAP);

    // 设置Flex布局间距
    lv_obj_set_style_pad_row(parent, 5, 0);     // 行间距5像素
    lv_obj_set_style_pad_column(parent, 5, 0);  // 列间距5像素

    // 设置滚动边界，防止过度滚动
    lv_obj_set_scroll_snap_y(parent, LV_SCROLL_SNAP_START);

    // 设置滑动条样式，确保可见
    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, LV_PART_SCROLLBAR);

    // 扫描虚拟路径
    lv_fs_dir_t dir;
    lv_fs_res_t res = lv_fs_dir_open(&dir, PHOTO_ALBUM_IMAGE_PATH);

    if(res != LV_FS_RES_OK) {
        // 创建错误提示标签
        lv_obj_t *error_label = lv_label_create(parent);
        lv_label_set_text(error_label, "无法打开图片目录");
        MLOG_DBG("无法打开图片目录\n");
        lv_obj_set_style_text_color(error_label, lv_color_hex(0xFF0000), 0);
        lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
        return;
    }

    // 首先统计媒体文件数量并收集所有文件名
    g_total_media_files = 0;
    char temp_filename[64];

    // 第一次遍历：统计文件数量
    while(1) {
        lv_fs_res_t read_res = lv_fs_dir_read(&dir, temp_filename, sizeof(temp_filename));
        if(read_res != LV_FS_RES_OK || strlen(temp_filename) == 0) break;
        if(is_media_file(temp_filename)) {
            g_total_media_files++;
        }
    }

    // 分配文件名数组
    g_all_filenames = (char **)malloc(sizeof(char *) * g_total_media_files);
    if(g_all_filenames == NULL) {
        MLOG_DBG("内存分配失败\n");
        lv_fs_dir_close(&dir);
        return;
    }
    memset(g_all_filenames, 0, sizeof(char *) * g_total_media_files);

    // 重新打开目录
    lv_fs_dir_close(&dir);
    res = lv_fs_dir_open(&dir, PHOTO_ALBUM_IMAGE_PATH);
    if(res != LV_FS_RES_OK) {
        MLOG_DBG("重新打开目录失败\n");
        cleanup_global_vars();
        return;
    }

    // 第二次遍历：收集所有文件名
    int filename_index = 0;
    while(filename_index < g_total_media_files) {
        lv_fs_res_t read_res = lv_fs_dir_read(&dir, temp_filename, sizeof(temp_filename));
        if(read_res != LV_FS_RES_OK || strlen(temp_filename) == 0) break;

        if(is_media_file(temp_filename)) {
            g_all_filenames[filename_index] = strdup(temp_filename);
            filename_index++;
        }
    }

    g_total_pages = (g_total_media_files + g_images_per_page - 1) / g_images_per_page;
    g_slider_height = 440 / g_total_pages;

    // 更新滑动条大小和位置（添加安全检查）
    if(ui && ui->page_photoalbum.scrollbar_slider &&
       lv_obj_is_valid(ui->page_photoalbum.scrollbar_slider)) {
        lv_obj_set_size(ui->page_photoalbum.scrollbar_slider, 12, g_slider_height);
        lv_obj_set_pos(ui->page_photoalbum.scrollbar_slider, 625, 40 + g_slider_height * g_current_page);
    }

    // 重置加载计数器
    g_current_loaded_count = 0;
    g_current_page = 0;

    lv_fs_dir_close(&dir);

    MLOG_DBG("找到 %d 个媒体文件\n", g_total_media_files);
    MLOG_DBG("总页数: %d\n", g_total_pages);

    // 初始加载第一页图片
    load_page(0);

    // 添加滚动事件监听
    lv_obj_add_event_cb(parent, scroll_event_handler, LV_EVENT_SCROLL, NULL);
    // 强制启用滚动，即使内容不足
    lv_obj_set_scroll_dir(parent, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(parent, LV_SCROLL_SNAP_START);
}

static void screen_PhotoAlbum_btn_delete_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            lv_obj_t *parent   = g_ui.page_photoalbum.cont_album_grid;
            uint32_t child_cnt = lv_obj_get_child_cnt(parent);

            lv_obj_t *selected_photos[child_cnt]; // 存储选中对象指针
            const char *selected_filenames[child_cnt]; // 存储选中文件名
            uint32_t selected_count = 0;

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

            // 第二次遍历：批量删除（避免索引错乱）
            for(uint32_t i = 0; i < selected_count; i++) {
                // 删除文件系统中的文件
                if(selected_filenames[i]) {
                    char cmd[512];

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
                        snprintf(cmd, sizeof(cmd), "rm -f %s%s", real_path, selected_filenames[i]);
                        MLOG_DBG("执行命令: %s\n", cmd);
                        system(cmd);
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

            // 重新加载第一页
            lv_obj_clean(parent);
            load_page(0);

            // 退出选择模式
            lv_obj_add_flag(g_ui.page_photoalbum.btn_cancel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_cancel, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_choose, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_choose, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_delete, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_delete, LV_OBJ_FLAG_HIDDEN);
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
            // 清理全局变量
            cleanup_global_vars();

            ui_load_scr_animation(&g_ui, &g_ui.page_home2.scr, g_ui.screenHome2_del, &g_ui.screen_PhotoAlbum_del,
                                  setup_scr_screenHome2, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbum_btn_choose_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            MLOG_DBG("btn_choose clicked - 进入选择模式\n");
            mark_photos_selectable(g_ui.page_photoalbum.cont_album_grid);

            lv_obj_remove_flag(g_ui.page_photoalbum.btn_cancel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_cancel, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_choose, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_choose, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_delete, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_delete, LV_OBJ_FLAG_CLICKABLE);
            break;
        }
        default: break;
    }
}

static void screen_PhotoAlbum_btn_cancel_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            MLOG_DBG("btn_cancel clicked - 退出选择模式\n");
            lv_obj_add_flag(g_ui.page_photoalbum.btn_cancel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_cancel, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_remove_flag(g_ui.page_photoalbum.btn_choose, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_choose, LV_OBJ_FLAG_CLICKABLE);
            mark_photos_unselectable(g_ui.page_photoalbum.cont_album_grid);

            lv_obj_remove_flag(g_ui.page_photoalbum.btn_delete, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_flag(g_ui.page_photoalbum.btn_delete, LV_OBJ_FLAG_HIDDEN);
            break;
        }
        default: break;
    }
}

void events_init_screen_PhotoAlbum(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_photoalbum.btn_delete, screen_PhotoAlbum_btn_delete_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(ui->page_photoalbum.btn_back, screen_PhotoAlbum_btn_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->page_photoalbum.btn_choose, screen_PhotoAlbum_btn_choose_event_handler, LV_EVENT_CLICKED,
                        ui);
    lv_obj_add_event_cb(ui->page_photoalbum.btn_cancel, screen_PhotoAlbum_btn_cancel_event_handler, LV_EVENT_CLICKED,
                        ui);
}

void setup_scr_screen_PhotoAlbum(lv_ui_t *ui)
{
    MLOG_DBG("loading page_PhotoAlbum...\n");

    PhotoAlbum_t *PhotoAlbum = &ui->page_photoalbum;
    PhotoAlbum->del          = true;

    // 创建主页面1 容器
    if(PhotoAlbum->scr != NULL) {
        if(lv_obj_is_valid(PhotoAlbum->scr)) {
            MLOG_DBG("page_PhotoAlbum->scr 仍然有效，删除旧对象\n");
            lv_obj_del(PhotoAlbum->scr);
        } else {
            MLOG_DBG("page_PhotoAlbum->scr 已被自动销毁，仅重置指针\n");
        }
        PhotoAlbum->scr = NULL;
    }

    // Write codes scr
    PhotoAlbum->scr = lv_obj_create(NULL);
    lv_obj_set_size(PhotoAlbum->scr, 640, 480);
    lv_obj_set_scrollbar_mode(PhotoAlbum->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbum->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_top
    PhotoAlbum->cont_top = lv_obj_create(PhotoAlbum->scr);
    lv_obj_set_pos(PhotoAlbum->cont_top, 0, 0);
    lv_obj_set_size(PhotoAlbum->cont_top, 640, 40);
    lv_obj_set_scrollbar_mode(PhotoAlbum->cont_top, LV_SCROLLBAR_MODE_OFF);

    // Write style for cont_top, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(PhotoAlbum->cont_top, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbum->cont_top, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(PhotoAlbum->cont_top, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbum->cont_top, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_delete
    PhotoAlbum->btn_delete = lv_button_create(PhotoAlbum->cont_top);
    lv_obj_set_pos(PhotoAlbum->btn_delete, 500, 4);  // 调整位置，避免与btn_choose重叠
    lv_obj_set_size(PhotoAlbum->btn_delete, 56, 32);
    lv_obj_add_flag(PhotoAlbum->btn_delete, LV_OBJ_FLAG_HIDDEN);
    PhotoAlbum->label_delete = lv_label_create(PhotoAlbum->btn_delete);
    lv_label_set_text(PhotoAlbum->label_delete, " " LV_SYMBOL_TRASH " ");
    lv_label_set_long_mode(PhotoAlbum->label_delete, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoAlbum->label_delete, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(PhotoAlbum->btn_delete, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(PhotoAlbum->label_delete, LV_PCT(100));

    // Write style for btn_delete, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbum->btn_delete, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbum->btn_delete, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbum->btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->btn_delete, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbum->btn_delete, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbum->btn_delete, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbum->btn_delete, &lv_font_SourceHanSerifSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbum->btn_delete, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbum->btn_delete, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back
    PhotoAlbum->btn_back = lv_button_create(PhotoAlbum->cont_top);
    lv_obj_set_pos(PhotoAlbum->btn_back, 0, 4);
    lv_obj_set_size(PhotoAlbum->btn_back, 56, 32);
    PhotoAlbum->label_back = lv_label_create(PhotoAlbum->btn_back);
    lv_label_set_text(PhotoAlbum->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(PhotoAlbum->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoAlbum->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(PhotoAlbum->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(PhotoAlbum->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbum->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbum->btn_back, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbum->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->btn_back, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbum->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbum->btn_back, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbum->btn_back, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbum->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbum->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_choose
    PhotoAlbum->btn_choose = lv_button_create(PhotoAlbum->cont_top);
    lv_obj_set_pos(PhotoAlbum->btn_choose, 573, 4);
    lv_obj_set_size(PhotoAlbum->btn_choose, 56, 32);
    PhotoAlbum->label_choose = lv_label_create(PhotoAlbum->btn_choose);
    lv_label_set_text(PhotoAlbum->label_choose, "选择");
    lv_label_set_long_mode(PhotoAlbum->label_choose, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoAlbum->label_choose, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(PhotoAlbum->btn_choose, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(PhotoAlbum->label_choose, LV_PCT(100));

    // Write style for btn_choose, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbum->btn_choose, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbum->btn_choose, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbum->btn_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->btn_choose, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbum->btn_choose, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbum->btn_choose, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbum->btn_choose, &lv_font_SourceHanSerifSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbum->btn_choose, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbum->btn_choose, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_cancel
    PhotoAlbum->btn_cancel = lv_button_create(PhotoAlbum->cont_top);
    lv_obj_set_pos(PhotoAlbum->btn_cancel, 572, 3);
    lv_obj_set_size(PhotoAlbum->btn_cancel, 56, 32);
    lv_obj_add_flag(PhotoAlbum->btn_cancel, LV_OBJ_FLAG_HIDDEN);
    PhotoAlbum->label_cancel = lv_label_create(PhotoAlbum->btn_cancel);
    lv_label_set_text(PhotoAlbum->label_cancel, "取消");
    lv_label_set_long_mode(PhotoAlbum->label_cancel, LV_LABEL_LONG_WRAP);
    lv_obj_align(PhotoAlbum->label_cancel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(PhotoAlbum->btn_cancel, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(PhotoAlbum->label_cancel, LV_PCT(100));

    // Write style for btn_cancel, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(PhotoAlbum->btn_cancel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbum->btn_cancel, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbum->btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->btn_cancel, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbum->btn_cancel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(PhotoAlbum->btn_cancel, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(PhotoAlbum->btn_cancel, &lv_font_SourceHanSerifSC_Regular_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(PhotoAlbum->btn_cancel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(PhotoAlbum->btn_cancel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes cont_album_grid
    PhotoAlbum->cont_album_grid = lv_obj_create(PhotoAlbum->scr);
    lv_obj_set_pos(PhotoAlbum->cont_album_grid, 0, 40);
    lv_obj_set_size(PhotoAlbum->cont_album_grid, 625, 440);
    lv_obj_set_scrollbar_mode(PhotoAlbum->cont_album_grid, LV_SCROLLBAR_MODE_ON);

    // Write style for cont_album_grid, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(PhotoAlbum->cont_album_grid, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(PhotoAlbum->cont_album_grid, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(PhotoAlbum->cont_album_grid, lv_color_hex(0x2195f6), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(PhotoAlbum->cont_album_grid, LV_BORDER_SIDE_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->cont_album_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(PhotoAlbum->cont_album_grid, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(PhotoAlbum->cont_album_grid, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(PhotoAlbum->cont_album_grid, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(PhotoAlbum->cont_album_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(PhotoAlbum->cont_album_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(PhotoAlbum->cont_album_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(PhotoAlbum->cont_album_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(PhotoAlbum->cont_album_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建滚动条滑块
    PhotoAlbum->scrollbar_slider = lv_obj_create(PhotoAlbum->scr);
    lv_obj_set_size(PhotoAlbum->scrollbar_slider, 12, 60);
    lv_obj_set_pos(PhotoAlbum->scrollbar_slider, 625, 40);
    lv_obj_set_style_bg_color(PhotoAlbum->scrollbar_slider, lv_color_hex(0xFFD600), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(PhotoAlbum->scrollbar_slider, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(PhotoAlbum->scrollbar_slider, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(PhotoAlbum->scrollbar_slider, 1, LV_PART_MAIN | LV_STATE_DEFAULT);  // 添加边框
    lv_obj_set_style_border_color(PhotoAlbum->scrollbar_slider, lv_color_hex(0x1A1A1A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(PhotoAlbum->scrollbar_slider, 10, LV_PART_MAIN | LV_STATE_DEFAULT);  // 圆角

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

    char *real_path_s;
    char *real_path_l;
    // 检查并创建小缩略图目录
    if(PHOTO_ALBUM_IMAGE_PATH_S) {
        real_path_s = strchr(PHOTO_ALBUM_IMAGE_PATH_S, '/');
        DIR *dir_s = opendir(real_path_s);
        if(dir_s == NULL) {
            MLOG_DBG("PHOTO_ALBUM_IMAGE_PATH_S 目录不存在，创建它\n");
            char cmd[1024];
            if(real_path_s) {
                snprintf(cmd, sizeof(cmd), "mkdir -p %s", real_path_s);
                MLOG_DBG("执行命令: %s\n", cmd);
                system(cmd);
            }
        } else {
            MLOG_DBG("PHOTO_ALBUM_IMAGE_PATH_S 目录已存在\n");
            closedir(dir_s);
        }
    }

    // 检查并创建大缩略图目录
    if(PHOTO_ALBUM_IMAGE_PATH_L) {
        real_path_l = strchr(PHOTO_ALBUM_IMAGE_PATH_L, '/');
        DIR *dir_l = opendir(real_path_l);
        if(dir_l == NULL) {
            MLOG_DBG("PHOTO_ALBUM_IMAGE_PATH_L 目录不存在，创建它\n");
            char cmd[1024];
            if(real_path_l) {
                snprintf(cmd, sizeof(cmd), "mkdir -p %s", real_path_l);
                MLOG_DBG("执行命令: %s\n", cmd);
                system(cmd);
            }
        } else {
            MLOG_DBG("PHOTO_ALBUM_IMAGE_PATH_L 目录已存在\n");
            closedir(dir_l);
        }
    }

    create_album_grid(PhotoAlbum->cont_album_grid, ui);

    // Update current screen layout.
    lv_obj_update_layout(PhotoAlbum->scr);

    // Init events for screen.
    events_init_screen_PhotoAlbum(ui);
}

#ifndef EXTRACT_THUMBNAIL_H
#define EXTRACT_THUMBNAIL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "thumbnail_extractor.h"
typedef enum {
    PHOTO_REL_PATH   = 0, // 原图路径
    PHOTO_SMALL_PATH = 1, // 小缩略图路径
    PHOTO_LARGE_PATH = 2, // 大缩略图路径
    VIDEO_REL_PATH   = 3, // 视频原文件路径
    VIDEO_SMALL_PATH = 4, // 视频小缩略图路径
    VIDEO_LARGE_PATH = 5, // 视频大缩略图路径
} path_type_t;

/**
 * @brief 提取图片的缩略图
 *
 * 从指定的输入图片文件中提取小缩略图(APP0)和大缩略图(APP3)，
 * 并分别保存到指定的输出文件中。
 *
 * @param input_file 输入图片文件路径
 * @param output_file_small 小缩略图输出文件路径
 * @param output_file_large 大缩略图输出文件路径
 *
 * @note 此函数依赖于 thumbnail_extractor 组件
 */
void extract_thumbnail(const char *input_file, const char *output_file_small, const char *output_file_large);

uint8_t extract_video_thumbnail(const char *video_path, const char *output_path_small, const char *output_path_large);

int get_video_duration(const char *video_path);

/*
 *获取缩略图路径
 *   file_name 输入路径或文件名
 *   output_path 输出路径数组
 *   output_size 输出路径数组大小
 *   type 输出类型
 */
void get_thumbnail_path(const char* file_name, char* output_path, uint16_t output_size, path_type_t type);

/**
 * @brief 判断并移除路径中的 "A:" 前缀
 *
 * @param path 路径字符串数组
 * @return int 0-成功移除或无需移除, -1-失败(path为NULL)
 *
 * @note 如果路径不以 "A:" 开头，则不做任何修改
 */
int remove_A_prefix(char* path);

/**
 * @brief 判断并添加 "A:" 前缀
 *
 * @param path 路径字符串数组
 * @param len  路径数组长度
 * @return int 0-成功添加或已存在, -1-失败(数组长度不足等)
 *
 * @note 如果路径已以 "A:" 开头，则不做任何修改
 */
int add_A_prefix(char* path, uint16_t len);

/**
 * @brief 获取路径的文件名部分
 *
 * @param path 路径字符串
 * @return char* 文件名指针，如果path为NULL则返回NULL
 *
 * @note 不修改原字符串，只返回指向文件名开头的指针
 */
char* get_basename(char* path);

/**
 * @brief 修复文件路径的合法性
 *
 * @param path 路径字符串（会被原地修改）
 * @return int 0-路径已修复或无需修复, -1-失败(path为NULL)
 *
 * @note 检查并移除路径中的 "//"（替换为 "/"），移除前缀 A:
 */
int fix_path_validity(char* path);

#ifdef __cplusplus
}
#endif

#endif /* EXTRACT_THUMBNAIL_H */

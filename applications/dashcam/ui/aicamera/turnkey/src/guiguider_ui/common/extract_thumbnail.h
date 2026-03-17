#ifndef EXTRACT_THUMBNAIL_H
#define EXTRACT_THUMBNAIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thumbnail_extractor.h"

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

#ifdef __cplusplus
}
#endif

#endif /* EXTRACT_THUMBNAIL_H */
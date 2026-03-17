#define DEBUG
#include <stdio.h>
#include <stdlib.h> // 包含 malloc 和 free
#include "mlog.h"
#include "extract_thumbnail.h"
#include "config.h"
#include "page_all.h"
#include <unistd.h>
#include "filemng.h"

#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
#include "thumbnail_extractor.h"
#endif

int get_video_duration(const char *video_path);
// 提取缩略图
void extract_thumbnail(const char *input_file, const char *output_file_small, const char *output_file_large)
{
    THUMBNAIL_EXTRACTOR_HANDLE_T extractor = NULL;
    THUMBNAIL_PACKET_S app0_thumbnail = {
        .data = NULL,
        .size = 0,
        .pts = 0,
        .duration = 0.0,
        .creationtime = 0,
        .errorccode = {0, 0, 0, 0}
    };
    THUMBNAIL_PACKET_S app3_thumbnail = {
        .data = NULL,
        .size = 0,
        .pts = 0,
        .duration = 0.0,
        .creationtime = 0,
        .errorccode = {0, 0, 0, 0}
    };

    // 创建缩略图提取器实例
    if (THUMBNAIL_EXTRACTOR_Create(&extractor) != 0) {
        MLOG_ERR("创建缩略图提取器失败\n");
        return;
    }
    // MLOG_DBG("缩略图提取器创建成功\n");

    // 提取APP0（小缩略图）
    if (THUMBNAIL_EXTRACTOR_GetThumbnailByType(extractor, input_file, &app0_thumbnail, 0) == 0) {
        FILE *fp = fopen(output_file_small, "wb");
        if (fp) {
            fwrite(app0_thumbnail.data, 1, app0_thumbnail.size, fp);
            fclose(fp);
            // MLOG_DBG("小缩略图(APP0)已保存到 %s\n", output_file_small);
        } else {
            MLOG_ERR("无法保存APP0缩略图到文件\n");
        }
    } else {
        MLOG_ERR("未找到APP0小缩略图\n");
    }

    // 提取APP3（大缩略图）
    if (THUMBNAIL_EXTRACTOR_GetThumbnailByType(extractor, input_file, &app3_thumbnail, 1) == 0) {
        FILE *fp = fopen(output_file_large, "wb");
        if (fp) {
            fwrite(app3_thumbnail.data, 1, app3_thumbnail.size, fp);
            fclose(fp);
            // MLOG_DBG("大缩略图(APP3)已保存到 %s\n", output_file_large);
        } else {
            MLOG_ERR("无法保存APP3缩略图到文件\n");
        }
    } else {
        MLOG_ERR("未找到APP3大缩略图\n");
    }

    // 清理缩略图数据包
    if (THUMBNAIL_EXTRACTOR_ClearPacket(&app0_thumbnail) != 0) {
        MLOG_ERR("清理APP0缩略图数据包失败\n");
    }
    if (THUMBNAIL_EXTRACTOR_ClearPacket(&app3_thumbnail) != 0) {
        MLOG_ERR("清理APP3缩略图数据包失败\n");
    }

    // 销毁缩略图提取器实例
    if (THUMBNAIL_EXTRACTOR_Destroy(&extractor) != 0) {
        MLOG_ERR("销毁缩略图提取器失败\n");
    } else {
        // MLOG_DBG("缩略图提取器销毁成功\n");
    }
}

// 用于写入PNG数据的回调函数
static void png_write_func(void *context, void *data, int size)
{
    FILE *fp = (FILE *)context;
    fwrite(data, 1, size, fp);
}

uint8_t extract_video_thumbnail(const char *video_path, const char *output_path_small, const char *output_path_large)
{
    uint8_t ret = 1; // 默认失败
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
    THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;

    THUMBNAIL_PACKET_S small_thumbnail = {
        .data = NULL, .size = 0, .pts = 0, .duration = 0.0, .creationtime = 0, .errorccode = {0, 0, 0, 0}};
    THUMBNAIL_PACKET_S large_thumbnail = {
        .data = NULL, .size = 0, .pts = 0, .duration = 0.0, .creationtime = 0, .errorccode = {0, 0, 0, 0}};

    // 创建缩略图提取器
    if(THUMBNAIL_EXTRACTOR_Create(&viewer_handle) != 0) {
        MLOG_DBG("创建缩略图提取器失败\n");
        return ret;
    }

    // 提取APP3（大缩略图）
    if(THUMBNAIL_EXTRACTOR_GetThumbnailByType(viewer_handle, video_path, &large_thumbnail, 1) == 0) {
        // 保存大尺寸缩略图
        FILE *fp_large = fopen(output_path_large, "wb");
        if(fp_large) {
            fwrite(large_thumbnail.data, 1, large_thumbnail.size, fp_large);
            fclose(fp_large);
            MLOG_DBG("视频大缩略图保存成功: %s\n", output_path_large);
        } else {
            MLOG_DBG("无法创建大缩略图文件: %s\n", output_path_large);
        }
    } else {
        MLOG_ERR("未找到APP3大缩略图\n");
    }

    // 提取APP0（小缩略图）
    if(THUMBNAIL_EXTRACTOR_GetThumbnailByType(viewer_handle, video_path, &small_thumbnail, 0) == 0) {
        FILE *fp_small = fopen(output_path_small, "wb");
        if(fp_small) {
            fwrite(small_thumbnail.data, 1, small_thumbnail.size, fp_small);
            fclose(fp_small);
            MLOG_DBG("视频小缩略图保存成功: %s\n", output_path_small);
            ret = 0; // 成功
        } else {
            MLOG_DBG("无法创建小缩略图文件: %s\n", output_path_small);
        }
    } else {
        MLOG_ERR("未找到APP0小缩略图\n");
    }

    // 清理缩略图数据包
    if(THUMBNAIL_EXTRACTOR_ClearPacket(&large_thumbnail) != 0) {
        MLOG_ERR("清理APP0缩略图数据包失败\n");
    }
    if(THUMBNAIL_EXTRACTOR_ClearPacket(&small_thumbnail) != 0) {
        MLOG_ERR("清理APP3缩略图数据包失败\n");
    }

    // 销毁缩略图提取器实例
    if(THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle) != 0) {
        MLOG_ERR("销毁缩略图提取器失败\n");
    } else {
        // MLOG_DBG("缩略图提取器销毁成功\n");
    }

#endif

    return ret;
}

// 获取视频时长（单位：秒）
int get_video_duration(const char *video_path)
{
    int duration = 0; // 默认0秒
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
    THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;
    THUMBNAIL_PACKET_S packet                  = {0};

    // 创建缩略图提取器实例
    if(THUMBNAIL_EXTRACTOR_Create(&viewer_handle) == 0) {
        // 获取视频元数据
        if(THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, video_path, &packet) == 0) {
            // 从数据包中提取时长信息（单位秒）
            duration = (int)packet.duration;
            MLOG_DBG("获取视频时长成功: %d秒 (%s)\n", duration, video_path);
        } else {
            MLOG_DBG("获取视频元数据失败: %s\n", video_path);
        }

        // 清理数据包
        THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
        // 销毁提取器
        THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
    } else {
        MLOG_DBG("创建缩略图提取器失败\n");
    }
#endif
    return duration;
}

// 路径检查与创建函数
static void check_and_create_dir(const char* path)
{
    if (!path)
        return;

    const char* real_path = strchr(path, '/');
    if (!real_path)
        real_path = path;

    DIR* dir = opendir(real_path);
    if (dir == NULL) {
        MLOG_DBG("目录不存在，创建它: %s\n", real_path);
        char cmd[100];
        snprintf(cmd, sizeof(cmd), "mkdir -p %s", real_path);
        MLOG_INFO("执行命令: %s\n", cmd);
        system(cmd);

        snprintf(cmd, sizeof(cmd), "fatattr +h %s", HIDDLE_THUMB_PATH);
        MLOG_INFO("执行命令: %s\n", cmd);
        system(cmd);
    } else {
        closedir(dir);
    }
}

void get_thumbnail_path(const char *file_name, char *output_path, uint16_t output_size, path_type_t type)
{

    if(file_name == NULL) {
        MLOG_ERR("文件名为空\n");
        return;
    }
    const char *pure_filename = strrchr(file_name, '/');
    char *dot                 = NULL;
    char base_name[256]       = {0};

    if(pure_filename == NULL) {
        dot = strrchr(file_name, '.');
        snprintf(base_name, sizeof(base_name), "%.*s", (int)(dot - file_name), file_name);
    } else {
        dot = strrchr(pure_filename, '.');
        snprintf(base_name, sizeof(base_name), "%.*s", (int)(dot - pure_filename), pure_filename);
    }
    // MLOG_DBG("BUG调试 base_name %s dot:%s \n", base_name,dot);

    if(output_path != NULL) {
        // 使用三元运算符根据type选择路径
        const char *path_prefix = type == VIDEO_REL_PATH     ? PHOTO_ALBUM_MOVIE_PATH
                                  : type == VIDEO_SMALL_PATH ? PHOTO_ALBUM_VIDEO_THUMB_PATH_S
                                  : type == VIDEO_LARGE_PATH ? PHOTO_ALBUM_VIDEO_THUMB_PATH_L
                                  : type == PHOTO_REL_PATH   ? PHOTO_ALBUM_IMAGE_PATH
                                  : type == PHOTO_SMALL_PATH ? PHOTO_ALBUM_IMAGE_PATH_S
                                  : type == PHOTO_LARGE_PATH ? PHOTO_ALBUM_IMAGE_PATH_L
                                                             : NULL;

        const char *file_extension = type == VIDEO_REL_PATH ? ".mov" : ".jpg";

        if(path_prefix == NULL) {
            MLOG_ERR("不支持的路径类型: %d\n", type);
            output_path[0] = '\0';
            return;
        }
        snprintf(output_path, output_size, "%s%s%s", path_prefix, base_name, file_extension);
        normalize_path(output_path);
        // MLOG_DBG("bug调试: %s\n", output_path);
    }

    // 根据提取策略处理缩略图
    const char *source_path        = NULL;
    const char *small_path         = NULL;
    const char *large_path         = NULL;
    char thumbnail_path_small[100] = {0};
    char thumbnail_path_large[100] = {0};
    char thumbnail_path_real[100]  = {0};
    if(strcmp(dot,".mov") == 0) {
        // 视频文件
        // 路径检查
        check_and_create_dir(PHOTO_ALBUM_VIDEO_THUMB_PATH_S); // 视频小缩略图目录
        check_and_create_dir(PHOTO_ALBUM_VIDEO_THUMB_PATH_L); // 视频大缩略图目录
        // 创建小缩略图路径
        snprintf(thumbnail_path_small, sizeof(thumbnail_path_small), "%s%s.jpg", PHOTO_ALBUM_VIDEO_THUMB_PATH_S,
                 base_name);
        // 创建大缩略图路径
        snprintf(thumbnail_path_large, sizeof(thumbnail_path_large), "%s%s.jpg", PHOTO_ALBUM_VIDEO_THUMB_PATH_L,
                 base_name);
        // 构建完整视频路径
        snprintf(thumbnail_path_real, sizeof(thumbnail_path_real), "%s%s.mov", PHOTO_ALBUM_MOVIE_PATH, base_name);
        source_path = strchr(thumbnail_path_real, '/');
        small_path  = strchr(thumbnail_path_small, '/');
        large_path  = strchr(thumbnail_path_large, '/');

        if(source_path && ((access(small_path, F_OK) != 0 || access(large_path, F_OK) != 0))) {
            extract_video_thumbnail(source_path, small_path, large_path);
            MLOG_DBG("当前视频没有缩略图,提取缩略图 %s\n",source_path);
        }
    } else {
        // 路径检查
        check_and_create_dir(PHOTO_ALBUM_IMAGE_PATH_S);       // 小缩略图目录
        check_and_create_dir(PHOTO_ALBUM_IMAGE_PATH_L);       // 大缩略图目录
        // 照片文件
        snprintf(thumbnail_path_small, sizeof(thumbnail_path_small), "%s%s.jpg", PHOTO_ALBUM_IMAGE_PATH_S, base_name);
        snprintf(thumbnail_path_large, sizeof(thumbnail_path_large), "%s%s.jpg", PHOTO_ALBUM_IMAGE_PATH_L, base_name);
        snprintf(thumbnail_path_real, sizeof(thumbnail_path_real), "%s%s.jpg", PHOTO_ALBUM_IMAGE_PATH, base_name);
        source_path = strchr(thumbnail_path_real, '/');
        small_path  = strchr(thumbnail_path_small, '/');
        large_path  = strchr(thumbnail_path_large, '/');

        if(source_path && ((access(small_path, F_OK) != 0 || access(large_path, F_OK) != 0))) {
            extract_thumbnail(source_path, small_path, large_path);
            MLOG_DBG("当前照片没有缩略图,提取缩略图 %s\n",source_path);
        }
    }
}

/**
 * @brief 判断并移除路径中的 "A:" 前缀
 *
 * @param path 路径字符串数组
 * @return int 0-成功移除或无需移除, -1-失败(path为NULL)
 *
 * @note 如果路径不以 "A:" 开头，则不做任何修改
 */
int remove_A_prefix(char* path)
{
    if (path == NULL) {
        return -1;
    }

    // 检查是否以 "A:" 开头
    if (path[0] == 'A' && path[1] == ':') {
        size_t path_len = strlen(path);
        // 移动字符串，移除 "A:" 前缀
        memmove(path, path + 2, path_len - 1); // -1 保留结尾的 '\0'
        path[path_len - 2] = '\0';
        MLOG_DBG("移除 A: 前缀成功: %s\n", path);
    }

    return 0;
}

/**
 * @brief 判断并添加 "A:" 前缀
 *
 * @param path 路径字符串数组
 * @param len  路径数组长度
 * @return int 0-成功添加或已存在, -1-失败(数组长度不足等)
 *
 * @note 如果路径已以 "A:" 开头，则不做任何修改
 */
int add_A_prefix(char* path, uint16_t len)
{
    if (path == NULL || len < 3) {
        return -1;
    }

    // 检查是否已存在 "A:" 前缀
    if (path[0] == 'A' && path[1] == ':') {
        MLOG_DBG("A: 前缀已存在: %s\n", path);
        return 0;
    }

    size_t path_len = strlen(path);
    // 需要 "A:" + 原路径 + '\0'，至少需要 path_len + 3 个字符
    if (path_len + 3 > len) {
        MLOG_ERR("数组长度不足，无法添加 A: 前缀\n");
        return -1;
    }

    // 移动原字符串，为 "A:" 留出空间
    memmove(path + 2, path, path_len + 1); // +1 包括 '\0'
    path[0] = 'A';
    path[1] = ':';
    MLOG_DBG("添加 A: 前缀成功: %s\n", path);

    return 0;
}

/**
 * @brief 获取路径的文件名部分
 *
 * @param path 路径字符串
 * @return char* 文件名指针，如果path为NULL则返回NULL
 *
 * @note 不修改原字符串，只返回指向文件名开头的指针
 */
char* get_basename(char* path)
{
    if (path == NULL) {
        return NULL;
    }
    char* basename = strrchr(path, '/');
    return basename ? basename + 1 : path;
}

/**
 * @brief 修复文件路径的合法性
 *
 * @param path 路径字符串（会被原地修改）
 * @return int 0-路径已修复或无需修复, -1-失败(path为NULL)
 *
 * @note 检查并移除路径中的 "//"（替换为 "/"），移除前缀 A:
 */
int fix_path_validity(char* path)
{
    if (path == NULL) {
        return -1;
    }

    remove_A_prefix(path);

    // 查找 "//" 并替换为 "/"
    char* pos = strstr(path, "//");
    while (pos != NULL) {
        *pos = '/'; // 将第一个 '/' 保留，第二个被覆盖
        // 移动后续字符串覆盖多余的 '/'
        memmove(pos + 1, pos + 2, strlen(pos + 2) + 1);
        pos = strstr(path, "//"); // 继续查找是否还有 "//"
    }

    return 0;
}

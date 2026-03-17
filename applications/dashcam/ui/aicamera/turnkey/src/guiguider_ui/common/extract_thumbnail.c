#define DEBUG
#include <stdio.h>
#include <stdlib.h> // 包含 malloc 和 free
#include "mlog.h"
#include "extract_thumbnail.h"

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

    THUMBNAIL_PACKET_S app0_thumbnail = {
        .data = NULL, .size = 0, .pts = 0, .duration = 0.0, .creationtime = 0, .errorccode = {0, 0, 0, 0}};
    THUMBNAIL_PACKET_S app3_thumbnail = {
        .data = NULL, .size = 0, .pts = 0, .duration = 0.0, .creationtime = 0, .errorccode = {0, 0, 0, 0}};

    // 创建缩略图提取器
    if(THUMBNAIL_EXTRACTOR_Create(&viewer_handle) != 0) {
        MLOG_DBG("创建缩略图提取器失败\n");
        return ret;
    }

    // 提取原始缩略图
    THUMBNAIL_PACKET_S orig_packet = {0};
    if(THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, video_path, &orig_packet) != 0) {
        MLOG_DBG("获取视频缩略图失败: %s\n", video_path);
        THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
        return ret;
    }

    if(orig_packet.size == 0) {
        MLOG_DBG("获取的视频缩略图大小为0\n");
        THUMBNAIL_EXTRACTOR_ClearPacket(&orig_packet);
        THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
        return ret;
    }

    // 提取APP3（大缩略图）
    if(THUMBNAIL_EXTRACTOR_GetThumbnailByType(viewer_handle, video_path, &app3_thumbnail, 1) == 0) {
        // 保存大尺寸缩略图
        FILE *fp_large = fopen(output_path_large, "wb");
        if(fp_large) {
            fwrite(orig_packet.data, 1, orig_packet.size, fp_large);
            fclose(fp_large);
            MLOG_DBG("视频大缩略图保存成功: %s\n", output_path_large);
        } else {
            MLOG_DBG("无法创建大缩略图文件: %s\n", output_path_large);
        }
    } else {
        MLOG_ERR("未找到APP3大缩略图\n");
    }

    // 提取APP0（小缩略图）
    if(THUMBNAIL_EXTRACTOR_GetThumbnailByType(viewer_handle, video_path, &app0_thumbnail, 0) == 0) {
        FILE *fp_small = fopen(output_path_small, "wb");
        if(fp_small) {
            fwrite(orig_packet.data, 1, orig_packet.size, fp_small);
            fclose(fp_small);
            MLOG_DBG("视频小缩略图保存成功: %s\n", output_path_small);
            ret = 0; // 成功
        } else {
            MLOG_DBG("无法创建小缩略图文件: %s\n", output_path_small);
        }
    } else {
        MLOG_ERR("未找到APP0小缩略图\n");
    }

    // 清理资源
    THUMBNAIL_EXTRACTOR_ClearPacket(&orig_packet);
    // 清理缩略图数据包
    if(THUMBNAIL_EXTRACTOR_ClearPacket(&app0_thumbnail) != 0) {
        MLOG_ERR("清理APP0缩略图数据包失败\n");
    }
    if(THUMBNAIL_EXTRACTOR_ClearPacket(&app3_thumbnail) != 0) {
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

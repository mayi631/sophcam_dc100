#include "thumbnail_extractor.h"
#include <stdio.h>
#include <string.h>

int main() {
    THUMBNAIL_EXTRACTOR_HANDLE_T extractor = NULL;
    const char *input_file = "input.mov"; // 输入视频文件路径
    THUMBNAIL_PACKET_S thumbnail = {
        .data = NULL,
        .size = 0,
        .pts = 0,
        .duration = 0.0,
        .creationtime = 0,
        .errorccode = {0, 0, 0, 0}
    };

    // 创建缩略图提取器实例
    if (THUMBNAIL_EXTRACTOR_Create(&extractor) != 0) {
        printf("创建缩略图提取器失败\n");
        return -1;
    }
    printf("缩略图提取器创建成功\n");

    // 提取缩略图
    if (THUMBNAIL_EXTRACTOR_GetThumbnail(extractor, input_file, &thumbnail) != 0) {
        printf("提取缩略图失败: %s\n", input_file);
        THUMBNAIL_EXTRACTOR_Destroy(&extractor);
        return -1;
    }
    printf("缩略图提取成功: 大小 = %zu 字节, PTS = %lld\n", thumbnail.size, thumbnail.pts);

    // 将缩略图保存为文件
    const char *output_file = "thumbnail.jpg";
    FILE *output_fp = fopen(output_file, "wb");
    if (output_fp) {
        fwrite(thumbnail.data, 1, thumbnail.size, output_fp);
        fclose(output_fp);
        printf("缩略图已保存到文件: %s\n", output_file);
    } else {
        printf("无法保存缩略图到文件: %s\n", output_file);
    }

    // 清理缩略图数据包
    if (THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail) != 0) {
        printf("清理缩略图数据包失败\n");
    } else {
        printf("缩略图数据包已清理\n");
    }

    // 销毁缩略图提取器实例
    if (THUMBNAIL_EXTRACTOR_Destroy(&extractor) != 0) {
        printf("销毁缩略图提取器失败\n");
    } else {
        printf("缩略图提取器销毁成功\n");
    }

    return 0;
}
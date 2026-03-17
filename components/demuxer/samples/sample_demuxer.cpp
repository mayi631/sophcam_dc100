#include "demuxer.h"
// #include "cvi_log.h"
#include <stdio.h>

int main() {
    DEMUXER_HANDLE_T demuxer = NULL;
    const char *input_file = "sample.mov"; // 输入文件路径
    // DEMUXER_PACKET_S packet;
    DEMUXER_MEDIA_INFO_S media_info;

    // 创建解复用器
    if (DEMUXER_Create(&demuxer) != 0) {
        printf("创建解复用器失败\n");
        return -1;
    }
    printf("解复用器创建成功\n");

    // 设置输入源
    if (DEMUXER_SetInput(demuxer, input_file) != 0) {
        printf("设置输入源失败: %s\n", input_file);
        DEMUXER_Destroy(&demuxer);
        return -1;
    }
    printf("输入源设置为: %s\n", input_file);

    // 打开解复用器
    if (DEMUXER_Open(demuxer) != 0) {
        printf("打开解复用器失败\n");
        DEMUXER_Destroy(&demuxer);
        return -1;
    }
    printf("解复用器打开成功\n");

    // 获取媒体信息
    if (DEMUXER_GetMediaInfo(demuxer, &media_info) == 0) {
        printf("媒体信息:\n");
        printf("  时长: %f ms\n", media_info.duration_sec);
        printf("  视频宽度: %d\n", media_info.width);
        printf("  视频高度: %d\n", media_info.height);
    } else {
        printf("获取媒体信息失败\n");
    }

    // // 读取数据包
    // while (DEMUXER_Read(demuxer, &packet) == 0) {
    //     printf("读取到数据包: size = %d, pts = %lld\n", packet.size, packet.pts);
    //     // 模拟处理数据包
    //     // ...
    // }
    // printf("数据包读取完成\n");

    // 关闭解复用器
    if (DEMUXER_Close(demuxer) != 0) {
        printf("关闭解复用器失败\n");
    } else {
        printf("解复用器关闭成功\n");
    }

    // 销毁解复用器
    if (DEMUXER_Destroy(&demuxer) != 0) {
        printf("销毁解复用器失败\n");
    } else {
        printf("解复用器销毁成功\n");
    }

    return 0;
}
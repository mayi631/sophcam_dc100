#include "muxer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    void *muxer = NULL;
    MUXER_ATTR_S attr = {0};
    const char *output_file = "output.mov";

    // 配置多路复用器属性
    attr.stvideocodec.en = 1; // 启用视频流
    attr.stvideocodec.codec = MUXER_TRACK_VIDEO_CODEC_H264;
    attr.stvideocodec.w = 2560;
    attr.stvideocodec.h = 1440;
    attr.stvideocodec.framerate = 25.0;

    attr.staudiocodec.en = 1; // 启用音频流
    attr.staudiocodec.codec = MUXER_TRACK_AUDIO_CODEC_ADPCM;
    attr.staudiocodec.samplerate = 16000;
    attr.staudiocodec.chns = 2;
    attr.staudiocodec.framerate = 25.0;

    // 创建多路复用器
    if (MUXER_Create(attr, &muxer) != 0) {
        printf("创建多路复用器失败\n");
        return -1;
    }
    printf("多路复用器创建成功\n");

    // 启动多路复用器并创建输出文件
    if (MUXER_Start(muxer, output_file) != 0) {
        printf("启动多路复用器失败\n");
        MUXER_Destroy(muxer);
        return -1;
    }
    printf("多路复用器已启动，输出文件: %s\n", output_file);

    // 模拟写入视频数据包
    MUXER_FRAME_INFO_S video_packet = {0};
    video_packet.type = MUXER_FRAME_TYPE_VIDEO;
    video_packet.dataLen = 1024; // 模拟数据长度
    video_packet.pts = 0; // 时间戳
    video_packet.isKey = 1; // 关键帧
    if (MUXER_WritePacket(muxer, &video_packet) != 0) {
        printf("写入视频数据包失败\n");
    } else {
        printf("写入视频数据包成功\n");
    }

    // 模拟写入音频数据包
    MUXER_FRAME_INFO_S audio_packet = {0};
    audio_packet.type = MUXER_FRAME_TYPE_AUDIO;
    audio_packet.dataLen = 512; // 模拟数据长度
    audio_packet.pts = 0; // 时间戳
    if (MUXER_WritePacket(muxer, &audio_packet) != 0) {
        printf("写入音频数据包失败\n");
    } else {
        printf("写入音频数据包成功\n");
    }

    // 停止多路复用器
    MUXER_Stop(muxer);
    printf("多路复用器已停止\n");

    // 销毁多路复用器
    MUXER_Destroy(muxer);
    printf("多路复用器已销毁\n");

    return 0;
}
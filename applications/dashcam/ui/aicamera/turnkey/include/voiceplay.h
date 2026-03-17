#ifndef __VOICEPLAY_H__
#define __VOICEPLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化音效播放系统
 *
 * 该函数会初始化音频输出设备和音效播放队列，
 * 创建音效播放线程，准备播放音效文件。
 *
 * @return void
 */
void UI_VOICEPLAY_Init(void);

/**
 * @brief 异步清理音效播放系统
 *
 * 该函数用于在后台线程中清理音效播放资源，
 * 包括等待音频播放完成、释放音效播放队列和音频输出设备。
 *
 * @param arg 线程参数（通常为NULL）
 * @return void* 线程返回值（通常为NULL）
 */
void* UI_VOICEPLAY_DeInit(void* arg);

/**
 * @brief 异步清理音效播放系统
 *
 * 该函数用于在后台线程中清理音效播放资源，
 * 包括等待音频播放完成、释放音效播放队列和音频输出设备。
 * 添加了延时
 *
 * @param arg 线程参数（通常为NULL）
 * @return void* 线程返回值（通常为NULL）
 */
void* UI_VOICEPLAY_DeInit_Delay(void* arg);

#ifdef __cplusplus
}
#endif

#endif /* __VOICEPLAY_H__ */
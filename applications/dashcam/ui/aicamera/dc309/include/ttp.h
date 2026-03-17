/* Text To Player */
#ifndef __TTP_H__
#define __TTP_H__

#include <stdint.h>

typedef enum __VOICE_TYPE_E {
    VOICE_TYPE_LONGXIAOCHUN = 0,
    VOICE_TYPE_LONGNAN,
} VOICE_TYPE_E;

typedef enum __TTP_STATUS_E {
    /* 文字转语音空闲状态 */
    TTP_STATUS_TTS_IDLE = 0,
    /* 文字转语音工作状态 */
    TTP_STATUS_TTS_RUNNING,
    /* player空闲状态 */
    TTP_STATUS_PLAYER_IDLE,
    /* player工作状态 */
    TTP_STATUS_PLAYER_RUNNING,
} TTP_STATUS_E;

typedef struct __ttp_status_s {
    /* 文字转语音状态 */
    TTP_STATUS_E tts_status;
    /* player状态 */
    TTP_STATUS_E player_status;
} ttp_status_s;

/**
 * @brief 初始化
 * 创建player线程
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t ttp_init(void);

/**
 * @brief 去初始化
 * 销毁player线程
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t ttp_deinit(void);

/**
 * @brief 播放文本
 * 阻塞播放
 * @param text 待播放的文本
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t ttp_play(const char* text);

/**
 * @brief 复位ttp
 * 在播放的时候用于打断正在执行的播放任务
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t ttp_reset(void);

/**
 * @brief 复位ttp并等待完成
 *
 * @param s 等待超时时间(s)
 * @return int32_t int32_t 错误码, 成功0, 超时-1
 */
int32_t ttp_reset_sync(uint32_t s);

/**
 * @brief 设置播放的音色
 *
 * @param voice 音色种类
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t ttp_set_voice(const char* voice);

/**
 * @brief 获取TTP所处的阶段
 *
 * @param status 状态
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t ttp_get_status(ttp_status_s* status);

#endif /* Text To Player */
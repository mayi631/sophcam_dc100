#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <stdint.h>
#include <pthread.h>
#include "voice_chat/voice_chat.h"

enum {
    CHAT_START_RECORD = 0,
    CHAT_RECORDING,
    CHAT_ASR,
    CHAT_START_PLAY,
    CHAT_PLAYING,
    CHAT_IDLE,
};

typedef void(*AsrFunc)(void);
typedef void(*ChatErrFunc)(int);

/**
 * @brief 创建录音线程, 连接网络
 *
 */
void chat_create_recorder(void);

/**
 * @brief 开始录音
 *
 */
void chat_start_recorder(void);

/**
 * @brief 停止录音
 *
 */
void chat_stop_recorder(void);

/**
 * @brief 摧毁录音线程
 *
 */
void chat_destory_recorder(void);

/**
 * @brief 创建播放线程
 *
 */
 void chat_create_player(void);

/**
 * @brief 开始播放
 *
 */
void chat_start_player(void);

/**
 * @brief 停止播放
 *
 */
void chat_stop_player(void);

/**
 * @brief 销毁播放线程
 *
 */
void chat_destory_player(void);

/**
 * @brief 获取录音时间
 *
 * @param time_ms 时间
 */
void chat_get_recorder_time(uint32_t *time_ms);

/**
 * @brief 获取asr文本
 *
 * @return char* 存放asr文本的指针
 */
char *chat_get_asr_text(void);

/**
 * @brief 获取llm应答的文本
 *
 * @param data 存数据的buf
 * @param len 存数据的buf的长度，函数返回后为获取数据的长度
 * @return int32_t 错误码
 */
int32_t chat_get_llm_text(void* data, uint32_t* len);

/**
 * @brief 进入休眠：断开云端连接
 *
 */
void chat_suspend(void);

/**
 * @brief 唤醒：重新连接云端
 *
 */
void chat_resume(void);

/**
 * @brief 设置工作状态
 *
 * @param state 见CHAT_X
 */
void chat_set_state(uint32_t state);

/**
 * @brief 获取当前工作状态
 *
 * @param state 见CHAT_X
 */
void chat_get_state(uint32_t *state);

/**
 * @brief 设置音色
 *
 * @param voice 音色名称
 */
void chat_set_voice_param(const char *voice);

/**
 * @brief 注册ASR后处理函数, ASR完成后调用
 *
 * @param func 回调函数名
 */
void chat_register_asr_post_callback(AsrFunc func);

/**
 * @brief 注册ASR错误处理函数
 * 
 * @param func 回调函数名
 */
void chat_register_asr_err_process_callback(ChatErrFunc func);

/**
 * @brief 复位chat,在需要打断AI应答时调用
 *
 */
void chat_set_reset(void);

/**
 * @brief 获取复位状态
 *
 * @return int 复位状态: 1表示处于复位, 0表示未复位
 */
int chat_get_reset(void);

#endif /* __DIALOG_H__ */

#ifndef __SPEECH_TO_TEXT_H__
#define __SPEECH_TO_TEXT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file speech_to_text.h
 * @brief sophnet流式语音转文字处理接口
 *
 * 基于sophnet API实现的流式语音转文字功能
 * 支持实时语音识别
 */

// ===============================
// 错误码定义
// ===============================

enum STT_ERROR {
    STT_COMPLETED = 1,
    STT_SUCCESS = 0,
    STT_INVALID_PARAM = -1,
    STT_NETWORK_ERROR = -2,
    STT_API_ERROR = -3,
    STT_PARSE_ERROR = -4,
    STT_BUFFER_TOO_SMALL = -5,
    STT_CONFIG_ERROR = -6,
    STT_INTERNAL_ERROR = -7,
    STT_NOT_CONNECTED = -8,
    STT_TIMEOUT = -9
};

// ===============================
// 参数结构体定义
// ===============================

/**
 * @brief 语音识别参数结构体
 */
typedef struct {
    const char *format;      /* 支持pcm、wav、mp3、opus、speex、aac、amr */
    uint32_t sample_rate;    /* 采样率，任意音频采样率，但16k效果更好 */
    bool heartbeat;          /* 是否开启心跳 */
} stt_params_t;

/**
 * @brief 语音转文字参数结构体
 */
typedef struct {
    const char *project_id;  /* 项目ID */
    const char *api_key;     /* API密钥 */
    const char *easyllm_id;  /* easyllm id */
    stt_params_t stt_params; /* 语音识别参数 */
} speech_to_text_params_t;

// ===============================
// 处理器句柄定义
// ===============================

/**
 * @brief 语音转文字处理器句柄（不透明指针）
 */
typedef struct speech_to_text_processor speech_to_text_processor_t;

// ===============================
// 核心接口函数
// ===============================

/**
 * @brief 创建语音转文字处理器实例
 *
 * @param processor 处理器实例
 * @param params 语音转文字参数
 * @return int 错误码，0表示成功
 */
 int speech_to_text_create(speech_to_text_processor_t **processor, speech_to_text_params_t *params);

/**
 * @brief 发送音频数据
 *
 * @param processor 处理器实例
 * @param input_buffer 输入音频数据缓冲区
 * @param input_size 输入数据大小
 * @return int 错误码，0表示成功
 */
int speech_to_text_send_frame(speech_to_text_processor_t *processor, const void *input_buffer, size_t input_size);

/**
 * @brief 发送结束信号
 *
 * @param processor 处理器实例
 * @return int 错误码，0表示成功
 */
int speech_to_text_send_finish(speech_to_text_processor_t *processor);

/**
 * @brief 获取文本结果
 *
 * @param processor 处理器实例
 * @param text 文本回复缓冲区
 * @param text_size in: 输入缓冲区大小, out: 实际返回文本大小
 * @param is_final out: 是否为最终结果
 * @return int 错误码，0表示成功
 */
int speech_to_text_get_text(speech_to_text_processor_t *processor, char *text, size_t *text_size, bool *is_final);

/**
 * @brief 销毁语音转文字处理器实例
 *
 * @param processor 处理器实例
 */
void speech_to_text_destroy(speech_to_text_processor_t *processor);

// ===============================
// 工具函数
// ===============================

/**
 * @brief 获取默认处理参数
 *
 * @return speech_to_text_params_t 默认参数结构体
 */
speech_to_text_params_t speech_to_text_default_params(void);

/**
 * @brief 验证参数有效性
 *
 * @param params 待验证的参数
 * @return int 0表示有效，其他表示无效
 */
int speech_to_text_validate_params(const speech_to_text_params_t *params);

/**
 * @brief 获取错误描述
 *
 * @param error_code 错误码
 * @return const char* 错误描述字符串
 */
const char *speech_to_text_get_error_string(int error_code);

#ifdef __cplusplus
}
#endif

#endif // __SPEECH_TO_TEXT_H__
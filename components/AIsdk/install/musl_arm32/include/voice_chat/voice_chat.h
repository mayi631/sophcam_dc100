#ifndef __VOICE_CHAT_H__
#define __VOICE_CHAT_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file voice_chat.h
 * @brief sophnet语音对话处理接口
 *
 * 基于sophnet API实现的语音对话功能
 * 支持实时语音识别、文本回复和语音合成
 */

// ================================
// 错误码定义
// ================================

enum VOICE_CHAT_ERROR {
	VOICE_CHAT_CHAT_COMPLETED = 1,
	VOICE_CHAT_SUCCESS = 0,
	VOICE_CHAT_INVALID_PARAM = -1,
	VOICE_CHAT_NETWORK_ERROR = -2,
	VOICE_CHAT_API_ERROR = -3,
	VOICE_CHAT_PARSE_ERROR = -4,
	VOICE_CHAT_BUFFER_TOO_SMALL = -5,
	VOICE_CHAT_CONFIG_ERROR = -6,
	VOICE_CHAT_INTERNAL_ERROR = -7,
	VOICE_CHAT_NOT_CONNECTED = -8,
	VOICE_CHAT_TIMEOUT = -9,
	VOICE_CHAT_BUFFER_EMPTY = -10
};

// ================================
// 参数结构体定义
// ================================

/**
 * @brief 语音识别参数结构体
 */
typedef struct {
	const char *format;      /* 支持pcm、wav、mp3 */
	uint32_t sample_rate;    /* 采样率，如16000 */
} asr_params_t;

/**
 * @brief 大模型参数结构体
 */
typedef struct {
	const char *model_name;  /* 模型名称 */
	const char *prompt;      /* 系统提示词 */
} llm_params_t;

/**
 * @brief 语音合成参数结构体
 */
typedef struct {
	const char *voice;       /* 语音角色，如"longxiaochun" */
	const char *format;      /* 支持pcm、wav、mp3 */
} tts_params_t;

/**
 * @brief 语音对话参数结构体
 */
typedef struct {
	const char *project_id;  /* 项目ID */
	const char *api_key;     /* API密钥 */
	const char *asr_easyllm_id; /* ASR模型ID */
	const char *tts_easyllm_id; /* TTS模型ID */
	asr_params_t asr_params; /* 语音识别参数 */
	llm_params_t llm_params; /* 大模型参数 */
	tts_params_t tts_params; /* 语音合成参数 */
} voice_chat_params_t;

// ================================
// 处理器句柄定义
// ================================

/**
 * @brief 语音对话处理器句柄（不透明指针）
 */
typedef struct voice_chat_processor voice_chat_processor_t;

// ================================
// 核心接口函数
// ================================

/**
 * @brief 创建语音对话处理器实例
 *
 * @param params 语音对话参数
 * @return voice_chat_processor_t* 成功返回处理器实例，失败返回NULL
 */
int voice_chat_create(voice_chat_processor_t **processor, voice_chat_params_t *params);

/**
 * @brief 发送音频数据
 *
 * @param processor 处理器实例
 * @param input_buffer 输入音频数据缓冲区
 * @param input_size 输入数据大小
 * @return int 错误码，0表示成功
 */
int voice_chat_send_frame(voice_chat_processor_t *processor, const void *input_buffer, size_t input_size);

/**
 * @brief 发送结束信号
 *
 * @param processor 处理器实例
 * @return int 错误码，0表示成功
 */
int voice_chat_send_finish(voice_chat_processor_t *processor);

/**
 * @brief 获取文本回复
 *
 * @param processor 处理器实例
 * @param text 文本回复缓冲区
 * @param text_size in: 输入缓冲区大小, out: 实际返回文本大小
 * @return int 错误码，0表示成功
 */
int voice_chat_get_asr_text(voice_chat_processor_t *processor, char *text, size_t *text_size);

/**
 * @brief 获取文本回复
 *
 * @param processor 处理器实例
 * @param text 文本回复缓冲区
 * @param text_size in: 输入缓冲区大小, out: 实际返回文本大小
 * @return int 错误码，0表示成功
 */
int voice_chat_get_llm_text(voice_chat_processor_t *processor, char *text, size_t *text_size);

/**
 * @brief 获取音频回复
 *
 * @param processor 处理器实例
 * @param audio_buffer 音频回复缓冲区
 * @param audio_size in: 输入缓冲区大小, out: 实际返回音频大小
 * @return int 错误码，0表示成功
 */
int voice_chat_get_audio(voice_chat_processor_t *processor, void *audio_buffer, size_t *audio_size);


/**
 * @brief 取消llm+tts, 返回asr
 *
 * @param processor 处理器实例
 * @return int 错误码，0表示成功
 */
int voice_chat_cancel(voice_chat_processor_t* processor);

/**
 * @brief 销毁语音对话处理器实例
 *
 * @param processor 处理器实例
 */
void voice_chat_destroy(voice_chat_processor_t *processor);

// ================================
// 工具函数
// ================================

/**
 * @brief 获取默认处理参数
 *
 * @return voice_chat_params_t 默认参数结构体
 */
voice_chat_params_t voice_chat_default_params(void);

/**
 * @brief 验证参数有效性
 *
 * @param params 待验证的参数
 * @return int 0表示有效，其他表示无效
 */
int voice_chat_validate_params(const voice_chat_params_t *params);

/**
 * @brief 更新参数, 在第一次对话前更新才有效
 *
 * @param processor 处理器实例
 * @param params 需要更新的参数
 * @return int 0表示有效，其他表示无效
 */
int voice_chat_set_params(voice_chat_processor_t *processor, const voice_chat_params_t *params);

/**
 * @brief 获取错误描述
 *
 * @param error_code 错误码
 * @return const char* 错误描述字符串
 */
const char *voice_chat_get_error_string(int error_code);

#ifdef __cplusplus
}
#endif

#endif // __VOICE_CHAT_H__

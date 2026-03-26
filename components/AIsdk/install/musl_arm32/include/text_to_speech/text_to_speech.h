#ifndef TEXT_TO_SPEECH_H
#define TEXT_TO_SPEECH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief 错误码定义
 */
typedef enum {
    TTS_SUCCESS = 0,                    /**< 成功 */
    TTS_ERROR_INVALID_PARAM = -1,       /**< 无效参数 */
    TTS_ERROR_NETWORK = -2,             /**< 网络错误 */
    TTS_ERROR_AUTH = -3,                /**< 认证失败 */
    TTS_ERROR_SERVER = -4,              /**< 服务器错误 */
    TTS_ERROR_MEMORY = -5,              /**< 内存分配失败 */
    TTS_ERROR_EMPTY = -6,              /**< 数据为空 */
    TTS_ERROR_UNKNOWN = -100            /**< 未知错误 */
} tts_error_t;

/**
 * @brief 音频格式定义
 */
typedef enum {
    TTS_FORMAT_MP3_16000HZ_MONO_128KBPS,    /**< MP3 16kHz 单声道 128kbps */
    TTS_FORMAT_PCM_16000HZ_MONO_16BIT      /**< PCM 16kHz 单声道 16bit */
} tts_audio_format_t;

/**
 * @brief 语音合成参数
 */
typedef struct {
    const char* model;          /**< 模型名称，默认"cosyvoice-v1" */
    const char* voice;          /**< 音色，默认"longxiaochun" */
    tts_audio_format_t format;  /**< 音频格式 */
    int volume;                 /**< 音量 [0-100]，默认80 */
    float speech_rate;          /**< 语速 [0.5-2.0]，默认1.2 */
    float pitch_rate;           /**< 语调 [0.5-2.0]，默认1.0 */
} tts_synthesis_param_t;

/**
 * @brief 配置参数
 */
typedef struct {
    const char* project_id;     /**< 项目ID */
    const char* api_key;        /**< API密钥 */
    const char* easyllm_id;     /**< EasyLLM服务ID */
    const char* base_url;       /**< 基础URL，默认为官方地址 */
    int timeout_ms;             /**< 超时时间(毫秒)，默认30000 */
} tts_config_t;

/**
 * @brief 音频数据缓冲区
 */
typedef struct {
    uint8_t* data;      /**< 数据指针 */
    size_t size;        /**< 数据大小 */
    size_t capacity;    /**< 缓冲区容量 */
} tts_audio_buffer_t;

/**
 * @brief 合成单个文本到语音
 * @param config 配置参数
 * @param text 要转换的文本
 * @param param 合成参数
 * @param audio_buffer 用于接收音频数据的缓冲区
 * @return 错误码
 */
tts_error_t tts_synthesize(const tts_config_t* config,
                           const char* text,
                           const tts_synthesis_param_t* param,
                           tts_audio_buffer_t* audio_buffer);

/**
 * @brief 合成单个文本到语音
 * @param config 配置参数
 * @param text 要转换的文本
 * @param param 合成参数
 * @return 错误码
 */
 tts_error_t tts_synthesize_stream(const tts_config_t* config,
                            const char* text,
                            const tts_synthesis_param_t* param);

/**
 * @brief 在流式模式下获取audio stream
 *
 * @param audio_buffer 存放audio stream的缓冲区，有内部分配，需要外部释放
 * @return tts_error_t 错误码
 */
tts_error_t tts_get_audio_stream(tts_audio_buffer_t* audio_buffer);

/**
 * @brief 获取音频队列中音频的数量
 *
 * @param size 队列中音频数量
 * @return tts_error_t 错误码
 */
tts_error_t tts_get_audio_size(uint32_t* size);

/**
 * @brief 在流式模式下检查audio stream是否发送完成
 *
 * @param is_finished 完成标志
 * @return tts_error_t 错误码
 */
tts_error_t tts_check_audio_stream_finished(uint32_t* is_finished);

/**
 * @brief 释放音频缓冲区
 * @param audio_buffer 要释放的缓冲区
 */
void tts_free_audio_buffer(tts_audio_buffer_t* audio_buffer);

/**
 * @brief 获取版本信息
 * @return 版本字符串
 */
const char* tts_get_version(void);

/**
 * @brief 将错误码转换为字符串
 * @param error_code 错误码
 * @return 错误信息字符串
 */
const char* tts_error_to_string(tts_error_t error_code);

#ifdef __cplusplus
}
#endif

#endif // TEXT_TO_SPEECH_H
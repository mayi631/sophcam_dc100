#ifndef __IMAGE_RECOGNIZE_H__
#define __IMAGE_RECOGNIZE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// 错误码定义
enum IMAGE_RECOGNIZE_ERROR {
	IMAGE_RECOGNIZE_SUCCESS = 0,
	IMAGE_RECOGNIZE_INVALID_PARAM = -1,
	IMAGE_RECOGNIZE_FILE_NOT_FOUND = -2,
	IMAGE_RECOGNIZE_NETWORK_ERROR = -3,
	IMAGE_RECOGNIZE_API_ERROR = -4,
	IMAGE_RECOGNIZE_PARSE_ERROR = -5,
	IMAGE_RECOGNIZE_BUFFER_TOO_SMALL = -6,
	IMAGE_RECOGNIZE_CONFIG_ERROR = -7,
	IMAGE_RECOGNIZE_INTERNAL_ERROR = -8
};

// 图片识别器句柄（不透明指针）
typedef struct image_recognizer image_recognizer_t;

/**
 * @brief 创建图片识别器实例
 *
 * @param model_name 模型名称，如"Qwen2.5-VL-72B-Instruct"
 * @param api_key API密钥
 * @param base_url API服务器地址
 * @return image_recognizer_t* 成功返回识别器实例，失败返回NULL
 */
image_recognizer_t *image_recognizer_create(const char *model_name, const char *api_key, const char *base_url);

/**
 * @brief 从图片文件识别物体
 *
 * @param recognizer 识别器实例
 * @param image_path 图片文件路径
 * @param prompt 提示文本，可为NULL则使用默认提示
 * @param result 结果缓冲区，需要保证result_size足够大。
 * @param result_size 结果缓冲区大小
 * @return int 错误码，0表示成功
 */
int image_recognizer_from_file(image_recognizer_t *recognizer, const char *image_path, const char *prompt, char *result,
			       size_t result_size);

/**
 * @brief 从内存缓冲区识别物体
 *
 * @param recognizer 识别器实例
 * @param buffer 图片数据缓冲区
 * @param buffer_size 缓冲区大小
 * @param prompt 提示文本，可为NULL则使用默认提示
 * @param result 结果缓冲区
 * @param result_size 结果缓冲区大小
 * @return int 错误码，0表示成功
 */
int image_recognizer_from_buffer(image_recognizer_t *recognizer, const void *buffer, size_t buffer_size,
				 const char *prompt, char *result, size_t result_size);

/**
 * @brief 销毁图片识别器实例
 *
 * @param recognizer 识别器实例
 */
void image_recognizer_destroy(image_recognizer_t *recognizer);

/**
 * @brief 获取错误描述
 *
 * @param error_code 错误码
 * @return const char* 错误描述字符串
 */
const char *image_recognizer_get_error_string(int error_code);

#ifdef __cplusplus
}
#endif

#endif // __IMAGE_RECOGNIZE_H__

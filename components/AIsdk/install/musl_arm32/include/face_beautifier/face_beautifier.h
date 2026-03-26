/**
 * @file face_beautifier.h
 * @brief 人脸美颜组件接口
 *
 * 基于火山引擎CV接口实现的人脸美颜功能
 * 支持调整美颜程度和处理单人脸/多人脸
 */

#ifndef __FACE_BEAUTIFIER_H__
#define __FACE_BEAUTIFIER_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ================================
// 错误码定义
// ================================

enum FACE_BEAUTIFIER_ERROR {
    FACE_BEAUTIFIER_SUCCESS = 0,           ///< 成功
    FACE_BEAUTIFIER_INVALID_PARAM = -1,    ///< 无效参数
    FACE_BEAUTIFIER_NETWORK_ERROR = -2,    ///< 网络错误
    FACE_BEAUTIFIER_API_ERROR = -3,        ///< API错误
    FACE_BEAUTIFIER_PARSE_ERROR = -4,      ///< 响应解析错误
    FACE_BEAUTIFIER_AUTH_ERROR = -5,       ///< 认证失败
    FACE_BEAUTIFIER_FILE_ERROR = -6,       ///< 文件操作错误
    FACE_BEAUTIFIER_INTERNAL_ERROR = -7,   ///< 内部错误
    
    // 火山引擎服务通用错误码
    FACE_BEAUTIFIER_EC_REQ_INVALID_ARGS = -100,     ///< 参数错误
    FACE_BEAUTIFIER_EC_REQ_MISSING_ARGS = -101,     ///< 缺少参数
    FACE_BEAUTIFIER_EC_PARSE_ARGS = -102,           ///< 参数类型错误/参数缺失
    FACE_BEAUTIFIER_EC_IMAGE_SIZE_LIMITED = -103,   ///< 图像尺寸超过限制
    FACE_BEAUTIFIER_EC_IMAGE_EMPTY = -104,          ///< 请求参数中没有获取到图像
    FACE_BEAUTIFIER_EC_IMAGE_DECODE_ERROR = -105,   ///< 图像解码错误
    FACE_BEAUTIFIER_EC_REQ_BODY_SIZE_LIMITED = -106, ///< 请求Body过大
    FACE_BEAUTIFIER_EC_RPC_PROCESS = -107,          ///< 请求处理失败
    FACE_BEAUTIFIER_EC_FS_LEADER_RISK_ERROR = -108  ///< 输入图片中包含敏感信息，未通过审核
};

// ================================
// 参数结构体定义
// ================================

/**
 * @brief 人脸美颜参数结构体
 */
typedef struct {
    float beauty_level;   ///< 美颜程度，取值范围[0.0, 1.0]，值越大美颜程度越高，默认1.0
    bool multi_face;      ///< 是否处理多人脸，true为处理所有人脸，false只处理最大人脸，默认false
} face_beautifier_params_t;

// ================================
// 处理器句柄定义
// ================================

/**
 * @brief 人脸美颜处理器句柄（不透明指针）
 */
typedef struct face_beautifier_processor face_beautifier_processor_t;

// ================================
// 核心接口函数
// ================================

/**
 * @brief 创建人脸美颜处理器实例
 *
 * @param access_key 火山引擎访问密钥
 * @param secret_key 火山引擎密钥
 * @return face_beautifier_processor_t* 成功返回处理器实例，失败返回NULL
 */
face_beautifier_processor_t *face_beautifier_create(const char *access_key, const char *secret_key);

/**
 * @brief 处理图片文件
 *
 * @param processor 处理器实例
 * @param input_file 输入图片文件路径
 * @param output_file 输出文件路径，结果图片将保存到此文件
 * @param params 处理参数，可为NULL使用默认参数
 * @return int 错误码，0表示成功
 */
int face_beautifier_process_file(face_beautifier_processor_t *processor, 
                                const char *input_file, 
                                const char *output_file,
                                const face_beautifier_params_t *params);

/**
 * @brief 处理内存中的图片数据（Base64编码）
 *
 * @param processor 处理器实例
 * @param input_base64 输入图片的Base64编码数据
 * @param output_file 输出文件路径，结果图片将保存到此文件
 * @param params 处理参数，可为NULL使用默认参数
 * @return int 错误码，0表示成功
 */
int face_beautifier_process_base64(face_beautifier_processor_t *processor, 
                                  const char *input_base64, 
                                  const char *output_file,
                                  const face_beautifier_params_t *params);

/**
 * @brief 销毁人脸美颜处理器实例
 *
 * @param processor 处理器实例
 */
void face_beautifier_destroy(face_beautifier_processor_t *processor);

// ================================
// 工具函数
// ================================

/**
 * @brief 获取默认处理参数
 *
 * @return face_beautifier_params_t 默认参数结构体
 */
face_beautifier_params_t face_beautifier_default_params(void);

/**
 * @brief 根据错误码获取错误信息字符串
 *
 * @param error_code 错误码
 * @return const char* 错误信息字符串
 */
const char *face_beautifier_get_error_string(int error_code);

/**
 * @brief 获取最近一次错误信息
 *
 * @param processor 处理器实例
 * @return const char* 错误信息字符串，处理器无效时返回NULL
 */
const char *face_beautifier_get_last_error(face_beautifier_processor_t *processor);

#ifdef __cplusplus
}
#endif

#endif // __FACE_BEAUTIFIER_H__
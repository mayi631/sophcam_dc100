/* Recoder To Text */
#ifndef __STT_H__
#define __STT_H__

#include <stdint.h>

typedef enum {
    RTT_SUCCESS = 0,
    RTT_ERR_NETWORK = -1,
    RTT_ERR_NO_SPEAKER = -2,
    RTT_ERR_NO_BUF = -3,
} RTT_ERR_CODE_E;

/**
 * @brief 初始化, 创建recoder和asr线程
 *
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t rtt_init(void);

/**
 * @brief 去初始化，销毁recoder和asr线程
 *
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t rtt_deinit(void);

/**
 * @brief 开始录音和asr
 *
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t rtt_start(void);

/**
 * @brief 停止录音和asr
 *
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t rtt_stop(void);

/**
 * @brief 获取asr文本
 *
 * @param text 存放文本的指针, 返回后
 *   该指针指向的地址为当前获取到的所有的文本
 * @return int32_t 错误码, 成功0, 失败RTT_ERR_CODE_E
 */
int32_t rtt_get_text(char **text);

/**
 * @brief 复位为初始状态
 *
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t rtt_reset(void);

/**
 * @brief 当前asr是否结束
 *
 * @return int32_t 错误码, 成功0, 失败-1
 */
int32_t rtt_is_finial(void);

#endif /* Recoder To Text */

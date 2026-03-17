#ifndef __AI_PROCESS_H__
#define __AI_PROCESS_H__

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

enum {
    AI_PROCESS_IDLE = 0, //等待
    AI_PROCESS_START, //处理开始
};

#define DEFALT_RETVAL -1

/* 设置提示词 */
void aiprocess_set_prompt(const char* word);

/* 获取处理结果真实路径 */
char* process_result_get(void);

/* 获取处理结果缩略图路径，带 A: 前缀 */
char* process_result_get_thumbnail(void);

// 线程处理状态设置
void ai_process_state_set(bool state);

//线程主函数
void* thread_ai_process_main(void* arg);

//获取处理返回值
int get_retval(void);

//设置返回值默认状态
void set_defalt_retval(void);

/* 清除处理缓存 */
void aiprocess_clean_cache(void);
#endif /* __DIALOG_H__ */

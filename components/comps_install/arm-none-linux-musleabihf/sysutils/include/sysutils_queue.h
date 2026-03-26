#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef void * QUEUE_HANDLE_T;

QUEUE_HANDLE_T QUEUE_Create(uint32_t nodeSize, uint32_t maxLen);

void QUEUE_Destroy(QUEUE_HANDLE_T queueHdl);

void QUEUE_Clear(QUEUE_HANDLE_T queueHdl);

int32_t QUEUE_GetLen(QUEUE_HANDLE_T queueHdl);

int32_t QUEUE_Push(QUEUE_HANDLE_T queueHdl, const void *node);

int32_t QUEUE_Pop(QUEUE_HANDLE_T queueHdl, void *node);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  // __CVI_QUEUE_H__

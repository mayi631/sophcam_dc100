#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef void *TIMER_HANDLE_T;

typedef void TIMER_PROC_CALLBACK(void *client_data, struct timespec *nowP);

/** timer config info */
typedef struct TIMER_CONF {
    struct timespec *now;       /**< timer create time point32_t */
    long interval_ms;          /**< periodic timer timeout interval or one shot timer timeout interval */
    bool periodic;          /**< periodic or ont-shot */
    TIMER_PROC_CALLBACK *timer_proc; /**< time out call back */
    void *clientData;
} TIMER_S;

int32_t Timer_Init(bool bBlock);
int32_t Timer_DeInit(int32_t grpHdl);
TIMER_HANDLE_T Timer_Create(int32_t grpHdl, TIMER_S *timerConf);
int32_t Timer_Reset(int32_t grpHdl, TIMER_HANDLE_T tmrHdle, struct timespec *timeVal, uint32_t timeLen);
int32_t Timer_SetTickValue(int32_t grpHdl, uint32_t u32TickVal_us);
int32_t Timer_Destroy(int32_t grpHdl, TIMER_HANDLE_T tmrHdle);
int32_t Timer_CleanUp(int32_t grpHdl);
int32_t Timer_SetPeriodicAttr(TIMER_HANDLE_T tmrHdle, bool periodic);
int32_t Timer_GetPastTime(TIMER_HANDLE_T tmrHdle, uint32_t *pu32Time);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
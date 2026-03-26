
#ifndef _PERF_H
#define _PERF_H

#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *PERF_STAT_HANDLE_T;

void PERF_StatInit(PERF_STAT_HANDLE_T *hdl,
        const char *name, uint64_t print_interval);
void PERF_StatDeinit(PERF_STAT_HANDLE_T hdl);
void PERF_StatAdd(PERF_STAT_HANDLE_T hdl, uint64_t val_us);

typedef void *PERF_MARK_HANDLE_T;

void PERF_MarkInit(PERF_MARK_HANDLE_T *hdl,
        const char *name, uint64_t print_interval, uint64_t skip);
void PERF_MarkDeinit(PERF_MARK_HANDLE_T hdl);
void PERF_MarkAdd(PERF_MARK_HANDLE_T hdl, uint64_t time_us);

#ifdef __cplusplus
}
#endif

#endif

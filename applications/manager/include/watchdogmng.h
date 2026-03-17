#ifndef _WATCHDOGMNG_H
#define _WATCHDOGMNG_H

#include "appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t WATCHDOGMNG_Init(int32_t s32Time_s);
int32_t WATCHDOGMNG_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _WATCHDOGMNG_H */


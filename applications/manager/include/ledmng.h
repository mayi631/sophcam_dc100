
/**
* @file    ledmng.h
* @brief   product ledmng struct and interface
* @version   1.0

*/

#ifndef _LEDMNG_H
#define _LEDMNG_H
#include "hal_led.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
void LEDMNG_Control(int32_t control);
int32_t LEDMNG_Init();
int32_t LEDMNG_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#endif /* #ifdef _LEDMNG_H */
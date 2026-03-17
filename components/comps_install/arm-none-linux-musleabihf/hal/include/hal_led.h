/**
* @file    hal_key.h
* @brief   product hal key interface
*/
#ifndef __HAL_LED_H__
#define __HAL_LED_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "hal_gpio.h"

/** \addtogroup     HAL_LED */

typedef enum HAL_LED_STATE_E
{
    HAL_LED_STATE_H = 0,/**<Led high state*/
    HAL_LED_STATE_L, /**<led low state*/
    HAL_LED_STATE_BUIT
} HAL_LED_STATE_E;

typedef enum HAL_LED_IDX_E
{
    HAL_LED_IDX_0 = 0, /**<led index 0*/
    HAL_LED_IDX_1,
    HAL_LED_IDX_BUIT
} HAL_LED_IDX_E;

typedef struct GPIO_ID_E {
	HAL_LED_IDX_E id;
	int32_t gpioid;
} LED_GPIO_ID_E;

int32_t HAL_LED_Init(void);
int32_t HAL_LED_GetState(HAL_LED_IDX_E enKeyIdx, HAL_LED_STATE_E* penKeyState);
int32_t HAL_LED_Deinit(void);
int32_t HAL_LED_SetValue(HAL_GPIO_NUM_E gpio, HAL_GPIO_VALUE_E value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HAL_LED_H__*/
/**
* @file    hal_key.h
* @brief   product hal key interface
*/
#ifndef __HAL_KEY_H__
#define __HAL_KEY_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */
#include "hal_gpio.h"

/** \addtogroup     HAL_KEY */

#define HAL_FD_INITIALIZATION_VAL (-1)
#define HAL_MMAP_LENGTH (0x500)

/* @brief key status enum*/
typedef enum HAL_KEY_STATE_E
{
    HAL_KEY_STATE_DOWN = 0,/**<key down state*/
    HAL_KEY_STATE_UP, /**<key up state*/
    HAL_KEY_STATE_BUIT
} HAL_KEY_STATE_E;

/* @brief key index enum*/
typedef enum HAL_KEY_IDX_E
{
    HAL_KEY_IDX_0 = 0, /**<key index 0*/
    HAL_KEY_IDX_1,
    HAL_KEY_IDX_2,
    HAL_KEY_IDX_3,
    HAL_KEY_IDX_4,
    HAL_KEY_IDX_5,
    HAL_KEY_IDX_6,
    HAL_KEY_IDX_BUIT
} HAL_KEY_IDX_E;

/* @Key GPIO list*/
typedef struct GPIO_ID_E {
	HAL_KEY_IDX_E id;
	int32_t gpioid;
} GPIO_ID_E;

/**
* @brief    hal key initialization, open gpio device and mmap CRG regist address
* @return 0 success,non-zero error code.
*/
int32_t HAL_KEY_Init(void);

/**
* @brief    get hal key state
* @param[in] enKeyIdx: key index
* @param[out] penKeyState:key state
* @return 0 success,non-zero error code.
*/
int32_t HAL_KEY_GetState(HAL_KEY_IDX_E enKeyIdx, HAL_KEY_STATE_E* penKeyState);

/**
* @brief    hal key deinitialization, close gpio device and ummap CRG regist address
* @return 0 success,non-zero error code.
*/
int32_t HAL_KEY_Deinit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HAL_KEY_H__*/

/**
 * @file    hal_gsensor.h
 * @brief   product hal gsensor interface
 *
 * Copyright (c) 2020 Hal Co.,Ltd
 *
 *
 */

#ifndef __HAL_GSENSOR_H__
#define _HAL_GSENSOR_H__
#include <stdint.h>
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


typedef struct cvHAL_GSENSOR_OBJ_S {
	int32_t (*i2c_bus_init)(int32_t busname);
	int32_t (*i2c_bus_deinit)(void);
	int32_t (*init)(void);
	int32_t (*deinit)(void);
	int32_t (*read_data)(short *x, short *y, short *z);
	int32_t (*set_sensitity)(unsigned char sensitity);
	int32_t (*read_int_status)(unsigned char *int_status);
	int32_t (*open_interrupt)(int32_t num);
	int32_t (*read_interrupt)(void);
} HAL_GSENSOR_OBJ_S;

/** screen context */
typedef struct tagHAL_GSESOR_CTX_S {
	HAL_GSENSOR_OBJ_S stGsensorObj;
	uint8_t bRegister;
	uint8_t bInit;
} HAL_GSENSOR_CTX_S;

/* @brief sensitity enum*/
typedef enum cvHAL_GSENSOR_SENSITITY_E {
	HAL_GSENSOR_SENSITITY_OFF = 0, /**<gsensor off*/
	HAL_GSENSOR_SENSITITY_LOW,     /**<low sensitity*/
	HAL_GSENSOR_SENSITITY_MIDDLE,  /**<middle sensitity*/
	HAL_GSENSOR_SENSITITY_HIGH,    /**<high sensitity*/
	HAL_GSENSOR_SENSITITY_BUIT
} HAL_GSENSOR_SENSITITY_E;

/* @brief gesensor value*/
typedef struct cvHAL_GSENSOR_VALUE_S {
	int16_t s16XDirValue; /**<x direction value,unit acceleration of gravity g*/
	int16_t s16YDirValue; /**<y direction value,unit acceleration of gravity g*/
	int16_t s16ZDirValue; /**<z direction value,unit acceleration of gravity g*/
} HAL_GSENSOR_VALUE_S;

/* @brief gesensor chip work attr*/
typedef struct cvHAL_GSENSOR_ATTR_S {
	uint32_t u32SampleRate; /**<sample rate,0 mean Adopt default,not config,unit
                               kps*/
} HAL_GSENSOR_ATTR_S;

typedef int32_t (*HAL_GSENSOR_ON_COLLISION_PFN)(void *pvPrivData);

/** hal gsensor Configure */
typedef struct cvHAL_GSENSOR_CFG_S {
	HAL_GSENSOR_SENSITITY_E enSensitity;
	HAL_GSENSOR_ATTR_S stAttr;
	int32_t busnum;
} HAL_GSENSOR_CFG_S;

int32_t HAL_GSNESOR_ReadInterrupt(void);
int32_t HAL_GSENSOR_Register(const HAL_GSENSOR_OBJ_S *pstGsensorObj);
int32_t HAL_GSENSOR_Init(const HAL_GSENSOR_CFG_S *pstCfg);
int32_t HAL_GSENSOR_SetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity);
int32_t HAL_GSENSOR_SetAttr(const HAL_GSENSOR_ATTR_S *pstAttr);
int32_t HAL_GSENSOR_GetCurValue(HAL_GSENSOR_VALUE_S *pstCurValue);
int32_t HAL_GSENSOR_GetCollisionStatus(uint8_t *pbOnCollison);
int32_t HAL_GSENSOR_OpenInterrupt(int32_t IntNum);
int32_t HAL_GSENSOR_DeInit(void);

/** @}*/ /** <!-- ==== HAL_GSENSOR End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HAL_GSENSOR_H__*/

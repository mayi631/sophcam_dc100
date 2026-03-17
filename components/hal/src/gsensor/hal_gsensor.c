/**
 * @file    hal_gsensor.c
 * @brief   hal gsensor implemention
 *
 * Copyright (c) 2020 Hal.Co.,Ltd
 *
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hal_gsensor.h"
//#include "gsensor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

static HAL_GSENSOR_CTX_S stGsensorCtx;


int32_t HAL_GSENSOR_Register(const HAL_GSENSOR_OBJ_S *pstGsensorObj)
{
	if (stGsensorCtx.bRegister) {
		printf("G-sensor has been registered\n");
		return -1;
	}

	memcpy(&stGsensorCtx.stGsensorObj, pstGsensorObj, sizeof(HAL_GSENSOR_OBJ_S));
	stGsensorCtx.bRegister = 1;

	return 0;
}

int32_t HAL_GSENSOR_Init(const HAL_GSENSOR_CFG_S *pstCfg)
{
	int32_t ret = 0;
	if (stGsensorCtx.bInit) {
		printf("gsensor is inited\n");
		return 0;
	}
	if (stGsensorCtx.stGsensorObj.i2c_bus_init) {
		ret = stGsensorCtx.stGsensorObj.i2c_bus_init(pstCfg->busnum);
	}
//    ret = gsensor_i2c_bus_init(pstCfg->busnum);
	if (ret != 0) {
		printf("/dev/i2c-%d init failed\n", pstCfg->busnum);
	}
	if (stGsensorCtx.stGsensorObj.init) {
		ret = stGsensorCtx.stGsensorObj.init();
	}
	if (ret != 0) {
		return ret;
	} else {
		stGsensorCtx.bInit = 1;
	}
	//ret = gsensor_init();
	return 0;
}

int32_t HAL_GSENSOR_GetCollisionStatus(unsigned char *pbOnCollison)
{
	int32_t ret = 0;
	if (stGsensorCtx.stGsensorObj.read_int_status) {
		ret = stGsensorCtx.stGsensorObj.read_int_status(pbOnCollison);
	}
	//  ret = gsensor_read_int_status(pbOnCollison);
	return ret;
}

int32_t HAL_GSENSOR_OpenInterrupt(int32_t IntNum)
{
	int32_t ret;
	ret = stGsensorCtx.stGsensorObj.open_interrupt(IntNum);
	if (ret) {
		printf("Open Gsensor Interrupt Failed%d \n", ret);
	}
	return ret;
}

int32_t HAL_GSNESOR_ReadInterrupt(void)
{
	int32_t ret;
	ret = stGsensorCtx.stGsensorObj.read_interrupt();
	return ret;
}

int32_t HAL_GSENSOR_SetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity)
{
	int32_t ret = 0;
	uint8_t thrd;
	if ((enSensitity != HAL_GSENSOR_SENSITITY_OFF) &&
	    (enSensitity != HAL_GSENSOR_SENSITITY_LOW) &&
	    (enSensitity != HAL_GSENSOR_SENSITITY_MIDDLE) &&
	    (enSensitity != HAL_GSENSOR_SENSITITY_HIGH)) {
		return -1;
	}
	switch (enSensitity) {
	case HAL_GSENSOR_SENSITITY_LOW:
		thrd = 0x50;//0x80;
		break;
	case HAL_GSENSOR_SENSITITY_MIDDLE:
		thrd = 0x10;//0x80;
		break;
	case HAL_GSENSOR_SENSITITY_HIGH:
		thrd = 0x05;//0x20;
		break;
	default:
		thrd = 0x7e;//0xFF;
		break;
	}
	if (stGsensorCtx.stGsensorObj.set_sensitity) {
		ret = stGsensorCtx.stGsensorObj.set_sensitity(thrd);
	}
	//ret = gsensor_set_sensitity(thrd);

	return ret;
}

int32_t HAL_GSENSOR_GetCurValue(HAL_GSENSOR_VALUE_S *pstCurValue)
{
	int32_t ret = 0;
	if (stGsensorCtx.stGsensorObj.read_data) {
		ret = stGsensorCtx.stGsensorObj.read_data(&pstCurValue->s16XDirValue,
				&pstCurValue->s16YDirValue,
				&pstCurValue->s16ZDirValue);
	}
	return ret;
}

int32_t HAL_GSENSOR_SetAttr(const HAL_GSENSOR_ATTR_S *pstAttr)
{
	(void) pstAttr;
	return 0;
}

int32_t HAL_GSENSOR_DeInit(void)
{
	int32_t ret = 0;
	if (stGsensorCtx.stGsensorObj.deinit) {
		ret = stGsensorCtx.stGsensorObj.deinit();
	}
	//gsensor_deinit();
	if (stGsensorCtx.stGsensorObj.i2c_bus_deinit) {
		ret = stGsensorCtx.stGsensorObj.i2c_bus_deinit();
	}
	//gsensor_i2c_bus_deinit();
	stGsensorCtx.bInit = 0;
	return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

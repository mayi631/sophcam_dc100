/**
* @file    hal_key.c
* @brief   HAL key implemention
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "hal_key.h"
#include "hal_adc.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#define ADC_CHANNEL1_KEY   (1)
#define ADC_CH_KEY_VALUE_RANGE (200)

static int32_t Vkey_Get_Value(void)
{
    int32_t ret = HAL_ADC_GetValue(ADC_CHANNEL1_KEY);
    if (-1 == ret) {
        CVI_LOGE("HAL_ADC_GetValue fail\n");
        return -1;
    }
    return ret;
}

static int32_t ADCKey_Get_Value(int32_t val, HAL_GPIO_VALUE_E *value)
{
    int32_t ret = Vkey_Get_Value();
    if (-1 == ret) {
        CVI_LOGE("Vkey_Get_Value fail\n");
        return -1;
    }

    if ((val <= ret + ADC_CH_KEY_VALUE_RANGE) && (val >= ret - ADC_CH_KEY_VALUE_RANGE)) {
        *value = HAL_GPIO_VALUE_L;//HAL_GPIO_VALUE_H;
    }
    else {
        *value = HAL_GPIO_VALUE_H;//HAL_GPIO_VALUE_L;
    }

    return 0;
}

/** macro define */
static bool key_init = false;
#if 0
static GPIO_ID_E GPIOID[] = {{HAL_KEY_IDX_0, HAL_GPIOE_08},
                          {HAL_KEY_IDX_1, HAL_GPIOE_14},
                          {HAL_KEY_IDX_2, HAL_GPIOB_04},
                          {HAL_KEY_IDX_3, HAL_GPIOE_17}};
#else
// for adc1 values using gpioid int32_t && PWR_GPIO_7&8 is gpio (PWR_GPIO_7:pwr_wakeup1:POWER_ON/OFF_DET;PWR_GPIO_8:pwr_button1)
static GPIO_ID_E GPIOID[] = {{HAL_KEY_IDX_0, HAL_GPIOE_08},
                          {HAL_KEY_IDX_1, 200},
                          {HAL_KEY_IDX_2, 1024},
                          {HAL_KEY_IDX_3, 2220}};
#endif

int32_t HAL_KEY_Init(void)
{
    //int32_t gpiocunt = 0;
    int32_t s32Ret = 0;
    CVI_LOGI("HAL_KEY_Init KEY\n");
	if (key_init == false) {
		key_init = true;

        //key in adc1 init
        s32Ret = HAL_ADC_Init();
        if (0 != s32Ret) {
            CVI_LOGE("HAL_ADC_Init KEY Failed\n");
            return -1;
        }
        //gpio
        s32Ret = HAL_GPIO_Direction_Input(GPIOID[0].gpioid);
        if (0 != s32Ret) {
            CVI_LOGE("[Error]set gpio dir failed\n");
            return -1;
        }

	} else {
        CVI_LOGE("key already init\n");
        return -1;
    }
    return 0;
}


int32_t HAL_KEY_GetState(HAL_KEY_IDX_E enKeyIdx, HAL_KEY_STATE_E* penKeyState)
{
    uint32_t u32Val;
    int32_t s32Ret = 0;
    int32_t i = 0;
    int32_t gpiocunt = 0;
    int32_t flage = 0;
    gpiocunt = sizeof(GPIOID)/sizeof(GPIOID[0]);

    /* init check */
    if (key_init == false) {
        CVI_LOGE("key not initialized\n");
        return -1;
    }

    /* parm penKeyState check */
    if (NULL == penKeyState) {
        CVI_LOGE("penKeyState is null\n");
        return -1;
    }

    //for adc1 read key form array 1
    for (i = 1; i < gpiocunt; i++) {
        if (enKeyIdx == GPIOID[i].id) {
            s32Ret = ADCKey_Get_Value(GPIOID[i].gpioid, &u32Val);
            if (0 != s32Ret) {
                CVI_LOGE("[Error]read adc data failed\n");
                return -1;
            }
            flage = 1;
            break;
        }
    }
    //gpio key power key
    if (enKeyIdx == GPIOID[0].id) {
        s32Ret = HAL_GPIO_Get_Value(GPIOID[0].gpioid, &u32Val);
        if (0 != s32Ret) {
            CVI_LOGE("[Error]read gpio data failed\n");
            return -1;
        }
        flage = 1;
    }

    if (0 == flage) {
        CVI_LOGE("illeagel enkey(%d) out of range\n",enKeyIdx);
        return -1;
    }

    *penKeyState = (1 == u32Val) ? HAL_KEY_STATE_UP : HAL_KEY_STATE_DOWN;

    return 0;
}

int32_t HAL_KEY_Deinit(void)
{
    if (key_init == false) {
        CVI_LOGE("key not initialized,no need to close\n");
        return -1;
    }

    key_init = 0;
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */



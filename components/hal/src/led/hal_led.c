/**
* @file    hal_led.c
* @brief   HAL led implemention
*/

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>
#include "hal_led.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

/** macro define */

static bool led_init = false;
static LED_GPIO_ID_E GPIOID[] = {{HAL_LED_IDX_0, HAL_GPIOA_28},
                                    {HAL_LED_IDX_1, HAL_GPIOA_29}};

int32_t HAL_LED_Init(void)
{
    int32_t gpiocunt = 0;
    int32_t s32Ret = 0;
	if (led_init == false) {
		led_init = true;
        gpiocunt = sizeof(GPIOID)/sizeof(GPIOID[0]);
        for (int32_t i = 0; i < gpiocunt; i++) {
            s32Ret = HAL_GPIO_Direction_Input(GPIOID[i].gpioid);
            if (0 != s32Ret) {
                CVI_LOGE("[Error]set gpio dir failed\n");
                return -1;
            }
        }
	} else {
        CVI_LOGE("led already init\n");
        return -1;
    }
    return 0;
}


int32_t HAL_LED_GetState(HAL_LED_IDX_E enLedIdx, HAL_LED_STATE_E* penLedState)
{
    u_int32_t u32Val;
    int32_t s32Ret = 0;
    int32_t i = 0;
    int32_t gpiocunt = 0;
    int32_t flage = 0;
    gpiocunt = sizeof(GPIOID)/sizeof(GPIOID[0]);

    /* init check */
    if (led_init == false) {
        CVI_LOGE("led not initialized\n");
        return -1;
    }

    /* parm penLedState check */
    if (NULL == penLedState) {
        CVI_LOGE("penLedState is null\n");
        return -1;
    }

    for (i = 0; i < gpiocunt; i++) {
        if (enLedIdx == GPIOID[i].id) {
            s32Ret = HAL_GPIO_Get_Value(GPIOID[i].gpioid, &u32Val);
            if (0 != s32Ret) {
                CVI_LOGE("[Error]read gpio data failed\n");
                return -1;
            }
            flage = 1;
            break;
        }
    }

    if (0 == flage) {
        CVI_LOGE("illeagel enLed(%d) out of range\n",enLedIdx);
        return -1;
    }

    *penLedState = (1 == u32Val) ? HAL_LED_STATE_H : HAL_LED_STATE_L;

    return 0;
}

int32_t HAL_LED_Deinit(void)
{
    if (led_init == false) {
        CVI_LOGE("led not initialized,no need to close\n");
        return -1;
    }

    led_init = 0;
    return 0;
}

int32_t HAL_LED_SetValue(HAL_GPIO_NUM_E gpio, HAL_GPIO_VALUE_E value)
{
    int32_t ret = 0;
    ret = HAL_GPIO_Set_Value(gpio, value);
    if (0 != ret) {
        CVI_LOGE("set led value error\n");
        return ret;
    }
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
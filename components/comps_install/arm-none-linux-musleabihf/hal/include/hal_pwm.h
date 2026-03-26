#ifndef __PWM_H__
#define __PWM_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct HAL_PWM_S {
	uint8_t group;       //组号
	uint8_t channel;      //通道号
	uint32_t period;      //周期
	uint32_t duty_cycle;  //占空比
} HAL_PWM_S;

int32_t HAL_PWM_Init(HAL_PWM_S pwmAttr);
int32_t HAL_PWM_Deinit(HAL_PWM_S pwmAttr);
int32_t HAL_PWM_Get_Percent(void);
int32_t HAL_PWM_Set_Percent(int32_t percentage);
int32_t HAL_PWM_Set_Param(HAL_PWM_S pwmAttr);
int32_t HAL_PWM_Get_Param(uint32_t *period, uint32_t *duty_cycle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __PWM_H__ */

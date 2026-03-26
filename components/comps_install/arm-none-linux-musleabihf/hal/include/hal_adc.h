#ifndef __HAL_ADC_H__
#define __HAL_ADC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t HAL_ADC_Init(void);
int32_t HAL_ADC_Deinit(void);
int32_t HAL_ADC_GetValue(int32_t int_adc_channel);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __ADC_H__ */

#ifndef __GAUGERMNG_H__
#define __GAUGERMNG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include "appcomm.h"

typedef enum EVENT_GAUGEMNG_E
{
    EVENT_GAUGEMNG_LEVEL_CHANGE = APPCOMM_EVENT_ID(APP_MOD_GAUGEMNG, 0), /**<refresh current count of electric quantity*/
    EVENT_GAUGEMNG_LEVEL_LOW,        /**<low level , an alarm show*/
    EVENT_GAUGEMNG_LEVEL_ULTRALOW,   /**<ultra low level , power off*/
    EVENT_GAUGEMNG_LEVEL_NORMAL,      /**<after charging,restore normal*/
    EVENT_GAUGEMNG_CHARGESTATE_CHANGE,      /**<after charging,restore normal*/
    EVENT_GAUGEMNG_BUIT
} EVENT_GAUGEMNG_E;

/** gauge mng configure */
typedef struct GAUGEMNG_CFG_S
{
    int32_t s32LowLevel; /**< in percent */
    int32_t s32UltraLowLevel; /**< in percent */
    int32_t s32ADCChannelVbat; /**< ADC channel of VBAT */
    int32_t s32USBChargerDetectGPIO; /**< GPIO number for USB charger detection */
} GAUGEMNG_CFG_S;

int32_t GAUGEMNG_GetPercentage(void);
int32_t GAUGEMNG_Init(const GAUGEMNG_CFG_S* pstCfg);
int32_t GAUGEMNG_GetBatteryLevel(uint8_t* ps32Level);
int32_t GAUGEMNG_GetChargeState(bool* pbCharge);
int32_t GAUGEMNG_DeInit(void);
int32_t GAUGEMNG_RegisterEvent(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __GAUGEMNG_H__ */
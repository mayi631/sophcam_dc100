
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>
#include <stdbool.h>
#include "cvi_log.h"
#include "mapi_ao.h"
#include "hal_gpio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum SYSTEM_STARTUP_SRC_E
{
    SYSTEM_STARTUP_SRC_GSENSORWAKEUP = 0,   /* gsensor wake up*/
    SYSTEM_STARTUP_SRC_USBWAKEUP,           /* usb wake up*/
    SYSTEM_STARTUP_SRC_STARTUP,             /* power key start up*/
    SYSTEM_STARTUP_SRC_BUTT
} SYSTEM_STARTUP_SRC_E;

typedef struct tagCVI_SYSTEM_TM_S {
    int32_t s32year;
    int32_t s32mon;
    int32_t s32mday;
    int32_t s32hour;
    int32_t s32min;
    int32_t s32sec;
} SYSTEM_TM_S;

#define DATETIME_MAX_YEAR       2037
#define DATETIME_DEFAULT_YEAR   2024
#define DATETIME_DEFAULT_MONTH     1
#define DATETIME_DEFAULT_DAY       1
#define DATETIME_DEFAULT_HOUR      0
#define DATETIME_DEFAULT_MINUTE    0
#define DATETIME_DEFAULT_SECOND    0

#define POWER_WAKEUP_SOURCE 0x05026000
#define PWR_WAKEUP0_PIN_MEM 0x03001090
#define GSENSOR_WAKEUP_SOURCE HAL_GPIOE_06
#define USB_WAKEUP_SOURCE HAL_GPIOB_06
#define GSENSOR_PWR_WAKEUP0_VALUE 0
#define GSENSOR_GPIO_WAKEUP0_VALUE 3

int32_t SYSTEM_SetDateTime(const SYSTEM_TM_S* pstDateTime);
int32_t SYSTEM_GetRTCDateTime(SYSTEM_TM_S* pstDateTime);
int32_t SYSTEM_SetDefaultDateTime(void);
void SYSTEM_Reboot(int32_t need_sync);
int32_t SYSTEM_GetStartupWakeupSource(SYSTEM_STARTUP_SRC_E* penStartupSrc);
int32_t SYSTEM_BootSound(MAPI_AO_HANDLE_T AoHdl);
int32_t SYSTEM_SetGpioWakeup(HAL_GPIO_NUM_E gpio, uint32_t value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif
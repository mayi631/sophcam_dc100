/**
* @file    hal_gps.h
* @brief   product hal gps interface
*
*
* @author    hal team
* @date      2021/6/19
* @version

*/
#ifndef __HAL_GPS_INNER_H__
#define __HAL_GPS_INNER_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#include "hal_gps.h"

typedef struct halGPSHAL_DEVICE
{
   int32_t (*Init)(void);
   int32_t (*DeInit)(void);
   int32_t (*GetRawData)(GPSDATA_S *gpsData, int32_t timeout_ms);
}HAL_GPSHAL_DEVICE;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_INNER_H__*/

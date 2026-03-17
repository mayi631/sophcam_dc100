/**
* @file    hal_gps.h
* @brief    hal gps interface
*
*
* @author    hal team
* @date      2021/6/19
* @version

*/
#ifndef __HAL_GPS_H__
#define __HAL_GPS_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#define HAL_GPS_DATA_SIZE 1

typedef struct GPSDATA
{
    uint32_t wantReadLen;   /**want read length */
    uint32_t actualReadLen; /**actual read length */
    unsigned char  rawData[HAL_GPS_DATA_SIZE];
} GPSDATA_S;

int32_t HAL_GPS_Init(void);

int32_t HAL_GPS_Deinit(void);

int32_t HAL_GPS_GetRawData(GPSDATA_S *gpsData, int32_t timeout_ms);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HAL_GPS_H__*/

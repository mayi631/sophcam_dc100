/**
* @file    hal_dummy_gps.c
* @brief   product gps interface
*
*
* @author    hal team
* @date      2021/6/19
* @version

*/
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>

#include "hal_gps_inner.h"
#include "cvi_log.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define GPS_INFO_TYPE 9
#define HAL_GPS_STR_LEN 120

static int32_t g_gpsInfoTypeIndex = 0;
static int32_t g_gpsStrLenIndex = 0;

static char g_gpsInfo[GPS_INFO_TYPE][HAL_GPS_STR_LEN] =
{
    {"$GPRMC,081836,A,3751.65,S,14507.36,E,000.0,360.0,130998,011.3,E*62,\n"},
    {"$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47,\n"},
    {"$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39,\n"},
    {"$GPGLL,3723.2475,N,12158.3416,W,161229.487,A,A*41,\n"},
    {"$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K*48,\n"},
    {"$GPGSV,4,1,11,03,03,111,00,04,15,270,00,06,01,010,00,13,06,292,00*74,\n"},
    {"$GPGSV,4,2,11,22,42,067,42,24,14,311,43,27,05,244,00,,,,*4D,\n"},
    {"$GPGSV,4,3,11,08,51,203,30,09,45,215,28*75,\n"},
    {"$GPGSV,4,4,13,31,07,126,*4B,\n"},
};

static int32_t GPSDummyInit(void)
{
    CVI_LOGI("GPSDummyInit\n");
    return 0;
}

static int32_t GPSDummyDeInit(void)
{
    CVI_LOGI("GPSDummyDeInit\n");
    return 0;
}

static int32_t GPSDummyGetRawData(GPSDATA_S* gpsData, int32_t timeout_ms)
{
    if (NULL == gpsData)
    {
        CVI_LOGE("gpsData is NULL pointer!\n");
        return -1;
    }

    gpsData->actualReadLen = 1;

    if (g_gpsStrLenIndex < (int32_t)(strlen(g_gpsInfo[g_gpsInfoTypeIndex]) + 1)) {
        memcpy(gpsData->rawData, &g_gpsInfo[g_gpsInfoTypeIndex][g_gpsStrLenIndex], gpsData->actualReadLen);
        g_gpsStrLenIndex++;
    }

    /* array[i] tail */
    if (g_gpsStrLenIndex == (int32_t)(strlen(g_gpsInfo[g_gpsInfoTypeIndex]) + 1)) {
        /* if not is laset array[i] index */
        if (g_gpsInfoTypeIndex < GPS_INFO_TYPE)
        {
            g_gpsInfoTypeIndex++;
            usleep(10 * 1000);
        }

        if (g_gpsInfoTypeIndex != GPS_INFO_TYPE)
        {
            g_gpsStrLenIndex = 0;
        }

        /* array tail */
        if (g_gpsInfoTypeIndex == GPS_INFO_TYPE)
        {
            g_gpsInfoTypeIndex = 0;
            g_gpsStrLenIndex = 0;
            usleep(920 * 1000);
            return -110;
        }
    }

    return 0;
}

HAL_GPSHAL_DEVICE g_gpsDevice =
{
    .Init = GPSDummyInit,
    .DeInit = GPSDummyDeInit,
    .GetRawData = GPSDummyGetRawData,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

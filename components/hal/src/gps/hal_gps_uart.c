/**
* @file    hal_gps_uart.c
* @brief   product gps hal uart interface
*
*
* @author    hal team
* @date      2021/6/19
* @version
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>
#include <errno.h>
#include <termios.h>

#include "cvi_log.h"
#include "hal_gps_inner.h"
#include "hal_uart.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HAL_GPS_UART_DEVICE  "/dev/ttyS1"

static int32_t g_uart_init = 0;

static int32_t GpsDeviceInit(void)
{
    int32_t ret = 0;

    if (g_uart_init != 0)
    {
        CVI_LOGI("GPS already init\n");
        return 0;
    }

    ret = UART_Init(HAL_GPS_UART_DEVICE);
    if(0 > ret)
    {
        CVI_LOGI("uart init failed\n");
        return -1;
    }

    ret = UART_Set_Param(B9600, 0, 8, 1, 'n');
    if(0 != ret)
    {
        CVI_LOGI("uart set attr failed\n");
        return ret;
    }
    g_uart_init = 1;

    return 0;
}

static int32_t GpsDeviceDeInit(void)
{
    int32_t ret = 0;
    if (g_uart_init == 0)
    {
        CVI_LOGE("Device has already deinit or Uninitialized!\n");
        return 0;
    }

    ret = UART_Exit();
    if(0 != ret)
    {
        CVI_LOGI("uart Deinit failed\n");
        return ret;
    }

    g_uart_init = 0;
    return 0;
}

static int32_t GpsGetRawData(GPSDATA_S *gpsData, int32_t timeout_ms)
{
    int32_t ret = 0;

    if (gpsData == NULL)
    {
        CVI_LOGE("gpsData is NULL pointer!\n");
        return -12;
    }

    if (gpsData->wantReadLen > HAL_GPS_DATA_SIZE)
    {
        CVI_LOGE("Too much data acquired at one time len:%d!\n", gpsData->wantReadLen);
        return -12;
    }

    if (g_uart_init == 0)
    {
        CVI_LOGE("gps uart is not inited!\n");
        return -1;
    }

    ret = UART_Receive(gpsData->rawData, gpsData->wantReadLen, timeout_ms);
    if ( ret > 0 ) {
        gpsData->actualReadLen = ret;
        ret = 0;
    } else {
        if (ret != -110)
            CVI_LOGE("Receive gpsData failed\n");
    }

    return ret;
}

HAL_GPSHAL_DEVICE g_gpsDevice =
{
    .Init = GpsDeviceInit,
    .DeInit = GpsDeviceDeInit,
    .GetRawData = GpsGetRawData,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

/**
* @file    hal_gps.c
* @brief   product gps interface
*
* Copyright (c) 2018 Huawei Tech.Co.,Ltd
*
* @author    Huawei team
* @date      2018/12/19
* @version

*/
#include "cvi_log.h"
#include "hal_gps.h"
#include "hal_gps_inner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern HAL_GPSHAL_DEVICE  g_gpsDevice;

int32_t HAL_GPS_Init(void)
{
    int32_t ret = 0;

    ret = g_gpsDevice.Init();
    if(0 != ret)
    {
        CVI_LOGE("Gps Device Init failed\n");
        return ret;
    }

    return 0;
}

int32_t HAL_GPS_Deinit(void)
{
    int32_t ret = 0;

    ret = g_gpsDevice.DeInit();

    if (0 != ret)
    {
        CVI_LOGE("Gps Device DeInit failed\n");
        return ret;
    }

    return 0;
}

int32_t HAL_GPS_GetRawData(GPSDATA_S *gpsData, int32_t timeout_ms)
{
    int32_t ret = 0;

    ret = g_gpsDevice.GetRawData(gpsData, timeout_ms);
    if (0 != ret)
    {
        return ret;
    }

    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

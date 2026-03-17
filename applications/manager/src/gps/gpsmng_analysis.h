#ifndef __GPSMNG_ANALYSIS_H__
#define __GPSMNG_ANALYSIS_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */


#include"gpsmng.h"

#define GPSMNG_MESSAGE_MAX_LEN (120)

/* GPS RAW DATA*/
typedef struct tagGPSMNG_RAW_DATA
{
    char ggaStr[GPSMNG_MESSAGE_MAX_LEN];
    char gllStr[GPSMNG_MESSAGE_MAX_LEN];
    char gsaStr[GPSMNG_MESSAGE_MAX_LEN];
    char rmcStr[GPSMNG_MESSAGE_MAX_LEN];
    char vtgStr[GPSMNG_MESSAGE_MAX_LEN];
    char gsvStr[GPSMNG_GSV_MAX_MSG_NUM][GPSMNG_MESSAGE_MAX_LEN];
}GPSMNG_RAW_DATA;

int32_t nmea_parse_rawdata(GPSMNG_RAW_DATA* gpsRawData, GPSMNG_MSG_PACKET* gpsMsgPack);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HI_HAL_GPS_H__*/

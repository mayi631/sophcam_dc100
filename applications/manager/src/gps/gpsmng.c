/**
* @file    gpsmng.c
* @brief   product gpsmng function
*
*
* @author    team
* @date      2021/10/18
* @version
*/
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <unistd.h>
#include "gpsmng.h"
#include "gpsmng_analysis.h"
#include "hal_gps.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

//#define GPSMNG_DEBUG
#define GPSMNG_GET_DATA_TIMEOUT_MS 300

/*  config for user,such as: config bit0 is 1 Represents the need to have gga raw data before it can be parsed
 *  GPSMNG_RAW_DATA_MASK:|bit7 -- bit6 -- bit5 -- bit4 -- bit3 -- bit2 -- bit1 -- bit0|
 *                                       GPGSV   GPVTG   GPRMC   GPGSA   GPGLL   GPGGA
 */

#define GPSMNG_RAW_DATA_MASK 0X1f /* contain GPVTG && GPRMC && GPGSA && GPGLL && GPGGA */


typedef enum  tagGPSMNG_STATE
{
    GPSMNG_STATE_DEINITED,
    GPSMNG_STATE_INITED,
    GPSMNG_STATE_STARTED,
    GPSMNG_STATE_STOP,
    GPSMNG_STATE_BUTT,
} GPSMNG_STATE;

typedef struct tagGPSMNG_CONTEXT
{
    bool gpsReadFlg;
    bool gpsRefreshData;
    pthread_t gpsReadThd;
    pthread_mutex_t gpsLock;
    GPSMNG_STATE gpsMngState;
    GPSMNG_CALLBACK fnGpsCB[GPSMNG_CALLBACK_MAX_NUM];
} GPSMNG_CONTEXT;

typedef struct tagGPSMNG_RAWDATA_BUFF
{
    int32_t msgCount;
    int32_t gpsGPGSVMsgTotalNum;
    int32_t gpsGPGSVMsgCurNum;
    GPSMNG_RAW_DATA gpsRawData;
} GPSMNG_RAWDATA_BUFF;

static GPSMNG_CONTEXT g_gpsMngCtx;
static GPSMNG_RAWDATA_BUFF g_gpsRawdataBuff;
static GPSMNG_MSG_PACKET g_gpsMngPacket;

/* Refresh a complete set of gps data to buf */
static int32_t GPSMNG_MsgProc(char* msgStr)
{
    if (msgStr == NULL)
    {
        CVI_LOGW("Gps Message String is NULL\n");
        return -1;
    }

    if (NULL != strstr(msgStr, "GPGGA") || NULL != strstr(msgStr, "GNGGA"))
    {
        snprintf(g_gpsRawdataBuff.gpsRawData.ggaStr, GPSMNG_MESSAGE_MAX_LEN, "%s", msgStr);
        g_gpsRawdataBuff.msgCount |= 1 << 0;
    }
    else if (NULL != strstr(msgStr, "GPGLL") || NULL != strstr(msgStr, "GNGLL"))
    {
        snprintf(g_gpsRawdataBuff.gpsRawData.gllStr, GPSMNG_MESSAGE_MAX_LEN, "%s", msgStr);
        g_gpsRawdataBuff.msgCount |= 1 << 1;
    }
    else if (NULL != strstr(msgStr, "GPGSA") || NULL != strstr(msgStr, "GNGSA"))
    {
        snprintf(g_gpsRawdataBuff.gpsRawData.gsaStr, GPSMNG_MESSAGE_MAX_LEN, "%s", msgStr);
        g_gpsRawdataBuff.msgCount |= 1 << 2;
    }
    else if (NULL != strstr(msgStr, "GPRMC") || NULL != strstr(msgStr, "GNRMC"))
    {
        snprintf(g_gpsRawdataBuff.gpsRawData.rmcStr, GPSMNG_MESSAGE_MAX_LEN, "%s", msgStr);
        g_gpsRawdataBuff.msgCount |= 1 << 3;
    }
    else if (NULL != strstr(msgStr, "GPVTG") || NULL != strstr(msgStr, "GNVTG"))
    {
        snprintf(g_gpsRawdataBuff.gpsRawData.vtgStr, GPSMNG_MESSAGE_MAX_LEN, "%s", msgStr);
        g_gpsRawdataBuff.msgCount |= 1 << 4;
    }
    else if (NULL != strstr(msgStr, "GPGSV") || NULL != strstr(msgStr, "GNGSV") || NULL != strstr(msgStr, "GLGSV"))
    {
        int32_t totalMsgNum = 0;
        int32_t curMsgNum = 0;
        sscanf(msgStr + strlen("$GPGSV,"), "%d,%d,", &totalMsgNum, &curMsgNum);

        /* void gsv num 4 str which have random wrong string */
        if (curMsgNum <= GPSMNG_GSV_MAX_MSG_NUM)
        {
            g_gpsRawdataBuff.gpsGPGSVMsgTotalNum = (totalMsgNum > GPSMNG_GSV_MAX_MSG_NUM) ? GPSMNG_GSV_MAX_MSG_NUM : totalMsgNum;
            g_gpsRawdataBuff.gpsGPGSVMsgCurNum = curMsgNum;
            snprintf(g_gpsRawdataBuff.gpsRawData.gsvStr[curMsgNum - 1], GPSMNG_MESSAGE_MAX_LEN, "%s", msgStr);

            if (g_gpsRawdataBuff.gpsGPGSVMsgTotalNum == g_gpsRawdataBuff.gpsGPGSVMsgCurNum)
            {
                g_gpsRawdataBuff.msgCount |= 1 << 5;
            }
        }
    }

    return 0;
}

static int32_t GPS_DataProc(void)
{
    int32_t i;
    int32_t ret = 0;

    ret = nmea_parse_rawdata(&g_gpsRawdataBuff.gpsRawData, &g_gpsMngPacket);
    if (ret != 0)
    {
        CVI_LOGE("Parser GPS Raw data failed!\n");
    }

    pthread_mutex_lock(&g_gpsMngCtx.gpsLock);

    for (i = 0; i < GPSMNG_CALLBACK_MAX_NUM; i++)
    {
        if (NULL != g_gpsMngCtx.fnGpsCB[i].fnGpsDataCB)
        {
            g_gpsMngCtx.fnGpsCB[i].fnGpsDataCB(&g_gpsMngPacket, g_gpsMngCtx.fnGpsCB[i].privateData);
        }
    }

    memset(&g_gpsRawdataBuff, 0x0, sizeof(GPSMNG_RAWDATA_BUFF));
    pthread_mutex_unlock(&g_gpsMngCtx.gpsLock);

    return 0;
}

void* MNG_GPS_MsgReadThread(void* para)
{
    /* if raw date end have "\n" and "\r", GPSMNG_MsgProc will  proc twice: reset variable prevent thise case,just proc once */
    bool reset = false;
    int32_t ret = 0;
    unsigned char recvNum = 0;
    unsigned char recv = 0;
    char buffer[GPSMNG_MESSAGE_MAX_LEN];
    GPSDATA_S gpsData = {0};
    gpsData.wantReadLen= 1;

    prctl(PR_SET_NAME, (unsigned long)"GPS_MsgReadThread", 0, 0, 0);

    while (g_gpsMngCtx.gpsReadFlg) {

        ret = HAL_GPS_GetRawData(&gpsData, GPSMNG_GET_DATA_TIMEOUT_MS);
        if(ret == -11) {
            continue;

        }else if (ret == -110) {
            if ((g_gpsRawdataBuff.msgCount & GPSMNG_RAW_DATA_MASK) == GPSMNG_RAW_DATA_MASK) {
                GPS_DataProc();
            }
        } else if (ret == -1) {
            CVI_LOGE("error\n");
            usleep(1000 * 1000);
        } else {
            if (gpsData.actualReadLen == 0) {
                continue;
            }

            recv = gpsData.rawData[0];

            if (reset && (recv != '\n' && recv != '\r')) {

                if (recvNum >= (GPSMNG_MESSAGE_MAX_LEN - 1)) {
                    CVI_LOGE("should expand buff size,discard data[%c]\n", recv);
                    buffer[GPSMNG_MESSAGE_MAX_LEN - 1] = '\n';
                    continue;
                }

                buffer[recvNum] = recv;
                recvNum++;
            }

            if (recv == '$') {
                reset = true;
                recvNum = 0;
                memset(buffer, 0 , GPSMNG_MESSAGE_MAX_LEN);
                /* not to cut '$' symbol*/
                buffer[recvNum] = recv;
                recvNum++;
            }

            if ((recv == '\n' || recv == '\r') && reset) {
                reset = false;
                GPSMNG_MsgProc(buffer);
            }

        }

    }

    return NULL;
}

int32_t GPSMNG_Init(void)
{
    int32_t ret = 0;

    if (g_gpsMngCtx.gpsMngState != GPSMNG_STATE_DEINITED)
    {
        CVI_LOGE("gps mng has already init or start!\n");
        return GPSMNG_ENOTINIT;
    }

    pthread_mutex_init(&g_gpsMngCtx.gpsLock, NULL);

    ret = HAL_GPS_Init();

    if (ret != 0)
    {
        CVI_LOGE("hal gps init failed!\n");
        return GPSMNG_EINTER;
    }

    g_gpsMngCtx.gpsMngState = GPSMNG_STATE_INITED;

    return ret;
}

int32_t GPSMNG_Start(void)
{
    int32_t ret = 0;

    if (g_gpsMngCtx.gpsMngState == GPSMNG_STATE_DEINITED)
    {
        CVI_LOGE("gps mng has not init!\n");
        return GPSMNG_ENOTINIT;
    }
    else if (g_gpsMngCtx.gpsMngState == GPSMNG_STATE_STARTED)
    {
        CVI_LOGE("gps mng has already start!\n");
        return 0;
    }

    if (0 != ret)
    {
        CVI_LOGE("gps mng has already start!\n");
        return GPSMNG_EALREADYSTART;
    }

    g_gpsMngCtx.gpsReadFlg = true;
    ret = pthread_create(&g_gpsMngCtx.gpsReadThd, 0, MNG_GPS_MsgReadThread, NULL);

    if (ret != 0)
    {
        CVI_LOGE("create MsgReadThread failed\n");
        return GPSMNG_EREGISTEREVENT;
    }

    CVI_LOGI("GPSMNG_Start END !\n");
    g_gpsMngCtx.gpsMngState = GPSMNG_STATE_STARTED;
    return ret;
}

int32_t GPSMNG_Register(GPSMNG_CALLBACK* pfnGpsCB)
{
    int32_t recordIdx = -1;
    int32_t i;

    g_gpsMngCtx.gpsRefreshData = true;

    for (i = 0; i < GPSMNG_CALLBACK_MAX_NUM; i++)
    {
        if (NULL == g_gpsMngCtx.fnGpsCB[i].fnGpsDataCB)
        {
            recordIdx = i;
        }

        if (pfnGpsCB->fnGpsDataCB == g_gpsMngCtx.fnGpsCB[i].fnGpsDataCB \
            && pfnGpsCB->privateData == g_gpsMngCtx.fnGpsCB[i].privateData)
        {
            return 0;
        }
    }

    if (-1 != recordIdx)
    {
        pthread_mutex_lock(&g_gpsMngCtx.gpsLock);
        g_gpsMngCtx.fnGpsCB[recordIdx].fnGpsDataCB = pfnGpsCB->fnGpsDataCB;
        g_gpsMngCtx.fnGpsCB[recordIdx].privateData = pfnGpsCB->privateData;
        pthread_mutex_unlock(&g_gpsMngCtx.gpsLock);
        return 0;
    }

    return GPSMNG_EREGISTER;
}

int32_t GPSMNG_UnRegister(GPSMNG_CALLBACK* pfnGpsCB)
{
    int32_t i;

    g_gpsMngCtx.gpsRefreshData = false;

    for (i = 0; i < GPSMNG_CALLBACK_MAX_NUM; i++)
    {
        if ((pfnGpsCB->privateData == g_gpsMngCtx.fnGpsCB[i].privateData) && \
            (pfnGpsCB->fnGpsDataCB == g_gpsMngCtx.fnGpsCB[i].fnGpsDataCB))
        {
            pthread_mutex_lock(&g_gpsMngCtx.gpsLock);
            g_gpsMngCtx.fnGpsCB[i].fnGpsDataCB = NULL;
            g_gpsMngCtx.fnGpsCB[i].privateData = NULL;
            pthread_mutex_unlock(&g_gpsMngCtx.gpsLock);
            return 0;
        }
    }

    return GPSMNG_EUREGISTER;
}

int32_t GPSMNG_Stop(void)
{
    int32_t ret = 0;

    if (g_gpsMngCtx.gpsMngState == GPSMNG_STATE_DEINITED)
    {
        CVI_LOGE("gps mng has not init!\n");
        return GPSMNG_ENOTINIT;
    }

    if (GPSMNG_STATE_STOP == g_gpsMngCtx.gpsMngState)
    {
        CVI_LOGE("gps mng has already stop!\n");
        return 0;
    }

    g_gpsMngCtx.gpsReadFlg = false;
    ret = pthread_join(g_gpsMngCtx.gpsReadThd, NULL);

    if (ret != 0)
    {
        CVI_LOGE("join MsgReadThread failed\n");
        return GPSMNG_EREGISTEREVENT;
    }

    g_gpsMngCtx.gpsMngState = GPSMNG_STATE_STOP;
    return ret;
}

int32_t GPSMNG_DeInit(void)
{
    int32_t ret = 0;
    if (GPSMNG_STATE_STARTED == g_gpsMngCtx.gpsMngState)
    {
        CVI_LOGE("gps must stoped!\n");
        return -1;
    }

    if (GPSMNG_STATE_DEINITED == g_gpsMngCtx.gpsMngState)
    {
        CVI_LOGE("gps mng has already deinit!\n");
        return 0;
    }

    ret = HAL_GPS_Deinit();

    if (ret != 0)
    {
        CVI_LOGE("hal gps init failed!\n");
        return GPSMNG_EDEINIT;
    }

    g_gpsMngCtx.gpsMngState = GPSMNG_STATE_DEINITED;

    pthread_mutex_destroy(&g_gpsMngCtx.gpsLock);
    return ret;
}

int32_t GPSMNG_GetData(GPSMNG_MSG_PACKET* msgPacket)
{
    int32_t ret = 0;

    if (g_gpsMngCtx.gpsMngState != GPSMNG_STATE_STARTED)
    {
        CVI_LOGE("gps mng has not start!\n");
        return GPSMNG_EGETDATA;
    }
    memcpy(msgPacket, &g_gpsMngPacket, sizeof(GPSMNG_MSG_PACKET));

    return ret;
}

/* vim: set ts=4 sw=4 et: */
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

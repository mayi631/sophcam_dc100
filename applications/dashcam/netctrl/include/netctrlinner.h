#ifndef __NETCTRLINNER_H__
#define __NETCTRLINNER_H__
#include <stdbool.h>
#include "thttpd.h"
#include "libhttpd.h"
#include "mode.h"
#include "sysutils_eventhub.h"
#include "cvi_ae.h"
#include <sys/socket.h>
#include "net.h"
#include "netctrl.h"
#include "netctrlinner.h"
#include "sysutils_md5.h"
#include "storagemng.h"
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
#include "thumbnail_extractor.h"
#endif
#ifdef COMPONENTS_PLAYER_ON
#include "player.h"
#endif
#include "dtcf.h"
#if CONFIG_PWM_ON
#include "hal_pwm.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "player_service.h"
#endif
#include "timedtask.h"
#include "hal_wifi.h"
#include "media_init.h"
#include "netctrlinner.h"
#include "system.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifndef PTHREAD_MUTEX_INITIALIZER
# define PTHREAD_MUTEX_INITIALIZER \
  { { 0, 0, 0, 0, 0, { __PTHREAD_SPINS } } }
#endif

#define WIFIAPP_OK 0
#define WIFIAPP_FALSE -1
#define VERSION_NUM "CVITEK20210901"

typedef enum {
	WIFI_SD_FORMAT_FAIL = -1,
	WIFI_SD_FORMAT_SUCCESS = 0,
    WIFI_SD_FORMAT_INIT = 1
} WIFI_SD_STATUS_E;

#define LBUFFSIZE (128)
#define BUFFSIZE (256)
#define XBUFFSIZE (512)
#define OBUFFSIZE (1024)
#define TBUFFSIZE (4096)
#define NET_WIFI_SSID_LEN (32)
#define NET_WIFI_PASS_LEN (26)
#define TIMECOUNTMAX (10)

typedef struct tagPDT_NETCTRL_MESSAGE_CONTEXT
{
    MESSAGE_S stMsg;     /**< the message that has been sent*/
    bool bMsgProcessed;  /**< the flag of message has been processed or not*/
    pthread_mutex_t MsgMutex;  /**< lock*/
} PDT_NETCTRL_MESSAGE_CONTEXT;

int32_t NETCTRLINNER_SendSyncMsg(MESSAGE_S* pstMsg, int* ps32Result);
int32_t NETCTRLINNER_InitCMDSocket(void);
int32_t NETCTRLINNER_DeInitCMDSocket(void);
int32_t NETCTRLINNER_TimeoutRest(void);
int32_t NETCTRLINNER_APPConnetState(void);
int32_t NETCTRLINNER_GetSdState(void);
void NETCTRLINNER_SetSdState(int32_t value);
int32_t NETCTRLINNER_SubscribeEvents(void);
void NETCTRLINNER_ScanFile(void);
int32_t NETCTRLINNER_InitTimer(void);
int32_t NETCTRLINNER_DeInitTimer(void);
int32_t NETCTRLINNER_StartTimer(void);
int32_t NETCTRLINNER_StopTimer(void);
void NETCTRLINNER_UiUpdate(void);
void NETCTRLINNER_Enablecheckconnect(void *privData, struct timespec *now);
void NETCTRLINNER_CMDRegister(void);
void* NETCTRLINNER_SocketXml(void *arg);
void* NETCTRLINNER_SocketCgi(void *arg);
void* NETCTRLINNER_SocketcJson(void *arg);
int32_t NETCTRLINNER_GetFlagFile(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
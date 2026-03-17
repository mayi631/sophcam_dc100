#include <stdbool.h>

#ifndef __NETCTRL_H__
#define __NETCTRL_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum _NET_EVENT_E
{
    EVENT_NETCTRL_CONNECT = APPCOMM_EVENT_ID(APP_MOD_NETCTRL, 0),
    EVENT_NETCTRL_UIUPDATE,
    EVENT_NETCTRL_APPCONNECT_SUCCESS,
    EVENT_NETCTRL_APPDISCONNECT,
    EVENT_NETCTRL_APPCONNECT_SETTING,
    EVENT_NETCTRL_BUIT
} NET_EVENT_E;

typedef enum {
	WIFI_APP_DISCONNECT = 0,
	WIFI_APP_CONNECTTED,
} WIFI_APP_CONNECT_E;

typedef enum {
	WIFI_APP_SETTING_OUT = 0,
	WIFI_APP_SETTING_IN,
} WIFI_APP_SETTING_E;

int32_t NETCTRL_Init(void);
int32_t NETCTRL_DeInit(void);
int32_t NETCTRL_NetToUiConnectState(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif
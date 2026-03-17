#ifndef __HAL_WIFI_H__
#define __HAL_WIFI_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define  HAL_WIFI_SSID_LEN (32)
#define  HAL_WIFI_PASSWORD_LEN (64)
#define  HAL_WIFI_STR_LEN (64)
#define  HAL_WIFI_RESULT_SIZE (64)

#if defined(CONFIG_WIFI_HI3881)
#define HAL_WIFI_INTERFACE_NAME "ap0"
#else
#define HAL_WIFI_INTERFACE_NAME "wlan0"
#endif

#define HAL_WIFI_IP "192.168.169.1"

/* WiFi mode enum*/
typedef enum HAL_WIFI_MODE_E {
	HAL_WIFI_MODE_STA = 0,/**<STA mode*/
	HAL_WIFI_MODE_AP,/**<AP mode*/
	HAL_WIFI_MODE_BUIT
} HAL_WIFI_MODE_E;

/* WiFi STA workmode*/
typedef enum HAL_STA_WIFI_MODE_E {
	HAL_WIFI_STA_MODE_COMMON = 0,/**<ap broadcast ssid*/
	HAL_WIFI_STA_MODE_SENIOR,/**<ap hide ssid*/
	HAL_WIFI_STA_MODE_BUIT
} HAL_WIFI_STA_MODE_E;



/** wifi ap && sta common configure */
typedef struct HAL_WIFI_COMMON_CFG_S {
	char szWiFiSSID[HAL_WIFI_SSID_LEN]; /**<wifi ssid,aszWiFiSSID len decided by strlen(aszWiFiSSID)*/
	char szWiFiPassWord[HAL_WIFI_PASSWORD_LEN];/**<wifi password,szWiFiPassWord len decided by strlen(szWiFiPassWord)*/
} HAL_WIFI_COMMON_CFG_S;


/** wifi ap configure */
typedef struct HAL_WIFI_APMODE_CFG_S {
	bool bHideSSID;/**<true:ssid broadcast,false:hide*/
	int32_t s32Channel; /**<wifi channel */
	HAL_WIFI_COMMON_CFG_S stCfg;/**<wifi common cfg*/
} HAL_WIFI_APMODE_CFG_S;


/** wifi sta common configure */
typedef struct HAL_WIFI_STAMODE_COMMON_CFG_S {
	HAL_WIFI_COMMON_CFG_S stCfg;
} HAL_WIFI_STAMODE_COMMON_CFG_S;


/** wifi sta common configure */
typedef struct HAL_WIFI_STAMODE_SENIOR_CFG_S {
	HAL_WIFI_COMMON_CFG_S stCfg;
} HAL_WIFI_STAMODE_SENIOR_CFG_S;

/** wifi sta configure */
typedef struct HAL_WIFI_STAMODE_CFG_S {
	HAL_WIFI_STA_MODE_E enStaMode;/**<STA mode*/
	union tagHAL_WIFI_STAMODE_CFG_U {
		HAL_WIFI_STAMODE_COMMON_CFG_S stCommonCfg;/**<cfg for common connect ap mode*/
		HAL_WIFI_STAMODE_SENIOR_CFG_S stSeniorCfg;/**<cfg for senior connect ap mode*/
	} unCfg;
} HAL_WIFI_STAMODE_CFG_S;


/** wifi configure */
typedef struct HAL_WIFI_CFG_S {
	HAL_WIFI_MODE_E enMode;/**<wifi mode*/
	union tagHAL_WIFI_CFG_U {
		HAL_WIFI_APMODE_CFG_S stApCfg;/**<wifi AP cfg*/
		HAL_WIFI_STAMODE_CFG_S stStaCfg;/**<wifi STA cfg*/
	} unCfg;
} HAL_WIFI_CFG_S;


/* WiFi scan result*/
typedef struct HAL_WIFI_SCAN_RESULT_S {
	char szSSID[HAL_WIFI_SSID_LEN];/**<Hotspot SSID */
	char szBssid[HAL_WIFI_STR_LEN];/**<Hotspot BSSID */
	uint32_t u32Channel;/**<Hotspot channel*/
	uint32_t u32Signal;/**<Hotspot signal strength,value[0,100]*/
} HAL_WIFI_SCAN_RESULT_S;

/* WiFi scan result*/
typedef struct HAL_WIFI_SCAN_RESULTSET_S {
	uint32_t u32Num;/**<Hotspot num*/
	HAL_WIFI_SCAN_RESULT_S astResult[HAL_WIFI_RESULT_SIZE];
} HAL_WIFI_SCAN_RESULTSET_S;

int32_t HAL_WIFI_Init(HAL_WIFI_MODE_E enMode);
int32_t HAL_WIFI_CheckeCfgValid(const HAL_WIFI_CFG_S *pstCfg, bool *pCfgValid);
int32_t HAL_WIFI_Start(const HAL_WIFI_CFG_S *pstCfg);
int32_t HAL_WIFI_GetStartedStatus(bool *pbEnable);
int32_t HAL_WIFI_Stop(void);
int32_t HAL_WIFI_Deinit(void);

/** @}*/  /** <!-- ==== HAL_WIFI End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __CVI_HAL_WIFI_H__*/

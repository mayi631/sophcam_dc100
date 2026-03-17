#ifndef __HAL_WIFI_COMMON_H__
#define __HAL_WIFI_COMMON_H__

#include "hal_wifi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_WIFI_COMMON */
/** @{ */  /** <!-- [HAL_WIFI_COMMON] */

/** macro define */
#define HAL_WIFI_IFNAMSIZ (16)
#define HAL_WIFI_DEV_DIR  "/dev/wifi"
#define HAL_WIFI_KILL_EXECUTE_FILE  "/usr/bin/killall"

/** @}*/  /** <!-- ==== HAL_WIFI_COMMON End ====*/

int32_t WIFI_HAL_GetInterface(const char* pszIfname);
int32_t WIFI_UTILS_Ifconfig (const char* pszIfname, int32_t s32Up);
int32_t WIFI_UTILS_SetIp(const char* pszIfname, const char* pszIp);
int32_t WIFI_UTILS_AllFDClosexec(void);
void WIFI_HAL_PowerOnReset(void);
void WIFI_HAL_SysPreInit(void);
int32_t WIFI_HAL_LoadAPDriver(void);
int32_t WIFI_HAL_LoadSTADriver(void);
int32_t WIFI_HAL_RemoveDriver(void);
int32_t WIFI_HAL_UpdateApConfig(char *ifname,  const HAL_WIFI_APMODE_CFG_S *pstApCfg, const char *pszConfigFile);
int32_t WIFI_AP_Start(char *ifname,const HAL_WIFI_APMODE_CFG_S* pstApCfg);
int32_t WIFI_AP_Stop(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HAL_WIFI_COMMON_H__*/

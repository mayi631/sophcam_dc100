#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "hal_wifi.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

static int32_t WIFIMNG_UpdateWifiApCfg(HAL_WIFI_APMODE_CFG_S* pstApCfg, char *pstDefaultssid)
{
#define PDT_LAST_MAC_ADDR_STR_LEN (4)
    if (0 == strcmp(pstApCfg->stCfg.szWiFiSSID, pstDefaultssid)) {
        char szMacAddrResult[PDT_LAST_MAC_ADDR_STR_LEN + 1] = {0};

        char szCmd[192] = {'\0'};
        snprintf(szCmd, sizeof(szCmd), "%s", "mac=`ifconfig -a "HAL_WIFI_INTERFACE_NAME" | head -1 | awk '{ print $5 }' | tr -d ':' `;  echo ${mac:8:4}\n");
        FILE* fp = popen(szCmd, "r");
        if (NULL == fp) {
            perror("popen failed\n");
            return 0;
        } else {
            fgets(szMacAddrResult, sizeof(szMacAddrResult), fp);
            pclose(fp);
        }

        if(((strlen(pstApCfg->stCfg.szWiFiSSID) - 1) + PDT_LAST_MAC_ADDR_STR_LEN) >= HAL_WIFI_SSID_LEN) {
            CVI_LOGE("WiFiSSID to long, Not added mac addr\n");
        } else {
            strncat(pstApCfg->stCfg.szWiFiSSID, szMacAddrResult, PDT_LAST_MAC_ADDR_STR_LEN);
        }
    }

    CVI_LOGI("szWiFiSSID: %s  szWiFiPassWord: %s\n", pstApCfg->stCfg.szWiFiSSID, pstApCfg->stCfg.szWiFiPassWord);

    return 0;
}

int32_t WIFIMNG_Start(HAL_WIFI_CFG_S WifiCfg, char *pstDefaultssid)
{
	int32_t s32Ret = 0;

    s32Ret = HAL_WIFI_Init(WifiCfg.enMode);
    if (s32Ret < 0) {
        CVI_LOGE("HAL_WIFI_Init faile! \n");
        return 1;
    }
    WIFIMNG_UpdateWifiApCfg(&WifiCfg.unCfg.stApCfg, pstDefaultssid);

    s32Ret = HAL_WIFI_Start(&WifiCfg);
    if(s32Ret < 0) {
        CVI_LOGE("HAL_WIFI_Start faile! \n");
        return 1;
    }

    CVI_LOGI("HAL_WIFI_Start\n");

	return 0;
}

int32_t WIFIMNG_Stop(void)
{
	int32_t s32Ret = 0;

	// stop
	s32Ret = HAL_WIFI_Stop();
	if (s32Ret < 0) {
		CVI_LOGE("HAL_WIFI_Stop faile! \n");
		return -1;
	}

    s32Ret = HAL_WIFI_Deinit();
    if (s32Ret < 0) {
        CVI_LOGE("HAL_WIFI_Deinit faile! \n");
		return -1;
    }

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "hal_wifi_common.h"
#include "hal_wifi.h"
#include "hal_gpio.h"
#include "osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

void WIFI_HAL_PowerOnReset(void)
{
	return; /** 8189 need not power on reset*/
}

void WIFI_HAL_SysPreInit(void)
{
	static bool bPinOutInit = false;
	if (true == bPinOutInit) {
		printf("hal_system_preinit already init\n");
		return;
	}
	bPinOutInit = true;
	/** SDIO1-MUX*/
}

int32_t WIFI_HAL_LoadAPDriver(void)
{
	int32_t s32Ret = 0;

	s32Ret = OSAL_FS_Insmod(KOMOD_PATH "/ko/3rd/bcmdhd.ko", "firmware_path="KOMOD_PATH"/3rd/fw_bcm43013c1_ag_apsta.bin");
	if (0 != s32Ret) {
		printf("insmod ap: failed, errno(%d)\n", errno);
		return -1;
	}
	usleep(1000 * 1000);

	return 0;
}

int32_t WIFI_HAL_LoadSTADriver(void)
{
	printf("insmod sta mode driver not support,failed\n");
	return -1;
}

int32_t WIFI_HAL_RemoveDriver(void)
{
	int32_t s32Ret = 0;

	s32Ret = OSAL_FS_Rmmod(KOMOD_PATH "/3rd/bcmdhd.ko");
	if (0 != s32Ret) {
		printf("insmod ap: failed, errno(%d)\n", errno);
		return -1;
	}
	return 0;
}
int32_t WIFI_HAL_UpdateApConfig(char *ifname, const HAL_WIFI_APMODE_CFG_S *pstApCfg, const char *pszConfigFile)
{
	int32_t s32Ret = 0;
	int32_t s32fd;
	char *szWbuf = NULL;
	/** open configure file, if not exist, create it */
	s32fd = open(pszConfigFile, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (s32fd < 0) {
		printf("WiFi: Cann't open configure file '%s',errno(%d)", pszConfigFile, errno);
		return -1;
	}

	asprintf(&szWbuf, "interface=%s\n"
					  "driver=%s\n"
					  "wpa=2\n"
					  "ctrl_interface=/dev/wifi/hostapd\n"
					  "ssid=%s\n"
					  "wpa_passphrase=%s\n"
					  "channel=%d\n"
					  "ignore_broadcast_ssid=%d\n"
					  "hw_mode=g\n"
					  "ieee80211n=1\n"
					  "wpa_key_mgmt=WPA-PSK\n"
					  "wpa_pairwise=CCMP\n"
					  "max_num_sta=1\n",
			 ifname, "nl80211", pstApCfg->stCfg.szWiFiSSID, pstApCfg->stCfg.szWiFiPassWord, pstApCfg->s32Channel,
			 pstApCfg->bHideSSID ? 1 : 0);

	if (write(s32fd, szWbuf, strlen(szWbuf)) < 0) {
		printf("WiFi: Cann't write configuration to '%s',errno(%d)\n", pszConfigFile, errno);
		s32Ret = -1;
	}
	close(s32fd);
	free(szWbuf);
	return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <linux/watchdog.h>

#include "hal_watchdog.h"
#include "osal.h"
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define HAL_FD_INITIALIZATION_VAL (-1)
#define HAL_WATCHDOG_DEV "/dev/watchdog"

static int32_t s_s32HALWATCHDOGfd = HAL_FD_INITIALIZATION_VAL;

int32_t HAL_WATCHDOG_Init(int32_t s32Time_s)
{
	int32_t s32Ret = 0;
	if (s_s32HALWATCHDOGfd != HAL_FD_INITIALIZATION_VAL) {
		CVI_LOGE("already init");
		return -1;
	}

	if (s32Time_s < 2 || s32Time_s > 1000) {
		CVI_LOGE("Interval time should not be less then two and bigger then 100. %d\n", s32Time_s);
		return -1;
	}

	char szWdtString[128] = {0};
	snprintf(szWdtString, sizeof(szWdtString), " default_margin=%d nodeamon=1", s32Time_s);

	s32Ret = OSAL_FS_Insmod(KOMOD_PATH "/cv181x_wdt.ko", szWdtString);
	if (0 != s32Ret) {
		CVI_LOGE("insmod wdt.ko: failed, errno(%d)\n", errno);
		return -1;
	}

	s_s32HALWATCHDOGfd = open(HAL_WATCHDOG_DEV, O_RDWR);

	if (s_s32HALWATCHDOGfd < 0) {
		CVI_LOGE("open [%s] failed\n", HAL_WATCHDOG_DEV);
		return -1;
	}

	s32Ret = ioctl(s_s32HALWATCHDOGfd, WDIOC_KEEPALIVE); /**feed dog */
	if (-1 == s32Ret) {
		CVI_LOGE("WDIOC_KEEPALIVE: failed, errno(%d)\n", errno);
		return -1;
	}

	return 0;
}

int32_t HAL_WATCHDOG_Feed(void)
{
	int32_t s32Ret = 0;
	s32Ret = ioctl(s_s32HALWATCHDOGfd, WDIOC_KEEPALIVE); /**feed dog */
	if (-1 == s32Ret) {
		CVI_LOGE("WDIOC_KEEPALIVE: failed, errno(%d)\n", errno);
		return -1;
	}
	return 0;
}

int32_t HAL_WATCHDOG_Deinit(void)
{
	int32_t s32Ret;

	if (s_s32HALWATCHDOGfd == HAL_FD_INITIALIZATION_VAL) {
		CVI_LOGE("watchdog not initialized,no need to close\n");
		return -1;
	}
	s32Ret = close(s_s32HALWATCHDOGfd);
	if (0 > s32Ret) {
		CVI_LOGE("wdrfd[%d] close,fail,errno(%d)\n", s_s32HALWATCHDOGfd, errno);
		return -1;
	}
	s_s32HALWATCHDOGfd = HAL_FD_INITIALIZATION_VAL;
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

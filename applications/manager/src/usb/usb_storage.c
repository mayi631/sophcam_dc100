/**
 * @file      usb_storage.c
 * @brief     usb storage interface implementation
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "appcomm.h"
#include "cvi_log.h"
#include "osal.h"
#include "usb_storage.h"
#define USB_DEV_SD "/dev/mmcblk0p1"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static bool g_bInited = false;
static pthread_mutex_t g_Mutex = PTHREAD_MUTEX_INITIALIZER;

static int32_t USB_STORAGE_LoadMod(const char *pszDevPath)
{
	char cmds[50] = {0};
	OSAL_FS_System(KOMOD_PATH "/loadusbstorageko.sh");
	sprintf(cmds, "/etc/run_usb.sh probe msc %s", pszDevPath);
	OSAL_FS_System(cmds);
	OSAL_FS_System("/etc/run_usb.sh start");

	return 0;
}

static void USB_STORAGE_UnloadMod(void)
{
	OSAL_FS_System("/etc/run_usb.sh stop");
}

int32_t USB_STORAGE_Init(const char *pszDevPath)
{
	pthread_mutex_lock(&g_Mutex);
	if (true == g_bInited) {
		CVI_LOGD("already inited\n");
		pthread_mutex_unlock(&g_Mutex);
		return 0;
	}

	int32_t s32Ret = USB_STORAGE_LoadMod(pszDevPath);
	APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, -1, "LoadKo");

	g_bInited = true;
	pthread_mutex_unlock(&g_Mutex);
	return 0;
}

int32_t USB_STORAGE_Deinit(void)
{
	pthread_mutex_lock(&g_Mutex);
	if (false == g_bInited) {
		CVI_LOGD("already deinited\n");
		pthread_mutex_unlock(&g_Mutex);
		return 0;
	}

	USB_STORAGE_UnloadMod();
	g_bInited = false;
	pthread_mutex_unlock(&g_Mutex);
	return 0;
}

int32_t USB_STORAGE_PrepareDev(const USB_STORAGE_CFG_S *pstCfg)
{
	APPCOMM_CHECK_POINTER(pstCfg, -1);

	/* Read file content */
	int32_t s32Ret = 0;
	char szBuf[APPCOMM_MAX_PATH_LEN] = {0};
	int32_t s32Fd = open(pstCfg->szSysFile, O_CREAT | O_RDWR, 0644);
	if (s32Fd < 0) {
		CVI_LOGE("open %s failed\n", pstCfg->szSysFile);
		return -1;
	}
	s32Ret = read(s32Fd, szBuf, APPCOMM_MAX_PATH_LEN);
	if (s32Ret < 0) {
		CVI_LOGE("read %s failed\n", pstCfg->szSysFile);
		close(s32Fd);
		return -1;
	}

	ftruncate(s32Fd, 0);
	write(s32Fd, pstCfg->szDevPath, strnlen(pstCfg->szDevPath, APPCOMM_MAX_PATH_LEN));
	close(s32Fd);

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

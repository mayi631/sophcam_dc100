/**
 * @file      usb.c
 * @brief     usb interface implementation
 * @version   1.0
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "cvi_log.h"
#include "osal.h"
#include "usb.h"
#include "uvc_gadget.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** usb connect state */
typedef enum tagUSB_CONNECT_STATE_E {
	USB_CONNECT_STATE_DISCONNECT = 0,
	USB_CONNECT_STATE_CHARGE,
	USB_CONNECT_STATE_PC_CONNECTED,
	USB_CONNECT_STATE_BUTT
} USB_CONNECT_STATE_E;

/** USB Context */
typedef struct tagUSB_CONTEXT_S {
	bool bInit;
	bool bCheckTskRun;	  /**<check task run flag */
	pthread_t CheckTskId; /**<check task thread id */
	USB_MODE_E enMode;
	USB_STATE_E enState;
	USB_CFG_S stUsbCfg;
	pthread_mutex_t stateMutex;
} USB_CONTEXT_S;
static USB_CONTEXT_S s_stUSBCtx =
	{
		.bInit = false,
};

void USB_CheckPower_Soure(USB_POWER_SOURCE_E *PowerSoureState)
{
	char *Powerprocfile = "/proc/cviusb/chg_det_fast";
#define MAX_SCRLIEN 128
	FILE *fp = NULL;
	char temp[MAX_SCRLIEN];
	char szStateString[MAX_SCRLIEN] = {
		0,
	};
	memset(temp, 0x0, sizeof(char) * MAX_SCRLIEN);
	fp = fopen(Powerprocfile, "r");
	if (fp != NULL) {
		if (!feof(fp)) {
			while (fgets(temp, (MAX_SCRLIEN - 1), fp) != NULL) {
				snprintf(szStateString, MAX_SCRLIEN, "%s", temp);
				break;
			}
		}
		fclose(fp);

		if (strstr(szStateString, "hub")) {
			*PowerSoureState = USB_POWER_SOURCE_PC;
		} else if (strstr(szStateString, "adapter")) {
			*PowerSoureState = USB_POWER_SOURCE_POWER;
		} else if (strstr(szStateString, "none")) {
			*PowerSoureState = USB_POWER_SOURCE_NONE;
		}
	}
}

static inline const char *USB_GetModeStr(USB_MODE_E enMode)
{
	switch (enMode) {
	case USB_MODE_CHARGE:
		return "Charge";
	case USB_MODE_UVC:
		return "UVC";
	case USB_MODE_STORAGE:
		return "USBStorage";
	case USB_MODE_HOSTUVC:
		return "Host UVC";
	default:
		return "Unknown";
	}
}

static inline const char *USB_GetStateStr(USB_STATE_E enState)
{
	switch (enState) {
	case USB_STATE_OUT:
		return "Out";
	case USB_STATE_INSERT:
		return "Insert";
	case USB_STATE_UVC_READY:
		return "UVC Ready";
	case USB_STATE_UVC_PC_READY:
		return "UVC PC Ready";
	case USB_STATE_UVC_MEDIA_READY:
		return "UVC Media Ready";
	case USB_STATE_STORAGE_READY:
		return "Storage Ready";
	case USB_STATE_STORAGE_PC_READY:
		return "Storage PC Ready";
	case USB_STATE_STORAGE_SD_READY:
		return "Storage SD Ready";
	case USB_STATE_HOSTUVC_READY:
		return "Host UVC Ready";
	case USB_STATE_HOSTUVC_CAMERA_READY:
		return "Host UVC Camera Ready";
	case USB_STATE_HOSTUVC_MEDIA_READY:
		return "Host UVC Media Ready";
	default:
		return "Unknown";
	}
}

static inline void USB_SetState(USB_STATE_E enState)
{
	if (enState != s_stUSBCtx.enState) {
		CVI_LOGD("%s -> %s\n", USB_GetStateStr(s_stUSBCtx.enState), USB_GetStateStr(enState));
		s_stUSBCtx.enState = enState;
	}
}

static inline int32_t USB_EventProc(const USB_EVENT_INFO_S *pstEvent)
{
	APPCOMM_CHECK_POINTER(s_stUSBCtx.stUsbCfg.pfnEventProc, -1);
	return s_stUSBCtx.stUsbCfg.pfnEventProc(pstEvent);
}

static void *USB_StateCheckProc(void *arg)
{
	prctl(PR_SET_NAME, "USB_StateCheckProc", 0, 0, 0);
	USB_POWER_SOURCE_E enPowerState = USB_POWER_SOURCE_NONE;
	USB_POWER_SOURCE_E enPowerLastState = USB_POWER_SOURCE_NONE;
	while (s_stUSBCtx.bCheckTskRun) {
		USB_CheckPower_Soure(&enPowerState);
		if (enPowerState != enPowerLastState) {
			if (USB_POWER_SOURCE_PC == enPowerState) {
				USB_SetState(USB_STATE_PC_INSERT);
				USB_EVENT_INFO_S stEventInfo;
				stEventInfo.s32EventId = EVENT_USB_PC_INSERT;
				USB_EventProc(&stEventInfo);
			} else if (USB_POWER_SOURCE_POWER == enPowerState) {
				USB_SetState(USB_STATE_INSERT);
				USB_EVENT_INFO_S stEventInfo;
				stEventInfo.s32EventId = EVENT_USB_INSERT;
				USB_EventProc(&stEventInfo);
			} else if (enPowerLastState != USB_POWER_SOURCE_NONE && USB_POWER_SOURCE_NONE == enPowerState) {
				USB_SetState(USB_STATE_OUT);
				USB_EVENT_INFO_S stEventInfo;
				stEventInfo.s32EventId = EVENT_USB_OUT;
				USB_EventProc(&stEventInfo);
				CVI_LOGD("EVENT_USB_OUT ...................................");
			}
			enPowerLastState = enPowerState;
		}
		OSAL_TASK_Sleep(100 * 1000);
	}

	return NULL;
}

int32_t USB_Init(const USB_CFG_S *pstCfg)
{
	APPCOMM_CHECK_POINTER(pstCfg, USB_EINVAL);
	APPCOMM_CHECK_POINTER(pstCfg->pfnEventProc, USB_EINVAL);
	if (s_stUSBCtx.bInit) {
		CVI_LOGE("has already inited!\n");
		return 0;
	}
	pthread_mutex_init(&s_stUSBCtx.stateMutex, NULL);

	/* record usb configure */
	memcpy(&s_stUSBCtx.stUsbCfg, pstCfg, sizeof(USB_CFG_S));

	/* Create usb check task thread */
	s_stUSBCtx.bCheckTskRun = true;
	s_stUSBCtx.enMode = USB_MODE_CHARGE;
	s_stUSBCtx.enState = USB_STATE_BUTT;
	s_stUSBCtx.bInit = true;

	// OSAL_FS_Insmod(KOMOD_PATH"/usb-common.ko", NULL);
	// OSAL_FS_Insmod(KOMOD_PATH"/udc-core.ko", NULL);
	// OSAL_FS_Insmod(KOMOD_PATH"/usbcore.ko", NULL);
	// OSAL_FS_Insmod(KOMOD_PATH"/roles.ko", NULL);
	// OSAL_FS_Insmod(KOMOD_PATH"/dwc2.ko", NULL);
	OSAL_FS_System("devmem 0x05027018 32 0x40");
	OSAL_FS_System("devmem 0x03001820 32 0x40");
	OSAL_FS_Insmod(KOMOD_PATH "/libcomposite.ko", NULL);
	OSAL_FS_Insmod(KOMOD_PATH "/videobuf2-common.ko", NULL);
	OSAL_FS_Insmod(KOMOD_PATH "/videobuf2-memops.ko", NULL);
	OSAL_FS_Insmod(KOMOD_PATH "/videobuf2-v4l2.ko", NULL);
	OSAL_FS_Insmod(KOMOD_PATH "/videobuf2-vmalloc.ko", NULL);
	OSAL_FS_Insmod(KOMOD_PATH "/usb_f_uvc.ko", NULL);
	OSAL_FS_System("echo device > /proc/cviusb/otg_role");

	USB_POWER_SOURCE_E enPowerState = USB_POWER_SOURCE_BUTT;
	USB_CheckPower_Soure(&enPowerState);

	if (pthread_create(&s_stUSBCtx.CheckTskId, NULL, USB_StateCheckProc, NULL)) {
		CVI_LOGE("USB_CheckTask create failed\n");
		s_stUSBCtx.bCheckTskRun = false;
		return -1;
	}

	return 0;
}

int32_t USB_SetMode(USB_MODE_E enMode)
{
	APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT);
	APPCOMM_CHECK_EXPR((enMode < USB_MODE_BUTT) && (enMode >= USB_MODE_CHARGE), USB_EINVAL);
	CVI_LOGD("usb mode change[%s->%s]\n", USB_GetModeStr(s_stUSBCtx.enMode), USB_GetModeStr(enMode));
	pthread_mutex_lock(&s_stUSBCtx.stateMutex);
	if (s_stUSBCtx.enMode == enMode) {
		pthread_mutex_unlock(&s_stUSBCtx.stateMutex);
		return 0;
	}

	if (USB_MODE_UVC == s_stUSBCtx.enMode) {
		UVC_Stop();
		UVC_Deinit();
	} else if (USB_MODE_STORAGE == s_stUSBCtx.enMode) {
		USB_STORAGE_Deinit();
	} else if (s_stUSBCtx.enMode == USB_MODE_HOSTUVC) {
		// USB_HostUvcDeinit();
		CVI_LOGE("host uvc not support now\n");
	}

	if (USB_MODE_UVC == enMode) {
		// UVC_Init(&s_stUSBCtx.stUsbCfg.stUvcCfg.stDevCap, &s_stUSBCtx.stUsbCfg.stUvcCfg.stDataSource,
		//     &s_stUSBCtx.stUsbCfg.stUvcCfg.stBufferCfg);
		// UVC_Start(s_stUSBCtx.stUsbCfg.stUvcCfg.szDevPath);
	} else if (USB_MODE_STORAGE == enMode) {
		USB_STORAGE_Init(s_stUSBCtx.stUsbCfg.stStorageCfg.szDevPath);
	} else if (enMode == USB_MODE_HOSTUVC) {
		CVI_LOGE("host uvc not support now\n");
		// USB_HostUvcInit();
	}
	s_stUSBCtx.enMode = enMode;
	pthread_mutex_unlock(&s_stUSBCtx.stateMutex);
	return 0;
}

int32_t USB_Deinit(void)
{
	// APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT);

	/* Destroy check task */
	s_stUSBCtx.bCheckTskRun = false;
	pthread_join(s_stUSBCtx.CheckTskId, NULL);

	// USB_DeinitProc(&s_stUSBCtx);
	s_stUSBCtx.bInit = false;
	pthread_mutex_destroy(&s_stUSBCtx.stateMutex);
	return 0;
}

int32_t USB_SetUvcCfg(const UVC_CFG_S *pstCfg)
{
	APPCOMM_CHECK_POINTER(pstCfg, USB_EINVAL);
	APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT);
	memcpy(&s_stUSBCtx.stUsbCfg.stUvcCfg, pstCfg, sizeof(UVC_CFG_S));
	return 0;
}

int32_t USB_GetUvcCfg(USB_CFG_S *pstCfg)
{
	/*     APPCOMM_CHECK_POINTER(pstCfg, USB_EINVAL);
		APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT); */
	memcpy(pstCfg, &s_stUSBCtx.stUsbCfg, sizeof(USB_CFG_S));
	return 0;
}

int32_t USB_SetStorageCfg(const USB_STORAGE_CFG_S *pstCfg)
{
	APPCOMM_CHECK_POINTER(pstCfg, USB_EINVAL);
	APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT);
	memcpy(&s_stUSBCtx.stUsbCfg.stStorageCfg, pstCfg, sizeof(USB_STORAGE_CFG_S));
	return 0;
}

int32_t USB_GetStorageCfg(USB_STORAGE_CFG_S *pstCfg)
{
	memcpy(pstCfg, &s_stUSBCtx.stUsbCfg.stStorageCfg, sizeof(USB_STORAGE_CFG_S));
	return 0;
}

int32_t USB_GetMode(USB_MODE_E *penMode)
{
	APPCOMM_CHECK_POINTER(penMode, USB_EINVAL);
	APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT);
	CVI_LOGD("usb mode[%s]\n", USB_GetModeStr(s_stUSBCtx.enMode));
	*penMode = s_stUSBCtx.enMode;
	return 0;
}

int32_t USB_GetState(USB_STATE_E *penState)
{
	APPCOMM_CHECK_POINTER(penState, USB_EINVAL);
	APPCOMM_CHECK_EXPR(s_stUSBCtx.bInit, USB_ENOTINIT);
	// CVI_LOGD("usb state[%s]\n", USB_GetStateStr(s_stUSBCtx.enState));
	*penState = s_stUSBCtx.enState;
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

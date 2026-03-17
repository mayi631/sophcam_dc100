#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "storage.h"
#include "storagemng.h"
#include "osal.h"
#include "sysutils_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

static STORAGE_SERVICE_HANDLE_T StgHdl;
static STORAGE_SERVICE_PARAM_S StgParam;

static int32_t STG_Monitor_StatusNotify(STG_STATE_E enState)
{
	/* Publish Event */
	EVENT_S stEvent;
	switch (enState) {
	case STG_STATE_DEV_UNPLUGGED:
		stEvent.topic = EVENT_STORAGEMNG_DEV_UNPLUGED;
		break;
	case STG_STATE_DEV_CONNECTING:
		stEvent.topic = EVENT_STORAGEMNG_DEV_CONNECTING;
		break;
	case STG_STATE_DEV_ERROR:
		stEvent.topic = EVENT_STORAGEMNG_DEV_ERROR;
		break;
	case STG_STATE_FS_CHECKING:
		stEvent.topic = EVENT_STORAGEMNG_FS_CHECKING;
		break;
	case STG_STATE_FS_CHECK_FAILED:
		stEvent.topic = EVENT_STORAGEMNG_FS_CHECK_FAILED;
		break;
	case STG_STATE_FS_EXCEPTION:
		stEvent.topic = EVENT_STORAGEMNG_FS_EXCEPTION;
		break;
	case STG_STATE_MOUNTED:
		stEvent.topic = EVENT_STORAGEMNG_MOUNTED;
		break;
	case STG_STATE_MOUNT_FAILED:
		stEvent.topic = EVENT_STORAGEMNG_MOUNT_FAILED;
		break;
	case STG_STATE_READ_ONLY:
		stEvent.topic = EVENT_STORAGEMNG_MOUNT_READ_ONLY;
		break;
	case STG_STATE_IDLE:
		CVI_LOGD("Idle State, igore\n");
		return -1;
	default:
		CVI_LOGW("Invalid State[%d]\n", enState);
		return -1;
	}
	EVENTHUB_Publish(&stEvent);
	return 0;
}

static int32_t STORAGEMNG_CallBack(STG_STATE_E state)
{
	int32_t s32Ret = 0;
	s32Ret = STG_Monitor_StatusNotify(state);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_RegisterEvent(void)
{
	int32_t s32Ret = 0;
	s32Ret = EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_DEV_UNPLUGED);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_DEV_CONNECTING);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_DEV_ERROR);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_FS_CHECKING);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_FS_CHECK_FAILED);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_FS_EXCEPTION);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_MOUNTED);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_MOUNT_FAILED);
	s32Ret |= EVENTHUB_RegisterTopic(EVENT_STORAGEMNG_MOUNT_READ_ONLY);
	APPCOMM_CHECK_RETURN(s32Ret, STORAGEMNG_EREGISTER_EVENT);
	return 0;
}

int32_t STORAGEMNG_Create(STG_DEVINFO_S *stg_attr)
{
	APPCOMM_CHECK_POINTER(stg_attr, STORAGEMNG_EINVAL);
	int32_t s32Ret = 0;
	memcpy(&StgParam.devinfo, stg_attr, sizeof(STG_DEVINFO_S));
	StgParam.storage_event_callback = STORAGEMNG_CallBack;

	s32Ret = STORAGE_SERVICE_Create(&StgHdl, &StgParam);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_Destroy(void)
{
	int32_t s32Ret = 0;
	s32Ret = STORAGE_SERVICE_Destroy(StgHdl);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_GetFSInfo(STG_FS_INFO_S *pstInfo)
{
	APPCOMM_CHECK_POINTER(pstInfo, STORAGEMNG_EINVAL);
	int32_t s32Ret = 0;
	s32Ret = STG_GetFsInfo(StgHdl, pstInfo);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_GetInfo(STG_DEV_INFO_S *pstInfo)
{
	APPCOMM_CHECK_POINTER(pstInfo, STORAGEMNG_EINVAL);
	int32_t s32Ret = 0;
	s32Ret = STORAGE_SERVICE_GetDevInfo(StgHdl, pstInfo);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_Format(char *labelname)
{
	int32_t s32Ret = 0;
	s32Ret = STORAGE_SERVICE_Format(StgHdl, labelname);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_Mount(void)
{
	int32_t s32Ret = 0;
	s32Ret = STORAGE_SERVICE_Mount(StgHdl);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

int32_t STORAGEMNG_Umount(void)
{
	int32_t s32Ret = 0;
	s32Ret = STORAGE_SERVICE_Umount(StgHdl);
	APPCOMM_CHECK_EXPR(0 == s32Ret, s32Ret);

	return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

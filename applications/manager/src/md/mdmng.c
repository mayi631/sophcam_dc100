#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>

#include "mdmng.h"
#include "osal.h"
#include "sysutils_eventhub.h"
#include "md_ser.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t MD_RegisterEvent(void)
{
	int32_t s32Ret = 0;
	s32Ret = EVENTHUB_RegisterTopic(EVENT_MD_CHANGE);
	APPCOMM_CHECK_RETURN(s32Ret, MD_EREGISTER_EVENT);
	return s32Ret;
}

int32_t MD_Callback(int32_t id, int32_t event)
{
	EVENT_S stEvent;
	memset(&stEvent, 0x0, sizeof(stEvent));
	switch (event) {
	case MD_EVENT_CHANGE:
		stEvent.topic = EVENT_MD_CHANGE;
		stEvent.arg1 = id;
		break;

	default:
		break;
	}
	EVENTHUB_Publish(&stEvent);
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
#include <sys/statfs.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "net.h"
#include "netctrl.h"
#include "netctrlinner.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static int32_t NETCTRL_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = EVENTHUB_RegisterTopic(EVENT_NETCTRL_CONNECT);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_NETCTRL_UIUPDATE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_NETCTRL_APPCONNECT_SUCCESS);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_NETCTRL_APPDISCONNECT);
    if (0 != s32Ret) {
        CVI_LOGE("Register NET event fail\n");
        return -1;
    }

    return 0;
}

int32_t NETCTRL_Init(void)
{
    int32_t s32Ret = 0;
    s32Ret = NETCTRLINNER_InitTimer();
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "NETCTRLINNER_InitTimer");
    s32Ret = NETCTRL_RegisterEvent();
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "NETCTRL_RegisterEvent");
    s32Ret = NETCTRLINNER_SubscribeEvents();
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "NETCTRLINNER_SubscribeEvents");
    NETCTRLINNER_CMDRegister();
    s32Ret = NET_Init();
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "NET_Init");
    return s32Ret;
}

int32_t NETCTRL_DeInit(void)
{
    int32_t s32Ret = 0;
    s32Ret = NET_DeInit();
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "NET_DeInit");
    s32Ret = NETCTRLINNER_DeInitTimer();
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, s32Ret, "NETCTRLINNER_DeInitTimer");
    return s32Ret;
}

int32_t NETCTRL_NetToUiConnectState(void)
{
    return NETCTRLINNER_APPConnetState();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>

#include "appcomm.h"
#include "media_init.h"
#include "param.h"
#include "system.h"


int MEDIA_DISP_Pause(void){
    MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
    int32_t s32Ret = 0;

    s32Ret = MAPI_DISP_Pause(Syshdl->dispHdl);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_DISP_Pause fail");
    return 0;
}

int MEDIA_DISP_Resume(void){
    MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
    int32_t s32Ret = 0;

    s32Ret = MAPI_DISP_Resume(Syshdl->dispHdl);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_DISP_Resume fail");
    return 0;
}

int MEDIA_DISP_ClearBuf(void){
    MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
    int32_t s32Ret = 0;

    s32Ret = MAPI_DISP_ClearBuf(Syshdl->dispHdl);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_DISP_ClearBuf fail");
    return 0;
}

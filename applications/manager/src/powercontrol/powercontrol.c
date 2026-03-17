#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "powercontrol.h"


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define PWRCTRL_TASK_NAME_LEN 16
#define MUTEX_TRYLOCK(mutex) (pthread_mutex_trylock(&mutex))
/** power control task context */
typedef struct tagPWRCTRL_TASK_CONTEXT_S
{
    bool bDormantState;/**<true:dormant,false:normal*/
    int32_t s32PauseCount;
    uint32_t TimedTaskId;
    char szPwrCtrlTaskName[PWRCTRL_TASK_NAME_LEN];
    bool bCtrlEventHandleOnDormant;
    PWRCTRL_TASK_CFG_S stCfg;
} PWRCTRL_TASK_CONTEXT_S;

/** power control mangae info */
typedef struct tagPWRCTRL_MGR_S
{
    pthread_mutex_t Mutex;
    bool bInitState;/**<true:initallized,false:not init or deinitallize*/
    PWRCTRL_TASK_CONTEXT_S astTaskContext[PWRCTRL_TASK_BUIT];
} PWRCTRL_MGR_S;

static PWRCTRL_MGR_S s_stPWRCTRLMgr = {.Mutex = PTHREAD_MUTEX_INITIALIZER,.bInitState = false,\
    .astTaskContext[PWRCTRL_TASK_SCREENDORMANT].szPwrCtrlTaskName="screendormant",\
    .astTaskContext[PWRCTRL_TASK_SCREENDORMANT].bCtrlEventHandleOnDormant=true,\
    .astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT].szPwrCtrlTaskName="sysdormant",\
    .astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT].bCtrlEventHandleOnDormant=true};


static void POWERCTRL_DormantProc(void *privData, struct timespec *now)
{
    int32_t s32Ret = 0;
    TIMEDTASK_ATTR_S stAttr;

    s32Ret = MUTEX_TRYLOCK(s_stPWRCTRLMgr.Mutex);/**MUTEX_LOCK(s_stPWRCTRLMgr.Mutex)*/
    if (s32Ret != 0) {
        /** <mutex fail */
        CVI_LOGI("be about to dormant,but busy\n");
        return;
    }
    if (true != s_stPWRCTRLMgr.bInitState) {
        MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        CVI_LOGE("power control not init\n");
        return;
    }
    PWRCTRL_TASK_CONTEXT_S *pstTaskContext = (PWRCTRL_TASK_CONTEXT_S *)privData;

    if (NULL == pstTaskContext->stCfg.pfnDormantProc) {
        MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        CVI_LOGE("pfnDormantProc is NULL.should run here\n");
        return;
    }
    if (true != pstTaskContext->bDormantState) {
        if (0 == pstTaskContext->s32PauseCount) {
            s32Ret = pstTaskContext->stCfg.pfnDormantProc(pstTaskContext->stCfg.pvDormantPrivData);
            if (0 != s32Ret) {
                CVI_LOGW("pfnScreenDormantProc fail\n");
            } else {
                 pstTaskContext->bDormantState = true;
                 stAttr.bEnable = false;
                 stAttr.u32Time_sec = pstTaskContext->stCfg.stAttr.u32Time_sec;
                 s32Ret = TIMEDTASK_SetAttr(pstTaskContext->TimedTaskId,&stAttr);
                 if (0 != s32Ret) {
                     CVI_LOGE("set task[%s] attr error\n",pstTaskContext->szPwrCtrlTaskName);
                 }
            }
        } else {
            CVI_LOGE("task(%s) need dormant,but s32PauseCount(%d),need not run here\n", \
                pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
        }
    }
    MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return;
}

static int32_t POWERCTRL_WakeupEventTaskProc(void* pvPrivData)
{
    int32_t s32Ret = 0;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    if (NULL == pstTaskContext->stCfg.pfnWakeupProc) {
        CVI_LOGE("pfnWakeupProc is NULL.should run here\n");
        return PWRCTRL_EFATA;
    }
    s32Ret = pstTaskContext->stCfg.pfnWakeupProc(pstTaskContext->stCfg.pvWakeupPrivData);
    if (0 != s32Ret) {
        CVI_LOGE("pfnWakeupProc (%s) fail\n",pstTaskContext->szPwrCtrlTaskName);
        s32Ret = PWRCTRL_EWAKEUPCB;
    }
    return s32Ret;
}

static int32_t POWERCTRL_WakeupEventProc(PWRCTRL_WAKEUP_TACTICS_E enWakeupTacticsType,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = NULL;
    int32_t s32Index = PWRCTRL_TASK_BUIT;
    *pbEventContinueHandle = true;
    switch (enWakeupTacticsType) {
        case PWRCTRL_WAKEUP_TACTICS_DISCARD: /** pause screen dormant check proc */
        case PWRCTRL_WAKEUP_TACTICS_CONTINUE:
            break;
        default:
            return PWRCTRL_EINVAL;
    }
    for (s32Index = PWRCTRL_TASK_SYSTEMDORMANT ;s32Index >= PWRCTRL_TASK_SCREENDORMANT;s32Index--) {
        pstTaskContext = &s_stPWRCTRLMgr.astTaskContext[s32Index];
        if (true == pstTaskContext->bDormantState) {
            s32Ret = POWERCTRL_WakeupEventTaskProc(pstTaskContext);
            if (0 !=s32Ret) {
                CVI_LOGE("(%s) wakeup fail\n",pstTaskContext->szPwrCtrlTaskName);
                return s32Ret;
            }
            pstTaskContext->bDormantState = false;

            if (0 == pstTaskContext->s32PauseCount) {
               s32Ret = TIMEDTASK_SetAttr(pstTaskContext->TimedTaskId,&pstTaskContext->stCfg.stAttr);
               if (0 != s32Ret) {
                    CVI_LOGE("reset dormant (%s) attr fail\n",pstTaskContext->szPwrCtrlTaskName);
                    return PWRCTRL_ETIMEDTASK;
               }
            } else {
                CVI_LOGI("wakeup TaskName(%s),but s32PauseCount(%d) can not restore timetask\n",pstTaskContext->szPwrCtrlTaskName, \
                    pstTaskContext->s32PauseCount);
                if (pstTaskContext->bCtrlEventHandleOnDormant == false) {
                    CVI_LOGE("task(%s) wakeup,but s32PauseCount(%d),never run here,fail\n", pstTaskContext->szPwrCtrlTaskName, \
                        pstTaskContext->s32PauseCount);
                    return PWRCTRL_EINTER;
                }
            }
            /** when dormant,the event just use for wakeup ,no need proc the event */
            if (PWRCTRL_WAKEUP_TACTICS_DISCARD == enWakeupTacticsType) {
                *pbEventContinueHandle = false;
            }
        }
    }
    return 0;
}

static int32_t POWERCTRL_PauseEventTaskProc(void* pvPrivData,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    TIMEDTASK_ATTR_S stAttr;

    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    *pbEventContinueHandle = true;
    if (true == pstTaskContext->bDormantState) {
        CVI_LOGI("pause TaskName(%s) dormant,control event Low probability come here\n",pstTaskContext->szPwrCtrlTaskName);
        *pbEventContinueHandle = false;/**event discard on system dormant,to solve occur switch hdmi on dormant*/
        if (pstTaskContext->bCtrlEventHandleOnDormant == false) {
            return PWRCTRL_ELOGICFLOW;
        }
    }
    if (0 == pstTaskContext->s32PauseCount) {
        pstTaskContext->s32PauseCount++;

        stAttr.bEnable = false;
        stAttr.u32Time_sec = pstTaskContext->stCfg.stAttr.u32Time_sec;

        s32Ret = TIMEDTASK_SetAttr(pstTaskContext->TimedTaskId,&stAttr);
        if (0 != s32Ret) {
            CVI_LOGE("set task[%s] attr error\n",pstTaskContext->szPwrCtrlTaskName);
            s32Ret = PWRCTRL_ETIMEDTASK;
        }
    } else if(0 < pstTaskContext->s32PauseCount) {
        pstTaskContext->s32PauseCount++;
    } else {
        CVI_LOGE("TaskName[%s] pause ,s32PauseCount(%d),error\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
        return PWRCTRL_EINTER;
    }
    CVI_LOGD("after pause TaskName[%s] dormant check,s32PauseCount(%d)\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
    return 0;
}


static int32_t POWERCTRL_PauseEventProc(PWRCTRL_EVENT_SCOPE_E enScopeType,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    bool bContinueHandle = true;
    switch(enScopeType)
    {
        case PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN: /** pause screen dormant check proc */
            s32Ret = POWERCTRL_PauseEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SCREENDORMANT],&bContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            s32Ret = POWERCTRL_PauseEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        case PWRCTRL_EVENT_SCOPE_SYSTEM:
            s32Ret = POWERCTRL_PauseEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        default:
            s32Ret = PWRCTRL_EINVAL;
            break;
    }
    if(false == bContinueHandle)
        *pbEventContinueHandle = false;

    return s32Ret;
}

static int32_t POWERCTRL_ResumeEventTaskProc(void* pvPrivData,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    *pbEventContinueHandle = true;
    if (true == pstTaskContext->bDormantState) {
        CVI_LOGE("resumme TaskName[%s] dormant,control event should not come here,fail\n",pstTaskContext->szPwrCtrlTaskName);
        *pbEventContinueHandle = false;
        if (pstTaskContext->bCtrlEventHandleOnDormant == false) {
            return PWRCTRL_ELOGICFLOW;
        }
    }
    if (1 == pstTaskContext->s32PauseCount) {
        pstTaskContext->s32PauseCount = 0;

        s32Ret = TIMEDTASK_SetAttr(pstTaskContext->TimedTaskId,&pstTaskContext->stCfg.stAttr);
        if (0 != s32Ret) {
             CVI_LOGE("reset dormant (%s) attr fail\n",pstTaskContext->szPwrCtrlTaskName);
             return PWRCTRL_ETIMEDTASK;
        }
    } else if (1 < pstTaskContext->s32PauseCount) {
        pstTaskContext->s32PauseCount--;
    } else {
        CVI_LOGE("TaskName[%s] resumme ,s32PauseCount(%d),error\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
        return PWRCTRL_EFATA;
    }
    CVI_LOGD("after resumme TaskName[%s] dormant check,s32PauseCount(%d)\n",pstTaskContext->szPwrCtrlTaskName,pstTaskContext->s32PauseCount);
    return 0;
}

static int32_t POWERCTRL_ResumeEventProc(PWRCTRL_EVENT_SCOPE_E enScopeType,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    bool bContinueHandle = true;
    switch(enScopeType)
    {
        case PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN: /** pause screen dormant check proc */
            s32Ret = POWERCTRL_ResumeEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SCREENDORMANT],&bContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            s32Ret = POWERCTRL_ResumeEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        case PWRCTRL_EVENT_SCOPE_SYSTEM:
            s32Ret = POWERCTRL_ResumeEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT],pbEventContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            break;
        default:
            s32Ret = PWRCTRL_EINVAL;
            break;
    }
    if(false == bContinueHandle)
        *pbEventContinueHandle = false;
    return s32Ret;
}

static int32_t POWERCTRL_CommonEventTaskProc(void* pvPrivData,bool* pbEventContinueHandle)
{
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    *pbEventContinueHandle = true;
    if (true == pstTaskContext->bDormantState) {
        *pbEventContinueHandle = false;
    }
    return 0;
}


static int32_t POWERCTRL_CommonEventProc(PWRCTRL_EVENT_SCOPE_E enScopeType,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    *pbEventContinueHandle = true;
    switch(enScopeType)
    {
        case PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN: /** touch action discard on screen dormant*/
            s32Ret = POWERCTRL_CommonEventTaskProc(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SCREENDORMANT],pbEventContinueHandle);
            APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
            /**event continue handler on system dormant,to solve common message and system dormant supervene*/
            break;
        case PWRCTRL_EVENT_SCOPE_SYSTEM:
            /**event continue handler on system dormant,to solve common message and system dormant supervene*/
            break;
        default:
            s32Ret = PWRCTRL_EINVAL;
            break;
    }
    return s32Ret;
}

static int32_t POWERCTRL_ResetTaskCheckTimer(void* pvPrivData)
{
    int32_t s32Ret = 0;
    PWRCTRL_TASK_CONTEXT_S* pstTaskContext = (PWRCTRL_TASK_CONTEXT_S*)pvPrivData;
    if (true == pstTaskContext->stCfg.stAttr.bEnable) {
        s32Ret=TIMEDTASK_ResetTime(pstTaskContext->TimedTaskId);
        if (0 != s32Ret) {
            CVI_LOGE("reset timer (%s) error\n",pstTaskContext->szPwrCtrlTaskName);
            s32Ret = PWRCTRL_ETIMEDTASK;
        }
    }
    return s32Ret;
}
static int32_t POWERCTRL_ResetCheckTimer(void)
{
    int32_t s32Ret = 0;
    s32Ret = POWERCTRL_ResetTaskCheckTimer(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SCREENDORMANT]);
    APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    s32Ret = POWERCTRL_ResetTaskCheckTimer(&s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT]);
    APPCOMM_CHECK_RETURN(s32Ret,s32Ret);
    return s32Ret;
}


int32_t POWERCTRL_Init(const PWRCTRL_CFG_S* pstCfg)
{
    int32_t s32Ret = 0;
    int32_t s32Index = PWRCTRL_TASK_BUIT;
    uint32_t TimedTaskId = 0;
    TIMEDTASK_CFG_S stTimedTaskCfg;
    PWRCTRL_TASK_PROC_CALLBACK pfnDormantProc = NULL;
    PWRCTRL_TASK_PROC_CALLBACK pfnWakeupProc = NULL;
    if (true == s_stPWRCTRLMgr.bInitState) {
        CVI_LOGE("power control already init\n");
        return PWRCTRL_EINITIALIZED;
    }
    /** parm invalid check */
    APPCOMM_CHECK_POINTER(pstCfg,PWRCTRL_EINVAL);
    for (s32Index = PWRCTRL_TASK_SCREENDORMANT ;s32Index < PWRCTRL_TASK_BUIT;s32Index++) {
        pfnDormantProc = pstCfg->astTaskCfg[s32Index].pfnDormantProc;
        pfnWakeupProc = pstCfg->astTaskCfg[s32Index].pfnWakeupProc;
        APPCOMM_CHECK_EXPR(((pfnDormantProc != NULL && pfnWakeupProc != NULL) \
            || (pfnDormantProc == NULL && pfnWakeupProc == NULL)),PWRCTRL_EINVAL);

        APPCOMM_CHECK_EXPR((pstCfg->astTaskCfg[s32Index].stAttr.bEnable ==false) || \
            ((pstCfg->astTaskCfg[s32Index].stAttr.bEnable ==true)&&(pstCfg->astTaskCfg[s32Index].stAttr.u32Time_sec !=0)), \
            PWRCTRL_EINVAL);
    }
    /**check relation screen dormant time and system dormant time invalid*/
    if (pstCfg->astTaskCfg[PWRCTRL_TASK_SCREENDORMANT].stAttr.bEnable && pstCfg->astTaskCfg[PWRCTRL_TASK_SYSTEMDORMANT].stAttr.bEnable) {
        APPCOMM_CHECK_EXPR((pstCfg->astTaskCfg[PWRCTRL_TASK_SCREENDORMANT].stAttr.u32Time_sec <= \
            pstCfg->astTaskCfg[PWRCTRL_TASK_SYSTEMDORMANT].stAttr.u32Time_sec),PWRCTRL_EINVAL);
    }

    /** time task register */
    for (s32Index = PWRCTRL_TASK_SCREENDORMANT; s32Index < PWRCTRL_TASK_BUIT; s32Index++) {
        stTimedTaskCfg.timerProc = POWERCTRL_DormantProc;
        stTimedTaskCfg.pvPrivData = (void*)&s_stPWRCTRLMgr.astTaskContext[s32Index];
        stTimedTaskCfg.stAttr.bEnable = pstCfg->astTaskCfg[s32Index].stAttr.bEnable;
        stTimedTaskCfg.stAttr.u32Time_sec = pstCfg->astTaskCfg[s32Index].stAttr.u32Time_sec;
        stTimedTaskCfg.stAttr.periodic = false;
        s32Ret = TIMEDTASK_Create(&stTimedTaskCfg,&TimedTaskId);
        if (0 != s32Ret) {
            CVI_LOGE("dormant task(%s) creat error\n",s_stPWRCTRLMgr.astTaskContext[s32Index].szPwrCtrlTaskName);
            return PWRCTRL_ETIMEDTASK;
        }
        s_stPWRCTRLMgr.astTaskContext[s32Index].TimedTaskId= TimedTaskId;
    }
    /** parm init */
    for (s32Index = PWRCTRL_TASK_SCREENDORMANT; s32Index < PWRCTRL_TASK_BUIT; s32Index++) {
        s_stPWRCTRLMgr.astTaskContext[s32Index].bDormantState = false;
        s_stPWRCTRLMgr.astTaskContext[s32Index].s32PauseCount = 0;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnDormantProc = pstCfg->astTaskCfg[s32Index].pfnDormantProc;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvDormantPrivData = pstCfg->astTaskCfg[s32Index].pvDormantPrivData;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnWakeupProc = pstCfg->astTaskCfg[s32Index].pfnWakeupProc;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvWakeupPrivData = pstCfg->astTaskCfg[s32Index].pvWakeupPrivData;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.bEnable = pstCfg->astTaskCfg[s32Index].stAttr.bEnable;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.u32Time_sec = pstCfg->astTaskCfg[s32Index].stAttr.u32Time_sec;
        CVI_LOGD("bEnable[%d] u32Time_sec[%d],szPwrCtrlTaskName[%s]\n",s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.bEnable,
            s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.u32Time_sec,s_stPWRCTRLMgr.astTaskContext[s32Index].szPwrCtrlTaskName);
    }
    s_stPWRCTRLMgr.bInitState = true;
    return 0;
}

int32_t POWERCTRL_DeInit(void)
{
    int32_t s32Ret = 0;
    int32_t s32Index = PWRCTRL_TASK_BUIT;
    MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if (true != s_stPWRCTRLMgr.bInitState) {
        MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        CVI_LOGE("power control not init\n");
        return PWRCTRL_ENOINIT;
    }
    for (s32Index = PWRCTRL_TASK_SCREENDORMANT ;s32Index < PWRCTRL_TASK_BUIT;s32Index++) {
        s32Ret = TIMEDTASK_Destroy(s_stPWRCTRLMgr.astTaskContext[s32Index].TimedTaskId);
        if (0 != s32Ret) {
            MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
            CVI_LOGE("dormant task(%s) destroy error\n",s_stPWRCTRLMgr.astTaskContext[s32Index].szPwrCtrlTaskName);
            return PWRCTRL_ETIMEDTASK;
        }
    }
    for (s32Index = PWRCTRL_TASK_SCREENDORMANT ;s32Index < PWRCTRL_TASK_BUIT;s32Index++) {
        s_stPWRCTRLMgr.astTaskContext[s32Index].TimedTaskId= 0;
        s_stPWRCTRLMgr.astTaskContext[s32Index].s32PauseCount = 0;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnDormantProc = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvDormantPrivData = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pfnWakeupProc = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.pvWakeupPrivData = NULL;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.bEnable = false;
        s_stPWRCTRLMgr.astTaskContext[s32Index].stCfg.stAttr.u32Time_sec = 0;
    }
    s_stPWRCTRLMgr.bInitState = false;
    MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return 0;
}

int32_t POWERCTRL_GetTaskAttr(PWRCTRL_TASK_E enType,TIMEDTASK_ATTR_S* pstTaskAttr)
{
    APPCOMM_CHECK_POINTER(pstTaskAttr,PWRCTRL_EINVAL);
    MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if (true != s_stPWRCTRLMgr.bInitState) {
        MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        CVI_LOGE("power control not init\n");
        return PWRCTRL_ENOINIT;
    }
    switch(enType)
    {
        case PWRCTRL_TASK_SCREENDORMANT:
            break;
        case PWRCTRL_TASK_SYSTEMDORMANT:
            break;
        default:
            MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
            CVI_LOGE("set time enType(%d),value exceed\n",enType);
            return PWRCTRL_EINVAL;
    }
    pstTaskAttr->bEnable = s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.bEnable;
    pstTaskAttr->u32Time_sec = s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.u32Time_sec;
    MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return 0;
}

int32_t POWERCTRL_SetTaskAttr(PWRCTRL_TASK_E enType,const TIMEDTASK_ATTR_S* pstTaskAttr)
{
    int32_t s32Ret = 0;
    uint32_t s32Index = 0;
    PWRCTRL_TASK_E enAutoTaskType = PWRCTRL_TASK_BUIT;
    APPCOMM_CHECK_POINTER(pstTaskAttr,PWRCTRL_EINVAL);
    APPCOMM_CHECK_EXPR((pstTaskAttr->bEnable == false) || ((pstTaskAttr->bEnable == true)&&(pstTaskAttr->u32Time_sec != 0)), \
        PWRCTRL_EINVAL);

    MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if (true != s_stPWRCTRLMgr.bInitState) {
        MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        CVI_LOGE("power control not init\n");
        return PWRCTRL_ENOINIT;
    }
    switch(enType)
    {
        case PWRCTRL_TASK_SCREENDORMANT:
            if ((true == pstTaskAttr->bEnable) && \
                (true == s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT].stCfg.stAttr.bEnable) && \
                (pstTaskAttr->u32Time_sec  > s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SYSTEMDORMANT].stCfg.stAttr.u32Time_sec )) {
                enAutoTaskType = PWRCTRL_TASK_SYSTEMDORMANT;
            }
            break;
        case PWRCTRL_TASK_SYSTEMDORMANT:
            if ((true == pstTaskAttr->bEnable) && \
                (true == s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SCREENDORMANT].stCfg.stAttr.bEnable) && \
               (pstTaskAttr->u32Time_sec  < s_stPWRCTRLMgr.astTaskContext[PWRCTRL_TASK_SCREENDORMANT].stCfg.stAttr.u32Time_sec )) {
                enAutoTaskType = PWRCTRL_TASK_SCREENDORMANT;
            }
            break;
        default:
            MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
            CVI_LOGE("set time enType(%d),value exceed\n",enType);
            return PWRCTRL_EINVAL;
    }
    for (s32Index = PWRCTRL_TASK_SCREENDORMANT;s32Index < PWRCTRL_TASK_BUIT;s32Index++) {
        if (s32Index == enType) {
            s32Ret = TIMEDTASK_SetAttr(s_stPWRCTRLMgr.astTaskContext[enType].TimedTaskId,pstTaskAttr);
            if (0 != s32Ret) {
                MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
                CVI_LOGE("timetask[%s] set Attr error\n",s_stPWRCTRLMgr.astTaskContext[enType].szPwrCtrlTaskName);
                return PWRCTRL_ETIMEDTASK;
            }
            s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.bEnable = pstTaskAttr->bEnable;
            s_stPWRCTRLMgr.astTaskContext[enType].stCfg.stAttr.u32Time_sec = pstTaskAttr->u32Time_sec;
            if (PWRCTRL_TASK_BUIT != enAutoTaskType) {
                s32Ret = TIMEDTASK_SetAttr(s_stPWRCTRLMgr.astTaskContext[enAutoTaskType].TimedTaskId,pstTaskAttr);
                if (0 != s32Ret) {
                    MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
                    CVI_LOGE("timetask[%s] set Attr error\n",s_stPWRCTRLMgr.astTaskContext[enAutoTaskType].szPwrCtrlTaskName);
                    return PWRCTRL_ETIMEDTASK;
                }
                s_stPWRCTRLMgr.astTaskContext[enAutoTaskType].stCfg.stAttr.u32Time_sec = pstTaskAttr->u32Time_sec;
                s32Ret = PWRCTRL_ETASKTIMEAUTO;
            }
            break;
        }
    }
    MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return s32Ret;
}

int32_t POWERCTRL_EventPreProc(const PWRCTRL_EVENT_ATTR_S* pstEventAttr,bool* pbEventContinueHandle)
{
    int32_t s32Ret = 0;
    bool bResetTimer = false;
    APPCOMM_CHECK_POINTER(pstEventAttr,PWRCTRL_EINVAL);
    APPCOMM_CHECK_POINTER(pbEventContinueHandle,PWRCTRL_EINVAL);
    MUTEX_LOCK(s_stPWRCTRLMgr.Mutex);
    if (true != s_stPWRCTRLMgr.bInitState) {
        MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
        CVI_LOGE("power control not init\n");
        return PWRCTRL_ENOINIT;
    }
    switch(pstEventAttr->enType)
    {
        case PWRCTRL_EVENT_TYPE_WAKEUP:
            s32Ret = POWERCTRL_WakeupEventProc(pstEventAttr->unCfg.stWakeupCfg.enType,pbEventContinueHandle);
            bResetTimer = pstEventAttr->unCfg.stWakeupCfg.stCommonCfg.bResetTimer;
            break;

        case PWRCTRL_EVENT_TYPE_CONTROL:
            switch(pstEventAttr->unCfg.stCtrlCfg.enType)
            {

                case PWRCTRL_EVENT_CONTROL_PAUSE:
                    s32Ret = POWERCTRL_PauseEventProc(pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.enType,pbEventContinueHandle);
                    bResetTimer = pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.bResetTimer;
                    break;
                case PWRCTRL_EVENT_CONTROL_RESUME:
                    s32Ret = POWERCTRL_ResumeEventProc(pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.enType,pbEventContinueHandle);
                    bResetTimer = pstEventAttr->unCfg.stCtrlCfg.stCommonCfg.bResetTimer;
                    break;
                default:
                    s32Ret = PWRCTRL_EINVAL;
                    break;
            }
            break;
        case PWRCTRL_EVENT_TYPE_COMMON:
            s32Ret = POWERCTRL_CommonEventProc(pstEventAttr->unCfg.stCommonCfg.enType,pbEventContinueHandle);
            bResetTimer = pstEventAttr->unCfg.stCommonCfg.bResetTimer;
            break;
        default:
            s32Ret = PWRCTRL_EINVAL;
            break;
    }
    if (0 == s32Ret) {
        if(true == bResetTimer)
            s32Ret = POWERCTRL_ResetCheckTimer();
    }
    MUTEX_UNLOCK(s_stPWRCTRLMgr.Mutex);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

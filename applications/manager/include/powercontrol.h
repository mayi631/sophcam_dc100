#ifndef __POWERCONTROL_H__
#define __POWERCONTROL_H__

#include "appcomm.h"
#include "timedtask.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     POWERCONTROL */
/** @{ */  /** <!-- [POWERCONTROL] */
/** macro define */
#define PWRCTRL_EINVAL            APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_EINVAL)    /**<parm invlid*/
#define PWRCTRL_EINTER            APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_EINTER)    /**<intern error*/
#define PWRCTRL_ENOINIT           APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ENOINIT)  /**< no initialize*/
#define PWRCTRL_EBUSY             APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_EBUSY)  /**<service busy*/
#define PWRCTRL_EINITIALIZED      APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_EINITIALIZED) /**< already initialized */
#define PWRCTRL_ETIMEDTASK        APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ERRNO_CUSTOM_BOTTOM) /**<timedtask module error*/
#define PWRCTRL_EWAKEUPCB         APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ERRNO_CUSTOM_BOTTOM+1) /**<wakeup callback error*/
#define PWRCTRL_EDORMANTCB        APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ERRNO_CUSTOM_BOTTOM+2) /**<dormant callback error*/
#define PWRCTRL_ELOGICFLOW        APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ERRNO_CUSTOM_BOTTOM+3) /**<logical flow error*/
#define PWRCTRL_EFATA             APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ERRNO_CUSTOM_BOTTOM+4) /**<fata error*/

/**set screen time > sys time,sys time self-adjusting with screen time;
   set sys time < screen timescreen time self-adjusting with screen time*/
#define PWRCTRL_ETASKTIMEAUTO     APP_APPCOMM_ERR_ID(APP_MOD_PM,APP_ERRNO_CUSTOM_BOTTOM+5)


/**  ==== power control task configure begin ==== */

typedef int32_t (*PWRCTRL_TASK_PROC_CALLBACK)(void* pvPrivData);/**< dormant callback process function */

typedef enum PWRCTRL_TASK_E /**<pwrctrl task type */
{
    PWRCTRL_TASK_SCREENDORMANT = 0,
    PWRCTRL_TASK_SYSTEMDORMANT,
    PWRCTRL_TASK_BUIT
}PWRCTRL_TASK_E;

typedef struct PWRCTRL_TASK_CFG__S /**< task static attribute */
{
    TIMEDTASK_ATTR_S stAttr;
    PWRCTRL_TASK_PROC_CALLBACK pfnDormantProc;
    void* pvDormantPrivData;
    PWRCTRL_TASK_PROC_CALLBACK pfnWakeupProc;
    void* pvWakeupPrivData;
} PWRCTRL_TASK_CFG_S;

typedef struct PWRCTRL_CFG__S
{
    PWRCTRL_TASK_CFG_S astTaskCfg[PWRCTRL_TASK_BUIT];
} PWRCTRL_CFG_S;

/** ==== power control task attribute end===*/


/**  ==== event configuue begin ==== */
typedef enum PWRCTRL_EVENT_TYPE_E /**<event type */
{
    PWRCTRL_EVENT_TYPE_WAKEUP = 0,    /**<wakeup dormant*/
    PWRCTRL_EVENT_TYPE_CONTROL,       /**<pause or resumme dormant check */
    PWRCTRL_EVENT_TYPE_COMMON,        /**<general event,no both of wakeup and control function */
    PWRCTRL_EVENT_TYPE_BUIT
}PWRCTRL_EVENT_TYPE_E;


typedef enum PWRCTRL_EVENT_SCOPE_E /**< event action scope */
{
    /**control event: control(pause or resume) system dormant check at normal state(no system dormant)
       common event:  do not continue handle the event at sys dormant
       wakeup event:  wakeup sstem at sys dormant  */
    PWRCTRL_EVENT_SCOPE_SYSTEM = 0,
    /**control event: control(pause or resume) system dormant and screen dormant check at normal state
       common event:  do not continue handle the event at sys dormant or screen dormant
       wakeup event:  wakeup sstem at sys dormant or screen dormant */
    PWRCTRL_EVENT_SCOPE_SYSTEM_SCREEN,
    PWRCTRL_EVENT_SCOPE_BUIT
}PWRCTRL_EVENT_SCOPE_E;


typedef struct PWRCTRL_EVENT_COMMON_ATTR_S/**< common event attribute */
{
    PWRCTRL_EVENT_SCOPE_E enType;
    bool bResetTimer;
} PWRCTRL_EVENT_COMMON_ATTR_S;


typedef enum PWRCTRL_WAKEUP_TACTICS_E /**< wakeup event process tactics type,whether event need continue proc or not*/
{
    PWRCTRL_WAKEUP_TACTICS_DISCARD = 0, /**<after dong wakeup,event need not continue to proc*/
    PWRCTRL_WAKEUP_TACTICS_CONTINUE,    /**<after dong wakeup,event need continue to proc*/
    PWRCTRL_WAKEUP_TACTICS_BUIT
}PWRCTRL_WAKEUP_TACTICS_E;


typedef struct PWRCTRL_EVENT_WAKEUP_ATTR_S/**< wakeup event attribute */
{
    PWRCTRL_WAKEUP_TACTICS_E enType;
    PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
} PWRCTRL_EVENT_WAKEUP_ATTR_S;


typedef enum PWRCTRL_EVENT_CONTROL_E/**< control event type of action*/
{
    PWRCTRL_EVENT_CONTROL_PAUSE = 0,/**<after wakeup ,ui need not to proc*/
    PWRCTRL_EVENT_CONTROL_RESUME,/**<after wakeup ,ui continue to proc*/
    PWRCTRL_EVENT_CONTROL_BUIT
}PWRCTRL_EVENT_CONTROL_E;



typedef struct PWRCTRL_EVENT_CONTROL_ATTR_S/**< control event attribute*/
{
    PWRCTRL_EVENT_CONTROL_E enType;
    PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
} PWRCTRL_EVENT_CONTROL_ATTR_S;


typedef struct PWRCTRL_EVENT_ATTR_S/**< event configure */
{
    PWRCTRL_EVENT_TYPE_E enType;
    union tagPWRCTRL_EVENT_ATTR_U
    {
        PWRCTRL_EVENT_WAKEUP_ATTR_S stWakeupCfg;/**<wakeup event cfg*/
        PWRCTRL_EVENT_CONTROL_ATTR_S stCtrlCfg;/**<control event cfg*/
        PWRCTRL_EVENT_COMMON_ATTR_S stCommonCfg;
    } unCfg;
} PWRCTRL_EVENT_ATTR_S;

/**  ==== event configuue end ==== */
int32_t POWERCTRL_Init(const PWRCTRL_CFG_S* pstCfg);
int32_t POWERCTRL_GetTaskAttr(PWRCTRL_TASK_E enType,TIMEDTASK_ATTR_S* pstTaskAttr);
int32_t POWERCTRL_SetTaskAttr(PWRCTRL_TASK_E enType,const TIMEDTASK_ATTR_S* pstTaskAttr);
int32_t POWERCTRL_EventPreProc(const PWRCTRL_EVENT_ATTR_S* pstEventAttr,bool* pbEventContinueHandle);
int32_t POWERCTRL_DeInit(void);

/** @}*/  /** <!-- ==== POWERCONTROL End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __POWERCONTROL_H__*/

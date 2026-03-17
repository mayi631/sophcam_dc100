#ifndef __TIMEDTASK_H__
#define __TIMEDTASK_H__

#include "appcomm.h"
#include "sysutils_timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     TIMEDTASK */
/** @{ *//** <!-- [TIMEDTASK] */

typedef struct _TIMEDTASK_ATTR_S {
    bool bEnable;
    uint32_t u32Time_sec; /**<timed-task trigger time, canbe reset */
    bool periodic;   /**< periodic or ont-shot */
} TIMEDTASK_ATTR_S;

typedef struct _TIMEDTASK_CFG_S {
    TIMEDTASK_ATTR_S stAttr;
    TIMER_PROC_CALLBACK *timerProc;
    void *pvPrivData;
} TIMEDTASK_CFG_S;

int32_t TIMEDTASK_Init(void);
int32_t TIMEDTASK_DeInit(void);
int32_t TIMEDTASK_Create(const TIMEDTASK_CFG_S *pstTimeTskCfg, uint32_t *pTimeTskid);
int32_t TIMEDTASK_Destroy(uint32_t TimeTskid);
int32_t TIMEDTASK_GetAttr(uint32_t TimeTskid, TIMEDTASK_ATTR_S *pstTimeTskAttr);
int32_t TIMEDTASK_SetAttr(uint32_t TimeTskid, const TIMEDTASK_ATTR_S *pstTimeTskAttr);
int32_t TIMEDTASK_ResetTime(uint32_t TimeTskid);

/** @} *//** <!-- ==== TIMEDTASK End ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __TIMEDTASK_H__ */


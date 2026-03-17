#ifndef HFSM_H
#define HFSM_H
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "sysutils_eventhub.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define STATE_NAME_LEN                64
#define STATE_MAX_AMOUNT              32
#define PROCESS_MSG_RESULTE_OK        (0)
#define PROCESS_MSG_UNHANDLER         (-1)
typedef void *           HFSM_HANDLE;
typedef EVENT_S      MESSAGE_S;

typedef struct StateInfo {
    uint32_t stateID;                         /* state id */
    char name[STATE_NAME_LEN];            /* state name */
    int32_t (*open)(void);             /* call when state open */
    int32_t (*close)(void);              /* call when state close */
    /* call when process a message */
    int32_t (*processMessage)(MESSAGE_S *msg, void* argv, uint32_t *stateID);
    void* argv;                              /* User private, used by processMessage */
}STATE_S;

/* hfsm event enum */
typedef enum HfsmEventE {
    HFSM_EVENT_HANDLE_MSG = 0,       /* handler message */
    HFSM_EVENT_UNHANDLE_MSG,         /* unhandler message */
    HFSM_EVENT_TRANSTION_ERROR,      /* transtion error */

    HFSM_EVENT_BUTT
} HFSM_EVENT_E;

/* HFSM event information */
typedef struct HiHfsmEventInfo {
    int32_t s32ErrorNo;
    HFSM_EVENT_E enEventCode;
    MESSAGE_S *pstunHandlerMsg;
} HFSM_EVENT_INFO_S;

typedef int32_t (*HFSM_EVENT_CALLBACK)(HFSM_HANDLE hfsmHandle, const HFSM_EVENT_INFO_S *eventInfo);

typedef struct HfsmAttr {
    HFSM_EVENT_CALLBACK fnHfsmEventCallback;
    uint32_t u32StateMaxAmount;
    uint32_t u32MessageQueueSize;
} HFSM_ATTR_S;

int32_t HFSM_Create(HFSM_ATTR_S *fsmAttr, HFSM_HANDLE *hfsm);
int32_t HFSM_Destroy(HFSM_HANDLE hfsm);
int32_t HFSM_AddState(HFSM_HANDLE hfsm, STATE_S *state, STATE_S *parent);
int32_t HFSM_SetInitialState(HFSM_HANDLE hfsm, uint32_t stateID);
int32_t HFSM_GetCurrentState(HFSM_HANDLE hfsm, STATE_S *state);
int32_t HFSM_Start(HFSM_HANDLE hfsm);
int32_t HFSM_Stop(HFSM_HANDLE hfsm);
int32_t HFSM_SendAsyncMessage(HFSM_HANDLE hfsm, MESSAGE_S *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif
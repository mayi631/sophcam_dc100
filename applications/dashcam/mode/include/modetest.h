#ifndef __CC_TOOL_H__
#define __CC_TOOL_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mapi.h"
#include "sysutils_mq.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define CMD_CLIENT_ID_MT_TOOL (MQ_CLIENT_ID_USER_0)
#define CMD_CHANNEL_ID_MT(mt_id) (0x00 + (mt_id))

typedef struct MT_SERVICE_PARAM_S {
    uint32_t    parm;
} MT_SERVICE_PARAM_T;

typedef void * MT_HANDLE_T;
typedef MT_SERVICE_PARAM_T mt_param_t, *mt_param_handle_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

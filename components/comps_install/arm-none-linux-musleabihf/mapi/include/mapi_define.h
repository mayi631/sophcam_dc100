#ifndef __MAPI_DEFINE_H__
#define __MAPI_DEFINE_H__

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAPI_SUCCESS           ((int)(0))
#define MAPI_ERR_FAILURE       ((int)(-1001))
#define MAPI_ERR_NOMEM         ((int)(-1002))
#define MAPI_ERR_TIMEOUT       ((int)(-1003))
#define MAPI_ERR_INVALID       ((int)(-1004))

typedef void * MAPI_HANDLE_T;

#ifdef __cplusplus
}
#endif

#endif

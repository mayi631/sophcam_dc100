#ifndef __PS_PARAM_H__
#define __PS_PARAM_H__

#include "player_service.h"

typedef PLAYER_SERVICE_PARAM_S* PS_PARAM_HANDLE;

int32_t ps_load_default_params(PS_PARAM_HANDLE param);

#define PS_SUCCESS             0
#define PS_FAILURE             (-1)
#define PS_TRUE                1
#define PS_FALSE               0

#endif // __PS_PARAM_H__

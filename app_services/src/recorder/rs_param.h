#ifndef __CS_PARAM_H__
#define __CS_PARAM_H__

#include "rs_define.h"
#include "record_service.h"

typedef RECORD_SERVICE_PARAM_S rs_param_t, *rs_param_handle_t;

int32_t rs_param_load_default(rs_param_handle_t param);

#endif  // __CS_PARAM_H__

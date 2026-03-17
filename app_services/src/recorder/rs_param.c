#include "rs_param.h"

#include <stdio.h>

#include "cvi_log.h"

int32_t rs_param_load_default(rs_param_handle_t p) {
    (void)p;

    CVI_LOGE("Standalone mode only, please fill here\n");
    return RS_ERR_FAILURE;
}

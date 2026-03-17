#include "ps_param.h"

int32_t ps_load_default_params(PS_PARAM_HANDLE parm)
{
    parm->disp_rotate = ROTATION_90;
    parm->disp_fmt = PIXEL_FORMAT_NV21;
    parm->disp_aspect_ratio = ASPECT_RATIO_AUTO;

    return PS_SUCCESS;
}

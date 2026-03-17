#ifndef __RS_EVENT_H__
#define __RS_EVENT_H__

#include "rs_context.h"
#include "rs_define.h"
#include "rs_param.h"
#include "osal.h"

int32_t rs_start_event_task(rs_context_handle_t rs);
int32_t rs_stop_event_task(rs_context_handle_t rs);

#endif // __RS_EVENT_H__

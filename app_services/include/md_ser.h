#ifndef __MD_SER_H__
#define __MD_SER_H__

#include <unistd.h>

#include "mapi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _MD_ATTR_S {
    int32_t camid;
    int32_t state;
    int32_t threshold;
    MAPI_VPROC_HANDLE_T vprocHandle;
    uint32_t vprocChnId;
    uint32_t isExtVproc;
    uint32_t w;
    uint32_t h;
    void *pfnCb;
} MD_ATTR_S;

typedef enum MD_EVENT_E {
    MD_EVENT_CHANGE,
    MD_EVENT_BUTT
} MD_EVENT_E;

void MD_SetState(int32_t id, int32_t en);
int32_t MD_Init(MD_ATTR_S *attr);
void MD_DeInit(int32_t id);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __VIDEOMD_SER_H__ */

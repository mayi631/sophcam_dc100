#ifndef __PHOTOMNG_H__
#define __PHOTOMNG_H__

#include "appcomm.h"
#include "photo_service.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum EVENT_PHOTOMNG_E {
    EVENT_PHOTOMNG_OPEN_FAILED = APPCOMM_EVENT_ID(APP_MOD_PHOTOMNG, 0),
    EVENT_PHOTOMNG_PIV_START,
    EVENT_PHOTOMNG_PIV_END,
    EVENT_PHOTOMNG_PIV_ERROR,
    EVENT_PHOTOMNG_BUTT
} EVENT_PHOTOMNG_E;

int32_t PHOTOMNG_ContCallBack(PHOTO_SERVICE_EVENT_E event_type, const char *filename, void *param);
int32_t POHTOMNG_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __RECORDMNG_H__ */
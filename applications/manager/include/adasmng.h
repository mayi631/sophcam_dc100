#ifndef __ADASMNG_H__
#define __ADASMNG_H__

#include <stdint.h>
#include "appcomm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define ADASMNG_EREGISTER_EVENT APP_APPCOMM_ERR_ID(APP_MOD_ADASMNG, APP_ERRNO_CUSTOM_BOTTOM)

/** event ID define */
typedef enum EVENT_ADASMNG_E {
    EVENT_ADASMNG_CAR_MOVING = APPCOMM_EVENT_ID(APP_MOD_ADASMNG, 0),
    EVENT_ADASMNG_CAR_CLOSING,
    EVENT_ADASMNG_CAR_COLLISION,
    EVENT_ADASMNG_CAR_LANE,
    EVENT_ADASMNG_LABEL_CAR,
    EVENT_ADASMNG_LABEL_LANE,
    EVENT_ADASMNG_BUTT
} EVENT_ADASMNG_E;

typedef enum _ADASMNG_CMD_E
{
    ADASMNG_NORMAL = 0,
    ADASMNG_CAR_MOVING,
    ADASMNG_CAR_CLOSING,
    ADASMNG_CAR_COLLISION,
    ADASMNG_CAR_LANE,
    ADASMNG_LABEL_CAR,
    ADASMNG_LABEL_LANE,
    ADASMNG_BUTT
} ADASMNG_CMD_E;

int32_t ADASMNG_RegisterEvent(void);
int32_t ADASMNG_VoiceCallback(int32_t index);
int32_t ADASMNG_LabelCallback(int32_t camid, int32_t index, uint32_t count, char* coordinates);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __ADASMNG_H__ */
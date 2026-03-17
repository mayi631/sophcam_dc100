#include <stdio.h>
#include <string.h>
#include "adasmng.h"
#include "appcomm.h"
#include "sysutils_eventhub.h"

int32_t ADASMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = EVENTHUB_RegisterTopic(EVENT_ADASMNG_CAR_MOVING);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_ADASMNG_CAR_CLOSING);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_ADASMNG_CAR_COLLISION);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_ADASMNG_CAR_LANE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_ADASMNG_LABEL_CAR);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_ADASMNG_LABEL_LANE);
    APPCOMM_CHECK_RETURN(s32Ret, ADASMNG_EREGISTER_EVENT);
    return s32Ret;
}

int32_t ADASMNG_VoiceCallback(int32_t index)
{
    EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(EVENT_S));
    switch (index)
    {
    case ADASMNG_CAR_MOVING:
        stEvent.topic = EVENT_ADASMNG_CAR_MOVING;
        break;
    case ADASMNG_CAR_CLOSING:
        stEvent.topic = EVENT_ADASMNG_CAR_CLOSING;
        break;
    case ADASMNG_CAR_COLLISION:
        stEvent.topic = EVENT_ADASMNG_CAR_COLLISION;
        break;
    case ADASMNG_CAR_LANE:
        stEvent.topic = EVENT_ADASMNG_CAR_LANE;
        break;
    default:
        break;
    }
    EVENTHUB_Publish(&stEvent);
    return 0;
}

int32_t ADASMNG_LabelCallback(int32_t camid, int32_t index, uint32_t count, char* coordinates)
{
    EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(EVENT_S));
    switch (index)
    {
    case ADASMNG_LABEL_CAR:
        stEvent.topic = EVENT_ADASMNG_LABEL_CAR;
        stEvent.arg1 = count;
        stEvent.arg2 = camid;
        memcpy(stEvent.aszPayload, coordinates, sizeof(stEvent.aszPayload));
        break;
    case ADASMNG_LABEL_LANE:
        stEvent.topic = EVENT_ADASMNG_LABEL_LANE;
        stEvent.arg1 = count;
        stEvent.arg2 = camid;
        memcpy(stEvent.aszPayload, coordinates, sizeof(stEvent.aszPayload));
        break;
    default:
        break;
    }
    EVENTHUB_Publish(&stEvent);
    return 0;
}
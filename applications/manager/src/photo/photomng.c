#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "cvi_log.h"
#include "sysutils_eventhub.h"
#include "photomng.h"

int32_t POHTOMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = EVENTHUB_RegisterTopic(EVENT_PHOTOMNG_OPEN_FAILED);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PHOTOMNG_PIV_START);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PHOTOMNG_PIV_END);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PHOTOMNG_PIV_ERROR);

    return s32Ret;
}

int32_t PHOTOMNG_ContCallBack(PHOTO_SERVICE_EVENT_E event_type, const char *filename, void *param)
{
    /* Publish Event */
    EVENT_S stEvent;
    memset(&stEvent, 0x0, sizeof(EVENT_S));
    switch (event_type) {
        case PHOTO_SERVICE_EVENT_OPEN_FILE_FAILED:
            stEvent.topic = EVENT_PHOTOMNG_OPEN_FAILED;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case PHOTO_SERVICE_EVENT_PIV_START:
            stEvent.topic = EVENT_PHOTOMNG_PIV_START;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case PHOTO_SERVICE_EVENT_PIV_END:
            stEvent.topic = EVENT_PHOTOMNG_PIV_END;
            stEvent.arg1 = *(int32_t *)param;
            snprintf((char *)stEvent.aszPayload, MSG_PAYLOAD_LEN, "%s", filename);
            break;
        case PHOTO_SERVICE_EVENT_PIV_ERROR:
            stEvent.topic = EVENT_PHOTOMNG_PIV_ERROR;
            stEvent.arg1 = *(int32_t *)param;
        break;
        case PHOTO_SERVICE_EVENT_SYNC_DONE:
            CVI_LOGD("RECORDER_EVENT_SYNC_DONE == %d param = %d %s\n",PHOTO_SERVICE_EVENT_SYNC_DONE, *(int32_t *)param, filename);
            break;
        default:
            break;
    }

    EVENTHUB_Publish(&stEvent);

    return 0;
}

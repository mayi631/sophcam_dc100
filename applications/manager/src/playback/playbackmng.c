#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "cvi_log.h"
#include "sysutils_eventhub.h"
#ifdef SERVICES_PLAYER_ON
#include "player_service_command.h"
#endif
#include "playbackmng.h"

void PLAYBACKMNG_EventCallBack(PLAYER_SERVICE_HANDLE_T hdl, PLAYER_SERVICE_EVENT_S *event_t)
{
    (void)hdl;

    if (event_t == NULL) {
        CVI_LOGE("event is null");
        return;
    }
    /* Publish Event */
    EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(stEvent));

    switch(event_t->type) {
    case PLAYER_SERVICE_EVENT_PLAY_FINISHED:
        stEvent.topic = EVENT_PLAYBACKMNG_FINISHED;
        CVI_LOGI("Player play finish");
        break;
    case PLAYER_SERVICE_EVENT_PLAY_PROGRESS:
        stEvent.topic = EVENT_PLAYBACKMNG_PROGRESS;
        break;
    case PLAYER_SERVICE_EVENT_PAUSE:
        stEvent.topic = EVENT_PLAYBACKMNG_PAUSE;
        break;
    case PLAYER_SERVICE_EVENT_RESUME:
        stEvent.topic = EVENT_PLAYBACKMNG_RESUME;
        break;
    case PLAYER_SERVICE_EVENT_OPEN_FAILED:
    case PLAYER_SERVICE_EVENT_RECOVER_FAILED:
        stEvent.topic = EVENT_PLAYBACKMNG_FILE_ABNORMAL;
        break;
    case PLAYER_SERVICE_EVENT_PLAY:
        CVI_LOGI("Player playing...");
        stEvent.topic = EVENT_PLAYBACKMNG_PLAY;
        break;
    default:
        break;
    }
    EVENTHUB_Publish(&stEvent);
}

int32_t PLAYBACKMNG_RegisterEvent(void)
{
    int32_t s32Ret = 0;
    s32Ret = EVENTHUB_RegisterTopic(EVENT_PLAYBACKMNG_FINISHED);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PLAYBACKMNG_PLAY);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PLAYBACKMNG_PROGRESS);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PLAYBACKMNG_PAUSE);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PLAYBACKMNG_RESUME);
    s32Ret |= EVENTHUB_RegisterTopic(EVENT_PLAYBACKMNG_FILE_ABNORMAL);
    APPCOMM_CHECK_RETURN(s32Ret, PLAYBACKMNG_EREGISTER_EVENT);

    return 0;
}

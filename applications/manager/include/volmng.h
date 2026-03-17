#ifndef __VOLMNG_H__
#define __VOLMNG_H__

#include "appcomm.h"
#include "mapi_ao.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** macro define */
#define VOLMNG_EINVAL            APP_APPCOMM_ERR_ID(APP_MOD_VOLMNG, APP_EINVAL)               /**<parm error*/
#define VOLMNG_EINTER            APP_APPCOMM_ERR_ID(APP_MOD_VOLMNG, APP_EINTER)               /**<intern error*/
#define VOLMNG_ENOINIT           APP_APPCOMM_ERR_ID(APP_MOD_VOLMNG, APP_ENOINIT)              /**< no initialize*/
#define VOLMNG_EINITIALIZED      APP_APPCOMM_ERR_ID(APP_MOD_VOLMNG, APP_EINITIALIZED)         /**< already initialized */
#define VOLMNG_EREGISTEREVENT    APP_APPCOMM_ERR_ID(APP_MOD_VOLMNG, APP_ERRNO_CUSTOM_BOTTOM)  /**<thread creat or join error*/
#define VOLMNG_ETHREAD           APP_APPCOMM_ERR_ID(APP_MOD_VOLMNG, APP_ERRNO_CUSTOM_BOTTOM+1)/**<thread creat or join error*/


/** Path Maximum Length */
#define VOICE_MAX_SEGMENT_CNT (5)
#define VOLUME_DEFAULT 6

typedef struct _VOICEPLAYER_AOUT_OPT_S {
    void * hAudDevHdl;    // device id
    void * hAudTrackHdl;  // chn id
} VOICEPLAY_AOUT_OPT_S;

typedef struct _VOICEPLAY_VOICETABLE_S
{
    uint32_t u32VoiceIdx;
    char aszFilePath[APPCOMM_MAX_PATH_LEN];
} VOICEPLAY_VOICETABLE_S;

/** voiceplay configuration */
typedef struct __VOICEPLAY_CFG_S
{
    uint32_t u32MaxVoiceCnt;
    VOICEPLAY_VOICETABLE_S* pstVoiceTab;
    VOICEPLAY_AOUT_OPT_S stAoutOpt;
} VOICEPLAY_CFG_S;

typedef struct _VOICEPLAY_VOICE_S
{
    uint32_t volume;
    uint32_t u32VoiceCnt;
    uint32_t au32VoiceIdx[VOICE_MAX_SEGMENT_CNT];
    bool bDroppable;
} VOICEPLAY_VOICE_S;


int32_t VOICEPLAY_Init(const VOICEPLAY_CFG_S* pstCfg);                              /*the api will call in the media arrangment*/
int32_t VOICEPLAY_DeInit(void);
int32_t VOICEPLAY_Push(const VOICEPLAY_VOICE_S* pstVoice, int32_t u32Timeout_ms);   /*the api will push in queue */
int32_t VOICEPLAY_SetAmplifier(bool en);
int32_t VOICEPLAY_SetAmplifierFlage(bool en);
int32_t VOICEPLAY_SetVolume(int32_t volume);
int32_t VOICEPLAY_GetVolume(int32_t* volume);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __PLAYBACKMNG_H__ */

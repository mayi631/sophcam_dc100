#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include "mlog.h"
#include "media_init.h"
#include "mapi_ao.h"
#include "text_to_speech/text_to_speech.h"
#include "ttp.h"
#include "ui_common.h"

/* 文字转语音播放线程 */
static pthread_t g_player_thread = 0;
/* 播放线程运行标志 */
static int32_t g_player_runnnig = 0;
/* 播放信号量 */
static sem_t g_player_sem;
/* 播放复位 */
static int32_t g_player_reset = 0;
/* 文字转语音线程 */
static pthread_t g_tts_thread = 0;
/* 文字转语音线程运行标志 */
static int32_t g_tts_runnnig = 0;
/* 文字转语音信号量 */
static sem_t g_tts_sem;
/* 文字转语音复位 */
static int32_t g_tts_reset = 0;
/* 播放的文本 */
static char g_tts_text[4096] = { 0 };
/* 当前音色名称 */
static char g_voice_name[32] = { 0 };
/* 不同阶段的状态 */
static ttp_status_s g_ttp_status = { 0 };

/* 创建播放线程 */
static void create_player(void);
/* 销毁播放线程 */
static void destory_player(void);
/* player线程 */
static void *thread_player_main(void *arg);
/* 创建tts线程 */
static void create_tts(void);
/* 销毁tts线程 */
static void destory_tts(void);
/* tts线程 */
static void *thread_tts_main(void *arg);

static void create_player(void) {
    if(g_player_thread == 0){
        MLOG_INFO("create player\n");
        pthread_create(&g_player_thread, NULL, thread_player_main, NULL);
    } else {
        MLOG_ERR("player has created\n");
    }
}

static void destory_player(void) {
    if(g_player_runnnig) {
        g_player_runnnig = 0;
        sem_post(&g_player_sem);
        pthread_join(g_player_thread, NULL);
        g_player_thread = 0;
    }
}

static void *thread_player_main(void *arg)
{
    int ret = 0;
    AUDIO_FRAME_S stFrame = {0};
    MAPI_AO_HANDLE_T AoHdl = MEDIA_GetCtx()->SysHandle.aohdl;
    tts_audio_buffer_t audio_buffer = {0};
    uint32_t is_finished = 0;
    tts_error_t tts_ret = TTS_SUCCESS;
    uint32_t audio_size = 0;

    sem_init(&g_player_sem, 0, 0);
    g_player_runnnig = 1;

    while(g_player_runnnig) {
        MLOG_INFO("wait play...\n");
        sem_wait(&g_player_sem);
        g_player_reset = 0;

        if(!g_player_runnnig) {
            break;
        }

        ui_common_mute_btn_voice();

        ret = MAPI_AO_Unmute(AoHdl);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_AO_Unmute failed\n");
        }

        ret = MAPI_AO_SetAmplifier(AoHdl, true);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_AO_SetAmplifier failed\n");
        }

        ret = MAPI_AO_Start(AoHdl, 0);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_AO_Start failed\n");
        }

        /* 等待buffer充足, 不然会出现断音 */
        while(g_player_runnnig) {
            if(g_player_reset) {
                MLOG_INFO("player reset\n");
                break;
            }

            tts_get_audio_size(&audio_size);
            if(audio_size < 2) {
                MLOG_INFO("wait size: %d...\n", audio_size);
                usleep(200 * 1000);
                continue;
            } else {
                MLOG_INFO("start play: %d...\n", audio_size);
                break;
            }
        }

        g_ttp_status.player_status = TTP_STATUS_PLAYER_RUNNING;

        while (g_player_runnnig) {
            if (g_player_reset) {
                MLOG_INFO("player reset\n");
                break;
            }

            memset(&audio_buffer, 0, sizeof(audio_buffer));
            /* 获取音频 */
            tts_ret = tts_get_audio_stream(&audio_buffer);
            if(tts_ret == TTS_ERROR_EMPTY) {
                tts_check_audio_stream_finished(&is_finished);
                if(is_finished) {
                    MLOG_INFO("playe finished\n");
                    break;
                }
                MLOG_INFO("sleep...\n");
                usleep(10 * 1000);
                continue;
            }
            MLOG_INFO("audio size:%d\n", audio_buffer.size);

            /* 播放 */
            stFrame.u32Len       = audio_buffer.size / 2;
            stFrame.u64TimeStamp = 0;
            stFrame.enSoundmode  = AUDIO_SOUND_MODE_MONO;
            stFrame.enBitwidth   = AUDIO_BIT_WIDTH_16;
            stFrame.u64VirAddr[0] = audio_buffer.data;
            ret = MAPI_AO_SendFrame(AoHdl, 0, &stFrame, 1000);
            if(ret) {
                MLOG_ERR("MAPI_AO_SendFrame failed!\n");
            }

            tts_free_audio_buffer(&audio_buffer);
        }

        MAPI_AO_Stop(AoHdl, 0);
        ui_common_unmute_btn_voice();
        g_ttp_status.player_status = TTP_STATUS_PLAYER_IDLE;
    }

    sem_destroy(&g_player_sem);
    g_player_thread = 0;
    MLOG_INFO("player exit\n");
    return NULL;
}

static void create_tts(void) {
    if(g_tts_thread == 0){
        MLOG_INFO("create tts\n");
        pthread_create(&g_tts_thread, NULL, thread_tts_main, NULL);
    } else {
        MLOG_ERR("tts has created\n");
    }
}

static void destory_tts(void) {
    if(g_tts_runnnig) {
        g_tts_runnnig = 0;
        sem_post(&g_tts_sem);
        pthread_join(g_tts_thread, NULL);
        g_tts_thread = 0;
    }
}

static void *thread_tts_main(void *arg)
{
    sem_init(&g_tts_sem, 0, 0);
    g_tts_runnnig = 1;

    while(g_tts_runnnig) {
        MLOG_INFO("wait tts...\n");
        sem_wait(&g_tts_sem);
        g_tts_reset = 0;

        if(!g_tts_runnnig) {
            break;
        }

        g_ttp_status.tts_status = TTP_STATUS_TTS_RUNNING;

        /* Set up configuration */
        tts_config_t tts_config;
        memset(&tts_config, 0, sizeof(tts_config));
        tts_config.project_id = "6i9jCXvF6kwjShzsslV4N7";
        tts_config.api_key = "MEJvhg4R9uQ-w3Cz7WCORUU_z1FCcs37uIZr3LMzIftXI-ICyoywpO_xY_6y2E2BRQZm7xUk2ayRqluu6CKRjw";
        tts_config.easyllm_id = "7W3iwdmNhGs1ktPicpJPls";
        tts_config.base_url = "https://www.sophnet.com/api/open-apis";
        tts_config.timeout_ms = 30000;

        /* Set up synthesis parameters */
        tts_synthesis_param_t tts_param;
        memset(&tts_param, 0, sizeof(tts_param));
        tts_param.model = "cosyvoice-v1";
        tts_param.voice = strlen(g_voice_name) > 0 ? g_voice_name : "longxiaochun";
        tts_param.format = TTS_FORMAT_PCM_16000HZ_MONO_16BIT;
        tts_param.volume = 80;
        tts_param.speech_rate = 1.0f;
        tts_param.pitch_rate = 1.0f;

        /* 阻塞等待tts完成 */
        tts_error_t result = tts_synthesize_stream(&tts_config, g_tts_text, &tts_param);

        g_ttp_status.tts_status = TTP_STATUS_TTS_IDLE;

        /* Check for errors */
        if (result != TTS_SUCCESS) {
            MLOG_ERR("Synthesis failed with code: %d\n", result);
        } else {
            MLOG_INFO("Synthesis finished!.\n");
        }
    }

    MLOG_INFO("tts exit\n");
    return NULL;
}

int32_t ttp_init(void) {
    create_player();
    create_tts();
    return 0;
}

int32_t ttp_deinit(void) {
    destory_player();
    destory_tts();
    return 0;
}

int32_t ttp_play(const char* text) {
    MLOG_INFO("start ttp\n");

    memset(g_tts_text, 0, sizeof(g_tts_text));
    strncpy(g_tts_text, text, sizeof(g_tts_text) - 1);

    /* 通知player */
    sem_post(&g_player_sem);

    /* 通知tts */
    sem_post(&g_tts_sem);

    return 0;
}

int32_t ttp_reset(void) {
    MLOG_INFO("ttp reset\n");
    g_player_reset = 1;
    return 0;
}

int32_t ttp_reset_sync(uint32_t s)
{
    uint32_t timeout_s = 0;
    MLOG_INFO("ttp reset\n");
    g_player_reset = 1;
    while (timeout_s < s && g_player_reset == 1) {
        sleep(s);
        timeout_s++;
        MLOG_WARN("wait reset:%d %d %d\n", timeout_s, s, g_player_reset);
    }
    if (timeout_s >= s) {
        return -1;
    } else {
        return 0;
    }
}

int32_t ttp_set_voice(const char* voice)
{
    if (voice == NULL) {
        MLOG_ERR("voice is NULL");
        return -1;
    }
    memset(g_voice_name, 0, sizeof(g_voice_name));
    strncpy(g_voice_name, voice, sizeof(g_voice_name) - 1);
    return 0;
}

int32_t ttp_get_status(ttp_status_s* status)
{
    if (status) {
        *status = g_ttp_status;
    } else {
        MLOG_ERR("status is NULL");
        return -1;
    }
    return 0;
}

/*
 * 进入 AI 对话页面创建线程，退出页面，销毁线程
 * 1. 获取音频数据，然后喂给算法的线程。
 * 2. 获取AI音频回复并播放。
 *
 * 使用状态接口，来控制线程的运行。
 */
#define DEBUG
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include "mlog.h"
#include "dialogxx.h"
#include "media_init.h"
#include "mapi_acap.h"
#include "mapi_ao.h"
#include "ui_common.h"

// todo: 使用原子变量
/* 聊天状态 */
static uint32_t chat_state = CHAT_IDLE;

/* 聊天参数 */
static voice_chat_params_t params;
static pthread_mutex_t param_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 聊天handler */
static voice_chat_processor_t *processor = NULL;
static pthread_mutex_t processor_mutex   = PTHREAD_MUTEX_INITIALIZER;

/* 语音的文字 */
static char g_voice_asr_text[2048] = {0};
static int32_t g_asr_text_prepare_ok = 0;

/* asr后处理函数 */
static AsrFunc g_asr_post_callback = NULL;
static ChatErrFunc g_chat_err_callback = NULL;

/* 录音和播放信号量 */
static sem_t g_recoder_sem;
static sem_t g_player_sem;

/* 录音和播放线程运行标志 */
static int32_t g_recoder_runnnig = 0;
static int32_t g_player_runnnig = 0;

/* 录音时间 */
static uint32_t g_recorder_time_ms = 0;

/* 录音和播放线程 */
static pthread_t g_recoder_thread = 0;
static pthread_t g_player_thread    = 0;

/* 休眠唤醒, 息屏场景 */
static uint32_t g_is_suspend = 0;

/* 被打断, 复位 */
static uint32_t g_is_recoder_reset = 0;
static uint32_t g_is_player_reset = 0;

/* 当前音色名称 */
static char g_voice_name[32] = {0};

/* 提供接口给 UI，更改状态 */
void chat_set_state(uint32_t state)
{
    char state_str[32];

    /* 状态可以向后转换; 任意状态可以转换到IDLE; IDLE可以转换到CHAT_START_RECORD和CHAT_START_PLAY */
    if((chat_state != CHAT_IDLE && state != CHAT_IDLE && state != chat_state + 1) ||
        (chat_state == CHAT_IDLE && (state != CHAT_START_RECORD && state != CHAT_START_PLAY))) {
        MLOG_ERR("Invalid state transition from %d to %d\n", chat_state, state);
        return;
    }

    chat_state = state;
    switch(state) {
        case CHAT_START_RECORD: snprintf(state_str, sizeof(state_str), "START_RECORD"); break;
        case CHAT_RECORDING: snprintf(state_str, sizeof(state_str), "RECORDING"); break;
        case CHAT_START_PLAY: snprintf(state_str, sizeof(state_str), "START_PLAY"); break;
        case CHAT_PLAYING: snprintf(state_str, sizeof(state_str), "PLAYING"); break;
        case CHAT_IDLE: snprintf(state_str, sizeof(state_str), "IDLE"); break;
        case CHAT_ASR: snprintf(state_str, sizeof(state_str), "ASR"); break;
        default: snprintf(state_str, sizeof(state_str), "UNKNOWN"); break;
    }
    MLOG_INFO("chat state set to %s\n", state_str);
}

void chat_get_state(uint32_t *state)
{
    *state = chat_state;
}

static void param_init()
{
    pthread_mutex_lock(&param_mutex);
    memset(&params, 0, sizeof(voice_chat_params_t));
    params.project_id     = "6i9jCXvF6kwjShzsslV4N7";
    params.api_key        = "MEJvhg4R9uQ-w3Cz7WCORUU_z1FCcs37uIZr3LMzIftXI-ICyoywpO_xY_6y2E2BRQZm7xUk2ayRqluu6CKRjw";
    params.asr_easyllm_id = "4Q0eQg6Jy38wDIRB5P5d6z";
    params.tts_easyllm_id = "7W3iwdmNhGs1ktPicpJPls";

    params.asr_params.format      = "pcm";
    params.asr_params.sample_rate = AUDIO_SAMPLE_RATE_16000;
    params.llm_params.model_name  = "DeepSeek-V3-Fast";
    params.llm_params.prompt      = "回答不要超过200字。";
    params.tts_params.voice       = strlen(g_voice_name) > 0 ? g_voice_name : "longxiaochun";
    params.tts_params.format      = "PCM_16000HZ_MONO_16BIT";
    pthread_mutex_unlock(&param_mutex);
}

/* 录音线程，当状态为 录音 */
static void *thread_recoder_main(void *arg)
{
    (void)arg;
    int ret;
    int readbytes       = 0;
    int total_readbytes = 0;
    /* 第一帧超时时间 */
    const int max_wait_time = 5;
    int wait_count = 0;
    /* 帧间超时时间 */
    const int continue_wait_time = 2;
    int continue_wait_count = 0;
    /* 第一帧标识 */
    int first_text_flag = 1;
    int asr_text_offset = 0;
    /* 录音时间测量 */
    struct timespec rec_t0, rec_t1;

    MAPI_ACAP_HANDLE_T AcapHdl;
    MAPI_ACAP_ATTR_S stACapAttr;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S stAecFrm;

    param_init();
    processor = NULL;
    ret       = voice_chat_validate_params(&params);
    MLOG_INFO("voice_chat_validate_params ret=%d\n", ret);
    ret = voice_chat_create(&processor, &params);
    MLOG_INFO("voice_chat_create ret=%d\n", ret);
    if(ret) {
        MLOG_ERR("Failed to create processor\n");
        /* UI给出提示 */
        if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_NETWORK_ERROR);
        goto ERR_PROCESSER;
    }

    stACapAttr.channel        = 2;
    stACapAttr.enSampleRate   = AUDIO_SAMPLE_RATE_16000;
    stACapAttr.u32PtNumPerFrm = 640;
    stACapAttr.bVqeOn         = 1;
    stACapAttr.volume         = 24;
    ret = MAPI_ACAP_Init(&AcapHdl, &stACapAttr);
    if(ret != MAPI_SUCCESS) {
        MLOG_ERR("MAPI_ACAP_Init failed\n");
        if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_INTERNAL_ERROR);
        goto ERR_ACAP_INIT;
    }

    ret = MAPI_ACAP_Start(AcapHdl);
    if(ret != MAPI_SUCCESS) {
        MLOG_ERR("MAPI_ACAP_Start failed\n");
        if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_INTERNAL_ERROR);
        goto ERR_ACAP_START;
    }

    sem_init(&g_recoder_sem ,0 ,0);
    g_recoder_runnnig = 1;

    while(g_recoder_runnnig) {
        MLOG_INFO("wait record...\n");
        sem_wait(&g_recoder_sem);
        g_is_recoder_reset = 0;

        if(!g_recoder_runnnig) {
            break;
        }

        if(g_is_suspend) {
            MLOG_INFO("on suspend\n");
            continue;
        }

        /* 新的对话 */
        first_text_flag = 1;
        asr_text_offset = 0;
        g_asr_text_prepare_ok = 0;
        wait_count = 0;
        continue_wait_count = 0;
        total_readbytes = 0;
        g_recorder_time_ms = 0;

        if(chat_state == CHAT_START_RECORD) {
            pthread_mutex_lock(&processor_mutex);

            /* 按下立马松开 */
            if(chat_state == CHAT_IDLE){
                /* 录音时间很短, 还没开始就结束了 */
                MLOG_WARN("too short\n");
                /* 通知用户更新UI */
                if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_BUFFER_TOO_SMALL);
                pthread_mutex_unlock(&processor_mutex);
                continue;
            }else{
                /* 处理完之后，就进入 recording 状态 */
                chat_set_state(CHAT_RECORDING);
            }

            /* 打开文件用于保存录音数据 */
            FILE *pcm_file = fopen("input.pcm", "wb");
            if(pcm_file == NULL) {
                MLOG_ERR("Failed to open input.pcm for writing\n");
                chat_set_state(CHAT_IDLE);
                pthread_mutex_unlock(&processor_mutex);
                continue;
            }

            /* 开始录音 */
            clock_gettime(CLOCK_MONOTONIC, &rec_t0);
            while(chat_state == CHAT_RECORDING) {
                /* 获取音频数据 */
                ret = MAPI_ACAP_GetFrame(AcapHdl, &stFrame, &stAecFrm);
                if(ret != MAPI_SUCCESS) {
                    MLOG_ERR("MAPI_ACAP_GetFrame failed, ret:%#x.!!\n", ret);
                    usleep(1000 * (1000 * stACapAttr.u32PtNumPerFrm / stACapAttr.enSampleRate) / 2);
                    continue;
                }
                readbytes = stFrame.u32Len * 1 * 2; // 采样点数*通道数*每个采样点字节数
                total_readbytes += readbytes;

                /* 保存录音数据到文件 */
                fwrite((void *)stFrame.u64VirAddr[0], 1, readbytes, pcm_file);
                MLOG_DBG("录音数据: %d 字节\n", readbytes);

                /* 发送数据到云端 */
                voice_chat_send_frame(processor, stFrame.u64VirAddr[0], readbytes);

                /* 释放音频数据 */
                ret = MAPI_ACAP_ReleaseFrame(AcapHdl, &stFrame, &stAecFrm);
                if(ret != CVI_SUCCESS) {
                    MLOG_ERR("MAPI_ACAP_ReleaseFrame, failed with %d!\n", ret);
                }
            }
            /* 录音结束 */
            clock_gettime(CLOCK_MONOTONIC, &rec_t1);
            g_recorder_time_ms = (rec_t1.tv_sec - rec_t0.tv_sec) * 1000 + (rec_t1.tv_nsec - rec_t0.tv_nsec)/1000000;

            /* 关闭文件 */
            fclose(pcm_file);
            MLOG_INFO("recoder data(%d bytes) saved to input.pcm\n", total_readbytes);

            /* 将数据提交给云端asr */
            ret = voice_chat_send_finish(processor);
            MLOG_DBG("send finished: %s\n", voice_chat_get_error_string(ret));
            if(ret != VOICE_CHAT_SUCCESS) {
                if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_NETWORK_ERROR);
                break;
            }

            /* 打断 */
            if(g_is_recoder_reset) {
                MLOG_WARN("interupt and reset\n");
                chat_set_state(CHAT_IDLE);
                pthread_mutex_unlock(&processor_mutex);
                continue;
            }

            /* 录音时间太短不处理 */
            MLOG_INFO("recoder time: %dms\n", g_recorder_time_ms);
            if(g_recorder_time_ms < 1000){
                MLOG_WARN("too short\n");
                chat_set_state(CHAT_IDLE);
                /* 通知用户更新UI */
                if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_BUFFER_TOO_SMALL);
                pthread_mutex_unlock(&processor_mutex);
                continue;
            }

            memset(g_voice_asr_text, 0, sizeof(g_voice_asr_text));

            /* 进入ASR，等待ASR完成 */
            chat_set_state(CHAT_ASR);
            /* 一次性获取完所有的asr text */
            while (wait_count < max_wait_time && chat_state == CHAT_ASR) {
                int ret = 0;
                size_t text_size = sizeof(g_voice_asr_text) - asr_text_offset;
                if(text_size <= 0){
                    MLOG_ERR("no buf\n");
                    break;
                }
                ret = voice_chat_get_asr_text(processor, g_voice_asr_text + asr_text_offset, &text_size);
                if (ret == VOICE_CHAT_SUCCESS) {
                    MLOG_INFO("asr text(size:%u): %s\n", text_size, g_voice_asr_text);
                    continue_wait_count = 0;
                    first_text_flag = 0;
                    asr_text_offset += text_size;
                } else if (ret == VOICE_CHAT_BUFFER_EMPTY) {
                    if(first_text_flag) {
                        ++wait_count;
                        MLOG_INFO("wait first asr: %d\n", wait_count);
                    }else {
                        ++continue_wait_count;
                        MLOG_INFO("wait asr: %d\n", continue_wait_count);
                    }
                    usleep(1000*1000);
                } else if(ret == VOICE_CHAT_NOT_CONNECTED || ret == VOICE_CHAT_NETWORK_ERROR) {
                    MLOG_ERR("%s\n", voice_chat_get_error_string(ret));
                    if(g_chat_err_callback) g_chat_err_callback(ret);
                    break;
                } else {
                    MLOG_ERR("asr ret: %d\n", ret);
                }
                /* 连续等待continue_wait_count后没有收到数据，说明已经接收完成了 */
                if(continue_wait_count >= continue_wait_time) {
                    MLOG_INFO("asr text finished\n");
                    g_asr_text_prepare_ok = 1;
                    if(g_asr_post_callback != NULL){
                        g_asr_post_callback();
                    }
                    break;
                }
                /* 打断 */
                if(g_is_recoder_reset) {
                    MLOG_WARN("interupt and reset\n");
                    chat_set_state(CHAT_IDLE);
                    pthread_mutex_unlock(&processor_mutex);
                    continue;
                }
            }

            /* 第一帧超时: 到这里说明不是网络连接的原因 */
            if(wait_count >= max_wait_time) {
                MLOG_ERR("wait first asr timeout\n");
                chat_set_state(CHAT_IDLE);
                if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_TIMEOUT);
            }

            pthread_mutex_unlock(&processor_mutex);
        }
    }

    sem_destroy(&g_recoder_sem);
    MAPI_ACAP_Stop(AcapHdl);
ERR_ACAP_START:
    MAPI_ACAP_Deinit(AcapHdl);
ERR_ACAP_INIT:
ERR_PROCESSER:
    chat_set_state(CHAT_IDLE);
    g_recoder_thread = 0;
    g_asr_post_callback = NULL;
    MLOG_INFO("recoder exit\n");
    return NULL;
}

static void *thread_player_main(void *arg)
{
    int ret = 0;
    AUDIO_FRAME_S stFrame = {0};
    size_t audio_size     = 8192;
    MAPI_AO_HANDLE_T AoHdl = MEDIA_GetCtx()->SysHandle.aohdl;

    stFrame.u64VirAddr[0] = malloc(8192);
    if(stFrame.u64VirAddr[0] == NULL) {
        MLOG_ERR("malloc failed\n");
        if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_INTERNAL_ERROR);
        goto ERR_MALLOC;
    }

    sem_init(&g_player_sem, 0, 0);
    g_player_runnnig = 1;

    while(g_player_runnnig) {
        MLOG_INFO("wait play...\n");
        sem_wait(&g_player_sem);
        g_is_player_reset = 0;

        if(!g_player_runnnig) {
            break;
        }

        if(g_is_suspend) {
            MLOG_INFO("on suspend\n");
            continue;
        }

        ui_common_mute_btn_voice();

        ret = MAPI_AO_Unmute(AoHdl);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_AO_Unmute failed\n");
            if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_INTERNAL_ERROR);
        }

        ret = MAPI_AO_SetAmplifier(AoHdl, true);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_AO_SetAmplifier failed\n");
            if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_INTERNAL_ERROR);
        }

        ret = MAPI_AO_Start(AoHdl, 0);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_AO_Start failed\n");
            if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_INTERNAL_ERROR);
        }

        if(chat_state == CHAT_START_PLAY) {
            pthread_mutex_lock(&processor_mutex);

            chat_set_state(CHAT_PLAYING);
            while(chat_state == CHAT_PLAYING) {
                audio_size = 8192;
                memset(stFrame.u64VirAddr[0], 0, audio_size);
                ret = voice_chat_get_audio(processor, stFrame.u64VirAddr[0], &audio_size);
                if(ret == VOICE_CHAT_SUCCESS && audio_size > 0) {
                    // fwrite(stFrame.u64VirAddr[0], 1, audio_size, output_file);
                    MLOG_DBG("收到音频数据: %zu 字节\n", audio_size);
                } else if(ret == VOICE_CHAT_BUFFER_TOO_SMALL) {
                    MLOG_WARN("too small\n");
                    usleep(10000); // 10ms
                    continue;
                } else if(ret == VOICE_CHAT_CHAT_COMPLETED) {
                    MLOG_DBG("对话完成\n");
                    break;
                } else {
                    MLOG_ERR("获取音频失败: %s\n", voice_chat_get_error_string(ret));
                    break;
                }

                stFrame.u32Len       = audio_size / 2;
                stFrame.u64TimeStamp = 0;
                stFrame.enSoundmode  = AUDIO_SOUND_MODE_MONO;
                stFrame.enBitwidth   = AUDIO_BIT_WIDTH_16;

                ret = MAPI_AO_SendFrame(AoHdl, 0, &stFrame, 1000);
                if(ret) {
                    MLOG_ERR("MAPI_AO_SendFrame failed!\n");
                    break;
                }

                /* 打断 */
                if(g_is_player_reset) {
                    MLOG_WARN("interupt and reset\n");
                    break;
                }
            }
            /* 在打断模式下,  状态复位会在打断的时候做，这里做会影响打断后触发的recoder状态 */
            if(!g_is_player_reset) {
                chat_set_state(CHAT_IDLE);
            }else {
                /* 取消llm tts, 清空旧对话的音频数据 */
                voice_chat_cancel(processor);
            }
            pthread_mutex_unlock(&processor_mutex);
        }

        MAPI_AO_Stop(AoHdl, 0);
        ui_common_unmute_btn_voice();
    }

    if(stFrame.u64VirAddr[0] != NULL){
        free(stFrame.u64VirAddr[0]);
        stFrame.u64VirAddr[0] = NULL;
    }

    sem_destroy(&g_player_sem);
ERR_MALLOC:
    if(processor != NULL) {
        voice_chat_destroy(processor);
        processor = NULL;
    }
    g_player_thread = 0;
    g_chat_err_callback = NULL;
    MLOG_INFO("player exit\n");
    return NULL;
}

char *chat_get_asr_text(void) {
    if(g_asr_text_prepare_ok) {
        g_asr_text_prepare_ok = 0;
        MLOG_INFO("asr text ready\n");
        return g_voice_asr_text;
    } else {
        MLOG_WARN("asr text don't ready\n");
        return NULL;
    }
    return NULL;
}

int32_t chat_get_llm_text(void* data, uint32_t* len) {
    if (processor != NULL) {
        return voice_chat_get_llm_text(processor, data, len);
    }else {
        return VOICE_CHAT_INTERNAL_ERROR;
    }
}

void chat_register_asr_post_callback(AsrFunc func) {
    g_asr_post_callback = func;
}

void chat_register_asr_err_process_callback(ChatErrFunc func) {
    g_chat_err_callback = func;
}

void chat_create_recorder(void) {
    if(g_recoder_thread == 0){
        MLOG_INFO("create recoder\n");
        pthread_create(&g_recoder_thread, NULL, thread_recoder_main, NULL);
    } else {
        MLOG_ERR("recoder has created\n");
    }
}

void chat_start_recorder(void) {
    if(g_recoder_runnnig) {
        chat_set_state(CHAT_START_RECORD);
        sem_post(&g_recoder_sem);
    }
}

void chat_stop_recorder(void) {
    if(g_recoder_runnnig) {
        chat_set_state(CHAT_ASR);
    }
}

void chat_destory_recorder(void) {
    if(g_recoder_runnnig) {
        g_recoder_runnnig = 0;
        chat_set_state(CHAT_IDLE);
        sem_post(&g_recoder_sem);
        pthread_join(g_recoder_thread, NULL);
        g_recoder_thread = 0;
    }
}

void chat_create_player(void) {
    if(g_player_thread == 0){
        MLOG_INFO("create player\n");
        pthread_create(&g_player_thread, NULL, thread_player_main, NULL);
    } else {
        MLOG_ERR("player has created\n");
    }
}

void chat_start_player(void) {
    if(g_player_runnnig) {
        chat_set_state(CHAT_START_PLAY);
        sem_post(&g_player_sem);
    }
}

void chat_stop_player(void) {
    chat_set_state(CHAT_IDLE);
}

void chat_destory_player(void) {
    if(g_player_runnnig) {
        g_player_runnnig = 0;
        chat_set_state(CHAT_IDLE);
        sem_post(&g_player_sem);
        pthread_join(g_player_thread, NULL);
        g_player_thread = 0;
    }
}

void chat_get_recorder_time(uint32_t* time_ms) {
    *time_ms = g_recorder_time_ms;
}

void chat_suspend(void) {
    if(g_recoder_runnnig) {
        g_is_suspend = 1;
        /* 断开网络连接 */
        if(processor != NULL) {
            voice_chat_destroy(processor);
            processor = NULL;
        }
    }
}

void chat_resume(void) {
    int ret = 0;
    if(g_recoder_runnnig) {
        g_is_suspend = 0;
        /* 参数使用休眠前的 */
        // param_init();
        processor = NULL;
        ret  = voice_chat_validate_params(&params);
        MLOG_INFO("voice_chat_validate_params ret=%d\n", ret);
        ret = voice_chat_create(&processor, &params);
        MLOG_INFO("voice_chat_create ret=%d\n", ret);
        if(ret) {
            MLOG_ERR("Failed to create processor\n");
            chat_set_state(CHAT_IDLE);
            if(g_chat_err_callback) g_chat_err_callback(VOICE_CHAT_NETWORK_ERROR);
        }
    }
}

void chat_set_reset(void) {
    MLOG_INFO("chat_set_reset\n");
    chat_set_state(CHAT_IDLE);
    g_is_recoder_reset = 1;
    g_is_player_reset = 1;
}

int chat_get_reset(void) {
    return g_is_recoder_reset & g_is_player_reset;
}

/* 给 UI 提供接口设置音色参数 */
void chat_set_voice_param(const char *voice)
{
    int ret = 0;
    if(voice == NULL) {
        MLOG_ERR("voice is NULL");
        return;
    }

    /* 参数变化, 打断 */
    chat_set_reset();

    memset(g_voice_name, 0, sizeof(g_voice_name));
    strncpy(g_voice_name, voice, sizeof(g_voice_name));

    pthread_mutex_lock(&param_mutex);
    params.tts_params.voice = voice;
    pthread_mutex_unlock(&param_mutex);

    /* 该事件发生后对话配置不允许再被更新，需要重新建立连接 */
    if(processor != NULL) {
        voice_chat_destroy(processor);
        processor = NULL;
    } else {
        /* 连接好之后的时候才需要重新连接来更新参数，否则就暂存参数就行 */
        MLOG_WARN("update param when connect\n");
        return;
    }

    ret  = voice_chat_validate_params(&params);
    MLOG_INFO("voice_chat_validate_params ret=%d\n", ret);
    ret = voice_chat_create(&processor, &params);
    MLOG_INFO("voice_chat_create ret=%d\n", ret);
    if(ret) {
        MLOG_ERR("Failed to create processor\n");
        if (g_chat_err_callback)
            g_chat_err_callback(VOICE_CHAT_NETWORK_ERROR);
    }
}

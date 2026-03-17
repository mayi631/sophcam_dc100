#define DEBUG
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include "mlog.h"
#include "mapi_acap.h"
#include "rtt.h"
#include "speech_to_text/speech_to_text.h"

/* asr参数 */
static speech_to_text_params_t g_stt_params = {0};
static pthread_mutex_t g_stt_params_mutex = PTHREAD_MUTEX_INITIALIZER;
/* asr handler */
static speech_to_text_processor_t *g_processor = NULL;
static pthread_mutex_t g_processor_mutex   = PTHREAD_MUTEX_INITIALIZER;
/* 录音时间 */
static uint32_t g_recorder_time_ms = 0;
/* 录音线程 */
static pthread_t g_recoder_thread = 0;
/* 处理线程 */
static pthread_t g_processor_thread = 0;
/* 被打断, 复位 */
static uint32_t g_is_recoder_reset = 0;
/* 存放asr结果 */
static uint32_t g_asr_text_offset = 0;
static char g_voice_asr_text[2048] = {0};
/* 正在录音 */
static uint32_t g_is_recoding = 0;
/* rtt 结束 */
static uint32_t g_is_finial = 0;
/* 录音信号量 */
static sem_t g_recoder_sem;
/* 录音线程运行标志 */
static int32_t g_recoder_runnnig = 0;
/* 创建process信号量  */
static sem_t g_processor_sem;
/* 创建process线程 */
static int32_t g_processor_runnnig = 0;
/* asr第一帧结果数据 */
static int32_t g_is_get_first_text = 0;
/* 网络错误 */
static int32_t g_is_network_error = 0;
/* 创建录音线程 */
static void create_recorder(void);
/* 销毁录音线程 */
static void destory_recorder(void);
/* 录音线程，当状态为 录音 */
static void *thread_recoder_main(void *arg);
/* 创建asr处理线程 */
static void create_processor(void);
/* 销毁asr处理线程 */
static void destory_processor(void);
/* asr线程 */
static void *thread_processor_main(void *arg);

/* queue local: TODO>单独成文件 */
typedef struct _queue_local_s {
    void* data;
    uint32_t len;
} queue_local_s;
#define QUEUE_MAX 128
static queue_local_s g_quque_ring[QUEUE_MAX] = {0};
static uint32_t g_quque_front_index = 0;
static uint32_t g_quque_tail_index = 0;
static uint32_t g_queue_size = 0;
static pthread_mutex_t g_queue_mutex = PTHREAD_MUTEX_INITIALIZER;

static int32_t queue_push(queue_local_s q) {
    if(q.data != NULL && q.len != 0) {
        if(g_queue_size < QUEUE_MAX) {
            MLOG_DBG("push: %d\n", g_quque_tail_index);
            pthread_mutex_lock(&g_queue_mutex);
            g_quque_ring[g_quque_tail_index].data = malloc(q.len);
            memcpy(g_quque_ring[g_quque_tail_index].data, q.data, q.len);
            g_quque_ring[g_quque_tail_index].len = q.len;
            g_quque_tail_index ++;
            g_quque_tail_index = g_quque_tail_index % QUEUE_MAX;
            g_queue_size++;
            pthread_mutex_unlock(&g_queue_mutex);
        } else {
            return -1;
        }
    } else {
        return -1;
    }
    return 0;
}

static int32_t queue_front(queue_local_s* q) {
    if(g_queue_size > 0) {
        MLOG_DBG("front: %d\n", g_quque_front_index);
        pthread_mutex_lock(&g_queue_mutex);
        q->data = g_quque_ring[g_quque_front_index].data;
        q->len = g_quque_ring[g_quque_front_index].len;
        pthread_mutex_unlock(&g_queue_mutex);
        return 0;
    } else {
        return -1;
    }
    return 0;
}

static int32_t queue_pop(void) {
    if(g_queue_size > 0) {
        if(g_quque_ring[g_quque_front_index].data != NULL) {
            MLOG_DBG("pop: %d\n", g_quque_front_index);
            pthread_mutex_lock(&g_queue_mutex);
            free(g_quque_ring[g_quque_front_index].data);
            g_quque_ring[g_quque_front_index].data = NULL;
            g_quque_ring[g_quque_front_index].len = 0;
            pthread_mutex_unlock(&g_queue_mutex);
        }
        g_quque_front_index++;
        g_quque_front_index = g_quque_front_index % QUEUE_MAX;
        g_queue_size--;
    } else {
        g_queue_size = 0;
        return -1;
    }

    return 0;
}

static int32_t queue_clear(void) {
    uint32_t i = 0;
    MLOG_DBG("clear\n");
    pthread_mutex_lock(&g_queue_mutex);
    for(i = 0; i< QUEUE_MAX; i++) {
        if(g_quque_ring[i].data) {
            free(g_quque_ring[i].data);
            g_quque_ring[i].data = NULL;
            g_quque_ring[i].len = 0;
        }
    }
    g_queue_size = 0;
    g_quque_front_index = 0;
    g_quque_tail_index = 0;
    pthread_mutex_unlock(&g_queue_mutex);
    return 0;
}

static int32_t queue_empty(void) {
    return !g_queue_size;
}

static int32_t queue_size(void) {
    return g_queue_size;
}

/* queue local: TODO>单独成文件 */

static void create_recorder(void) {
    if(g_recoder_thread == 0){
        MLOG_INFO("create recoder\n");
        pthread_create(&g_recoder_thread, NULL, thread_recoder_main, NULL);
    } else {
        MLOG_ERR("recoder has created\n");
    }
}

static void destory_recorder(void) {
    if(g_recoder_thread) {
        MLOG_INFO("destory recoder\n");
        g_recoder_runnnig = 0;
        sem_post(&g_recoder_sem);
        pthread_join(g_recoder_thread, NULL);
        g_recoder_thread = 0;
    } else {
        MLOG_ERR("recoder isn't created\n");
    }
}

static void create_processor(void) {
    if(g_processor_thread == 0){
        MLOG_INFO("create processor\n");
        pthread_create(&g_processor_thread, NULL, thread_processor_main, NULL);
    } else {
        MLOG_ERR("processor has created\n");
    }
}

static void destory_processor(void) {
    if(g_processor_thread) {
        MLOG_INFO("destory processor\n");
        g_processor_runnnig = 0;
        sem_post(&g_processor_sem);
        pthread_join(g_processor_thread, NULL);
        g_processor_thread = 0;
    } else {
        MLOG_ERR("processor isn't created\n");
    }
}

static void param_init()
{
    pthread_mutex_lock(&g_stt_params_mutex);
    g_stt_params = speech_to_text_default_params();
    pthread_mutex_unlock(&g_stt_params_mutex);
}

static void *thread_recoder_main(void *arg)
{
    (void)arg;
    int ret;
    int readbytes       = 0;
    int total_readbytes = 0;
    /* 录音时间测量 */
    struct timespec rec_t0, rec_t1;

    MAPI_ACAP_HANDLE_T AcapHdl;
    MAPI_ACAP_ATTR_S stACapAttr;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S stAecFrm;

    stACapAttr.channel        = 2;
    stACapAttr.enSampleRate   = AUDIO_SAMPLE_RATE_16000;
    stACapAttr.u32PtNumPerFrm = 640;
    stACapAttr.bVqeOn         = 1;
    stACapAttr.volume         = 24;

    sem_init(&g_recoder_sem, 0, 0);
    g_recoder_runnnig = 1;

    while(g_recoder_runnnig) {
        MLOG_INFO("wait record...\n");
        sem_wait(&g_recoder_sem);
        g_is_recoder_reset = 0;

        if(!g_recoder_runnnig) {
            break;
        }

        queue_clear();

        ret = MAPI_ACAP_Init(&AcapHdl, &stACapAttr);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_ACAP_Init failed\n");
            continue;
        }

        ret = MAPI_ACAP_Start(AcapHdl);
        if(ret != MAPI_SUCCESS) {
            MLOG_ERR("MAPI_ACAP_Start failed\n");
            MAPI_ACAP_Deinit(AcapHdl);
            AcapHdl = NULL;
            continue;
        }

        /* 新的对话 */
        g_asr_text_offset = 0;
        total_readbytes = 0;
        g_recorder_time_ms = 0;
        g_is_finial = 0;

        /* 打开文件用于保存录音数据 */
#ifdef DEBUG
        FILE *pcm_file = fopen("input.pcm", "wb");
        if(pcm_file == NULL) {
            MLOG_ERR("Failed to open input.pcm for writing\n");
        }
#endif
        /* 开始录音 */
        clock_gettime(CLOCK_MONOTONIC, &rec_t0);
        while(g_is_recoding) {
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
#ifdef DEBUG
            if(pcm_file != NULL) {
                fwrite((void *)stFrame.u64VirAddr[0], 1, readbytes, pcm_file);
            }
#endif
            MLOG_INFO("recode size: %d bytes\n", readbytes);

            /* 保存数据到队列 */
            queue_local_s audio_chunk = {0};
            audio_chunk.data = (void *)stFrame.u64VirAddr[0];
            audio_chunk.len = readbytes;
            queue_push(audio_chunk);

            /* 释放音频数据 */
            ret = MAPI_ACAP_ReleaseFrame(AcapHdl, &stFrame, &stAecFrm);
            if(ret != CVI_SUCCESS) {
                MLOG_ERR("MAPI_ACAP_ReleaseFrame, failed with %d!\n", ret);
            }
        }

        /* 录音结束 */
        clock_gettime(CLOCK_MONOTONIC, &rec_t1);
        g_recorder_time_ms = (rec_t1.tv_sec - rec_t0.tv_sec) * 1000 + (rec_t1.tv_nsec - rec_t0.tv_nsec)/1000000;
        /* 录音时间 */
        MLOG_INFO("recoder time: %dms\n", g_recorder_time_ms);

        MAPI_ACAP_Stop(AcapHdl);
        MAPI_ACAP_Deinit(AcapHdl);
        AcapHdl = NULL;

        /* 关闭文件 */
#ifdef DEBUG
        if(pcm_file != NULL) {
            fclose(pcm_file);
            MLOG_INFO("recoder data(%d bytes) saved to input.pcm\n", total_readbytes);
        }
#endif
    }

    if(g_processor != NULL) {
        speech_to_text_destroy(g_processor);
        g_processor = NULL;
    }
    sem_destroy(&g_recoder_sem);
    g_recoder_thread = 0;
    MLOG_INFO("recoder exit\n");
    return NULL;
}

static void *thread_processor_main(void *arg) {
    int ret = 0;

    sem_init(&g_processor_sem, 0, 0);
    g_processor_runnnig = 1;

    while(g_processor_runnnig) {

        MLOG_INFO("wait process...\n");
        sem_wait(&g_processor_sem);

        if(g_processor_runnnig) {
            g_is_network_error = 0;
            param_init();
            pthread_mutex_lock(&g_processor_mutex);
            g_processor = NULL;
            ret = speech_to_text_create(&g_processor, &g_stt_params);
            pthread_mutex_unlock(&g_processor_mutex);
            MLOG_INFO("speech_to_text_create ret=%d\n", ret);
            if(ret) {
                MLOG_ERR("Failed to create processor\n");
                g_processor = NULL;
                g_is_network_error = 1;
                continue;
            }
        }

        /* 发送数据到云端 */
        while(1) {
            if(queue_empty()) {
                if(g_is_recoding) {
                    MLOG_INFO("audio empty\n");
                    usleep(10 * 1000);
                    continue;
                } else {
                    MLOG_INFO("porcess finished\n");
                    break;
                }
            }
            queue_local_s audio_chunk = {0};
            queue_front(&audio_chunk);
            if(g_processor) {
                pthread_mutex_lock(&g_processor_mutex);
                speech_to_text_send_frame(g_processor, audio_chunk.data, audio_chunk.len);
                pthread_mutex_unlock(&g_processor_mutex);
            }
            queue_pop();
        }

        /* 将数据提交给云端asr */
        if(g_processor) {
            pthread_mutex_lock(&g_processor_mutex);
            ret = speech_to_text_send_finish(g_processor);
            pthread_mutex_unlock(&g_processor_mutex);
        }
        MLOG_DBG("send finished: %s\n", speech_to_text_get_error_string(ret));
        if(ret != STT_SUCCESS) {
            MLOG_ERR("error\n");
        }

        /* 录音和识别都结束 */
        if(g_processor != NULL && !g_is_recoding && g_is_finial) {
            pthread_mutex_lock(&g_processor_mutex);
            speech_to_text_destroy(g_processor);
            pthread_mutex_unlock(&g_processor_mutex);
            g_processor = NULL;
        }
    }
    return NULL;
}

int32_t rtt_init(void) {
    create_processor();
    create_recorder();
    return 0;
}

int32_t rtt_deinit(void) {
    destory_recorder();
    destory_processor();
    return 0;
}

int32_t rtt_get_text(char **text){
    bool is_final = false;
    int ret;
    size_t text_size = 0;
    static size_t last_text_size = -1;
    static uint32_t no_new_text_count = 0;

    /* processor 正在连接 */
    if(g_processor == NULL && !g_is_network_error) {
        *text = g_voice_asr_text;
        MLOG_ERR("waiting for processor\n");
        return RTT_SUCCESS;
    }

    /* 连接失败 */
    if(g_is_network_error) {
        *text = "NETWORK ERROR";
        return RTT_ERR_NETWORK;
    }

    /* 已经停止录音, 没有检测到语音 */
    if(!g_is_recoding && !g_is_get_first_text) {
        MLOG_ERR("no detect spk\n");
        *text = "NO SPEAKER";
        return RTT_ERR_NO_SPEAKER;
    }

    text_size = sizeof(g_voice_asr_text) - g_asr_text_offset;
    if(text_size <= 0){
        MLOG_ERR("no buf\n");
        *text = "NO BUF";
        return RTT_ERR_NO_BUF;
    }

    pthread_mutex_lock(&g_processor_mutex);
    MLOG_INFO("lock....in\n");
    ret = speech_to_text_get_text(g_processor, g_voice_asr_text, &text_size, &is_final);
    if (ret == STT_SUCCESS) {
        MLOG_INFO("rtt text(size:%u): %s\n", text_size, g_voice_asr_text);
        g_asr_text_offset = text_size;
        g_is_get_first_text = 1;
    }
    MLOG_INFO("lock....out\n");
    pthread_mutex_unlock(&g_processor_mutex);

    /* 停止录音后, 连续多次没有文字则结束 */
    if(g_is_recoding == 0 && text_size == last_text_size) {
        no_new_text_count++;
        MLOG_INFO("no new: %dn", no_new_text_count);
    }
    last_text_size = text_size;
    if(no_new_text_count > 4) {
        is_final = 1;
        no_new_text_count = 0;
        last_text_size = -1;
    }

    /* asr结束 */
    if (is_final) {
        g_is_finial = 1;
        if(g_processor != NULL && !g_is_recoding) {
            speech_to_text_destroy(g_processor);
            g_processor = NULL;
        }
        MLOG_INFO("rtt finished\n");
    }

    /* 存放所有的text */
    *text = g_voice_asr_text;
    return RTT_SUCCESS;
}

int32_t rtt_reset(void) {
    MLOG_INFO("reset\n");
    g_is_recoder_reset = 1;
    g_is_get_first_text = 0;
    g_is_network_error = 0;

    if(g_processor != NULL) {
        speech_to_text_destroy(g_processor);
        g_processor = NULL;
    }
    return 0;
}

int32_t rtt_start(void) {
    MLOG_INFO("start\n");
    if(g_recoder_runnnig) {
        memset(g_voice_asr_text, 0, sizeof(g_voice_asr_text));
        sem_post(&g_recoder_sem);
        sem_post(&g_processor_sem);
        g_is_recoding = 1;
        g_is_get_first_text = 0;
    }
    return 0;
}

int32_t rtt_stop(void) {
    MLOG_INFO("stop\n");
    g_is_recoding = 0;
    return 0;
}

int32_t rtt_is_finial(void) {
    return g_is_finial && !g_is_recoding;
}

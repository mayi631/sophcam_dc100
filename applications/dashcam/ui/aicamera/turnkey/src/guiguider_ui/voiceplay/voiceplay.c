#include <unistd.h>
#ifdef SERVICES_LIVEVIEW_ON
#include "volmng.h"
#endif
#include "media_init.h"
#include "mlog.h"
#include "voiceplay.h"
#include "ui_common.h"

/**
 * @brief 音频播放状态检查函数
 *
 * 等待音频播放完成，根据实际音效长度调整等待时间。
 * 通常音效文件长度在200-800ms之间。
 *
 * @return bool 总是返回true
 */
static bool is_audio_playing_complete(void)
{
    // 等待音频播放完成
    usleep(800 * 1000);
    return true;
}

/**
 * @brief 异步清理音效播放系统
 *
 * 该函数用于在后台线程中清理音效播放资源，
 * 包括等待音频播放完成、释放音效播放队列和音频输出设备。
 * 添加了延时
 * @param arg 线程参数（通常为NULL）
 * @return void* 线程返回值（通常为NULL）
 */
void* UI_VOICEPLAY_DeInit_Delay(void* arg)
{
    int32_t s32Ret = 0;

    // 等待音频播放完成
    is_audio_playing_complete();
#ifdef SERVICES_LIVEVIEW_ON
    // 退出音效播放
    s32Ret = VOICEPLAY_DeInit();
    if (s32Ret != CVI_SUCCESS) {
        MLOG_ERR("VOICEPLAY_DeInit failed with %d!\n", s32Ret);
    }
#endif
    // 退出AO
    s32Ret = MEDIA_AoDeInit();
    if (s32Ret != CVI_SUCCESS) {
        MLOG_ERR("MEDIA_AoDeInit failed with %d!\n", s32Ret);
    }

    return NULL;
}

/**
 * @brief 异步清理音效播放系统
 *
 * 该函数用于在后台线程中清理音效播放资源，
 * 包括等待音频播放完成、释放音效播放队列和音频输出设备。
 *
 * @param arg 线程参数（通常为NULL）
 * @return void* 线程返回值（通常为NULL）
 */
void* UI_VOICEPLAY_DeInit(void* arg)
{
    int32_t s32Ret = 0;

#ifdef SERVICES_LIVEVIEW_ON
    // 退出音效播放
    s32Ret = VOICEPLAY_DeInit();
    if (s32Ret != CVI_SUCCESS) {
        MLOG_ERR("VOICEPLAY_DeInit failed with %d!\n", s32Ret);
    }
#endif
    // 退出AO
    s32Ret = MEDIA_AoDeInit();
    if (s32Ret != CVI_SUCCESS) {
        MLOG_ERR("MEDIA_AoDeInit failed with %d!\n", s32Ret);
    }

    return NULL;
}

/**
 * @brief 初始化音效播放系统
 *
 * 该函数会初始化音频输出设备和音效播放队列，
 * 创建音效播放线程，准备播放音效文件。
 *
 * @return void
 */
void UI_VOICEPLAY_Init(void)
{
    // 初始化音频
    int32_t s32Ret = 0;
    s32Ret = MEDIA_AoInit();
    if (s32Ret != CVI_SUCCESS) {
        MLOG_ERR("MEDIA_AoInit failed with %d!\n", s32Ret);
    }

#ifdef SERVICES_LIVEVIEW_ON
    MEDIA_SYSHANDLE_S SysHandle = MEDIA_GetCtx()->SysHandle;
    VOICEPLAY_CFG_S stVoicePlayCfg = {0};

    stVoicePlayCfg.stAoutOpt.hAudDevHdl = SysHandle.aohdl;

    stVoicePlayCfg.u32MaxVoiceCnt = UI_VOICE_MAX_NUM;
    VOICEPLAY_VOICETABLE_S astVoiceTab[UI_VOICE_MAX_NUM] =
        {
            {UI_VOICE_START_UP_IDX, UI_VOICE_START_UP_SRC},
            {UI_VOICE_TOUCH_BTN_IDX, UI_VOICE_TOUCH_BTN_SRC},
            {UI_VOICE_CLOSE_IDX, UI_VOICE_CLOSE_SRC},
            {UI_VOICE_PHOTO_IDX, UI_VOICE_PHOTO_SRC},
        };
    stVoicePlayCfg.pstVoiceTab = astVoiceTab;
    s32Ret = VOICEPLAY_Init(&stVoicePlayCfg);
    if (s32Ret != 0) {
        CVI_LOGE("VOICEPLAY_Init failed!\n");
    }
    MLOG_DBG("VOICEPLAY_Init success!\n");
#endif
}
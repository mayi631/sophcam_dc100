/*
 * Copyright 2025 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "gui_guider.h"
#include "events_init.h"
#include "config.h"
#include "custom.h"
#include "mapi_acap.h"
#include "mapi_ao.h"
#include "voice_chat/voice_chat.h"

static bool g_is_recording = false;
static bool g_is_playing = false;
static pthread_t g_thread_record;
static pthread_t g_thread_play;

static voice_chat_processor_t* processor = NULL;

void *thread_record_func(void *arg)
{
    voice_chat_params_t params =  voice_chat_default_params();
	params.project_id = "6i9jCXvF6kwjShzsslV4N7";
	params.api_key = "MEJvhg4R9uQ-w3Cz7WCORUU_z1FCcs37uIZr3LMzIftXI-ICyoywpO_xY_6y2E2BRQZm7xUk2ayRqluu6CKRjw";
	params.asr_easyllm_id = "4Q0eQg6Jy38wDIRB5P5d6z";
	params.tts_easyllm_id = "7W3iwdmNhGs1ktPicpJPls";
	params.asr_params.format = "pcm";
	params.asr_params.sample_rate = 16000;
	params.llm_params.model_name = "DeepSeek-V3-Fast";
	params.llm_params.prompt = "回答不要超过200字。";
	params.tts_params.voice = "longxiaochun";
	params.tts_params.format = "PCM_16000HZ_MONO_16BIT";
    if (processor != NULL) {
        MLOG_ERR("processor already created\n");
        voice_chat_destroy(processor);
        processor = NULL;
        return NULL;
    }
    processor = voice_chat_create(&params);
    if (processor == NULL) {
        MLOG_ERR("Failed to create voice_chat_processor\n");
        return NULL;
    }

    uint32_t ChannelCount = 1;
    uint32_t samplerate = 16000;
    uint32_t u32PtNumPerFrm = 640;

    MAPI_ACAP_HANDLE_T AcapHdl;
    MAPI_ACAP_ATTR_S stACapAttr;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S   stAecFrm;
    stACapAttr.channel = ChannelCount;
    stACapAttr.enSampleRate = (AUDIO_SAMPLE_RATE_E)samplerate;
    stACapAttr.u32PtNumPerFrm = u32PtNumPerFrm;
    stACapAttr.bVqeOn = 0;
    stACapAttr.volume = 32;

    int ret;
    int readbytes = 0;
    int total_readbytes = 0;

    // 打开文件用于保存录音数据
    FILE* pcm_file = fopen("input.pcm", "wb");
    if (pcm_file == NULL) {
        MLOG_ERR("Failed to open input.pcm for writing\n");
        return NULL;
    }

    MAPI_ACAP_Init(&AcapHdl, &stACapAttr);
    MAPI_ACAP_Start(AcapHdl);

    g_is_recording = true;
    while(g_is_recording) {

         // 获取音频数据
        ret = MAPI_ACAP_GetFrame(AcapHdl, &stFrame, &stAecFrm);
        if (ret != CVI_SUCCESS) {
            MLOG_ERR("MAPI_ACAP_GetFrame failed,ret:%d.!!\n",ret);
            usleep(1000*(1000*u32PtNumPerFrm/samplerate)/2);
            continue;
        }
        readbytes = stFrame.u32Len*ChannelCount*2;
        total_readbytes += readbytes;

        // 保存录音数据到文件
        fwrite((void*)stFrame.u64VirAddr[0], 1, readbytes, pcm_file);
        MLOG_INFO("r: %d\n", readbytes);

        // 发送数据到云端
        voice_chat_send_frame(processor, stFrame.u64VirAddr[0], readbytes);

        // 释放音频数据
        ret = MAPI_ACAP_ReleaseFrame(AcapHdl,&stFrame,&stAecFrm);
        if (ret != CVI_SUCCESS) {
            MLOG_ERR("MAPI_ACAP_ReleaseFrame, failed with %d!\n",ret);
        }
    }
    ret = voice_chat_send_finish(processor);
    MLOG_INFO("发送完成信号: %s\n", voice_chat_get_error_string(ret));

    // 关闭文件
    fclose(pcm_file);
    MLOG_INFO("录音数据已保存到 input.pcm\n");

    MAPI_ACAP_Deinit(AcapHdl);
    MLOG_INFO("total_readbytes: %d\n", total_readbytes);

    return NULL;
}

void *thread_play_func(void *arg)
{

    g_is_playing = true;
    // 尝试获取音频回复
	printf("等待音频回复...\n");
	// char audio_output[] = "test_output.pcm";
	// FILE* output_file = fopen(audio_output, "wb");
    int ret;

    MAPI_AO_ATTR_S stAoAttr;
    stAoAttr.enSampleRate=16000;
    stAoAttr.channels=1;
    stAoAttr.u32PtNumPerFrm=320;
    stAoAttr.u32PowerPinId=0;
    stAoAttr.AudioChannel=1;
    stAoAttr.volume=15;

    AUDIO_FRAME_S stFrame;
    // CVI_S32 AoChn =0;

    MAPI_AO_HANDLE_T AoHdl;
    MAPI_AO_Init(&AoHdl, &stAoAttr);
    if (MAPI_AO_Start(AoHdl,0)){
        MLOG_ERR("MAPI_AO_Start() failed\n");
        goto out;
    }
    if(MAPI_AO_SetVolume(AoHdl,20)) {
        MLOG_ERR("CVI_AO_SetVolume() ERR\n");
        goto out;
    }

    size_t audio_size = 8192;
    // uint8_t *buffer = malloc(audio_size);
    // size_t buffer_index = 0;
    // stFrame.u64VirAddr[0] =  malloc(320*2);
    stFrame.u64VirAddr[0] =  malloc(8192);

    while(g_is_playing)
    {
        audio_size = 8192;
        memset(stFrame.u64VirAddr[0], 0, audio_size);
        ret = voice_chat_get_audio(processor, stFrame.u64VirAddr[0], &audio_size);
        if (ret == VOICE_CHAT_SUCCESS && audio_size > 0) {
            // fwrite(stFrame.u64VirAddr[0], 1, audio_size, output_file);
            printf("收到音频数据: %zu 字节\n", audio_size);
        } else if (ret == VOICE_CHAT_BUFFER_TOO_SMALL) {
            usleep(1000);  // 1ms
            continue;
        } else if (ret == VOICE_CHAT_CHAT_COMPLETED) {
            printf("对话完成\n");
            break;
        } else {
            printf("获取音频失败: %s\n", voice_chat_get_error_string(ret));
            break;
        }

        // buffer_index = 0;
        // while (audio_size > 0) {
        //     size_t send_size = audio_size > 640 ? 640 : audio_size;
        //     memcpy(stFrame.u64VirAddr[0], buffer + buffer_index, send_size);
        //     buffer_index += send_size;
        //     audio_size -= send_size;
        //     stFrame.u32Len = 320;
        //     stFrame.u64TimeStamp = 0;
        //     stFrame.enSoundmode = 0;
        //     stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
        //     ret = MAPI_AO_SendFrame(AoHdl, 0, &stFrame, 1000);
        //     if(ret){
        //         MLOG_ERR("MAPI_AO_SendFrame failed!\n");
        //         goto out;
        //     }
        //     printf("p1: %zu\n", send_size);
        // }
        stFrame.u32Len = audio_size / 2;
        stFrame.u64TimeStamp = 0;
        stFrame.enSoundmode = 0;
        stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
        ret = MAPI_AO_SendFrame(AoHdl, 0, &stFrame, 1000);
        if(ret){
            MLOG_ERR("MAPI_AO_SendFrame failed!\n");
            goto out;
        }

    }
    sleep(1);
    MAPI_AO_Stop(AoHdl, 0);
    MAPI_AO_Deinit(AoHdl);
out:
    // 清理
    g_is_playing = false;
	voice_chat_destroy(processor);
    processor = NULL;
    return NULL;
}

static void screen_AIDialogDr_btn_talk_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_PRESSED: {
            // 开始录音，并发送数据到云端
            if (g_is_recording || g_is_playing) {
                MLOG_ERR("already recording or playing\n");
                break;
            }
            pthread_create(&g_thread_record, NULL, thread_record_func, NULL);
            pthread_detach(g_thread_record);
            break;
        }
        case LV_EVENT_RELEASED: {
            // 停止录音，并播放收到的音频数据
            g_is_recording = false; // 停止录音
            pthread_create(&g_thread_play, NULL, thread_play_func, NULL);
            pthread_detach(g_thread_play);
            break;
        }
        default: break;
    }
}

static void screen_AIDialogDr_btn_back_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_CLICKED: {
            ui_load_scr_animation(&g_ui, &g_ui.page_aidialog.scr, g_ui.screen_AIDialog_del, &g_ui.screen_AIDialogDr_del,
                                  setup_scr_screen_AIDialog, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default: break;
    }
}

void events_init_screen_AIDialogDr(lv_ui_t *ui)
{
    lv_obj_add_event_cb(ui->page_aidialogdr.btn_talk, screen_AIDialogDr_btn_talk_event_handler, LV_EVENT_RELEASED, ui);
    lv_obj_add_event_cb(ui->page_aidialogdr.btn_talk, screen_AIDialogDr_btn_talk_event_handler, LV_EVENT_PRESSED, ui);
    lv_obj_add_event_cb(ui->page_aidialogdr.btn_back, screen_AIDialogDr_btn_back_event_handler, LV_EVENT_CLICKED, ui);
}

void setup_scr_screen_AIDialogDr(lv_ui_t *ui)
{

    MLOG_DBG("loading page_AIDialogDr...\n");

    AIDialogDr_t *AIDialogDr = &ui->page_aidialogdr;
    AIDialogDr->del          = true;

    // 创建主页面1 容器
    if(AIDialogDr->scr != NULL) {
        if(lv_obj_is_valid(AIDialogDr->scr)) {
            MLOG_DBG("page_AIDialogDr->scr 仍然有效，删除旧对象\n");
            lv_obj_del(AIDialogDr->scr);
        } else {
            MLOG_DBG("page_AIDialogDr->scr 已被自动销毁，仅重置指针\n");
        }
        AIDialogDr->scr = NULL;
    }

    // Write codes screen_AIDialogDr
    AIDialogDr->scr = lv_obj_create(NULL);
    lv_obj_set_size(AIDialogDr->scr, 640, 480);
    lv_obj_set_scrollbar_mode(AIDialogDr->scr, LV_SCROLLBAR_MODE_OFF);

    // Write style for scr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogDr->scr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogDr->scr, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogDr->scr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes top_cont (顶部栏)
    AIDialogDr->top_cont = lv_obj_create(AIDialogDr->scr);
    lv_obj_set_pos(AIDialogDr->top_cont, 0, 0);
    lv_obj_set_size(AIDialogDr->top_cont, 640, 60);
    lv_obj_set_scrollbar_mode(AIDialogDr->top_cont, LV_SCROLLBAR_MODE_OFF);

    // Write style for top_cont, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AIDialogDr->top_cont, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogDr->top_cont, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogDr->top_cont, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogDr->top_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_back (返回按钮)
    AIDialogDr->btn_back = lv_button_create(AIDialogDr->scr);
    lv_obj_set_pos(AIDialogDr->btn_back, 0, 0);
    lv_obj_set_size(AIDialogDr->btn_back, 77, 56);
    AIDialogDr->label_back = lv_label_create(AIDialogDr->btn_back);
    lv_label_set_text(AIDialogDr->label_back, "" LV_SYMBOL_LEFT " ");
    lv_label_set_long_mode(AIDialogDr->label_back, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialogDr->label_back, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialogDr->btn_back, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialogDr->label_back, LV_PCT(100));

    // Write style for btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogDr->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialogDr->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogDr->btn_back, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogDr->btn_back, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialogDr->btn_back, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialogDr->btn_back, &lv_font_montserratMedium_45, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialogDr->btn_back, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialogDr->btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_dr (百科博士按钮)
    AIDialogDr->btn_dr = lv_button_create(AIDialogDr->scr);
    lv_obj_set_pos(AIDialogDr->btn_dr, 252, 130);
    lv_obj_set_size(AIDialogDr->btn_dr, 136, 193);
    AIDialogDr->label_dr = lv_label_create(AIDialogDr->btn_dr);
    lv_label_set_text(AIDialogDr->label_dr, "百科博士");
    lv_label_set_long_mode(AIDialogDr->label_dr, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialogDr->label_dr, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialogDr->btn_dr, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialogDr->label_dr, LV_PCT(100));

    // Write style for btn_dr, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogDr->btn_dr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogDr->btn_dr, lv_color_hex(0x2C2C2C), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogDr->btn_dr, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialogDr->btn_dr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogDr->btn_dr, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogDr->btn_dr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialogDr->btn_dr, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialogDr->btn_dr, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialogDr->btn_dr, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialogDr->btn_dr, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Write codes btn_talk (按住对话按钮)
    AIDialogDr->btn_talk = lv_button_create(AIDialogDr->scr);
    lv_obj_set_pos(AIDialogDr->btn_talk, 139, 428);
    lv_obj_set_size(AIDialogDr->btn_talk, 361, 38);
    AIDialogDr->label_talk = lv_label_create(AIDialogDr->btn_talk);
    lv_label_set_text(AIDialogDr->label_talk, "按住对话");
    lv_label_set_long_mode(AIDialogDr->label_talk, LV_LABEL_LONG_WRAP);
    lv_obj_align(AIDialogDr->label_talk, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(AIDialogDr->btn_talk, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(AIDialogDr->label_talk, LV_PCT(100));

    // Write style for btn_talk, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(AIDialogDr->btn_talk, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(AIDialogDr->btn_talk, lv_color_hex(0xFFC107), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(AIDialogDr->btn_talk, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AIDialogDr->btn_talk, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(AIDialogDr->btn_talk, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AIDialogDr->btn_talk, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(AIDialogDr->btn_talk, lv_color_hex(0x181818), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(AIDialogDr->btn_talk, &lv_font_SourceHanSerifSC_Regular_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(AIDialogDr->btn_talk, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(AIDialogDr->btn_talk, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Update current screen layout.
    lv_obj_update_layout(AIDialogDr->scr);

    // Init events for screen.
    events_init_screen_AIDialogDr(ui);
}

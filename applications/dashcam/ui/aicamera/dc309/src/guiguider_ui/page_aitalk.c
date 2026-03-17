#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "gui_guider.h"
#include "page_all.h"
#include "custom.h"
#include "config.h"
#include "indev.h"
#include <linux/input.h>
#include "dialogxx.h"
#include "mapi_ao.h"
#include "media_init.h"
#include "voice_chat/voice_chat.h"

#define MAX_MSG_NUM 32

// 字体样式
extern lv_style_t ttf_font_20;
extern lv_style_t ttf_font_24;
extern lv_style_t ttf_font_30;

lv_obj_t* page_ai_talk_s;   // 底层窗口
lv_obj_t* chat_scroll_s;    // 对话列表窗口

// 添加全局变量用于语音输入
static lv_obj_t* voice_btn = NULL; // 语音按钮
static lv_obj_t* voice_btn_label = NULL; // 语音按钮文本标签

static lv_obj_t* voice_man = NULL;

// 音色选项配置
// static const char *voice_style_names[] = {
//     str_language_male_voice[get_curr_language()], str_language_female_voice[get_curr_language()]
//     // 后续可以在这里添加更多音色选项
// };

lv_timer_t *wait_llm_text_timer_s = NULL; // 等待定时器
lv_timer_t *wait_asr_text_timer_s = NULL; // 等待定时器
static bool recevice_timeout_flag = false; // 等待超时标志

// 聊天消息结构
typedef struct {
    const char* text[MAX_MSG_NUM];    // 消息内容（最多100条）
    const char* time;         // 消息时间
    uint8_t text_index;
    uint8_t need_hide[MAX_MSG_NUM];
} message_date_t;

message_date_t self_message_s = {0};
message_date_t ai_message_s = {0};

// 全局状态变量
static uint8_t wait_count = 0;           // 等待计数器
static bool waiting_for_response = false; // 等待响应标志
static uint8_t current_msg_index = 0;     // 当前等待的消息索引
static uint8_t llm_text_refresh = 0;

static void page_message_list_create(void);
void simulate_ai_response(void);

// 音色选择相关函数声明
static void voice_value_btn_event_cb(lv_event_t *e);
static void voice_style_btn_event_cb(lv_event_t *e);
static void wifi_return_to_ai_talk(void *user_data);
static int32_t update_llm_text(uint32_t times);
static void release_aitalk_resources(void);

// 获取AI回复内容
static const char *get_ai_char(void)
{
    return str_language_hello_how_can_i_help_you[get_curr_language()];
}

// 删除等待定时器
static void delete_wait_timer_cb(void)
{
    if(wait_llm_text_timer_s != NULL) {
        lv_timer_delete(wait_llm_text_timer_s);
        wait_llm_text_timer_s = NULL;
    }
    wait_count = 0;
    waiting_for_response = false;
}

// 定时器回调函数
static void recevice_llm_text_timer_cb(lv_timer_t *timer)
{
    // 超时时间timeout_100ms * 100ms
    static int32_t timeout_100ms = 50;
    uint32_t chat_state = CHAT_START_PLAY;
    if (!waiting_for_response) return;
    if (!lv_obj_is_valid(page_ai_talk_s)) {
        release_aitalk_resources();
        lv_timer_delete(timer);
        return;
    }

    // 检查是否被打断
    if(chat_get_reset()) {
        MLOG_INFO("reset, stop refresh llm text\n");
        // 收到消息，停止定时器
        delete_wait_timer_cb();
        ai_message_s.text_index ++;
        // 恢复5s超时: ASR完成之后, timeout_100ms内需要出结果
        timeout_100ms = 50;
        return;
    }

    // simulate_ai_response();
    // 更新llm text
    // 当回答的音频很短的时候, 音频立马完成(state=idle), 这时文字还没更新
    // 所以这里首先调用更新显示, 确保能更新一次
    update_llm_text(4);
    if(llm_text_refresh && ai_message_s.text[current_msg_index] != NULL){
        MLOG_INFO("refresh llm text:%d", current_msg_index);
        page_message_list_create();
        // 定时刷新
        lv_timer_reset(wait_llm_text_timer_s);
        llm_text_refresh = 0;
        // 30s超时: 文字需要timeout_100ms内全部显示完成
        wait_count = 0;
        timeout_100ms = 300;
        return;
    }

    // 检查是否收结束, 文字是在播放完成之前
    chat_get_state(&chat_state);
    if(chat_state != CHAT_START_PLAY && chat_state != CHAT_PLAYING) {
        MLOG_INFO("llm text finished\n");
        // 收到消息，停止定时器
        delete_wait_timer_cb();
        ai_message_s.text_index ++;
        // 恢复5s超时: ASR完成之后, timeout_100ms内需要出结果
        timeout_100ms = 50;
        return;
    }

    // 检查是否超时
    wait_count++;
    if (wait_count >= timeout_100ms) {
        MLOG_INFO("[%d]timeout:%d\n", current_msg_index, timeout_100ms);
        // 超时处理
        ai_message_s.text[current_msg_index] = str_language_timeout_please_try_again[get_curr_language()];
        recevice_timeout_flag = true;
        // 停止定时器
        delete_wait_timer_cb();
        // 刷新界面显示超时消息
        page_message_list_create();
        ai_message_s.text_index ++;
    }
}

// 定时器回调函数
static void recevice_asr_text_timer_cb(lv_timer_t *timer)
{
    // 处理语音输入 - 模拟用户输入
    // const char *asr_text = "用户模拟输入";
    if (!lv_obj_is_valid(page_ai_talk_s)) {
        release_aitalk_resources();
        lv_timer_delete(timer);
        return;
    }
    char *asr_text = chat_get_asr_text();
    if(asr_text && strlen(asr_text) > 0 && self_message_s.text_index < MAX_MSG_NUM) {
        // 分配内存并复制字符串
        self_message_s.text[self_message_s.text_index] = strdup(asr_text);
        if(self_message_s.text[self_message_s.text_index]) {
            // 初始化AI消息
            ai_message_s.text[self_message_s.text_index] = NULL;

            // 增加消息索引
            self_message_s.text_index++;

            // 更新asr消息列表, 触发llm显示
            page_message_list_create();
        }
    } else {
        MLOG_ERR("ASR failed\n");
    }

    // ASR完成, 恢复按键信息
    if(voice_btn_label) {
        lv_label_set_text(voice_btn_label, str_language_hold_to_speak[get_curr_language()]);
        lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    // 删除定时器
    if(wait_asr_text_timer_s != NULL) {
        lv_timer_delete(wait_asr_text_timer_s);
        wait_asr_text_timer_s = NULL;
    }
}


// 复位按钮
static void reset_voice_btn_timer_cb(lv_timer_t *timer){
    if(voice_btn_label) {
        lv_label_set_text(voice_btn_label, str_language_hold_to_speak[get_curr_language()]);
        lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    lv_timer_delete(timer);
}

// 网络错误处理
static void proc_network_err_timer_cb(lv_timer_t *timer){
    if (!lv_obj_is_valid(page_ai_talk_s)) {
        release_aitalk_resources();
        lv_timer_delete(timer);
        return;
    }
    const char *asr_text = str_language_net_connect_failed_please_check_network[get_curr_language()];
    // 分配内存并复制字符串
    ai_message_s.text[self_message_s.text_index] = strdup(asr_text);
    if(ai_message_s.text[self_message_s.text_index]) {
        // 用户消息不显示
        self_message_s.text[self_message_s.text_index] = NULL;
        self_message_s.need_hide[self_message_s.text_index] = 1;

        // 增加消息索引
        self_message_s.text_index++;
        ai_message_s.text_index ++;

        // 更新asr消息列表, 触发llm显示
        page_message_list_create();
    }

    if(voice_btn_label) {
        lv_label_set_text(voice_btn_label, str_language_hold_to_speak[get_curr_language()]);
        lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_timer_delete(timer);
}

static void proc_internal_err_timer_cb(lv_timer_t *timer){
    if (!lv_obj_is_valid(page_ai_talk_s)) {
        release_aitalk_resources();
        lv_timer_delete(timer);
        return;
    }
    const char *asr_text = str_language_unknown_error_please_restart_app[get_curr_language()];
    // 分配内存并复制字符串
    ai_message_s.text[self_message_s.text_index] = strdup(asr_text);
    if(ai_message_s.text[self_message_s.text_index]) {
        // 用户消息不显示
        self_message_s.text[self_message_s.text_index] = NULL;
        self_message_s.need_hide[self_message_s.text_index] = 1;

        // 增加消息索引
        self_message_s.text_index++;
        ai_message_s.text_index ++;

        // 更新asr消息列表, 触发llm显示
        page_message_list_create();
    }

    if(voice_btn_label) {
        lv_label_set_text(voice_btn_label, str_language_hold_to_speak[get_curr_language()]);
        lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_timer_delete(timer);
}

static void proc_timeout_timer_cb(lv_timer_t *timer){
    if (!lv_obj_is_valid(page_ai_talk_s)) {
        release_aitalk_resources();
        lv_timer_delete(timer);
        return;
    }
    const char *asr_text = str_language_timeout_please_try_again[get_curr_language()];
    // 分配内存并复制字符串
    ai_message_s.text[self_message_s.text_index] = strdup(asr_text);
    if(ai_message_s.text[self_message_s.text_index]) {
        // 用户消息不显示
        self_message_s.text[self_message_s.text_index] = NULL;
        self_message_s.need_hide[self_message_s.text_index] = 1;

        // 增加消息索引
        self_message_s.text_index++;
        ai_message_s.text_index ++;

        // 更新asr消息列表, 触发llm显示
        page_message_list_create();
    }

    if(voice_btn_label) {
        lv_label_set_text(voice_btn_label, str_language_hold_to_speak[get_curr_language()]);
        lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    lv_timer_delete(timer);
}

// 释放所有消息内存
void free_all_messages(void)
{
    for(uint8_t i = 0; i < self_message_s.text_index; i++) {
        if(self_message_s.text[i]) {
            free((void*)self_message_s.text[i]);
            self_message_s.text[i] = NULL;
        }
        // 修复：添加AI消息的内存释放
        if(ai_message_s.text[i]) {
            // 检查是否是动态分配的内存（不是常量字符串）
            if (strcmp(ai_message_s.text[i], str_language_timeout_please_try_again[get_curr_language()]) != 0 &&
                strcmp(ai_message_s.text[i], get_ai_char()) != 0 ) {
                free((void*)ai_message_s.text[i]);
            }
            ai_message_s.text[i] = NULL;
        }
        self_message_s.need_hide[i] = 0;
        ai_message_s.need_hide[i] = 0;
    }
    self_message_s.text_index = 0;
    ai_message_s.text_index = 0;
}

// 返回按钮回调
static void back_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch(code)
    {
        case LV_EVENT_CLICKED:
        {
            release_aitalk_resources();
            /* 关闭WiFi提示窗口 */
            wifi_check_dialog_close();
            ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1,
                                            LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        }
        default:
        {
            // 其他事件处理
            break;
        }
    }
}

static void aitalk_key_handler(int key_code, int key_value)
{
    uint32_t chat_state = 0;
    switch(key_code)
    {
        case KEY_MENU:
            release_aitalk_resources();
            ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1,
                                            LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
            break;
        case KEY_PLAY: {
            /* ASR不能被打断 */
            chat_get_state(&chat_state);
            if(chat_state == CHAT_ASR) {
                MLOG_WARN("asr can't be interupt\n");
                break;
            }
            if(key_value) {
                lv_obj_set_style_bg_color(voice_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(voice_btn, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_color(voice_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

                // 更新按钮文本
                if(voice_btn_label) {
                    lv_label_set_text(voice_btn_label, str_language_listening[get_curr_language()]);
                    lv_obj_set_style_text_color(voice_btn_label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
                }

                chat_set_reset();
                /* 开始录音 */
                chat_start_recorder();
            } else {
                // 恢复按钮状态 - 抬起时的样式 - 恢复默认黑白主题
                lv_obj_set_style_bg_color(voice_btn, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_text_color(voice_btn, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_color(voice_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

                // 恢复按钮文本
                if(voice_btn_label) {
                    lv_label_set_text(voice_btn_label, str_language_recognizing[get_curr_language()]);
                    lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
                }

                /* 停止录音 */
                chat_stop_recorder();
            }

        }; break;
        case KEY_MODE: {
            if(key_value) {
                voice_style_management();
            }
        }; break;
        case KEY_UP:
        case KEY_DOWN:
            // 上下键选择音色（在模态框内）
            if(key_value) {
                static uint8_t selected_index = 0;
                if(key_code == KEY_UP) {
                    selected_index = (selected_index > 0) ? 0 : 1;
                } else {
                    selected_index = (selected_index < 1) ? 1 : 0;
                }
                set_checkbox_index(selected_index);
            }
            break;

        case KEY_OK: make_sure_ok(); break;
        case KEY_ZOOMIN:
            do_zoomin(key_value);
            break;
        case KEY_ZOOMOUT:
            do_zoomout(key_value);
            break;
    }
}

// 创建消息列表
static void page_message_list_create(void)
{
    // 清除现有消息
    lv_obj_clean(chat_scroll_s);

    // 当前Y轴位置
    lv_coord_t current_y = 10;

    // 遍历所有消息
    for(uint8_t i = 0; i < self_message_s.text_index; i++)
    {
        if(!self_message_s.need_hide[i]) {
            // 创建用户消息容器（右侧）
            lv_obj_t * self_cont = lv_obj_create(chat_scroll_s);
            lv_obj_remove_style_all(self_cont);
            lv_obj_set_size(self_cont, LV_PCT(70), LV_SIZE_CONTENT);
            lv_obj_set_style_radius(self_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(self_cont, lv_color_hex(0x95EC69), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(self_cont, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(self_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

            // 用户消息文本
            lv_obj_t *text = lv_label_create(self_cont);
            lv_obj_set_width(text, lv_pct(80));
            lv_obj_align(text, LV_ALIGN_LEFT_MID, 36, 0);
            lv_obj_add_style(text, &ttf_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(text, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(text, self_message_s.text[i]);
            lv_label_set_long_mode(text, LV_LABEL_LONG_WRAP);
            lv_obj_update_layout(text);

            // 用户头像
            lv_obj_t *img_user = lv_img_create(self_cont);
            lv_obj_set_size(img_user, 24, 24);
            lv_obj_align(img_user, LV_ALIGN_RIGHT_MID, 0, 0);
            show_image(img_user, "user.png");

            // 设置用户消息位置（右侧）
            lv_coord_t x_pos = lv_obj_get_width(chat_scroll_s) - lv_obj_get_width(self_cont) - 20;
            lv_obj_set_pos(self_cont, x_pos, current_y);
            current_y += lv_obj_get_height(self_cont) + 10;
        }

        // 检查是否需要等待AI响应：最后一个asr，且没有llm应答
        if (!ai_message_s.need_hide[i] && i == self_message_s.text_index - 1 && ai_message_s.text[i] == NULL)
        {
            // 这是最新消息，需要等待AI响应
            current_msg_index = i;
            waiting_for_response = true;

            // 创建或重置定时器
            if(wait_llm_text_timer_s == NULL){
                wait_llm_text_timer_s = lv_timer_create(recevice_llm_text_timer_cb, 100, NULL);
            } else {
                lv_timer_reset(wait_llm_text_timer_s);
            }

            // 创建等待提示（左侧）
            lv_obj_t * waiting_cont = lv_obj_create(chat_scroll_s);
            lv_obj_remove_style_all(waiting_cont);
            lv_obj_set_size(waiting_cont, LV_PCT(70), LV_SIZE_CONTENT);
            lv_obj_set_style_radius(waiting_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(waiting_cont, lv_color_hex(0x95EC69), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(waiting_cont, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(waiting_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_t *waiting_text = lv_label_create(waiting_cont);
            lv_obj_set_width(waiting_text, lv_pct(80));
            lv_obj_align(waiting_text, LV_ALIGN_LEFT_MID, 36, 0);
            lv_obj_add_style(waiting_text, &ttf_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(waiting_text, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(waiting_text, str_language_waiting_for_response[get_curr_language()]);
            lv_obj_update_layout(waiting_text);

            lv_obj_t *waiting_img = lv_img_create(waiting_cont);
            lv_obj_set_size(waiting_img, 24, 24);
            lv_obj_align(waiting_img, LV_ALIGN_LEFT_MID, 6, 0);
            show_image(waiting_img, "user.png");

            // 设置等待提示位置（左侧）
            // lv_coord_t x_pos = lv_obj_get_width(chat_scroll_s) - lv_obj_get_width(waiting_cont) - 20;
            lv_obj_set_pos(waiting_cont, 10, current_y);
            current_y += lv_obj_get_height(waiting_cont) + 10;
        }

        // 如果AI消息存在，创建AI消息（左侧）
        if(!ai_message_s.need_hide[i] && ai_message_s.text[i] != NULL)
        {
            lv_obj_t * ai_cont = lv_obj_create(chat_scroll_s);
            lv_obj_remove_style_all(ai_cont);
            lv_obj_set_size(ai_cont, LV_PCT(70), LV_SIZE_CONTENT);
            lv_obj_set_style_radius(ai_cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_color(ai_cont, lv_color_hex(0x4A90E2), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(ai_cont, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_all(ai_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

            lv_obj_t *ai_text = lv_label_create(ai_cont);
            lv_obj_set_width(ai_text, lv_pct(80));
            lv_obj_align(ai_text, LV_ALIGN_LEFT_MID, 36, 0);
            lv_obj_add_style(ai_text, &ttf_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(ai_text, ai_message_s.text[i]);
            lv_label_set_long_mode(ai_text, LV_LABEL_LONG_WRAP);
            lv_obj_update_layout(ai_text);

            lv_obj_t *ai_img_user = lv_img_create(ai_cont);
            lv_obj_set_size(ai_img_user, 24, 24);
            lv_obj_align(ai_img_user, LV_ALIGN_LEFT_MID, 6, 0);
            show_image(ai_img_user, "user.png");

            // 设置AI消息位置（左侧）
            // lv_coord_t x_pos = lv_obj_get_width(chat_scroll_s) - lv_obj_get_width(ai_cont) - 20;
            lv_obj_set_pos(ai_cont, 10, current_y);
            current_y += lv_obj_get_height(ai_cont) + 10;
        }
    }

    lv_obj_t * occ_cont = lv_obj_create(chat_scroll_s);
    lv_obj_remove_style_all(occ_cont);
    lv_obj_set_size(occ_cont, LV_PCT(100), 40);
    lv_obj_set_pos(occ_cont, 0, current_y);
    lv_obj_set_style_radius(occ_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(occ_cont, lv_color_hex(0x95EC69), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(occ_cont, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(occ_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置滚动区域高度
    // lv_obj_set_height(chat_scroll_s, current_y + 20);

    // 滚动到底部
    if(self_message_s.text_index > 0) {
        lv_obj_scroll_to_y(chat_scroll_s, current_y, LV_ANIM_OFF);
    }
}

// 语音按钮事件回调
static void voice_btn_event_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn                    = lv_event_get_target(e);
    uint32_t chat_state = 0;

    /* ASR不能被打断 */
    chat_get_state(&chat_state);
    if(chat_state == CHAT_ASR &&
        (code == LV_EVENT_PRESSED || code == LV_EVENT_CANCEL
        || code == LV_EVENT_RELEASED)) {
        MLOG_WARN("asr can't be interupt\n");
        return;
    }

    switch(code) {
        case LV_EVENT_PRESSED: {
            // 更新按钮状态 - 按下时的样式 - 白色背景
            lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(btn, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

            // 更新按钮文本
            if(voice_btn_label) {
                lv_label_set_text(voice_btn_label, str_language_listening[get_curr_language()]);
                lv_obj_set_style_text_color(voice_btn_label, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
            }

            chat_set_reset();
            /* 开始录音 */
            chat_start_recorder();
            break;
        }
        case LV_EVENT_RELEASED:
        case LV_EVENT_CANCEL: {
            // 恢复按钮状态 - 抬起时的样式 - 恢复默认黑白主题
            lv_obj_set_style_bg_color(btn, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(btn, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);

            // 恢复按钮文本
            if(voice_btn_label) {
                lv_label_set_text(voice_btn_label, str_language_recognizing[get_curr_language()]);
                lv_obj_set_style_text_color(voice_btn_label, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
            }

            /* 停止录音 */
            chat_stop_recorder();
            break;
        }
        default:
            break;
    }
}

static void gesture_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_GESTURE: {
            // 获取手势方向，需要 TP 驱动支持
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
            switch(dir) {
                case LV_DIR_RIGHT: {
                    release_aitalk_resources();
                    ui_load_scr_animation(&g_ui, &obj_home_s, 1, NULL, setup_scr_home1, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                                          false, true);
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

static int32_t update_llm_text(uint32_t times)
{
    #define TEXT_PRE_MALLOC_SIZE 2048
    static char *llm_text_ptr = NULL;
    static char *voice_llm_text = NULL;
    uint32_t i = 0;

    if(ai_message_s.text[ai_message_s.text_index] == NULL){
        MLOG_INFO("new malloc for %d\n", ai_message_s.text_index);
        // 分配的内存在free_all_messages中释放
        voice_llm_text = malloc(TEXT_PRE_MALLOC_SIZE);
        if(voice_llm_text == NULL){
            MLOG_ERR("llm text malloc failed\n");
            return -1;
        }
        memset(voice_llm_text, 0, TEXT_PRE_MALLOC_SIZE);
        llm_text_ptr = voice_llm_text;
        ai_message_s.text[ai_message_s.text_index] = voice_llm_text;
    }

    for(i = 0; i < times; i++) {
        size_t text_size = TEXT_PRE_MALLOC_SIZE - (size_t)(llm_text_ptr - voice_llm_text);
        // MLOG_INFO("llm:%p/%p/%u\n", llm_text_ptr, voice_llm_text, text_size);
        if(text_size > 0 && llm_text_ptr && voice_llm_text){
            int ret = chat_get_llm_text(llm_text_ptr, &text_size);
            if (ret == VOICE_CHAT_SUCCESS) {
                MLOG_INFO("llm(size:%u):%s\n", text_size, llm_text_ptr);
                llm_text_ptr += text_size;
                /* llm text更新, 需要刷新 */
                llm_text_refresh = 1;
            } else {
                break;
            }
        }
    }

    return 0;
}

static void asr_post_callback(void) {
    MLOG_INFO("start show asr text\n");
    if(wait_asr_text_timer_s == NULL){
        wait_asr_text_timer_s = lv_timer_create(recevice_asr_text_timer_cb, 10, NULL);
    } else {
        lv_timer_reset(wait_asr_text_timer_s);
    }

    MLOG_INFO("start player\n");
    chat_start_player();
}

static void chat_err_callback(int32_t err_code) {
    MLOG_INFO("err_code: %d\n", err_code);
    switch(err_code){
        case VOICE_CHAT_NOT_CONNECTED:
        case VOICE_CHAT_NETWORK_ERROR:
            /* 网络连接错误, UI显示 */
            lv_timer_create(proc_network_err_timer_cb, 10, NULL);
        break;
        case VOICE_CHAT_BUFFER_TOO_SMALL:
            /* 触发定时器更新UI */
            lv_timer_create(reset_voice_btn_timer_cb, 10, NULL);
        break;
        case VOICE_CHAT_TIMEOUT:
            lv_timer_create(proc_timeout_timer_cb, 10, NULL);
        break;
        case VOICE_CHAT_INTERNAL_ERROR:
            lv_timer_create(proc_internal_err_timer_cb, 10, NULL);
        break;
        default:
        break;
    }
}

// 创建AI聊天页面
void create_chat_screen(lv_ui_t *ui) {
    // 创建录音\播放线程
    chat_create_recorder();
    chat_create_player();

    // 初始化消息索引
    self_message_s.text_index = 0;

    // 创建页面容器
    page_ai_talk_s = lv_obj_create(NULL);
    lv_obj_remove_style_all(page_ai_talk_s);
    lv_obj_set_scrollbar_mode(page_ai_talk_s, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_size(page_ai_talk_s, H_RES, V_RES);
    lv_obj_set_style_bg_color(page_ai_talk_s, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(page_ai_talk_s, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(page_ai_talk_s, gesture_event_handler, LV_EVENT_GESTURE, ui);

    // 创建顶部标题栏
    lv_obj_t* header = lv_obj_create(page_ai_talk_s);
    lv_obj_set_size(header, LV_PCT(100), 60);
    lv_obj_set_style_radius(header,0,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(header, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // 返回按钮
    lv_obj_t* back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 60, 52);
    lv_obj_set_pos(back_btn, 4, 4);
    lv_obj_set_style_pad_all(back_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(back_btn, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(back_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);



    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_obj_set_style_text_align(back_label, LV_TEXT_ALIGN_CENTER,LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(back_label, &lv_font_SourceHanSerifSC_Regular_30,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加返回事件
    lv_obj_add_event_cb(back_btn, back_cb, LV_EVENT_CLICKED, NULL);

    // 标题
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, str_language_ai_dialogue[get_curr_language()]);
    lv_obj_add_style(title, &ttf_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(title);

    // 菜单按钮
    voice_man = lv_imagebutton_create(header);
    lv_obj_align(voice_man, LV_ALIGN_RIGHT_MID, -88, 0);
    lv_obj_set_size(voice_man, 55, 48);
    set_voice_style_icon(voice_man);
    if(get_currindex()) {
        show_image(voice_man, "女声.png");
    } else {
        show_image(voice_man, "男声.png");
    }

    lv_obj_set_style_pad_all(voice_man, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(voice_man, lv_color_hex(0x1296db), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 设置焦点状态下的边框样式（保持原有）
    lv_obj_set_style_border_color(voice_man, lv_color_hex(0x1296db), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_event_cb(voice_man, voice_style_btn_event_cb, LV_EVENT_CLICKED, NULL);


    // 音色选择按钮（右侧）
    lv_obj_t *voice_style_btn = lv_btn_create(header);
    lv_obj_set_size(voice_style_btn, 60, 52);
    lv_obj_set_pos(voice_style_btn, H_RES - 64, 4); // 右侧位置
    lv_obj_set_style_pad_all(voice_style_btn, 0, LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(voice_style_btn, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(voice_style_btn, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(voice_style_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *voice_style_label = lv_label_create(voice_style_btn);
    lv_obj_set_style_text_align(voice_style_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text(voice_style_label, LV_SYMBOL_VOLUME_MAX); // 使用音量图标
    lv_obj_align(voice_style_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(voice_style_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加音色选择事件
    lv_obj_add_event_cb(voice_style_btn, voice_value_btn_event_cb, LV_EVENT_CLICKED, NULL);

    // 创建聊天内容容器
    chat_scroll_s = lv_obj_create(page_ai_talk_s);
    lv_obj_set_size(chat_scroll_s, LV_PCT(100), V_RES - 60); // 留出标题和输入区域空间
    lv_obj_align(chat_scroll_s, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_scrollbar_mode(chat_scroll_s, LV_SCROLLBAR_MODE_ON);
    lv_obj_set_style_bg_color(chat_scroll_s, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(chat_scroll_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(chat_scroll_s, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(chat_scroll_s, LV_SCROLLBAR_MODE_AUTO);


    // 在上方添加一条分割线
    lv_obj_t *up_line                       = lv_line_create(page_ai_talk_s);
    static lv_point_precise_t points_line[] = {{10, 60}, {640, 60}};
    lv_line_set_points(up_line, points_line, 2);
    lv_obj_set_style_line_width(up_line, 2, 0);
    lv_obj_set_style_line_color(up_line, lv_color_hex(0xFFFFFF), 0);


    // 创建语音按钮
    voice_btn = lv_btn_create(page_ai_talk_s);
    lv_obj_set_size(voice_btn, LV_PCT(80), 60);
    lv_obj_align(voice_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    // 设置默认样式 - 黑白主题
    lv_obj_set_style_bg_color(voice_btn, lv_color_hex(0x2A2A2A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(voice_btn, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(voice_btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(voice_btn, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(voice_btn, 8, LV_PART_MAIN | LV_STATE_DEFAULT);

    voice_btn_label = lv_label_create(voice_btn);
    lv_label_set_text(voice_btn_label, str_language_hold_to_speak[get_curr_language()]);
    lv_obj_center(voice_btn_label);
    lv_obj_add_style(voice_btn_label, &ttf_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 添加语音按钮事件
    lv_obj_add_event_cb(voice_btn, voice_btn_event_cb, LV_EVENT_ALL, NULL);

    // 设置当前页面按键处理回调
    set_current_page_handler(aitalk_key_handler);

    // 注册asr后处理回调
    chat_register_asr_post_callback(asr_post_callback);
    chat_register_asr_err_process_callback(chat_err_callback);
    /* 检查WiFi连接状态 */
    wifi_check_and_show_dialog(page_ai_talk_s, wifi_return_to_ai_talk, ui);
}

// 模拟AI响应（在实际应用中，这应该由网络回调触发）
void simulate_ai_response(void)
{
    if (waiting_for_response && ai_message_s.text[current_msg_index] == NULL)
    {
        ai_message_s.text[current_msg_index] = get_ai_char();
        ai_message_s.text_index++;
    }
}

// 音色选择按钮事件回调
static void voice_value_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        if(get_arc_handel() == NULL) {
            voice_setting_arc_create();
        } else {
            voice_arc_delete();
        }
    }
}

static void wifi_return_to_ai_talk(void *user_data)
{
    lv_ui_t *ui = (lv_ui_t *)user_data;
    if(ui == NULL) {
        MLOG_ERR("UI is NULL, using default UI...\n");
        ui = &g_ui;
    }
    ui_load_scr_animation(ui, &page_ai_talk_s, 1, NULL, create_chat_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false, true);
}

static void voice_style_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    switch(code) {
        case LV_EVENT_CLICKED: {
            // 创建音色选择模态框
            create_voice_style_modal();

        }; break;
        default: break;
    }
}

static void release_aitalk_resources(void)
{
    // 修复：添加定时器删除
    if(wait_llm_text_timer_s) {
        lv_timer_del(wait_llm_text_timer_s);
        wait_llm_text_timer_s = NULL;
    }

    if(wait_asr_text_timer_s) {
        lv_timer_del(wait_asr_text_timer_s);
        wait_asr_text_timer_s = NULL;
    }

    // 清理语音相关资源
    voice_btn       = NULL;
    voice_btn_label = NULL;

    voice_arc_delete(); //删除音；

    // 清理音色选择模态框
    close_voice_style_modal();

    chat_destory_recorder();
    chat_destory_player();

    free_all_messages(); // 释放内存

    set_current_page_handler(NULL);
}

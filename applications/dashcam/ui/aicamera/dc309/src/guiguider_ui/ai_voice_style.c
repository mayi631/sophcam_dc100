#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui_guider.h"
#include "page_all.h"
#include "custom.h"
#include "config.h"
#include "dialogxx.h"
#include "ttp.h"
#include "voice_chat/voice_chat.h"

// 添加音色选择相关全局变量
static lv_obj_t *voice_style_modal          = NULL;   // 音色选择模态框
static lv_obj_t *voice_style_cont           = NULL;   // 音色选择容器
static lv_obj_t *voice_style_checkboxes[10] = {NULL}; // 音色选择复选框数组
static uint8_t current_voice_style          = 1;      // 当前选择的音色索引
static uint8_t voice_style_count            = 2;      // 音色选项数量
static lv_obj_t *voice_style_icon_updata    = NULL;   // 给外部设置图片更新
extern lv_style_t ttf_font_20;
extern lv_style_t ttf_font_24;
static lv_obj_t *play_switch = NULL;
static bool play_switch_flag = true;
// 音色选项配置，注意模型用的v2，这里音色也只能用v2
static const char *voice_style_names[][32] = {
    {"男声", "longnan"}, {"女声", "longxiaochun"}
    // 后续可以在这里添加更多音色选项
};


// 关闭音色选择模态框
void close_voice_style_modal(void)
{
    if(voice_style_modal) {
        lv_obj_del(voice_style_modal);
        voice_style_modal = NULL;
        voice_style_cont  = NULL;
    }
}
// 模态框背景点击事件回调
static void voice_style_modal_bg_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MLOG_DBG("event: %s\n", lv_event_code_get_name(code));
    switch(code) {
        case LV_EVENT_GESTURE: {
            // 获取手势方向，需要 TP 驱动支持
            lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
            switch(dir) {
                case LV_DIR_RIGHT: {
                    close_voice_style_modal();
                }
                default: break;
            }
            break;
        }
        default: break;
    }
}

// 音色选择复选框事件回调
static void voice_style_checkbox_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *checkbox   = lv_obj_get_child(btn,0);

    if(code == LV_EVENT_CLICKED || code == LV_EVENT_VALUE_CHANGED) {
        // 找到被点击的复选框索引
        int clicked_index = -1;
        for(int i = 0; i < voice_style_count; i++) {
            if(voice_style_checkboxes[i] == checkbox) {
                clicked_index = i;
                break;
            }
        }

        if(clicked_index >= 0) {
            // 更新当前选择的音色
            current_voice_style = clicked_index;

            // 取消其他复选框的选中状态（实现单选效果）
            for(int i = 0; i < voice_style_count; i++) {
                if(i != clicked_index) {
                    lv_obj_clear_state(voice_style_checkboxes[i], LV_STATE_CHECKED);
                }
            }
            if(current_voice_style) {
                show_image(voice_style_icon_updata, "女声.png");
            } else {
                show_image(voice_style_icon_updata, "男声.png");
            }
            // 这里可以添加音色切换的逻辑
            // 例如：切换TTS引擎的音色设置
            MLOG_DBG("Selected voice style: %s\n", voice_style_names[current_voice_style][0]);
            chat_set_voice_param(voice_style_names[current_voice_style][1]);
            ttp_set_voice(voice_style_names[current_voice_style][1]);

            // 关闭模态框
            close_voice_style_modal();
        }
    }
}

// WiFi提示窗口开关回调
static void play_witch_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        play_switch_flag = !play_switch_flag;
        if(play_switch_flag == true) {
            lv_obj_add_state(play_switch, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(play_switch, LV_STATE_CHECKED);
        }
    }
}

// 创建音色选择模态框
void create_voice_style_modal(void)
{
    if(lv_obj_is_valid(voice_style_modal)) {
        lv_obj_del(voice_style_modal);
        voice_style_modal = NULL;
        MLOG_WARN("voice_style_modal 有效，删除后重新创建");
    }
    // 创建模态背景
    voice_style_modal = lv_obj_create(lv_scr_act());
    lv_obj_set_size(voice_style_modal, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(voice_style_modal, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(voice_style_modal, LV_OPA_50, 0);
    lv_obj_set_style_border_width(voice_style_modal, 0, 0);
    lv_obj_clear_flag(voice_style_modal, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(voice_style_modal, LV_OBJ_FLAG_GESTURE_BUBBLE);
    // 添加点击背景关闭事件
    lv_obj_add_event_cb(voice_style_modal, voice_style_modal_bg_event_cb, LV_EVENT_ALL, NULL);

    // 创建音色选择容器
    voice_style_cont = lv_obj_create(voice_style_modal);
    lv_obj_set_size(voice_style_cont, LV_PCT(70), LV_PCT(70));
    lv_obj_center(voice_style_cont);
    lv_obj_set_style_bg_color(voice_style_cont, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(voice_style_cont, 10, 0);
    lv_obj_set_style_pad_all(voice_style_cont, 20, 0);
    // 设置容器为可滚动
    lv_obj_set_scrollbar_mode(voice_style_cont, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_scroll_dir(voice_style_cont, LV_DIR_VER);

    // 创建标题
    lv_obj_t *title = lv_label_create(voice_style_cont);
    lv_label_set_text(title, str_language_select_voice_tone[get_curr_language()]);
    lv_obj_add_style(title, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    if(lv_scr_act() != page_ai_talk_s) {
        play_switch = lv_switch_create(voice_style_cont);
        lv_obj_align(play_switch, LV_ALIGN_TOP_RIGHT, 0, 4);
        lv_obj_add_event_cb(play_switch, play_witch_cb, LV_EVENT_VALUE_CHANGED, NULL);
        // 检查当前WiFi状态
        if(play_switch_flag == true) {
            lv_obj_add_state(play_switch, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(play_switch, LV_STATE_CHECKED);
        }
    }

    // 创建复选框容器
    lv_obj_t *checkbox_cont = lv_obj_create(voice_style_cont);
    lv_obj_set_size(checkbox_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(checkbox_cont, LV_ALIGN_TOP_MID, 0, 60);
    lv_obj_set_style_border_width(checkbox_cont, 0, 0);
    lv_obj_set_style_pad_all(checkbox_cont, 0, 0);
    lv_obj_set_style_bg_color(checkbox_cont, lv_color_hex(0x020524), LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建复选框选项
    for(int i = 0; i < voice_style_count; i++) {
        lv_obj_t *btn = lv_button_create(checkbox_cont);
        lv_obj_set_size(btn, LV_PCT(100), 50);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, (50 + 10) * i);
        lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT); // 透明背景
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x010414), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(btn, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(btn, get_usr_fonts(ALI_PUHUITI_FONTPATH, MENU_FONT_SIZE), LV_PART_MAIN | LV_STATE_DEFAULT);


        voice_style_checkboxes[i] = lv_checkbox_create(btn);
        lv_checkbox_set_text(voice_style_checkboxes[i], voice_style_names[i][0]);
        lv_obj_add_style(voice_style_checkboxes[i], &ttf_font_20, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(voice_style_checkboxes[i], lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
        // 设置复选框位置
        lv_obj_align(voice_style_checkboxes[i], LV_ALIGN_TOP_LEFT, 0, 0);

        // 设置当前选中的音色
        if(i == current_voice_style) {
            lv_obj_add_state(voice_style_checkboxes[i], LV_STATE_CHECKED);
        }
        lv_obj_clear_flag(voice_style_checkboxes[i], LV_OBJ_FLAG_CLICKABLE);
        // 添加事件回调
        lv_obj_add_event_cb(btn, voice_style_checkbox_event_cb, LV_EVENT_ALL, NULL);
    }
}

void set_checkbox_index(uint8_t index)
{
    if(voice_style_modal) {
        // 更新选中状态
        for(int i = 0; i < voice_style_count; i++) {
            if(voice_style_checkboxes[i]) {
                if(i == index) {
                    lv_obj_add_state(voice_style_checkboxes[i], LV_STATE_CHECKED);
                } else {
                    lv_obj_clear_state(voice_style_checkboxes[i], LV_STATE_CHECKED);
                }
            }
        }
    }
}

void make_sure_ok(void)
{
    // 确认选择
    if(voice_style_modal) {
        // 查找当前选中的音色
        for(int i = 0; i < voice_style_count; i++) {
            if(voice_style_checkboxes[i] && lv_obj_has_state(voice_style_checkboxes[i], LV_STATE_CHECKED)) {
                current_voice_style = i;
                MLOG_DBG("Selected voice style: %s\n", voice_style_names[current_voice_style][0]);
                chat_set_voice_param(voice_style_names[current_voice_style][1]);
                break;
            }
        }
        // 关闭模态框
        close_voice_style_modal();
    }
}

void voice_style_management(void)
{
    if(!lv_obj_is_valid(voice_style_modal) && voice_style_modal == NULL) {
        create_voice_style_modal();
    } else {
        close_voice_style_modal();
    }
}

uint8_t get_currindex(void)
{
    return current_voice_style;
}

void set_voice_style_icon(lv_obj_t *handler)
{
    voice_style_icon_updata = handler;
}

bool get_play_switch(void)
{
    return play_switch_flag;
}
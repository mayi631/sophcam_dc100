#define DEBUG
/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include <stdlib.h>
#include <string.h>
#include "custom.h"
#include <time.h>
#include "lvgl/src/libs/freetype/lv_freetype.h"
#include "ui_common.h"
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include "hal_wifi_ctrl.h"
#include "page_all.h"
#include "filemng.h"
#include "common/extract_thumbnail.h"
#include "mapi_ao.h"



// 语音输入弹框相关变量
static lv_obj_t *voice_input_popup = NULL;
static lv_obj_t *voice_text_label = NULL;
static lv_timer_t *voice_popup_timer = NULL;
extern lv_style_t ttf_font_16;
static lv_obj_t* ai_voice_confirm_btn = NULL;
static lv_obj_t* ai_voice_cancel_btn = NULL;
bool ai_custom_is_confire = false;  //AI自定义是否确定

void destroy_voice_input_popup(void)
{
    if (voice_input_popup) {
        lv_obj_del(ai_voice_confirm_btn);
        ai_voice_confirm_btn = NULL;
        lv_obj_del(ai_voice_cancel_btn);
        ai_voice_cancel_btn = NULL;
        lv_obj_del(voice_input_popup);
        voice_input_popup = NULL;
        voice_text_label = NULL;
    }

    if (voice_popup_timer) {
        lv_timer_del(voice_popup_timer);
        voice_popup_timer = NULL;
    }
}

static void confirm_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        destroy_voice_input_popup();
        ai_custom_is_confire = true;
    }
}

static void cancel_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        destroy_voice_input_popup();
        ai_custom_is_confire = false;
    }
}

// 创建语音输入弹框
void create_voice_input_popup(void)
{
    ai_custom_is_confire = false;

    voice_input_popup = lv_obj_create(lv_scr_act());
    lv_obj_set_size(voice_input_popup, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(voice_input_popup, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_opa(voice_input_popup, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(voice_input_popup, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(voice_input_popup, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(voice_input_popup, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(voice_input_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(voice_input_popup, LV_OBJ_FLAG_GESTURE_BUBBLE);

    // 创建弹框容器
    lv_obj_t *voice_input_popup_dispaly = lv_obj_create(voice_input_popup);
    lv_obj_set_size(voice_input_popup_dispaly, 400, 120);
    lv_obj_align(voice_input_popup_dispaly, LV_ALIGN_TOP_MID, 0, 100);

    // 现代化样式
    lv_obj_set_style_bg_color(voice_input_popup_dispaly, lv_color_hex(0x1E1E2E), 0);
    lv_obj_set_style_bg_opa(voice_input_popup_dispaly, LV_OPA_90, 0);
    lv_obj_set_style_radius(voice_input_popup_dispaly, 16, 0);
    lv_obj_set_style_border_width(voice_input_popup_dispaly, 0, 0);
    lv_obj_set_style_shadow_width(voice_input_popup_dispaly, 50, 0);
    lv_obj_set_style_shadow_opa(voice_input_popup_dispaly, LV_OPA_30, 0);
    lv_obj_set_style_pad_all(voice_input_popup_dispaly, 20, 0);

    // 语音文本
    voice_text_label = lv_label_create(voice_input_popup_dispaly);
    lv_label_set_text(voice_text_label, "用一段话描述需要创作的的风格");
    lv_obj_set_style_text_color(voice_text_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(voice_text_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);
    lv_obj_align(voice_text_label, LV_ALIGN_LEFT_MID, 40, 0);
    lv_obj_set_width(voice_text_label, 320);
    lv_label_set_long_mode(voice_text_label, LV_LABEL_LONG_SCROLL);

    // 确认按钮
    ai_voice_confirm_btn = lv_btn_create(voice_input_popup);
    lv_obj_set_size(ai_voice_confirm_btn, 100, 35);
    lv_obj_align(ai_voice_confirm_btn, LV_ALIGN_BOTTOM_LEFT, 100, -150);
    lv_obj_set_style_bg_color(ai_voice_confirm_btn, lv_color_hex(0x555555), 0);
    lv_obj_set_style_pad_all(ai_voice_confirm_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(ai_voice_confirm_btn, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *confirm_label = lv_label_create(ai_voice_confirm_btn);
    lv_label_set_text(confirm_label, "确认");
    lv_obj_add_style(confirm_label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(confirm_label);
    lv_obj_add_event_cb(ai_voice_confirm_btn, confirm_cb, LV_EVENT_CLICKED, NULL);

    // 取消按钮
    ai_voice_cancel_btn = lv_btn_create(voice_input_popup);
    lv_obj_set_size(ai_voice_cancel_btn, 100, 35);
    lv_obj_align(ai_voice_cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -100, -150);
    lv_obj_set_style_bg_color(ai_voice_cancel_btn, lv_color_hex(0x555555), 0);
    lv_obj_set_style_pad_all(ai_voice_cancel_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(ai_voice_cancel_btn, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *cancel_label = lv_label_create(ai_voice_cancel_btn);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_add_style(cancel_label, &ttf_font_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(cancel_label);
    lv_obj_add_event_cb(ai_voice_cancel_btn, cancel_cb, LV_EVENT_CLICKED, NULL);

}

void voice_display_button(void)
{
    if (lv_obj_is_valid(ai_voice_confirm_btn) && lv_obj_has_flag(ai_voice_confirm_btn, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(ai_voice_confirm_btn, LV_OBJ_FLAG_HIDDEN);
    }
    if (lv_obj_is_valid(ai_voice_cancel_btn) && lv_obj_has_flag(ai_voice_cancel_btn, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(ai_voice_cancel_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

void voice_hide_button(void)
{
    if (lv_obj_is_valid(ai_voice_confirm_btn) && !lv_obj_has_flag(ai_voice_confirm_btn, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(ai_voice_confirm_btn, LV_OBJ_FLAG_HIDDEN);
    }
    if (lv_obj_is_valid(ai_voice_cancel_btn) && !lv_obj_has_flag(ai_voice_cancel_btn, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(ai_voice_cancel_btn, LV_OBJ_FLAG_HIDDEN);
    }
}

void voice_text_set(char *text)
{
    if(voice_text_label != NULL) {
        lv_label_set_text(voice_text_label, text);
    }
}
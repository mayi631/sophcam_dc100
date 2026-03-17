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
#include "image_process.h"

lv_obj_t *AISelect_scroll_cont_s = NULL;      // 特效滚动控件
// 全局变量
int g_current_selected_index = 0;

void delete_aiselect_scroll(void)
{
    if(AISelect_scroll_cont_s != NULL && lv_obj_is_valid(AISelect_scroll_cont_s)) {
        lv_obj_del(AISelect_scroll_cont_s);
    }
    AISelect_scroll_cont_s = NULL;
}

lv_obj_t *get_aiselete_scroll_handl(void)
{
    return AISelect_scroll_cont_s;
}

// 创建选择浮窗
void photoAISelect_listCreat(lv_obj_t *parent,lv_event_cb_t event_cb)
{
    if(parent == NULL) return;

    uint8_t str_len                = 0;
    const char *scene_btn_labels[] = {str_language_3d_wind[get_curr_language()],     str_language_realistic_style[get_curr_language()],   str_language_angel_style[get_curr_language()],        str_language_anime_style[get_curr_language()],   str_language_japanese_manga_style[get_curr_language()],   str_language_princess_style[get_curr_language()],
                                      str_language_dreamy_style[get_curr_language()],   str_language_ink_wash_style[get_curr_language()],   str_language_new_monet_garden[get_curr_language()],    str_language_watercolor_style[get_curr_language()],   str_language_monet_garden[get_curr_language()], str_language_exquisite_american_comics[get_curr_language()],
                                      str_language_cyber_mechanical[get_curr_language()], str_language_exquisite_korean_comics[get_curr_language()], str_language_chinese_style_ink_wash[get_curr_language()],     str_language_romantic_light_and_shadow[get_curr_language()], str_language_porcelain_doll[get_curr_language()], str_language_chinese_red[get_curr_language()],
                                      str_language_ugly_cute_clay[get_curr_language()], str_language_cute_doll[get_curr_language()], str_language_3d_game_z_era[get_curr_language()], str_language_animated_film[get_curr_language()], str_language_doll[get_curr_language()],str_language_youth[get_curr_language()], str_language_middle_age[get_curr_language()], str_language_old_age[get_curr_language()]};

    static const char *style_image_files[sizeof(scene_btn_labels) / sizeof(scene_btn_labels[0])] = {
        "style_3D.png",         "xieshi_style.png",   "tianshi_style.png",
        "dongman_style.png",    "riman_style.png",    "gongzhu_style.png",
        "menghuan_style.png",   "shuimo.png",         "xin_monai_huayuan.png",
        "shuicai.png",          "monaihuayuan.png",   "jingzhi_meiman.png",
        "saibojixie.png",       "jingzhi_hanman.png", "guofeng_shuimo.png",
        "lanmanguangying.png",  "taoci.png",          "zhongguo_red.png",
        "choumeng.png",         "keai_wanou.png",     "gama_3D_Z.png",
        "donghua_dianying.png", "wanou.png","qing_nian.png", "zhon_nian.png", "old.png"};

    const char *bg_btn_labels[] = {str_language_great_wall_sunset[get_curr_language()], str_language_valley[get_curr_language()], str_language_autumn_leaves[get_curr_language()], str_language_winter_snow_scene[get_curr_language()], str_language_on_the_playground[get_curr_language()], str_language_on_the_beach[get_curr_language()]};
    static const char *bg_image_files[sizeof(bg_btn_labels) / sizeof(bg_btn_labels[0])] = {
        "长城落日.png", "山谷.png", "秋季落叶.png", "冬季雪景.png", "操场上.png", "沙滩上.png"};

    switch(AIModeSelect_GetMode()) {
        case AI_SCENE_CHANGE: str_len = sizeof(scene_btn_labels) / sizeof(scene_btn_labels[0]); break;
        case AI_BG_CHANGE: str_len = sizeof(bg_btn_labels) / sizeof(bg_btn_labels[0]); break;
        // case AI_AGE_CHANGE: str_len = sizeof(age_btn_labels) / sizeof(age_btn_labels[0]); break;
        case AI_VOICE_CUSTOM: str_len = AI_VOICE_CUSTOM/AI_VOICE_CUSTOM; break;
        default: str_len = AI_BEAUTY; break;
    }

    if(str_len == AI_BEAUTY) {
        delete_aiselect_scroll();
        return;
    }

    // 创建滚动容器
    AISelect_scroll_cont_s = lv_obj_create(parent);
    lv_obj_set_size(AISelect_scroll_cont_s, 440, 120);
    lv_obj_align(AISelect_scroll_cont_s, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(AISelect_scroll_cont_s, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(AISelect_scroll_cont_s, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(AISelect_scroll_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(AISelect_scroll_cont_s, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(AISelect_scroll_cont_s, 0, 0);
    lv_obj_set_scroll_dir(AISelect_scroll_cont_s, LV_DIR_HOR);

    // 在容器内部创建多个对象
    for(int i = 0; i < str_len; i++) {
        // 创建设置选项容器
        lv_obj_t *btn = lv_btn_create(AISelect_scroll_cont_s);
        lv_obj_set_size(btn, 70, 100);
        lv_obj_align(btn, LV_ALIGN_TOP_LEFT, (70 + 10) * i, 0);
        if(AI_BG_CHANGE == AIModeSelect_GetMode()) {
            lv_obj_align(btn, LV_ALIGN_TOP_MID, -160 + (70 + 10) * i, 0);
        } else if(AI_VOICE_CUSTOM == AIModeSelect_GetMode()) {
            lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 0);
            lv_obj_set_size(btn, 100, 100);
        }
        lv_obj_set_style_pad_all(btn, 0, LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, NULL);
        lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN | LV_STATE_DEFAULT);
        if(i == g_current_selected_index) {
            lv_obj_add_state(btn, LV_STATE_CHECKED);
            lv_obj_set_style_border_width(btn, 2, LV_STATE_CHECKED);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
            lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_STATE_CHECKED);
            lv_obj_scroll_to_view(btn, LV_ANIM_ON);
        }

        lv_obj_t *img = lv_img_create(btn);
        lv_obj_set_size(img, 58, 60);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_pad_all(img, 0, LV_STATE_DEFAULT);
        // 设置图片不可点击，确保事件传递给父按钮
        lv_obj_clear_flag(img, LV_OBJ_FLAG_CLICKABLE);

        if(AI_SCENE_CHANGE == AIModeSelect_GetMode()) {
            show_image(img, style_image_files[i]);
        } else if(AI_BG_CHANGE == AIModeSelect_GetMode()) {
            show_image(img, bg_image_files[i]);
        } else if(AI_VOICE_CUSTOM == AIModeSelect_GetMode()) {
            show_image(img, "语音输入中.png");
        }

        lv_obj_t *label = lv_label_create(btn);
        lv_obj_set_size(label, 70, 32);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 16), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        // 设置标签不可点击，确保事件传递给父按钮
        lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICKABLE);

        if(AI_SCENE_CHANGE == AIModeSelect_GetMode()) {
            lv_label_set_text_fmt(label, "%s", scene_btn_labels[i]);
        } else if(AI_BG_CHANGE == AIModeSelect_GetMode()) {
            lv_label_set_text_fmt(label, "%s", bg_btn_labels[i]);
        } else if(AI_VOICE_CUSTOM == AIModeSelect_GetMode()) {
            lv_label_set_text_fmt(label, "%s", str_language_hold_to_speak[get_curr_language()]);
            lv_obj_set_size(label, 100, 32);
        }
    }
}
/**
 * @brief AI选择上一个按钮（向左选择）
 */
void AISelect_prev(void)
{
    if(AISelect_scroll_cont_s == NULL || !lv_obj_is_valid(AISelect_scroll_cont_s)) {
        return;
    }
    
    int btn_count = lv_obj_get_child_cnt(AISelect_scroll_cont_s);
    if(btn_count == 0) return;
    
    // 清除上一个按钮的选中样式
    lv_obj_t *prev_btn = lv_obj_get_child(AISelect_scroll_cont_s, g_current_selected_index);
    if(prev_btn) {
        lv_obj_clear_state(prev_btn, LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(prev_btn, LV_OPA_0, LV_PART_MAIN);
    }
    
    // 计算新索引
    g_current_selected_index--;
    if(g_current_selected_index < 0) {
        g_current_selected_index = btn_count - 1;
    }
    
    // 设置新按钮的选中样式
    lv_obj_t *new_btn = lv_obj_get_child(AISelect_scroll_cont_s, g_current_selected_index);
    if(new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(new_btn, LV_OPA_0, LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
}

/**
 * @brief AI选择下一个按钮（向右选择）
 */
void AISelect_next(void)
{
    if(AISelect_scroll_cont_s == NULL || !lv_obj_is_valid(AISelect_scroll_cont_s)) {
        return;
    }
    
    int btn_count = lv_obj_get_child_cnt(AISelect_scroll_cont_s);
    if(btn_count == 0) return;
    
    // 清除上一个按钮的选中样式
    lv_obj_t *prev_btn = lv_obj_get_child(AISelect_scroll_cont_s, g_current_selected_index);
    if(prev_btn) {
        lv_obj_clear_state(prev_btn, LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(prev_btn, LV_OPA_0, LV_PART_MAIN);
    }
    
    // 计算新索引
    g_current_selected_index++;
    if(g_current_selected_index >= btn_count) {
        g_current_selected_index = 0;
    }
    
    // 设置新按钮的选中样式
    lv_obj_t *new_btn = lv_obj_get_child(AISelect_scroll_cont_s, g_current_selected_index);
    if(new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(new_btn, LV_OPA_0, LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
}

void set_currIndex_focus(int index)
{
    if(AISelect_scroll_cont_s == NULL || !lv_obj_is_valid(AISelect_scroll_cont_s)) {
        return;
    }
    g_current_selected_index = index;
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(AISelect_scroll_cont_s); i++) {
        lv_obj_t *btn = lv_obj_get_child(AISelect_scroll_cont_s, i);
        if(btn != NULL) lv_obj_clear_state(btn, LV_STATE_CHECKED);
    }
    lv_obj_t *new_btn = lv_obj_get_child(AISelect_scroll_cont_s, g_current_selected_index);
    if(new_btn) {
        lv_obj_add_state(new_btn, LV_STATE_CHECKED);
        lv_obj_set_style_border_width(new_btn, 2, LV_STATE_CHECKED);
        lv_obj_set_style_border_color(new_btn, lv_color_hex(0x007ACC), LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa(new_btn, LV_OPA_0, LV_STATE_CHECKED);
        lv_obj_scroll_to_view(new_btn, LV_ANIM_ON);
    }
}

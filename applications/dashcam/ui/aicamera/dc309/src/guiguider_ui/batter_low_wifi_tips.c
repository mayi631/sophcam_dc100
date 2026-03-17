#define DEBUG
/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include "page_all.h"


static lv_obj_t *mbox_s = NULL;//低电量

void delete_batter_tips_mbox(void)
{
    if (mbox_s != NULL) { // AI处理，低电量提示
        lv_obj_del(mbox_s);
        mbox_s = NULL;
    }
}

static void batter_tips_wifi_event_cb(lv_event_t *e)
{
    lv_msgbox_close(mbox_s);
    mbox_s = NULL;
}

// 电量检查函数
bool check_battery_for_wifi(lv_obj_t *parent)
{
    extern int32_t g_batter_image_index;
    // 电量等级1（0%-10%）时不允许打开WiFi
    if (g_batter_image_index == 1) {
        // 显示电量不足提示
        if (!lv_obj_is_valid(mbox_s)) {
            mbox_s = lv_msgbox_create(parent);
        } else {
            return false;
        }
        lv_obj_set_size(mbox_s, 640 / 3, 480 / 3);
        lv_msgbox_add_text(mbox_s, "电量过低");
        lv_obj_set_style_text_color(mbox_s, lv_color_hex(0xFF0000), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(mbox_s, LV_OPA_50, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_font(mbox_s, get_usr_fonts(ALI_PUHUITI_FONTPATH, 24),
            LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(mbox_s, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t* close_btn = lv_msgbox_add_footer_button(mbox_s, str_language_off[get_curr_language()]);
        lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_t* confire_btn = lv_msgbox_add_footer_button(mbox_s, str_language_confirm[get_curr_language()]);
        lv_obj_set_style_bg_color(confire_btn, lv_color_hex(0x171717), LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_add_event_cb(close_btn, batter_tips_wifi_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(confire_btn, batter_tips_wifi_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_align(mbox_s, LV_ALIGN_CENTER, 0, 8); // 底部居中位置
        return false;
    }

    return true;
}
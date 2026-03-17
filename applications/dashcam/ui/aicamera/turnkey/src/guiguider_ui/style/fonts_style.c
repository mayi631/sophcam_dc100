#include <stdio.h>
#include "config.h"
#include "custom.h"
#include "lvgl.h"

/* 预定义几种字体样式
 * Usage: lv_obj_add_style(obj, &ttf_font_12, LV_PART_MAIN | LV_STATE_DEFAULT);
 */
lv_style_t ttf_font_12;
lv_style_t ttf_font_14;
lv_style_t ttf_font_16;
lv_style_t ttf_font_18;
lv_style_t ttf_font_20;
lv_style_t ttf_font_22;
lv_style_t ttf_font_24;
lv_style_t ttf_font_30;

void init_fonts_style(void)
{
    // 初始化字体样式对象
    lv_style_t *style_list[] = {&ttf_font_12, &ttf_font_14, &ttf_font_16, &ttf_font_18,
                                &ttf_font_20, &ttf_font_22, &ttf_font_24, &ttf_font_30};
    for(size_t i = 0; i < sizeof(style_list) / sizeof(style_list[0]); i++) {
        lv_style_init(style_list[i]);
        lv_style_set_opa(style_list[i], 255);
        lv_style_set_text_color(style_list[i], lv_color_white());
        lv_style_set_text_opa(style_list[i], 255);
        lv_style_set_text_align(style_list[i], LV_TEXT_ALIGN_CENTER);
    }

    set_chs_fonts(ALI_PUHUITI_FONTPATH, 12, &ttf_font_12);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 14, &ttf_font_14);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 16, &ttf_font_16);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 18, &ttf_font_18);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 20, &ttf_font_20);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 22, &ttf_font_22);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 24, &ttf_font_24);
    set_chs_fonts(ALI_PUHUITI_FONTPATH, 30, &ttf_font_30);
}

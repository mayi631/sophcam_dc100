#include "config.h"
#include "custom.h"
#include "lvgl.h"
#include <stdio.h>

// #############################################################################
// ! #region 公共页面样式定义
// #############################################################################

// cont_top 顶部栏容器样式
lv_style_t style_common_cont_top;

// 页面主容器背景样式（黑色背景）
lv_style_t style_common_main_bg;

// btn_back 返回按钮样式
lv_style_t style_common_btn_back;

// 返回按钮标签样式（白色文字，SourceHanSerifSC_30字体）
lv_style_t style_common_label_back;

// 焦点样式（橙色轮廓）
lv_style_t style_focus_blue;
//防止设置选项的容器
lv_style_t style_common_cont_setting;

void init_common_style(void)
{
    // style_common_cont_top
    lv_style_init(&style_common_cont_top);
    lv_style_set_border_width(&style_common_cont_top, 0); // 无边框
    lv_style_set_radius(&style_common_cont_top, 0); // 无圆角
    lv_style_set_bg_opa(&style_common_cont_top, 255); // 不透明背景
    lv_style_set_bg_color(&style_common_cont_top, lv_color_hex(0x020524)); // 淡蓝色背景
    lv_style_set_bg_grad_dir(&style_common_cont_top, LV_GRAD_DIR_NONE); // 无渐变
    lv_style_set_pad_top(&style_common_cont_top, 0); // 无内边距
    lv_style_set_pad_bottom(&style_common_cont_top, 0); // 无内边距
    lv_style_set_pad_left(&style_common_cont_top, 0); // 无内边距
    lv_style_set_pad_right(&style_common_cont_top, 0); // 无内边距
    lv_style_set_shadow_width(&style_common_cont_top, 0); // 无阴影

    // style_common_main_bg
    lv_style_init(&style_common_main_bg);
    lv_style_set_bg_opa(&style_common_main_bg, LV_OPA_COVER); // 不透明背景
    lv_style_set_bg_color(&style_common_main_bg, lv_color_hex(0x020524)); // 淡蓝色背景

    // style_common_btn_back
    lv_style_init(&style_common_btn_back);
    lv_style_set_bg_opa(&style_common_btn_back, 255); // 不透明背景
    lv_style_set_bg_color(&style_common_btn_back, lv_color_hex(0x020524)); // 背景色
    lv_style_set_border_width(&style_common_btn_back, 0); // 无边框
    lv_style_set_radius(&style_common_btn_back, 20); // 圆角20
    lv_style_set_shadow_width(&style_common_btn_back, 0); // 无阴影
    lv_style_set_text_color(&style_common_btn_back, lv_color_hex(0x1A1A1A)); // 文本颜色
    lv_style_set_text_font(&style_common_btn_back, &lv_font_montserratMedium_13); // 字体
    lv_style_set_text_opa(&style_common_btn_back, 255); // 文本不透明
    lv_style_set_text_align(&style_common_btn_back, LV_TEXT_ALIGN_CENTER); // 文本居中
    lv_style_set_pad_top(&style_common_btn_back, 0); // 无内边距
    lv_style_set_pad_bottom(&style_common_btn_back, 0); // 无内边距
    lv_style_set_pad_left(&style_common_btn_back, 0); // 无内边距
    lv_style_set_pad_right(&style_common_btn_back, 0); // 无内边距

    // style_common_label_back
    lv_style_init(&style_common_label_back);
    lv_style_set_text_color(&style_common_label_back, lv_color_hex(0xFFFFFF)); // 白色文字
    lv_style_set_text_font(&style_common_label_back, &lv_font_SourceHanSerifSC_Regular_30); // 字体
    lv_style_set_text_opa(&style_common_label_back, 255); // 文本不透明
    lv_style_set_text_align(&style_common_label_back, LV_TEXT_ALIGN_CENTER); // 文本居中

    // style_focus_blue
    lv_style_init(&style_focus_blue);
    lv_style_set_outline_color(&style_focus_blue, lv_color_hex(0x035edb)); // 蓝色轮廓
    lv_style_set_outline_opa(&style_focus_blue, LV_OPA_COVER); // 轮廓不透明


    lv_style_init(&style_common_cont_setting);
    lv_style_set_bg_opa(&style_common_cont_setting, LV_OPA_0); // 透明背景
    lv_style_set_shadow_width(&style_common_cont_setting, 0);
    lv_style_set_border_width(&style_common_cont_setting, 0);
    lv_style_set_pad_top(&style_common_cont_setting, 10); 
    lv_style_set_pad_bottom(&style_common_cont_setting, 10); 
    lv_style_set_pad_left(&style_common_cont_setting, 10); 
    lv_style_set_pad_right(&style_common_cont_setting, 10); 
}

// #endregion

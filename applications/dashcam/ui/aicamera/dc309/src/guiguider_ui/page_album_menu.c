#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
// #include "events_init.h"
#include "config.h"
#include "custom.h"
#include "page_all.h"

lv_obj_t *obj_Album_Menu_s;

void albumMenu_DeleteMenu_Create(lv_obj_t *parent)
{}

void albumMenu_Create(lv_obj_t *parent, uint8_t index)
{

    obj_Album_Menu_s = lv_obj_create(parent);
    lv_obj_set_size(obj_Album_Menu_s, H_RES, V_RES);
    lv_obj_set_scrollbar_mode(obj_Album_Menu_s, LV_SCROLLBAR_MODE_OFF); // 禁用滚动条，默认 LV_SCROLLBAR_MODE_AUTO
    lv_obj_set_style_bg_opa(obj_Album_Menu_s, 255, LV_PART_MAIN | LV_STATE_DEFAULT); // 不透明度 0~255，255 完全不透明
    lv_obj_set_style_bg_color(obj_Album_Menu_s, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT); // 背景颜色
    lv_obj_set_style_bg_grad_dir(obj_Album_Menu_s, LV_GRAD_DIR_NONE, LV_PART_MAIN | LV_STATE_DEFAULT); // 无渐变色
}

#ifndef __PAGE_BRIGHTNESS_H__
#define __PAGE_BRIGHTNESS_H__

#include "lvgl.h"
#include "gui_guider.h"

// 亮度等级结构
typedef struct {
    int level;         // 亮度等级 (1-7)
    int value;         // 对应亮度值 (0-255)
    const char *label; // 等级标签
} brightness_level_t;
extern lv_obj_t *brightness_scr;
// 亮度等级数组
#define BRIGHTNESS_LEVEL_COUNT 7
extern brightness_level_t brightness_levels[BRIGHTNESS_LEVEL_COUNT];

// 函数声明
void sysMenu_brightness(lv_ui_t *ui);
void brightness_set_level(int level);
uint8_t get_curr_brightness(void);
void setsysMenu_brightness_Index(uint8_t index);
void setsysMenu_brightness_Label(char* plabel);
// int brightness_get_current_value(void);

#endif // __PAGE_BRIGHTNESS_H__
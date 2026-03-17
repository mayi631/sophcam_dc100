#define DEBUG
#include "lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui_guider.h"
#include "page_all.h"
#include "custom.h"
#include "config.h"
#include "mlog.h"
#include "ui_common.h"

#include "image_recognize/image_recognize.h"
#include "common/extract_thumbnail.h"
#include "common/takephoto.h"
#include "indev.h"
#include <linux/input.h>
#include "mapi_ao.h"
//音量设置变量
static lv_obj_t *value_label = NULL;  // 全局变量用于存储值标签
static lv_obj_t *voice_set_bar = NULL;
// 长按zoomin按键检测相关变量
static lv_timer_t *zoom_in_long_press_timer = NULL;
static bool zoomin_long_press_flag         = false;
// 长按zoomout按键检测相关变量
static lv_timer_t *zoom_out_long_press_timer = NULL;
static bool zoomout_long_press_flag         = false;


#define MAX_VOLUME 32
#define HALF_MAX_VOLUME (MAX_VOLUME / 2)
// 值改变事件回调函数
static void value_changed_cb(lv_event_t *e)
{
    lv_obj_t *bar = lv_event_get_target(e);
    if(value_label && lv_obj_is_valid(value_label)) {
        lv_label_set_text_fmt(value_label, "%d%%", lv_bar_get_value(bar));
    }
}

void volume_add(void)
{
    PARAM_MEDIA_COMM_S media_comm = { 0 };
    static int32_t volume_level = 0;
    MAPI_AO_GetVolume(MEDIA_GetCtx()->SysHandle.aohdl, &volume_level);
    volume_level++;
    if(volume_level > HALF_MAX_VOLUME) {
        volume_level = HALF_MAX_VOLUME;
    }
    MAPI_AO_SetVolume(MEDIA_GetCtx()->SysHandle.aohdl, volume_level);
    /* 保存音量 */
    PARAM_GetMediaComm(&media_comm);
    media_comm.Ao.volume = volume_level;
    PARAM_SetMediaComm(&media_comm);
    lv_bar_set_value(voice_set_bar, volume_level, LV_ANIM_ON);
    lv_label_set_text_fmt(value_label, "%d%%", (lv_bar_get_value(voice_set_bar) * 100) / HALF_MAX_VOLUME);
}

void volume_reduce(void)
{
    PARAM_MEDIA_COMM_S media_comm = { 0 };
    static int32_t volume_level = 0;
    MAPI_AO_GetVolume(MEDIA_GetCtx()->SysHandle.aohdl, &volume_level);
    volume_level--;
    if(volume_level < 0) {
        volume_level = 0;
    }
    MAPI_AO_SetVolume(MEDIA_GetCtx()->SysHandle.aohdl, volume_level);
    /* 保存音量 */
    PARAM_GetMediaComm(&media_comm);
    media_comm.Ao.volume = volume_level;
    PARAM_SetMediaComm(&media_comm);
    lv_bar_set_value(voice_set_bar, volume_level, LV_ANIM_ON);
    lv_label_set_text_fmt(value_label, "%d%%", (lv_bar_get_value(voice_set_bar) * 100) / HALF_MAX_VOLUME);
}

static void custom_bar_event_cb(lv_event_t* e)
{
    PARAM_MEDIA_COMM_S media_comm = { 0 };
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t* bar = lv_event_get_target(e);

    if(code == LV_EVENT_CLICKED) {
        // 获取点击位置
        lv_point_t p;
        lv_indev_get_point(lv_indev_active(), &p);

        // 转换为相对于进度条的位置
        lv_area_t bar_coords;
        lv_obj_get_coords(bar, &bar_coords);
        lv_coord_t click_y = p.y - bar_coords.y1;

        // 计算进度值（0-HALF_MAX_VOLUME
        lv_coord_t bar_height = lv_obj_get_height(bar);
        int value = HALF_MAX_VOLUME - (click_y * HALF_MAX_VOLUME / bar_height);

        // 限制在0-HALF_MAX_VOLUME
        value = value < 0 ? 0 : (value > HALF_MAX_VOLUME ? HALF_MAX_VOLUME : value);

        // 设置进度条的值
        lv_bar_set_value(bar, value, LV_ANIM_ON);
        lv_label_set_text_fmt(value_label, "%d%%", (lv_bar_get_value(bar) * 100) / HALF_MAX_VOLUME);

        MAPI_AO_SetVolume(MEDIA_GetCtx()->SysHandle.aohdl, value);
        /* 保存音量 */
        PARAM_GetMediaComm(&media_comm);
        media_comm.Ao.volume = value;
        PARAM_SetMediaComm(&media_comm);

    } else if (code == LV_EVENT_PRESSING) {
        // 持续按压事件，处理滑动
        lv_point_t p;
        lv_indev_get_point(lv_indev_active(), &p);

        // 转换为相对于进度条的位置
        lv_area_t bar_coords;
        lv_obj_get_coords(bar, &bar_coords);
        lv_coord_t click_y = p.y - bar_coords.y1;

        // 计算进度值（0-100）
        lv_coord_t bar_height = lv_obj_get_height(bar);
        int value = HALF_MAX_VOLUME - (click_y * HALF_MAX_VOLUME / bar_height);

        // 限制在0-100范围内
        value = value < 0 ? 0 : (value > HALF_MAX_VOLUME ? HALF_MAX_VOLUME : value);

        // 设置进度条的值
        lv_bar_set_value(bar, value, LV_ANIM_OFF); // 滑动时不用动画，更跟手
        lv_label_set_text_fmt(value_label, "%d%%", (lv_bar_get_value(bar) * 100) / HALF_MAX_VOLUME);
        MAPI_AO_SetVolume(MEDIA_GetCtx()->SysHandle.aohdl, value);
        /* 保存音量 */
        PARAM_GetMediaComm(&media_comm);
        media_comm.Ao.volume = value;
        PARAM_SetMediaComm(&media_comm);
    }
}

void voice_setting_arc_create(void)
{
    static int32_t volume_level = 0;
    MAPI_AO_GetVolume(MEDIA_GetCtx()->SysHandle.aohdl, &volume_level);
    // 创建自定义的垂直进度条
    voice_set_bar = lv_bar_create(lv_scr_act());
    lv_obj_set_size(voice_set_bar, 50, 200);
    lv_obj_align(voice_set_bar, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_bar_set_range(voice_set_bar, 0, HALF_MAX_VOLUME);
    lv_bar_set_value(voice_set_bar,volume_level, LV_ANIM_OFF); // 设置初始值

    // 设置进度条样式
    lv_obj_set_style_bg_color(voice_set_bar, lv_color_hex(0x4a4a4a), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(voice_set_bar, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(voice_set_bar, 15, LV_PART_MAIN);

    // 设置进度条指示器样式
    lv_obj_set_style_bg_color(voice_set_bar, lv_color_hex(0x4da6ff), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(voice_set_bar, LV_OPA_30, LV_PART_INDICATOR);
    lv_obj_set_style_radius(voice_set_bar, 15, LV_PART_INDICATOR);

    // 设置为可点击和可拖动
    lv_obj_add_flag(voice_set_bar, LV_OBJ_FLAG_CLICKABLE);

    // 注册事件处理函数
    lv_obj_add_event_cb(voice_set_bar, custom_bar_event_cb, LV_EVENT_ALL, NULL);

    // 添加一个标签显示当前值
    value_label = lv_label_create(lv_scr_act());
    lv_obj_align_to(value_label, voice_set_bar, LV_ALIGN_OUT_BOTTOM_MID, 4, 10);
    lv_label_set_text_fmt(value_label, "%d%%",(lv_bar_get_value(voice_set_bar) * 100) / HALF_MAX_VOLUME );
    lv_obj_set_style_text_color(value_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(value_label, get_usr_fonts(ALI_PUHUITI_FONTPATH, 20), 0);

    // 添加值改变事件回调
    lv_obj_add_event_cb(voice_set_bar, value_changed_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

lv_obj_t *get_arc_handel(void)
{
    return voice_set_bar;
}

void voice_arc_delete(void)
{
    if(voice_set_bar != NULL) {
        lv_obj_del(value_label);
        value_label = NULL;
        lv_obj_del(voice_set_bar);
        voice_set_bar           = NULL;
    }
}

// 长zoomin按键检测定时器回调函数
static void zoomin_long_press_timer_cb(lv_timer_t *t)
{
    // 定时器触发说明按键已经持续按下300ms，执行长按逻辑
    zoomin_long_press_flag = true;
    // 100ms周期继续检测, 直到按键松开
    lv_timer_set_period(t, 100);
    lv_timer_reset(t);

    voice_arc_delete();
    voice_setting_arc_create();
    volume_add();

    if(!lv_obj_is_valid(voice_set_bar)) {
        if(zoom_in_long_press_timer != NULL) {
            lv_timer_del(zoom_in_long_press_timer);
            zoom_in_long_press_timer = NULL;
            zoomin_long_press_flag = false;
        }
    }
}

int32_t do_zoomin(int32_t key_value)
{
    if(key_value == 1) {
        // 创建长按检测定时器, 300ms认为是长按
        zoomin_long_press_flag  = false;
        zoom_in_long_press_timer = lv_timer_create(zoomin_long_press_timer_cb, 300, NULL);
    } else {
        // zoomin(T)按键释放
        // 短按或者长按，在按键释放的时候都要删除定时器
        if(zoom_in_long_press_timer != NULL) {
            lv_timer_del(zoom_in_long_press_timer);
            zoom_in_long_press_timer = NULL;
        }
        // 短按触发一次放大
        if(!zoomin_long_press_flag) {
            MLOG_DBG("zoomin按键短按, 执行短按逻辑\n");
            voice_arc_delete();
            voice_setting_arc_create();
            volume_add();
        }
    }
    return 0;
}

// 长zoomout按键检测定时器回调函数
static void zoomout_long_press_timer_cb(lv_timer_t *t)
{
    // 定时器触发说明按键已经持续按下300ms，执行长按逻辑
    zoomout_long_press_flag = true;
    // 100ms周期继续检测, 直到按键松开
    lv_timer_set_period(t, 100);
    lv_timer_reset(t);

    voice_arc_delete();
    voice_setting_arc_create();
    volume_reduce();

    if(!lv_obj_is_valid(voice_set_bar)) {
        if(zoom_out_long_press_timer != NULL) {
            lv_timer_del(zoom_out_long_press_timer);
            zoom_out_long_press_timer = NULL;
            zoomout_long_press_flag = false;
        }
    }
}

int32_t do_zoomout(int32_t key_value)
{
    if(key_value == 1) {
        // 创建长按检测定时器, 300ms认为是长按
        zoomout_long_press_flag  = false;
        zoom_out_long_press_timer = lv_timer_create(zoomout_long_press_timer_cb, 300, NULL);
    } else {
        // zoomin(T)按键释放
        // 短按或者长按，在按键释放的时候都要删除定时器
        if(zoom_out_long_press_timer != NULL) {
            lv_timer_del(zoom_out_long_press_timer);
            zoom_out_long_press_timer = NULL;
        }
        // 短按触发一次缩小
        if(!zoomout_long_press_flag) {
            MLOG_DBG("zoomout按键短按, 执行短按逻辑\n");
            voice_arc_delete();
            voice_setting_arc_create();
            volume_reduce();
        }
    }
    return 0;
}
/*
 * Copyright 2024 NXP
 * NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
 * accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
 * terms, then you may not retain, install, activate or otherwise use the software.
 */
// #define DEBUG
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
#include "voiceplay.h"
#include "common/takephoto.h"

timer_manager_t timers;
lv_font_t* font_chs = NULL;
extern int32_t g_batter_image_index;
extern uint8_t is_start_video;     //录像状态
extern const char *batter_image_big[];

static void sys_time_update(lv_timer_t* timer)
{
    lv_ui_t *ui  = (lv_ui_t *)lv_timer_get_user_data(timer);
    time_t now   = time(NULL);
    struct tm *t = localtime(&now);

    // 更新日期
    lv_label_set_text_fmt(ui->date_text, "%04d/%02d/%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    // 更新时间
    lv_label_set_text_fmt(ui->digital_clock, "%02d:%02d", t->tm_hour, t->tm_min);
}

static void ui_dyna_getparam_update(lv_timer_t *timer)
{
    // 缩时录像逻辑
    check_and_start_timelapse();

    // 低电量逻辑
    static uint8_t times_count = 0;
    if(get_lowbatter_tips_flag()) {
        if(times_count == 0) {
            show_battery_warning();
            /* 如果现在处于录像，则关闭录像 */
            if(is_start_video == VEDIO_START) {
                is_start_video = VEDIO_STOP;
                MESSAGE_S Msg = {0};
                Msg.topic = EVENT_MODEMNG_STOP_REC;
                MODEMNG_SendMessage(&Msg);
                /* 停止录像，恢复TP事件 */
                enable_touch_events();
                /* 停止录像时，开启音频输出 */
                UI_VOICEPLAY_Init();
            }
        }
        if(times_count++ >= 2) { // 显示2秒后关机
            times_count = 0;
            MAPI_AO_HANDLE_T ao_hdl = NULL;
            int ret                 = MAPI_SUCCESS;
            set_lowbatter_tips_flag(false);
            ret = MAPI_AO_GetHandle(&ao_hdl);
            if(ret == MAPI_SUCCESS) {
                MAPI_AO_SetAmplifier(ao_hdl, CVI_FALSE);
            }
            system("poweroff");
        }
    } else {
        times_count = 0;
    }
}
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */
static void wifi_update_cb(lv_timer_t *t)
{
    lv_ui_t *ui    = (lv_ui_t *)lv_timer_get_user_data(t);
    int dbm        = Hal_Wpa_GetConnectSignal();
    // 转换 dBm 到 0-4 等级
    int wifi_level = 0;
    if(dbm == -1) wifi_level = 0;
    else if (dbm >= -50) wifi_level = 4;  // >=-50: 满格
    else if (dbm >= -60) wifi_level = 3;  // -60~-50: 3格
    else if (dbm >= -70) wifi_level = 2;  // -70~-60: 2格
    else if (dbm >= -80) wifi_level = 1;  // -80~-70: 1格
    else wifi_level = 0;
    // MLOG_DBG("dBm=%d → level=%d\n", dbm, wifi_level);

    if(wifi_level == 0) {
        lv_obj_add_flag(ui->img_wifi, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui->img_wifi, LV_OBJ_FLAG_HIDDEN);
        lv_img_set_src(ui->img_wifi, wifi_level == 1   ? &_wifi_1_RGB565A8_24x23
                                     : wifi_level == 2 ? &_wifi_2_RGB565A8_24x23
                                     : wifi_level == 3 ? &_wifi_3_RGB565A8_24x23
                                                       : &_wifi_4_RGB565A8_24x23);
    }
}

static void power_update_cb(lv_timer_t *t)
{
    lv_ui_t *ui = (lv_ui_t *)lv_timer_get_user_data(t);
    if(g_batter_image_index == 0) {
        show_image(ui->img_power, batter_image_big[g_batter_image_index]);
    } else {
        lv_obj_clear_flag(ui->img_power, LV_OBJ_FLAG_HIDDEN);
        show_image(ui->img_power, batter_image_big[g_batter_image_index]);
    }
}

void custom_init(lv_ui_t *ui)
{
    /* Add your codes here */
    /* 初始化定时器管理器 */
    memset(&timers, 0, sizeof(timer_manager_t));

    // 创建共享的顶部状态栏对象
    ui->status_bar = lv_obj_create(lv_layer_top()); // 创建在顶层对象上
    lv_obj_set_size(ui->status_bar, 640, 40);
    lv_obj_set_pos(ui->status_bar, 0, 0);
    // lv_obj_set_style_bg_color(ui->status_bar, lv_color_hex(0xFFD400), LV_PART_MAIN | LV_STATE_DEFAULT); // 明亮的黄色
    lv_obj_set_style_bg_opa(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT); // 完全不透明
    lv_obj_set_style_border_width(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 设置状态栏为浮动对象，这样它就会始终显示在最上层
    lv_obj_add_flag(ui->status_bar, LV_OBJ_FLAG_FLOATING);

    // 创建共享的日期文本
    // ui->date_text = lv_label_create(ui->status_bar);
    // lv_obj_align(ui->date_text, LV_ALIGN_LEFT_MID, 0, 0);
    // lv_obj_set_style_text_color(ui->date_text, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 保持黑色
    // // lv_obj_set_style_text_font(ui->date_text, &lv_font_montserratMedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_font(ui->date_text, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    // // 创建共享的数字时钟
    // ui->digital_clock = lv_label_create(ui->status_bar);
    // lv_obj_align(ui->digital_clock, LV_ALIGN_CENTER, 6, 0);
    // lv_obj_set_size(ui->digital_clock, 80, 23); // 增加宽度以确保时间显示在一行
    // lv_obj_set_style_text_color(ui->digital_clock, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 保持黑色
    // lv_obj_set_style_text_font(ui->digital_clock, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_text_align(ui->digital_clock, LV_TEXT_ALIGN_LEFT,
    //                             LV_PART_MAIN | LV_STATE_DEFAULT); // 确保文本左对齐



    // 创建共享的WiFi图标
    ui->img_wifi = lv_img_create(ui->status_bar);
    lv_obj_align(ui->img_wifi, LV_ALIGN_LEFT_MID, 538, -4);
    lv_obj_set_size(ui->img_wifi, 24, 23);
    lv_obj_add_flag(ui->img_wifi, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_pivot(ui->img_wifi, 50, 50);
    lv_image_set_rotation(ui->img_wifi, 0);
    // lv_obj_set_style_image_recolor_opa(ui->img_wifi, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_image_opa(ui->img_wifi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_image_recolor(ui->img_wifi, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_image_recolor_opa(ui->img_wifi, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_font(ui->img_wifi, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建共享的电源图标
    ui->img_power = lv_img_create(ui->status_bar);
    lv_obj_align(ui->img_power, LV_ALIGN_LEFT_MID, 583, -4);
    lv_obj_set_size(ui->img_power, 38, 32);
    lv_obj_add_flag(ui->img_power, LV_OBJ_FLAG_CLICKABLE);
    show_image(ui->img_power, batter_image_big[g_batter_image_index]);
    // lv_obj_set_style_text_color(ui->img_power, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT); // 保持黑色
    // lv_obj_set_style_text_font(ui->img_power, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建定时器
    timers.wifi_timer   = lv_timer_create(wifi_update_cb, 1000, ui);
    timers.power_timer  = lv_timer_create(power_update_cb, 1000, ui);
    // timers.date_timer   = lv_timer_create(sys_time_update, 1000, ui);
    timers.timelapse_timer = lv_timer_create(ui_dyna_getparam_update, 1000, ui);
    // 立即执行一次更新
    lv_timer_ready(timers.wifi_timer);
    lv_timer_ready(timers.power_timer);
    // lv_timer_ready(timers.date_timer);
    lv_timer_ready(timers.timelapse_timer);
    // setup_scr_screen_VolumeOverlay(ui);
}

void font_setting(lv_obj_t *obj, font_id_t font_id, font_size_config_t size_cfg, font_color_t color, int16_t x,
                  int16_t y)
{
    const lv_font_t *base_font = NULL;
    uint16_t base_line_height  = 32;

    switch(font_id) {
        case FONT_SC16:
            base_font        = &lv_font_SourceHanSerifSC_Regular_16;
            base_line_height = lv_font_SourceHanSerifSC_Regular_16.line_height;
            break;
        case FONT_DEFAULT:
        default:
            base_font        = LV_FONT_DEFAULT;
            base_line_height = base_font->line_height;
            break;
    }

    lv_color_t lv_color = lv_color_hex(color); // 转换颜色

    lv_obj_set_style_text_font(obj, base_font, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(obj, lv_color, LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(obj, LV_OPA_COVER, LV_STATE_DEFAULT);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_align(obj, LV_ALIGN_TOP_LEFT);

    // 缩放
    int zoom = 256;
    if(size_cfg.mode == FONT_SIZE_MODE_CUSTOM) {
        zoom = (int)(((float)size_cfg.size / (float)base_line_height) * 256.0f);
    }
    lv_obj_set_style_transform_zoom(obj, zoom, LV_STATE_DEFAULT);
}

void set_chs_fonts(const char *font_path, int font_size, lv_style_t *style) // 改为指针参数
{
    font_chs = lv_freetype_font_create(font_path,                           // 字体文件路径
                                       LV_FREETYPE_FONT_RENDER_MODE_BITMAP, // 渲染模式（位图或轮廓）
                                       font_size,                           // 字体大小（像素）
                                       LV_FREETYPE_FONT_STYLE_NORMAL        // 字体样式（普通/粗体/斜体）
    );

    if(!font_chs) {
        MLOG_DBG("create fzyt failed, fontpath: %s\n", font_path);
        return;
    }

    // 不要重新初始化样式，只设置字体相关属性
    lv_style_set_text_font(style, font_chs); // 设置字体
    lv_style_set_text_align(style, LV_TEXT_ALIGN_CENTER);
}

lv_font_t *get_usr_fonts(const char *font_path, int font_size)
{
    font_chs = lv_freetype_font_create(font_path,                           // 字体文件路径
                                       LV_FREETYPE_FONT_RENDER_MODE_BITMAP, // 渲染模式（位图或轮廓）
                                       font_size,                           // 字体大小（像素）
                                       LV_FREETYPE_FONT_STYLE_NORMAL        // 字体样式（普通/粗体/斜体）
    );
    if(!font_chs) {
        MLOG_DBG("create fzyt failed, fontpath: %s\n", font_path);
        return NULL;
    }
    return font_chs;
}

void lv_set_obj_grad_style(lv_obj_t *obj, lv_grad_dir_t value, lv_color_t star_color_value, lv_color_t end_color_value)
{
    // 直接设置对象的样式属性
    lv_obj_set_style_bg_grad_dir(obj, value, LV_PART_MAIN);
    lv_obj_set_style_bg_color(obj, star_color_value, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(obj, end_color_value, LV_PART_MAIN);
}

// 文件系统回调函数实现

// 安全路径拼接函数
char *safe_path_join(const char *dir, const char *file)
{
    if(!dir || !file) return NULL;

    // 计算所需长度
    size_t dir_len  = strlen(dir);
    size_t file_len = strlen(file);

    // 计算总长度（包括可能的路径分隔符和null终止符）
    size_t total_len = dir_len + file_len + 2; // +1 for separator, +1 for '\0'

    // 分配内存
    char *path = malloc(total_len);
    if(!path) return NULL;

    // 构建路径
    char *p = path;

    // 复制目录部分
    memcpy(p, dir, dir_len);
    p += dir_len;

    // 添加路径分隔符（如果需要）
    if(dir_len > 0 && dir[dir_len - 1] != '/') {
        *p++ = '/';
    }

    // 复制文件名部分
    memcpy(p, file, file_len);
    p += file_len;

    // 确保以null结尾
    *p = '\0';

    return path;
}

// 获取项目根目录（包含zhuohao_ui的目录）
const char *get_project_root(void)
{
    static char project_root[PATH_MAX] = {0};

    // 如果已经计算过，直接返回
    if(project_root[0] != '\0') {
        return project_root;
    }

    // 方法1：获取当前工作目录
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd))) {
        // 尝试在当前目录查找zhuohao_ui
        char *test_path = safe_path_join(cwd, "zhuohao_ui");
        if(test_path) {
            if(access(test_path, F_OK) == 0) {
                strncpy(project_root, cwd, sizeof(project_root));
                project_root[sizeof(project_root) - 1] = '\0';
                free(test_path);
                return project_root;
            }
            free(test_path);
        }

        // 向上查找zhuohao_ui目录
        char current_path[PATH_MAX];
        strncpy(current_path, cwd, sizeof(current_path));
        current_path[sizeof(current_path) - 1] = '\0';

        while(1) {
            // 获取父目录
            char *parent = dirname(current_path);
            if(strcmp(current_path, parent) == 0) {
                break; // 到达根目录
            }

            // 安全拼接路径
            char *test_path = safe_path_join(parent, "zhuohao_ui");
            if(!test_path) break;

            if(access(test_path, F_OK) == 0) {
                strncpy(project_root, parent, sizeof(project_root));
                project_root[sizeof(project_root) - 1] = '\0';
                free(test_path);
                return project_root;
            }
            free(test_path);

            // 更新当前路径
            strncpy(current_path, parent, sizeof(current_path));
            current_path[sizeof(current_path) - 1] = '\0';
        }
    }

// 方法2：通过可执行文件路径查找 (Linux only)
#ifdef __linux__
    char exe_path[PATH_MAX];
    if(readlink("/proc/self/exe", exe_path, sizeof(exe_path)) != -1) {
        char *dir = dirname(exe_path);
        char current[PATH_MAX];
        strncpy(current, dir, sizeof(current));
        current[sizeof(current) - 1] = '\0';

        while(1) {
            char *test_path = safe_path_join(current, "zhuohao_ui");
            if(!test_path) break;

            if(access(test_path, F_OK) == 0) {
                strncpy(project_root, current, sizeof(project_root));
                project_root[sizeof(project_root) - 1] = '\0';
                free(test_path);
                return project_root;
            }
            free(test_path);

            char *parent = dirname(current);
            if(strcmp(current, parent) == 0) break;
            strncpy(current, parent, sizeof(current));
            current[sizeof(current) - 1] = '\0';
        }
    }
#endif

// 方法3：使用编译时默认值
#ifdef DEFAULT_PROJECT_ROOT
    strncpy(project_root, DEFAULT_PROJECT_ROOT, sizeof(project_root));
    project_root[sizeof(project_root) - 1] = '\0';
#else
    // 最终回退到固定路径
    strncpy(project_root, "/home/ubuntu2004/CV184_DC309/", sizeof(project_root));
    project_root[sizeof(project_root) - 1] = '\0';
#endif

    return project_root;
}

// 获取图片目录
const char *get_image_dir(void)
{
    static char image_dir[PATH_MAX] = {0};

    if(image_dir[0] == '\0') {
        const char *root = get_project_root();

        // 安全拼接路径
        char *path = safe_path_join(root, "zhuohao_ui/src/guiguider_ui/images");
        if(path) {
            strncpy(image_dir, path, sizeof(image_dir));
            image_dir[sizeof(image_dir) - 1] = '\0';
            free(path);
        } else {
            // 回退到硬编码路径
            strncpy(image_dir, "/home/ubuntu2004/CV184_DC309/zhuohao_ui/src/guiguider_ui/images", sizeof(image_dir));
            image_dir[sizeof(image_dir) - 1] = '\0';
        }
    }

    return image_dir;
}

// 打开文件回调
static void *fs_open_cb(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    // MLOG_DBG("传入路径: %s\n", path);
    // // 获取图片目录
    // const char *image_dir = get_image_dir();
    // 处理开头的斜杠
    const char *filename = path;
    if(*filename == '/') {
        filename++;
        // MLOG_DBG("跳过开头的斜杠，新文件名: %s\n", filename);
    }

    // 构造完整路径
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", IMAGE_DIR, filename);

    // 打开文件
    FILE *f = fopen(full_path, "rb");
    if(f == NULL) {
        MLOG_ERR("文件打开失败: %s,  %s\n", full_path, strerror(errno));
        return NULL;
    }

    return f;
}

// 关闭文件回调
static lv_fs_res_t fs_close_cb(lv_fs_drv_t *drv, void *file_p)
{
    FILE *f = (FILE *)file_p;
    if(fclose(f) == 0) {
        // MLOG_DBG("文件关闭成功\n");
        return LV_FS_RES_OK;
    }
    MLOG_ERR("文件关闭失败: %s\n", strerror(errno));
    return LV_FS_RES_UNKNOWN;
}

// 读取文件回调
static lv_fs_res_t fs_read_cb(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    FILE *f           = (FILE *)file_p;
    size_t read_count = fread(buf, 1, btr, f);
    *br               = (uint32_t)read_count;

    if(read_count > 0) {
        // MLOG_DBG("读取 %u 字节\n", *br);
        return LV_FS_RES_OK;
    }

    if(feof(f)) {
        MLOG_DBG("文件结束\n");
        return LV_FS_RES_OK;
    }

    MLOG_ERR("读取失败: %s\n", strerror(errno));
    return LV_FS_RES_UNKNOWN;
}

static lv_fs_res_t fs_seek_cb(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    FILE *f = (FILE *)file_p;
    int origin;

    switch(whence) {
        case LV_FS_SEEK_SET:
            origin = SEEK_SET;
            // MLOG_DBG("定位到起始位置: %u\n", pos);
            break;
        case LV_FS_SEEK_CUR:
            origin = SEEK_CUR;
            // MLOG_DBG("相对当前位置定位: %u\n", pos);
            break;
        case LV_FS_SEEK_END:
            origin = SEEK_END;
            // MLOG_DBG("定位到文件末尾: %u\n", pos);
            break;
        default: MLOG_ERR("无效的定位方式: %d\n", whence); return LV_FS_RES_INV_PARAM;
    }

    if(fseek(f, (long)pos, origin) != 0) {
        MLOG_ERR("定位失败: %s\n", strerror(errno));
        return LV_FS_RES_UNKNOWN;
    }

    // MLOG_DBG("定位成功\n");
    return LV_FS_RES_OK;
}

// 获取位置回调
static lv_fs_res_t fs_tell_cb(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    FILE *f  = (FILE *)file_p;
    long pos = ftell(f);

    if(pos >= 0) {
        *pos_p = (uint32_t)pos;
        // MLOG_DBG("当前位置: %u\n", *pos_p);
        return LV_FS_RES_OK;
    }

    MLOG_ERR("获取位置失败: %s\n", strerror(errno));
    return LV_FS_RES_UNKNOWN;
}

// 初始化文件系统驱动
void init_image_filesystem(void)
{
    static bool fs_registered = false;
    if(fs_registered) {
        MLOG_DBG("文件系统驱动已注册，跳过初始化\n");
        return;
    }

    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    // 设置文件系统标识符为'S'
    fs_drv.letter     = 'S';
    fs_drv.cache_size = 0;

    // 设置回调函数
    fs_drv.open_cb  = fs_open_cb;
    fs_drv.close_cb = fs_close_cb;
    fs_drv.read_cb  = fs_read_cb;
    fs_drv.seek_cb  = fs_seek_cb;
    fs_drv.tell_cb  = fs_tell_cb;

    // 注册文件系统驱动
    lv_fs_drv_register(&fs_drv);
    fs_registered = true;

    // MLOG_DBG("文件系统驱动已注册 (驱动器字母: %c)\n", fs_drv.letter);

    // // 验证驱动注册
    // lv_fs_drv_t *drv = lv_fs_get_drv('S');
    // if (drv) {
    //     MLOG_DBG("驱动器 'S' 已成功注册\n");
    // } else {
    //     MLOG_DBG("驱动器 'S' 未注册\n");
    // }

    // // 检查文件是否存在
    // char full_path[512];
    // snprintf(full_path, sizeof(full_path), "%s/%s", IMAGE_DIR, "caochang.png");

    // if (access(full_path, F_OK) == 0) {
    //     MLOG_DBG("文件存在: %s\n", full_path);

    //     if (access(full_path, R_OK) == 0) {
    //         MLOG_DBG("文件可读\n");
    //     } else {
    //         MLOG_DBG("文件不可读 (权限问题)\n");
    //     }
    // } else {
    //     MLOG_DBG("文件不存在: %s (错误: %d - %s)\n",
    //             full_path, errno, strerror(errno));
    // }

    // // 列出目录内容
    // char cmd[256];
    // snprintf(cmd, sizeof(cmd), "ls -l %s", IMAGE_DIR);
    // MLOG_DBG("目录内容:\n");
    // system(cmd);
}

// 对象删除回调函数
static void image_cleanup_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_current_target(e);
    char *lv_path = (char *)lv_event_get_user_data(e);

    if(lv_path) {
        MLOG_DBG("内存释放成功:lv_path:%s\n", lv_path);
        lv_free(lv_path);
        lv_obj_set_user_data(obj, NULL);
    }
}

// 显示图像的主函数
void show_image(lv_obj_t *obj, const char *image)
{
    // 参数检查
    if(!obj || !image) {
        MLOG_ERR("无效的show_image参数 null");
        return;
    }

    // 1. 检查并释放旧路径  如果同一个控件多次调用,先要将之前的控制路径释放掉
    char *old_path = (char *)lv_obj_get_user_data(obj);
    if(old_path) {
        lv_free(old_path);
        lv_obj_set_user_data(obj, NULL);
    }

    // 为每个对象创建独立的路径存储
    size_t path_len = strlen(image) + 4; // "S:/" + image + null
    char *lv_path   = lv_malloc(path_len);
    if(!lv_path) {
        MLOG_ERR("内存分配失败");
        return;
    }

    // 构造LVGL文件路径
    snprintf(lv_path, path_len, "S:/%s", image);

    // 将路径指针存储在用户数据中
    lv_obj_set_user_data(obj, lv_path);

    // 添加对象删除回调
    lv_obj_remove_event_cb(obj, image_cleanup_cb);
    lv_obj_add_event_cb(obj, image_cleanup_cb, LV_EVENT_DELETE, NULL);

    // 判断对象类型并设置图片源
    if(lv_obj_check_type(obj, &lv_imagebutton_class)) {
        // 图像按钮对象 - 设置所有状态的图片源
        lv_imagebutton_set_src(obj, LV_IMAGEBUTTON_STATE_RELEASED, NULL, lv_path, NULL);
        lv_imagebutton_set_src(obj, LV_IMAGEBUTTON_STATE_PRESSED, NULL, lv_path, NULL);
        lv_imagebutton_set_src(obj, LV_IMAGEBUTTON_STATE_DISABLED, NULL, lv_path, NULL);
    } else if(lv_obj_check_type(obj, &lv_image_class)) {
        // 普通图像对象
        lv_image_set_src(obj, lv_path);
    } else {
        // 其他类型对象：设置背景图片
        lv_obj_set_style_bg_image_src(obj, lv_path, LV_PART_MAIN | LV_STATE_DEFAULT);
        LV_LOG_INFO("为普通对象设置背景图片: %s", lv_path);
    }
}

// 全局变量
static lv_obj_t *battery_popup = NULL;
static lv_timer_t *battery_timer = NULL;
extern lv_style_t ttf_font_20;
extern lv_style_t ttf_font_24;
extern lv_style_t ttf_font_18;
static bool low_batter_tips= false;

static void close_popup_event(lv_event_t *e);


void set_lowbatter_tips_flag(bool ind)
{
    low_batter_tips = ind;
}

bool get_lowbatter_tips_flag(void)
{
    return low_batter_tips;
}

// 低电量提醒浮窗创建函数
void create_battery_warning_popup(void) {
    // 如果已有浮窗存在，先删除
    if (battery_popup) {
        lv_obj_del(battery_popup);
        battery_popup = NULL;
    }

    // 创建遮罩层
    lv_obj_t *mask = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mask, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(mask, LV_OPA_50, 0);
    lv_obj_set_style_radius(mask, 0, 0);
    lv_obj_set_style_border_width(mask, 0, 0);
    lv_obj_clear_flag(mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(mask, LV_OBJ_FLAG_CLICKABLE);

    // 创建浮窗容器
    battery_popup = lv_obj_create(mask);
    lv_obj_set_size(battery_popup, 280, 220);
    lv_obj_center(battery_popup);
    lv_obj_set_style_bg_color(battery_popup, lv_color_hex(0x1E1E2E), 0);
    lv_obj_set_style_bg_opa(battery_popup, LV_OPA_100, 0);
    lv_obj_set_style_radius(battery_popup, 24, 0);
    lv_obj_set_style_shadow_width(battery_popup, 60, 0);
    lv_obj_set_style_shadow_opa(battery_popup, LV_OPA_40, 0);
    lv_obj_set_style_border_width(battery_popup, 0, 0);
    lv_obj_clear_flag(battery_popup, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(battery_popup, 0, 0);

    // 创建警告图标
    lv_obj_t *icon = lv_label_create(battery_popup);
    lv_label_set_text(icon, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_48, 0);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 30);

    // 创建标题
    lv_obj_t *title = lv_label_create(battery_popup);
    lv_label_set_text(title, "低电量警告!!!");
    lv_obj_add_style(title, &ttf_font_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF0000), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 80);

    // 创建提示文本
    lv_obj_t *hint = lv_label_create(battery_popup);
    lv_label_set_text(hint, "请及时连接电源");
    lv_obj_add_style(hint, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(hint, lv_color_hex(0xA6ADC8), 0);
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 120);

    // 创建确认按钮
    lv_obj_t *btn = lv_btn_create(battery_popup);
    lv_obj_set_size(btn, 120, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x89B4FA), 0);
    lv_obj_set_style_radius(btn, 20, 0);

    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "知道了");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x1E1E2E), 0);
    lv_obj_add_style(btn_label, &ttf_font_18, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_center(btn_label);

    // 添加按钮点击事件
    lv_obj_add_event_cb(btn, close_popup_event, LV_EVENT_CLICKED, mask);
    lv_obj_add_event_cb(mask, close_popup_event, LV_EVENT_CLICKED, mask);
}

// 关闭浮窗的事件处理函数
static void close_popup_event(lv_event_t *e) {
    lv_obj_t *mask = lv_event_get_user_data(e);
    lv_obj_del(mask);
    battery_popup = NULL;
    set_lowbatter_tips_flag(false);
    // 如果定时器存在，删除定时器
    if (battery_timer) {
        lv_timer_del(battery_timer);
        battery_timer = NULL;
    }
}

// 定时器回调函数（用于自动关闭浮窗）
static void auto_close_timer(lv_timer_t *timer) {

    if (battery_popup) {
        lv_obj_t *mask = lv_obj_get_parent(battery_popup);
        lv_obj_del(mask);
        battery_popup = NULL;
        set_lowbatter_tips_flag(false);
    }

    lv_timer_del(timer);
    battery_timer = NULL;
}

// 显示低电量提醒（外部调用接口）
void show_battery_warning(void)
{
    // 创建新浮窗
    create_battery_warning_popup();
    // 设置10秒后自动关闭
    if(battery_timer == NULL) {
        battery_timer = lv_timer_create(auto_close_timer, 10000, NULL);
    }
}

// 路径规范化函数，移除多余的斜杠
void normalize_path(char *path)
{
    if (path == NULL) return;

    int len = strlen(path);
    int i, j;
    int slash_count = 0;

    // 移除多余的斜杠
    for (i = 0, j = 0; i < len; i++) {
        if (path[i] == '/') {
            slash_count++;
            // 只保留一个斜杠
            if (slash_count <= 1) {
                path[j++] = path[i];
            }
        } else {
            slash_count = 0;
            path[j++] = path[i];
        }
    }
    path[j] = '\0';

    // 确保路径不以斜杠结尾（除非是根目录）
    if (j > 1 && path[j-1] == '/') {
        path[j-1] = '\0';
    }
}

mode_chage_completed_t completed_cb = NULL;
bool exit_complete_flag = true;
void completed_execution_and_unregister_cb(void)
{
    if(completed_cb != NULL) {
        completed_cb();
        completed_cb = NULL;
    }
}

void completed_register_cb(mode_chage_completed_t callback)
{
    completed_cb = callback;
}

bool get_exit_completed(void)
{
    return exit_complete_flag;
}

void set_exit_completed(bool sta)
{
    exit_complete_flag = sta;
}

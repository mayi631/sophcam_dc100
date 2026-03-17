#define DEBUG
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "config.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"

#include "cvi_log.h"
#include "osal.h"
#include "page_all.h"
#include "style_common.h"
#include "ui_common.h"

#include "common/takephoto.h"
#include "hal_backlight.h"
#include "hal_wifi_ctrl.h"
#include "indev.h"
#include "power.h"
#include "voiceplay.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include "cvi_comm_gfbg.h"

#define USE_DOUBLE_FB 0
#define BYEBYE_IMAGE_PATH "S:/BYEBYE.jpg"
typedef struct {
    const char * devname;
    lv_color_format_t color_format;
#if LV_LINUX_FBDEV_BSD
    struct bsd_fb_var_info vinfo;
    struct bsd_fb_fix_info finfo;
#else
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
#endif /* LV_LINUX_FBDEV_BSD */
#if LV_LINUX_FBDEV_MMAP
    char * fbp;
#endif
    uint8_t * rotated_buf;
    size_t rotated_buf_size;
    long int screensize;
    int fbfd;
    bool force_refresh;
} lv_linux_fb_t;

lv_ui_t g_ui;
static OSAL_TASK_HANDLE_S ui_task;
static bool s_buiInit    = false;
static bool g_ui_running = false;
// static OSAL_TASK_HANDLE_S      ui_task;

// 添加全局变量保存触摸设备
lv_indev_t *global_touch_indev = NULL;

#ifndef SERVICES_LIVEVIEW_ON
typedef struct _VOICEPLAYER_AOUT_OPT_S
{
    void *hAudDevHdl;   // device id
    void *hAudTrackHdl; // chn id
} VOICEPLAY_AOUT_OPT_S;

typedef struct _VOICEPLAY_VOICETABLE_S
{
    uint32_t u32VoiceIdx;
    char aszFilePath[APPCOMM_MAX_PATH_LEN];
} VOICEPLAY_VOICETABLE_S;

/** voiceplay configuration */
typedef struct __VOICEPLAY_CFG_S
{
    uint32_t u32MaxVoiceCnt;
    VOICEPLAY_VOICETABLE_S *pstVoiceTab;
    VOICEPLAY_AOUT_OPT_S stAoutOpt;
} VOICEPLAY_CFG_S;
#endif

// static void UI_VOLQUEUE_Init(void)
// {
// #ifdef SERVICES_LIVEVIEW_ON
// 	MEDIA_SYSHANDLE_S SysHandle = MEDIA_GetCtx()->SysHandle;
// 	VOICEPLAY_CFG_S stVoicePlayCfg = {0};

// 	stVoicePlayCfg.stAoutOpt.hAudDevHdl = SysHandle.aohdl;

// 	stVoicePlayCfg.u32MaxVoiceCnt = UI_VOICE_MAX_NUM;
// 	VOICEPLAY_VOICETABLE_S astVoiceTab[UI_VOICE_MAX_NUM] =
// 		{
// 			{UI_VOICE_START_UP_IDX, UI_VOICE_START_UP_SRC},
// 			{UI_VOICE_TOUCH_BTN_IDX, UI_VOICE_TOUCH_BTN_SRC},
// 			{UI_VOICE_CLOSE_IDX, UI_VOICE_CLOSE_SRC},
// 			{UI_VOICE_PHOTO_IDX, UI_VOICE_PHOTO_SRC},
// 		};
// 	stVoicePlayCfg.pstVoiceTab = astVoiceTab;
// 	int32_t s32Ret = VOICEPLAY_Init(&stVoicePlayCfg);
// 	if (s32Ret != 0) {
// 		CVI_LOGE("VOICEPLAY_Init failed!\n");
// 	}
// #endif
// }

static void write_to_fb(lv_linux_fb_t * dsc, uint32_t fb_pos, const void * data, size_t sz)
{
#if LV_LINUX_FBDEV_MMAP
    uint8_t * fbp = (uint8_t *)dsc->fbp;
    lv_memcpy(&fbp[fb_pos], data, sz);
#else
    if(pwrite(dsc->fbfd, data, sz, fb_pos) < 0)
        LV_LOG_ERROR("write failed: %d", errno);
#endif
}

static void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p)
{
    lv_linux_fb_t * dsc = lv_display_get_driver_data(disp);

#if LV_LINUX_FBDEV_MMAP
    if(dsc->fbp == NULL) {
        lv_display_flush_ready(disp);
        return;
    }
#endif

    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);
    lv_color_format_t cf = lv_display_get_color_format(disp);
    uint32_t px_size = lv_color_format_get_size(cf);

    lv_area_t rotated_area;
    lv_display_rotation_t rotation = lv_display_get_rotation(disp);

    /* Not all framebuffer kernel drivers support hardware rotation, so we need to handle it in software here */
    if(rotation != LV_DISPLAY_ROTATION_0 && LV_LINUX_FBDEV_RENDER_MODE == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        /* (Re)allocate temporary buffer if needed */
        size_t buf_size = w * h * px_size;
        if(!dsc->rotated_buf || dsc->rotated_buf_size != buf_size) {
            dsc->rotated_buf = realloc(dsc->rotated_buf, buf_size);
            dsc->rotated_buf_size = buf_size;
        }

        /* Rotate the pixel buffer */
        uint32_t w_stride = lv_draw_buf_width_to_stride(w, cf);
        uint32_t h_stride = lv_draw_buf_width_to_stride(h, cf);

        switch(rotation) {
            case LV_DISPLAY_ROTATION_0:
                break;
            case LV_DISPLAY_ROTATION_90:
                lv_draw_sw_rotate(color_p, dsc->rotated_buf, w, h, w_stride, h_stride, rotation, cf);
                break;
            case LV_DISPLAY_ROTATION_180:
                lv_draw_sw_rotate(color_p, dsc->rotated_buf, w, h, w_stride, w_stride, rotation, cf);
                break;
            case LV_DISPLAY_ROTATION_270:
                lv_draw_sw_rotate(color_p, dsc->rotated_buf, w, h, w_stride, h_stride, rotation, cf);
                break;
        }
        color_p = dsc->rotated_buf;

        /* Rotate the area */
        rotated_area = *area;
        lv_display_rotate_area(disp, &rotated_area);
        area = &rotated_area;

        if(rotation != LV_DISPLAY_ROTATION_180) {
            w = lv_area_get_width(area);
            h = lv_area_get_height(area);
        }
    }

    /* Ensure that we're within the framebuffer's bounds */
    if(area->x2 < 0 || area->y2 < 0 || area->x1 > (int32_t)dsc->vinfo.xres - 1 || area->y1 > (int32_t)dsc->vinfo.yres - 1) {
        lv_display_flush_ready(disp);
        return;
    }
    uint32_t fb_pos =
        (area->x1 + dsc->vinfo.xoffset) * px_size +
        (area->y1 + dsc->vinfo.yoffset) * dsc->finfo.line_length;

    int32_t y;
    
    /* 优化1: 减少系统调用频率 */
    static int flush_count = 0;
    bool need_pan_display = false;
    
    /* 修复编译错误：将有符号整数转换为无符号整数进行比较 */
    uint32_t w_u32 = (uint32_t)w;
    uint32_t h_u32 = (uint32_t)h;
    uint32_t area_y1_u32 = (uint32_t)area->y1;
    uint32_t area_y2_u32 = (uint32_t)area->y2;
    
    /* 优化2: 根据区域大小决定刷新策略 */
    if(LV_LINUX_FBDEV_RENDER_MODE == LV_DISPLAY_RENDER_MODE_DIRECT) {
        uint32_t color_pos =
            area->x1 * px_size +
            area->y1 * lv_display_get_horizontal_resolution(disp) * px_size;

        for(y = area->y1; y <= area->y2; y++) {
            write_to_fb(dsc, fb_pos, &color_p[color_pos], w * px_size);
            fb_pos += dsc->finfo.line_length;
            color_pos += lv_display_get_horizontal_resolution(disp) * px_size;
        }
        
        // 大面积更新时立即刷新
        if (w_u32 * h_u32 > (dsc->vinfo.xres * dsc->vinfo.yres) / 4) {
            need_pan_display = true;
        }
    }
    else {
        w = lv_area_get_width(area);
        w_u32 = (uint32_t)w;  // 更新w_u32
        
        /* 优化3: 优化内存拷贝策略 */
        // 如果是整行刷新，使用更高效的方式
        if (w_u32 == dsc->vinfo.xres && area->x1 == 0) {
            // 整行刷新，一次性写入多行
            size_t total_size = w_u32 * h_u32 * px_size;
            write_to_fb(dsc, fb_pos, color_p, total_size);
        } else {
            // 部分区域刷新，逐行写入
            for(y = area->y1; y <= area->y2; y++) {
                write_to_fb(dsc, fb_pos, color_p, w_u32 * px_size);
                fb_pos += dsc->finfo.line_length;
                color_p += w_u32 * px_size;
            }
        }
        
        // 大面积更新或重要区域立即刷新
        if (w_u32 * h_u32 > (dsc->vinfo.xres * dsc->vinfo.yres) / 3 || 
            (area_y1_u32 == 0 && area_y2_u32 == dsc->vinfo.yres - 1)) {
            need_pan_display = true;
        }
    }

    /* 优化4: 减少FBIOPAN_DISPLAY调用频率 */
    flush_count++;
    
    // 重要区域、大面积更新或达到一定刷新次数时进行显示同步
    if (need_pan_display || flush_count >= 5) {
        if (ioctl(dsc->fbfd, FBIOPAN_DISPLAY, &(dsc->vinfo)) < 0) {
            perror("FBIOPAN_DISPLAY failed!\n");
        }
        flush_count = 0;
    }

    /* 优化5: 只在必要时强制刷新 */
    if(dsc->force_refresh && (flush_count == 0 || need_pan_display)) {
        dsc->vinfo.activate |= FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
        if(ioctl(dsc->fbfd, FBIOPUT_VSCREENINFO, &(dsc->vinfo)) == -1) {
            perror("Error setting var screen info");
        }
    }

    lv_display_flush_ready(disp);
}

#ifndef DIV_ROUND_UP
    #define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

static void _linux_fbdev_set_file(lv_display_t *disp, const char * file)
{
    char * devname = lv_strdup(file);
    LV_ASSERT_MALLOC(devname);
    if(devname == NULL) return;

    lv_linux_fb_t * dsc = lv_display_get_driver_data(disp);
    dsc->devname = devname;

    if(dsc->fbfd > 0) close(dsc->fbfd);

    /* Open the file for reading and writing*/
    dsc->fbfd = open(dsc->devname, O_RDWR);
    if(dsc->fbfd == -1) {
        perror("Error: cannot open framebuffer device");
        return;
    }
    LV_LOG_INFO("The framebuffer device was opened successfully");

    /* Make sure that the display is on.*/
    if(ioctl(dsc->fbfd, FBIOBLANK, FB_BLANK_UNBLANK) != 0) {
        perror("ioctl(FBIOBLANK)");
        /* Don't return. Some framebuffer drivers like efifb or simplefb don't implement FBIOBLANK.*/
    }

#if LV_LINUX_FBDEV_BSD
    struct fbtype fb;
    unsigned line_length;

    /*Get fb type*/
    if(ioctl(dsc->fbfd, FBIOGTYPE, &fb) != 0) {
        perror("ioctl(FBIOGTYPE)");
        return;
    }

    /*Get screen width*/
    if(ioctl(dsc->fbfd, FBIO_GETLINEWIDTH, &line_length) != 0) {
        perror("ioctl(FBIO_GETLINEWIDTH)");
        return;
    }

    dsc->vinfo.xres = (unsigned) fb.fb_width;
    dsc->vinfo.yres = (unsigned) fb.fb_height;
    dsc->vinfo.bits_per_pixel = fb.fb_depth;
    dsc->vinfo.xoffset = 0;
    dsc->vinfo.yoffset = 0;
    dsc->finfo.line_length = line_length;
    dsc->finfo.smem_len = dsc->finfo.line_length * dsc->vinfo.yres;
#else /* LV_LINUX_FBDEV_BSD */

    /* Get fixed screen information*/
    if(ioctl(dsc->fbfd, FBIOGET_FSCREENINFO, &dsc->finfo) == -1) {
        perror("Error reading fixed information");
        return;
    }

    /* Get variable screen information*/
    if(ioctl(dsc->fbfd, FBIOGET_VSCREENINFO, &dsc->vinfo) == -1) {
        perror("Error reading variable information");
        return;
    }
    cvi_fb_point point = {0,60};
    ioctl (dsc->fbfd, FBIOPUT_SCREEN_ORIGIN_GFBG, &point);

#ifdef USE_DOUBLE_FB
    dsc->vinfo.yres_virtual = V_RES * 2;
    dsc->vinfo.xres_virtual = H_RES;
    if(ioctl(dsc->fbfd, FBIOPUT_VSCREENINFO, &dsc->vinfo) == -1) {
        perror("Error setting var screen info");
        return;
    }
#endif
#endif /* LV_LINUX_FBDEV_BSD */

    LV_LOG_INFO("%dx%d, %dbpp", dsc->vinfo.xres, dsc->vinfo.yres, dsc->vinfo.bits_per_pixel);

    /* Figure out the size of the screen in bytes*/
    dsc->screensize =  dsc->finfo.smem_len;/*finfo.line_length * vinfo.yres;*/

#if LV_LINUX_FBDEV_MMAP
    /* Map the device to memory*/
    dsc->fbp = (char *)mmap(0, dsc->screensize, PROT_READ | PROT_WRITE, MAP_SHARED, dsc->fbfd, 0);
    if((intptr_t)dsc->fbp == -1) {
        perror("Error: failed to map framebuffer device to memory");
        return;
    }
#endif

    /* Don't initialise the memory to retain what's currently displayed / avoid clearing the screen.
     * This is important for applications that only draw to a subsection of the full framebuffer.*/

    LV_LOG_INFO("The framebuffer device was mapped to memory successfully");

    switch(dsc->vinfo.bits_per_pixel) {
        case 16:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
            break;
        case 24:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB888);
            break;
        case 32:
            lv_display_set_color_format(disp, LV_COLOR_FORMAT_ARGB8888);
            break;
        default:
            LV_LOG_WARN("Not supported color format (%d bits)", dsc->vinfo.bits_per_pixel);
            return;
    }

    int32_t hor_res = dsc->vinfo.xres;
    int32_t ver_res = dsc->vinfo.yres;
    int32_t width = dsc->vinfo.width;
    uint32_t draw_buf_size = hor_res * (dsc->vinfo.bits_per_pixel >> 3);
    if(LV_LINUX_FBDEV_RENDER_MODE == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        // 部分渲染模式：使用固定的缓冲区行数
        draw_buf_size *= LV_LINUX_FBDEV_BUFFER_SIZE;
    } else {
        // 全屏渲染模式：确保缓冲区足够大
        draw_buf_size *= ver_res;
        // 对于360分辨率，需要确保缓冲区大小合适
        if (draw_buf_size < H_RES * V_RES * 4) {  // 32位色深
            draw_buf_size = H_RES * V_RES * 4;    // 确保最小缓冲区大小
        }
    }

    uint8_t * draw_buf = NULL;
    uint8_t * draw_buf_2 = NULL;

    draw_buf = malloc(draw_buf_size);

    if(LV_LINUX_FBDEV_BUFFER_COUNT == 2) {
        draw_buf_2 = malloc(draw_buf_size);
    }

    lv_display_set_resolution(disp, hor_res, ver_res);
    lv_display_set_buffers(disp, draw_buf, draw_buf_2, draw_buf_size, LV_LINUX_FBDEV_RENDER_MODE);

    if(width > 0) {
        lv_display_set_dpi(disp, DIV_ROUND_UP(hor_res * 254, width * 10));
    }

    LV_LOG_INFO("Resolution is set to %" LV_PRId32 "x%" LV_PRId32 " at %" LV_PRId32 "dpi",
                hor_res, ver_res, lv_display_get_dpi(disp));
}

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ?: dflt;
}

static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", FB_DEV_NAME);
    lv_display_t *disp = lv_linux_fbdev_create();

    lv_display_set_flush_cb(disp, my_flush_cb);
    _linux_fbdev_set_file(disp, device);
    // lv_linux_fbdev_set_file(disp, device);
    lv_display_set_resolution(disp, H_RES, V_RES);

    global_touch_indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, TOUCH_PANEL_EVENT_PATH);
    lv_indev_set_display(global_touch_indev, disp);

    // lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
}

// 全局硬件按键管理
static int global_input_fd_0 = -1;  // event0 的文件描述符
static int global_input_fd_1 = -1;  // event1 的文件描述符
static int global_input_fd_2 = -1;  // event2 的文件描述符
static lv_indev_t *global_hardware_indev_1 = NULL;
static lv_indev_t *global_hardware_indev_0 = NULL;
static lv_indev_t *global_hardware_indev_2 = NULL;

static lv_obj_t *global_byebye_img = NULL;
static lv_timer_t *global_power_long_press_timer = NULL;  // 保存定时器句柄，防止重复创建
static volatile bool global_power_key_pressed_state = false;
static volatile bool global_timeout = false;

static void timer_callback(lv_timer_t * timer)
{
    lv_obj_t * screen = lv_screen_active();
    HAL_BACKLIGHT_SetBackLightState(BACKLIGHT_STATE_OFF);
    // 清除屏幕上的所有对象
    lv_obj_clean(screen);
    // 删除定时器（一次性使用）
    lv_timer_del(global_power_long_press_timer);
    global_power_long_press_timer = NULL;  // 清除定时器句柄
    global_timeout = true;
    if (! global_power_key_pressed_state) {
        system("poweroff");
    }
}

static void show_power_long_press_image(void)
{
    MLOG_INFO("power_key long press\n");

    // 如果已有定时器在运行，先删除旧的定时器
    if(global_power_long_press_timer != NULL) {
        lv_timer_del(global_power_long_press_timer);
        global_power_long_press_timer = NULL;
    }

    // 放 toplayer 上显示图片
    global_byebye_img = lv_img_create(lv_layer_top());
    lv_img_set_src(global_byebye_img, BYEBYE_IMAGE_PATH);

    // 确保图片对象是可见的
    lv_obj_set_pos(global_byebye_img, 0, 0);
    lv_obj_set_size(global_byebye_img, 640, 480);
    lv_obj_clear_flag(global_byebye_img, LV_OBJ_FLAG_HIDDEN);
    // 创建定时器，2秒（2000ms）后执行黑屏操作
    global_timeout = false;
    global_power_long_press_timer = lv_timer_create(timer_callback, 2000, NULL);
}

// 统一的硬件按键读取回调函数
static void global_hardware_key_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    // 确定是哪个设备
    int device_fd = -1;
    int device_id = -1;
    // 录像状态
    uint8_t video_status = 0;

    if(indev == global_hardware_indev_0) {
        device_fd = global_input_fd_0;
        device_id = 0;
    } else if(indev == global_hardware_indev_1) {
        device_fd = global_input_fd_1;
        device_id = 1;
    } else if(indev == global_hardware_indev_2) {
        device_fd = global_input_fd_2;
        device_id = 2;
    }

    if(device_fd < 0) {
        return;
    }

    // 读取按键事件
    struct input_event ev;
    int rd = read(device_fd, &ev, sizeof(ev));
    static bool last_pressed = false;
    static time_t power_key_press_time = 0;
    static bool power_key_long_pressed = false;
    // extern bool album_video_mode;

    if(rd == sizeof(ev)) {
        if(ev.type == EV_KEY) {
            MLOG_DBG("Key event from device %d: code=%d, value=%d\n", device_id, ev.code, ev.value);

            // 更新最后活动时间
            update_last_activity_time();

            // 检测按键按下
            bool pressed = (ev.value == 1);
            if(pressed != last_pressed) {
                if(ev.code == KEY_POWER) {
                    if(ev.value == 1) {
                        // 按键按下，记录时间
                        power_key_press_time = time(NULL);
                        global_power_key_pressed_state = true;
                        MLOG_DBG("Power key pressed, start timing\n");
                    } else {
                        // 按键释放，重置状态
                        global_power_key_pressed_state = false;
                        MLOG_DBG("Power key released\n");
                        if(power_key_long_pressed && global_timeout) {
                            HAL_BACKLIGHT_SetBackLightState(BACKLIGHT_STATE_OFF);
                            power_key_long_pressed = false;
                            system("poweroff");
                        }
                    }
                }
                if(pressed && getaction_audio_Index()) {
                    // MAPI_AO_Unmute(MEDIA_GetCtx()->SysHandle.aohdl);
                    video_get_status(&video_status);
                    if((ev.code != KEY_CAMERA && ev.code != KEY_CAMERA_FOCUS &&
                        ev.code != KEY_ZOOMIN && ev.code != KEY_ZOOMOUT) ||
                        ((ev.code == KEY_ZOOMIN || ev.code == KEY_ZOOMOUT) &&
                        video_status == VEDIO_STOP)) {
                        EVENT_S stEvent = {0};
                        stEvent.topic = EVENT_UI_TOUCH;
                        EVENTHUB_Publish(&stEvent);
                    }
                }
                // 调用当前页面的按键处理回调
                if (current_page_key_handler != NULL) {
                    current_page_key_handler(ev.code, ev.value);
                }
            }
            last_pressed = pressed;
        }
    }

    // 检查长按3秒
    if(global_power_key_pressed_state && !power_key_long_pressed) {
        time_t current_time = time(NULL);
        int ret = MAPI_SUCCESS;
        MAPI_AO_HANDLE_T ao_hdl = NULL;
        if(current_time - power_key_press_time >= 2) {
            MLOG_DBG("Power key long press detected (2 seconds), showing power image\n");
            show_power_long_press_image();
            disable_touch_events();//延时倒计时，禁用触摸
            disable_hardware_input_device(0);
            disable_hardware_input_device(1);
            ret = MAPI_AO_GetHandle(&ao_hdl);
            if(ret == MAPI_SUCCESS) {
                MAPI_AO_SetAmplifier(ao_hdl, CVI_FALSE);
            }

            power_key_long_pressed = true;
        }
    }
}

// 硬件按键输入设备删除回调函数
static void global_hardware_indev_delete_cb(lv_event_t *e)
{
    if(global_input_fd_0 >= 0) {
        close(global_input_fd_0);
        global_input_fd_0 = -1;
        MLOG_DBG("Global hardware button input device 0 closed\n");
    }
    if(global_input_fd_1 >= 0) {
        close(global_input_fd_1);
        global_input_fd_1 = -1;
        MLOG_DBG("Global hardware button input device 1 closed\n");
    }
}

// 创建全局硬件按键输入设备
static void create_global_hardware_input_device(int device_id)
{
    const char *devices;
    int *fd_ptr;
    lv_indev_t **indev_ptr;

    if(device_id == 0) {
        devices = KEY_EVENT_PATH_0;
        fd_ptr = &global_input_fd_0;
        indev_ptr = &global_hardware_indev_0;
    } else if(device_id == 1) {
        devices = KEY_EVENT_PATH_1;
        fd_ptr = &global_input_fd_1;
        indev_ptr = &global_hardware_indev_1;
    } else if(device_id == 2) {
        devices = POWER_KEY_EVENT_PATH;
        fd_ptr = &global_input_fd_2;
        indev_ptr = &global_hardware_indev_2;
    } else {
        MLOG_ERR("Invalid device_id: %d\n", device_id);
        return;
    }

    MLOG_DBG("Opening input device %d: %s\n", device_id, devices);

    // 打开输入设备
    *fd_ptr = open(devices, O_RDONLY | O_NONBLOCK);
    if(*fd_ptr < 0) {
        MLOG_ERR("Failed to open input device %d: %s\n", device_id, devices);
        return;
    }

    MLOG_DBG("Global hardware button input device %d opened\n", device_id);

    // 创建LVGL输入设备
    *indev_ptr = lv_indev_create();
    lv_indev_set_type(*indev_ptr, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(*indev_ptr, global_hardware_key_read_cb);
    lv_indev_set_driver_data(*indev_ptr, NULL);
    lv_indev_add_event_cb(*indev_ptr, global_hardware_indev_delete_cb, LV_EVENT_DELETE, NULL);

    MLOG_DBG("Global hardware button input device %d created\n", device_id);
}

// 删除全局硬件按键输入设备
void delete_global_hardware_input_device(void)
{
    if(global_hardware_indev_1 != NULL) {
        lv_indev_delete(global_hardware_indev_1);
        global_hardware_indev_1 = NULL;
        MLOG_DBG("Global hardware button input device 1 deleted\n");
    }
    if(global_hardware_indev_0 != NULL) {
        lv_indev_delete(global_hardware_indev_0);
        global_hardware_indev_0 = NULL;
        MLOG_DBG("Global hardware button input device 0 deleted\n");
    }
}

static void start_uiapp(void* arg)
{
    UNUSED(arg);
    lv_init();

    // 初始化字体样式
    init_fonts_style();

    // 初始化公共页面样式
    init_common_style();

    /*Linux display device init*/
    lv_linux_disp_init();

    UI_VOICEPLAY_Init();

    // 创建全局硬件按键输入设备
    create_global_hardware_input_device(0);
    create_global_hardware_input_device(1);
    create_global_hardware_input_device(2);

    /* Create a GUI-Guider app */
    setup_ui(&g_ui);
    custom_init(&g_ui);

    // 启动自动关机定时器
    start_auto_poweroff_timer();

    /*Handle LVGL tasks*/
    g_ui_running = true;
    while(g_ui_running) {
        lv_timer_handler();
        usleep(5000);
    }
}

/* 获取底层的参数设置，并应用到UI中相关的静态变量 */
void update_setting_from_param(void)
{
    int32_t menu_index = 0;

    PARAM_CONTEXT_S* pstParamCtx = PARAM_GetCtx();
    menu_index = pstParamCtx->pstCfg->Menu.LightFrequence.Current;
    // 设置当前频率索引
    SetLightFreq_Index(menu_index);
    SetLightFreq_Label(pstParamCtx->pstCfg->Menu.LightFrequence.Items[menu_index].Desc);
    // 设置人脸侦测
    menu_index = pstParamCtx->pstCfg->Menu.FaceDet.Current;
    setface_Index(menu_index);
    setface_Label(pstParamCtx->pstCfg->Menu.FaceDet.Items[menu_index].Desc);
    // 设置笑脸侦测
    menu_index = pstParamCtx->pstCfg->Menu.FaceSmile.Current;
    setsmile_Index(menu_index);
    setsmile_Label(pstParamCtx->pstCfg->Menu.FaceSmile.Items[menu_index].Desc);
    // 设置照片分辨率
    menu_index = pstParamCtx->pstCfg->Menu.PhotoSize.Current;
    photo_setRes_Index(menu_index);
    photo_setRes_Label(pstParamCtx->pstCfg->Menu.PhotoSize.Items[menu_index].Desc);
    // 设置视频分辨率
    menu_index = pstParamCtx->pstCfg->Menu.VideoSize.Current;
    video_setRes_Index(menu_index);
    video_setRes_Label(pstParamCtx->pstCfg->Menu.VideoSize.Items[menu_index].Desc);

    int32_t osd_value = pstParamCtx->pstCfg->Menu.Osd.Current;
    SettimeSelect_Index(osd_value);
    extern char g_sysbtn_labelTime[32];
    extern char* sysmenu_time_btn_labels[];
    strncpy(g_sysbtn_labelTime, sysmenu_time_btn_labels[osd_value == 1 ? 0 : 1], sizeof(g_sysbtn_labelTime));

    // 设置自动关闭
    menu_index = pstParamCtx->pstCfg->Menu.PowerOff.Current;
    setpoweroff_Index(menu_index);
    extern char g_sysbtn_labelPowerdown[32];
    strncpy(g_sysbtn_labelPowerdown, pstParamCtx->pstCfg->Menu.PowerOff.Items[menu_index].Desc, sizeof(g_sysbtn_labelPowerdown));

    // 设置动作音
    menu_index = pstParamCtx->pstCfg->Menu.ActionAudio.Current;
    setaction_audio_Index(menu_index);
    extern char g_sysbtn_labelVolume[32];
    strncpy(g_sysbtn_labelVolume, pstParamCtx->pstCfg->Menu.ActionAudio.Items[menu_index].Desc, sizeof(g_sysbtn_labelVolume));

    // 设置闪光灯
    menu_index = pstParamCtx->pstCfg->Menu.FlashLed.Current;
    photo_setFlash_Index(menu_index);
    photo_setFlash_Label(pstParamCtx->pstCfg->Menu.FlashLed.Items[menu_index].Desc);

    // 设置照片质量
    menu_index = pstParamCtx->pstCfg->Menu.PhotoQuality.Current;
    photo_setQuality_Index(menu_index);
    photo_setQuality_Label(pstParamCtx->pstCfg->Menu.PhotoQuality.Items[menu_index].Desc);

    // 设置系统语言
    menu_index = pstParamCtx->pstCfg->Menu.Language.Current;
    setsysMenu_Language_Index(menu_index);
    setsysMenu_Language_Label(pstParamCtx->pstCfg->Menu.Language.Items[menu_index].Desc);

    // 自动息屏
    menu_index = pstParamCtx->pstCfg->Menu.AutoScreenOff.Current;
    setsysMenu_ScrOff_Index(menu_index);
    setsysMenu_ScrOff_Label(pstParamCtx->pstCfg->Menu.AutoScreenOff.Items[menu_index].Desc);
}

int UIAPP_Start(void)
{

    int32_t WorkMode = MODEMNG_GetCurWorkMode();
    MLOG_INFO("UIAPP_Start in mode %d\n", WorkMode);
    if ((WorkMode == WORK_MODE_MOVIE) || (WorkMode == WORK_MODE_PHOTO)) {
        MEDIA_SetLightFrequence();
    }

    if (s_buiInit == false) {
        /* 默认 wlan0 down */
        Hal_Wpa_Down();

        update_setting_from_param();

        /* 订阅消息 */
        ui_common_SubscribeEvents();

        // start ui task
        OSAL_TASK_ATTR_S ta;
        ta.name       = "cvi_ui";
        ta.entry      = start_uiapp;
        ta.param      = NULL;
        ta.priority   = OSAL_TASK_PRI_RT_LOWEST;
        ta.detached   = false;
        ta.stack_size = 256 * 1024;
        int32_t rc    = OSAL_TASK_Create(&ta, &ui_task);
        if(rc != OSAL_SUCCESS) {
            MLOG_ERR("cvi_ui task create failed, %d\n", rc);
            return -1;
        }
        s_buiInit = true;
    } else {
        MLOG_INFO("ui already init\n");
    }

    return 0;
}

int UIAPP_Stop(void)
{
    // 停止自动关机定时器
    stop_auto_poweroff_timer();

    // 删除全局硬件按键输入设备
    delete_global_hardware_input_device();

    UI_VOICEPLAY_DeInit(NULL);

    g_ui_running = false;
    OSAL_TASK_Join(ui_task);
    OSAL_TASK_Destroy(&ui_task);

    s_buiInit = false;
    return 0;
}


// 禁用触摸事件
void disable_touch_events(void)
{
    if (global_touch_indev != NULL) {
        lv_indev_enable(global_touch_indev, false);
        MLOG_INFO("触摸事件已禁用\n");
    }
}

// 启用触摸事件
void enable_touch_events(void)
{
    if (global_touch_indev != NULL) {
        lv_indev_enable(global_touch_indev, true);
        MLOG_INFO("触摸事件已启用\n");
    }
}


// 启用指定硬件按键输入设备
int enable_hardware_input_device(int device_id)
{
    const char *device_path = NULL;
    int *fd_ptr = NULL;
    lv_indev_t **indev_ptr = NULL;

    // 根据设备ID确定对应的路径和全局变量
    switch(device_id) {
        case 0:
            device_path = KEY_EVENT_PATH_0;
            fd_ptr = &global_input_fd_0;
            indev_ptr = &global_hardware_indev_0;
            break;
        case 1:
            device_path = KEY_EVENT_PATH_1;
            fd_ptr = &global_input_fd_1;
            indev_ptr = &global_hardware_indev_1;
            break;
        default:
            MLOG_ERR("Invalid device_id: %d\n", device_id);
            return -1;
    }

    // 检查设备是否已经启用
    if (*fd_ptr >= 0) {
        MLOG_DBG("Hardware input device %d is already enabled\n", device_id);
        return 0;
    }

    MLOG_DBG("Enabling hardware input device %d: %s\n", device_id, device_path);

    // 打开输入设备
    *fd_ptr = open(device_path, O_RDONLY | O_NONBLOCK);
    if (*fd_ptr < 0) {
        MLOG_ERR("Failed to enable input device %d: %s, error: %s\n",
                device_id, device_path, strerror(errno));
        return -1;
    }

    // 创建LVGL输入设备（如果不存在）
    if (*indev_ptr == NULL) {
        *indev_ptr = lv_indev_create();
        lv_indev_set_type(*indev_ptr, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_read_cb(*indev_ptr, global_hardware_key_read_cb);
        lv_indev_set_driver_data(*indev_ptr, NULL);
        lv_indev_add_event_cb(*indev_ptr, global_hardware_indev_delete_cb, LV_EVENT_DELETE, NULL);

        MLOG_DBG("Created LVGL input device for hardware device %d\n", device_id);
    }

    MLOG_DBG("Hardware input device %d enabled successfully\n", device_id);
    return 0;
}

// 禁用指定硬件按键输入设备
int disable_hardware_input_device(int device_id)
{
    int *fd_ptr = NULL;
    lv_indev_t **indev_ptr = NULL;

    // 根据设备ID确定对应的全局变量
    switch(device_id) {
        case 0:
            fd_ptr = &global_input_fd_0;
            indev_ptr = &global_hardware_indev_0;
            break;
        case 1:
            fd_ptr = &global_input_fd_1;
            indev_ptr = &global_hardware_indev_1;
            break;
        default:
            MLOG_ERR("Invalid device_id: %d\n", device_id);
            return -1;
    }

    // 检查设备是否已经禁用
    if (*fd_ptr < 0) {
        MLOG_DBG("Hardware input device %d is already disabled\n", device_id);
        return 0;
    }

    MLOG_DBG("Disabling hardware input device %d\n", device_id);

    // 关闭文件描述符
    if (*fd_ptr >= 0) {
        close(*fd_ptr);
        *fd_ptr = -1;
        MLOG_DBG("Closed file descriptor for hardware device %d\n", device_id);
    }

    // 删除LVGL输入设备（但不释放指针，以便重新启用）
    if (*indev_ptr != NULL) {
        // 注意：这里我们不删除indev对象，只是重置文件描述符
        // 实际的删除操作会在global_hardware_indev_delete_cb中处理
        MLOG_DBG("LVGL input device for hardware device %d remains created\n", device_id);
    }

    MLOG_DBG("Hardware input device %d disabled successfully\n", device_id);
    return 0;
}

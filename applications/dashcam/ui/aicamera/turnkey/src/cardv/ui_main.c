#define DEBUG
#include <sys/prctl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "config.h"
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"
#include "wifi_core.h"

#include "ui_common.h"
#include "osal.h"
#include "cvi_log.h"

#include <linux/input.h>
#include <fcntl.h>
#include "indev.h"

lv_ui_t g_ui;
static OSAL_TASK_HANDLE_S ui_task;
static bool s_buiInit    = false;
static bool g_ui_running = false;
// static OSAL_TASK_HANDLE_S      ui_task;

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

static const char *getenv_default(const char *name, const char *dflt)
{
    return getenv(name) ?: dflt;
}

static void lv_linux_disp_init(void)
{
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", FB_DEV_NAME);
    lv_display_t *disp = lv_linux_fbdev_create();

    lv_linux_fbdev_set_file(disp, device);
    lv_display_set_resolution(disp, H_RES, V_RES);

    lv_indev_t *touch = lv_evdev_create(LV_INDEV_TYPE_POINTER, TOUCH_PANEL_EVENT_PATH);
    lv_indev_set_display(touch, disp);

    // lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_layer_bottom(), LV_OPA_TRANSP, LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_0, LV_PART_MAIN);
}

// 全局硬件按键管理
static int global_input_fd               = -1;
static lv_indev_t *global_hardware_indev = NULL;

// 统一的硬件按键读取回调函数
static void global_hardware_key_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    // 读取按键事件
    struct input_event ev;
    int rd = read(global_input_fd, &ev, sizeof(ev));

    if(rd == sizeof(ev)) {
        if(ev.type == EV_KEY) {
            MLOG_DBG("Key event: code=%d, value=%d\n", ev.code, ev.value);

            // 检测按键按下
            bool pressed = (ev.value == 1);
            if(pressed) {
                // 调用当前页面的按键处理回调
                if(current_page_key_handler != NULL) {
                    current_page_key_handler(ev.code, ev.value);
                }
            }
        }
    }
}

// 硬件按键输入设备删除回调函数
static void global_hardware_indev_delete_cb(lv_event_t *e)
{
    if(global_input_fd >= 0) {
        close(global_input_fd);
        global_input_fd = -1;
        MLOG_DBG("Global hardware button input device closed\n");
    }
}

// 创建全局硬件按键输入设备
static void create_global_hardware_input_device(void)
{
    const char *devices = KEY_EVENT_PATH;
    printf("devices: %s\n", devices);
    // 打开输入设备
    global_input_fd = open(devices, O_RDONLY | O_NONBLOCK);
    if(global_input_fd < 0) {
        MLOG_ERR("Failed to open input device: %s\n", devices);
        return;
    }

    MLOG_DBG("Global hardware button input device opened\n");

    // 创建LVGL输入设备
    global_hardware_indev = lv_indev_create();
    lv_indev_set_type(global_hardware_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(global_hardware_indev, global_hardware_key_read_cb);
    lv_indev_set_driver_data(global_hardware_indev, NULL);
    lv_indev_add_event_cb(global_hardware_indev, global_hardware_indev_delete_cb, LV_EVENT_DELETE, NULL);

    MLOG_DBG("Global hardware button input device created\n");
    printf("global_hardware_indev: %p\n", global_hardware_indev);
}

// 删除全局硬件按键输入设备
void delete_global_hardware_input_device(void)
{
    if(global_hardware_indev != NULL) {
        lv_indev_delete(global_hardware_indev);
        global_hardware_indev = NULL;
        MLOG_DBG("Global hardware button input device deleted\n");
    }
}

static void start_uiapp(void *arg)
{
    UNUSED(arg);
    lv_init();

    // 初始化wpa_ctrl
    init_wpa_ctrl();
    // 初始化字体样式
    init_fonts_style();

    /*Linux display device init*/
    lv_linux_disp_init();

    // 创建全局硬件按键输入设备
    create_global_hardware_input_device();

    /* Create a GUI-Guider app */
    setup_ui(&g_ui);
    custom_init(&g_ui);

    /*Handle LVGL tasks*/
    g_ui_running = true;
    while(g_ui_running) {
        lv_timer_handler();
        usleep(5000);
    }
}

int UIAPP_Start(void)
{

    if(s_buiInit == false) {
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
    // 删除全局硬件按键输入设备
    delete_global_hardware_input_device();

    g_ui_running = false;
    OSAL_TASK_Join(ui_task);
    OSAL_TASK_Destroy(&ui_task);
    s_buiInit = false;
    return 0;
}

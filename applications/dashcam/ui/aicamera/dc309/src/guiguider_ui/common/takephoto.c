#define DEBUG
#include "takephoto.h"
#include "config.h"
#include "custom.h"
#include "cvi_af.h"
#include "cvi_comm_vo.h"
#include "cvi_vo.h"
#include "indev.h"
#include "lvgl.h"
#include "mlog.h"
#include "page_all.h"
#include "storagemng.h"
#include "ui_common.h"
#include "voiceplay.h"
#include "zoom_bar.h"
#include <linux/input.h>

// 分辨率枚举值用于数组索引时，确保枚举值连续且从0开始
#define PHOTO_RES_CNT 5 // 照片分辨率数量
#define QUALITY_CNT 3 // 画质等级数量
#define VIDEO_RES_CNT 4 // 录像分辨率数量

#define MAX_VIDEO_BYTE_PER_SEC_4K 4096000
#define MAX_VIDEO_BYTE_PER_SEC_2_7K 2048000
#define MAX_VIDEO_BYTE_PER_SEC_FULL 1024000
#define MAX_VIDEO_BYTE_PER_SEC_HD 1024000

// 记录不同分辨率和画质下照片的估算大小(单位:Byte)
// 拍照方法，自拍，对着人脸
static const uint32_t photo_bytes[PHOTO_RES_CNT][QUALITY_CNT] = {
    // {超高画质，高画质，普通画质}
    { 11114905, 5557452, 4613734 }, // 64M
    { 9227468, 4718592, 3879731 }, // 48M
    { 5557452, 2726297, 2359296 }, // 24M
    { 3250585, 1572864, 1363148 }, // 12M
    { 2726297, 1279262, 1121976 }, // 8M
};

// 录像每秒字节数数组，单位:Byte
static const uint32_t video_bytes_per_sec[VIDEO_RES_CNT] = {
    MAX_VIDEO_BYTE_PER_SEC_4K, // 4K
    MAX_VIDEO_BYTE_PER_SEC_2_7K, // 2.7K
    MAX_VIDEO_BYTE_PER_SEC_FULL, // FULL
    MAX_VIDEO_BYTE_PER_SEC_HD, // HD
};

// 录像剩余时间
static char remain_time_of_video[32] = {0};
// 对焦相关全局变量
static lv_timer_t* focus_timer = NULL; // 对焦延时定时器
static bool focus_pending = false; // 是否等待执行对焦
static lv_timer_t* focus_lock_timer = NULL; // 对焦锁定定时器

// 取景框相关全局变量
static lv_obj_t* g_viewfinder = NULL; // 取景框对象

// 拍照后处理回调函数
static takephoto_callback_t g_takephoto_after_callback = NULL;
// 拍照前执行回调
static takephoto_callback_t g_takephoto_before_callback = NULL;

bool is_video_mode = false;
uint8_t is_start_video = VEDIO_STOP;     //录像状态

extern menu_callback_t g_menu_callback;
extern mode_callback_t g_mode_callback;
extern ok_callback_t g_ok_callback;
extern long_menu_callback_t g_long_menu_callback;
extern long_menu_callback_t g_long_mode_callback;
extern play_callback_t g_play_callback;
extern zoomin_callback_t g_zoomin_callback;
extern zoomout_callback_t g_zoomout_callback;
extern zoomout_callback_t g_down_callback ;
extern zoomout_callback_t g_up_callback;
extern zoomout_callback_t g_left_callback;
extern zoomout_callback_t g_right_callback;
extern power_callback_t g_power_callback;

// 长按菜单按键检测相关变量
extern lv_timer_t *menu_long_press_timer;
extern bool menu_long_press_triggered;
// 长按模式按键检测相关变量
extern lv_timer_t *mode_long_press_timer;
extern bool mode_long_press_triggered;
// 长按zoomin按键检测相关变量
extern lv_timer_t *zoomin_long_press_timer;
extern bool zoomin_long_press_triggered;
// 长按zoomout按键检测相关变量
extern lv_timer_t *zoomout_long_press_timer;
extern bool zoomout_long_press_triggered;
// 长按对焦
static bool focus_long_press_flag = false;
// 录像模式标定focus检测标志, 当标志都置为1时可以标定focus
static int32_t g_left_focus_turnning_flag = 0;
static int32_t g_right_focus_lock_flag = 0;
// 对焦功能使能
static int32_t g_focus_enable = 1;

// 计算剩余可拍照片数量, 根据最大分辨率估算
uint32_t photo_CalculateRemainingPhotoCount(void)
{
    uint32_t remainingPhotoCount = 0;

    // 获取SD卡信息
    STG_FS_INFO_S stFSInfo = {0};
    int32_t s32Ret = STORAGEMNG_GetFSInfo(&stFSInfo);
    if (s32Ret == 0) {
        // 获取可用空间（字节）
        uint64_t availableSpaceBytes = stFSInfo.u64AvailableSize;
        // 获取SD卡总空间
        uint64_t totalSpaceBytes = stFSInfo.u64TotalSize;

        // 添加边界检查
        if (totalSpaceBytes == 0) {
            MLOG_ERR("Total storage space is 0");
            return 0;
        }

        // 计算reserve space (5%)
        uint64_t reserveSpaceBytes = totalSpaceBytes * 0.05; // 等同于 * 0.05

        // MLOG_DBG("BUG调试 reserveSpaceBytes:%llu kb availableSpaceBytes:%llu totalSpaceBytes:%llu\n",reserveSpaceBytes>>10,availableSpaceBytes>>10,totalSpaceBytes>>10);
        // 确保可用空间不会下溢
        if (availableSpaceBytes > reserveSpaceBytes) {
            availableSpaceBytes = availableSpaceBytes - reserveSpaceBytes;
        } else {
            availableSpaceBytes = 0;
        }

        // 估算单张照片文件大小（字节）
        // 枚举值直接作为数组索引，枚举值必须从0开始且连续
        uint8_t res_idx = photo_getRes_Index();
        uint8_t qual_idx = photo_getQuality_Index();

        // 添加边界检查，防止数组越界
        if (res_idx >= PHOTO_RES_CNT) {
            res_idx = PHOTO_RES_CNT - 1; // 默认使用最后一个分辨率
        }
        if (qual_idx >= QUALITY_CNT) {
            qual_idx = QUALITY_CNT - 1; // 默认使用最后一个画质
        }

        uint32_t estimatedPhotoSizeBytes = photo_bytes[res_idx][qual_idx];
        // MLOG_DBG("res_idx=%u, qual_idx=%u, estimatedPhotoSizeBytes=%u bytes\n",
        //     res_idx, qual_idx, estimatedPhotoSizeBytes);

        // 计算剩余可拍照片数量，添加溢出保护
        if (estimatedPhotoSizeBytes > 0 && availableSpaceBytes > 0) {
            uint64_t tempCount = availableSpaceBytes / estimatedPhotoSizeBytes;

            // 限制最大值，防止溢出
            if (tempCount > UINT32_MAX) {
                remainingPhotoCount = UINT32_MAX;
            } else {
                remainingPhotoCount = (uint32_t)tempCount;
            }
        } else {
            remainingPhotoCount = 0;
        }

        // MLOG_DBG("Photo count calc: space=%llu bytes, photo_size=%u bytes, remaining_count=%u\n",
        //           availableSpaceBytes, estimatedPhotoSizeBytes, remainingPhotoCount);
    }

    return remainingPhotoCount;
}
// 计算剩余可录像时间, 根据最大分辨率估算
char * video_Calculateremainingvideo(void)
{
    uint32_t remainingvideo = 0;
    // 获取SD卡信息
    STG_FS_INFO_S stFSInfo = {0};
    int32_t s32Ret = STORAGEMNG_GetFSInfo(&stFSInfo);
    if (s32Ret == 0) {
        // 获取可用空间（字节）
        uint64_t availableSpaceBytes = stFSInfo.u64AvailableSize;
        // 获取SD卡总空间
        uint64_t totalSpaceBytes = stFSInfo.u64TotalSize;

        // 添加边界检查
        if (totalSpaceBytes == 0) {
            MLOG_ERR("Total storage space is 0");
            snprintf(remain_time_of_video, sizeof(remain_time_of_video), "0:00:00");
            return remain_time_of_video;
        }

        // 计算reserve space (5%)
        uint64_t reserveSpaceBytes = totalSpaceBytes * 0.05;
        // 确保可用空间不会下溢
        if (availableSpaceBytes > reserveSpaceBytes) {
            availableSpaceBytes = availableSpaceBytes - reserveSpaceBytes;
        } else {
            availableSpaceBytes = 0;
        }

        // 估算单秒视频文件大小（字节）
        // 枚举值直接作为数组索引，枚举值必须从0开始且连续
        uint8_t video_res_idx = video_getRes_Index();

        // 添加边界检查，防止数组越界
        if (video_res_idx >= VIDEO_RES_CNT) {
            video_res_idx = 0; // 默认使用第一个分辨率
        }

        uint32_t estimatedvideoSizeBytes = video_bytes_per_sec[video_res_idx];

        // 计算剩余可录时间
        if (estimatedvideoSizeBytes > 0) {
            remainingvideo = (uint32_t)(availableSpaceBytes / estimatedvideoSizeBytes);
        }

        // 计算时、分、秒
        uint32_t hours   = remainingvideo / 3600;
        uint32_t minutes = (remainingvideo % 3600) / 60;
        uint32_t seconds = remainingvideo % 60;
        snprintf(remain_time_of_video, sizeof(remain_time_of_video), "%d:%d:%d", hours, minutes, seconds);
        // MLOG_ERR("time calc: space=%llu bytes, photo_size=%u bytes, remaining_count=%u\n",
        //           availableSpaceBytes, estimatedvideoSizeBytes, remainingvideo);
    } else {
        MLOG_ERR("get fs info failed, use default value 00:00:00\n");
        snprintf(remain_time_of_video, sizeof(remain_time_of_video), "00:00:00");
    }

    return remain_time_of_video;
}

void video_get_status(uint8_t* status){
    *status = is_start_video;
}

// 长zoomin按键检测定时器回调函数
static uint8_t entyr_count = 0;
void zoomin_long_press_timer_cb(lv_timer_t *t)
{
    uint32_t zoom_level = get_zoom_level();
    entyr_count++;
    if (entyr_count >= 2) {
        zoomin_long_press_triggered = true;
        if (g_zoomin_callback != NULL) {
            g_zoomin_callback();
        }
    }
    if(get_arc_handel() != NULL) {
        zoomin_long_press_triggered = true;
        if(g_zoomin_callback != NULL) {
            g_zoomin_callback();
        }
        lv_timer_set_period(t, 100);
        lv_timer_reset(t);
    } else if(zoom_level < ZOOM_RADIO_MAX) {
        zoom_level++;
        set_zoom_level(zoom_level);
        MLOG_DBG("long zoomin level:%d\n", zoom_level);
        // 定时器触发说明按键已经持续按下300ms，执行长按逻辑
        zoomin_long_press_triggered = true;
        // 放大
        if (g_zoomin_callback != NULL) {
            g_zoomin_callback();
        }
        // 100ms周期继续检测, 直到按键松开
        lv_timer_set_period(t, 100);
        lv_timer_reset(t);
    }
}

// 长zoomout按键检测定时器回调函数
void zoomout_long_press_timer_cb(lv_timer_t *t)
{
    uint32_t zoom_level = get_zoom_level();
    entyr_count++;
    if (entyr_count >= 2) {
        zoomout_long_press_triggered = true;
        if(g_zoomout_callback != NULL) {
            g_zoomout_callback();
        }
    }
    if(get_arc_handel() != NULL) {
        zoomout_long_press_triggered = true;
        if(g_zoomout_callback != NULL) {
            g_zoomout_callback();
        }
        lv_timer_set_period(t, 100);
        lv_timer_reset(t);
    } else if(zoom_level > 1) {
        zoom_level--;
        set_zoom_level(zoom_level);
        MLOG_DBG("long zoomout level:%d\n", zoom_level);
        // 定时器触发说明按键已经持续按下300ms，执行长按逻辑
        zoomout_long_press_triggered = true;
        // 放大
        if (g_zoomout_callback != NULL) {
            g_zoomout_callback();
        }
        // 100ms周期继续检测, 直到按键松开
        lv_timer_set_period(t, 100);
        lv_timer_reset(t);
    }
}

static bool led_on_flag = false;
int32_t do_zoom_in(int32_t key_value)
{
    if(key_value == 1){
        // 创建长按检测定时器, 300ms认为是长按
        zoomin_long_press_triggered = false;
        zoomin_long_press_timer = lv_timer_create(zoomin_long_press_timer_cb, 300, NULL);
    } else {
        entyr_count = 0;
        // 短按或者长按，在按键释放的时候都要删除定时器
        if(zoomin_long_press_timer != NULL) {
            lv_timer_del(zoomin_long_press_timer);
            zoomin_long_press_timer = NULL;
        }
        // 短按触发一次放大
        if (!zoomin_long_press_triggered) {
            MLOG_DBG("zoomin按键短按, 执行短按逻辑\n");
            if (g_up_callback != NULL) {
                MLOG_DBG("BUG调试 %d %d\n",led_on_flag,brightness_level);
                if (led_on_flag == false) {
                    ircut_on();
                    led_on();
                    led_on_flag = true;
                }
                if (led_on_flag == true) {
                    int8_t level = brightness_level;
                    level += 1;
                    if (level >= 7) {
                        level = 7;
                    }
                    led_on_with_brightness(level);
                }
                g_up_callback();
            }
        }
    }
    return 0;
}

int32_t do_zoom_out(int32_t key_value)
{
    if(key_value == 1){
        // 创建长按检测定时器, 300ms认为是长按
        zoomout_long_press_triggered = false;
        zoomout_long_press_timer = lv_timer_create(zoomout_long_press_timer_cb, 300, NULL);
    } else {
        entyr_count = 0;
        // 短按或者长按，在按键释放的时候都要删除定时器
        if(zoomout_long_press_timer != NULL) {
            lv_timer_del(zoomout_long_press_timer);
            zoomout_long_press_timer = NULL;
        }
        // 短按触发一次缩小
        if (!zoomout_long_press_triggered) {
            MLOG_DBG("zoomout按键短按, 执行短按逻辑\n");
            if (g_down_callback != NULL) {
                MLOG_DBG("执行自定义zoomout处理逻辑\n");
                if (led_on_flag == true) {
                    int8_t level = brightness_level;
                    level -= 1;
                    if(level <= 0) {
                        level = 0;
                    }
                    led_on_with_brightness(level);
                }

                if(brightness_level == 0) {
                    led_off();
                    ircut_off();
                    led_on_flag = false;
                }
                g_down_callback();
            }
        }
    }
    return 0;
}

// 删除SD卡提示标签的回调函数
static void sd_not_ready_timer_cb(lv_timer_t *t)
{
    lv_obj_t *label = (lv_obj_t *)t->user_data;
    lv_obj_del(label);
    lv_timer_del(t);
}

// SD卡提示标签
void sd_card_status_label(void)
{
    extern lv_style_t ttf_font_30;
    lv_obj_t* label_card_status = lv_label_create(lv_layer_top());
    //SD卡状态在线并且获取的数量为0时，提示空间已满
    if (ui_event_type == EVT_SDCARD_SPACE_FULL || (photo_CalculateRemainingPhotoCount() == 0 && ui_common_cardstatus())) {
        MLOG_ERR("SD卡空间已满,无法拍照/录像\n");
        lv_label_set_text(label_card_status, "空间已满!!!\n");
    } else {
        MLOG_ERR("SD卡未就绪,无法拍照/录像\n");
        lv_label_set_text(label_card_status, "请插入SD卡!!!\n");
    }
    lv_obj_add_style(label_card_status, &ttf_font_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(label_card_status, lv_color_hex(0xFF0000), 0);
    lv_obj_center(label_card_status);

    // 创建定时器，2秒后删除标签
    lv_timer_create(sd_not_ready_timer_cb, 2000, label_card_status);
}

void enable_focus(void)
{
    g_focus_enable = 1;
    MLOG_INFO("enable focus\n");
}

void disable_focus(void)
{
    g_focus_enable = 0;
    // 取消取景框,防止切换的时候刚好有对焦框
    cancel_viewfinder();
    MLOG_INFO("disable focus\n");
}

// 对焦延时定时器回调函数
static void focus_timer_cb(lv_timer_t *t)
{
    // 延时执行对焦的回调
    MLOG_DBG("执行对焦\n");

    // 调用对焦逻辑
    CVI_S32 ret;
    ISP_FOCUS_Q_INFO_S *pbuf = (ISP_FOCUS_Q_INFO_S *)malloc(sizeof(ISP_FOCUS_Q_INFO_S));
    ret = CVI_ISP_AFQueryFocusInfo(0, pbuf);
    if (ret != CVI_SUCCESS) {
        MLOG_ERR("查询对焦信息失败\n");
    }
    MLOG_DBG("对焦状态: %d\n", pbuf->eStatus);
    if(pbuf->eStatus != AF_INIT) {
        MLOG_DBG("对焦中\n");
    } else {
        ret = CVI_ISP_AFAutoFocus(0);
        if (ret != CVI_SUCCESS) {
            MLOG_ERR("对焦操作失败\n");
        } else {
            MLOG_DBG("对焦操作成功\n");
        }
    }
    // 删除定时器，确保只执行一次
    lv_timer_del(t);
    if(!focus_long_press_flag) {
        cancel_viewfinder(); // 取消取景框
        MESSAGE_S Msg = {0};
        Msg.topic     = EVENT_MODEMNG_FOCUS;
        MODEMNG_SendMessage(&Msg);
    }
    // 清理状态
    focus_pending = false;
    focus_timer = NULL;
    free(pbuf);
}

// 对焦锁定延时定时器回调函数
static void focus_lock_timer_cb(lv_timer_t *t)
{
    // 延时执行对焦的回调
    MLOG_INFO("in\n");
    CVI_S32 ret = CVI_SUCCESS;
    ISP_FOCUS_Q_INFO_S focus_info = {0};
    ISP_FOCUS_ATTR_S focus_attr = {0};
    PARAM_MEDIA_COMM_S mediacomm = {0};
    ret = CVI_ISP_AFQueryFocusInfo(0, &focus_info);
    if (ret != CVI_SUCCESS) {
        MLOG_ERR("ISP_AFQueryFocusInfo failed with %#x\n", ret);
    }
    MLOG_INFO("focus status: %d\n", focus_info.eStatus);
    // 对焦完成后设置为自动对焦
    if(focus_info.eStatus == AF_INIT){//AF_FOCUSED AF_INIT
        // 对焦完成删除定时器，确保只执行一次
        lv_timer_del(t);
        focus_lock_timer = NULL;
        MLOG_INFO("focus locked\n");
        ret = CVI_ISP_GetAFAttr(0, &focus_attr);
        if(ret != CVI_SUCCESS){
            MLOG_ERR("ISP_GetAFAttr failed with %#x\n", ret);
        }
        focus_attr.enOpType = OP_TYPE_MANUAL;
        focus_attr.stManual.enManualOpType = OP_TYPE_FOCUS_POS;
        focus_attr.stManual.u16ManualPos = focus_info.u16FocusPos;
        ret = CVI_ISP_SetAFAttr(0, &focus_attr);
        if(ret != CVI_SUCCESS){
            MLOG_ERR("ISP_SetAFAttr failed with %#x\n", ret);
        }
        /* 获取当前对焦位置 */
        MLOG_INFO("focus pos:%d l:%d r:%d",
                focus_info.u16FocusPos, g_left_focus_turnning_flag, g_right_focus_lock_flag);
        /* 锁定 */
        if(g_right_focus_lock_flag) {
            PARAM_GetMediaComm(&mediacomm);
            if(!mediacomm.Record.ChnAttrs[0].FocusPosLock) {
                mediacomm.Record.ChnAttrs[0].FocusPosLock = 1;
                PARAM_SetMediaComm(&mediacomm);
                PARAM_SetSaveFlg();
                /* 蓝色 */
                set_viewfinder_color(0x0000FF);
            } else {
                /* 绿色 */
                set_viewfinder_color(0x1afa29);
            }
            g_right_focus_lock_flag = 0;
        } else if (g_left_focus_turnning_flag) {
            MLOG_INFO("save focus pos");
            PARAM_GetMediaComm(&mediacomm);
            /* 未锁定可以修改值 */
            if(!mediacomm.Record.ChnAttrs[0].FocusPosLock) {
                mediacomm.Record.ChnAttrs[0].FocusPos = focus_info.u16FocusPos;
                PARAM_SetMediaComm(&mediacomm);
                PARAM_SetSaveFlg();
                /* 红色 */
                set_viewfinder_color(0xFF0000);
            } else {
                /* 绿色 */
                set_viewfinder_color(0x1afa29);
            }
            g_left_focus_turnning_flag = 0;
        } else {
            /* 绿色 */
            set_viewfinder_color(0x1afa29);
        }

        MESSAGE_S Msg = { 0 };
        Msg.topic = EVENT_MODEMNG_FOCUS;
        MODEMNG_SendMessage(&Msg);
    }else{
        // 对焦未完成则重新设置定时器进行周期检测
        MLOG_INFO("focus recheck\n");
        lv_timer_set_period(t, 200);
    }
}

// 取消对焦锁定
static void cancel_focus_lock(void)
{
    CVI_S32 ret = CVI_SUCCESS;
    ISP_FOCUS_ATTR_S focus_attr = {0};
    MLOG_INFO("cancel focus lock\n");
    if (focus_lock_timer) {
        lv_timer_del(focus_lock_timer);
        focus_lock_timer = NULL;
        // 设置为自动对焦
    }
    ret = CVI_ISP_GetAFAttr(0, &focus_attr);
    if(ret != CVI_SUCCESS){
        MLOG_ERR("ISP_GetAFAttr failed with %#x\n", ret);
    }
    if(focus_attr.enOpType != OP_TYPE_AUTO){
        focus_attr.enOpType = OP_TYPE_AUTO;
        focus_attr.stManual.enManualOpType = OP_TYPE_AUTO_FOCUS;
        ret = CVI_ISP_SetAFAttr(0, &focus_attr);
        if(ret != CVI_SUCCESS){
            MLOG_ERR("ISP_SetAFAttr failed with %#x\n", ret);
        }
    }
}

// 拍照按键处理回调函数
void takephoto_key_handler(int key_code, int key_value)
{

    //不是录像按键&&是录像模式&&当前正在录像 就丢弃按键事件
    if ((key_code == KEY_POWER || key_code == KEY_PLAY) && is_video_mode && is_start_video == VEDIO_START) {
        return;
    }

    if (key_code == KEY_OK && key_value == 1) {
        // KEY_CAMERA 按下
        MLOG_DBG("收到拍照/录像事件\n");
        if (!is_video_mode&&focus_pending) {
            MLOG_DBG("对焦中，忽略拍照事件\n");
            return;
        }
        if(ui_common_cardstatus()) {
            if(ui_event_type == EVT_SDCARD_SPACE_FULL || photo_CalculateRemainingPhotoCount() == 0) {
                // 提示用户SD卡空间已满
                sd_card_status_label();
                return;
            }
            MESSAGE_S Msg = {0};

            if(is_video_mode) {
                MLOG_DBG("录像模式\n");
                is_start_video++;
                if(is_start_video > VEDIO_START) {
                    is_start_video = VEDIO_STOP;
                }
                // 调用录像逻辑
                if(is_start_video == VEDIO_START) {
                    MLOG_DBG("VEDIO_START\n");
                    // 开始录像

                    // 通知底层 开始录像
                    Msg.topic = EVENT_MODEMNG_START_REC;
                    MODEMNG_SendMessage(&Msg);
                    video_effect_scr_delete();
                    MLOG_DBG("开始录像\n");
                    // 开始录像时，取消TP事件
                    disable_touch_events(); // 屏蔽tp事件
                    // 开始录像时，关闭音频输出
                    // UI_VOICEPLAY_DeInit(NULL);
                } else if (is_start_video == VEDIO_STOP) {
                    MLOG_DBG("VEDIO_PAUSE\n");
                    // 停止录像

                    // 通知底层 停止录像
                    Msg.topic = EVENT_MODEMNG_STOP_REC;
                    MODEMNG_SendMessage(&Msg);
                    enable_touch_events(); // 停止录像，恢复TP事件
                    // 停止录像时，开启音频输出
                    // UI_VOICEPLAY_Init();
                }
            } else {
                MLOG_DBG("拍照模式\n");
                if(g_takephoto_before_callback != NULL) {
                    g_takephoto_before_callback();
                }
                // 调用拍照逻辑
                Msg.topic = EVENT_MODEMNG_START_PIV;
                MODEMNG_SendMessage(&Msg);
                ui_common_wait_piv_end();

                // 执行自定义的拍照后处理逻辑
                if (g_takephoto_after_callback != NULL && access(pic_filepath, F_OK) == 0) {
                    MLOG_DBG("执行自定义拍照后处理逻辑\n");
                    g_takephoto_after_callback();
                }
            }
        } else {
                sd_card_status_label();
        }
    } else if (key_code == KEY_MENU) {
        if (key_value == 1) {
            // 菜单按键按下
            menu_long_press_triggered = false;
            MLOG_DBG("菜单按键按下，开始长按检测\n");

            // 创建长按检测定时器，300ms后触发长按事件
            menu_long_press_timer = lv_timer_create(menu_long_press_timer_cb, 300, NULL);
        } else if (key_value == 0) {
            // 菜单按键释放
            if (menu_long_press_timer && !menu_long_press_triggered) {
                // 如果定时器还在运行且未触发长按事件，说明是短按
                lv_timer_del(menu_long_press_timer);
                menu_long_press_timer = NULL;

                MLOG_DBG("菜单按键短按，执行短按逻辑\n");
                if (g_menu_callback != NULL) {
                    MLOG_DBG("执行自定义菜单处理逻辑\n");
                    g_menu_callback();
                }
            }
        }
    } else if (key_code == KEY_PLAY && key_value == 1) {
        if (g_play_callback != NULL) {
            MLOG_DBG("执行AI按键处理逻辑\n");
            g_play_callback();
        }
    } else if (key_code == KEY_DOWN) // 快捷删除按键
    {
        do_zoom_out(key_value);
    } else if (key_code == KEY_UP) {
        do_zoom_in(key_value);
    } else if (key_code == KEY_POWER && key_value == 1) {
        if (g_power_callback != NULL) {
            MLOG_DBG("执行电源按键处理逻辑\n");
            g_power_callback();
        }
    }
}

// 注册拍照后处理回调函数
void takephoto_register_callback(takephoto_callback_t callback)
{
    g_takephoto_after_callback = callback;
}

// 取消注册拍照后处理回调函数
void takephoto_unregister_callback(void)
{
    g_takephoto_after_callback = NULL;
}
//注册拍照之前回调函数
void takephoto_register_before_callback(takephoto_callback_t callback)
{
    g_takephoto_before_callback = callback;
}
// 取消注册拍照前处理回调函数
void takephoto_unregister_before_callback(void)
{
    g_takephoto_before_callback = NULL;
}

// 拍照按键处理回调函数（带重复按下检测）
void takephoto_delay_handler(int key_code, int key_value)
{
    // 静态变量记录上次KEY_CAMERA按键时间
    static uint32_t last_camera_time = 0;

    // 获取当前时间
    uint32_t current_time = lv_tick_get();
    MLOG_DBG("dur_tim:%d\n",current_time - last_camera_time);
    // 仅对KEY_CAMERA按键进行重复按下检测
    if (key_code == KEY_OK && key_value == 1) {
        // 检测规则：相同按键在300ms内重复按下视为无效
        if ((current_time - last_camera_time) < 300) {
            MLOG_DBG("重复按下KEY_CAMERA已忽略\n");
            return;
        }
        // 更新按键记录
        last_camera_time = current_time;
    }

    if(key_code == KEY_OK && key_value == 1) {
        // KEY_CAMERA 按下（已通过重复按下检测）
        MLOG_DBG("收到拍照/录像事件\n");
        if(focus_pending) {
            MLOG_DBG("对焦中，忽略拍照事件\n");
            return;
        }
        if(ui_common_cardstatus()) {
            if(ui_event_type == EVT_SDCARD_SPACE_FULL) {
                // 提示用户SD卡空间已满
                sd_card_status_label();
                return;
            }
            // 执行自定义的拍照后处理逻辑
            if(g_takephoto_after_callback != NULL) {
                MLOG_DBG("执行自定义拍照后处理逻辑\n");
                g_takephoto_after_callback();
            }
        } else {
            sd_card_status_label();
        }
    } if (key_code == KEY_PLAY && key_value == 1) {
        if (g_play_callback != NULL) {
            MLOG_DBG("执行AI按键处理逻辑\n");
            g_play_callback();
        }
    } else if (key_code == KEY_DOWN) // 快捷删除按键
    {
        do_zoom_out(key_value);
    } else if (key_code == KEY_UP) {
        do_zoom_in(key_value);
    } else if (key_code == KEY_POWER && key_value == 1) {
        if (g_power_callback != NULL) {
            MLOG_DBG("执行电源按键处理逻辑\n");
            g_power_callback();
        }
    }
}

// ==================== 取景框相关函数 ====================

// 创建取景框
void create_viewfinder(lv_obj_t *parent)
{
    // 如果已存在取景框，先销毁旧的
    if(g_viewfinder != NULL && lv_obj_is_valid(g_viewfinder)) {
        MLOG_DBG("销毁旧的取景框对象\n");
        lv_obj_del(g_viewfinder);
        g_viewfinder = NULL;
    }

    // 创建取景框容器
    g_viewfinder = lv_obj_create(parent);
    lv_obj_set_size(g_viewfinder, H_RES, V_RES);
    lv_obj_set_style_bg_opa(g_viewfinder, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_viewfinder, 0, 0);
    lv_obj_set_style_pad_all(g_viewfinder, 0, 0);
    lv_obj_clear_flag(g_viewfinder, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(g_viewfinder, LV_OBJ_FLAG_CLICKABLE);

    // 计算取景框尺寸和位置
    const lv_coord_t frame_width  = H_RES * 0.3; // 取景框宽度
    const lv_coord_t frame_height = V_RES * 0.3; // 取景框高度
    const lv_coord_t frame_x      = (H_RES - frame_width) / 2;
    const lv_coord_t frame_y      = (V_RES - frame_height) / 2;

    // 角标记参数
    const lv_coord_t corner_size      = 20; // 角标记大小
    const lv_coord_t corner_thickness = 4;  // 角标记线宽

    // 创建四个角的标记
    // 左上角
    lv_obj_t *corner_tl = lv_obj_create(g_viewfinder);
    lv_obj_set_size(corner_tl, corner_size + 2, corner_size + 2);
    lv_obj_set_pos(corner_tl, frame_x, frame_y);
    lv_obj_set_style_bg_opa(corner_tl, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(corner_tl, 0, 0);
    lv_obj_set_style_pad_all(corner_tl, 0, 0);
    lv_obj_set_style_shadow_width(corner_tl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 左上角 - 水平线
    lv_obj_t *line_tl_h                     = lv_line_create(corner_tl);
    static lv_point_precise_t points_tl_h[] = {{0, 1}, {corner_size, 1}};
    lv_line_set_points(line_tl_h, points_tl_h, 2);
    lv_obj_set_style_line_width(line_tl_h, corner_thickness, 0);
    lv_obj_set_style_line_color(line_tl_h, lv_color_white(), 0);

    // 左上角 - 垂直线
    lv_obj_t *line_tl_v                     = lv_line_create(corner_tl);
    static lv_point_precise_t points_tl_v[] = {{1, 0}, {1, corner_size}};
    lv_line_set_points(line_tl_v, points_tl_v, 2);
    lv_obj_set_style_line_width(line_tl_v, corner_thickness, 0);
    lv_obj_set_style_line_color(line_tl_v, lv_color_white(), 0);

    // 右上角
    lv_obj_t *corner_tr = lv_obj_create(g_viewfinder);
    lv_obj_set_size(corner_tr, corner_size + 2, corner_size + 2);
    lv_obj_set_pos(corner_tr, frame_x + frame_width - corner_size, frame_y);
    lv_obj_set_style_bg_opa(corner_tr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(corner_tr, 0, 0);
    lv_obj_set_style_pad_all(corner_tr, 0, 0);
    lv_obj_set_style_shadow_width(corner_tr, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 右上角 - 水平线
    lv_obj_t *line_tr_h                     = lv_line_create(corner_tr);
    static lv_point_precise_t points_tr_h[] = {{1, 1}, {corner_size, 1}};
    lv_line_set_points(line_tr_h, points_tr_h, 2);
    lv_obj_set_style_line_width(line_tr_h, corner_thickness, 0);
    lv_obj_set_style_line_color(line_tr_h, lv_color_white(), 0);

    // 右上角 - 垂直线
    lv_obj_t *line_tr_v                     = lv_line_create(corner_tr);
    static lv_point_precise_t points_tr_v[] = {{corner_size, 0}, {corner_size, corner_size}};
    lv_line_set_points(line_tr_v, points_tr_v, 2);
    lv_obj_set_style_line_width(line_tr_v, corner_thickness, 0);
    lv_obj_set_style_line_color(line_tr_v, lv_color_white(), 0);

    // 左下角
    lv_obj_t *corner_bl = lv_obj_create(g_viewfinder);
    lv_obj_set_size(corner_bl, corner_size + 2, corner_size + 2);
    lv_obj_set_pos(corner_bl, frame_x, frame_y + frame_height - corner_size);
    lv_obj_set_style_bg_opa(corner_bl, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(corner_bl, 0, 0);
    lv_obj_set_style_pad_all(corner_bl, 0, 0);
    lv_obj_set_style_shadow_width(corner_bl, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 左下角 - 水平线
    lv_obj_t *line_bl_h                     = lv_line_create(corner_bl);
    static lv_point_precise_t points_bl_h[] = {{0, corner_size - 1}, {corner_size, corner_size - 1}};
    lv_line_set_points(line_bl_h, points_bl_h, 2);
    lv_obj_set_style_line_width(line_bl_h, corner_thickness, 0);
    lv_obj_set_style_line_color(line_bl_h, lv_color_white(), 0);

    // 左下角 - 垂直线
    lv_obj_t *line_bl_v                     = lv_line_create(corner_bl);
    static lv_point_precise_t points_bl_v[] = {{1, 0}, {1, corner_size}};
    lv_line_set_points(line_bl_v, points_bl_v, 2);
    lv_obj_set_style_line_width(line_bl_v, corner_thickness, 0);
    lv_obj_set_style_line_color(line_bl_v, lv_color_white(), 0);

    // 右下角
    lv_obj_t *corner_br = lv_obj_create(g_viewfinder);
    lv_obj_set_size(corner_br, corner_size + 2, corner_size + 2);
    lv_obj_set_pos(corner_br, frame_x + frame_width - corner_size, frame_y + frame_height - corner_size);
    lv_obj_set_style_bg_opa(corner_br, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(corner_br, 0, 0);
    lv_obj_set_style_pad_all(corner_br, 0, 0);
    lv_obj_set_style_shadow_width(corner_br, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // 右下角 - 水平线
    lv_obj_t *line_br_h                     = lv_line_create(corner_br);
    static lv_point_precise_t points_br_h[] = {{0, corner_size - 1}, {corner_size, corner_size - 1}};
    lv_line_set_points(line_br_h, points_br_h, 2);
    lv_obj_set_style_line_width(line_br_h, corner_thickness, 0);
    lv_obj_set_style_line_color(line_br_h, lv_color_white(), 0);

    // 右下角 - 垂直线
    lv_obj_t *line_br_v                     = lv_line_create(corner_br);
    static lv_point_precise_t points_br_v[] = {{corner_size-2, 0}, {corner_size-2, corner_size}};
    lv_line_set_points(line_br_v, points_br_v, 2);
    lv_obj_set_style_line_width(line_br_v, corner_thickness, 0);
    lv_obj_set_style_line_color(line_br_v, lv_color_white(), 0);

    // 将取景框移到背景层，避免遮挡其他控件
    lv_obj_move_background(g_viewfinder);

    lv_obj_add_flag(g_viewfinder, LV_OBJ_FLAG_HIDDEN);
}

// 显示取景框
void display_viewfinder(void)
{
    if(g_viewfinder == NULL || !lv_obj_is_valid(g_viewfinder)) {
        MLOG_ERR("取景框对象无效\n");
        return;
    }

    if(lv_obj_has_flag(g_viewfinder, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(g_viewfinder, LV_OBJ_FLAG_HIDDEN);
    }

    // 设置角标记为白色，中心十字为黄色
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(g_viewfinder); i++) {
        lv_obj_t *child = lv_obj_get_child(g_viewfinder, i);
        lv_obj_t *f_child = lv_obj_get_child(child, 0);
        lv_obj_t *d_child = lv_obj_get_child(child, 1);
        if(f_child != NULL) {
            // 角标记（有两条线的子对象）
            lv_obj_set_style_line_color(f_child, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_color(d_child, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

// 设置取景框颜色（对焦成功时变为绿色）
void set_viewfinder_color(uint32_t color)
{
    if(g_viewfinder == NULL || !lv_obj_is_valid(g_viewfinder)) {
        MLOG_ERR("取景框对象无效\n");
        return;
    }

    // 将所有线条设置为绿色
    for(uint8_t i = 0; i < lv_obj_get_child_cnt(g_viewfinder); i++) {
        lv_obj_t *child = lv_obj_get_child(g_viewfinder, i);
        lv_obj_t *f_child = lv_obj_get_child(child, 0);
        lv_obj_t *d_child = lv_obj_get_child(child, 1);
        if(f_child != NULL) {
            // 角标记
            lv_obj_set_style_line_color(f_child, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_line_color(d_child, lv_color_hex(color), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

// 取消/隐藏取景框
void cancel_viewfinder(void)
{
    if(g_viewfinder == NULL || !lv_obj_is_valid(g_viewfinder)) {
        MLOG_DBG("取景框对象无效或已销毁\n");
        return;
    }

    if(!lv_obj_has_flag(g_viewfinder, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_add_flag(g_viewfinder, LV_OBJ_FLAG_HIDDEN);
    }
}

void delete_viewfinder(void)
{
    if(g_viewfinder != NULL && lv_obj_is_valid(g_viewfinder)) {
        MLOG_DBG("销毁旧的取景框对象\n");
        lv_obj_del(g_viewfinder);
        g_viewfinder = NULL;
    }
}

void takephoto_cancel_focus(void)
{
    if(focus_lock_timer != NULL){
        lv_timer_del(focus_lock_timer);
        focus_lock_timer = NULL;
    }

    if(focus_timer != NULL){
        lv_timer_del(focus_timer);
        focus_timer = NULL;
        focus_pending = false;
    }
}

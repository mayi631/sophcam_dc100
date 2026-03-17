#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "mlog.h"

/************************** 屏幕相关 ******************************************/
#define FB_DEV_NAME "/dev/fb0"
#define H_RES 640 /* 水平分辨率 */
#define V_RES 360 /* 垂直分辨率 */
#define DISP_BUF_SIZE (H_RES * V_RES * 2)

/************************** INPUT EVENT 对应的文件路径 ************************/
#define KEY_EVENT_PATH_0 "/dev/input/adc-key1"
#define KEY_EVENT_PATH_1 "/dev/input/adc-key2"
#define TOUCH_PANEL_EVENT_PATH "/dev/input/touchscreen"
#define POWER_KEY_EVENT_PATH "/dev/input/power-key"

/************************** 按键对应的 KEY CODE ******************************/
#define AUDIO_KEY 2  /* 音频按键 */
#define POWER_KEY 3  /* 电源按键 */
#define CAMERA_KEY 4 /* 摄像头按键 */

/************************** 闪光灯GPIO配置 ************************************/
#define FLASH_GPIO_NUM 352  /* 闪光灯GPIO编号 pwr_gpio0 */

/************************** WIFI 列表展示的数量 *******************************/
#define WIFI_LIST_SHOW_NUM 20

// 自定义图片目录路径
// #define IMAGE_DIR "/home/ubuntu2004/CV184_DC309/zhuohao_ui/src/guiguider_ui/images"
#define IMAGE_DIR "/mnt/data/bin/res"

/************************** 相册图片路径  ************************************/
#define PHOTO_ALBUM_IMAGE_PATH "A:/mnt/sd/DCIM/PHOTO/" /* 原图路径 */
#define PHOTO_ALBUM_IMAGE_PATH_S "A:/mnt/sd/DCIM/.thumb/PHOTO_SMALL/" /* 提取小缩略图路径 */
#define PHOTO_ALBUM_IMAGE_PATH_L "A:/mnt/sd/DCIM/.thumb/PHOTO_LARGE/" /* 提取大缩略图路径 */

/************************** 相册录像路径  ************************************/
#define PHOTO_ALBUM_MOVIE_PATH "A:/mnt/sd/DCIM/MOVIE/" /* 原始视频路径 */
#define PHOTO_ALBUM_VIDEO_THUMB_PATH_S "A:/mnt/sd/DCIM/.thumb/MOVIE_SMALL/" /* 提取录像小缩略图路径 */
#define PHOTO_ALBUM_VIDEO_THUMB_PATH_L "A:/mnt/sd/DCIM/.thumb/MOVIE_LARGE/" /* 提取录像大缩略图路径 */

#define HIDDLE_THUMB_PATH "/mnt/sd/DCIM/.thumb/" /* 缩略图路径 */

/************************** IMAGE_RECOGNIZE 配置  ************************************/
#define IMAGE_RECOGNIZE_MODEL_NAME "Qwen2.5-VL-72B-Instruct"
#define IMAGE_RECOGNIZE_API_KEY "EIg_6FZelCDedHTE3U2mJM-gzOLSz0ZtTdzya1xh5BJH2QUWB_EV1I-efCRBWh5FV5Xm7OdpAuM_zygTQguWQQ"
#define IMAGE_RECOGNIZE_BASE_URL "https://www.sophnet.com/api/open-apis/v1/chat/completions"

/************************** IMAGE_TO_IMAGE 配置  ************************************/
#define IMAGE_TO_IMAGE_ENDPOINT "https://www.sophnet.com"
#define IMAGE_TO_IMAGE_PROJECT_UUID "default_project"
#define IMAGE_TO_IMAGE_EASYLLM_ID "img2img_model"
#define IMAGE_TO_IMAGE_ACCESS_KEY "AKLTYmI3MmE0OGJkYWFiNDIzOThmNmFmNWIxZWEwNzk3MWE"
#define IMAGE_TO_IMAGE_SECRET_KEY "T1RoaE5EbG1ZakZqTUdSbU5EUmhOamt4TTJSaU56VTBObVprTURVd1pHTQ=="
/* Sophnet image-edit 默认配置（img2img_create 调用时使用） */
#define IMAGE_TO_IMAGE_MODEL_NAME "Qwen-Image-Edit-Plus"
#define IMAGE_TO_IMAGE_API_KEY "EIg_6FZelCDedHTE3U2mJM-gzOLSz0ZtTdzya1xh5BJH2QUWB_EV1I-efCRBWh5FV5Xm7OdpAuM_zygTQguWQQ"
#define IMAGE_TO_IMAGE_DEFAULT_WIDTH 1280
#define IMAGE_TO_IMAGE_DEFAULT_HEIGHT 720
#define IMAGE_TO_IMAGE_DEFAULT_SIZE_STR "1280*720"

/************************** AI图片路径  ************************************/
#define AI_TMP_IMAGE_PATH "A:/tmp/ai_res.jpg"        /* AI拍照临时图片路径 */
#define AI_EFFECT_TMP_IMAGE_PATH "A:/tmp/ai_res.jpg" /* AI拍照临时图片路径 */
#define AI_EFFECT_OUT_IMAGE_PATH "A:/tmp/output.jpg" /* AI拍照临时图片路径 */

/************************** 版本信息  ************************************/
#define MAIN_VERSION "V1.0.0"   //主程序
#define KERNEL_VERSION "V1.0.0" //系统内核
#define DEVICE_MODEL_CODE "DC309E_IMX135_ST7701" //设备型号


/*****************************方正姚体加载路径**********************************/
// #define ALI_PUHUITI_FONTPATH "/mnt/sd/output/fonts/Alibaba-PuHuiTi-Light.ttf"
// #define ALI_PUHUITI_FONTPATH
// "/home/ubuntu2004/CV184_DC309/zhuohao_ui/third_party/freetype/fonts/HarmonyOS_Sans_SC_Regular.ttf"

#define ALI_PUHUITI_FONTPATH "/usr/local/fonts/HarmonyOS_Sans_SC_Regular.ttf"

#define ALI_PUHUITI_WEIGHT 32
/* TP 最大的触摸点数 */
#define MAX_COUNT 2

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#endif

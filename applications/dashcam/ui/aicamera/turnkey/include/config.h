#ifndef __CONFIG_H_
#define __CONFIG_H_

#include "mlog.h"

/************************** 屏幕相关 ******************************************/
#define FB_DEV_NAME "/dev/fb0"
#define H_RES 640 /* 水平分辨率 */
#define V_RES 480 /* 垂直分辨率 */
#define DISP_BUF_SIZE (H_RES * V_RES)

/************************** INPUT EVENT 对应的文件路径 ************************/
#define KEY_EVENT_PATH "/dev/input/adc-key2"
#define TOUCH_PANEL_EVENT_PATH "/dev/input/touchscreen"
#define POWER_KEY_EVENT_PATH "/dev/input/power-key"

/************************** 按键对应的 KEY CODE ******************************/
#define AUDIO_KEY 2  /* 音频按键 */
#define POWER_KEY 3  /* 电源按键 */
#define CAMERA_KEY 4 /* 摄像头按键 */

/************************** WIFI 列表展示的数量 *******************************/
#define WIFI_LIST_SHOW_NUM 20

/************************** 相册图片路径  ************************************/
#define PHOTO_ALBUM_IMAGE_PATH "A:/mnt/sd/DCIM/PHOTO/" /* 原图路径 */
#define PHOTO_ALBUM_IMAGE_PATH_S "A:/mnt/sd/DCIM/PHOTO_SMALL/" /* 提取小缩略图路径 */
#define PHOTO_ALBUM_IMAGE_PATH_L "A:/mnt/sd/DCIM/PHOTO_LARGE/" /* 提取大缩略图路径 */

/************************** IMAGE_RECOGNIZE 配置  ************************************/
#define IMAGE_RECOGNIZE_MODEL_NAME "Qwen2.5-VL-72B-Instruct"
#define IMAGE_RECOGNIZE_API_KEY    "EIg_6FZelCDedHTE3U2mJM-gzOLSz0ZtTdzya1xh5BJH2QUWB_EV1I-efCRBWh5FV5Xm7OdpAuM_zygTQguWQQ"
#define IMAGE_RECOGNIZE_BASE_URL   "https://www.sophnet.com/api/open-apis/v1/chat/completions"

/************************** IMAGE_TO_IMAGE 配置  ************************************/
#define IMAGE_TO_IMAGE_ACCESS_KEY "AKLTYmI3MmE0OGJkYWFiNDIzOThmNmFmNWIxZWEwNzk3MWE"
#define IMAGE_TO_IMAGE_SECRET_KEY "T1RoaE5EbG1ZakZqTUdSbU5EUmhOamt4TTJSaU56VTBObVprTURVd1pHTQ=="

/************************** AI图片路径  ************************************/
#define AI_TMP_IMAGE_PATH "A:/tmp/ai_res.jpg"        /* AI拍照临时图片路径 */
#define AI_EFFECT_TMP_IMAGE_PATH "A:/tmp/ai_res.jpg" /* AI拍照临时图片路径 */
#define AI_EFFECT_OUT_IMAGE_PATH "A:/tmp/output.jpg" /* AI拍照临时图片路径 */

/************************** AI对话图片路径  ************************************/
#define DIALOG_DR_IMAGE_PATH "A:/mnt/data/bin/res/dialog_dr.png"
#define DIALOG_NEZHA_IMAGE_PATH "A:/mnt/data/bin/res/dialog_nezha.png"
#define DIALOG_DEEPSEEK_IMAGE_PATH "A:/mnt/data/bin/res/dialog_deepseek.png"

/*****************************方正姚体加载路径**********************************/
// #define ALI_PUHUITI_FONTPATH "/mnt/sd/output/fonts/Alibaba-PuHuiTi-Light.ttf"
#define ALI_PUHUITI_FONTPATH "/usr/local/fonts/hei.TTF"
#define ALI_PUHUITI_WEIGHT 32
/* TP 最大的触摸点数 */
#define MAX_COUNT 2

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#endif

#include "hal_backlight.h"
#include <stdio.h>
#include <stdlib.h>
#include "cvi_log.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* 从系统文件读取最大亮度值 */
static int32_t readMaxBrightness(void) {
	int32_t max_brightness_val;
	FILE *fp = fopen("/sys/devices/platform/backlight/backlight/backlight/max_brightness", "r");
	if(fp == NULL){
		CVI_LOGE("HAL_BACKLIGHT_ReadMaxBrightness: failed to open max_brightness file\n");
		return -1;
	}

	if(fscanf(fp, "%u", &max_brightness_val) != 1) {
		CVI_LOGE("HAL_BACKLIGHT_ReadMaxBrightness: failed to read max_brightness value\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);
	CVI_LOGD("HAL_BACKLIGHT_ReadMaxBrightness: max_brightness = %d\n", max_brightness_val);
	return max_brightness_val;
}

/* 从系统文件读取屏幕状态 */
static BACKLIGHT_STATE_E readScreenState(void) {
	BACKLIGHT_STATE_E s_halBacklightState;
	FILE *fp = fopen("/sys/devices/platform/backlight/backlight/backlight/bl_power", "r");
	if(fp == NULL){
		CVI_LOGE("HAL_BACKLIGHT_ReadScreenState: failed to open bl_power file\n");
		return -1;
	}
	char buffer[10];
	int32_t bl_power = 0;
	fgets(buffer, sizeof(buffer), fp);
	bl_power = atoi(buffer);
	if(bl_power == 0){
		s_halBacklightState = BACKLIGHT_STATE_ON;
	}else{
		s_halBacklightState = BACKLIGHT_STATE_OFF;
	}
	fclose(fp);
	CVI_LOGD("HAL_BACKLIGHT_ReadScreenState: screen_state = %u\n", s_halBacklightState);
	return s_halBacklightState;
}

/* 从系统文件读取亮度值 */
static int32_t readBrightness(void){
	int32_t brightness = 0;
	int32_t percent_val = 0;
	int32_t max_brightness_val = readMaxBrightness();
	FILE *fp = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "r");
	if(fp == NULL){
		CVI_LOGE("HAL_BACKLIGHT_ReadBrightness: failed to open brightness file\n");
		return -1;
	}
	fscanf(fp, "%u", &brightness);
	fclose(fp);
	// 将亮度值转换为百分比
	percent_val = brightness * 100 / max_brightness_val;
	CVI_LOGD("HAL_BACKLIGHT_ReadBrightness: brightness = %d, percent_val = %d\n", brightness, percent_val);
	return percent_val;
}

int32_t HAL_BACKLIGHT_SetLuma(int32_t brightness_val){
	int32_t brightness = 0;
	int32_t max_brightness_val = readMaxBrightness();
	if(brightness_val == readBrightness()){
		return 0;
	}
	if(brightness_val < BACKLIGHT_PERCENT_MIN){
		brightness_val = BACKLIGHT_PERCENT_MIN;
	}
	if(brightness_val > BACKLIGHT_PERCENT_MAX){
		brightness_val = BACKLIGHT_PERCENT_MAX;
	}
	brightness = brightness_val * max_brightness_val / 100;
	FILE *fp = fopen("/sys/devices/platform/backlight/backlight/backlight/brightness", "w");
	if(fp == NULL){
		CVI_LOGE("HAL_BACKLIGHT_SetLuma failed! \n");
		return -1;
	}
	CVI_LOGD("%s percent_val = %d, brightness = %d\n", __func__, brightness_val, brightness);
	fprintf(fp, "%d", brightness);
	fclose(fp);

	return 0;
}

int32_t HAL_BACKLIGHT_GetLuma(void){
	return readBrightness();
}

int32_t HAL_BACKLIGHT_SetBackLightState(BACKLIGHT_STATE_E enBackLightState){
	CVI_LOGD("HAL_BACKLIGHT_SetBackLightState = %d\n", enBackLightState);
	FILE *fp = fopen("/sys/devices/platform/backlight/backlight/backlight/bl_power", "w");
		if(fp == NULL){
			CVI_LOGE("HAL_BACKLIGHT_SetBackLightState: failed to open bl_power file\n");
			return -1;
		}
	if (enBackLightState == BACKLIGHT_STATE_ON) {
		fprintf(fp, "0");
		HAL_BACKLIGHT_SetLuma(readBrightness());
	} else {
		fprintf(fp, "1");
	}
	fclose(fp);
	return 0;
}

BACKLIGHT_STATE_E HAL_BACKLIGHT_GetBackLightState(){
	return readScreenState();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


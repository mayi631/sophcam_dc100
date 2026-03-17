/*
 ********************************************************************
 * Demo program on hal
 *
 * Copyright hal Techanelologies. All Rights Reserved.
 *
 ********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include "osal.h"
#include "cvi_log.h"

static uint8_t adc_init = 0;
#ifdef CHIP_184X
#define ADC_KO_PATH KOMOD_PATH "/cv184x_saradc.ko"
#define MAX_ADC_CHANNEL 15
#else
#define ADC_KO_PATH KOMOD_PATH "/cv181x_saradc.ko"
#define MAX_ADC_CHANNEL 6
#endif
#define ADC_SYS_PATH "/sys/bus/iio/devices/iio:device0/"

static int32_t HAL_ADC_Insmod(void)
{
	struct stat st;
	/* if build-in, no need to insmod */
	if (stat(ADC_SYS_PATH, &st) != 0) {
		return OSAL_FS_Insmod(ADC_KO_PATH, NULL);
	}
	return 0;
}

static int32_t HAL_ADC_Rmmod(void)
{
	/* if build-in, no need to remove */
	if (access(ADC_KO_PATH, F_OK) != 0) {
		return 0;
	}

	return OSAL_FS_Rmmod(ADC_KO_PATH);
}

static int32_t hal_adc_get_value(const char *node)
{
	int32_t val_pre = 0;
	int32_t fd_adc = -1;
	char buf[5];
	memset(buf, 0, sizeof(buf));
	ssize_t len;

	fd_adc = open(node, O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd_adc < 0) {
		CVI_LOGE("open adc failed\n");
		return -1;
	}
	len = read(fd_adc, buf, sizeof(buf));
	buf[len] = 0;
	val_pre = atoi(buf);
	close(fd_adc);

	return val_pre;
}

int32_t HAL_ADC_GetValue(int32_t int_adc_channel)
{
	char *adc_path = ADC_SYS_PATH;
	char char_channel[128] = {0};
	int32_t ret = 0;

	if (int_adc_channel < 1 || int_adc_channel > MAX_ADC_CHANNEL) {
		CVI_LOGE("channel should between in 1 and %d\n", MAX_ADC_CHANNEL);
		return -1;
	}
	sprintf(char_channel, "%sin_voltage%d_raw", adc_path, int_adc_channel);
	if (NULL == char_channel) {
		CVI_LOGE("point must vailable\n");
		return -1;
	}
	ret = hal_adc_get_value(char_channel);
	return ret;
}

int32_t HAL_ADC_Init(void)
{
	int32_t ret = 0;
	if (adc_init == 0) {
		ret |= HAL_ADC_Insmod();
		adc_init = 1;
	}
	return ret;
}

int32_t HAL_ADC_Deinit(void)
{
	if (adc_init == 0)
		return 0;
	adc_init = 0;
	return HAL_ADC_Rmmod();
}

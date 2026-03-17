/*
 ********************************************************************
 * Demo program on Hal
 *
 * Copyright Hal Techanelologies. All Rights Reserved.
 *
 ********************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>

#include "hal_pwm.h"
#include "osal.h"

#ifndef CONFIG_KERNEL_RHINO
#define MAX_BUF 64
#ifdef CHIP_184X
#define PWM_KO_PATH KOMOD_PATH "/cv184x_pwm.ko"
#else
#define PWM_KO_PATH KOMOD_PATH "/cv181x_pwm.ko"
#endif
#define SYSFS_PWM_DIR "/sys/class/pwm/pwmchip"
static bool pwm_init = false;
static HAL_PWM_S s_PwmAttr = {0};
#else
#include <drv/pwm.h>
#endif

#ifndef CONFIG_KERNEL_RHINO
static int32_t check_grp_chn_value(int32_t grp, int32_t chn)
{
	if (!((chn >= 0) && (chn <= 3))) {
		printf("pwm valid range: 0 ~ 3, input chn:%d\n", chn);
		return -1;
	}

	if (!((grp >= 0) && (grp <= 3))) {
		printf("pwm valid range : 0 ~ 3, input grp:%d\n", grp);
		return -1;
	}

	return 0;
}

// static void pwm_save_value(int32_t duty_cycle)
// {
//     char str_pwm[16] = {0};
//     char shellcmd[32] = {0};
//     int32_t pwmstatus = (((s_PwmAttr.period & 0xFFFF) << 16) | ((duty_cycle & 0xFFFF) << 0));
//     memset(str_pwm, 0 ,sizeof(str_pwm));
//     sprintf(str_pwm, "%X", pwmstatus);
//     snprintf(shellcmd, sizeof(shellcmd), "%s %s %s",
//             "fw_setenv", "pwmstatus", str_pwm);
//     system(shellcmd);
// }

// static int32_t get_pwm_value_sum(void)
// {
//     char *shellcmd = "fw_printenv pwmstatus";
//     char *buf = NULL;
//     char val[32] = {0};
//     uint32_t pwm_env = 0;
//     FILE* fp = NULL;
//     char *envPWMStat = getenv("pwmstatus");

//     if (envPWMStat) {
//         buf = envPWMStat;
//         printf("env ");
//     } else {
//         fp = popen(shellcmd, "r");
//         if (fp == NULL) {
//             printf("get_pwm_value_sum popen failed\n");
//             return false;
//         }

//         fgets(val, sizeof(val), fp);
//         pclose(fp);

//         buf = strtok(val, "=");
//         buf = strtok(NULL, "=");
//     }
//     printf("pwmstatus buf=%s\n", buf);

//     if (buf != NULL) {
//         pwm_env = strtoul(buf, NULL, 16);
//         if (envPWMStat) {
//             unsetenv("pwmstatus");
//         }
//         return pwm_env;
//     }
//     return -1;
// }
#endif

static int32_t HAL_PWM_Export(int32_t grp, int32_t chn)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t fd;
	char buf[MAX_BUF] = {0}, buf1[MAX_BUF] = {0};

	if (check_grp_chn_value(grp, chn) != 0)
		return -1;

#ifdef CHIP_184X
	int32_t pwmchip = grp * 6;
#else
	int32_t pwmchip = grp * 4;
#endif

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "%d", pwmchip);
	if ((access(buf, F_OK)) != -1) {
		snprintf(buf1, sizeof(buf1), SYSFS_PWM_DIR "%d/export", pwmchip);
		fd = open(buf1, O_WRONLY);
		if (fd < 0) {
			printf("open export error\n");
			return -1;
		}

		char chn_buf[12] = {0};
		snprintf(chn_buf, sizeof(chn_buf), "%d", chn);
		printf("enter chn_buf = %s\n", chn_buf);
		write(fd, chn_buf, strlen(chn_buf));
		
		close(fd);
	}

	return 0;
#else
	return 0;
#endif
}

static int32_t HAL_PWM_UnExport(int32_t grp, int32_t chn)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t fd;
	char buf[MAX_BUF] = {0}, buf1[MAX_BUF] = {0};

	if (check_grp_chn_value(grp, chn) != 0)
		return -1;

#ifdef CHIP_184X
	int32_t pwmchip = grp * 6;
#else
	int32_t pwmchip = grp * 4;
#endif

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "%d", pwmchip);

	if ((access(buf, F_OK)) != -1) {
		snprintf(buf1, sizeof(buf1), SYSFS_PWM_DIR "%d/unexport", pwmchip);
		fd = open(buf1, O_WRONLY);
		if (fd < 0) {
			printf("open unexport error\n");
			return -1;
		}

		char chn_buf[12] = {0};
		snprintf(chn_buf, sizeof(chn_buf), "%d", chn);
		write(fd, chn_buf, strlen(chn_buf));

		close(fd);
	}
	return 0;
#else
	return 0;
#endif
}

static int32_t HAL_PWM_Enable(int32_t grp, int32_t chn)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t fd;
	char buf[MAX_BUF] = {0};

	if (check_grp_chn_value(grp, chn) != 0)
		return -1;

#ifdef CHIP_184X
	int32_t pwmchip = grp * 6;
#else
	int32_t pwmchip = grp * 4;
#endif

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "%d/pwm%d/enable", pwmchip, chn);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open enable error\n");
		return -1;
	}

	write(fd, "1", strlen("1"));

	close(fd);

	return 0;
#else
	return 0;
#endif
}

static int32_t HAL_PWM_Disable(int32_t grp, int32_t chn)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t fd;
	char buf[MAX_BUF] = {0};

	if (check_grp_chn_value(grp, chn) != 0)
		return -1;

#ifdef CHIP_184X
	int32_t pwmchip = grp * 6;
#else
	int32_t pwmchip = grp * 4;
#endif

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "%d/pwm%d/enable", pwmchip, chn);

	if ((access(buf, F_OK)) != -1) {
		fd = open(buf, O_WRONLY);
		if (fd < 0) {
			printf("open disable error\n");
			return -1;
		}

		write(fd, "0", strlen("0"));

		close(fd);
	}

	return 0;
#else
	return 0;
#endif
}

static int32_t HAL_PWM_Insmod(void)
{
#ifndef CONFIG_KERNEL_RHINO
	return OSAL_FS_Insmod(PWM_KO_PATH, NULL);
#else
	return 0;
#endif
}

static int32_t HAL_PWM_Rmmod(void)
{
#ifndef CONFIG_KERNEL_RHINO
	return OSAL_FS_Rmmod(PWM_KO_PATH);
#else
	return 0;
#endif
}

// int32_t HAL_PWM_Get_Param(uint32_t *period, uint32_t *duty_cycle)
// {
// 	#ifndef CONFIG_KERNEL_RHINO
// 	int32_t pwm_value = get_pwm_value_sum();
// 	if (pwm_value < 0) {
// 		*period = s_PwmAttr.period;
// 		*duty_cycle = s_PwmAttr.duty_cycle;
// 		pwm_save_value(s_PwmAttr.duty_cycle);
// 		return 0;
// 	}
// 	int32_t pwm_period = 0xFFFF & (pwm_value >> 16);
// 	int32_t pwm_duty_cycle = 0xFFFF & (pwm_value >> 0);
// 	if (pwm_duty_cycle < 10) {
// 		pwm_duty_cycle = 10;
// 	}

// 	*period = pwm_period;
// 	*duty_cycle = pwm_duty_cycle;

// 	return 0;
// 	#else
// 	return 0;
// 	#endif
// }

int32_t HAL_PWM_Set_Param(HAL_PWM_S pwmAttr)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t fd;
	char buf[MAX_BUF] = {0}, buf1[MAX_BUF] = {0};

	if (pwmAttr.duty_cycle < 10) {
		pwmAttr.duty_cycle = 10;
	} else {
		if (pwmAttr.duty_cycle > s_PwmAttr.period)
			pwmAttr.duty_cycle = s_PwmAttr.period;
	}

	if (check_grp_chn_value(pwmAttr.group, pwmAttr.channel) != 0)
		return -1;

#ifdef CHIP_184X
	int32_t pwmchip = pwmAttr.group * 6;
#else
	int32_t pwmchip = pwmAttr.group * 4;
#endif

	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "%d/pwm%d/period", pwmchip, pwmAttr.channel);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open period error\n");
		return -1;
	}

	snprintf(buf1, sizeof(buf1), "%d", pwmAttr.period);
	write(fd, buf1, sizeof(buf1));
	close(fd);

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "%d/pwm%d/duty_cycle", pwmchip, pwmAttr.channel);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("open duty_cycle error\n");
		return -1;
	}
	memset(buf1, 0, sizeof(buf));
	snprintf(buf1, sizeof(buf1), "%d", pwmAttr.duty_cycle);
	write(fd, buf1, sizeof(buf1));
	close(fd);

	return 0;
#else
	return 0;
#endif
}

int32_t HAL_PWM_Init(HAL_PWM_S pwmAttr)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	if (pwm_init == false) {
		s_PwmAttr = pwmAttr;
#ifndef CHIP_184X
		ret |= HAL_PWM_Insmod();
#endif
		ret |= HAL_PWM_Export(s_PwmAttr.group, s_PwmAttr.channel);
		// ret |= HAL_PWM_Get_Param(&s_PwmAttr.period, &s_PwmAttr.duty_cycle);
		ret |= HAL_PWM_Set_Param(s_PwmAttr);
		ret |= HAL_PWM_Enable(s_PwmAttr.group, s_PwmAttr.channel);
		pwm_init = true;
	}
	return ret;
#else
	csi_pwm_t pwm;
	int32_t polarity = 1;
	int32_t ret = 0;
	ret = csi_pwm_init(&pwm, pwmAttr.group);
	if (ret != 0) {
		printf("alios csi_pwm_init failed !\n");
		return -1;
	}
	ret = csi_pwm_out_config(&pwm, pwmAttr.channel, pwmAttr.period, pwmAttr.duty_cycle,
							 polarity ? PWM_POLARITY_HIGH : PWM_POLARITY_LOW);
	if (ret != 0) {
		printf("alios csi_pwm_out_config failed !\n");
		return -1;
	}
	ret = csi_pwm_out_start(&pwm, pwmAttr.channel);
	if (ret != 0) {
		printf("alios csi_pwm_out_start failed !\n");
		return -1;
	}
	return ret;
#endif
}

/*the input parm: percentage should between 0 - 100*/
int32_t HAL_PWM_Set_Percent(int32_t percentage)
{
#ifndef CONFIG_KERNEL_RHINO
	if (pwm_init == false) {
		printf("pwm not init\n");
		return -1;
	}

	if ((percentage < 0) || (percentage > 100)) {
		printf("input error parm\n");
		return -1;
	}

	int32_t duty_cycle = s_PwmAttr.period * percentage / 100;
	if (duty_cycle < 10) {
		duty_cycle = 10;
	}
	HAL_PWM_S Attr = {0};
	Attr.group = s_PwmAttr.group;
	Attr.channel = s_PwmAttr.channel;
	Attr.period = s_PwmAttr.period;
	Attr.duty_cycle = duty_cycle;
	HAL_PWM_Set_Param(Attr);

	// pwm_save_value(duty_cycle);
	return 0;
#else
	return 0;
#endif
}

int32_t HAL_PWM_Get_Percent(void)
{
#ifndef CONFIG_KERNEL_RHINO
	if (pwm_init == false) {
		printf("pwm not init\n");
		return -1;
	}

	// uint32_t pwm_value = get_pwm_value_sum();
	// uint32_t pwm_period = 0xFFFF & (pwm_value >> 16);
	// uint32_t pwm_duty_cycle = 0xFFFF & (pwm_value >> 0);
	// return (pwm_duty_cycle * 100 / pwm_period);

	return (s_PwmAttr.duty_cycle * 100 / s_PwmAttr.period);
#else
	return 0;
#endif
}

int32_t HAL_PWM_Deinit(HAL_PWM_S pwmAttr)
{
#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	if (pwm_init == false)
		return 0;

	ret |= HAL_PWM_Disable(pwmAttr.group, pwmAttr.channel);
	ret |= HAL_PWM_UnExport(pwmAttr.group, pwmAttr.channel);
	if (ret != 0) {
		printf("deinit pwm failed!\n");
		return ret;
	}
#ifndef CHIP_184X
	ret |= HAL_PWM_Rmmod();
#endif
	pwm_init = false;
	return ret;
#else
	csi_pwm_t pwm;
	csi_pwm_out_stop(&pwm, pwmAttr.channel);
	csi_pwm_uninit(&pwm);
	return 0;
#endif
}
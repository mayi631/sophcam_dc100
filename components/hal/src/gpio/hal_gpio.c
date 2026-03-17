/*
 ********************************************************************
 * Demo program on hal
 *
 * Copyright hal Technologies. All Rights Reserved.
 *
 *
 ********************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include "hal_gpio.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
#define CHECK_GPIO_NUMBER(x)                                                                                       \
        do {                                                                                                       \
            if ((x < HAL_GPIO_MIN) || (x > HAL_GPIO_MAX)) {                                                        \
                printf("\033[0;31m GPIO %d is invalid at %s: LINE: %d!\033[0;39m\n", x, __func__, __LINE__);       \
                return -1;                                                                                         \
            }                                                                                                      \
        } while (0)
#define MAX_BUF 64
#define EVB_GPIO(x) (x<32)?(480+x):((x<64)?(448+x-32):((x<96)?(416+x-64):((x<=108)?(404+x-96):(-1))))

#ifndef CONFIG_KERNEL_RHINO
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#else
#include "mmio.h"
#define _reg_read(addr) mmio_read_32(addr)
#define _reg_write(addr, data) mmio_write_32(addr, data)
#define _reg_write_mask(addr, mask, data) mmio_clrsetbits_32(addr, mask, data)
#define BIT(x) (1 << (x))
static uintptr_t gpio_base[5] = {
	(uintptr_t)0x03020000, (uintptr_t)0x03021000, (uintptr_t)0x03022000,
	(uintptr_t)0x03023000, (uintptr_t)0x05021000};
#endif

#ifndef CONFIG_KERNEL_RHINO
static int32_t GpioPoll(uint32_t gpio, uint32_t event, FunType Fp)
{
	struct pollfd pfd;
	int32_t fd;
	char value;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/value", gpio);
	fd = open(buf, O_RDWR);
	if (fd < 0) {
		printf("gpio %d open error\n", gpio);
		return -1;
	}
	if (lseek(fd, 0, SEEK_SET) == -1) {
		printf("gpio %d lseek error\n", gpio);
		return -1;
	}
	if (read(fd, &value, 1) == -1) {
		printf("gpio %d read error\n", gpio);
		return -1;
	}

	pfd.fd = fd;
	pfd.events = event;

	while (1) {
		if (poll(&pfd, 1, 1000) == -1) {
			printf("gpio %d poll error\n", gpio);
			return -1;
		}

		if (pfd.revents & POLLPRI) {
			if (lseek(fd, 0, SEEK_SET) == -1) {
				printf("gpio %d lseek error\n", gpio);
				return -1;
			}
			if (read(fd, &value, 1) == -1) {
				printf("gpio %d read error\n", gpio);
				return -1;
			}
			printf("poll value:%c\n", value);
			if (Fp) {
				Fp();
			}

		}
		if (pfd.revents & POLLERR) {
			printf("gpio %d POLLERR\n", gpio);
		}

		usleep(500 * 1000);
	}

	close(fd);

	return 0;
}

static int32_t GpioExport(uint32_t gpio)
{
	int32_t fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
	if (fd < 0) {
		printf("gpio %d export error\n", gpio);
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

static int32_t GpioUnexport(uint32_t gpio)
{
	int32_t fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
	if (fd < 0) {
		printf("gpio %d unexport error\n", gpio);
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);
	return 0;
}

static int32_t GpioSetDirection(uint32_t gpio, uint32_t out_flag)
{
	int32_t fd;
	char buf[MAX_BUF], buf1[MAX_BUF];
	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/direction", gpio);
	if (access(buf, 0) == -1) {
		GpioExport(gpio);
	}

	fd = open(buf, O_RDWR);
	if (fd < 0) {
		printf("gpio %d set-direction error\n", gpio);
		return fd;
	}

	//printf("mark %d , %s \n",out_flag, buf);
	read(fd, buf1, 4);
	if((strstr(buf1, "out") && out_flag) || (strstr(buf1, "in") && !out_flag))
		goto done;

	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

done:
	close(fd);
	return 0;
}

static int32_t GpioSetValue(uint32_t gpio, uint32_t value)
{
	int32_t fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/value", gpio);
	if (access(buf, 0) == -1) {
		//BM_LOG(LOG_DEBUG_ERROR, cout << buf << " not exist!" << endl);
		GpioExport(gpio);
	}

	fd = GpioSetDirection(gpio, 1); //output
	if (fd < 0) {
		printf("gpio %d set-value error\n", gpio);
		return fd;
	}

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("gpio %d set-value error\n", gpio);
		return fd;
	}

	if (value != 0) {
		write(fd, "1", 2);
	} else {
		write(fd, "0", 2);
	}

	close(fd);
	return 0;
}

static int32_t GpioGetValue(uint32_t gpio, uint32_t *value)
{
	int32_t fd;
	char buf[MAX_BUF];
	char ch;

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

	if (access(buf, 0) == -1) {
		//BM_LOG(LOG_DEBUG_ERROR, cout << buf << " not exist!" << endl);
		GpioExport(gpio);
	}

	fd = GpioSetDirection(gpio, 0); //input
	if (fd < 0) {
		printf("gpio %d get-value error\n", gpio);
		return fd;
	}

	fd = open(buf, O_RDONLY);
	if (fd < 0) {
		printf("gpio %d get-value error\n", gpio);
		return fd;
	}

	read(fd, &ch, 1);
	// printf(" GpioGetValue = %c \n",ch);

	if (ch != '0') {
		*value = 1;
	} else {
		*value = 0;
	}

	close(fd);
	return 0;
}

static int32_t GpioSetEdge(uint32_t gpio, uint32_t edge_flag)
{
	int32_t fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/edge", gpio);
	if (access(buf, 0) == -1) {
		GpioExport(gpio);
	}

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		printf("gpio %d set-edge error\n", gpio);
		return fd;
	}

	//printf("mark %d , %s \n",edge_flag, buf);
	switch (edge_flag) {
	case HAL_GPIO_NONE:
		write(fd, "none", 5);
		break;
	case HAL_GPIO_RISING:
		write(fd, "rising", 7);
		break;
	case HAL_GPIO_FALLING:
		write(fd, "falling", 8);
		break;
	case HAL_GPIO_BOTH:
		write(fd, "both", 5);
		break;
	default:
		write(fd, "none", 5);
		break;
	}

	close(fd);
	return 0;
}
#else
static int32_t gpio_set_value(uint32_t gpio_num, uint32_t value)
{
	int32_t rc = 0;

	switch (gpio_num) {
	case HAL_GPIOE_00 ... HAL_GPIOE_23:
		_reg_write_mask(gpio_base[4] + 4, BIT(gpio_num - HAL_GPIOE_00), BIT(gpio_num - HAL_GPIOE_00));
		_reg_write_mask(gpio_base[4], BIT(gpio_num - HAL_GPIOE_00), value ? BIT(gpio_num - HAL_GPIOE_00) : 0);
	break;

	case HAL_GPIOD_00 ... HAL_GPIOD_11:
		_reg_write_mask(gpio_base[3] + 4, BIT(gpio_num - HAL_GPIOD_00), BIT(gpio_num - HAL_GPIOD_00));
		_reg_write_mask(gpio_base[3], BIT(gpio_num - HAL_GPIOD_00), value ? BIT(gpio_num - HAL_GPIOD_00) : 0);
	break;

	case HAL_GPIOC_00 ... HAL_GPIOC_31:
		_reg_write_mask(gpio_base[2] + 4, BIT(gpio_num - HAL_GPIOC_00), BIT(gpio_num - HAL_GPIOC_00));
		_reg_write_mask(gpio_base[2], BIT(gpio_num - HAL_GPIOC_00), value ? BIT(gpio_num - HAL_GPIOC_00) : 0);
	break;

	case HAL_GPIOB_00 ... HAL_GPIOB_31:
		_reg_write_mask(gpio_base[1] + 4, BIT(gpio_num - HAL_GPIOB_00), BIT(gpio_num - HAL_GPIOB_00));
		_reg_write_mask(gpio_base[1], BIT(gpio_num - HAL_GPIOB_00), value ? BIT(gpio_num - HAL_GPIOB_00) : 0);
	break;

	case HAL_GPIOA_00 ... HAL_GPIOA_31:
		_reg_write_mask(gpio_base[0] + 4, BIT(gpio_num - HAL_GPIOA_00), BIT(gpio_num - HAL_GPIOA_00));
		_reg_write_mask(gpio_base[0], BIT(gpio_num - HAL_GPIOA_00), value ? BIT(gpio_num - HAL_GPIOA_00) : 0);
	break;

	default:
		rc = -EINVAL;
	break;
	}

	return rc;
}

static int32_t gpio_get_value(uint32_t gpio_num)
{
	int32_t rc = 0;

	switch (gpio_num) {
	case HAL_GPIOE_00 ... HAL_GPIOE_23:
		_reg_write_mask(gpio_base[4] + 4, BIT(gpio_num - HAL_GPIOE_00), BIT(gpio_num - HAL_GPIOE_00));
		rc = _reg_read(gpio_base[4] + 80);
		// printf("addr = 0x%lx, rc = %x\n",(gpio_base[4] + 80), rc);
		rc = (rc >> (gpio_num - HAL_GPIOE_00)) & 1;
	break;

	case HAL_GPIOD_00 ... HAL_GPIOD_11:
		_reg_write_mask(gpio_base[3] + 4, BIT(gpio_num - HAL_GPIOD_00), BIT(gpio_num - HAL_GPIOD_00));
		rc = _reg_read(gpio_base[3] + 80);
		rc = (rc >> (gpio_num - HAL_GPIOE_00)) & 1;
	break;

	case HAL_GPIOC_00 ... HAL_GPIOC_31:
		_reg_write_mask(gpio_base[2] + 4, BIT(gpio_num - HAL_GPIOC_00), BIT(gpio_num - HAL_GPIOC_00));
		rc = _reg_read(gpio_base[2] + 80);
		rc = (rc >> (gpio_num - HAL_GPIOE_00)) & 1;
	break;

	case HAL_GPIOB_00 ... HAL_GPIOB_31:
		_reg_write_mask(gpio_base[1] + 4, BIT(gpio_num - HAL_GPIOB_00), BIT(gpio_num - HAL_GPIOB_00));
		rc = _reg_read(gpio_base[1]);
		rc = (rc >> (gpio_num - HAL_GPIOE_00)) & 1;
	break;

	case HAL_GPIOA_00 ... HAL_GPIOA_31:
		_reg_write_mask(gpio_base[0] + 4, BIT(gpio_num - HAL_GPIOA_00), BIT(gpio_num - HAL_GPIOA_00));
		rc = _reg_read(gpio_base[0] + 80);
		rc = (rc >> (gpio_num - HAL_GPIOE_00)) & 1;
	break;

	default:
		rc = -EINVAL;
	break;
	}

	return rc;
}
#endif

int32_t HAL_GPIO_Export(HAL_GPIO_NUM_E gpio)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioExport(gpio);
	return ret;
	#else
	(void)gpio;
	return 0;
	#endif
}

int32_t HAL_GPIO_Unexport(HAL_GPIO_NUM_E gpio)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioUnexport(gpio);
	return ret;
	#else
	(void)gpio;
	return 0;
	#endif
}

int32_t HAL_GPIO_Direction_Input(HAL_GPIO_NUM_E gpio)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioSetDirection(gpio, 0);
	return ret;
	#else
	(void)gpio;
	return 0;
	#endif
}

int32_t HAL_GPIO_Direction_Output(HAL_GPIO_NUM_E gpio)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioSetDirection(gpio, 1);
	return ret;
	#else
	(void)gpio;
	return 0;
	#endif
}

int32_t HAL_GPIO_Set_Value(HAL_GPIO_NUM_E gpio, HAL_GPIO_VALUE_E value)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioSetValue(gpio, value);
	return ret;
	#else
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = gpio_set_value(gpio, value);
	return ret;
	#endif
}

int32_t HAL_GPIO_Get_Value(HAL_GPIO_NUM_E gpio, HAL_GPIO_VALUE_E *value)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = GpioGetValue(gpio, value);
	return ret;
	#else
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);
	ret = gpio_get_value(gpio);
	*value = ret;

	return ret;
	#endif
}

int32_t HAL_GPIO_Poll(HAL_GPIO_NUM_E gpio, HAL_GPIO_EDGE_E edge, FunType Fp)
{
	#ifndef CONFIG_KERNEL_RHINO
	int32_t ret = 0;
	CHECK_GPIO_NUMBER(gpio);

	//打开gpio
	GpioExport(gpio);

	//设为输入模式
	ret = GpioSetDirection(gpio, 0);

	//设置中断触发
	ret = GpioSetEdge(gpio, edge);

	ret = GpioPoll(gpio, POLLPRI | POLLERR, Fp);

	//操作完毕,释放gpio
	ret = GpioUnexport(gpio);

	return ret;
	#else
	(void)gpio;
	(void)edge;
	(void)Fp;
	return 0;
	#endif
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

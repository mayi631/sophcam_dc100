/*
 ********************************************************************
 * Demo program on Hal
 *
 * Copyright Hal Technologies. All Rights Reserved.
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
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <poll.h>
#include <termios.h>

#include "hal_uart.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */
static int32_t fd_uart = -1;
static int32_t speed_arr[] = {B115200, B19200, B9600, B4800, B2400, B1200, B300};
static int32_t buad_rate[] = {115200, 19200, 9600, 4800, 2400, 1200, 300};
// #define UART_DEBUG

int32_t UART_Open(char *node)
{
	int32_t fd;

	fd = open(node, O_RDWR);
	if (fd < 0) {
		perror("uart open");
		return fd;
	}

	return fd;
}

int32_t UART_Close(int32_t fd)
{
	int32_t ret;

	ret = close(fd);
	if (ret < 0)
		perror("uart close");

	return ret;
}

int32_t UART_Set_Param(int32_t speed, int32_t flow_ctrl, int32_t databits, int32_t stopbits, char parity)
{
	uint32_t i;
	struct termios options;

	(void)flow_ctrl;
	if (tcgetattr(fd_uart, &options) != 0) {
		perror("tcgetattr");
		return -1;
	}

	for (i = 0; i < sizeof(speed_arr) / sizeof(int32_t); i++) {
		if (speed == buad_rate[i]) {
			cfsetispeed(&options, speed_arr[i]);
			cfsetospeed(&options, speed_arr[i]);
		}
	}

	bzero(&options, sizeof(options));
	options.c_cflag |= CLOCAL | CREAD;
	options.c_cflag &= ~CSIZE;

	switch (databits) {
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	}

	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	case 's':
	case 'S':
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return -1;
	}

	if (stopbits == 1)
		options.c_cflag &= ~CSTOPB;
	else if (stopbits == 2)
		options.c_cflag |= CSTOPB;

	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;

	tcflush(fd_uart, TCIFLUSH);

	options.c_oflag = ~ICANON;

	if ((tcsetattr(fd_uart, TCSANOW, &options)) != 0) {
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

int32_t UART_Receive(unsigned char *rcv_buf, int32_t data_len, int32_t timeout_ms)
{
	int32_t ret = 0;
	struct timeval time;
	fd_set fs_read;

	FD_ZERO(&fs_read);
	FD_SET(fd_uart, &fs_read);
    time.tv_sec = timeout_ms / 1000;
    time.tv_usec = (timeout_ms % 1000) * 1000;

	ret = select(fd_uart + 1, &fs_read, NULL, NULL, &time);
	if (ret == -1) {
		perror("select fail");
		return -1;
	} else if(ret == 0) {
		#ifdef UART_DEBUG
		perror("receve date timeout");
		#endif
		ret = -110;
	} else if (ret > 0) {
		if (FD_ISSET(fd_uart, &fs_read)) {
			ret = read(fd_uart, rcv_buf, data_len);
			if (ret < 0) {
                if (EAGAIN == errno) {
                    perror("No data readable!");
                    return -11;
                } else {
                    perror("read data failed!");
                    return -1;
                }
            }
		}
	}

	return ret;
}

int32_t UART_Send(char *send_buf, int32_t data_len)
{
	int32_t len;

	len = write(fd_uart, send_buf, data_len);

	return len;
}

int32_t UART_Init(char *node)
{
	fd_uart = UART_Open(node);
	if (fd_uart < 0) {
		printf("open uart failed\n");
		return -1;
	}

	return 0;
}

int32_t UART_Exit(void)
{
	if (UART_Close(fd_uart) < 0) {
		printf("close rs485 failed.\n");
		return -1;
	}

	return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

/*
 * Copyright (C) Hal Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: uart.h
 * Description:
 */

#ifndef __HAL_UART_H__
#define __HAL_UART_H__
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

int32_t UART_Open(char *node);
int32_t UART_Close(int32_t fd);
int32_t UART_Set_Param(int32_t speed, int32_t flow_ctrl, int32_t databits, int32_t stopbits, char parity);
int32_t UART_Receive(unsigned char *rcv_buf, int32_t data_len,int32_t timeout_ms);
int32_t UART_Send(char *send_buf, int32_t data_len);
int32_t UART_Init(char *node);
int32_t UART_Exit(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __UART_H__ */
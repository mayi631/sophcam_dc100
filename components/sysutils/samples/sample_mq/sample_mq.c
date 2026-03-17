#include "sysutils_mq.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 消息接收回调函数
int recv_callback(MQ_ENDPOINT_HANDLE_t ep, MQ_MSG_S *msg, void *arg) {
    (void) ep;
    (void) arg;
    printf("收到消息: target_id = 0x%x, arg1 = %d, arg2 = %d, payload = %s\n",
           msg->target_id, msg->arg1, msg->arg2, msg->payload);
    return MQ_SUCCESS;
}

int main() {
    MQ_ENDPOINT_HANDLE_t endpoint = NULL;
    MQ_ENDPOINT_CONFIG_S config;
    memset(&config, 0, sizeof(config));
    MQ_ID_t target_id = 0x12345678; // 示例目标 ID
    const char *payload = "Hello, MQ!";
    int payload_len = strlen(payload);

    // 配置消息队列端点
    config.id = target_id;
    config.recv_cb = recv_callback; // 设置接收回调函数
    config.recv_cb_arg = NULL;

    // 创建消息队列端点
    if (MQ_CreateEndpoint(&config, &endpoint) != MQ_SUCCESS) {
        printf("创建消息队列端点失败\n");
        return -1;
    }
    printf("消息队列端点创建成功\n");

    // 发送消息
    if (MQ_Send(target_id, 1, 2, 0, (char *)payload, payload_len) != MQ_SUCCESS) {
        printf("发送消息失败\n");
    } else {
        printf("消息已发送: %s\n", payload);
    }

    // 等待一段时间以接收消息
    printf("等待接收消息...\n");
    sleep(2);

    // 销毁消息队列端点
    if (MQ_DestroyEndpoint(endpoint) != MQ_SUCCESS) {
        printf("销毁消息队列端点失败\n");
    } else {
        printf("消息队列端点已销毁\n");
    }

    return 0;
}

/*
result:
消息队列端点创建成功
收到消息: target_id = 0x12345678, arg1 = 1, arg2 = 2, payload = Hello, MQ!
消息已发送: Hello, MQ!
等待接收消息...
消息队列端点已销毁
*/
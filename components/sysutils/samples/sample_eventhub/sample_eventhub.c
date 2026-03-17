#include "sysutils_eventhub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 自定义回调函数，用于处理新事件
int32_t on_new_event(void *arg, EVENT_S *event) {
    (void) arg;
    printf("收到新事件: 主题 = %u\n", event->topic);
    return 0;
}

int main() {
    // 初始化事件中心
    if (EVENTHUB_Init() != 0) {
        printf("事件中心初始化失败\n");
        return -1;
    }
    printf("事件中心初始化成功\n");

    // 注册主题
    uint32_t topic_id = 1; // 示例主题 ID
    if (EVENTHUB_RegisterTopic(topic_id) != 0) {
        printf("注册主题失败: %u\n", topic_id);
        EVENTHUB_DeInit();
        return -1;
    }
    printf("注册主题成功: %u\n", topic_id);

    // 创建订阅者
    EVENTHUB_SUBSCRIBER_S subscriber;
    memset(&subscriber, 0, sizeof(EVENTHUB_SUBSCRIBER_S)); // 初始化结构体为零
    subscriber.sync = 1; // 同步模式
    subscriber.new_msg_cb = on_new_event; // 设置回调函数
    MW_PTR subscriber_handle = NULL;

    if (EVENTHUB_CreateSubscriber(&subscriber, &subscriber_handle) != 0) {
        printf("创建订阅者失败\n");
        EVENTHUB_DeInit();
        return -1;
    }
    printf("订阅者创建成功\n");

    // 订阅主题
    if (EVENTHUB_Subcribe(subscriber_handle, topic_id) != 0) {
        printf("订阅主题失败: %u\n", topic_id);
        EVENTHUB_DestroySubscriber(subscriber_handle);
        EVENTHUB_DeInit();
        return -1;
    }
    printf("订阅主题成功: %u\n", topic_id);

    // 发布事件
    EVENT_S event;
    memset(&event, 0, sizeof(EVENT_S)); // 初始化结构体为零
    event.topic = topic_id;

    if (EVENTHUB_Publish(&event) != 0) {
        printf("发布事件失败\n");
    } else {
        printf("事件已发布: 主题 = %u\n", event.topic);
    }

    // 取消订阅主题
    if (EVENTHUB_UnSubcribe(subscriber_handle, topic_id) != 0) {
        printf("取消订阅主题失败: %u\n", topic_id);
    } else {
        printf("取消订阅主题成功: %u\n", topic_id);
    }

    // 注销主题
    if (EVENTHUB_UnRegisterTopic(topic_id) != 0) {
        printf("注销主题失败: %u\n", topic_id);
    } else {
        printf("注销主题成功: %u\n", topic_id);
    }

    // 销毁事件中心
    if (EVENTHUB_DeInit() != 0) {
        printf("事件中心销毁失败\n");
    } else {
        printf("事件中心销毁成功\n");
    }

    return 0;
}
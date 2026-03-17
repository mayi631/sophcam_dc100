#include "sysutils_hfsm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t stateid = 1;
// 状态回调函数
int32_t state_open() {
    printf("状态已打开, stateid:%d\n", stateid);
    return 0;
}

int32_t state_close() {
    printf("状态已关闭, stateid:%d\n", stateid);
    return 0;
}

int32_t state_process_message(MESSAGE_S *msg, void *argv, uint32_t *next_state_id) {
    printf("处理消息: %d\n", msg->arg1);
    (void)argv;
    if (msg->arg1 == 2) {
        *next_state_id = 2; // 切换到状态 2
        stateid = 2;
    }
    return 0;
}

// 事件回调函数
int32_t hfsm_event_callback(HFSM_HANDLE hfsm, const HFSM_EVENT_INFO_S *event_info) {
    (void)hfsm;
    printf("事件回调: 事件代码 = %d\n", event_info->enEventCode);
    return 0;
}

int main() {
    HFSM_HANDLE hfsm = NULL;
    HFSM_ATTR_S hfsm_attr;
    memset(&hfsm_attr, 0, sizeof(HFSM_ATTR_S));

    // 配置 HFSM 属性
    hfsm_attr.fnHfsmEventCallback = hfsm_event_callback;
    hfsm_attr.u32StateMaxAmount = 2; // 最大状态数
    hfsm_attr.u32MessageQueueSize = 10; // 消息队列大小

    // 创建 HFSM
    if (HFSM_Create(&hfsm_attr, &hfsm) != 0) {
        printf("创建 HFSM 失败\n");
        return -1;
    }
    printf("HFSM 创建成功\n");

    // 定义状态 1
    STATE_S state1;
    memset(&state1, 0, sizeof(STATE_S));
    state1.stateID = 1;
    memcpy(state1.name, "State1", sizeof("State1"));
    state1.open = state_open;
    state1.close = state_close;
    state1.processMessage = state_process_message;

    // 定义状态 2
    STATE_S state2;
    memset(&state2, 0, sizeof(STATE_S));
    state2.stateID = 2;
    memcpy(state2.name, "State2", sizeof("State2"));
    state2.open = state_open;
    state2.close = state_close;
    state2.processMessage = state_process_message;

    // 添加状态到 HFSM
    if (HFSM_AddState(hfsm, &state1, NULL) != 0) {
        printf("添加状态 1 失败\n");
        HFSM_Destroy(hfsm);
        return -1;
    }
    printf("状态 1 添加成功\n");

    if (HFSM_AddState(hfsm, &state2, NULL) != 0) {
        printf("添加状态 2 失败\n");
        HFSM_Destroy(hfsm);
        return -1;
    }
    printf("状态 2 添加成功\n");

    // 设置初始状态State1
    if (HFSM_SetInitialState(hfsm, stateid) != 0) {
        printf("设置初始状态失败\n");
        HFSM_Destroy(hfsm);
        return -1;
    }
    printf("初始状态设置成功\n");

    // 启动 HFSM
    if (HFSM_Start(hfsm) != 0) {
        printf("启动 HFSM 失败\n");
        HFSM_Destroy(hfsm);
        return -1;
    }
    printf("HFSM 启动成功\n");

    // 发送异步消息
    MESSAGE_S msg;
    memset(&msg, 0, sizeof(MESSAGE_S));
    msg.arg1 = 2; // 消息内容
    if (HFSM_SendAsyncMessage(hfsm, &msg) != 0) {
        printf("发送消息失败\n");
    } else {
        printf("消息已发送: %d\n", msg.arg1);
    }

    sleep(2);
    // 停止 HFSM
    if (HFSM_Stop(hfsm) != 0) {
        printf("停止 HFSM 失败\n");
    } else {
        printf("HFSM 已停止\n");
    }

    // 销毁 HFSM
    if (HFSM_Destroy(hfsm) != 0) {
        printf("销毁 HFSM 失败\n");
    } else {
        printf("HFSM 已销毁\n");
    }

    return 0;
}

/*
result:
HFSM 创建成功
状态 1 添加成功
状态 2 添加成功
状态已打开, stateid:1
初始状态设置成功
HFSM 启动成功
消息已发送: 2
处理消息: 2
状态已关闭, stateid:2
状态已打开, stateid:2
事件回调: 事件代码 = 0
HFSM 已停止
HFSM 已销毁


*/
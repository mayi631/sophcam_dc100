#include "sysutils_signal_slot.h"
#include <stdio.h>

// 槽函数
void onSignalReceived(void *handle, int32_t value) {
    (void) handle;
    printf("槽函数被触发: 接收到的值 = %d\n", value);
}

int main() {
    // 初始化信号
    SIGNAL_S signal;
    signal.type = SIGNAL_SLOT_TYPE_INT; // 信号类型为 int
    SIGNAL_Init(&signal);
    printf("信号已初始化\n");

    // 初始化槽
    SLOT_S slot;
    SLOT_InitCpp(&slot, nullptr, onSignalReceived);
    printf("槽已初始化\n");

    // 连接信号与槽
    if (SIGNAL_Connect(signal, slot) == 0) {
        printf("信号与槽已连接\n");
    } else {
        printf("信号与槽连接失败\n");
    }

    // 触发信号
    int32_t value = 42;
    SIGNAL_INT_Emit(signal, value);
    printf("信号已触发\n");

    // 断开信号与槽
    if (SIGNAL_Disconnect(signal, slot) == 0) {
        printf("信号与槽已断开\n");
    } else {
        printf("信号与槽断开失败\n");
    }

    // 销毁信号与槽
    SIGNAL_Deinit(&signal);
    SLOT_Deinit(&slot);
    printf("信号与槽已销毁\n");

    return 0;
}

/*
result:
信号已初始化
槽已初始化
信号与槽已连接
槽函数被触发: 接收到的值 = 42
信号已触发
信号与槽已断开
信号与槽已销毁
*/
#include "sysutils_timer.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 定时器回调函数
void timer_callback(void *clientData, struct timespec *curTime) {
    (void) clientData;
    printf("定时器触发: 当前时间 = %ld.%09ld 秒\n", curTime->tv_sec, curTime->tv_nsec);
}

int main() {
    int32_t grpHdl;
    TIMER_HANDLE_T timerHdl;
    TIMER_S timerConf;

    // 初始化定时器组
    grpHdl = Timer_Init(false); // 非阻塞模式
    if (grpHdl < 0) {
        printf("初始化定时器组失败\n");
        return -1;
    }
    printf("定时器组初始化成功，句柄: %d\n", grpHdl);

    // 配置定时器
    timerConf.interval_ms = 1000; // 定时器间隔 1000 毫秒
    timerConf.periodic = true;   // 周期性定时器
    timerConf.timer_proc = timer_callback; // 定时器回调函数
    timerConf.clientData = NULL; // 用户数据

    // 创建定时器
    timerHdl = Timer_Create(grpHdl, &timerConf);
    if (timerHdl == NULL) {
        printf("创建定时器失败\n");
        Timer_DeInit(grpHdl);
        return -1;
    }
    printf("定时器创建成功\n");

    // 等待 5 秒以观察定时器触发
    printf("等待 5 秒...\n");
    sleep(5);

    // 销毁定时器
    if (Timer_Destroy(grpHdl, timerHdl) != 0) {
        printf("销毁定时器失败\n");
    } else {
        printf("定时器销毁成功\n");
    }

    // 销毁定时器组
    if (Timer_DeInit(grpHdl) != 0) {
        printf("销毁定时器组失败\n");
    } else {
        printf("定时器组销毁成功\n");
    }

    return 0;
}

/*
result:
定时器组初始化成功，句柄: 3
定时器创建成功
等待 5 秒...
定时器触发: 当前时间 = 1425.594436000 秒
定时器触发: 当前时间 = 1426.594414000 秒
定时器触发: 当前时间 = 1427.594412880 秒
定时器触发: 当前时间 = 1428.594412960 秒
定时器触发: 当前时间 = 1429.594412160 秒
定时器销毁成功
[19700101_082349.594 Timer_DeInit:199] Timer_DeInit success

定时器组销毁成功

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "STG.h"

int main() {
    // 初始化存储模块
    STG_DEVINFO_S stgDevInfo = {
        .aszDevPath = "/dev/mmcblk0p1",
        .aszMntPath = "/mnt/sd",
        .aszDevPort = "cvi.0",
    };
    STG_HANDLE_T stgHandle;

    if (STG_Init(&stgDevInfo, &stgHandle) != 0) {
        printf("Failed to initialize storage module.\n");
        return -1;
    }

    // 检测分区
    if (STG_DetectPartition(stgHandle) != 0) {
        printf("Failed to detect partition.\n");
    } else {
        printf("Partition detected successfully.\n");
    }

    // 测试分区
    if (STG_TestPartition(stgHandle) != 0) {
        printf("Failed to test partition.\n");
    } else {
        printf("Partition tested successfully.\n");
    }

    // 挂载设备
    if (STG_Mount(stgHandle) != 0) {
        printf("Failed to mount device.\n");
    } else {
        printf("Device mounted successfully.\n");
    }

    // 获取设备信息
    STG_DEV_INFO_S devInfo;
    if (STG_GetInfo(stgHandle, &devInfo) != 0) {
        printf("Failed to get device info.\n");
    } else {
        printf("Device Info:\n");
        printf("  Work Mode: %s\n", devInfo.aszWorkMode);
        printf("  Device Type: %s\n", devInfo.aszDevType);
        printf("  Error Count: %d\n", devInfo.szErrCnt);
    }

    // 卸载设备
    if (STG_Umount(stgHandle) != 0) {
        printf("Failed to unmount device.\n");
    } else {
        printf("Device unmounted successfully.\n");
    }

    // 反初始化存储模块
    if (STG_DeInit(stgHandle) != 0) {
        printf("Failed to deinitialize storage module.\n");
    } else {
        printf("Storage module deinitialized successfully.\n");
    }

    return 0;
}

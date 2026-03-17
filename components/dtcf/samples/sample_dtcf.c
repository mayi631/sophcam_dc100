#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <inttypes.h>
#include "dtcf.h"

int main() {
    void *dtcf_handle = NULL;
    DTCF_PARAM_S param = {
        .main_dir = "/tmp/dtcf_test",
        .file_selector_cb = NULL,
        .file_compare_cb = NULL,
        .file_selector_prealloc_cb = NULL
    };

    // 初始化 DTCF 上下文
    if (DTCF_Init(&dtcf_handle, &param) != 0) {
        printf("Failed to initialize DTCF\n");
        return -1;
    }
    DTCF_ShowFileList(dtcf_handle, __func__);

    // 创建目录
    const char dirs[][DTCF_PATH_MAX_LEN] = {"dir1", "dir2"};
    if (DTCF_CreateDir(dtcf_handle, dirs, 2, 0755) != 0) {
        printf("Failed to create directories\n");
    }
    DTCF_ShowFileList(dtcf_handle, __func__);

    // 添加文件到目录
    if (DTCF_AddFile(dtcf_handle, "dir1", "file1.txt") != 0) {
        printf("Failed to add file to directory\n");
    }
    DTCF_ShowFileList(dtcf_handle, __func__);

    // 获取目录文件大小
    uint64_t size = 0;
    if (DTCF_GetDirFileSize(dtcf_handle, "dir1", &size) == 0) {
        printf("Directory 'dir1' size: %" PRIu64 " bytes\n", size);
    }

    // 销毁 DTCF 上下文
    if (DTCF_Deinit(dtcf_handle) != 0) {
        printf("Failed to deinitialize DTCF\n");
    }

    return 0;
}


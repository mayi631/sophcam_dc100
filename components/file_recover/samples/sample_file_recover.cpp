#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "file_recover.h"

// 自定义事件处理器
void customEventHandler(void* custom_arg, FILE_RECOVER_EVENT_S* event) {
    printf("自定义事件处理器: 事件类型 = %d, 自定义参数 = %s\n", event->type, (char*)custom_arg);
}

int main(int argc, char* argv[]) {
    FILE_RECOVER_HANDLE_T file_recover = NULL;
    const char* input_file = (argc > 1) ? argv[1] : "input.mov"; // 输入文件路径
    // const char* output_file = "output.mov"; // 恢复后的文件路径
    bool has_create_time = true; // 是否包含创建时间

    // 创建文件恢复实例
    if (FILE_RECOVER_Create(&file_recover) != 0) {
        printf("创建文件恢复实例失败\n");
        return -1;
    }
    printf("文件恢复实例创建成功\n");

    // 打开文件
    if (FILE_RECOVER_Open(file_recover, input_file) != 0) {
        printf("打开文件失败: %s\n", input_file);
        FILE_RECOVER_Destroy(&file_recover);
        return -1;
    }
    printf("文件打开成功: %s\n", input_file);

    // 设置预分配状态
    FILE_RECOVER_PreallocateState(file_recover, true);
    printf("文件预分配状态已设置\n");

    // 设置自定义事件处理器
    // const char* custom_arg = "CustomArg123";
    // FILE_RECOVER_SetCustomArgEventHandler(file_recover, customEventHandler, (void*)custom_arg);
    printf("自定义事件处理器已设置\n");

    // 恢复文件
    if (FILE_RECOVER_Recover(file_recover, input_file, "", has_create_time) != 0) {
        printf("文件恢复失败\n");
        FILE_RECOVER_Close(file_recover);
        FILE_RECOVER_Destroy(&file_recover);
        return -1;
    }
    printf("文件恢复成功: %s\n", input_file);

    // 关闭文件
    if (FILE_RECOVER_Close(file_recover) != 0) {
        printf("关闭文件失败\n");
    } else {
        printf("文件关闭成功\n");
    }

    // 销毁文件恢复实例
    if (FILE_RECOVER_Destroy(&file_recover) != 0) {
        printf("销毁文件恢复实例失败\n");
    } else {
        printf("文件恢复实例销毁成功\n");
    }

    return 0;
}

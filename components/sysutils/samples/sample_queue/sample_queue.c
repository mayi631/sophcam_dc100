#include "sysutils_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char name[32];
} SampleNode;

int main() {
    // 创建队列
    uint32_t nodeSize = sizeof(SampleNode);
    uint32_t maxLen = 5; // 队列最大长度
    QUEUE_HANDLE_T queue = QUEUE_Create(nodeSize, maxLen);
    if (queue == 0) {
        printf("创建队列失败\n");
        return -1;
    }
    printf("队列创建成功，最大长度: %u\n", maxLen);

    // 入队操作
    for (int i = 0; i < 6; i++) { // 尝试插入 6 个元素
        SampleNode node = {i, "Node"};
        snprintf(node.name, sizeof(node.name), "Node_%d", i);
        if (QUEUE_Push(queue, &node) == 0) {
            printf("入队成功: id = %d, name = %s\n", node.id, node.name);
        } else {
            printf("入队失败: 队列已满\n");
        }
    }

    // 获取队列长度
    int32_t len = QUEUE_GetLen(queue);
    printf("当前队列长度: %d\n", len);

    // 出队操作
    for (int i = 0; i < 6; i++) { // 尝试取出 6 个元素
        SampleNode node;
        if (QUEUE_Pop(queue, &node) == 0) {
            printf("出队成功: id = %d, name = %s\n", node.id, node.name);
        } else {
            printf("出队失败: 队列为空\n");
        }
    }

    // 清空队列
    QUEUE_Clear(queue);
    printf("队列已清空\n");

    // 销毁队列
    QUEUE_Destroy(queue);
    printf("队列已销毁\n");

    return 0;
}

/*
队列创建成功，最大长度: 5
入队成功: id = 0, name = Node_0
入队成功: id = 1, name = Node_1
入队成功: id = 2, name = Node_2
入队成功: id = 3, name = Node_3
入队成功: id = 4, name = Node_4
queue is full!
入队失败: 队列已满
当前队列长度: 5
出队成功: id = 0, name = Node_0
出队成功: id = 1, name = Node_1
出队成功: id = 2, name = Node_2
出队成功: id = 3, name = Node_3
出队成功: id = 4, name = Node_4
queue is empity!
出队失败: 队列为空
队列已清空
队列已销毁

*/
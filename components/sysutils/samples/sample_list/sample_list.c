#include "cvi_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义一个结构体，包含链表节点
typedef struct {
    int id;
    char name[32];
    CVI_LIST_HEAD_S list; // 嵌入链表节点
} SampleNode;

int main() {
    // 初始化链表头
    CVI_LIST_HEAD_S my_list = CVI_LIST_HEAD_INIT(my_list);
    printf("链表已初始化\n");

    // 添加节点到链表
    for (int i = 0; i < 5; i++) {
        SampleNode *node = (SampleNode *)malloc(sizeof(SampleNode));
        node->id = i;
        snprintf(node->name, sizeof(node->name), "Node_%d", i);
        CVI_LIST_ADD_TAIL(&node->list, &my_list); // 添加到链表尾部
        printf("添加节点: id = %d, name = %s\n", node->id, node->name);
    }

    // 遍历链表
    printf("遍历链表:\n");
    CVI_LIST_HEAD_S *pos;
    CVI_LIST_FOR_EACH(pos, &my_list) {
        SampleNode *node = CVI_LIST_ENTRY(pos, SampleNode, list);
        printf("节点: id = %d, name = %s\n", node->id, node->name);
    }

    // 检查链表状态
    if (CVI_LIST_IS_EMPTY(&my_list)) {
        printf("链表为空\n");
    } else {
        printf("链表不为空\n");
    }

    // 替换链表中的一个节点
    printf("替换节点 id = 3\n");
    CVI_LIST_FOR_EACH(pos, &my_list) {
        SampleNode *node = CVI_LIST_ENTRY(pos, SampleNode, list);
        if (node->id == 3) {
            SampleNode *new_node = (SampleNode *)malloc(sizeof(SampleNode));
            new_node->id = 99;
            snprintf(new_node->name, sizeof(new_node->name), "Node_99");
            CVI_LIST_REPLACE(&node->list, &new_node->list);
            free(node);
            printf("节点 id = 3 已替换为 id = %d, name = %s\n", new_node->id, new_node->name);
            break;
        }
    }

    // 从尾到头遍历链表
    printf("从尾到头遍历链表:\n");
    CVI_LIST_FOR_EACH_FORWAORD(pos, &my_list) {
        SampleNode *node = CVI_LIST_ENTRY(pos, SampleNode, list);
        printf("节点: id = %d, name = %s\n", node->id, node->name);
    }

    // 删除链表中的一个节点
    printf("删除节点 id = 2\n");
    CVI_LIST_HEAD_S *n;
    CVI_LIST_FOR_EACH_SAFE(pos, n, &my_list) {
        SampleNode *node = CVI_LIST_ENTRY(pos, SampleNode, list);
        if (node->id == 2) {
            CVI_LIST_DEL(&node->list);
            free(node);
            printf("节点 id = 2 已删除\n");
            break;
        }
    }

    // 再次遍历链表
    printf("再次遍历链表:\n");
    CVI_LIST_FOR_EACH(pos, &my_list) {
        SampleNode *node = CVI_LIST_ENTRY(pos, SampleNode, list);
        printf("节点: id = %d, name = %s\n", node->id, node->name);
    }

    // 清空链表并释放内存
    printf("清空链表\n");
    CVI_LIST_FOR_EACH_SAFE(pos, n, &my_list) {
        SampleNode *node = CVI_LIST_ENTRY(pos, SampleNode, list);
        CVI_LIST_DEL(&node->list);
        free(node);
    }

    // 检查链表是否为空
    if (CVI_LIST_IS_EMPTY(&my_list)) {
        printf("链表已清空\n");
    } else {
        printf("链表未清空\n");
    }

    return 0;
}

/*
result:
链表已初始化
添加节点: id = 0, name = Node_0
添加节点: id = 1, name = Node_1
添加节点: id = 2, name = Node_2
添加节点: id = 3, name = Node_3
添加节点: id = 4, name = Node_4
遍历链表:
节点: id = 0, name = Node_0
节点: id = 1, name = Node_1
节点: id = 2, name = Node_2
节点: id = 3, name = Node_3
节点: id = 4, name = Node_4
链表不为空
替换节点 id = 3
节点 id = 3 已替换为 id = 99, name = Node_99
从尾到头遍历链表:
节点: id = 4, name = Node_4
节点: id = 99, name = Node_99
节点: id = 2, name = Node_2
节点: id = 1, name = Node_1
节点: id = 0, name = Node_0
删除节点 id = 2
节点 id = 2 已删除
再次遍历链表:
节点: id = 0, name = Node_0
节点: id = 1, name = Node_1
节点: id = 99, name = Node_99
节点: id = 4, name = Node_4
清空链表
链表已清空

*/


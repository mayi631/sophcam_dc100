#include "sysutils_md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 打印 MD5 哈希值
void print_md5(unsigned char digest[16]) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");
}

// 计算字符串的 MD5 哈希值
void compute_md5_string(const char *input) {
    MD5_CTX_S context;
    unsigned char digest[16];

    MD5_Init(&context);
    MD5_Update(&context, (unsigned char *)input, strlen(input));
    MD5_Final(&context, digest);

    printf("字符串 \"%s\" 的 MD5 哈希值: ", input);
    print_md5(digest);
}

// 计算文件的 MD5 哈希值
void compute_md5_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("无法打开文件: %s\n", filename);
        return;
    }

    MD5_CTX_S context;
    unsigned char buffer[1024];
    unsigned char digest[16];
    size_t bytes_read;

    MD5_Init(&context);

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        MD5_Update(&context, buffer, bytes_read);
    }

    fclose(file);

    MD5_Final(&context, digest);

    printf("文件 \"%s\" 的 MD5 哈希值: ", filename);
    print_md5(digest);
}

int main() {
    // 示例字符串
    const char *test_string = "Hello, MD5!";
    compute_md5_string(test_string);

    // 示例文件
    const char *test_file = "sample.txt";
    compute_md5_file(test_file);

    return 0;
}

/*
result:
字符串 "Hello, MD5!" 的 MD5 哈希值: 383e139e64e5f46de9d03ba3695da2d8
文件 "sample.txt" 的 MD5 哈希值: a47381fc61894ed366e2648426b2c9ed

*/
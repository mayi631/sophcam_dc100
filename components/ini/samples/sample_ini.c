#include <stdio.h>
#include <stdlib.h>

#include "minIni.h"

// 回调函数，用于 ini_browse
static int callback(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, void *UserData) {
    // void *UserData;
    printf("Section: %s, Key: %s, Value: %s\n", Section, Key, Value);
    return 1; // 返回非零值以继续浏览
}

int32_t main(int32_t argc, char *argv[])
{
    /* Parse command line arguments */
    if (argc != 3) {
        goto help;
    }

    char name[50];
    long age;
    int enabled;
    float value;
    char section[50];
    char key[50];

    // 读取字符串值
    ini_gets("Settings", "name", "default", name, sizeof(name), "config.ini");
    // 读取长整型值
    age = ini_getl("Settings", "age", 0, "config.ini");
    // 读取布尔值
    enabled = ini_getbool("Settings", "enabled", 0, "config.ini");
    // 读取浮点值
    value = ini_getf("Settings", "value", 0.0, "config.ini");

    // 打印读取的值
    printf("Name: %s\n", name);
    printf("Age: %ld\n", age);
    printf("Enabled: %d\n", enabled);
    printf("Value: %f\n", value);

    // 获取第一个节名称
    ini_getsection(0, section, sizeof(section), "config.ini");
    printf("First section: %s\n", section);

    // 获取第一个键名称
    ini_getkey("Settings", 0, key, sizeof(key), "config.ini");
    printf("First key in 'Settings': %s\n", key);

    // 修改并写入新的值
    ini_putl("Settings", "age", 25, "config.ini");
    ini_puts("Settings", "name", "new_example", "config.ini");
    ini_putf("Settings", "value", 6.28, "config.ini");

    // 浏览 INI 文件中的所有设置
    ini_browse(callback, NULL, "config.ini");

    return 0;

help:
printf("Parameter parsing: \n"
    "    -i   <input file name> : Enter the ini file name to parse\n");
    exit(0);
}

/*
result:
Name: example
Age: 30
Enabled: 1
Value: 3.140000
First section: Settings
First key in 'Settings': name
Section: Settings, Key: name, Value: new_example
Section: Settings, Key: age, Value: 25
Section: Settings, Key: enabled, Value: true
Section: Settings, Key: value, Value: 6.280000

*/

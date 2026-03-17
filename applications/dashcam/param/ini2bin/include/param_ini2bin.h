#ifndef __PARAM_INI2BIN_H__
#define __PARAM_INI2BIN_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#pragma pack(push)
#pragma pack(8)

#include "param.h"

#define PARAM_CHECK_LOAD_RESULT(ret, name)      \
    do {                                            \
        if (0 != ret){                              \
            printf("[Error] Load [%s] failed\n", name);  \
            return -1;                     \
        }                                           \
    } while (0)

#define PARAM_CHECK_FOPEN_RESULT(ret, name)     \
    do {                                            \
        if (!ret){                                  \
            printf("[Error] fopen [%s] failed\n", name); \
            return -1;                     \
        }                                           \
    } while (0)

#define PARAM_MODULE_PATH_LEN   (16)
#define PARAM_MODULE_NAME_LEN   (64)
#define PARAM_MODULE_MAX        (20)

#define PARAM_SECTION_LEN       (32)
#define PARAM_KEY_LEN           (16)

typedef struct _PARAM_MODULE_INFO_S {
    char path[PARAM_MODULE_NAME_LEN];
    char name[PARAM_MODULE_NAME_LEN];
} PARAM_MODULE_INFO_S;

typedef struct _PARAM_ACCESS {
    uint32_t module_num;
    PARAM_MODULE_INFO_S modules[PARAM_MODULE_MAX];
} PARAM_ACCESS;



int32_t  INI_PARAM_LoadAccessEntry();
int32_t  INI_PARAM_LoadWorkModeCfg(PARAM_WORK_MODE_S *WorkMode);
int32_t  INI_PARAM_LoadMediaCommCfg(PARAM_MEDIA_COMM_S *MediaParam);
int32_t  INI_PARAM_LoadMediaCamCfg(PARAM_CAM_CFG *MediaMode);
int32_t  INI_PARAM_LoadDevmngCfg(PARAM_DEVMNG_S *DevMng);
int32_t  INI_PARAM_LoadFilemngCfg(PARAM_FILEMNG_S *FileMng);
int32_t  INI_PARAM_LoadMenuCfg(PARAM_MENU_S *MenuMng);
int32_t  INI_PARAM_MediaString2Uint(uint32_t *MediaMode, char *String);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __PARAM_INI2BIN_H__ */
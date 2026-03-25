#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <inttypes.h>
#include <sys/mman.h>

#include "mapi.h"
#include "param.h"
#include "appcomm.h"
#include "mode.h"
#include "flash_error.h"

#pragma pack(push)
#pragma pack(8)

#define PARAM_CUR_BIN_PATH  "/mnt/data/bin/param/app_cfg.bin"
#define PARAM_DEF_BIN_PATH  "/mnt/data/bin/param/app_cfg_def.bin"
#define PARAM_CUR_BIN_BACK_PATH  "/mnt/sd/param/app_cfg.bin"
#define PARAM_DEF_BIN_BACK_PATH  "/mnt/sd/param/app_cfg_def.bin"
#define PARAM_PARTITION_ADDR    (0x780000)
#define PARAM_DEF_PARTI_ADDR    (0x800000)
#define PARAM_PARTITION_SIZE    (0x80000)  // 512*1024 bit
#define PARAM_PARTITION_NAME    "/dev/mtd3"
#define PARAM_DEF_PARTI_NAME    "/dev/mtd4"
#define PARAM_DEF_BIN_NAME      "./app_cfg.bin"
#define APP_CFG_PART_NUM    3
#define DEF_APP_CFG_PART_NUM    4

static PARAM_CONTEXT_S g_stParamCtx = {
    .bInit = 0,
    .bChanged = 0,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .pstCfg = NULL
};

typedef struct _PARAM_SAVETSK_CTX_S {
    bool bRun;
    pthread_t tskId;
    pthread_mutex_t mutexLock;
    bool bSaveFlg;
} PARAM_SAVETSK_CTX_S;

static PARAM_SAVETSK_CTX_S s_stPARAMSaveTskCtx = {
    .bRun = false,
    .tskId = 0,
    .mutexLock = PTHREAD_MUTEX_INITIALIZER,
    .bSaveFlg = false
};

static uint32_t PARAM_Crc32(const uint8_t *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    static const uint32_t crcTable[256] = {
        0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
        0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
        0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
        0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
        0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
        0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
        0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
        0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
        0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
        0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
        0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
        0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
        0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
        0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
        0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
        0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
        0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
        0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
        0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
        0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
        0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
        0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
        0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
        0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
        0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
        0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
        0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
        0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
        0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
        0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
        0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
        0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
    };

    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ crcTable[(crc ^ data[i]) & 0xFF];
    }

    return crc ^ 0xFFFFFFFF;
}

static void *PARAM_SaveTsk(void *pParam)
{
    prctl(PR_SET_NAME, (unsigned long)"CviParamSave", 0, 0, 0);
    PARAM_SAVETSK_CTX_S *pstTskCtx = (PARAM_SAVETSK_CTX_S *)pParam;
    while (pstTskCtx->bRun) {
        usleep(1 * 1000 * 1000);
        if (true == pstTskCtx->bSaveFlg) {
            pstTskCtx->bSaveFlg = false;
            pthread_mutex_lock(&pstTskCtx->mutexLock);
            PARAM_SaveParam();
            pthread_mutex_unlock(&pstTskCtx->mutexLock);
        }
    }
    pthread_mutex_lock(&pstTskCtx->mutexLock);
    PARAM_SaveParam();
    pthread_mutex_unlock(&pstTskCtx->mutexLock);
    return NULL;
}

/* add for ini param */
int32_t PARAM_Init(void)
{
    if (g_stParamCtx.bInit) {
        CVI_LOGW("has already inited\n");
        return 0;
    }
   int32_t  s32Ret = 0;
#ifndef LOAD_PARAM_FROM_FILE
    if (CVIMMAP_SHARE_PARAM_SIZE == 0) {
        uint64_t cfg_addr_s, cfg_addr_e;

        hal_flash *flash = NULL;
        ERROR_TYPE check_ret;

        /* init */
        flash = flash_init();
        if (flash == NULL) {
            printf("Cannot init flash\n");
            return 1;
        }

        cfg_addr_s = flash->part_info[APP_CFG_PART_NUM].offset;
        cfg_addr_e = flash->part_info[APP_CFG_PART_NUM].offset + flash->part_info[APP_CFG_PART_NUM].size - 1;

        printf("cfg_s = 0x%#"PRIx64", cfg_e = 0x%#"PRIx64"\n", cfg_addr_s, cfg_addr_e);
        s32Ret = PARAM_LoadFromFlash(flash, PARAM_PARTITION_NAME, cfg_addr_s, cfg_addr_e - cfg_addr_s + 1);
        if(s32Ret != 0) {
            printf("load failed\n");
            uint64_t def_cfg_addr_s, def_cfg_addr_e;

            def_cfg_addr_s = flash->part_info[DEF_APP_CFG_PART_NUM].offset;
            def_cfg_addr_e = flash->part_info[DEF_APP_CFG_PART_NUM].offset + flash->part_info[APP_CFG_PART_NUM].size - 1;

            printf("cfg_s = 0x%#"PRIx64", cfg_e = 0x%#"PRIx64"\n", cfg_addr_s, cfg_addr_e);
            s32Ret = PARAM_LoadFromFlash(flash, PARAM_DEF_PARTI_NAME, def_cfg_addr_s, def_cfg_addr_e - def_cfg_addr_s + 1);
            if(s32Ret != 0) {
                printf("load failed\n");
            }
        }

        check_ret = flash_destroy(flash);
        if (check_ret != FLASH_OK) {
            printf("cannot destroy hal_flash !\n");
        }
    } else {
       int32_t  fd = -1;

        fd = open("/dev/mem", O_RDWR | O_NDELAY);
        if (fd < 0) {
            CVI_LOGE("open /dev/mem/ failed !\n");
            return -1;
        }
        g_stParamCtx.pstCfg = (PARAM_CFG_S *)mmap(NULL, CVIMMAP_SHARE_PARAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CVIMMAP_SHARE_PARAM_ADDR);
        CVI_LOGI("PARAM_Init lcrc32 %u", g_stParamCtx.pstCfg->crc32);
        uint32_t crc32 = PARAM_Crc32((const uint8_t *)g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S) - sizeof(uint32_t));
        CVI_LOGI("PARAM_Init ccrc32 %u", crc32);
        if ((g_stParamCtx.pstCfg->MagicStart != PARAM_MAGIC_START)
                || (g_stParamCtx.pstCfg->MagicEnd != PARAM_MAGIC_END) || crc32 != g_stParamCtx.pstCfg->crc32) {
            CVI_LOGE("error, magic error\n");
            munmap((void *)g_stParamCtx.pstCfg, CVIMMAP_SHARE_PARAM_SIZE);
            CVI_LOGE("load back param\n");
            g_stParamCtx.pstCfg = (PARAM_CFG_S *)mmap(NULL, CVIMMAP_SHARE_PARAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, CVIMMAP_SHARE_PARAM_ADDR_BAK);
            if ((g_stParamCtx.pstCfg->MagicStart != PARAM_MAGIC_START)
                || (g_stParamCtx.pstCfg->MagicEnd != PARAM_MAGIC_END)){
                CVI_LOGE("error, back param magic error\n");
            } else {
                PARAM_SetSaveFlg();
                CVI_LOGI("read  back param ok\n");
                CVI_LOGI("power on mode: %d\n", g_stParamCtx.pstCfg->MediaComm.PowerOnMode);
            }
        } else {
            CVI_LOGI("read param ok\n");
            CVI_LOGI("power on mode: %d\n", g_stParamCtx.pstCfg->MediaComm.PowerOnMode);
        }
        close(fd);
    }
#else
    // open bin file
    pthread_mutex_lock(&g_stParamCtx.mutexLock);
    s32Ret = PARAM_LoadFromBin(PARAM_CUR_BIN_PATH);
    pthread_mutex_unlock(&g_stParamCtx.mutexLock);

    if(s32Ret != 0) {
        printf("try load %s\n", PARAM_CUR_BIN_BACK_PATH);
        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        s32Ret = PARAM_LoadFromBin(PARAM_CUR_BIN_BACK_PATH);
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
        if(s32Ret != 0) {
            printf("load failed\n");
            return s32Ret;
        }
    }
    printf("magic start: %x, %x\n", g_stParamCtx.pstCfg->MagicStart, PARAM_MAGIC_START);
    printf("magic end: %x, %x\n", g_stParamCtx.pstCfg->MagicEnd, PARAM_MAGIC_END);
    CVI_LOGI("PARAM_SaveParam lcrc32 %u", g_stParamCtx.pstCfg->crc32);
    uint32_t crc32 = PARAM_Crc32((const uint8_t *)g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S) - sizeof(uint32_t));
    CVI_LOGI("PARAM_SaveParam ccrc32 %u", crc32);

    if ((g_stParamCtx.pstCfg->MagicStart != PARAM_MAGIC_START)
            || (g_stParamCtx.pstCfg->MagicEnd != PARAM_MAGIC_END) || crc32 != g_stParamCtx.pstCfg->crc32) {
        printf("error, magic error\n");
        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        //PARAM_LoadFromBin(PARAM_DEF_BIN_PATH);
        //PARAM_Save2Bin();
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
    }

#endif

    s_stPARAMSaveTskCtx.bRun = true;
    s32Ret = pthread_create(&s_stPARAMSaveTskCtx.tskId, NULL, PARAM_SaveTsk, &s_stPARAMSaveTskCtx);
    APPCOMM_CHECK_RETURN_WITH_ERRINFO(s32Ret, -1, "CreateSaveTsk");
    // CVI_LOGD("Param SaveTsk(%u) create successful\n", s_stPARAMSaveTskCtx.tskId);
    g_stParamCtx.bInit = 1;
    return 0;
}

int32_t  PARAM_Deinit(void)
{
    /* Destroy SaveParam Task */
    s_stPARAMSaveTskCtx.bRun = false;
    pthread_join(s_stPARAMSaveTskCtx.tskId, NULL);
    s_stPARAMSaveTskCtx.tskId = 0;
    s_stPARAMSaveTskCtx.bSaveFlg = false;
    g_stParamCtx.bInit = 0;
    g_stParamCtx.bChanged = 0;

    if (g_stParamCtx.pstCfg != NULL) {
#ifndef LOAD_PARAM_FROM_FILE
        if (CVIMMAP_SHARE_PARAM_SIZE == 0) {
            free(g_stParamCtx.pstCfg);
            g_stParamCtx.pstCfg = NULL;
        }
#else
        free(g_stParamCtx.pstCfg);
        g_stParamCtx.pstCfg = NULL;
#endif
    }

    return 0;
}

int32_t PARAM_Ini2bin(void)
{
    const char* def_bin_path = "/mnt/data/bin/param/app_cfg_def.bin";
    const char* bin_path = "/mnt/data/bin/param/app_cfg_def.bin";
    if (access(bin_path, F_OK) == 0 &&
        access(def_bin_path, F_OK) == 0) {
        CVI_LOGI("bin is exist");
        return 0;
    }else{
        system("chmod +x /mnt/data/bin/param/ini2bin_board");
        system("cd /mnt/data/bin/param && ./ini2bin_board");
    }

    if (access(bin_path, F_OK) == 0 &&
        access(def_bin_path, F_OK) == 0) {
        CVI_LOGI("ini2bin success");
        return 0;
    }else{
        CVI_LOGE("ini2bin failed");
        return -1;
    }
    return 0;
}

PARAM_CONTEXT_S *PARAM_GetCtx(void)
{
    return &g_stParamCtx;
}

int32_t  PARAM_GetParam(PARAM_CFG_S *param)
{
    if (!g_stParamCtx.bInit) {
        CVI_LOGE(" Param not init\n");
        return -1;
    }
    if (param) {
        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(param, g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
    }
    return 0;
}

int32_t  PARAM_Save2Bin(void)
{
    if (!g_stParamCtx.bChanged) {
        printf("%s: param not changed\n", __func__);
        return 0;
    }

    if (g_stParamCtx.pstCfg == NULL) {
        printf("[Error] global param is NULL\n");
        return 0;
    }

   int32_t  writeCnt = 0;
    FILE *fp = NULL;
    fp = fopen(PARAM_CUR_BIN_PATH, "w+b");
    if (fp == NULL) {
        printf("try fopen %s\n", PARAM_CUR_BIN_BACK_PATH);
        fp = fopen(PARAM_CUR_BIN_BACK_PATH, "w+b");
        if (fp == NULL) {
            printf("[Error] %s and %s: fopen failed\n", PARAM_CUR_BIN_PATH, PARAM_CUR_BIN_BACK_PATH);
            return -1;
        }
    }

    writeCnt = fwrite(&g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S), 1, fp);
    if (writeCnt != 1) {
        printf("[Error] write %d\n", writeCnt);
    }
    fflush(fp);
    fclose(fp);
    fp = NULL;
    return 0;
}

int32_t PARAM_Save2Flash(hal_flash *flash, const char *partition, uint64_t addr, uint64_t len)
{
   int32_t  s32Ret = 0;
    uint64_t u64Address = addr;
	uint64_t u64Len = len;
    uint8_t *pBuf = NULL;
    //(void)partition;
    // char *pPartitionName = NULL;
    // TODO: Change the way of validcheck
    if (addr == 0) {
        printf("[Error] input flash addr is NULL\n");
        s32Ret = -1;
        goto EXIT;
    }

    if (g_stParamCtx.pstCfg == NULL) {
        printf("[Error] global param is NULL\n");
        goto EXIT;
    }
    // end of todo
    pBuf = (uint8_t *)malloc(u64Len);
    memcpy(pBuf, g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S));


    //erase
    uint32_t real_erase, real_write;
    ERROR_TYPE check_ret;
    check_ret = flash_erase(flash, u64Address, u64Len,
                                &real_erase);
    if (check_ret != FLASH_OK) {
        printf("Cannot Erase\n");
        goto CLOSE;
    } else {
        printf("Erase success %#"PRIx32"byte\n",
                    real_erase);
    }

    //write buffer
    check_ret = flash_write(flash, u64Address, u64Len, (uint8_t *)pBuf,
                                &real_write, FLASH_RW_FLAG_RAW);
    if (check_ret != FLASH_OK) {
        printf("Cannot Write\n");
    } else {
        printf("Write success %#"PRIx32"byte\n",
                    real_write);
    }

CLOSE:
EXIT:
    if(pBuf){
        free(pBuf);
    }
    return s32Ret;
}

int32_t  PARAM_LoadFromBin(const char *path)
{
   int32_t  fd = 0;
   int32_t  readCount = 0;
    // TODO: change the way of validcheck
    if (path == NULL) {
        printf("[Error] bin file path is NULL\n");
        return -1;
    }
    // endof todo

    if(g_stParamCtx.pstCfg != NULL) {
        return -1;
    }

    g_stParamCtx.pstCfg = (PARAM_CFG_S*)malloc(sizeof(PARAM_CFG_S));
    printf("g_stParamCtx.pstCfg:%p", g_stParamCtx.pstCfg);
    printf("sizeof(PARAM_CFG_S): %zu\n", sizeof(PARAM_CFG_S));
    printf("sizeof(PARAM_HEAD_S): %zu\n", sizeof(PARAM_HEAD_S));
    printf("sizeof(PARAM_FILEMNG_S): %zu\n", sizeof(PARAM_FILEMNG_S));
    printf("sizeof(PARAM_DEVMNG_S): %zu\n", sizeof(PARAM_DEVMNG_S));
    printf("sizeof(PARAM_CAM_CFG): %zu\n", sizeof(PARAM_CAM_CFG));
    printf("sizeof(PARAM_WORK_MODE_S): %zu\n", sizeof(PARAM_WORK_MODE_S));
    printf("sizeof(PARAM_MEDIA_COMM_S): %zu\n", sizeof(PARAM_MEDIA_COMM_S));
    printf("sizeof(PARAM_MENU_S): %zu\n", sizeof(PARAM_MENU_S));
    printf("sizeof(PARAM_MEDIA_SPEC_S): %zu\n", sizeof(PARAM_MEDIA_SPEC_S));
    printf("PARAM_MEDIA_CNT: %d\n", PARAM_MEDIA_CNT);

    fd = open(path, O_RDWR, 777);
    if (fd < 0) {
        printf("load [%s] failed\n", path);
        free(g_stParamCtx.pstCfg);
        g_stParamCtx.pstCfg = NULL;
        return -1;
    }

    readCount = read(fd, g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S));
    if (0 > readCount) {
        printf("read %s failed\n", path);
        free(g_stParamCtx.pstCfg);
        g_stParamCtx.pstCfg = NULL;
        close(fd);
        return -1;
    }
    close(fd);
    printf("[INFO] total: %zd, read: %d\n", sizeof(PARAM_CFG_S), readCount);
    return 0;
}

int32_t  PARAM_LoadFromDefBin(PARAM_CFG_S* param)
{
   int32_t  fd = 0;
   int32_t  readCount = 0;
    const char *path = PARAM_DEF_BIN_PATH;

    fd = open(path, O_RDWR, 777);
    if (fd < 0) {
        CVI_LOGE("load [%s] failed\n", path);
        return -1;
    }

    readCount = read(fd, param, sizeof(PARAM_CFG_S));
    if (0 > readCount) {
        CVI_LOGE("read %s failed\n", path);
        close(fd);
        return -1;
    }

    close(fd);

    CVI_LOGI("magic start: %x, %x\n", param->MagicStart, PARAM_MAGIC_START);
    CVI_LOGI("magic end: %x, %x\n", param->MagicEnd, PARAM_MAGIC_END);
    uint32_t crc32 = PARAM_Crc32((const uint8_t*)param, sizeof(PARAM_CFG_S) - sizeof(uint32_t));
    CVI_LOGI("PARAM_SaveParam ccrc32 %u", crc32);

    if ((param->MagicStart != PARAM_MAGIC_START)
        || (param->MagicEnd != PARAM_MAGIC_END) || crc32 != param->crc32) {
        CVI_LOGE("error, magic error\n");
        return -1;
    }
    CVI_LOGI("[INFO] total: %zd, read: %d\n", sizeof(PARAM_CFG_S), readCount);
    return 0;
}

int32_t PARAM_LoadFromFlash(hal_flash *flash, char *partition, uint64_t addr, uint64_t len)
{
   int32_t  s32Ret = 0;
    //int32_t  readCount = 0;
    uint64_t u64Addr = addr;
    uint64_t u64Len = len;
    ERROR_TYPE check_ret;
    uint8_t *pBuf = (uint8_t *)malloc(len);
    uint32_t real_read;

    if (addr == 0) {
        printf("[Error] input flash addr is NULL\n");
        s32Ret = -1;
        goto EXIT;
    }

    // read flash
    check_ret = flash_read(flash, u64Addr, u64Len, pBuf, &real_read, FLASH_RW_FLAG_RAW);

    if (check_ret == FLASH_OK) {
		printf("read flash fail\n");
        goto EXIT;
    } else {
        printf("read success %#"PRIx32"byte\n",
                        real_read);
    }

    g_stParamCtx.pstCfg = (PARAM_CFG_S*)malloc(sizeof(PARAM_CFG_S));
    memcpy(g_stParamCtx.pstCfg, pBuf, sizeof(PARAM_CFG_S));

    if ((g_stParamCtx.pstCfg->MagicStart != PARAM_MAGIC_START)
            || (g_stParamCtx.pstCfg->MagicEnd != PARAM_MAGIC_END)) {
        printf("error, magic error\n");
        s32Ret = -1;
        free(g_stParamCtx.pstCfg);
        g_stParamCtx.pstCfg = NULL;
        goto EXIT;
    } else {
        printf("read param ok\n");
        printf("power on mode: %d\n", g_stParamCtx.pstCfg->MediaComm.PowerOnMode);
    }
    // printf("head: %x end: %x, length: %d\n",
    //     g_stParamCtx.pstCfg->MagicStart, g_stParamCtx.pstCfg->MagicEnd,
    //     g_stParamCtx.pstCfg->Head.ParamLen);

EXIT:
    if(pBuf){
        free(pBuf);
    }
    return s32Ret;
}

int32_t PARAM_SaveParam(void)
{
    int32_t  s32Ret = 0;
    static PARAM_CFG_S s_stParam;

    if (memcmp(&s_stParam, g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S)) == 0) {
        CVI_LOGI("PARAM_SaveParam: no change, skip\n");
        return 0;
    } else {
        memcpy(&s_stParam, g_stParamCtx.pstCfg, sizeof(PARAM_CFG_S));
    }
    CVI_LOGI("PARAM_SaveParam lcrc32 %u", s_stParam.crc32);
    uint32_t crc32 = PARAM_Crc32((const uint8_t *)&s_stParam, sizeof(PARAM_CFG_S) - sizeof(uint32_t));
    s_stParam.crc32 = crc32;
    g_stParamCtx.pstCfg->crc32 = crc32;
    CVI_LOGI("PARAM_SaveParam ccrc32 %u", crc32);

#ifndef LOAD_PARAM_FROM_FILE
    uint64_t def_cfg_addr_s, def_cfg_addr_e;
    hal_flash *flash = NULL;
    ERROR_TYPE check_ret;
    /* init */
    flash = flash_init();
    if (flash == NULL) {
        CVI_LOGE("Cannot init flash\n");
        return 1;
    }

    def_cfg_addr_s = flash->part_info[APP_CFG_PART_NUM].offset;
    def_cfg_addr_e = flash->part_info[APP_CFG_PART_NUM].offset + flash->part_info[APP_CFG_PART_NUM].size - 1;

    s32Ret = PARAM_Save2Flash(flash, PARAM_PARTITION_NAME, def_cfg_addr_s, def_cfg_addr_e - def_cfg_addr_s + 1);
    if (s32Ret != 0) {
        CVI_LOGE("PARAM_Save2Flash faile \n");

        check_ret = flash_destroy(flash);
        if (check_ret != FLASH_OK) {
            printf("cannot destroy hal_flash !\n");
        }

        return s32Ret;
    }

    check_ret = flash_destroy(flash);
    if (check_ret != FLASH_OK) {
        printf("cannot destroy hal_flash !\n");
    }
#else
    int32_t  fd = 0;

    fd = open(PARAM_CUR_BIN_PATH, O_RDWR, 777);
    if (fd < 0) {
        printf("try open %s\n", PARAM_CUR_BIN_BACK_PATH);
        fd = open(PARAM_CUR_BIN_BACK_PATH, O_RDWR, 777);
        if (fd < 0) {
            printf("open [%s] and %s failed\n", PARAM_CUR_BIN_PATH, PARAM_CUR_BIN_BACK_PATH);
            return -1;
        }
    }

    ssize_t writeCount = write(fd, &s_stParam, sizeof(PARAM_CFG_S));
    if (writeCount == -1) {
        CVI_LOGE("write failed\n");
        close(fd);
        return -1;
    }

    if (fdatasync(fd) == -1) {
        CVI_LOGE("fdatasync failed\n");
        close(fd);
        return -1;
    }

    close(fd);
#endif

    return s32Ret;

}

void PARAM_SetSaveFlg(void)
{
    s_stPARAMSaveTskCtx.bSaveFlg = true;
}

int32_t  PARAM_GetCamStatus(uint32_t CamId, bool *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            memcpy(Param, &pstParamCtx->pstCfg->CamCfg[i].CamEnable, sizeof(bool));
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetMediaMode(uint32_t CamId, PARAM_MEDIA_SPEC_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    memcpy(Param, &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j], sizeof(PARAM_MEDIA_SPEC_S));
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_GetSensorParam(uint32_t CamId, PARAM_MEDIA_SNS_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    memcpy(Param, &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].SnsAttr, sizeof(PARAM_MEDIA_SNS_ATTR_S));
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_SetSensorParam(uint32_t CamId, PARAM_MEDIA_SNS_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    memcpy( &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].SnsAttr, Param, sizeof(PARAM_MEDIA_SNS_ATTR_S));
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_GetVcapParam(uint32_t CamId, PARAM_MEDIA_VACP_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    memcpy(Param, &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VcapAttr, sizeof(PARAM_MEDIA_VACP_ATTR_S));
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_SetVcapParam(uint32_t CamId, PARAM_MEDIA_VACP_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    memcpy(&pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VcapAttr, Param, sizeof(PARAM_MEDIA_VACP_ATTR_S));
                    break;
                }
            }
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}


int32_t PARAM_GetAhdDefaultMode(uint32_t CamId, int32_t *mode)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, value = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            value = pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode;
            break;
        }
    }

    switch (value) {
        case MEDIA_VIDEO_SIZE_1280X720P25:
            *mode = AHD_MODE_1280X720P25;
            break;
        case MEDIA_VIDEO_SIZE_1280X720P30:
           *mode = AHD_MODE_1280X720P30;
            break;
        case MEDIA_VIDEO_SIZE_1920X1080P25:
           *mode = AHD_MODE_1920X1080P25;
            break;
        case MEDIA_VIDEO_SIZE_1920X1080P30:
           *mode = AHD_MODE_1920X1080P30;
            break;
        default:
            CVI_LOGE("value %d is invalid\n", value);
            break;
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetMediaComm(PARAM_MEDIA_COMM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->MediaComm, sizeof(PARAM_MEDIA_COMM_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  PARAM_SetMediaComm(PARAM_MEDIA_COMM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(&pstParamCtx->pstCfg->MediaComm, Param, sizeof(PARAM_MEDIA_COMM_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  PARAM_GetVbParam(MAPI_MEDIA_SYS_ATTR_T *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0, z = 0, t = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&Param->stVIVPSSMode, &pstParamCtx->pstCfg->MediaComm.Vpss.stVIVPSSMode, sizeof(VI_VPSS_MODE_S));
    memcpy(&Param->stVPSSMode, &pstParamCtx->pstCfg->MediaComm.Vpss.stVPSSMode, sizeof(VPSS_MODE_S));
    PARAM_MEDIA_VB_ATTR_S vb[MAX_CAMERA_INSTANCES] = {0};
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++) {
            if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                memcpy(&vb[i], &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Vb, sizeof(PARAM_MEDIA_VB_ATTR_S));
                if (i == 0) {
                    Param->vb_pool_num = vb[i].Poolcnt;
                    for (z = 0; z < Param->vb_pool_num; z++) {
                        memcpy(&Param->vb_pool[z], &vb[i].Vbpool[z], sizeof(MAPI_MEDIA_SYS_VB_POOL_T));
                    }
                } else {
                    MAPI_MEDIA_SYS_ATTR_T tmp;
                    memcpy(&tmp, Param, sizeof(MAPI_MEDIA_SYS_ATTR_T));
                    for (z = 0; z < vb[i].Poolcnt; z++) {
                        for (t = 0; t < tmp.vb_pool_num; t++) {
                            if (vb[i].Vbpool[z].is_frame && tmp.vb_pool[z].is_frame) {
                                if (vb[i].Vbpool[z].vb_blk_size.frame.width == tmp.vb_pool[t].vb_blk_size.frame.width &&\
                                    vb[i].Vbpool[z].vb_blk_size.frame.height == tmp.vb_pool[t].vb_blk_size.frame.height &&\
                                    vb[i].Vbpool[z].vb_blk_size.frame.fmt == tmp.vb_pool[t].vb_blk_size.frame.fmt) {
                                        Param->vb_pool[t].vb_blk_num += vb[i].Vbpool[z].vb_blk_num;
                                        break;
                                }
                            } else {
                                if (vb[i].Vbpool[t].vb_blk_size.size == tmp.vb_pool[z].vb_blk_size.size) {
                                        Param->vb_pool[z].vb_blk_num += vb[i].Vbpool[t].vb_blk_num;
                                        break;
                                }
                            }
                        }
                        if (t == tmp.vb_pool_num) {
                            memcpy(&Param->vb_pool[Param->vb_pool_num], &vb[i].Vbpool[z], sizeof(MAPI_MEDIA_SYS_VB_POOL_T));
                            Param->vb_pool_num += 1;
                        }
                    }
                }
            }
        }
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetWorkModeParam(PARAM_WORK_MODE_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->WorkModeCfg, sizeof(PARAM_WORK_MODE_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetVpssModeParam(PARAM_VPSS_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Vpss, sizeof(PARAM_VPSS_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetKeyMngCfg(KEYMNG_CFG_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->DevMng.stkeyMngCfg, sizeof(KEYMNG_CFG_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  PARAM_GetGaugeMngCfg(GAUGEMNG_CFG_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->DevMng.GaugeCfg, sizeof(GAUGEMNG_CFG_S));
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

int32_t  PARAM_GetVoParam(PARAM_DISP_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Vo, sizeof(PARAM_DISP_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetWndParam(PARAM_WND_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Window, sizeof(PARAM_WND_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetAiParam(MAPI_ACAP_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Ai, sizeof(MAPI_ACAP_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetAencParam(MAPI_AENC_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Aenc, sizeof(MAPI_AENC_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetAoParam(MAPI_AO_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Ao, sizeof(MAPI_AO_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetOsdParam(PARAM_MEDIA_OSD_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
   int32_t  OsdCnt = 0;
    for (int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++) {
            if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                for (int32_t  k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                    Param->OsdAttrs[OsdCnt++] = pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k];
                }
            }
        }
    }
    Param->OsdCnt = OsdCnt;

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetOsdParam(PARAM_MEDIA_OSD_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

   int32_t  OsdCnt = 0;
    for (int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++) {
            if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                for (int32_t  k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k] = Param->OsdAttrs[OsdCnt++];
                }
            }
        }
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    PARAM_SetSaveFlg();
    return 0;
}

int32_t  PARAM_GetStgInfoParam(STG_DEVINFO_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.Stg, sizeof(STG_DEVINFO_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetFileMngParam(PARAM_FILEMNG_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->FileMng, sizeof(PARAM_FILEMNG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetUsbParam(PARAM_USB_MODE_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->WorkModeCfg.UsbMode, sizeof(PARAM_USB_MODE_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

#ifdef SERVICES_SPEECH_ON
int32_t  PARAM_GetSpeechParam(SPEECHMNG_PARAM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }
    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Speech, sizeof(SPEECHMNG_PARAM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetSpeechParam(SPEECHMNG_PARAM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->MediaComm.Speech, Param, sizeof(SPEECHMNG_PARAM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    PARAM_SetSaveFlg();

    return 0;
}
#endif

int32_t  PARAM_GetMediaPhotoSize(PARAM_MEDIA_COMM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    memcpy(Param, &pstParamCtx->pstCfg->MediaComm, sizeof(PARAM_MEDIA_COMM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetMenuParam(PARAM_MENU_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->Menu, sizeof(PARAM_MENU_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetDevParam(PARAM_DEVMNG_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng, sizeof(PARAM_DEVMNG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetWifiParam(PARAM_WIFI_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (true != pstParamCtx->bInit) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.Wifi, sizeof(PARAM_WIFI_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetWifiParam(PARAM_WIFI_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->DevMng.Wifi, Param, sizeof(PARAM_WIFI_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    PARAM_SetSaveFlg();

    return 0;
}

int32_t  PARAM_GetPWMParam(PARAM_PWM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (true != pstParamCtx->bInit) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.PWM, sizeof(PARAM_PWM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetPWMParam(PARAM_PWM_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->DevMng.PWM, Param, sizeof(PARAM_PWM_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    PARAM_SetSaveFlg();

    return 0;
}


int32_t  PARAM_GetGsensorParam(GSENSORMNG_CFG_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }
    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->DevMng.Gsensor, sizeof(GSENSORMNG_CFG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}
int32_t  PARAM_SetGsensorParam(GSENSORMNG_CFG_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->DevMng.Gsensor, Param, sizeof(GSENSORMNG_CFG_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetMenuScreenDormantParam(int32_t  *Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.ScreenDormant.Current)
    {
    case MENU_SCREENDORMANT_OFF:
        *Value = 0;
        break;
    case MENU_SCREENDORMANT_1MIN:
        *Value = 60;
        break;
    case MENU_SCREENDORMANT_3MIN:
        *Value = 180;
        break;
    case MENU_SCREENDORMANT_5MIN:
        *Value = 300;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}
int32_t  PARAM_GetKeyTone(int32_t  *Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.KeyTone.Current)
    {
    case MEDIA_AUDIO_KEYTONE_OFF:
        *Value = 0;
        break;
    case MEDIA_AUDIO_KEYTONE_ON:
        *Value = 1;
        break;
    case MEDIA_AUDIO_KEYTONE_BUIT:
        *Value = 2;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetFatigueDrive(int32_t  *Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.FatigueDirve.Current)
    {
    case MENU_FATIGUEDRIVE_OFF:
        *Value = 0;
        break;
    case MENU_FATIGUEDRIVE_1HOUR:
        *Value = 1;
        break;
    case MENU_FATIGUEDRIVE_2HOUR:
        *Value = 2;
        break;
    case MENU_FATIGUEDRIVE_3HOUR:
        *Value = 3;
        break;
    case MENU_FATIGUEDRIVE_4HOUR:
        *Value = 4;
        break;
    case MENU_FATIGUEDRIVE_BUIT:
        *Value = 5;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetMenuSpeedStampParam(int32_t  *Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.SpeedStamp.Current)
    {
    case MENU_SPEEDSTAMP_OFF:
        *Value = 0;
        break;
    case MENU_SPEEDSTAMP_ON:
        *Value = 1;
        break;
    case MENU_SPEEDSTAMP_BUIT:
        *Value = 2;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetMenuGPSStampParam(int32_t  *Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (pstParamCtx->pstCfg->Menu.GPSStamp.Current)
    {
    case MENU_GPSSTAMP_OFF:
        *Value = 0;
        break;
    case MENU_GPSSTAMP_ON:
        *Value = 1;
        break;
    case MENU_GPSSTAMP_BUIT:
        *Value = 2;
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetParam(PARAM_CFG_S  *param)
{
    if (!g_stParamCtx.bInit) {
        CVI_LOGE(" Param not init\n");
        return -1;
    }

    pthread_mutex_lock(&g_stParamCtx.mutexLock);
    memcpy(g_stParamCtx.pstCfg, param, sizeof(PARAM_CFG_S));
    pthread_mutex_unlock(&g_stParamCtx.mutexLock);

    return 0;
}


void PARAM_GetOsdCarNameParam(char *string_carnum_stamp, MENU_LANGUAGE_E* lang)
{
    PARAM_MEDIA_OSD_ATTR_S Osd;
    PARAM_GetOsdParam(&Osd);
    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);

    pthread_mutex_lock(&g_stParamCtx.mutexLock);
    for(int32_t  i = 0; i < Osd.OsdCnt; i++){
        if(Osd.OsdAttrs[i].stContent.enType == MAPI_OSD_TYPE_STRING){
           int32_t  length = strlen(Osd.OsdAttrs[i].stContent.stStrContent.szStr);
            strncpy(string_carnum_stamp,Osd.OsdAttrs[i].stContent.stStrContent.szStr,length);
            *lang = param.Language.Current;
            break;
        }
    }
    pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}

void PARAM_SetOsdCarNameParam(char *string_carnum_stamp)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
   int32_t  str_size;

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    string_carnum_stamp[strlen(string_carnum_stamp)] = '\0';
    for (int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++) {
            for (int32_t  k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                if(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.enType == MAPI_OSD_TYPE_STRING){
                    str_size = sizeof(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr);
                    memset(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr, 0 , str_size);
                    strncpy(pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr, string_carnum_stamp, strlen(string_carnum_stamp));
                    // printf("set car num = %s\n", pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.stStrContent.szStr);
                }
            }
        }
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    PARAM_SetSaveFlg();
}

void PARAM_GetMdConfigParam(PARAM_MD_ATTR_S *Md)
{
        PARAM_MEDIA_COMM_S MediaComm = {0};
        PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        //Md = &MediaComm.Md;
        memcpy(Md, &MediaComm.Md, sizeof(PARAM_MD_ATTR_S));
        CVI_LOGD("Md.motionSensitivity = %d, MediaComm.Md.motionSensitivity = %d\n", Md->motionSensitivity, MediaComm.Md.motionSensitivity);
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}

#ifdef SERVICES_ADAS_ON
void PARAM_GetADASConfigParam(PARAM_ADAS_ATTR_S *ADAS)
{
        PARAM_MEDIA_COMM_S MediaComm = {0};
        PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(ADAS, &MediaComm.ADAS, sizeof(PARAM_ADAS_ATTR_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}
#endif

#ifdef SERVICES_QRCODE_ON
void PARAM_GetQRCodeConfigParam(PARAM_QRCODE_ATTR_S *QRCODE)
{
        PARAM_MEDIA_COMM_S MediaComm = {0};
        PARAM_GetMediaComm(&MediaComm);

        pthread_mutex_lock(&g_stParamCtx.mutexLock);
        memcpy(QRCODE, &MediaComm.QRCODE, sizeof(PARAM_QRCODE_ATTR_S));
        pthread_mutex_unlock(&g_stParamCtx.mutexLock);
}
#endif

int32_t  PARAM_SetCamStatus(uint32_t CamId, bool Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            pstParamCtx->pstCfg->CamCfg[i].CamEnable = Param;
            uint32_t enSns = pstParamCtx->pstCfg->Menu.ViewWin.Current & 0xFFFF;
            if(Param == true){
                enSns |= (0x1 << CamId);
            }else{
                enSns &= ((~(0x1 << CamId)) & 0xFFFF);
            }
            pstParamCtx->pstCfg->Menu.ViewWin.Current = ((enSns << 16) | enSns);
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetCamIspInfoStatus(uint32_t CamId, bool Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CamID == CamId) {
            pstParamCtx->pstCfg->CamCfg[i].CamIspEnable = Param;
            // CVI_LOGD("CamCfg[i].CamIspEnable  = %d, i = %d, CamId = %d\n", pstParamCtx->pstCfg->CamCfg[i].CamIspEnable , i, CamId);
            break;
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetWndParam(PARAM_WND_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->MediaComm.Window, Param, sizeof(PARAM_WND_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_SetFaceParam(PARAM_FACEP_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(&pstParamCtx->pstCfg->MediaComm.Facep, Param, sizeof(PARAM_FACEP_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_GetFaceParam(PARAM_FACEP_ATTR_S *Param)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    memcpy(Param, &pstParamCtx->pstCfg->MediaComm.Facep, sizeof(PARAM_FACEP_ATTR_S));

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetMenuVideoSize(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.VideoSize.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetMenuVideoLoop(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.VideoLoop.Current = Value;
    PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    switch (Value) {
        case MEDIA_VIDEO_LOOP_1MIN:
            astRecord->SplitTime = 60000; //msec
            break;
        case MEDIA_VIDEO_LOOP_3MIN:
            astRecord->SplitTime = 180000; //msec
            break;
        case MEDIA_VIDEO_LOOP_5MIN:
            astRecord->SplitTime = 300000; //msec
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }
    printf("pstParamCtx->pstCfg->MagicStart == %0x\n", pstParamCtx->pstCfg->MagicStart);
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}



static int32_t PARAM_SetMenuVideoCodec(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0, z = 0;
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.VideoCodec.Current = Value;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
            for (z = 0; z < MAX_VENC_CNT; z++) {
                if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].VencChnEnable == true) {
                    MAPI_VENC_CHN_PARAM_T *attr = &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[z].MapiVencAttr;
                    if (attr->codec == MAPI_VCODEC_H264 || attr->codec == MAPI_VCODEC_H265) {
                        switch (Value) {
                            case MEDIA_VIDEO_VENCTYPE_H264:
                                attr->codec = MAPI_VCODEC_H264;
                                break;
                            case MEDIA_VIDEO_VENCTYPE_H265:
                                attr->codec = MAPI_VCODEC_H265;
                                break;
                            default:
                                CVI_LOGE("Value is invalid");
                                break;
                        }
                    }
                }
            }
        }
    }

    printf("pstParamCtx->pstCfg->MagicStart == %0x\n", pstParamCtx->pstCfg->MagicStart);
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetVENCParam()
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t i = 0, j = 0, z = 0;
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
            for (z = 0; z < MAX_VENC_CNT; z++) {
                MEDIA_VENC_CHN_ATTR_S *s_VencAttr = &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[0];
                MEDIA_VENC_CHN_ATTR_S *st_VencAttr = &pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[1];
                s_VencAttr->bindMode = 0;
                s_VencAttr->MapiVencAttr.pixel_format = 19;
                s_VencAttr->MapiVencAttr.gop = 1;
                s_VencAttr->MapiVencAttr.bitrate_kbps = 500;
                s_VencAttr->MapiVencAttr.bufSize = 1048576;
                s_VencAttr->MapiVencAttr.initialDelay = 2000;
                s_VencAttr->MapiVencAttr.thrdLv = 3;
                s_VencAttr->MapiVencAttr.statTime = 2;
                s_VencAttr->MapiVencAttr.videoSignalTypePresentFlag = 0;
                s_VencAttr->MapiVencAttr.videoFullRangeFlag = 0;
                s_VencAttr->MapiVencAttr.videoFormat = 0;
                st_VencAttr->bindMode = 0;
                st_VencAttr->MapiVencAttr.pixel_format = 19;
                st_VencAttr->MapiVencAttr.gop = 1;
                st_VencAttr->MapiVencAttr.bitrate_kbps = 1500;
                st_VencAttr->MapiVencAttr.bufSize = 262144;
                st_VencAttr->MapiVencAttr.initialDelay = 2000;
                st_VencAttr->MapiVencAttr.thrdLv = 3;
                st_VencAttr->MapiVencAttr.statTime = 2;
                st_VencAttr->MapiVencAttr.videoSignalTypePresentFlag = 0;
                st_VencAttr->MapiVencAttr.videoFullRangeFlag = 0;
                st_VencAttr->MapiVencAttr.videoFormat = 0;
            }
        }
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    PARAM_SetSaveFlg();

    return 0;
}

static int32_t PARAM_SetMenuLapseTime(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
   int32_t  gop = 0;

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.LapseTime.Current = Value;

    PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    switch (Value) {
        case MEDIA_VIDEO_LAPSETIME_OFF:
            gop = 0;
            break;
        case MEDIA_VIDEO_LAPSETIME_1S:
            gop = 1; //sec
            break;
        case MEDIA_VIDEO_LAPSETIME_2S:
            gop = 2; //sec
            break;
        case MEDIA_VIDEO_LAPSETIME_3S:
            gop = 3; //sec
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }
    astRecord->TimelapseGop = gop;

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetMenuAudioStatus(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    pstParamCtx->pstCfg->Menu.AudioEnable.Current = Value;

    PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    switch (Value) {
        case MEDIA_VIDEO_AUDIO_OFF:
            astRecord->AudioStatus = false;
            break;
        case MEDIA_VIDEO_AUDIO_ON:
            astRecord->AudioStatus = true;
            break;
        default:
            CVI_LOGE("Value is invalid");
            break;
    }

    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetMenuOsd(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Osd.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_Set_Pwm_Bri(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PwmBri.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_Set_View_Win(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.ViewWin.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_Get_View_Win(void)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
   int32_t  cur = pstParamCtx->pstCfg->Menu.ViewWin.Current;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return cur;
}

static int32_t PARAM_SetMenuScreenDormant(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.ScreenDormant.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetKeyTone(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.KeyTone.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetFatigueDrive(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.FatigueDirve.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetSpeedStamp(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.SpeedStamp.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetGPSStamp(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.GPSStamp.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetMenuGsensor(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    switch (Value)
    {
    case MENU_GENSOR_OFF:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 0;
        break;
    case MENU_GENSOR_LOW:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 1;
        break;
    case MENU_GENSOR_MID:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 2;
        break;
    case MENU_GENSOR_HIGH:
        pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity = 3;
        break;
    case MENU_GENSOR_BUIT:
        break;
    default:
        break;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);
    return 0;
}

static int32_t PARAM_SetSpeedUnit(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.SpeedUnit.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetRearCamMirror(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.CamMirror.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetLanguage(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Language.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetTimeFormat(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.TimeFormat.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetTimeZone(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.TimeZone.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_Frequence(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Frequence.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetRecEn(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    PARAM_RECORD_CHN_ATTR_S *astRecord = &pstParamCtx->pstCfg->MediaComm.Record.ChnAttrs[CamId];
    astRecord->Enable = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_GetRecLoop(int32_t *Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    *Value = pstParamCtx->pstCfg->Menu.RecLoop.Current;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetRecLoop(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.RecLoop.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetParking(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Parking.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetMotionDet(uint32_t CamId,int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.MotionDet.Current = Value;
    PARAM_MD_CHN_ATTR_S *astMd = &pstParamCtx->pstCfg->MediaComm.Md.ChnAttrs[CamId];
    astMd->Enable = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetPhotoSize(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PhotoSize.Current = Value;
    pstParamCtx->pstCfg->MediaComm.Photo.photoid = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetPhotoQuality(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PhotoQuality.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetLightFrequence(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.LightFrequence.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetFaceDet(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.FaceDet.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetFaceSmile(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.FaceSmile.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetPowerOff(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.PowerOff.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetActionAudio(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.ActionAudio.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetFlashLed(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.FlashLed.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetIspEffect(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.IspEffect.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetAutoScreenOff(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.AutoScreenOff.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t  PARAM_SetCarNumStamp(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.CarNumStamp.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    //PARAM_SetSaveFlg();
    return 0;
}
int32_t  PARAM_SetBootFirstFlag(uint8_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.UserData.bBootFirst = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    PARAM_SetSaveFlg();
    return 0;
}

int32_t PARAM_LoadDefaultParamFromFlash(PARAM_CFG_S* param)
{
    int32_t  ret = 0;
    uint64_t def_cfg_addr_s, def_cfg_addr_e;

    hal_flash *flash = NULL;
    ERROR_TYPE check_ret;
    uint32_t real_read;

    flash = flash_init();
    if (flash == NULL) {
        printf("Cannot init flash\n");
        return 1;
    }

    def_cfg_addr_s = flash->part_info[APP_CFG_PART_NUM].offset;
    def_cfg_addr_e = flash->part_info[APP_CFG_PART_NUM].offset + flash->part_info[APP_CFG_PART_NUM].size - 1;

    uint64_t u64Len = def_cfg_addr_e - def_cfg_addr_s + 1;
    printf("cfg_s = 0x%#"PRIx64", cfg_e = 0x%#"PRIx64"\n", def_cfg_addr_s, def_cfg_addr_e);
    uint8_t *pBuf = (uint8_t *)malloc(u64Len);

    // read flash
    check_ret = flash_read(flash, def_cfg_addr_s, u64Len, pBuf, &real_read, FLASH_RW_FLAG_RAW);

    if (check_ret == FLASH_OK) {
		printf("read flash fail\n");
        goto EXIT;
    } else {
        printf("read success %#"PRIx32"byte\n",
                        real_read);
    }

    memcpy(param, pBuf, sizeof(PARAM_CFG_S));

    if ((param->MagicStart != PARAM_MAGIC_START) ||
        (param->MagicEnd != PARAM_MAGIC_END)) {
        printf("error, magic error\n");
        ret = -1;
        goto EXIT;
    } else {
        printf("read param ok\n");
        printf("power on mode: %d\n", param->MediaComm.PowerOnMode);
    }
    printf("PARAM_SaveParam in load default\n");

EXIT:
    check_ret = flash_destroy(flash);
    if (check_ret != FLASH_OK) {
        printf("cannot destroy hal_flash !\n");
    }

    if(pBuf){
        free(pBuf);
    }
    return ret;
}

int32_t  PARAM_GetVideoSizeEnum(int32_t Value, MEDIA_VIDEO_SIZE_E *VideoSize)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);

    if (pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt < (uint32_t)Value) {
        CVI_LOGE("Value is illeage!, Value = %d, ItemCnt = %d\n", Value, pstParamCtx->pstCfg->Menu.VideoSize.ItemCnt);
        pthread_mutex_unlock(&pstParamCtx->mutexLock);
        return PARAM_ENOTINIT;
    }
    if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_1280X720P25)) {
        *VideoSize = MEDIA_VIDEO_SIZE_1280X720P25;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_1280X720P30)) {
        *VideoSize = MEDIA_VIDEO_SIZE_1280X720P30;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_1920X1080P25)) {
        *VideoSize = MEDIA_VIDEO_SIZE_1920X1080P25;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_1920X1080P30)) {
        *VideoSize = MEDIA_VIDEO_SIZE_1920X1080P30;
    }  else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_1920X1080P60)) {
        *VideoSize = MEDIA_VIDEO_SIZE_1920X1080P60;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_2304X1296P25)) {
        *VideoSize = MEDIA_VIDEO_SIZE_2304X1296P25;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_2304X1296P30)) {
        *VideoSize = MEDIA_VIDEO_SIZE_2304X1296P30;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_2560X1440P25)) {
        *VideoSize = MEDIA_VIDEO_SIZE_2560X1440P25;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_2560X1440P30)) {
        *VideoSize = MEDIA_VIDEO_SIZE_2560X1440P30;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_2560X1600P25)) {
        *VideoSize = MEDIA_VIDEO_SIZE_2560X1600P25;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_2560X1600P30)) {
        *VideoSize = MEDIA_VIDEO_SIZE_2560X1600P30;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_3840X2160P25)) {
        *VideoSize = MEDIA_VIDEO_SIZE_3840X2160P25;
    } else if (!APPCOMM_STRCMP_ENUM(pstParamCtx->pstCfg->Menu.VideoSize.Items[Value].Desc, MEDIA_VIDEO_SIZE_3840X2160P30)) {
        *VideoSize = MEDIA_VIDEO_SIZE_3840X2160P30;
    }
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_SetAoVolume(int32_t volume)
{
    PARAM_CONTEXT_S* pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    if (volume < 0 || volume > 31) {
        CVI_LOGE("Ao Volume is invalid[0~31]: %d\n", volume);
        return PARAM_EINVAL;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->MediaComm.Ao.volume = volume;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetStatusLight(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.StatusLight.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

static int32_t PARAM_SetBrightness(int32_t Value)
{
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();

    if (pstParamCtx->bInit != true) {
        CVI_LOGE("param is not init!\n");
        return PARAM_ENOTINIT;
    }

    pthread_mutex_lock(&pstParamCtx->mutexLock);
    pstParamCtx->pstCfg->Menu.Brightness.Current = Value;
    pthread_mutex_unlock(&pstParamCtx->mutexLock);

    return 0;
}

int32_t PARAM_SetMenuParam(uint32_t CamId, PARAM_MENU_E MenuItem, int32_t Value)
{

    switch(MenuItem) {
        case PARAM_MENU_VIDEO_SIZE:
            PARAM_SetMenuVideoSize(CamId, Value);
            break;
        case PARAM_MENU_VIDEO_LOOP:
            PARAM_SetMenuVideoLoop(CamId, Value);
            break;
        case PARAM_MENU_VIDEO_CODEC:
            PARAM_SetMenuVideoCodec(CamId, Value);
            break;
        case PARAM_MENU_LAPSE_TIME:
            PARAM_SetMenuLapseTime(CamId, Value);
            break;
        case PARAM_MENU_AUDIO_STATUS:
            PARAM_SetMenuAudioStatus(CamId, Value);
            break;
        case PARAM_MENU_OSD_STATUS:
            PARAM_SetMenuOsd(Value);
            break;
        case PARAM_MENU_PWM_BRI_STATUS:
            PARAM_Set_Pwm_Bri(Value);
            break;
        case PARAM_MENU_VIEW_WIN_STATUS:
            PARAM_Set_View_Win(Value);
            break;
        case PARAM_MENU_SCREENDORMANT:
            PARAM_SetMenuScreenDormant(Value);
            break;
        case PARAM_MENU_GSENSOR:
            PARAM_SetMenuGsensor(Value);
            break;
        case PARAM_MENU_FATIGUE_DRIVE:
            PARAM_SetFatigueDrive(Value);
            break;
        case PARAM_MENU_SPEED_STAMP:
            PARAM_SetSpeedStamp(Value);
            break;
        case PARAM_MENU_GPS_STAMP:
            PARAM_SetGPSStamp(Value);
            break;
        case PARAM_MENU_SPEED_UNIT:
            PARAM_SetSpeedUnit(Value);
            break;
        case PARAM_MENU_REARCAM_MIRROR:
            PARAM_SetRearCamMirror(Value);
            break;
        case PARAM_MENU_LANGUAGE:
            PARAM_SetLanguage(Value);
            break;
        case PARAM_MENU_TIME_FORMAT:
            PARAM_SetTimeFormat(Value);
            break;
        case PARAM_MENU_TIME_ZONE:
            PARAM_SetTimeZone(Value);
            break;
        case PARAM_MENU_FREQUENCY:
            PARAM_Frequence(Value);
            break;
        case PARAM_MENU_KEYTONE:
            PARAM_SetKeyTone(Value);
            break;
        case PARAM_MENU_PARKING:
            PARAM_SetParking(Value);
            break;
        case PARAM_MENU_REC_INX:
            PARAM_SetRecEn(CamId, Value);
            break;
        case PARAM_MENU_REC_LOOP:
            PARAM_SetRecLoop(Value);
            break;
        case PARAM_MENU_CARNUM: //stamp
            PARAM_SetCarNumStamp(Value);
            break;
        case PARAM_MENU_PHOTO_SIZE:
            PARAM_SetPhotoSize(Value);
            break;
        case PARAM_MENU_PHOTO_QUALITY:
            PARAM_SetPhotoQuality(Value);
            break;
        case PARAM_MENU_MOTION_DET:
            PARAM_SetMotionDet(CamId, Value);
            break;
        case PARAM_MENU_LIGHT_FREQUENCE:
            PARAM_SetLightFrequence(Value);
            break;
        case PARAM_MENU_FACE_DET:
            PARAM_SetFaceDet(Value);
            break;
        case PARAM_MENU_FACE_SMILE:
            PARAM_SetFaceSmile(Value);
            break;
        case PARAM_MENU_POWER_OFF:
            PARAM_SetPowerOff(Value);
            break;
        case PARAM_MENU_ACTION_AUDIO:
            PARAM_SetActionAudio(Value);
            break;
        case PARAM_MENU_FLASH_LED:
            PARAM_SetFlashLed(Value);
            break;
        case PARAM_MENU_ISP_EFFECT:
            PARAM_SetIspEffect(Value);
            break;
        case PARAM_MENU_AUTO_SCREEN_OFF:
            PARAM_SetAutoScreenOff(Value);
            break;
        case PARAM_MENU_AO_VOLUME:
            PARAM_SetAoVolume(Value);
            break;
        case PARAM_MENU_STATUS_LIGHT:
            PARAM_SetStatusLight(Value);
            break;
        case PARAM_MENU_BRIGHTNESS:
            PARAM_SetBrightness(Value);
            break;
        default:
            CVI_LOGE("not menu item\n");
            break;
    }

    PARAM_SetSaveFlg();

    return 0;
}

#pragma pack(pop)

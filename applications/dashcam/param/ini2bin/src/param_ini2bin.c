#include <stdio.h>
#include <string.h>
#include "param_ini2bin.h"
#include "ini.h"
#include "param_printf.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


#define PARAM_BIN_PATH      "./app_cfg.bin"
#define PARAM_DEF_BIN_PATH  "./app_cfg_def.bin"
#define ACCESS_ENTRY_PATH "./config_access_entry.ini"

static PARAM_CFG_S g_stParamCfg;
PARAM_ACCESS g_ParamAccess;
void verify()
{
    PARAM_CFG_S stParamCfg = {0};

    FILE * fp = fopen(PARAM_BIN_PATH, "rb");
    if(fp == NULL) {
        printf("[Error] verify: Failed to open %s\n", PARAM_BIN_PATH);
        return;
    }
    size_t dummy = fread(&stParamCfg, sizeof(stParamCfg), 1, fp);

    printf("read: %zd, %zd\n", dummy, sizeof(PARAM_CFG_S));
    printf("magic start: %x, %x\n", stParamCfg.MagicStart, PARAM_MAGIC_START);
    printf("magic end: %x, %x\n", stParamCfg.MagicEnd, PARAM_MAGIC_END);

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

    fclose(fp);
}


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

void parse_bin(char *argv[])
{
    int fd;
    const char *bin_path = argv[1];

    memset(&g_stParamCfg, 0, sizeof(g_stParamCfg));
    g_stParamCfg.MagicStart    = PARAM_MAGIC_START;
    g_stParamCfg.MagicEnd      = PARAM_MAGIC_END;
    g_stParamCfg.Head.ParamLen = sizeof(g_stParamCfg);
    printf("[Info] Expected struct size: %zu bytes\n", sizeof(PARAM_CFG_S));
    fd = open(bin_path, O_RDONLY);
    if(fd == -1) {
        printf("[Error] Failed to open %s: %s\n", bin_path, strerror(errno));
        exit(-1);
    }

    ssize_t read_size = read(fd, &g_stParamCfg, sizeof(PARAM_CFG_S));
    if(read_size == -1) {
        printf("[Error] Failed to read %s: %s\n", bin_path, strerror(errno));
        close(fd);
        exit(-1);

    } else if((size_t)read_size != sizeof(PARAM_CFG_S)) {
        printf("[Warning] Read size mismatch: expected %zu, got %zd\n", sizeof(PARAM_CFG_S), read_size);
    }
    close(fd);
    verify();
    printf("\n--- All parameters loaded successfully (from %s). Printing app_cfg_isperr.bin... ---\n", bin_path);
    print_param_cfg(&g_stParamCfg);
    printf("--- Parameter printing completed (from %s) ---\n", bin_path);
}

int32_t main(int argc, char *argv[])
{
    int32_t s32Ret    = 0;
    uint32_t u32Count = 0;
    if(argc >= 2) {
        parse_bin(argv);
        return 0;
    }

    memset(&g_stParamCfg, 0, sizeof(g_stParamCfg));
    g_stParamCfg.MagicStart = PARAM_MAGIC_START;
    g_stParamCfg.MagicEnd = PARAM_MAGIC_END;
    g_stParamCfg.Head.ParamLen = sizeof(g_stParamCfg);

    s32Ret = INI_PARAM_LoadAccessEntry();
    PARAM_CHECK_LOAD_RESULT(s32Ret, "AccessEntry Load");

    s32Ret = INI_PARAM_LoadWorkModeCfg(&g_stParamCfg.WorkModeCfg);
    PARAM_CHECK_LOAD_RESULT(s32Ret, "WorkMode Cfg");

    s32Ret = INI_PARAM_LoadMediaCommCfg(&g_stParamCfg.MediaComm);
    PARAM_CHECK_LOAD_RESULT(s32Ret, "MediaComm Cfg");

    s32Ret = INI_PARAM_LoadMediaCamCfg(g_stParamCfg.CamCfg);
    PARAM_CHECK_LOAD_RESULT(s32Ret, "CamCfg Cfg");

    s32Ret = INI_PARAM_LoadDevmngCfg(&g_stParamCfg.DevMng);
    PARAM_CHECK_LOAD_RESULT(s32Ret, "Devmng Cfg");

    s32Ret = INI_PARAM_LoadFilemngCfg(&g_stParamCfg.FileMng);
    PARAM_CHECK_LOAD_RESULT(s32Ret, "Filemng Cfg");

    s32Ret = INI_PARAM_LoadMenuCfg(&g_stParamCfg.Menu);
    PARAM_CHECK_LOAD_RESULT(s32Ret, "Menu Cfg");

    uint32_t crc32 = PARAM_Crc32((const uint8_t *)&g_stParamCfg, sizeof(PARAM_CFG_S) - sizeof(uint32_t));
    g_stParamCfg.crc32 = crc32;

    printf("PARAM_FILEMNG_S size: %zd\n", sizeof(PARAM_FILEMNG_S));
    printf("PARAM_DEVMNG_S size: %zd\n", sizeof(PARAM_DEVMNG_S));
    printf("PARAM_MENU_S size: %zd\n", sizeof(PARAM_MENU_S));
    printf("PARAM_WORK_MODE_S size: %zd\n", sizeof(PARAM_WORK_MODE_S));
    printf("PARAM_MEDIA_COMM_S size: %zd\n", sizeof(PARAM_MEDIA_COMM_S));
    printf("PARAM_MEDIA_SPEC_S size: %zd\n", sizeof(PARAM_MEDIA_SPEC_S));
    printf("PARAM_CRC32 : %u\n", crc32);
    printf("total size: %zd\n", sizeof(PARAM_CFG_S));

    /* save system settings */
    FILE *fp = NULL;
    fp = fopen(PARAM_BIN_PATH, "w+b");
    PARAM_CHECK_FOPEN_RESULT(fp, PARAM_BIN_PATH);

    u32Count = fwrite(&g_stParamCfg, sizeof(g_stParamCfg), 1, fp);
    if (u32Count != 1) {
        printf("[Error] fwrite: total %zd, write %u\n", sizeof(g_stParamCfg), u32Count);
    }
    fflush(fp);
    fclose(fp);
    fp = NULL;

    /* save system default settings */
    fp = fopen(PARAM_DEF_BIN_PATH, "w+b");
    PARAM_CHECK_FOPEN_RESULT(fp, PARAM_DEF_BIN_PATH);

    u32Count = fwrite(&g_stParamCfg, sizeof(g_stParamCfg), 1, fp);
    if (u32Count != 1) {
        printf("[Error] fwrite: total %zd, write %u\n", sizeof(g_stParamCfg), u32Count);
    }
    fflush(fp);
    fclose(fp);
    fp = NULL;

    verify();

    return 0;
}

int32_t PARAM_LoadModuleInfo(const char *module, char *path, char *name)
{
    INI_GetString(module, "path", "./", path,
        PARAM_MODULE_NAME_LEN, ACCESS_ENTRY_PATH);
    INI_GetString(module, "filename", "error", name,
        PARAM_MODULE_NAME_LEN, ACCESS_ENTRY_PATH);

    printf("load %s,path: %s, name: %s\n", module, path, name);
    return 0;
}

int32_t INI_PARAM_LoadAccessEntry() //(PARAM_ACCESS *entry)
{
    uint32_t i = 0;

    char tmp_module[PARAM_SECTION_LEN] = {0};
    char tmp_key[PARAM_KEY_LEN] = {0};

    printf("\n---enter: %s\n", __func__);
    memset(&g_ParamAccess, 0, sizeof(g_ParamAccess));

    g_ParamAccess.module_num = INI_GetLong("module", "module_num", 0, ACCESS_ENTRY_PATH);
    printf("load module number: %d\n", g_ParamAccess.module_num);

    for(i = 0; i < g_ParamAccess.module_num; i++) {
        memset(tmp_module, 0, sizeof(tmp_module));
        memset(tmp_key, 0, sizeof(tmp_key));
        // memset(tmp_name, 0, sizeof(tmp_name));
        snprintf(tmp_key, sizeof(tmp_key), "module%d", i);
        // get module section name
        INI_GetString("module", tmp_key, "", tmp_module, PARAM_SECTION_LEN, ACCESS_ENTRY_PATH);
        // printf("got key: %s, value: %s\n", tmp_key, tmp_module);
        // get module path and file name
        PARAM_LoadModuleInfo(tmp_module, g_ParamAccess.modules[i].path,
            g_ParamAccess.modules[i].name);
    }
    return 0;
}

int32_t INI_PARAM_MediaString2Uint(uint32_t *MediaMode, char *String)
{

    if(strcmp(String, "record_480p25") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_640X480P25;
    } else if (strcmp(String, "record_720p25") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_1280X720P25;
    } else if (strcmp(String, "record_720p60") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_1280X720P60;
    } else if (strcmp(String, "record_1080p25") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_1920X1080P25;
    } else if (strcmp(String, "record_1080p60") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_1920X1080P60;
    } else if (strcmp(String, "record_1512p25") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_2688X1512P25;
    } else if (strcmp(String, "record_1520p25") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_2688X1520P25;
    } else if (strcmp(String, "record_2160p25") == 0) {
        *MediaMode = MEDIA_VIDEO_SIZE_3840X2160P25;
    } else if (strcmp(String, "photo_6144p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_8192X6144P;
    } else if (strcmp(String, "photo_4536p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_8064X4536P;
    } else if (strcmp(String, "photo_8192p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_8192X8192P;
    } else if (strcmp(String, "photo_6000p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_8000X6000P;
    } else if (strcmp(String, "photo_4320p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_7680X4320P;
    } else if (strcmp(String, "photo_3240p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_5760X3240P;
    } else if (strcmp(String, "photo_4200p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_5600X4200P;
    } else if (strcmp(String, "photo_3456p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_4608X3456P;
    } else if (strcmp(String, "photo_3000p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_4000X3000P;
    } else if (strcmp(String, "photo_2160p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_3840X2160P;
    } else if (strcmp(String, "photo_1944p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_2592X1944P;
    } else if (strcmp(String, "photo_1512p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_2688X1512P;
    } else if (strcmp(String, "photo_1440p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_2560X1440P;
    } else if (strcmp(String, "photo_1080p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_1920X1080P;
    } else if (strcmp(String, "photo_720p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_1280X720P;
    } else if (strcmp(String, "photo_480p") == 0) {
        *MediaMode = MEDIA_PHOTO_SIZE_640X480P;
    } else {
        printf("error mode: %s\n", String);
        return -1;
    }

    return 0;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

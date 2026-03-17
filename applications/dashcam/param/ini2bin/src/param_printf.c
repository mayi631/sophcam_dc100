#include <stdio.h>
#include <string.h>
#include "param.h"
#include "param_printf.h"

static void print_main_header(int section_num, const char *title)
{
    printf("\n==================================================\n");
    printf("=== [%d] %s\n", section_num, title);
    printf("==================================================\n");
}

void print_sub_header(const char *title)
{
    printf("\n--- %s ---\n", title);
}

const char *bool_to_str(bool value)
{
    return value ? "TRUE" : "FALSE";
}

void print_param_cfg(const PARAM_CFG_S *cfg)
{
    if(cfg == NULL) {
        printf("[ERROR] print_param_cfg: Received NULL pointer.\n");
        return;
    }

    printf("===================== PARAM CONFIG START =====================");

    print_main_header(1, "Header & Integrity Check");
    printf("Magic Start:0x%08X\n", cfg->MagicStart);
    printf("Magic End:0x%08X\n", cfg->MagicEnd);
    printf("CRC32:0x%08X\n", cfg->crc32);
    printf("Param Length:%u bytes\n", cfg->Head.ParamLen);

    print_main_header(2, "File Manager Configuration (PARAM_FILEMNG_S)");
    print_param_filemng(&cfg->FileMng);

    print_main_header(3, "Device Manager Configuration (PARAM_DEVMNG_S)");
    print_param_devmng(&cfg->DevMng);

    print_main_header(4, "Device Manager Configuration (PARAM_camcfg_S)");
    print_param_cam_cfg(&cfg->CamCfg[0]);

    print_main_header(5, "Work Mode Configuration (PARAM_WORK_MODE_S)");
    print_param_workmode(&cfg->WorkModeCfg);

    print_main_header(6, "Media Common Configuration (PARAM_MEDIA_COMM_S)");
    printf("Power On Mode: %u (0:Record, 1:Photo, 2:Playback, 3:USB)\n", cfg->MediaComm.PowerOnMode);
    print_param_media_comm(&cfg->MediaComm);

    print_main_header(7, "Menu Configuration (PARAM_MENU_S)");
    print_param_menu(&cfg->Menu);

    printf("(Included in Menu Configuration above)\n");
    printf("\n====================== PARAM CONFIG END ======================\n");
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "screen_mipidsi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#ifndef CONFIG_KERNEL_RHINO
#endif
static int32_t g_MipiDsiFd = -1;

#ifdef CONFIG_SCREEN_I80ST7789
int32_t I80DSI_Init(int32_t dev, const HW_I80_CFG_S *i80_hw_cfg)
{
    CVI_HWI80_Init(0, i80_hw_cfg);
	return 0;
}
#endif

int32_t MIPIDSI_Init(void)
{
#ifndef CONFIG_KERNEL_RHINO
    // g_MipiDsiFd = open(MIPIDSI_DEV, O_RDWR);
    // if (g_MipiDsiFd == -1) {
    //     return -1;
    // }
#else

#endif
    return 0;
}

void MIPIDSI_Deinit(void)
{
#ifndef CONFIG_KERNEL_RHINO
	// int32_t ret = 0;

    // if (g_MipiDsiFd != -1) {
    //     close(g_MipiDsiFd);
    // }
    // ret = system("rmmod cvi_mipi_tx.ko");
	// if(ret != 0){
	// 	printf("rmmod failed!.\n");
	// }
#else

#endif
}

int32_t MIPIDSI_SetDeviceConfig(const struct combo_dev_cfg_s *devCfg)
{
    int32_t ret = 0;
#ifndef CONFIG_KERNEL_RHINO
    // struct combo_dev_cfg_s comboDevCfg = {0};
    // memcpy(&comboDevCfg, devCfg, sizeof(struct combo_dev_cfg_s));

	// ret = mipi_tx_cfg(g_MipiDsiFd, &comboDevCfg);
    // if (ret != 0) {
    //     printf("mipi_tx_cfg failed.\n");
    //     close(g_MipiDsiFd);
    //     return -1 ;
    // }
#else
    struct combo_dev_cfg_s comboDevCfg = {0};
    memcpy(&comboDevCfg, devCfg, sizeof(struct combo_dev_cfg_s));
    ret = CVI_MIPI_TX_INIT(&comboDevCfg);
    if (ret != 0) {
        printf("MIPI_TX_INIT failed with %#x\n", ret);
        return ret;
    }
    ret = CVI_MIPI_TX_Disable(g_MipiDsiFd);
    if (ret != 0) {
        printf("MIPI_TX_Disable failed with %#x\n", ret);
        return ret;
    }
    ret = CVI_MIPI_TX_Cfg(g_MipiDsiFd, &comboDevCfg);
    if (ret != 0) {
        printf("MIPI_TX_Cfg failed with %#x\n", ret);
        return ret;
    }
#endif
    return ret;
}

int32_t MIPIDSI_EnableVideoDataTransport(void)
{
    int32_t ret = 0;
#ifdef CONFIG_KERNEL_RHINO
	ret = CVI_MIPI_TX_Enable(g_MipiDsiFd);
    if (ret != 0) {
        printf("MIPI_TX_Enable failed\n");
        return -1;
    }
#endif
    return ret;
}

int32_t MIPIDSI_DisableVideoDataTransport(void)
{
    int32_t ret = 0;
#ifdef CONFIG_KERNEL_RHINO
	ret = CVI_MIPI_TX_Disable(g_MipiDsiFd);
    if (ret != 0) {
        printf("MIPI_TX_Disable failed\n");
        return -1;
    }
#endif
    return ret;
}

int32_t MIPIDSI_WriteCmd(struct cmd_info_s *cmd_info)
{
	int32_t ret = 0;
#ifdef CONFIG_KERNEL_RHINO
	if (cmd_info->cmd_size == 0)
		return -1;

	ret = CVI_MIPI_TX_SendCmd(g_MipiDsiFd, cmd_info);
	if (-1 == ret) {
		printf("MIPI_TX_SendCmd");
		return -1;
	}
#endif
	return ret;
}

int32_t MIPIDSI_ReadCmd(struct get_cmd_info_s *cmd_info)
{
	int32_t ret = 0;
#ifndef CONFIG_KERNEL_RHINO
	// ret = mipi_tx_recv_cmd(g_MipiDsiFd, cmd_info);
	// if (-1 == ret) {
	// 	printf("mipi_tx_recv_cmd");
	// 	return -1;
	// }
#else

#endif
	return ret;
}

int32_t MIPIDSI_SetHsSettle(const struct hs_settle_s *hs_timing_cfg)
{
	int32_t ret = 0;
#ifndef CONFIG_KERNEL_RHINO

#else
    struct hs_settle_s timing_cfg = {0};
    memcpy(&timing_cfg, hs_timing_cfg, sizeof(struct hs_settle_s));
    ret = CVI_MIPI_TX_SetHsSettle(g_MipiDsiFd, &timing_cfg);
    if (ret != 0) {
        printf("MIPI_TX_SetHsSettle failed with %#x\n", ret);
        return ret;
    }
#endif
	return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


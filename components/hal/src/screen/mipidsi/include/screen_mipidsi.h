#ifndef __SCREEN_MIPIDSI__H__
#define __SCREEN_MIPIDSI__H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif  /* End of #ifdef __cplusplus */

#include <stdint.h>
#include <stdbool.h>

#include "cvi_mipi_tx.h"

#ifdef CONFIG_SCREEN_I80ST7789
#include "cvi_hw_i80.h"
int32_t I80DSI_Init(int32_t dev, const HW_I80_CFG_S *i80_hw_cfg);
#endif

int32_t MIPIDSI_Init(void);
void MIPIDSI_Deinit(void);
int32_t MIPIDSI_SetDeviceConfig(const struct combo_dev_cfg_s* devCfg);
int32_t MIPIDSI_EnableVideoDataTransport(void);
int32_t MIPIDSI_DisableVideoDataTransport(void);
int32_t MIPIDSI_WriteCmd(struct cmd_info_s *cmd_info);
int32_t MIPIDSI_ReadCmd(struct get_cmd_info_s *cmd_info);
int32_t MIPIDSI_SetHsSettle(const struct hs_settle_s *timing_cfg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* SCREEN_MIPIDSI  */


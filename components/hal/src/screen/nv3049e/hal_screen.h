#ifndef __HAL_SCREEN_H__
#define __HAL_SCREEN_H__

// #include "cvi_type.h"
// #include "cvi_mipi_tx.h"
#include "cvi_comm_mipi_tx.h"
#include "hal_gpio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define HORIZONTAL_SYNC_ACTIVE      10
#define HORIZONTAL_BACK_PROCH       60
#define HORIZONTAL_FRONT_PROCH      60
#define HORIZONTAL_ACTIVE           480
#define VERTICAL_SYNC_ACTIVE        5
#define VERTICAL_BACK_PROCH         10
#define VERTICAL_FRONT_PROCH        15
#define VERTICAL_ACTIVE             360
#define HORIZONTAL_SYNC_POLIRATY    1
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	HAL_GPIOB_11  //VIVO_D10 PWM
#define REST_LIGHT	HAL_GPIOE_22  //JTAG_CPU_TRST  LCD_RST
#define POWER_LIGHT	HAL_GPIOB_11  //AUX0 LCD-STB

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

#define RESET_DELAY (10 * 1000)

#define PACKET_DCS 0x29

static const struct combo_dev_cfg_s dev_cfg = {
    .devno = 0,
    .lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
    .lane_pn_swap = {false, false, false, false, false},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .video_mode = BURST_MODE,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .sync_info = {
        .vid_hsa_pixels = HORIZONTAL_SYNC_ACTIVE,
        .vid_hbp_pixels = HORIZONTAL_BACK_PROCH,
        .vid_hfp_pixels = HORIZONTAL_FRONT_PROCH,
        .vid_hline_pixels = HORIZONTAL_ACTIVE,
        .vid_vsa_lines = VERTICAL_SYNC_ACTIVE,
        .vid_vbp_lines = VERTICAL_BACK_PROCH,
        .vid_vfp_lines = VERTICAL_FRONT_PROCH,
        .vid_active_lines = VERTICAL_ACTIVE,
        .vid_vsa_pos_polarity = VERTICAL_SYNC_POLIRATY,
        .vid_hsa_pos_polarity = HORIZONTAL_SYNC_POLIRATY,
    },
    .pixel_clk = 28463,
};

static const struct dsc_instr dsi_init_cmds[24] = {
	{.delay = 0, .data_type = 0x15, .size = 2, .data = {0xFF, 0x30}},
	{.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}},
	{.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x01}},
	{.delay = 0, .data_type = 0x15, .size = 2, .data = {0x31, 0x28}},
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x30, 0x2D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x23, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x25, 0x0F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x26, 0x0F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x04}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x12, 0x44}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x13, 0x44}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x40, 0x90}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x41, 0x63}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x42, 0x93}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x43, 0x73}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x44, 0x26}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x45, 0xFF}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x46, 0xF7}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x47, 0x31}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x48, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x49, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4A, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x17, 0x44}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x59, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x16, 0x31}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xFF, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x60, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x61, 0x0B}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x64, 0x63}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x65, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x66, 0x64}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x70, 0x08}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7D, 0xFE}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7E, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7F, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xd2, 0x76}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xd4, 0x37}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xd3, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xd5, 0x47}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA3, 0x0F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC3, 0x19}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA2, 0x25}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC2, 0x1B}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA7, 0x32}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC7, 0x3C}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA6, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC6, 0x2C}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA4, 0x06}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC4, 0x10}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAF, 0x09}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCF, 0x13}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAE, 0x13}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCE, 0x1D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAD, 0x14}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCD, 0x18}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAC, 0x14}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCC, 0x16}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAB, 0x14}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCB, 0x14}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAA, 0x15}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCA, 0x13}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA9, 0x08}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC9, 0x04}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA8, 0x0F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC8, 0x0D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA1, 0x24}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC1, 0x1E}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA5, 0x3F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC5, 0x3F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA0, 0x22}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC0, 0x25}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xB0, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x03, 0xA6}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x01, 0x85}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x02, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x03, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x04, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x05, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x09, 0x84}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x0A, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x0B, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x0C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x0D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x11, 0x83}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x12, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x13, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x14, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x15, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x19, 0x82}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x1A, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x1B, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x1C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x1D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x21, 0x31}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x22, 0x68}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x23, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x24, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x25, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x29, 0x31}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x2a, 0x69}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x2b, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x2c, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x2d, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x31, 0x31}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x32, 0x6A}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x33, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x34, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x35, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x39, 0x31}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x3A, 0x6B}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x3B, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x3C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x3D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x40, 0x38}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x41, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x42, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x43, 0x68}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x44, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x45, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x46, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x47, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x48, 0x38}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x49, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4A, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4B, 0x69}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4E, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x4F, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x50, 0x38}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x51, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x52, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x53, 0x6A}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x54, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x55, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x56, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x57, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x58, 0x38}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x59, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x5A, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x5B, 0x6B}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x5C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x5D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x5E, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x5F, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x60, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x61, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x62, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x63, 0x6C}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x64, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x65, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x66, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x67, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x68, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x69, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x6A, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x6B, 0x6D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x6C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x6D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x6E, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x6F, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x70, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x71, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x72, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x73, 0x6E}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x74, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x75, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x76, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x77, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x78, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x79, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7A, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7B, 0x6F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7C, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7D, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7E, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x7F, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x81, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x83, 0x10}}，
    #if 0
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x90, 0x09}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x91, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x92, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x93, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x94, 0x05}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x95, 0x07}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x96, 0x0B}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x97, 0x0D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x98, 0x0F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x99, 0x1D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x9A, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x9B, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x9C, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x9D, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x9E, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x9F, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA0, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA1, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA2, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA3, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA4, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA5, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA6, 0x1D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA7, 0x0E}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA8, 0x0C}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xA9, 0x0A}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAA, 0x06}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAB, 0x04}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAC, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAD, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAE, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xAF, 0x08}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x35, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x36, 0x00}}，
    #else
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC0, 0x0A}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC1, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC2, 0x06}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC3, 0x04}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC4, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC5, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC6, 0x08}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC7, 0x0E}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC8, 0x0C}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xC9, 0x1D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCA, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCB, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCC, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCD, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCE, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xCF, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD0, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD1, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD2, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD3, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD4, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD5, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD6, 0x1D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD7, 0x0D}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD8, 0x0F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xD9, 0x09}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xDA, 0x01}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xDB, 0x03}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xDC, 0x05}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xDD, 0x07}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xDE, 0x1F}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xDF, 0x0B}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xFF, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xFF, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xFF, 0x02}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x13, 0xd9}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x71, 0x80}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x30}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x49}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0xff, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x35, 0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x36, 0x03}}，
    #endif
    {.delay = 20, .data_type = 0x15, .size = 2, .data = {0x11,0x00}}，
    {.delay = 0, .data_type = 0x15, .size = 2, .data = {0x2d, 0x00}}，
    {.delay = 20, .data_type = 0x15, .size = 2, .data = {0x29,0x00}}，
};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */
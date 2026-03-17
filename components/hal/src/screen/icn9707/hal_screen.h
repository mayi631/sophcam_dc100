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

#define HORIZONTAL_SYNC_ACTIVE      16
#define HORIZONTAL_BACK_PROCH       58
#define HORIZONTAL_FRONT_PROCH      58
#define HORIZONTAL_ACTIVE           600
#define VERTICAL_SYNC_ACTIVE        10
#define VERTICAL_BACK_PROCH         26
#define VERTICAL_FRONT_PROCH        26
#define VERTICAL_ACTIVE             1600
#define HORIZONTAL_SYNC_POLIRATY    1
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	HAL_GPIOB_11  //VIVO_D10 PWM
#define REST_LIGHT	HAL_GPIOA_20  //JTAG_CPU_TRST  LCD_RST
#define POWER_LIGHT	HAL_GPIOA_30  //AUX0 LCD-STB

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
    .pixel_clk = 84592,
};

// static unsigned char data_ota7290b_1920_0[] = { 0x11, 0x00 };
// static unsigned char data_ota7290b_1920_1[] = { 0xb0, 0x5a };
static unsigned char data_icn9707_1600_2[] = { 0xF0,0x5A, 0x59 };
static unsigned char data_icn9707_1600_3[] = { 0xF1,0xA5, 0xA6 };
static unsigned char data_icn9707_1600_4[] = { 0xB0, 0x98, 0x00, 0x00, 0x89, 0x77, 0x77, 0x77, 0x77, 0x10, 0x00, 0x00, 0x46, 0x00, 0x00, 0x0F };
static unsigned char data_icn9707_1600_5[] = { 0xB1, 0x97, 0xC0, 0x00, 0x91, 0x10, 0x00, 0x00, 0x46, 0x00, 0x00, 0x5F };
static unsigned char data_icn9707_1600_6[] = { 0xB3, 0x05, 0x00, 0x00, 0x13, 0x00, 0x1B, 0x00, 0x11, 0x00, 0x19, 0x00, 0x0F, 0x00, 0x17, 0x03, 0x0D, 0x00, 0x15, 0x1D, 0x22, 0x0B, 0x00 };
static unsigned char data_icn9707_1600_7[] = { 0xB4, 0x04, 0x00, 0x00, 0x12, 0x00, 0x1A, 0x00, 0x10, 0x00, 0x18, 0x00, 0x0E, 0x00, 0x16, 0x00, 0x0C, 0x00, 0x14, 0x1D, 0x22, 0x0A, 0x00 };
static unsigned char data_icn9707_1600_8[] = { 0xB2, 0x00, 0x91, 0x08, 0x8B, 0x08, 0x00, 0x22, 0x00, 0x44, 0xD9 };
static unsigned char data_icn9707_1600_9[] = { 0xB6,0x0E, 0x0E };
static unsigned char data_icn9707_1600_10[] = { 0xB7,0x01, 0x01, 0x09, 0x0D, 0x11, 0x19, 0x1D, 0x15, 0x00, 0x25, 0x21, 0x00, 0x00, 0x00, 0x00, 0x02, 0xF7, 0x38 };
static unsigned char data_icn9707_1600_11[] = { 0xB8,0xB8, 0x52, 0x02, 0xCC };
static unsigned char data_icn9707_1600_12[] = { 0xBA,0x27, 0x63 };
static unsigned char data_icn9707_1600_13[] = { 0xBD,0x43, 0x0E, 0x0E, 0x64, 0x64, 0x23, 0x0A };
static unsigned char data_icn9707_1600_14[] = { 0xC1,0x00, 0x10, 0x10, 0x04, 0x00, 0x3A, 0x3A, 0x08 };
static unsigned char data_icn9707_1600_15[] = { 0xC2,0x31, 0x20 };
static unsigned char data_icn9707_1600_16[] = { 0xC3,0x22, 0x30 };
static unsigned char data_icn9707_1600_17[] = { 0xC6,0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00 };
static unsigned char data_icn9707_1600_18[] = { 0xC8, 0x7C, 0x62, 0x50, 0x43, 0x3E, 0x2F, 0x35, 0x21, 0x3F, 0x42, 0x45, 0x65, 0x54, 0x5A, 0x4C, 0x49, 0x3D, 0x2B, 0x06, 0x7C, 0x62, 0x50, 0x43, 0x3E, 0x2F, 0x35, 0x21, 0x3F, 0x42, 0x45, 0x65, 0x54, 0x5A, 0x4C, 0x49, 0x3D, 0x2B, 0x06 };
static unsigned char data_icn9707_1600_19[] = { 0xD0,0x07, 0xFF, 0xFF };
static unsigned char data_icn9707_1600_20[] = { 0xD2,0x63, 0x0B, 0x08, 0x88 };
static unsigned char data_icn9707_1600_21[] = { 0xD4,0x00, 0x00, 0x00, 0x32, 0x04, 0x51 };
static unsigned char data_icn9707_1600_22[] = { 0xF1,0x5A, 0x59 };
static unsigned char data_icn9707_1600_23[] = { 0xF0,0xA5, 0xA6 };
static unsigned char data_icn9707_1600_24[] = { 0x11,0x00 };
static unsigned char data_icn9707_1600_25[] = { 0x29,0x00 };

static const struct dsc_instr dsi_init_cmds[24] = {
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_2 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_3 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 16, .data = data_icn9707_1600_4 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 12, .data = data_icn9707_1600_5 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 23, .data = data_icn9707_1600_6 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 23, .data = data_icn9707_1600_7 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 11, .data = data_icn9707_1600_8 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_9 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 19, .data = data_icn9707_1600_10 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 5, .data = data_icn9707_1600_11 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_12 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 8, .data = data_icn9707_1600_13 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 9, .data = data_icn9707_1600_14 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_15 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_16 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 9, .data = data_icn9707_1600_17 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 39, .data = data_icn9707_1600_18 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 4, .data = data_icn9707_1600_19 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 5, .data = data_icn9707_1600_20 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 7, .data = data_icn9707_1600_21 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_22 },
	{.delay = 0, .data_type = PACKET_DCS, .size = 3, .data = data_icn9707_1600_23 },
	{.delay = 120, .data_type = PACKET_DCS, .size = 2, .data = data_icn9707_1600_24 },
	{.delay = 10, .data_type = PACKET_DCS, .size = 2, .data = data_icn9707_1600_25 },
};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */
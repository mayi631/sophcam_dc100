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

#define HORIZONTAL_SYNC_ACTIVE      64
#define HORIZONTAL_BACK_PROCH       36
#define HORIZONTAL_FRONT_PROCH      128
#define HORIZONTAL_ACTIVE           720
#define VERTICAL_SYNC_ACTIVE        16
#define VERTICAL_BACK_PROCH         4
#define VERTICAL_FRONT_PROCH        6
#define VERTICAL_ACTIVE             1280
#define HORIZONTAL_SYNC_POLIRATY    0
#define VERTICAL_SYNC_POLIRATY      1

#define BACK_LIGHT	HAL_GPIOE_00
#define REST_LIGHT	HAL_GPIOE_01
#define POWER_LIGHT	HAL_GPIOE_02

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

#define RESET_DELAY (10 * 1000)

static const struct combo_dev_cfg_s dev_cfg = {
    .devno = 0,
    .lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
    .lane_pn_swap = {true, true, true, true, true},
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
        .vid_vsa_pos_polarity = HORIZONTAL_SYNC_POLIRATY,
        .vid_hsa_pos_polarity = VERTICAL_SYNC_POLIRATY,
    },
    .pixel_clk = 74250,
};

static unsigned char data_hx8394_0[] = { 0xb9, 0xff, 0x83, 0x94 };
static unsigned char data_hx8394_1[] = {
	0xb1, 0x50, 0x15, 0x75, 0x09, 0x32, 0x44, 0x71, 0x31, 0x4d,
	0x2f, 0x56, 0x73, 0x02, 0x02
};

static unsigned char data_hx8394_2[] = {
/*#ifdef MIPI_PANEL_2_LANES
	0xba, 0x61, 0x03, 0x68, 0x6b, 0xb2, 0xc0
#else*/
	0xba, 0x63, 0x03, 0x68, 0x6b, 0xb2, 0xc0
//#endif
};

static unsigned char data_hx8394_3[] = { 0xd2, 0x88 };
static unsigned char data_hx8394_4[] = { 0xb2, 0x00, 0x80, 0x64, 0x10, 0x07 };
static unsigned char data_hx8394_5[] = {
	0xb4, 0x01, 0x75, 0x01, 0x75, 0x01, 0x75, 0x01, 0x0c, 0x86,
	0x75, 0x00, 0x3f, 0x01, 0x75, 0x01, 0x75, 0x01, 0x75, 0x01,
	0x0c, 0x86
};

static unsigned char data_hx8394_6[] = {
	0xd3, 0x00, 0x00, 0x06, 0x06, 0x40, 0x1a, 0x08, 0x00, 0x32,
	0x10, 0x08, 0x00, 0x08, 0x54, 0x15, 0x10, 0x05, 0x04, 0x02,
	0x12, 0x10, 0x05, 0x07, 0x23, 0x23, 0x0c, 0x0c, 0x27, 0x10,
	0x07, 0x07, 0x10, 0x40
};

static unsigned char data_hx8394_7[] = {
	0xd5, 0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x20,
	0x21, 0x22, 0x23, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x19, 0x19, 0x18, 0x18, 0x18, 0x18, 0x1b, 0x1b, 0x1a,
	0x1a, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18
};

static unsigned char data_hx8394_8[] = {
	0xd6, 0x03, 0x02, 0x01, 0x00, 0x07, 0x06, 0x05, 0x04, 0x23,
	0x22, 0x21, 0x20, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x58,
	0x58, 0x18, 0x18, 0x19, 0x19, 0x18, 0x18, 0x1b, 0x1b, 0x1a,
	0x1a, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18
};

static unsigned char data_hx8394_9[] = {
	0xe0, 0x00, 0x1a, 0x24, 0x2c, 0x2e, 0x32, 0x34, 0x32, 0x66,
	0x73, 0x82, 0x7f, 0x85, 0x95, 0x97, 0x99, 0xa4, 0xa5, 0xa0,
	0xab, 0xba, 0x5a, 0x59, 0x5d, 0x61, 0x63, 0x6c, 0x72, 0x7f,
	0x00, 0x19, 0x24, 0x2c, 0x2e, 0x32, 0x34, 0x32, 0x66, 0x73,
	0x82, 0x7f, 0x85, 0x95, 0x97, 0x99, 0xa4, 0xa5, 0xa0, 0xab,
	0xba, 0x5a, 0x59, 0x5d, 0x61, 0x63, 0x6c, 0x72, 0x7f
};

static unsigned char data_hx8394_10[] = { 0xcc, 0x03 };
static unsigned char data_hx8394_11[] = { 0xc0, 0x1f, 0x73 };
static unsigned char data_hx8394_12[] = { 0xb6, 0x42, 0x42 };
static unsigned char data_hx8394_13[] = { 0xd4, 0x02 };
static unsigned char data_hx8394_14[] = { 0xbd, 0x01 };
static unsigned char data_hx8394_15[] = { 0xb1, 0x00 };
static unsigned char data_hx8394_16[] = { 0xbd, 0x00 };
static unsigned char data_hx8394_17[] = {
	0xbf, 0x40, 0x81, 0x50, 0x00, 0x1a, 0xfc, 0x01
};

static unsigned char data_hx8394_18[] = { 0xc6, 0xef };
static unsigned char data_hx8394_19[] = { 0x36, 0x02 };// h-flip
static unsigned char data_hx8394_20[] = { 0x11 };
static unsigned char data_hx8394_21[] = { 0x29 };

static const struct dsc_instr dsi_init_cmds[] = {
	{.delay = 0, .data_type = 0x29, .size = 4, .data = data_hx8394_0 },
	{.delay = 0, .data_type = 0x29, .size = 15, .data = data_hx8394_1 },
	{.delay = 0, .data_type = 0x29, .size = 7, .data = data_hx8394_2 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_3 },
	{.delay = 0, .data_type = 0x29, .size = 6, .data = data_hx8394_4 },
	{.delay = 0, .data_type = 0x29, .size = 22, .data = data_hx8394_5 },
	{.delay = 0, .data_type = 0x29, .size = 34, .data = data_hx8394_6 },
	{.delay = 0, .data_type = 0x29, .size = 45, .data = data_hx8394_7 },
	{.delay = 0, .data_type = 0x29, .size = 45, .data = data_hx8394_8 },
	{.delay = 0, .data_type = 0x29, .size = 59, .data = data_hx8394_9 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_10 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_hx8394_11 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_hx8394_12 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_14 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_15 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_16 },
	{.delay = 0, .data_type = 0x29, .size = 8, .data = data_hx8394_17 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_18 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_hx8394_19 },
	{.delay = 120, .data_type = 0x05, .size = 1, .data = data_hx8394_20 },
	{.delay = 20, .data_type = 0x05, .size = 1, .data = data_hx8394_21 }
};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */
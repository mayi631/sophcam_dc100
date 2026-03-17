#ifndef _MIPI_TX_PARAM_ST_7701_H_
#define _MIPI_TX_PARAM_ST_7701_H_

// #include "cvi_type.h"
// #include "cvi_mipi_tx.h"
#include "cvi_comm_mipi_tx.h"
#include "hal_gpio.h"

#define PANEL_NAME "NETEASE-2"

#if 1
#define ST7701_NETEASE_VACT		360
#define ST7701_NETEASE_VSA		2
#define ST7701_NETEASE_VBP		12
#define ST7701_NETEASE_VFP		26

#define ST7701_NETEASE_HACT		480
#define ST7701_NETEASE_HSA		36
#define ST7701_NETEASE_HBP		60
#define ST7701_NETEASE_HFP		86

#else

#define ST7701_NETEASE_VACT		480//360
#define ST7701_NETEASE_VSA		6//2
#define ST7701_NETEASE_VBP		20//12
#define ST7701_NETEASE_VFP		20//26

#define ST7701_NETEASE_HACT		360//480
#define ST7701_NETEASE_HSA		2//6
#define ST7701_NETEASE_HBP		12//20
#define ST7701_NETEASE_HFP		26//20

#endif

#define HORIZONTAL_SYNC_ACTIVE      ST7701_NETEASE_HSA
#define HORIZONTAL_BACK_PROCH       ST7701_NETEASE_HBP
#define HORIZONTAL_FRONT_PROCH      ST7701_NETEASE_HFP
#define HORIZONTAL_ACTIVE           ST7701_NETEASE_HACT
#define VERTICAL_SYNC_ACTIVE        ST7701_NETEASE_VSA
#define VERTICAL_BACK_PROCH         ST7701_NETEASE_VBP
#define VERTICAL_FRONT_PROCH        ST7701_NETEASE_VFP
#define VERTICAL_ACTIVE             ST7701_NETEASE_VACT
#define HORIZONTAL_SYNC_POLIRATY    1
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	HAL_GPIOE_00
#define REST_LIGHT	HAL_GPIOA_20
#define POWER_LIGHT	HAL_GPIOE_00

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

#define RESET_DELAY (10 * 1000)

#define PIXEL_CLK(x) ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) \
	* (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * 60 / 1000)

static const struct combo_dev_cfg_s dev_cfg =
	{
		.devno = 0,
     	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_CLK, MIPI_TX_LANE_1, -1, -1},
     	.lane_pn_swap = {false, false, false, false, false},
		.output_mode = OUTPUT_MODE_DSI_VIDEO,
		.video_mode = BURST_MODE,
		.output_format = OUT_FORMAT_RGB_24_BIT,
		.sync_info = {
			.vid_hsa_pixels = ST7701_NETEASE_HSA,
			.vid_hbp_pixels = ST7701_NETEASE_HBP,
			.vid_hfp_pixels = ST7701_NETEASE_HFP,
			.vid_hline_pixels = ST7701_NETEASE_HACT,
			.vid_vsa_lines = ST7701_NETEASE_VSA,
			.vid_vbp_lines = ST7701_NETEASE_VBP,
			.vid_vfp_lines = ST7701_NETEASE_VFP,
			.vid_active_lines = ST7701_NETEASE_VACT,
			.vid_vsa_pos_polarity = false,
			.vid_hsa_pos_polarity = true,
		},
		.pixel_clk = 28173,//25248,//12624,//28463,
	};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };


static unsigned char data_st7701_0[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13 };
static unsigned char data_st7701_1[] = { 0xef, 0x08 };
static unsigned char data_st7701_2[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x10 };
static unsigned char data_st7701_3[] = { 0xc0, 0x2c, 0x00 };
static unsigned char data_st7701_4[] = { 0xc1, 0x10, 0x0c };
static unsigned char data_st7701_5[] = { 0xc2, 0x21, 0x0a };
static unsigned char data_st7701_6[] = { 0xcc, 0x10 };
static unsigned char data_st7701_7[] = { 0xB0, 0x00, 0x12, 0x9A, 0x0E, 0x12, 0x07, 0x0B, 0x08, 0x09, 0x26, 0x05, 0x51, 0x0F, 0x69, 0x30, 0x1C};
static unsigned char data_st7701_8[] = { 0xB1, 0x00, 0x12, 0x9B, 0x0D, 0x10, 0x06, 0x0B, 0x09, 0x08, 0x25, 0x03, 0x50, 0x0F, 0x68, 0x30, 0x1C};
static unsigned char data_st7701_9[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x11 };
static unsigned char data_st7701_10[] = { 0xb0, 0x5d };
static unsigned char data_st7701_11[] = { 0xb1, 0x66 };
static unsigned char data_st7701_12[] = { 0xb2, 0x84 };
static unsigned char data_st7701_13[] = { 0xb3, 0x80 };
static unsigned char data_st7701_14[] = { 0xb5, 0x4e };
static unsigned char data_st7701_15[] = { 0xb7, 0x85 };
static unsigned char data_st7701_16[] = { 0xb8, 0x20 };
static unsigned char data_st7701_17[] = { 0xC1, 0x78 };
static unsigned char data_st7701_18[] = { 0xC2, 0x78 };
static unsigned char data_st7701_19[] = { 0xD0, 0x88 };
static unsigned char data_st7701_20[] = { 0xE0, 0x00, 0x00, 0x02 };
static unsigned char data_st7701_21[] = { 0xE1, 0x06, 0xa0, 0x08, 0xa0, 0x05, 0xa0, 0x07, 0xa0, 0x00, 0x44, 0x44 };
static unsigned char data_st7701_22[] = { 0xE2, 0x00, 0x00, 0x44, 0x44, 0x01, 0xa0, 0x00, 0x00, 0x01, 0xa0, 0x00, 0x00, 0xE3, 0x00, 0x00, 0x11 };
static unsigned char data_st7701_23[] = { 0xE5, 0x0d, 0x79, 0x0a, 0xa0, 0x0f, 0x7b, 0x0a, 0xa0, 0x09, 0x75, 0x0a, 0xa0, 0x0b, 0x77, 0x0a, 0xa0 };
static unsigned char data_st7701_24[] = {	0xE6, 0x00, 0x00, 0x11, 0x11};
static unsigned char data_st7701_25[] = {	0xE7, 0x44, 0x44};
static unsigned char data_st7701_26[] = {	0xE8, 0x0c, 0x78, 0x0a, 0xa0, 0x0e, 0x7a, 0x0a, 0xa0, 0x08, 0x74, 0x0a, 0xa0, 0x0a, 0x76, 0x0a, 0xa0};
static unsigned char data_st7701_27[] = { 0xE9, 0x36, 0x00 };
static unsigned char data_st7701_28[] = { 0xEB, 0x00, 0x01, 0xE4, 0xE4, 0x44, 0x88, 0x40 };
static unsigned char data_st7701_29[] = {	0xED, 0xFF, 0x45, 0x67, 0xFA, 0x01, 0x2B, 0xAB, 0xFF, 0xFF, 0xBA, 0xB2, 0x10, 0xAF, 0x76, 0x54, 0xFF};
static unsigned char data_st7701_30[] = { 0xEF, 0x10, 0x0D, 0x04, 0x08, 0x3F, 0x1F };
static unsigned char data_st7701_31[] = { 0xFF, 0x77, 0x01, 0x00, 0x00, 0x00 };
static unsigned char data_st7701_32[] = {	0xFF, 0x77, 0x01, 0x00, 0x00, 0x13};
static unsigned char data_st7701_33[] = {	0xE8, 0x00, 0x0E};
static unsigned char data_st7701_34[] = {	0x11};
static unsigned char data_st7701_35[] = {	0xE8, 0x00, 0x0C};
static unsigned char data_st7701_36[] = { 0xE8, 0x00, 0x00 };
static unsigned char data_st7701_37[] = { 0xFF, 0x77, 0x01, 0x00, 0x00, 0x00 };
static unsigned char data_st7701_38[] = { 0x29 };

static const struct dsc_instr dsi_init_cmds[] = {
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701_0 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_1 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701_2 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_3 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_4 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_5 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_6 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701_7 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701_8 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701_9 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_10 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_11 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_12 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_14 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_15 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_16 },

	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_17 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_18 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_st7701_19 },

	{.delay = 0, .data_type = 0x39, .size = 4, .data = data_st7701_20 },
	{.delay = 0, .data_type = 0x39, .size = 12, .data = data_st7701_21 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701_22 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701_23 },
	{.delay = 0, .data_type = 0x39, .size = 5, .data = data_st7701_24 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_25 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701_26 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_27 },
	{.delay = 0, .data_type = 0x39, .size = 8, .data = data_st7701_28 },
	{.delay = 0, .data_type = 0x39, .size = 17, .data = data_st7701_29 },
	{.delay = 0, .data_type = 0x39, .size = 7, .data = data_st7701_30 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701_31 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701_32 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_33 },
	{.delay = 120, .data_type = 0x05, .size = 1, .data = data_st7701_34 },
	{.delay = 10, .data_type = 0x39, .size = 3, .data = data_st7701_35 },
	{.delay = 0, .data_type = 0x39, .size = 3, .data = data_st7701_36 },
	{.delay = 0, .data_type = 0x39, .size = 6, .data = data_st7701_37 },
	{.delay = 20, .data_type = 0x05, .size = 1, .data = data_st7701_38 },
};

#else
#error "_MIPI_TX_PARAM_ST_7701_H_ multi-delcaration!!"
#endif // _MIPI_TX_PARAM_ST_7701_H_

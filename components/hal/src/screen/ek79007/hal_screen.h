#ifndef _MIPI_TX_PARAM_EK79007_H_
#define _MIPI_TX_PARAM_EK79007_H_

#include "cvi_comm_mipi_tx.h"
#include "hal_gpio.h"

#define PANEL_NAME "NETEASE-2"

#define EK79007_NETEASE_VACT		600
#define EK79007_NETEASE_VSA		1
#define EK79007_NETEASE_VBP		23
#define EK79007_NETEASE_VFP		12

#define EK79007_NETEASE_HACT		1024
#define EK79007_NETEASE_HSA		10
#define EK79007_NETEASE_HBP		160
#define EK79007_NETEASE_HFP		160

#define RESET_DELAY (50 * 1000)

#define HORIZONTAL_SYNC_ACTIVE      EK79007_NETEASE_HSA
#define HORIZONTAL_BACK_PROCH       EK79007_NETEASE_HBP
#define HORIZONTAL_FRONT_PROCH      EK79007_NETEASE_HFP
#define HORIZONTAL_ACTIVE           EK79007_NETEASE_HACT
#define VERTICAL_SYNC_ACTIVE        EK79007_NETEASE_VSA
#define VERTICAL_BACK_PROCH         EK79007_NETEASE_VBP
#define VERTICAL_FRONT_PROCH        EK79007_NETEASE_VFP
#define VERTICAL_ACTIVE             EK79007_NETEASE_VACT
#define HORIZONTAL_SYNC_POLIRATY    1
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	HAL_GPIOE_00
#define REST_LIGHT	HAL_GPIOA_20
#define POWER_LIGHT	HAL_GPIOA_30

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

#define PIXEL_CLK(x) ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) \
	* (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * 60 / 1000)

static const struct combo_dev_cfg_s dev_cfg =
	{
		.devno = 0,
		.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
		.output_mode = OUTPUT_MODE_DSI_VIDEO,
		.video_mode = BURST_MODE,
		.output_format = OUT_FORMAT_RGB_24_BIT,
		.sync_info = {
			.vid_hsa_pixels = EK79007_NETEASE_HSA,
			.vid_hbp_pixels = EK79007_NETEASE_HBP,
			.vid_hfp_pixels = EK79007_NETEASE_HFP,
			.vid_hline_pixels = EK79007_NETEASE_HACT,
			.vid_vsa_lines = EK79007_NETEASE_VSA,
			.vid_vbp_lines = EK79007_NETEASE_VBP,
			.vid_vfp_lines = EK79007_NETEASE_VFP,
			.vid_active_lines = EK79007_NETEASE_VACT,
			.vid_vsa_pos_polarity = false,
			.vid_hsa_pos_polarity = true,
		},
		.pixel_clk = 170000,
	};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

static const struct dsc_instr dsi_init_cmds[] = {
	{ .delay = 30, .data_type = 0x15, .size = 2, .data = {0x1} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x80, 0x8B} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x81, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x82, 0x3C} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x83, 0xFF} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x84, 0xCA} },
	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x85, 0xAF} },

	{ .delay = 0, .data_type = 0x15, .size = 2, .data = {0x86, 0xB5} },
};

#else
#error "_MIPI_TX_PARAM_EK79007_H_ multi-delcaration!!"
#endif // _MIPI_TX_PARAM_GC9503V_H_

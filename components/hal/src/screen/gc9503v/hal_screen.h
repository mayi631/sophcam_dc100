#ifndef _MIPI_TX_PARAM_GC9503V_H_
#define _MIPI_TX_PARAM_GC9503V_H_

#include "cvi_comm_mipi_tx.h"
#include "hal_gpio.h"

#define PANEL_NAME "NETEASE-2"

#define GC9053V_NETEASE_VACT		640
#define GC9053V_NETEASE_VSA		8
#define GC9053V_NETEASE_VBP		30
#define GC9053V_NETEASE_VFP		30

#define GC9053V_NETEASE_HACT		480
#define GC9053V_NETEASE_HSA		8
#define GC9053V_NETEASE_HBP		60
#define GC9053V_NETEASE_HFP		60

#define RESET_DELAY (50 * 1000)

#define HORIZONTAL_SYNC_ACTIVE      GC9053V_NETEASE_HSA
#define HORIZONTAL_BACK_PROCH       GC9053V_NETEASE_HBP
#define HORIZONTAL_FRONT_PROCH      GC9053V_NETEASE_HFP
#define HORIZONTAL_ACTIVE           GC9053V_NETEASE_HACT
#define VERTICAL_SYNC_ACTIVE        GC9053V_NETEASE_VSA
#define VERTICAL_BACK_PROCH         GC9053V_NETEASE_VBP
#define VERTICAL_FRONT_PROCH        GC9053V_NETEASE_VFP
#define VERTICAL_ACTIVE             GC9053V_NETEASE_VACT
#define HORIZONTAL_SYNC_POLIRATY    1
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	HAL_GPIOE_00
#define REST_LIGHT	HAL_GPIOA_20
#define POWER_LIGHT	HAL_GPIOE_00

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

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
			.vid_hsa_pixels = GC9053V_NETEASE_HSA,
			.vid_hbp_pixels = GC9053V_NETEASE_HBP,
			.vid_hfp_pixels = GC9053V_NETEASE_HFP,
			.vid_hline_pixels = GC9053V_NETEASE_HACT,
			.vid_vsa_lines = GC9053V_NETEASE_VSA,
			.vid_vbp_lines = GC9053V_NETEASE_VBP,
			.vid_vfp_lines = GC9053V_NETEASE_VFP,
			.vid_active_lines = GC9053V_NETEASE_VACT,
			.vid_vsa_pos_polarity = false,
			.vid_hsa_pos_polarity = true,
		},
		.pixel_clk = (PIXEL_CLK(GC9053V_NETEASE)),
	};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };


static unsigned char data_gc9503v_0[] = { 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00 };
static unsigned char data_gc9503v_1[] = { 0xF6, 0x5A, 0x87 };
static unsigned char data_gc9503v_2[] = { 0xC1, 0x3F };
static unsigned char data_gc9503v_3[] = { 0xC2, 0x0E };
static unsigned char data_gc9503v_4[] = { 0xC6, 0xF8 };
static unsigned char data_gc9503v_5[] = { 0xC9, 0x10 };
static unsigned char data_gc9503v_6[] = { 0xCD, 0x25 };
static unsigned char data_gc9503v_7[] = { 0xFA, 0x08, 0x08, 0x00, 0x04 };
static unsigned char data_gc9503v_8[] = { 0xF8, 0x8A };
static unsigned char data_gc9503v_9[] = { 0x71, 0x48 };
static unsigned char data_gc9503v_10[] = { 0x72, 0x48 };
static unsigned char data_gc9503v_11[] = { 0x73, 0x00, 0x44 };
static unsigned char data_gc9503v_12[] = { 0x97, 0xEE };
static unsigned char data_gc9503v_13[] = { 0x83, 0x93 };
static unsigned char data_gc9503v_14[] = { 0xA3, 0x22 };
static unsigned char data_gc9503v_15[] = { 0xFD, 0x28, 0x3C, 0x00 };
static unsigned char data_gc9503v_16[] = { 0xAC, 0x45 };
static unsigned char data_gc9503v_17[] = { 0xA7, 0x47 };
static unsigned char data_gc9503v_18[] = { 0xA0, 0xDD };
static unsigned char data_gc9503v_19[] = { 0x9A, 0xC0 };
static unsigned char data_gc9503v_20[] = { 0x9B, 0x40 };
static unsigned char data_gc9503v_21[] = { 0x82, 0x3E, 0x3E };
static unsigned char data_gc9503v_22[] = { 0xB1, 0x10 };
static unsigned char data_gc9503v_23[] = { 0x7A, 0x13, 0x1A };
static unsigned char data_gc9503v_24[] = { 0x7B, 0x13, 0x1A };
static unsigned char data_gc9503v_25[] = { 0x6D, 0x00, 0x1F, 0x19, 0x1A, 0x10, 0x0E, 0x0C, 0x0A, 0x02, 0x08, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x07, 0x01, 0x09, 0x0B, 0x0D, 0x0F, 0x1A, 0x19, 0x1F, 0x00 };
static unsigned char data_gc9503v_26[] = { 0x64, 0x28, 0x04, 0x02, 0x7C, 0x03, 0x03, 0x28, 0x03, 0x02, 0x7D, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A };
static unsigned char data_gc9503v_27[] = { 0x65, 0x28, 0x02, 0x02, 0x7E, 0x03, 0x03, 0x28, 0x01, 0x02, 0x7F, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A };
static unsigned char data_gc9503v_28[] = { 0x66, 0x28, 0x00, 0x02, 0x80, 0x03, 0x03, 0x20, 0x01, 0x02, 0x81, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A };
static unsigned char data_gc9503v_29[] = { 0x67, 0x20, 0x02, 0x02, 0x82, 0x03, 0x03, 0x20, 0x03, 0x02, 0x83, 0x03, 0x03, 0x7A, 0x7A, 0x7A, 0x7A };
static unsigned char data_gc9503v_30[] = { 0x68, 0x77, 0x08, 0x06, 0x08, 0x05, 0x7A, 0x7A, 0x08, 0x06, 0x08, 0x05, 0x7A, 0x7A };
static unsigned char data_gc9503v_31[] = { 0x60, 0x28, 0x06, 0x7A, 0x7A, 0x28, 0x05, 0x7A, 0x7A };
static unsigned char data_gc9503v_32[] = { 0x63, 0x22, 0x7E, 0x7A, 0x7A, 0x22, 0x7F, 0x7A, 0x7A };
static unsigned char data_gc9503v_33[] = { 0x69, 0x14, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08 };
static unsigned char data_gc9503v_34[] = { 0x6B, 0x07 };
static unsigned char data_gc9503v_35[] = { 0xD1, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF };
static unsigned char data_gc9503v_36[] = { 0xD2, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF };
static unsigned char data_gc9503v_37[] = { 0xD3, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF };
static unsigned char data_gc9503v_38[] = { 0xD4, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF };
static unsigned char data_gc9503v_39[] = { 0xD5, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF };
static unsigned char data_gc9503v_40[] = { 0xD6, 0x00, 0x00, 0x00, 0x80, 0x00, 0xBF, 0x00, 0xF8, 0x01, 0x20, 0x01, 0x4D, 0x01, 0x68, 0x01, 0x88, 0x01, 0xB8, 0x01, 0xF0, 0x02, 0x28, 0x02, 0x7D, 0x02, 0xB8, 0x02, 0xBA, 0x03, 0x00, 0x03, 0x4D, 0x03, 0x6A, 0x03, 0x88, 0x03, 0xA0, 0x03, 0xB5, 0x03, 0xC8, 0x03, 0xD0, 0x03, 0xE8, 0x03, 0xF0, 0x03, 0xF8, 0x03, 0xFF };
static unsigned char data_gc9503v_41[] = { 0x11 };
static unsigned char data_gc9503v_42[] = { 0x29 };


static const struct dsc_instr dsi_init_cmds[] = {
	{.delay = 0, .data_type = 0x29, .size = 6, .data = data_gc9503v_0 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_1 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_2 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_3 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_4 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_5 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_6 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_gc9503v_7 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_8 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_9 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_10 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_11 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_12 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_14 },
	{.delay = 0, .data_type = 0x29, .size = 4, .data = data_gc9503v_15 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_16 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_17 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_18 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_19 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_20 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_21 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_22 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_23 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_24 },
	{.delay = 0, .data_type = 0x29, .size = 33, .data = data_gc9503v_25 },
	{.delay = 0, .data_type = 0x29, .size = 17, .data = data_gc9503v_26 },
	{.delay = 0, .data_type = 0x29, .size = 17, .data = data_gc9503v_27 },
	{.delay = 0, .data_type = 0x29, .size = 17, .data = data_gc9503v_28 },
	{.delay = 0, .data_type = 0x29, .size = 17, .data = data_gc9503v_29 },
	{.delay = 0, .data_type = 0x29, .size = 14, .data = data_gc9503v_30 },
	{.delay = 0, .data_type = 0x29, .size = 9, .data = data_gc9503v_31 },
	{.delay = 0, .data_type = 0x29, .size = 9, .data = data_gc9503v_32 },
	{.delay = 0, .data_type = 0x29, .size = 8, .data = data_gc9503v_33 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_34 },
	{.delay = 0, .data_type = 0x29, .size = 53, .data = data_gc9503v_35 },
	{.delay = 0, .data_type = 0x29, .size = 53, .data = data_gc9503v_36 },
	{.delay = 0, .data_type = 0x29, .size = 53, .data = data_gc9503v_37 },
	{.delay = 0, .data_type = 0x29, .size = 53, .data = data_gc9503v_38 },
    {.delay = 0, .data_type = 0x29, .size = 53, .data = data_gc9503v_39 },
    {.delay = 0, .data_type = 0x29, .size = 53, .data = data_gc9503v_40 },
    {.delay = 120, .data_type = 0x05, .size = 1, .data = data_gc9503v_41 },
    {.delay = 20, .data_type = 0x05, .size = 1, .data = data_gc9503v_42 },
};


static unsigned char data_gc9503v_44[] = { 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00 };
static unsigned char data_gc9503v_45[] = { 0xC1, 0x3F };
static unsigned char data_gc9503v_46[] = { 0x6C, 0x60 };
static unsigned char data_gc9503v_47[] = { 0xB1, 0x00 };
static unsigned char data_gc9503v_48[] = { 0xFA, 0x7F, 0x00, 0x00, 0x00 };
static unsigned char data_gc9503v_49[] = { 0x6C, 0x50 };
static unsigned char data_gc9503v_50[] = { 0x28, 0x00 };
static unsigned char data_gc9503v_51[] = { 0x10, 0x00 };
static unsigned char data_gc9503v_52[] = { 0xF0, 0x55, 0xAA, 0x52, 0x08, 0x00 };
static unsigned char data_gc9503v_53[] = { 0xC2, 0xce };
static unsigned char data_gc9503v_54[] = { 0xc3, 0xcd };
static unsigned char data_gc9503v_55[] = { 0xc6, 0xfc };
static unsigned char data_gc9503v_56[] = { 0xc5, 0x03 };
static unsigned char data_gc9503v_57[] = { 0xcd, 0x64 };
static unsigned char data_gc9503v_58[] = { 0xc4, 0xff };
static unsigned char data_gc9503v_59[] = { 0xc9, 0xcd };
static unsigned char data_gc9503v_60[] = { 0xF6, 0x5a, 0x87 };
static unsigned char data_gc9503v_61[] = { 0xFd, 0xaa,0xaa, 0x0a };
static unsigned char data_gc9503v_62[] = { 0xFe, 0x6a,0x0a };
static unsigned char data_gc9503v_63[] = { 0x78, 0x2a,0xaa };
static unsigned char data_gc9503v_64[] = { 0x92, 0x17,0x08 };
static unsigned char data_gc9503v_65[] = { 0x77, 0xaa,0x2a };
static unsigned char data_gc9503v_66[] = { 0x76, 0xaa,0xaa };
static unsigned char data_gc9503v_67[] = { 0x84, 0x00 };
static unsigned char data_gc9503v_68[] = { 0x78, 0x2b,0xba };
static unsigned char data_gc9503v_69[] = { 0x89, 0x73 };
static unsigned char data_gc9503v_70[] = { 0x88, 0x3A };
static unsigned char data_gc9503v_71[] = { 0x85, 0xB0 };
static unsigned char data_gc9503v_72[] = { 0x76, 0xeb,0xaa };
static unsigned char data_gc9503v_73[] = { 0x94, 0x80 };
static unsigned char data_gc9503v_74[] = { 0x87, 0x04,0x07,0x30 };
static unsigned char data_gc9503v_75[] = { 0x93, 0x27 };
static unsigned char data_gc9503v_76[] = { 0xaf, 0x02 };

static const struct dsc_instr dsi_deinit_cmds[] ={
    {.delay = 0, .data_type = 0x29, .size = 6, .data = data_gc9503v_44 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_45 },
    {.delay = 20, .data_type = 0x15, .size = 2, .data = data_gc9503v_46 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_47 },
    {.delay = 20, .data_type = 0x29, .size = 5, .data = data_gc9503v_48 },
    {.delay = 20, .data_type = 0x15, .size = 2, .data = data_gc9503v_49 },
    {.delay = 10, .data_type = 0x15, .size = 2, .data = data_gc9503v_50 },
    {.delay = 50, .data_type = 0x15, .size = 2, .data = data_gc9503v_51 },
    {.delay = 0, .data_type = 0x29, .size = 6, .data = data_gc9503v_52 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_53 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_54 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_55 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_56 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_57 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_58 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_59 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_60 },
    {.delay = 0, .data_type = 0x29, .size = 4, .data = data_gc9503v_61 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_62 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_63 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_64 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_65 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_66 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_67 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_68 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_69 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_70 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_71 },
    {.delay = 0, .data_type = 0x29, .size = 3, .data = data_gc9503v_72 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_73 },
    {.delay = 0, .data_type = 0x29, .size = 4, .data = data_gc9503v_74 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_75 },
    {.delay = 0, .data_type = 0x15, .size = 2, .data = data_gc9503v_76 }
};

#else
#error "_MIPI_TX_PARAM_GC9503V_H_ multi-delcaration!!"
#endif // _MIPI_TX_PARAM_GC9503V_H_

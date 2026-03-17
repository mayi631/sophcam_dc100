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

#define HORIZONTAL_SYNC_ACTIVE      24
#define HORIZONTAL_BACK_PROCH       136
#define HORIZONTAL_FRONT_PROCH      160
#define HORIZONTAL_ACTIVE           1024
#define VERTICAL_SYNC_ACTIVE        2
#define VERTICAL_BACK_PROCH         21
#define VERTICAL_FRONT_PROCH        12
#define VERTICAL_ACTIVE             600
#define HORIZONTAL_SYNC_POLIRATY    0
#define VERTICAL_SYNC_POLIRATY      1

#define BACK_LIGHT	HAL_GPIOE_00
#define REST_LIGHT	HAL_GPIOA_20
#define POWER_LIGHT	HAL_GPIOA_30

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

#define RESET_DELAY (10 * 1000)

static const struct combo_dev_cfg_s dev_cfg = {
    .devno = 0,
    //.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
	.lane_id = {MIPI_TX_LANE_CLK, MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_2, MIPI_TX_LANE_3},
    .lane_pn_swap = {false, false, false, false, false},
    .output_mode = OUTPUT_MODE_DSI_VIDEO,
    .video_mode = BURST_MODE,
    .output_format = OUT_FORMAT_RGB_24_BIT,
    .sync_info = {
        .vid_hsa_pixels = HORIZONTAL_SYNC_ACTIVE, //24
        .vid_hbp_pixels = HORIZONTAL_BACK_PROCH, //136
        .vid_hfp_pixels = HORIZONTAL_FRONT_PROCH, //160
        .vid_hline_pixels = HORIZONTAL_ACTIVE, //1024
        .vid_vsa_lines = VERTICAL_SYNC_ACTIVE, // 2
        .vid_vbp_lines = VERTICAL_BACK_PROCH, //21
        .vid_vfp_lines = VERTICAL_FRONT_PROCH, // 12
        .vid_active_lines = VERTICAL_ACTIVE, //600
        .vid_vsa_pos_polarity = HORIZONTAL_SYNC_POLIRATY, // 0
        .vid_hsa_pos_polarity = VERTICAL_SYNC_POLIRATY, // 1
    },
    .pixel_clk = 51206,
};

//pixel_clk=(htotal*vtotal)*fps/1000
//htotal=vid_hsa_pixels+ vid_hbp_pixels+ vid_hfp_pixels+ vid_hline_pixels
//vtotal= vid_vsa_lines+ vid_vbp_lines+ vid_vfp_lines+ vid_active_lines
//fps = 60


static unsigned char data_jd9165a_0[] = { 0x30, 0x00 };
static unsigned char data_jd9165a_1[] = { 0xF7, 0x49, 0x61, 0x02, 0x00 };

static unsigned char data_jd9165a_2[] = { 0x30, 0x01 };
//static unsigned char data_jd9165a_2_1[] = { 0x05, 0x01 };
//static unsigned char data_jd9165a_2_2[] = { 0x06, 0x30 };

static unsigned char data_jd9165a_3[] = { 0x04, 0x0C };
static unsigned char data_jd9165a_4[] = { 0x05, 0x00 };
static unsigned char data_jd9165a_5[] = { 0x0B, 0x10 };

static unsigned char data_jd9165a_6[] = { 0x23, 0x38 };

static unsigned char data_jd9165a_7[] = { 0x28, 0x18 };

static unsigned char data_jd9165a_8[] = { 0x29, 0x29 };

static unsigned char data_jd9165a_9[] = { 0x2A, 0x01 };

static unsigned char data_jd9165a_10[] = { 0x2B, 0x29 };
static unsigned char data_jd9165a_11[] = { 0x2C, 0x01 };
static unsigned char data_jd9165a_12[] = { 0x2F, 0x07 };
static unsigned char data_jd9165a_13[] = { 0x31, 0x88 };
static unsigned char data_jd9165a_14[] = { 0x32, 0xC1 };
static unsigned char data_jd9165a_15[] = { 0x33, 0x04 };
static unsigned char data_jd9165a_16[] = { 0x30, 0x02 };
static unsigned char data_jd9165a_17[] = { 0x00, 0x05 };

static unsigned char data_jd9165a_18[] = { 0x01, 0x22 };
static unsigned char data_jd9165a_19[] = { 0x02, 0x08 };
static unsigned char data_jd9165a_20[] = { 0x03, 0x12 };
static unsigned char data_jd9165a_21[] = { 0x04, 0x16 };

static unsigned char data_jd9165a_22[] = { 0x05, 0x64 };
static unsigned char data_jd9165a_23[] = { 0x06, 0x00 };
static unsigned char data_jd9165a_24[] = { 0x07, 0x00 };
static unsigned char data_jd9165a_25[] = { 0x08, 0x78 };
static unsigned char data_jd9165a_26[] = { 0x09, 0x00 };
static unsigned char data_jd9165a_27[] = { 0x0A, 0x04 };
static unsigned char data_jd9165a_28[] = { 0x0B, 0x16, 0x17, 0x0B, 0x0D, 0x0D, 0x0D, 0x11, 0x10, 0x07, 0x07, 0x09 };
static unsigned char data_jd9165a_29[] = { 0x0C, 0x09, 0x1E, 0x1E, 0x1C, 0x1C, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D };
static unsigned char data_jd9165a_30[] = { 0x0D, 0x0A, 0x05, 0x0B, 0x0D, 0x0D, 0x0D, 0x11, 0x10, 0x06, 0x06, 0x08 };
static unsigned char data_jd9165a_31[] = { 0x0E, 0x08, 0x1F, 0x1F, 0x1D, 0x1D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D };
static unsigned char data_jd9165a_32[] = { 0x0F, 0x0A, 0x05, 0x0D, 0x0B, 0x0D, 0x0D, 0x11, 0x10, 0x1D, 0x1D, 0x1F };
static unsigned char data_jd9165a_33[] = { 0x10, 0x1F, 0x08, 0x08, 0x06, 0x06, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D };
static unsigned char data_jd9165a_34[] = { 0x11, 0x16, 0x17, 0x0D, 0x0B, 0x0D, 0x0D, 0x11, 0x10, 0x1C, 0x1C, 0x1E };
static unsigned char data_jd9165a_35[] = { 0x12, 0x1E, 0x09, 0x09, 0x07, 0x07, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D };
static unsigned char data_jd9165a_36[] = { 0x13, 0x00, 0x00, 0x00, 0x00 };
static unsigned char data_jd9165a_37[] = { 0x14, 0x00, 0x00, 0x41, 0x41 };
static unsigned char data_jd9165a_38[] = { 0x15, 0x00, 0x00, 0x00, 0x00 };
static unsigned char data_jd9165a_39[] = { 0x17, 0x00 };
static unsigned char data_jd9165a_40[] = { 0x18, 0x85 };
static unsigned char data_jd9165a_41[] = { 0x19, 0x06, 0x09 };
static unsigned char data_jd9165a_42[] = { 0x1A, 0x05, 0x08 };
static unsigned char data_jd9165a_43[] = { 0x1B, 0x0A, 0x04 };
static unsigned char data_jd9165a_44[] = { 0x26, 0x00 };
static unsigned char data_jd9165a_45[] = { 0x27, 0x00 };
static unsigned char data_jd9165a_46[] = { 0x30, 0x06 };
static unsigned char data_jd9165a_47[] = { 0x12, 0x3F, 0x27, 0x28, 0x35, 0x1B, 0x17, 0x16, 0x13, 0x10, 0x01, 0x23, 0x1B, 0x10, 0x30 };
static unsigned char data_jd9165a_48[] = { 0x13, 0x3F, 0x27, 0x28, 0x35, 0x1D, 0x18, 0x16, 0x13, 0x10, 0x02, 0x24, 0x1B, 0x10, 0x30 };
static unsigned char data_jd9165a_49[] = { 0x30, 0x08 };
static unsigned char data_jd9165a_50[] = { 0x0C, 0x1A };
static unsigned char data_jd9165a_51[] = { 0x0D, 0x0E };
static unsigned char data_jd9165a_52[] = { 0x30, 0x0A };
static unsigned char data_jd9165a_53[] = { 0x02, 0x4F };
static unsigned char data_jd9165a_54[] = { 0x0B, 0x40 };
static unsigned char data_jd9165a_55[] = { 0x30, 0x0D };
static unsigned char data_jd9165a_56[] = { 0x01, 0x00 };
static unsigned char data_jd9165a_57[] = { 0x02, 0x21 };
static unsigned char data_jd9165a_58[] = { 0x03, 0x81 };
static unsigned char data_jd9165a_59[] = { 0x04, 0x40 };
static unsigned char data_jd9165a_60[] = { 0x0C, 0x20 };
static unsigned char data_jd9165a_61[] = { 0x0D, 0x04 };
static unsigned char data_jd9165a_62[] = { 0x0F, 0x13 };
static unsigned char data_jd9165a_63[] = { 0x10, 0x0C };
static unsigned char data_jd9165a_64[] = { 0x11, 0x0C };
static unsigned char data_jd9165a_65[] = { 0x12, 0x0C };
static unsigned char data_jd9165a_66[] = { 0x13, 0x0C };
static unsigned char data_jd9165a_67[] = { 0x30, 0x00 };
static unsigned char data_jd9165a_68[] = { 0x11 };
static unsigned char data_jd9165a_69[] = { 0x29 };

// 05 无数据 15一个数据 29多个数据
static const struct dsc_instr dsi_init_cmds[] = {
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_0 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_jd9165a_1 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_2 },
	//{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_2_1 },
	//{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_2_2 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_3 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_4 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_5 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_6 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_7 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_8 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_9 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_10 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_11 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_12 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_14 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_15 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_16 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_17 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_18 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_19 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_20 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_21 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_22 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_23 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_24 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_25 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_26 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_27 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_28 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_29 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_30 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_31 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_32 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_33 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_34 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_35 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_jd9165a_36 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_jd9165a_37 },
	{.delay = 0, .data_type = 0x29, .size = 5, .data = data_jd9165a_38 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_39 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_40 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_jd9165a_41 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_jd9165a_42 },
	{.delay = 0, .data_type = 0x29, .size = 3, .data = data_jd9165a_43 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_44 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_45 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_46 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_47 },
	{.delay = 0, .data_type = 0x29, .size = 12, .data = data_jd9165a_48 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_49 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_50 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_51 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_52 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_53 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_54 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_55 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_56 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_57 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_58 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_59 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_60 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_61 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_62 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_63 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_64 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_65 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_66 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_jd9165a_67 },

	{.delay = 120, .data_type = 0x05, .size = 1, .data = data_jd9165a_68 },
	{.delay = 20, .data_type = 0x05, .size = 1, .data = data_jd9165a_69 }
};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */
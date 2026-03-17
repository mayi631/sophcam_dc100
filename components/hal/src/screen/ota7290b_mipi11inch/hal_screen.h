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

#define HORIZONTAL_SYNC_ACTIVE      30
#define HORIZONTAL_BACK_PROCH       50
#define HORIZONTAL_FRONT_PROCH      150
#define HORIZONTAL_ACTIVE           440
#define VERTICAL_SYNC_ACTIVE        20
#define VERTICAL_BACK_PROCH         30
#define VERTICAL_FRONT_PROCH        150
#define VERTICAL_ACTIVE             1920
#define HORIZONTAL_SYNC_POLIRATY    1
#define VERTICAL_SYNC_POLIRATY      0

#define BACK_LIGHT	HAL_GPIOE_00
#define REST_LIGHT	HAL_GPIOA_20
#define POWER_LIGHT	HAL_GPIOA_30

#define  SCREEN_TYPE  HAL_COMP_SCREEN_INTF_TYPE_MIPI;

#define RESET_DELAY (10 * 1000)

static const struct combo_dev_cfg_s dev_cfg = {
    .devno = 0,
    .lane_id = {MIPI_TX_LANE_CLK, MIPI_TX_LANE_1, MIPI_TX_LANE_0, MIPI_TX_LANE_3, MIPI_TX_LANE_2},
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
        .vid_vsa_pos_polarity = VERTICAL_SYNC_POLIRATY,
        .vid_hsa_pos_polarity = HORIZONTAL_SYNC_POLIRATY,
    },
    .pixel_clk = 85224,
};

// static unsigned char data_ota7290b_1920_0[] = { 0x11, 0x00 };
// static unsigned char data_ota7290b_1920_1[] = { 0xb0, 0x5a };
static unsigned char data_ota7290b_1920_2[] = { 0xb0, 0x5a };
static unsigned char data_ota7290b_1920_3[] = { 0xb1, 0x00 };
static unsigned char data_ota7290b_1920_4[] = { 0x89, 0x01 };
#ifdef _MIPI_TX_BIST_MODE_
static unsigned char data_ota7290b_1920_5[] = { 0x91, 0x16 };
#else
static unsigned char data_ota7290b_1920_5[] = { 0x91, 0x17 };
#endif
static unsigned char data_ota7290b_1920_6[] = { 0xb1, 0x03 };
static unsigned char data_ota7290b_1920_7[] = { 0x2c, 0x28 };
static unsigned char data_ota7290b_1920_8[] = { 0x00, 0xb7 };
static unsigned char data_ota7290b_1920_9[] = { 0x01, 0x1b };
static unsigned char data_ota7290b_1920_10[] = { 0x02, 0x00 };
static unsigned char data_ota7290b_1920_11[] = { 0x03, 0x00 };
static unsigned char data_ota7290b_1920_12[] = { 0x04, 0x00 };
static unsigned char data_ota7290b_1920_13[] = { 0x05, 0x00 };
static unsigned char data_ota7290b_1920_14[] = { 0x06, 0x00 };
static unsigned char data_ota7290b_1920_15[] = { 0x07, 0x00 };
static unsigned char data_ota7290b_1920_16[] = { 0x08, 0x00 };
static unsigned char data_ota7290b_1920_17[] = { 0x09, 0x00 };
static unsigned char data_ota7290b_1920_18[] = { 0x0a, 0x01 };
static unsigned char data_ota7290b_1920_19[] = { 0x0b, 0x3c };
static unsigned char data_ota7290b_1920_20[] = { 0x0c, 0x00 };
static unsigned char data_ota7290b_1920_21[] = { 0x0d, 0x00 };
static unsigned char data_ota7290b_1920_22[] = { 0x0e, 0x24 };
static unsigned char data_ota7290b_1920_23[] = { 0x0f, 0x1c };
static unsigned char data_ota7290b_1920_24[] = { 0x10, 0xc9 };
static unsigned char data_ota7290b_1920_25[] = { 0x11, 0x60 };
static unsigned char data_ota7290b_1920_26[] = { 0x12, 0x70 };
static unsigned char data_ota7290b_1920_27[] = { 0x13, 0x01 };
static unsigned char data_ota7290b_1920_28[] = { 0x14, 0xe3 };
static unsigned char data_ota7290b_1920_29[] = { 0x15, 0xff };
static unsigned char data_ota7290b_1920_30[] = { 0x16, 0x3d };
static unsigned char data_ota7290b_1920_31[] = { 0x17, 0x0e };
static unsigned char data_ota7290b_1920_32[] = { 0x18, 0x01 };
static unsigned char data_ota7290b_1920_33[] = { 0x19, 0x00 };
static unsigned char data_ota7290b_1920_34[] = { 0x1a, 0x00 };
static unsigned char data_ota7290b_1920_35[] = { 0x1b, 0xfc };
static unsigned char data_ota7290b_1920_36[] = { 0x1c, 0x0b };
static unsigned char data_ota7290b_1920_37[] = { 0x1d, 0xa0 };
static unsigned char data_ota7290b_1920_38[] = { 0x1e, 0x03 };
static unsigned char data_ota7290b_1920_39[] = { 0x1f, 0x04 };
static unsigned char data_ota7290b_1920_40[] = { 0x20, 0x0c };
static unsigned char data_ota7290b_1920_41[] = { 0x21, 0x00 };
static unsigned char data_ota7290b_1920_42[] = { 0x22, 0x04 };
static unsigned char data_ota7290b_1920_43[] = { 0x23, 0x81 };
static unsigned char data_ota7290b_1920_44[] = { 0x24, 0x1f };
static unsigned char data_ota7290b_1920_45[] = { 0x25, 0x10 };
static unsigned char data_ota7290b_1920_46[] = { 0x26, 0x9b };
static unsigned char data_ota7290b_1920_47[] = { 0x2d, 0x01 };
static unsigned char data_ota7290b_1920_48[] = { 0x2e, 0x84 };
static unsigned char data_ota7290b_1920_49[] = { 0x2f, 0x00 };
static unsigned char data_ota7290b_1920_50[] = { 0x30, 0x02 };
static unsigned char data_ota7290b_1920_51[] = { 0x31, 0x08 };
static unsigned char data_ota7290b_1920_52[] = { 0x32, 0x01 };
static unsigned char data_ota7290b_1920_53[] = { 0x33, 0x1c };
static unsigned char data_ota7290b_1920_54[] = { 0x34, 0x70 };
static unsigned char data_ota7290b_1920_55[] = { 0x35, 0xff };
static unsigned char data_ota7290b_1920_56[] = { 0x36, 0xff };
static unsigned char data_ota7290b_1920_57[] = { 0x37, 0xff };
static unsigned char data_ota7290b_1920_58[] = { 0x38, 0xff };
static unsigned char data_ota7290b_1920_59[] = { 0x39, 0xff };
static unsigned char data_ota7290b_1920_60[] = { 0x3a, 0x05 };
static unsigned char data_ota7290b_1920_61[] = { 0x3b, 0x00 };
static unsigned char data_ota7290b_1920_62[] = { 0x3c, 0x00 };
static unsigned char data_ota7290b_1920_63[] = { 0x3d, 0x00 };
static unsigned char data_ota7290b_1920_64[] = { 0x3e, 0x0f };
static unsigned char data_ota7290b_1920_65[] = { 0x3f, 0x8c };
static unsigned char data_ota7290b_1920_66[] = { 0x40, 0x2a };
static unsigned char data_ota7290b_1920_67[] = { 0x41, 0xfc };
static unsigned char data_ota7290b_1920_68[] = { 0x42, 0x01 };
static unsigned char data_ota7290b_1920_69[] = { 0x43, 0x40 };
static unsigned char data_ota7290b_1920_70[] = { 0x44, 0x05 };
static unsigned char data_ota7290b_1920_71[] = { 0x45, 0xe8 };
static unsigned char data_ota7290b_1920_72[] = { 0x46, 0x16 };
static unsigned char data_ota7290b_1920_73[] = { 0x47, 0x00 };
static unsigned char data_ota7290b_1920_74[] = { 0x48, 0x00 };
static unsigned char data_ota7290b_1920_75[] = { 0x49, 0x88 };
static unsigned char data_ota7290b_1920_76[] = { 0x4a, 0x08 };
static unsigned char data_ota7290b_1920_77[] = { 0x4b, 0x05 };
static unsigned char data_ota7290b_1920_78[] = { 0x4c, 0x03 };
static unsigned char data_ota7290b_1920_79[] = { 0x4d, 0xd0 };
static unsigned char data_ota7290b_1920_80[] = { 0x4e, 0x13 };
static unsigned char data_ota7290b_1920_81[] = { 0x4f, 0xff };
static unsigned char data_ota7290b_1920_82[] = { 0x50, 0x0a };
static unsigned char data_ota7290b_1920_83[] = { 0x51, 0x53 };
static unsigned char data_ota7290b_1920_84[] = { 0x52, 0x26 };
static unsigned char data_ota7290b_1920_85[] = { 0x53, 0x22 };
static unsigned char data_ota7290b_1920_86[] = { 0x54, 0x09 };
static unsigned char data_ota7290b_1920_87[] = { 0x55, 0x22 };
static unsigned char data_ota7290b_1920_88[] = { 0x56, 0x00 };
static unsigned char data_ota7290b_1920_89[] = { 0x57, 0x1c };
static unsigned char data_ota7290b_1920_90[] = { 0x58, 0x03 };
static unsigned char data_ota7290b_1920_91[] = { 0x59, 0x3f };
static unsigned char data_ota7290b_1920_92[] = { 0x5a, 0x28 };
static unsigned char data_ota7290b_1920_93[] = { 0x5b, 0x01 };
static unsigned char data_ota7290b_1920_94[] = { 0x5c, 0xcc };
static unsigned char data_ota7290b_1920_95[] = { 0x5d, 0x20 };
static unsigned char data_ota7290b_1920_96[] = { 0x5e, 0xe8 };
static unsigned char data_ota7290b_1920_97[] = { 0x5f, 0x1d };
static unsigned char data_ota7290b_1920_98[] = { 0x60, 0xe1 };
static unsigned char data_ota7290b_1920_99[] = { 0x61, 0x73 };
static unsigned char data_ota7290b_1920_100[] = { 0x62, 0x8d };
static unsigned char data_ota7290b_1920_101[] = { 0x63, 0x2d };
static unsigned char data_ota7290b_1920_102[] = { 0x64, 0x25 };
static unsigned char data_ota7290b_1920_103[] = { 0x65, 0x82 };
static unsigned char data_ota7290b_1920_104[] = { 0x66, 0x09 };
static unsigned char data_ota7290b_1920_105[] = { 0x67, 0x21 };
static unsigned char data_ota7290b_1920_106[] = { 0x68, 0x84 };
static unsigned char data_ota7290b_1920_107[] = { 0x69, 0x10 };
static unsigned char data_ota7290b_1920_108[] = { 0x6a, 0x42 };
static unsigned char data_ota7290b_1920_109[] = { 0x6b, 0x08 };
static unsigned char data_ota7290b_1920_110[] = { 0x6c, 0x21 };
static unsigned char data_ota7290b_1920_111[] = { 0x6d, 0x84 };
static unsigned char data_ota7290b_1920_112[] = { 0x6e, 0x10 };
static unsigned char data_ota7290b_1920_113[] = { 0x6f, 0x42 };
static unsigned char data_ota7290b_1920_114[] = { 0x70, 0x08 };
static unsigned char data_ota7290b_1920_115[] = { 0x71, 0x21 };
static unsigned char data_ota7290b_1920_116[] = { 0x72, 0x84 };
static unsigned char data_ota7290b_1920_117[] = { 0x73, 0x10 };
static unsigned char data_ota7290b_1920_118[] = { 0x74, 0x42 };
static unsigned char data_ota7290b_1920_119[] = { 0x75, 0x08 };
static unsigned char data_ota7290b_1920_120[] = { 0x76, 0x00 };
static unsigned char data_ota7290b_1920_121[] = { 0x77, 0x00 };
static unsigned char data_ota7290b_1920_122[] = { 0x78, 0x0f };
static unsigned char data_ota7290b_1920_123[] = { 0x79, 0xe0 };
static unsigned char data_ota7290b_1920_124[] = { 0x7a, 0x01 };
static unsigned char data_ota7290b_1920_125[] = { 0x7b, 0xff };
static unsigned char data_ota7290b_1920_126[] = { 0x7c, 0xff };
static unsigned char data_ota7290b_1920_127[] = { 0x7d, 0x0c };
static unsigned char data_ota7290b_1920_128[] = { 0x7e, 0x41 };
static unsigned char data_ota7290b_1920_129[] = { 0x7f, 0xfe };
static unsigned char data_ota7290b_1920_130[] = { 0xb1, 0x02 };
static unsigned char data_ota7290b_1920_131[] = { 0x00, 0xff };
static unsigned char data_ota7290b_1920_132[] = { 0x01, 0x05 };
static unsigned char data_ota7290b_1920_133[] = { 0x02, 0xdc };
static unsigned char data_ota7290b_1920_134[] = { 0x03, 0x00 };
static unsigned char data_ota7290b_1920_135[] = { 0x04, 0x3e };
static unsigned char data_ota7290b_1920_136[] = { 0x05, 0x4e };
static unsigned char data_ota7290b_1920_137[] = { 0x06, 0x90 };
static unsigned char data_ota7290b_1920_138[] = { 0x07, 0x10 };
static unsigned char data_ota7290b_1920_139[] = { 0x08, 0xc0 };
static unsigned char data_ota7290b_1920_140[] = { 0x09, 0x01 };
static unsigned char data_ota7290b_1920_141[] = { 0x0a, 0x00 };
static unsigned char data_ota7290b_1920_142[] = { 0x0b, 0x14 };
static unsigned char data_ota7290b_1920_143[] = { 0x0c, 0xe6 };
static unsigned char data_ota7290b_1920_144[] = { 0x0d, 0x0d };
static unsigned char data_ota7290b_1920_145[] = { 0x0f, 0x08 };
static unsigned char data_ota7290b_1920_146[] = { 0x10, 0xf9 };
static unsigned char data_ota7290b_1920_147[] = { 0x11, 0xf5 };
static unsigned char data_ota7290b_1920_148[] = { 0x12, 0xa2 };
static unsigned char data_ota7290b_1920_149[] = { 0x13, 0x03 };
static unsigned char data_ota7290b_1920_150[] = { 0x14, 0x5e };
static unsigned char data_ota7290b_1920_151[] = { 0x15, 0xcf };
static unsigned char data_ota7290b_1920_152[] = { 0x16, 0x63 };
static unsigned char data_ota7290b_1920_153[] = { 0x17, 0x01 };
static unsigned char data_ota7290b_1920_154[] = { 0x18, 0xe9 };
static unsigned char data_ota7290b_1920_155[] = { 0x19, 0x5e };
static unsigned char data_ota7290b_1920_156[] = { 0x1a, 0x59 };
static unsigned char data_ota7290b_1920_157[] = { 0x1b, 0x0e };
static unsigned char data_ota7290b_1920_158[] = { 0x1c, 0xff };
static unsigned char data_ota7290b_1920_159[] = { 0x1d, 0xff };
static unsigned char data_ota7290b_1920_160[] = { 0x1e, 0xff };
static unsigned char data_ota7290b_1920_161[] = { 0x1f, 0xff };
static unsigned char data_ota7290b_1920_162[] = { 0x20, 0xff };
static unsigned char data_ota7290b_1920_163[] = { 0x21, 0xff };
static unsigned char data_ota7290b_1920_164[] = { 0x22, 0xff };
static unsigned char data_ota7290b_1920_165[] = { 0x23, 0xff };
static unsigned char data_ota7290b_1920_166[] = { 0x24, 0xff };
static unsigned char data_ota7290b_1920_167[] = { 0x25, 0xff };
static unsigned char data_ota7290b_1920_168[] = { 0x26, 0xff };
static unsigned char data_ota7290b_1920_169[] = { 0x27, 0x1f };
static unsigned char data_ota7290b_1920_170[] = { 0x28, 0xff };
static unsigned char data_ota7290b_1920_171[] = { 0x29, 0xff };
static unsigned char data_ota7290b_1920_172[] = { 0x2a, 0xff };
static unsigned char data_ota7290b_1920_173[] = { 0x2b, 0xff };
static unsigned char data_ota7290b_1920_174[] = { 0x2c, 0xff };
static unsigned char data_ota7290b_1920_175[] = { 0x2d, 0x07 };
static unsigned char data_ota7290b_1920_176[] = { 0x33, 0x00 };
static unsigned char data_ota7290b_1920_177[] = { 0x35, 0x7e };
static unsigned char data_ota7290b_1920_178[] = { 0x36, 0x00 };
static unsigned char data_ota7290b_1920_179[] = { 0x38, 0x7e };
static unsigned char data_ota7290b_1920_180[] = { 0x3a, 0x80 };
static unsigned char data_ota7290b_1920_181[] = { 0x3b, 0x01 };
static unsigned char data_ota7290b_1920_182[] = { 0x3c, 0xc0 };
static unsigned char data_ota7290b_1920_183[] = { 0x3d, 0x2d };
static unsigned char data_ota7290b_1920_184[] = { 0x3e, 0x00 };
static unsigned char data_ota7290b_1920_185[] = { 0x3f, 0xb8 };
static unsigned char data_ota7290b_1920_186[] = { 0x40, 0x05 };
static unsigned char data_ota7290b_1920_187[] = { 0x41, 0x00 };
static unsigned char data_ota7290b_1920_188[] = { 0x42, 0xb7 };
static unsigned char data_ota7290b_1920_189[] = { 0x43, 0x00 };
static unsigned char data_ota7290b_1920_190[] = { 0x44, 0xe0 };
static unsigned char data_ota7290b_1920_191[] = { 0x45, 0x06 };
static unsigned char data_ota7290b_1920_192[] = { 0x46, 0x00 };
static unsigned char data_ota7290b_1920_193[] = { 0x47, 0x00 };
static unsigned char data_ota7290b_1920_194[] = { 0x48, 0x9b };
static unsigned char data_ota7290b_1920_195[] = { 0x49, 0xd2 };
static unsigned char data_ota7290b_1920_196[] = { 0x4a, 0x71 };
static unsigned char data_ota7290b_1920_197[] = { 0x4b, 0xe3 };
static unsigned char data_ota7290b_1920_198[] = { 0x4c, 0x16 };
static unsigned char data_ota7290b_1920_199[] = { 0x4d, 0xc0 };
static unsigned char data_ota7290b_1920_200[] = { 0x4e, 0x0f };
static unsigned char data_ota7290b_1920_201[] = { 0x4f, 0x61 };
static unsigned char data_ota7290b_1920_202[] = { 0x50, 0x78 };
static unsigned char data_ota7290b_1920_203[] = { 0x51, 0x7a };
static unsigned char data_ota7290b_1920_204[] = { 0x52, 0x34 };
static unsigned char data_ota7290b_1920_205[] = { 0x53, 0x99 };
static unsigned char data_ota7290b_1920_206[] = { 0x54, 0xa2 };
static unsigned char data_ota7290b_1920_207[] = { 0x55, 0x02 };
static unsigned char data_ota7290b_1920_208[] = { 0x56, 0x24 };
static unsigned char data_ota7290b_1920_209[] = { 0x57, 0xf8 };
static unsigned char data_ota7290b_1920_210[] = { 0x58, 0xfc };
static unsigned char data_ota7290b_1920_211[] = { 0x59, 0xf4 };
static unsigned char data_ota7290b_1920_212[] = { 0x5a, 0xff };
static unsigned char data_ota7290b_1920_213[] = { 0x5b, 0xff };
static unsigned char data_ota7290b_1920_214[] = { 0x5c, 0xff };
static unsigned char data_ota7290b_1920_215[] = { 0x5d, 0xb2 };
static unsigned char data_ota7290b_1920_216[] = { 0x5e, 0xff };
static unsigned char data_ota7290b_1920_217[] = { 0x5f, 0xff };
static unsigned char data_ota7290b_1920_218[] = { 0x60, 0x8f };
static unsigned char data_ota7290b_1920_219[] = { 0x61, 0x62 };
static unsigned char data_ota7290b_1920_220[] = { 0x62, 0xb5 };
static unsigned char data_ota7290b_1920_221[] = { 0x63, 0xb2 };
static unsigned char data_ota7290b_1920_222[] = { 0x64, 0x5a };
static unsigned char data_ota7290b_1920_223[] = { 0x65, 0xad };
static unsigned char data_ota7290b_1920_224[] = { 0x66, 0x56 };
static unsigned char data_ota7290b_1920_225[] = { 0x67, 0x2b };
static unsigned char data_ota7290b_1920_226[] = { 0x68, 0x0c };
static unsigned char data_ota7290b_1920_227[] = { 0x69, 0x01 };
static unsigned char data_ota7290b_1920_228[] = { 0x6a, 0x01 };
static unsigned char data_ota7290b_1920_229[] = { 0x6b, 0xfc };
static unsigned char data_ota7290b_1920_230[] = { 0x6c, 0xfd };
static unsigned char data_ota7290b_1920_231[] = { 0x6d, 0xfd };
static unsigned char data_ota7290b_1920_232[] = { 0x6e, 0xfd };
static unsigned char data_ota7290b_1920_233[] = { 0x6f, 0xfd };
static unsigned char data_ota7290b_1920_234[] = { 0x70, 0xff };
static unsigned char data_ota7290b_1920_235[] = { 0x71, 0xff };
static unsigned char data_ota7290b_1920_236[] = { 0x72, 0x3f };
static unsigned char data_ota7290b_1920_237[] = { 0x73, 0x00 };
static unsigned char data_ota7290b_1920_238[] = { 0x74, 0x00 };
static unsigned char data_ota7290b_1920_239[] = { 0x75, 0x00 };
static unsigned char data_ota7290b_1920_240[] = { 0x76, 0x00 };
static unsigned char data_ota7290b_1920_241[] = { 0x77, 0x00 };
static unsigned char data_ota7290b_1920_242[] = { 0x78, 0x00 };
static unsigned char data_ota7290b_1920_243[] = { 0x79, 0x00 };
static unsigned char data_ota7290b_1920_244[] = { 0x7a, 0xdc };
static unsigned char data_ota7290b_1920_245[] = { 0x7b, 0xdc };
static unsigned char data_ota7290b_1920_246[] = { 0x7c, 0xdc };
static unsigned char data_ota7290b_1920_247[] = { 0x7d, 0xdc };
static unsigned char data_ota7290b_1920_248[] = { 0x7e, 0xdc };
static unsigned char data_ota7290b_1920_249[] = { 0x7f, 0x6e };
static unsigned char data_ota7290b_1920_250[] = { 0x0b, 0x04 };
static unsigned char data_ota7290b_1920_251[] = { 0xb1, 0x03 };
static unsigned char data_ota7290b_1920_252[] = { 0x2c, 0x2c };
static unsigned char data_ota7290b_1920_253[] = { 0xb1, 0x00 };
static unsigned char data_ota7290b_1920_254[] = { 0x89, 0x03 };
static unsigned char data_ota7290b_1920_255[] = { 0x29, 0x00 };

static const struct dsc_instr dsi_init_cmds[254] = {
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_2 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_3 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_4 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_5 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_6 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_7 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_8 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_9 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_10 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_11 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_12 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_13 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_14 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_15 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_16 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_17 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_18 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_19 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_20 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_21 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_22 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_23 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_24 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_25 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_26 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_27 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_28 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_29 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_30 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_31 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_32 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_33 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_34 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_35 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_36 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_37 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_38 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_39 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_40 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_41 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_42 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_43 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_44 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_45 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_46 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_47 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_48 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_49 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_50 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_51 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_52 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_53 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_54 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_55 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_56 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_57 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_58 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_59 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_60 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_61 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_62 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_63 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_64 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_65 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_66 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_67 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_68 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_69 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_70 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_71 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_72 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_73 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_74 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_75 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_76 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_77 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_78 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_79 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_80 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_81 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_82 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_83 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_84 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_85 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_86 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_87 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_88 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_89 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_90 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_91 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_92 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_93 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_94 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_95 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_96 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_97 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_98 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_99 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_100 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_101 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_102 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_103 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_104 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_105 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_106 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_107 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_108 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_109 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_110 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_111 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_112 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_113 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_114 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_115 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_116 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_117 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_118 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_119 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_120 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_121 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_122 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_123 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_124 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_125 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_126 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_127 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_128 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_129 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_130 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_131 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_132 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_133 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_134 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_135 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_136 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_137 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_138 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_139 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_140 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_141 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_142 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_143 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_144 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_145 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_146 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_147 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_148 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_149 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_150 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_151 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_152 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_153 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_154 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_155 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_156 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_157 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_158 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_159 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_160 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_161 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_162 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_163 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_164 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_165 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_166 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_167 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_168 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_169 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_170 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_171 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_172 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_173 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_174 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_175 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_176 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_177 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_178 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_179 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_180 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_181 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_182 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_183 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_184 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_185 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_186 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_187 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_188 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_189 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_190 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_191 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_192 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_193 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_194 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_195 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_196 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_197 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_198 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_199 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_200 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_201 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_202 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_203 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_204 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_205 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_206 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_207 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_208 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_209 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_210 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_211 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_212 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_213 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_214 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_215 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_216 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_217 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_218 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_219 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_220 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_221 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_222 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_223 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_224 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_225 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_226 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_227 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_228 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_229 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_230 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_231 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_232 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_233 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_234 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_235 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_236 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_237 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_238 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_239 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_240 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_241 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_242 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_243 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_244 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_245 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_246 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_247 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_248 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_249 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_250 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_251 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_252 },
	{.delay = 0, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_253 },
	{.delay = 50, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_254 },
	{.delay = 20, .data_type = 0x15, .size = 2, .data = data_ota7290b_1920_255 }
};

static const struct hs_settle_s hs_timing_cfg = { .prepare = 6, .zero = 32, .trail = 1 };

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* End of #ifdef __cplusplus */

#endif /* __HAL_SCREEN_H__  */
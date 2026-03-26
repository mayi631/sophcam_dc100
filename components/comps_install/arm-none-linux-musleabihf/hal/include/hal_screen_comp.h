#ifndef __HAL_SCREEN_COMP_H__
#define __HAL_SCREEN_COMP_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
//#include "hal_pwm.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** \addtogroup     HAL_SCREEN */
/** @{ */  /** <!-- [HAL_SCREEN] */
#define HAL_SCREEN_LANE_MAX_NUM 5
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

/** Screen Index, Currently can support two screen used in same time*/
typedef enum HAL_SCREEN_IDX_E {
	HAL_SCREEN_IDXS_0 = 0,
	HAL_SCREEN_IDXS_1,
	HAL_SCREEN_IDX_BUTT
} HAL_SCREEN_IDX_E;

typedef enum HAL_SCREEN_TYPE_E {
	HAL_COMP_SCREEN_INTF_TYPE_LCD = 0, /**<RGB interface type, such as lcd_6bit lcd_8bit, parallel communication protocal*/
	HAL_COMP_SCREEN_INTF_TYPE_MIPI, /**<MIPI interface type, serial communication protocal*/
	HAL_COMP_SCREEN_INTF_TYPE_I80,
	HAL_COMP_SCREEN_INTF_TYPE_BUIT
} HAL_SCREEN_TYPE_E;

/** brief general interface screen type enum*/
typedef enum HAL_SCREEN_LCD_INTF_TYPE_E {
	HAL_SCREEN_LCD_INTF_6BIT = 0, /**<6bit intf type*/
	HAL_SCREEN_LCD_INTF_8BIT,
	HAL_SCREEN_LCD_INTF_16BIT,
	HAL_SCREEN_LCD_INTF_24BIT,
	HAL_SCREEN_LCD_INTF_BUIT
} HAL_SCREEN_LCD_INTF_TYPE_E;

/* @brief mipi screen packet transport type*/
typedef enum HAL_SCREEN_MIPI_OUTPUT_TYPE_E {
	HAL_SCREEN_MIPI_OUTPUT_TYPE_CMD = 0x0,
	HAL_SCREEN_MIPI_OUTPUT_TYPE_VIDEO,
	HAL_SCREEN_MIPI_OUTPUT_TYPE_BUIT
} HAL_SCREEN_MIPI_OUTPUT_TYPE_E;

/* @brief mipi screen video mode type enum*/
typedef enum HAL_SCREEN_MIPI_VIDEO_MODE_E {
	HAL_SCREEN_MIPI_VIDEO_MODE_BURST = 0x0,/**<Burst Mode*/
	HAL_SCREEN_MIPI_VIDEO_MODE_PULSES,/**<Non-Burst Mode with Sync Pulses*/
	HAL_SCREEN_MIPI_VIDEO_MODE_EVENTS,/**<Non-Burst Mode with Sync Events*/
	HAL_SCREEN_MIPI_VIDEO_MODE_BUIT
} HAL_SCREEN_MIPI_VIDEO_MODE_E;

/* @brief mipi screen video format type enum*/
typedef enum HAL_SCREEN_MIPI_VIDEO_FORMAT_E {
	HAL_SCREEN_MIPI_VIDEO_RGB_16BIT = 0x0,
	HAL_SCREEN_MIPI_VIDEO_RGB_18BIT,
	HAL_SCREEN_MIPI_VIDEO_RGB_24BIT,
	HAL_SCREEN_MIPI_VIDEO_BUIT
} HAL_SCREEN_MIPI_VIDEO_FORMAT_E;

/** screen sync info*/
typedef struct HAL_SCREEN_SYNC_ATTR_S {
	uint16_t   u16Vact ;  /* vertical active area */
	uint16_t   u16Vbb;    /* vertical back blank porch */
	uint16_t   u16Vfb;    /* vertical front blank porch */
	uint16_t   u16Hact;   /* herizontal active area */
	uint16_t   u16Hbb;    /* herizontal back blank porch */
	uint16_t   u16Hfb;    /* herizontal front blank porch */

	uint16_t   u16Hpw;    /* horizontal pulse width */
	uint16_t   u16Vpw;    /* vertical pulse width */

	bool bIdv;/**< data Level polarity,0 mean high level valid,default 0,can not config*/
	bool bIhs;/**< horizon Level polarity,0 mean high level valid*/
	bool bIvs;/**< vertical Level polarity,0 mean high level valid*/
} HAL_SCREEN_SYNC_ATTR_S;

/** vo clk type*/
typedef enum HAL_SCREEN_CLK_TYPE_E {
	HAL_SCREEN_CLK_TYPE_PLL = 0x0,
	HAL_SCREEN_CLK_TYPE_LCDMCLK,
	HAL_SCREEN_CLK_TYPE_BUTT
} HAL_SCREEN_CLK_TYPE_E;

typedef struct HAL_SCREEN_CLK_PLL_S {
	uint32_t  u32Fbdiv;
	uint32_t  u32Frac;
	uint32_t  u32Refdiv;
	uint32_t  u32Postdiv1;
	uint32_t  u32Postdiv2;
} HAL_SCREEN_CLK_PLL_S;

typedef struct HAL_SCREEN_CLK_ATTR_S {
	bool bClkReverse; /**< vo clock reverse or not, if screen datasheet not mentioned, the value is true */
	uint32_t u32DevDiv;    /**< vo clock division factor, RGB6&8 is 3,RGB16&18 is 1, MIPI DSI is 1 */
	HAL_SCREEN_CLK_TYPE_E enClkType;  /**< vo clk type, pll or lcdmlck*/
	union {
		HAL_SCREEN_CLK_PLL_S stClkPll;
		uint32_t u32OutClk;    /**< for serial:(vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps*total_clk_per_pixel,
                                    for parallel:(vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps */
	};
} HAL_SCREEN_CLK_ATTR_S;

/** screen common attr*/
typedef struct HAL_SCREEN_COMMON_ATTR_S {
	HAL_SCREEN_SYNC_ATTR_S stSynAttr;/**<screen sync attr*/
	uint32_t u32Width;
	uint32_t u32Height;
	uint32_t u32Framerate;
} HAL_SCREEN_COMMON_ATTR_S;

/** screen mipi attr*/
typedef struct HAL_SCREEN_MIPI_ATTR_S {
	HAL_SCREEN_MIPI_OUTPUT_TYPE_E enType;
	HAL_SCREEN_MIPI_VIDEO_MODE_E enMode;
	HAL_SCREEN_MIPI_VIDEO_FORMAT_E enVideoFormat;
	int8_t as8LaneId[HAL_SCREEN_LANE_MAX_NUM];/**<lane use: value is index from zero start,lane not use:value is -1 */
	uint32_t u32PhyDataRate;  /**<mbps* (vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*total_bit_per_pixel/lane_num/100000 */
	uint32_t u32PixelClk;  /**<KHz* (vbp+vact+vfp+u16Vpw)*(hbp+hact+hfp+u16hpw)*fps/1000/*/

} HAL_SCREEN_MIPI_ATTR_S;

typedef enum HAL_LCD_MUX_E {
	HAL_LCD_MUX_B_0 = 0,
	HAL_LCD_MUX_B_1,
	HAL_LCD_MUX_B_2,
	HAL_LCD_MUX_B_3,
	HAL_LCD_MUX_B_4,
	HAL_LCD_MUX_B_5,
	HAL_LCD_MUX_B_6,
	HAL_LCD_MUX_B_7,
	HAL_LCD_MUX_G_0,
	HAL_LCD_MUX_G_1,
	HAL_LCD_MUX_G_2,
	HAL_LCD_MUX_G_3,
	HAL_LCD_MUX_G_4,
	HAL_LCD_MUX_G_5,
	HAL_LCD_MUX_G_6,
	HAL_LCD_MUX_G_7,
	HAL_LCD_MUX_R_0,
	HAL_LCD_MUX_R_1,
	HAL_LCD_MUX_R_2,
	HAL_LCD_MUX_R_3,
	HAL_LCD_MUX_R_4,
	HAL_LCD_MUX_R_5,
	HAL_LCD_MUX_R_6,
	HAL_LCD_MUX_R_7,
	HAL_LCD_MUX_VS,
	HAL_LCD_MUX_HS,
	HAL_LCD_MUX_HDE,
	HAL_LCD_MUX_MAX
} HAL_LCD_MUX_E;

typedef enum HAL_LCD_SEL_E {
	HAL_VIVO_D0 = 15,
	HAL_VIVO_D1 = 16,
	HAL_VIVO_D2 = 17,
	HAL_VIVO_D3 = 18,
	HAL_VIVO_D4 = 19,
	HAL_VIVO_D5 = 20,
	HAL_VIVO_D6 = 21,
	HAL_VIVO_D7 = 22,
	HAL_VIVO_D8 = 23,
	HAL_VIVO_D9 = 24,
	HAL_VIVO_D10 = 25,
	HAL_VIVO_CLK = 1,
	HAL_MIPI_TXM4 = 26,
	HAL_MIPI_TXP4 = 27,
	HAL_MIPI_TXM3 = 28,
	HAL_MIPI_TXP3 = 29,
	HAL_MIPI_TXM2 = 2,
	HAL_MIPI_TXP2 = 0,
	HAL_MIPI_TXM1 = 4,
	HAL_MIPI_TXP1 = 3,
	HAL_MIPI_TXM0 = 6,
	HAL_MIPI_TXP0 = 5,
	HAL_MIPI_RXN5 = 14,
	HAL_MIPI_RXP5 = 13,
	HAL_MIPI_RXN2 = 12,
	HAL_MIPI_RXP2 = 11,
	HAL_MIPI_RXN1 = 10,
	HAL_MIPI_RXP1 = 9,
	HAL_MIPI_RXN0 = 8,
	HAL_MIPI_RXP0 = 7,
	HAL_LCD_PAD_MAX = 30
} HAL_LCD_SEL_E;

typedef enum _VO_MUX_G {
	HAL_VO_MUX_BT_VS = 0,
	HAL_VO_MUX_BT_HS,
	HAL_VO_MUX_BT_HDE,
	HAL_VO_MUX_BT_DATA0,
	HAL_VO_MUX_BT_DATA1,
	HAL_VO_MUX_BT_DATA2,
	HAL_VO_MUX_BT_DATA3,
	HAL_VO_MUX_BT_DATA4,
	HAL_VO_MUX_BT_DATA5,
	HAL_VO_MUX_BT_DATA6,
	HAL_VO_MUX_BT_DATA7,
	HAL_VO_MUX_BT_DATA8,
	HAL_VO_MUX_BT_DATA9,
	HAL_VO_MUX_BT_DATA10,
	HAL_VO_MUX_BT_DATA11,
	HAL_VO_MUX_BT_DATA12,
	HAL_VO_MUX_BT_DATA13,
	HAL_VO_MUX_BT_DATA14,
	HAL_VO_MUX_BT_DATA15,
	HAL_VO_MUX_MCU_CTRL0 = 0,
	HAL_VO_MUX_MCU_CTRL1,
	HAL_VO_MUX_MCU_CTRL2,
	HAL_VO_MUX_MCU_CTRL3,
	HAL_VO_MUX_MCU_DATA0,
	HAL_VO_MUX_MCU_DATA1,
	HAL_VO_MUX_MCU_DATA2,
	HAL_VO_MUX_MCU_DATA3,
	HAL_VO_MUX_MCU_DATA4,
	HAL_VO_MUX_MCU_DATA5,
	HAL_VO_MUX_MCU_DATA6,
	HAL_VO_MUX_MCU_DATA7,
	HAL_VO_MUX_MCU_DATA8,
	HAL_VO_MUX_RGB_0 = 0,
	HAL_VO_MUX_RGB_1,
	HAL_VO_MUX_RGB_2,
	HAL_VO_MUX_RGB_3,
	HAL_VO_MUX_RGB_4,
	HAL_VO_MUX_RGB_5,
	HAL_VO_MUX_RGB_6,
	HAL_VO_MUX_RGB_7,
	HAL_VO_MUX_RGB_8,
	HAL_VO_MUX_RGB_9,
	HAL_VO_MUX_RGB_10,
	HAL_VO_MUX_RGB_11,
	HAL_VO_MUX_RGB_12,
	HAL_VO_MUX_RGB_13,
	HAL_VO_MUX_RGB_14,
	HAL_VO_MUX_RGB_15,
	HAL_VO_MUX_RGB_16,
	HAL_VO_MUX_RGB_17,
	HAL_VO_MUX_RGB_18,
	HAL_VO_MUX_RGB_19,
	HAL_VO_MUX_RGB_20,
	HAL_VO_MUX_RGB_21,
	HAL_VO_MUX_RGB_22,
	HAL_VO_MUX_RGB_23,
	HAL_VO_MUX_RGB_VS,
	HAL_VO_MUX_RGB_HS,
	HAL_VO_MUX_RGB_HDE,
	HAL_VO_MUX_TG_HS_TILE = 30,
	HAL_VO_MUX_TG_VS_TILE,
	HAL_VO_MUX_MAX,
} VO_MUX_G;

typedef enum HAL_LCD_FORMAT_E {
	HAL_LCD_FORMAT_RGB565 = 0,
	HAL_LCD_FORMAT_RGB666,
	HAL_LCD_FORMAT_RGB888,
	HAL_LCD_FORMAT_MAX
} HAL_LCD_FORMAT_E;

typedef struct HAL_LCD_D_REMAP_E {
	HAL_LCD_MUX_E mux;
	HAL_LCD_SEL_E sel;
} HAL_LCD_REMAP_S;

/* LCD's config*/
typedef struct HAL_SCREEN_LCD_CFG_S {
	uint32_t pixelclock;
	HAL_LCD_FORMAT_E fmt;
	uint16_t cycle_time;
	HAL_LCD_REMAP_S remap[HAL_LCD_PAD_MAX];
	uint8_t gpio_num;
} HAL_SCREEN_LCD_CFG_S;

/** screen lcd attr*/
typedef struct HAL_SCREEN_LCD_ATTR_S {
	HAL_SCREEN_LCD_INTF_TYPE_E enType;
	HAL_SCREEN_LCD_CFG_S stLcdCfg;
} HAL_SCREEN_LCD_ATTR_S;

typedef struct HAL_I80_D_REMAP_E {
	VO_MUX_G mux;
	HAL_LCD_SEL_E sel;
} HAL_I80_REMAP_S;

/* LCD's config*/
typedef struct HAL_SCREEN_I80_CFG_S {
	HAL_LCD_FORMAT_E fmt;
	uint16_t cycle_time;
	HAL_I80_REMAP_S remap[HAL_LCD_PAD_MAX];
	uint8_t pin_num;
} HAL_SCREEN_I80_CFG_S;

/** screen lcd attr*/
typedef struct HAL_SCREEN_I80_ATTR_S {
	HAL_SCREEN_LCD_INTF_TYPE_E enType;
	HAL_SCREEN_I80_CFG_S stI80Cfg;
} HAL_SCREEN_I80_ATTR_S;

/** screen attr*/
typedef struct HAL_SCREEN_ATTR_S {
	HAL_SCREEN_TYPE_E enType;
	union tagHAL_SCREEN_ATTR_U {
		HAL_SCREEN_LCD_ATTR_S stLcdAttr;
		HAL_SCREEN_MIPI_ATTR_S stMipiAttr;
		HAL_SCREEN_I80_ATTR_S stI80Attr;
	} unScreenAttr;
	HAL_SCREEN_COMMON_ATTR_S stAttr;
} HAL_SCREEN_ATTR_S;

typedef struct HAL_SCREEN_PWM_S {
	uint8_t group;       //组号
	uint8_t channel;      //通道号
	uint32_t period;      //周期
	uint32_t duty_cycle;  //占空比
} HAL_SCREEN_PWM_S;

/* @brief screen status enum*/
typedef enum HAL_SCREEN_STATE_E {
	HAL_SCREEN_STATE_OFF = 0,/**<screen off*/
	HAL_SCREEN_STATE_ON,     /**<screen on*/
	HAL_SCREEN_STATE_BUIT
} HAL_SCREEN_STATE_E;

typedef struct HAL_SCREEN_OBJ_S {
	int32_t (*pfnInit)(void);
	int32_t (*pfnGetAttr)(HAL_SCREEN_ATTR_S *pstAttr);
	int32_t (*pfnGetDisplayState)(HAL_SCREEN_STATE_E *penDisplayState);
	int32_t (*pfnSetDisplayState)(HAL_SCREEN_STATE_E enDisplayState);
	int32_t (*pfnGetBackLightState)(HAL_SCREEN_STATE_E *penBackLightState);
	int32_t (*pfnSetBackLightState)(HAL_SCREEN_STATE_E enBackLightState);
	HAL_SCREEN_PWM_S (*pfnGetLuma)(void);
	int32_t (*pfnSetLuma)(HAL_SCREEN_PWM_S pwmAttr);
	int32_t (*pfnGetSaturature)(uint32_t *pu32Satuature);
	int32_t (*pfnSetSaturature)(uint32_t u32Satuature);
	int32_t (*pfnGetContrast)(uint32_t *pu32Contrast);
	int32_t (*pfnSetContrast)(uint32_t u32Contrast);
	int32_t (*pfnDeinit)(void);
} HAL_SCREEN_OBJ_S;

int32_t HAL_SCREEN_COMM_Register(HAL_SCREEN_IDX_E enScreenIndex, const HAL_SCREEN_OBJ_S *pstScreenObj);
int32_t HAL_SCREEN_COMM_Init(HAL_SCREEN_IDX_E enScreenIndex);
int32_t HAL_SCREEN_COMM_GetAttr(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_ATTR_S *pstAttr);
int32_t HAL_SCREEN_COMM_GetDisplayState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E *penDisplayState);
int32_t HAL_SCREEN_COMM_SetDisplayState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E enDisplayState);
int32_t HAL_SCREEN_COMM_GetBackLightState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E *penBackLightState);
int32_t HAL_SCREEN_COMM_SetBackLightState(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_STATE_E enBackLightState);
HAL_SCREEN_PWM_S HAL_SCREEN_COMM_GetLuma(HAL_SCREEN_IDX_E enScreenIndex);
int32_t HAL_SCREEN_COMM_SetLuma(HAL_SCREEN_IDX_E enScreenIndex, HAL_SCREEN_PWM_S pwmAttr);
int32_t HAL_SCREEN_COMM_GetSaturature(HAL_SCREEN_IDX_E enScreenIndex, uint32_t *pu32Satuature);
int32_t HAL_SCREEN_COMM_SetSaturature(HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Saturature);
int32_t HAL_SCREEN_COMM_GetContrast(HAL_SCREEN_IDX_E enScreenIndex, uint32_t *pu32Contrast);
int32_t HAL_SCREEN_COMM_SetContrast(HAL_SCREEN_IDX_E enScreenIndex, uint32_t u32Contrast);
int32_t HAL_COMM_SCREEN_Deinit(HAL_SCREEN_IDX_E enScreenIndex);

 extern HAL_SCREEN_OBJ_S stHALSCREENObj;

/** @}*/  /** <!-- ==== HAL_SCREEN End ====*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __HAL_SCREEN_COMP_H__*/


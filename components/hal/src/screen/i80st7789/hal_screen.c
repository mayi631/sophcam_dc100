#include "hal_screen_comp.h"
#include "hal_gpio.h"
#include "screen_mipidsi.h"
#if CONFIG_PWM_ON
#include "hal_pwm.h"
#endif
#include "hal_screen.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/** \addtogroup 	SCREEN */
/** @{ */  /** <!-- [SCREEN] */

static HAL_SCREEN_STATE_E s_halScreenState = HAL_SCREEN_STATE_ON;

#if CONFIG_PWM_ON
// HAL_PWM_S screen_bl = {
// 	.group  = 2,   //  pwmchip0/4/8/12
// 	.channel = 0,  //  pwmchip0:pwm0~3 pwmchip1:pwm4~7  20us
// 	.period = 10000,    //unit:ns
// 	.duty_cycle = 10000 //unit:ns
// };
HAL_PWM_S screen_bl = { 0 };
#endif

/**MIPI Screen do reset in HAL, RGB do reset in DRV*/
static void HAL_SCREEN_Reset(void)
{
	HAL_GPIO_Export(REST_LIGHT);
	HAL_GPIO_Direction_Output(REST_LIGHT);
	HAL_GPIO_Export(POWER_LIGHT);
	HAL_GPIO_Direction_Output(POWER_LIGHT);
	#if CONFIG_PWM_ON

	#else
	HAL_GPIO_Export(BACK_LIGHT);
	HAL_GPIO_Direction_Output(BACK_LIGHT);
	#endif
	HAL_GPIO_Set_Value(REST_LIGHT, HAL_GPIO_VALUE_L);
    usleep(10 * 1000);
    //ctrl power
    HAL_GPIO_Set_Value(POWER_LIGHT, HAL_GPIO_VALUE_H);
    usleep(10 * 1000);
    //ctrl reset
    HAL_GPIO_Set_Value(REST_LIGHT, HAL_GPIO_VALUE_H);
    usleep(10 * 1000);
    HAL_GPIO_Set_Value(REST_LIGHT, HAL_GPIO_VALUE_L);
    usleep(10 * 1000);
    HAL_GPIO_Set_Value(REST_LIGHT, HAL_GPIO_VALUE_H);
    usleep(RESET_DELAY);
	HAL_GPIO_Set_Value(BACK_LIGHT, HAL_GPIO_VALUE_H);
    usleep(10 * 1000);
}

/**Use GPIO high_low level must be set GPIO pin reuse*/
/**Use PWM. must set Pin reuse and PWM config*/
static void HAL_SCREEN_LumaInit(void)
{

}

static int32_t HAL_SCREEN_Init(void)
{

	HAL_SCREEN_Reset();
	HAL_SCREEN_LumaInit();

	HAL_GPIO_Export(I80RDPIN);
	HAL_GPIO_Direction_Output(I80RDPIN);
	HAL_GPIO_Set_Value(I80RDPIN, HAL_GPIO_VALUE_H);	//i80 RD
	// printf("set i80 RD high------------\n");
	I80DSI_Init(0, &st7789vCfg);

	s_halScreenState = HAL_SCREEN_STATE_ON;
	return 0;
}

static int32_t HAL_SCREEN_GetAttr(HAL_SCREEN_ATTR_S* pstAttr)
{
	pstAttr->enType = SCREEN_TYPE;

    /* these magic value are given from screen attribute */
    pstAttr->stAttr.u32Width = HORIZONTAL_ACTIVE;
    pstAttr->stAttr.u32Height = VERTICAL_ACTIVE;

	pstAttr->stAttr.u32Framerate = 60;

    pstAttr->stAttr.stSynAttr.u16Vact = VERTICAL_ACTIVE;
    pstAttr->stAttr.stSynAttr.u16Vbb = VERTICAL_BACK_PROCH;
    pstAttr->stAttr.stSynAttr.u16Vfb = VERTICAL_FRONT_PROCH;
    pstAttr->stAttr.stSynAttr.u16Hact = HORIZONTAL_ACTIVE;
    pstAttr->stAttr.stSynAttr.u16Hbb = HORIZONTAL_BACK_PROCH;
    pstAttr->stAttr.stSynAttr.u16Hfb = HORIZONTAL_FRONT_PROCH;
    pstAttr->stAttr.stSynAttr.u16Hpw = HORIZONTAL_SYNC_ACTIVE;
    pstAttr->stAttr.stSynAttr.u16Vpw = VERTICAL_SYNC_ACTIVE;
    pstAttr->stAttr.stSynAttr.bIdv = 0;
    pstAttr->stAttr.stSynAttr.bIhs = 0;
    pstAttr->stAttr.stSynAttr.bIvs = 1;

	return 0;
}

static int32_t HAL_SCREEN_GetDisplayState(HAL_SCREEN_STATE_E* penDisplayState)
{
	*penDisplayState = s_halScreenState;
	return 0;
}

static int32_t HAL_SCREEN_SetDisplayState(HAL_SCREEN_STATE_E enDisplayState)
{
	s_halScreenState = enDisplayState;
	return 0;
}

HAL_SCREEN_PWM_S HAL_SCREEN_GetLuma(void)
{
	HAL_SCREEN_PWM_S hal_screen_pwm = {0};
	#if CONFIG_PWM_ON
	hal_screen_pwm.group = screen_bl.group;
	hal_screen_pwm.channel = screen_bl.channel;
	hal_screen_pwm.period = screen_bl.period;
	hal_screen_pwm.duty_cycle = screen_bl.duty_cycle;
	#endif
	return hal_screen_pwm;
}

static int32_t HAL_SCREEN_SetLuma(HAL_SCREEN_PWM_S pwmAttr)
{
	#if CONFIG_PWM_ON
	screen_bl.group = pwmAttr.group;
	screen_bl.channel = pwmAttr.channel;
	screen_bl.period = pwmAttr.period;
	screen_bl.duty_cycle = pwmAttr.duty_cycle;
	#endif
	return 0;
}

static int32_t HAL_SCREEN_GetSatuature(uint32_t* pu32Satuature)
{
	printf("HAL_SCREEN_GetSatuature = %d\n", *pu32Satuature);
	return 0;
}

static int32_t HAL_SCREEN_SetSatuature(uint32_t u32Satuature)
{
	printf("HAL_SCREEN_SetSatuature = %d\n", u32Satuature);
	return 0;
}

static int32_t HAL_SCREEN_GetContrast(uint32_t* pu32Contrast)
{
	printf("HAL_SCREEN_GetContrast = %d\n", *pu32Contrast);
	return 0;
}

static int32_t HAL_SCREEN_SetContrast(uint32_t u32Contrast)
{
	printf("HAL_SCREEN_SetContrast = %d\n", u32Contrast);
	return 0;
}

static int32_t HAL_SCREEN_SetBackLightState(HAL_SCREEN_STATE_E enBackLightState)
{
	printf("HAL_SCREEN_SetBackLightState = %d\n", enBackLightState);
	//ctrl backlight pwm
	int32_t ret = 0;
	if (enBackLightState == HAL_SCREEN_STATE_ON) {
		#if CONFIG_PWM_ON
		ret = HAL_PWM_Init(screen_bl);
		if (ret != 0) {
			printf("HAL_SCREEN_SetBackLightState failed! \n");
		}
		#else
		(void)ret;

		HAL_GPIO_Set_Value(BACK_LIGHT, HAL_GPIO_VALUE_H);
		s_halScreenState = HAL_SCREEN_STATE_ON;
		#endif
	} else {
		#if CONFIG_PWM_ON
		ret = HAL_PWM_Deinit(screen_bl);
		if (ret != 0) {
			printf("HAL_SCREEN_SetBackLightState failed! \n");
		}
		#else
		(void)ret;
		HAL_GPIO_Set_Value(BACK_LIGHT, HAL_GPIO_VALUE_L);
		s_halScreenState = HAL_SCREEN_STATE_OFF;
		#endif
	}

	//ctrl backlight gpio
	//HAL_GPIO_Set_Value(screen_gpio[GPIO_TYPE_BACKLIGHT].gpio, HAL_GPIO_VALUE_H);

	return 0;
}

static int32_t HAL_SCREEN_GetBackLightState(HAL_SCREEN_STATE_E* penBackLightState)
{
	*penBackLightState = s_halScreenState;
	printf("HAL_SCREEN_GetBackLightState = %d\n", *penBackLightState);
	return 0;
}

static int32_t HAL_SCREEN_Deinit(void)
{
    // MIPIDSI_Deinit();
    return 0;
}

HAL_SCREEN_OBJ_S stHALSCREENObj =
{
	.pfnInit = HAL_SCREEN_Init,
	.pfnGetAttr = HAL_SCREEN_GetAttr,
	.pfnSetDisplayState = HAL_SCREEN_SetDisplayState,
	.pfnGetDisplayState = HAL_SCREEN_GetDisplayState,
	.pfnSetBackLightState = HAL_SCREEN_SetBackLightState,
	.pfnGetBackLightState = HAL_SCREEN_GetBackLightState,
	.pfnSetLuma = HAL_SCREEN_SetLuma,
	.pfnGetLuma = HAL_SCREEN_GetLuma,
	.pfnSetSaturature = HAL_SCREEN_SetSatuature,
	.pfnGetSaturature = HAL_SCREEN_GetSatuature,
	.pfnSetContrast = HAL_SCREEN_SetContrast,
	.pfnGetContrast = HAL_SCREEN_GetContrast,
	.pfnDeinit = HAL_SCREEN_Deinit,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


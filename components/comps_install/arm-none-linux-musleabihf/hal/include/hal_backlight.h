#ifndef __HAL_BACKLIGHT_H__
#define __HAL_BACKLIGHT_H__

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define BACKLIGHT_PERCENT_MIN 10
#define BACKLIGHT_PERCENT_MAX 100

/* 背光状态枚举 */
typedef enum {
    BACKLIGHT_STATE_OFF = 0,  /* 背光关闭 */
    BACKLIGHT_STATE_ON,       /* 背光开启 */
    BACKLIGHT_STATE_BUTT
} BACKLIGHT_STATE_E;

int32_t HAL_BACKLIGHT_SetBackLightState(BACKLIGHT_STATE_E enBackLightState);
BACKLIGHT_STATE_E HAL_BACKLIGHT_GetBackLightState(void);
int32_t HAL_BACKLIGHT_SetLuma(int32_t percent_val);
int32_t HAL_BACKLIGHT_GetLuma(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __HAL_BACKLIGHT_H__ */
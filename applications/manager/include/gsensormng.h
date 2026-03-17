#ifndef __GSENSORMNG_H__
#define __GSENSORMNG_H__

#include "appcomm.h"
#include "hal_gsensor.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define pi 3.141592
#define ROLLOVER_MAX_DEGREE 75
#define ROLLOVER_MIN_DEGREE 25

#define GSENSORMNG_EINVAL            APP_APPCOMM_ERR_ID(APP_MOD_GSENSORMNG,APP_EINVAL)   /**<parm error*/
#define GSENSORMNG_EINTER            APP_APPCOMM_ERR_ID(APP_MOD_GSENSORMNG,APP_EINTER)        /**<intern error*/
#define GSENSORMNG_ENOINIT           APP_APPCOMM_ERR_ID(APP_MOD_GSENSORMNG,APP_ENOINIT)  /**< no initialize*/
#define GSENSORMNG_EINITIALIZED      APP_APPCOMM_ERR_ID(APP_MOD_GSENSORMNG,APP_EINITIALIZED) /**< already initialized */
#define GSENSORMNG_EREGISTEREVENT    APP_APPCOMM_ERR_ID(APP_MOD_GSENSORMNG,APP_ERRNO_CUSTOM_BOTTOM)            /**<thread creat or join error*/
#define GSENSORMNG_ETHREAD           APP_APPCOMM_ERR_ID(APP_MOD_GSENSORMNG,APP_ERRNO_CUSTOM_BOTTOM+1)            /**<thread creat or join error*/

typedef enum EVENT_GSENSORMNG_E
{
    EVENT_GSENSORMNG_COLLISION = APPCOMM_EVENT_ID(APP_MOD_GSENSORMNG, 0), /**<collision occur event*/
    EVENT_GSENSORMNG_BUIT
} EVENT_GSENSORMNG_E;

typedef struct GSENSORMNG_VALUE_S
{
    int32_t s32XDirValue; /**<x direction value,unit acceleration of gravity g*/
    int32_t s32YDirValue; /**<y direction value,unit acceleration of gravity g*/
    int32_t s32ZDirValue; /**<z direction value,unit acceleration of gravity g*/
} GSENSORMNG_VALUE_S;

/* @brief gesensor chip work attr*/
typedef struct GSENSORMNG_ATTR_S
{
    uint32_t u32SampleRate; /**<sample rate,0 mean Adopt default,not config,unit kps*/
} GSENSORMNG_ATTR_S;

typedef struct GSENSORMNG_CFG_S
{
    int32_t gsensor_level;
    int32_t gsensor_enable;
    HAL_GSENSOR_SENSITITY_E enSensitity;
    GSENSORMNG_ATTR_S stAttr;
} GSENSORMNG_CFG_S;

int32_t GSENSORMNG_RegisterEvent(void);
int32_t GSENSORMNG_Init(const GSENSORMNG_CFG_S* pstCfg);
int32_t GSENSORMNG_SetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity);
void GSENSORMNG_MenuSetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity);
int32_t GSENSORMNG_GetAttr(GSENSORMNG_ATTR_S* pstAttr);
int32_t GSENSORMNG_SetAttr(const GSENSORMNG_ATTR_S* pstAttr);
int32_t GSENSORMNG_OpenInterrupt(int32_t IntNum);
int32_t GSENSORMNG_DeInit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of __GSENSORMNG_H__ */
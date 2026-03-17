#ifndef __HAL_TOUCHPAD_H__
#define __HAL_TOUCHPAD_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/** touchpad touch info*/
typedef struct HAL_TOUCHPAD_INPUTINFO_S
{
    int s32ID;/**<input id info, one finger or two fingers*/
    int s32X;/**<x coordinate absolute*/
    int s32Y;/**<y coordinate absolute*/
    uint32_t u32Pressure;/**<is press on screen: 0, 1*/
    uint32_t u32TimeStamp;/**<time stamp*/
} HAL_TOUCHPAD_INPUTINFO_S;

typedef struct HAL_TOUCHPAD_OBJ_S
{
    int (*pfnInit)(void);
    int (*pfnStart)(uint32_t* ps32Fd);
    int (*pfnStop)(void);
    int (*pfnReadInputEvent)(HAL_TOUCHPAD_INPUTINFO_S* pstInputData);
    int (*pfnSuspend)(void);
    int (*pfnResume)(void);
    int (*pfnDeinit)(void);
} HAL_TOUCHPAD_OBJ_S;


int HAL_TOUCHPAD_Init(void);
int HAL_TOUCHPAD_Suspend(void);
int HAL_TOUCHPAD_Resume(void);
int HAL_TOUCHPAD_Start(int* ps32Fd);
int HAL_TOUCHPAD_Stop(void);
int HAL_TOUCHPAD_ReadInputEvent(HAL_TOUCHPAD_INPUTINFO_S* pstInputData);
int HAL_TOUCHPAD_Deinit(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif/* End of #ifdef __cplusplus */

#endif /* End of __HAL_TOUCHPAD_H__*/

/**
* @file    ledmng.c
* @brief   product keymng function
*/

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "ledmng.h"
#include "cvi_log.h"

#define true (1)
#define false (0)

static pthread_t s_LEDVALUETHREAD;
int32_t g_valueflage = true;
int32_t g_red = true;
int32_t g_twinkle = false;

/**
 * Function: control GPIO setting status
 * Parameter description:
 *     0: the red light goes out. ps:stop recording
 *     1: the red light is on and flashing. ps:Start recording
 *     g_red: red sign
 *     g_g_twinkle : red twinkle sign
 **/
void LEDMNG_Control(int32_t control)
{
    if (0 == control) {
        g_red = false;
        g_twinkle = false;
    } else {
        g_red = true;
        g_twinkle = true;
    }
}

static void* LED_SET_Value()
{
    int32_t ret = 0;
    HAL_GPIO_VALUE_E value = HAL_GPIO_VALUE_L;
    prctl(PR_SET_NAME, "LED_SET", 0, 0, 0); /**< Set Task Name */
    while (true == g_valueflage) {
        ret = HAL_LED_SetValue(HAL_GPIOA_29, HAL_GPIO_VALUE_L);
        if (0 != ret) {
            CVI_LOGE("set blue gpio Low level error");
        }
        if (false == g_red) {
            ret = HAL_LED_SetValue(HAL_GPIOA_28, HAL_GPIO_VALUE_H);
            if (0 != ret) {
                CVI_LOGE("set red gpio high level error");
            }
        } else {
            ret = HAL_LED_SetValue(HAL_GPIOA_28, value);
            if (0 != ret) {
                CVI_LOGE("set red gpio error, value = %d", value);
            }
            if (g_twinkle) {
                if (HAL_GPIO_VALUE_H == value) {
                    value = HAL_GPIO_VALUE_L;
                } else {
                    value = HAL_GPIO_VALUE_H;
                }
            }
        }
        sleep(1);
    }
    if (false == g_valueflage) {
        HAL_LED_SetValue(HAL_GPIOA_28, HAL_GPIO_VALUE_H);
        HAL_LED_SetValue(HAL_GPIOA_29, HAL_GPIO_VALUE_H);
    }
    return NULL;
}

int32_t LED_SET_ValueThread()
{
    int32_t ret = 0;
    ret = pthread_create(&s_LEDVALUETHREAD, NULL, LED_SET_Value, NULL);
    if (ret != 0) {
        CVI_LOGE("Create KeyCheck Thread Fail!\n");
        LEDMNG_DeInit();
        return -1;
    }
    return 0;
}

int32_t LEDMNG_Init()
{
    int32_t ret = 0;
    ret = HAL_LED_Init();
    if (0 != ret) {
        CVI_LOGE("led gpio init error");
        return -1;
    }
    LED_SET_ValueThread();

    return 0;
}

int32_t LEDMNG_DeInit(void)
{
    int32_t ret = 0;
    g_valueflage = false;
    g_red = true;
    g_twinkle = false;

    ret = pthread_join(s_LEDVALUETHREAD, NULL);
    if (ret != 0) {
        CVI_LOGE("Join led Thread Fail!\n");
        return -1;
    }

    ret = HAL_LED_Deinit();
    if (0 != ret) {
        CVI_LOGE("led deinit Fail!\n");
        return -1;
    }
    return 0;
}
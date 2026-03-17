#ifndef __USBCTRL_H__
#define __USBCTRL_H__
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


int32_t  USBCTRL_Init(void);
int32_t  USBCTRL_Deinit(void);
int32_t  USBCTRL_RegisterEvent(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


#endif
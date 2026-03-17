#ifndef __UI_POWERCONTROL_H__
#define __UI_POWERCONTROL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int32_t  UI_POWERCTRL_DormantScreen(void* pvPrivData);
int32_t  UI_POWERCTRL_WakeupScreen(void* pvPrivData);
int32_t  UI_POWERCTRL_Init(void);
int32_t  UI_POWERCTRL_Deinit(void);
int32_t  UI_POWERCTRL_PreProcessEvent(EVENT_S* pstEvent, bool* pbEventContinueHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

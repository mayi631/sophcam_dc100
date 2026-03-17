
#ifndef __PAGE_SETTINGWHITEBALAN_H_
#define __PAGE_SETTINGWHITEBALAN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"


uint8_t getwhiBlance_Index(void);
void menuSetting_WhiteBalance(lv_ui_t *ui);
int32_t Set_WhiteBalanceMode(int32_t index);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

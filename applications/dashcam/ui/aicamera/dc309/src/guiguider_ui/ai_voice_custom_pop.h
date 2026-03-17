
#ifndef __AI_VOICE_CUSTOM_POP_H_
#define __AI_VOICE_CUSTOM_POP_H_
#ifdef __cplusplus
extern "C" {
#endif
//语音自定义弹窗
#include "gui_guider.h"

//同时调用 -- 创建/销毁语音文本弹框
void create_voice_input_popup(void);
void destroy_voice_input_popup(void);
void voice_display_button(void);
void voice_hide_button(void);
//设置获取到的语音文本
void voice_text_set(char *text);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */

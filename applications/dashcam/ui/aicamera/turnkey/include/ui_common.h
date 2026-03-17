#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__
#include <stdbool.h>
#include "sysutils_mq.h"
#include "osal.h"
#include "sysutils_eventhub.h"
#include "cvi_log.h"
#include "storagemng.h"
#include "recordmng.h"
#include "mode.h"
// #include "filemng_dtcf.h"
#include "param.h"
#include "media_init.h"
#include "media_dump.h"
// #include "appfilemng_comm.h"
#include "keymng.h"
// #include "volmng.h"
#include "cvi_sys.h"
#include "usb.h"
#include "appuvc.h"
#include "gaugemng.h"
#ifdef CONFIG_PWM_ON
#include "hal_pwm.h"
#endif
#ifdef SERVICES_PHOTO_ON
#include "photomng.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define UI_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
typedef int32_t (*UI_MSGRESULTPROC_FN_PTR)(EVENT_S *pstEvent);

#define UI_VOICE_MAX_NUM (4)
#define UI_VOICE_START_UP_IDX (0)
#define UI_VOICE_START_UP_SRC "/mnt/data/bin/voice/ANI.wav"
#define UI_VOICE_TOUCH_BTN_IDX (1)
#define UI_VOICE_TOUCH_BTN_SRC "/mnt/data/bin/voice/touch.wav"
#define UI_VOICE_CLOSE_IDX (2)
#define UI_VOICE_CLOSE_SRC "/mnt/data/bin/voice/ANI.wav"
#define UI_VOICE_PHOTO_IDX (3)
#define UI_VOICE_PHOTO_SRC "/mnt/data/bin/voice/photo.wav"

#define EVT_USER_START 0x2000

typedef enum EVENT_UI_E {
	EVENT_UI_TOUCH = APPCOMM_EVENT_ID(APP_MOD_UI, 0),
	EVENT_UI_BUTT
} EVENT_UI_E;

typedef enum UIEVENT_COMMON_E
{
    EVT_LOW_BATTERY = EVT_USER_START + 1,
    EVT_NO_SDCARD,
    EVT_SDCARD_NEED_FORMAT,
    EVT_SDCARD_SLOW,
    EVT_SDCARD_CHECKING,
    EVT_SDCARD_ERROR,
    EVT_SDCARD_READ_ONLY,
    EVT_SDCARD_MOUNT_FAILED,
    EVT_SDCARD_SPACE_FULL,
    EVT_FORMAT_PROCESS,
    EVT_FORMAT_FAILED,
    EVT_FORMAT_SUCCESS,
    EVT_OPEN_HOMEPAGE,
    EVT_CLOSE_HOMEPAGE,
    EVT_OPEN_DIRPAGE,
    EVT_CLOSE_DIRPAGE,
    EVT_OPEN_MSGPAGE,
    EVT_CLOSE_MSGPAGE,
    EVT_FILE_ABNORMAL = EVT_USER_START + 100,
    EVT_NOVIDEO_FILE,
    EVT_NOPHOTO_FILE,
    EVT_DELETE_FILE_CONFIRM,
    EVT_NOTSUPPORT_H265,
} UIEVENT_COMMON_E;

typedef struct tagCVI_UI_MESSAGE_CONTEXT {
	MESSAGE_S stMsg;						  /**< the message that has been sent*/
	bool bMsgProcessed;						  /**< the message sent has been processed or not*/
	UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc; /**< used to process the response*/
	pthread_mutex_t MsgMutex;				  /**< the mutex protect sent msg and bool flag*/
} UI_MESSAGE_CONTEXT;

int32_t UIAPP_Start(void);
int32_t UIAPP_Stop(void);
int32_t ui_common_SubscribeEvents(void);
int32_t UI_POWERCTRL_PreProcessEvent(EVENT_S *pstEvent, bool *pbEventContinueHandle);
int32_t UICOMM_SendAsyncMsg(MESSAGE_S *pstMsg, UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);
void ui_common_wait_piv_end(void);
bool ui_common_cardstatus(void);

extern uint32_t ui_event_type;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

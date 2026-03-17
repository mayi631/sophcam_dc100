#ifndef __UI_COMMON_H__
#define __UI_COMMON_H__
#include <stdbool.h>
#include "awtk.h"
#include "main_loop/main_loop_simple.h"
#include "sysutils_mq.h"
#include "osal.h"
#include "sysutils_eventhub.h"
#include "cvi_log.h"
#include "storagemng.h"
#include "recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "photomng.h"
#endif
#include "mode.h"
#include "filemng_dtcf.h"
#include "param.h"
#include "media_init.h"
#include "media_dump.h"
#include "appfilemng_comm.h"
#include "keymng.h"
#include "volmng.h"
#include "liveviewmng.h"
#include "cvi_sys.h"
#include "usb.h"
#include "appuvc.h"
#include "gaugemng.h"
#include "gsensormng.h"
#include "ui_voice.h"
#include "ui_powercontrol.h"
#include "hal_screen_comp.h"
#include "system.h"
#include "net.h"
#include "netctrl.h"
#ifdef CONFIG_PWM_ON
#include "hal_pwm.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define UI_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
typedef int32_t (*UI_MSGRESULTPROC_FN_PTR)(EVENT_S *pstEvent);

typedef struct tagCVI_UI_MESSAGE_CONTEXT {
	MESSAGE_S stMsg;						  /**< the message that has been sent*/
	bool bMsgProcessed;						  /**< the message sent has been processed or not*/
	UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc; /**< used to process the response*/
	pthread_mutex_t MsgMutex;				  /**< the mutex protect sent msg and bool flag*/
} UI_MESSAGE_CONTEXT;

#define UI_KEY_POWER TK_KEY_F1
#define UI_KEY_MENU TK_KEY_F2
#define UI_KEY_UP TK_KEY_F3
#define UI_KEY_DOWN TK_KEY_F4

#define MSG_EVENT_ID_INVALID 100
#define DATETIME_BUF_LEN 32

typedef enum UIEVENT_COMMON_E {
	EVT_UI_KEY_DOWN = EVT_USER_START + 1,
	EVT_LOW_BATTERY
} UIEVENT_COMMON_E;

enum _MSG_EVENT_ID {
	MSG_EVENT_ID_LOW_BATTERY = 0,
	MSG_EVENT_ID_NO_CARD,
	MSG_EVENT_ID_SDCARD_NEED_FORMAT,
	MSG_EVENT_ID_SDCARD_SLOW,
	MSG_EVENT_ID_SDCARD_CHECKING,
	MSG_EVENT_ID_SDCARD_ERROR,
	MSG_EVENT_ID_SDCARD_READ_ONLY,
	MSG_EVENT_ID_SDCARD_MOUNT_FAILED,
	MSG_EVENT_ID_FORMAT_PROCESS,
	MSG_EVENT_ID_FORMAT_FAILED,
	MSG_EVENT_ID_FORMAT_SUCCESS,
	MSG_EVENT_ID_FILE_ABNORMAL,
	MSG_EVENT_ID_NOVIDEO_FILE,
	MSG_EVENT_ID_NOPHOTO_FILE,
	MSG_EVENT_ID_NOTSUPPORT_H265,
	MSG_EVENT_ID_SETTING_DEFUALT_FINISH,
	MSG_EVENT_ID_OTA_UP_FILE,
	MSG_EVENT_ID_OTA_UP_FILE_SUCCESSED,
	MSG_EVENT_ID_OTA_UP_FILE_FAIL,
	MSG_EVENT_ID_OTA_UP_FILE_FAIL_FILE_ERROR,
	MSG_EVENT_ID_UVC,
	MSG_EVENT_ID_MAX
};

enum _UI_DATE_FORAMT {
	DATE_YY_MM_DD,
	DATE_MM_DD_YY,
	DATE_DD_MM_YY,
};

typedef struct _UI_TIME_BUF_S {
	char year[DATETIME_BUF_LEN];
	char month[DATETIME_BUF_LEN];
	char day[DATETIME_BUF_LEN];
	char hour[DATETIME_BUF_LEN];
	char min[DATETIME_BUF_LEN];
	char sec[DATETIME_BUF_LEN];
	char ampm[DATETIME_BUF_LEN];
} UI_TIME_BUF_S;

int32_t UIAPP_Start(void);
int32_t UIAPP_Stop(void);
void ui_lock(void);
void ui_unlock(void);

int32_t ui_common_SubscribeEvents(void);

ret_t ui_home_page_open(void *ctx, event_t *e);
ret_t ui_home_page_close(void *ctx, event_t *e);
ret_t ui_set_page_close(void *ctx, event_t *e);
ret_t ui_set_page_open(void *ctx, event_t *e);
ret_t ui_option_page_open(void *ctx, event_t *e);
ret_t ui_option_page_close(void *ctx, event_t *e);
ret_t ui_ide_option_page_open(void *ctx, event_t *e);
ret_t ui_ide_option_page_close(void *ctx, event_t *e);
ret_t ui_identical_page_open(void *ctx, event_t *e);
ret_t ui_identical_page_close(void *ctx, event_t *e);
ret_t ui_datetime_page_open(void *ctx, event_t *e);
ret_t ui_datetime_page_close(void *ctx, event_t *e);
ret_t ui_wrnmsg_page_open(void *ctx, event_t *e);
ret_t ui_wrnmsg_page_close(void *ctx, event_t *e);
ret_t ui_dir_page_open(void *ctx, event_t *e);
ret_t ui_dir_page_close(void *ctx, event_t *e);
ret_t ui_dir_page_reset(void);
ret_t ui_playback_page_open(void *ctx, event_t *e);
ret_t ui_playback_page_close(void *ctx, event_t *e);
ret_t ui_playback_menu_page_open(void *ctx, event_t *e);
ret_t ui_playback_menu_page_close(void *ctx, event_t *e);
ret_t ui_playback_page_updatedir(uint32_t id);
ret_t ui_wrnmsg_update_type(uint32_t type);
ret_t ui_wrnmsg_get_type(void);
ret_t ui_wrnmsg_update_msglael(const idle_info_t *idle);
ret_t delete_file_playback(void);
ret_t delete_all_file_playback(void);
bool_t ui_check_time_format_type(void);
void option_setuiLanguage(int32_t param);
int32_t ui_homepage_eventcb(void *argv, EVENT_S *msg);
int32_t ui_playbackpage_eventcb(void *argv, EVENT_S *msg);
int32_t UICOMM_SendAsyncMsg(MESSAGE_S *pstMsg, UI_MSGRESULTPROC_FN_PTR pfnMsgResultProc);
uint8_t ui_settime_get_date_foramtid(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

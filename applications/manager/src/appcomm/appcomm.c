#include "appcomm.h"
#ifdef SERVICES_ADAS_ON
#include "adasmng.h"
#endif
#include "filemng.h"
#ifdef ADC_ON
#include "gaugemng.h"
#endif
#ifdef CONFIG_GSENSOR_ON
#include "gsensormng.h"
#endif
#ifdef KEY_ON
#include "keymng.h"
#endif
#ifdef SERVICES_VIDEO_MD_ON
#include "mdmng.h"
#endif
#ifdef SERVICES_PHOTO_ON
#include "photomng.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "playbackmng.h"
#endif
#include "recordmng.h"
#ifdef SERVICES_SPEECH_ON
#include "speechmng.h"
#endif
#include "storagemng.h"
#include "usb.h"
#ifdef APPLICATION_NET_ON
#include "netctrl.h"
#endif
#include "mode.h"
// #include "media_init.h"

/* Todo: UI Event include from ui_common.h */
typedef enum EVENT_UI_E {
	EVENT_UI_TOUCH = APPCOMM_EVENT_ID(APP_MOD_UI, 0),
	EVENT_UI_BUTT
} EVENT_UI_E;

const char *ui_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
	switch (topic)
	{
		ENUM_CASE(EVENT_UI_TOUCH);
		ENUM_CASE(EVENT_UI_BUTT);
	}
#undef ENUM_CASE
	return "UNKNOWN UI EVENT";
}

#ifdef SERVICES_ADAS_ON
static const char *adas_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_ADASMNG_CAR_MOVING);
        ENUM_CASE(EVENT_ADASMNG_CAR_CLOSING);
        ENUM_CASE(EVENT_ADASMNG_CAR_COLLISION);
        ENUM_CASE(EVENT_ADASMNG_CAR_LANE);
        ENUM_CASE(EVENT_ADASMNG_LABEL_CAR);
        ENUM_CASE(EVENT_ADASMNG_LABEL_LANE);
        ENUM_CASE(EVENT_ADASMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN ADAS EVENT";
}
#endif

static const char *file_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_FILEMNG_SCAN_COMPLETED);
        ENUM_CASE(EVENT_FILEMNG_SCAN_FAIL);
        ENUM_CASE(EVENT_FILEMNG_SPACE_FULL);
        ENUM_CASE(EVENT_FILEMNG_SPACE_ENOUGH);
        ENUM_CASE(EVENT_FILEMNG_REPAIR_BEGIN);
        ENUM_CASE(EVENT_FILEMNG_REPAIR_END);
        ENUM_CASE(EVENT_FILEMNG_REPAIR_FAILED);
        ENUM_CASE(EVENT_FILEMNG_UNIDENTIFICATION);
        ENUM_CASE(EVENT_FILEMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN FILE EVENT";
}

#ifdef ADC_ON
static const char *gauge_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_GAUGEMNG_LEVEL_CHANGE);
        ENUM_CASE(EVENT_GAUGEMNG_LEVEL_LOW);
        ENUM_CASE(EVENT_GAUGEMNG_LEVEL_ULTRALOW);
        ENUM_CASE(EVENT_GAUGEMNG_LEVEL_NORMAL);
        ENUM_CASE(EVENT_GAUGEMNG_CHARGESTATE_CHANGE);
        ENUM_CASE(EVENT_GAUGEMNG_BUIT);
    }
#undef ENUM_CASE
    return "UNKNOWN GAUGE EVENT";
}
#endif

#ifdef CONFIG_GSENSOR_ON
static const char *gsensor_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_GSENSORMNG_COLLISION);
        ENUM_CASE(EVENT_GSENSORMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN GSENSOR EVENT";
}
#endif

#ifdef KEY_ON
static const char *key_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_KEYMNG_SHORT_CLICK);
        ENUM_CASE(EVENT_KEYMNG_LONG_CLICK);
        ENUM_CASE(EVENT_KEYMNG_HOLD_DOWN);
        ENUM_CASE(EVENT_KEYMNG_HOLD_UP);
        ENUM_CASE(EVENT_KEYMNG_GROUP);
        ENUM_CASE(EVENT_KEYMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN KEY EVENT";
}
#endif

#ifdef SERVICES_VIDEO_MD_ON
static const char *md_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_MD_CHANGE);
        ENUM_CASE(EVENT_MD_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN MD EVENT";
}
#endif

#ifdef SERVICES_PHOTO_ON
static const char *photo_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_PHOTOMNG_OPEN_FAILED);
        ENUM_CASE(EVENT_PHOTOMNG_PIV_START);
        ENUM_CASE(EVENT_PHOTOMNG_PIV_END);
        ENUM_CASE(EVENT_PHOTOMNG_PIV_ERROR);
        ENUM_CASE(EVENT_PHOTOMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN PHOTO EVENT";
}
#endif

#ifdef SERVICES_PLAYER_ON
static const char *playback_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_PLAYBACKMNG_FINISHED);
        ENUM_CASE(EVENT_PLAYBACKMNG_PLAY);
        ENUM_CASE(EVENT_PLAYBACKMNG_PROGRESS);
        ENUM_CASE(EVENT_PLAYBACKMNG_PAUSE);
        ENUM_CASE(EVENT_PLAYBACKMNG_RESUME);
        ENUM_CASE(EVENT_PLAYBACKMNG_FILE_ABNORMAL);
        ENUM_CASE(EVENT_PLAYBACKMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN PLAYBACK EVENT";
}
#endif

static const char *record_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_RECMNG_STARTREC);
        ENUM_CASE(EVENT_RECMNG_STOPREC);
        ENUM_CASE(EVENT_RECMNG_SPLITSTART);
        ENUM_CASE(EVENT_RECMNG_SPLITREC);
        ENUM_CASE(EVENT_RECMNG_STARTEVENTREC);
        ENUM_CASE(EVENT_RECMNG_EVENTREC_END);
        ENUM_CASE(EVENT_RECMNG_STARTEMRREC);
        ENUM_CASE(EVENT_RECMNG_EMRREC_END);
        ENUM_CASE(EVENT_RECMNG_PIV_START);
        ENUM_CASE(EVENT_RECMNG_PIV_END);
        ENUM_CASE(EVENT_RECMNG_WRITE_ERROR);
        ENUM_CASE(EVENT_RECMNG_OPEN_FAILED);
        ENUM_CASE(EVENT_RECMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN RECORD EVENT";
}

#ifdef SERVICES_SPEECH_ON
static const char *speech_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_SPEECHMNG_STARTREC);
        ENUM_CASE(EVENT_SPEECHMNG_STOPREC);
        ENUM_CASE(EVENT_SPEECHMNG_OPENFRONT);
        ENUM_CASE(EVENT_SPEECHMNG_OPENREAR);
        ENUM_CASE(EVENT_SPEECHMNG_CLOSESCREEN);
        ENUM_CASE(EVENT_SPEECHMNG_OPENSCREEN);
        ENUM_CASE(EVENT_SPEECHMNG_EMRREC);
        ENUM_CASE(EVENT_SPEECHMNG_PIV);
        ENUM_CASE(EVENT_SPEECHMNG_CLOSEWIFI);
        ENUM_CASE(EVENT_SPEECHMNG_OPENWIFI);
        ENUM_CASE(EVENT_SPEECHMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN SPEECH EVENT";
}
#endif

static const char *storage_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_STORAGEMNG_DEV_UNPLUGED);
        ENUM_CASE(EVENT_STORAGEMNG_DEV_CONNECTING);
        ENUM_CASE(EVENT_STORAGEMNG_DEV_ERROR);
        ENUM_CASE(EVENT_STORAGEMNG_FS_CHECKING);
        ENUM_CASE(EVENT_STORAGEMNG_FS_CHECK_FAILED);
        ENUM_CASE(EVENT_STORAGEMNG_FS_EXCEPTION);
        ENUM_CASE(EVENT_STORAGEMNG_MOUNTED);
        ENUM_CASE(EVENT_STORAGEMNG_MOUNT_FAILED);
        ENUM_CASE(EVENT_STORAGEMNG_MOUNT_READ_ONLY);
        ENUM_CASE(EVENT_STORAGEMNG_START_UPFILE);
        ENUM_CASE(EVENT_STORAGEMNG_UPFILE_SUCCESSED);
        ENUM_CASE(EVENT_STORAGEMNG_UPFILE_FAIL);
        ENUM_CASE(EVENT_STORAGEMNG_UPFILE_FAIL_FILE_ERROR);
        ENUM_CASE(EVENT_STORAGEMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN STORAGE EVENT";
}

static const char *usb_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_USB_OUT);
        ENUM_CASE(EVENT_USB_INSERT);
        ENUM_CASE(EVENT_USB_UVC_READY);
        ENUM_CASE(EVENT_USB_STORAGE_READY);
        ENUM_CASE(EVENT_USB_HOSTUVC_READY);
        ENUM_CASE(EVENT_USB_HOSTUVC_PC);
        ENUM_CASE(EVENT_USB_HOSTUVC_HEAD);
        ENUM_CASE(EVENT_USB_PC_INSERT);
    }
#undef ENUM_CASE
    return "UNKNOWN USB EVENT";
}

#ifdef APPLICATION_NET_ON
static const char *netctrl_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_NETCTRL_CONNECT);
        ENUM_CASE(EVENT_NETCTRL_UIUPDATE);
        ENUM_CASE(EVENT_NETCTRL_APPCONNECT_SUCCESS);
        ENUM_CASE(EVENT_NETCTRL_APPDISCONNECT);
        ENUM_CASE(EVENT_NETCTRL_APPCONNECT_SETTING);
        ENUM_CASE(EVENT_NETCTRL_BUIT);
    }
#undef ENUM_CASE
    return "UNKNOWN NETCTRL EVENT";
}
#endif

static const char *mode_event_topic_get_name(uint32_t topic)
{
#define ENUM_CASE(x) case x: return #x
    switch (topic)
    {
        ENUM_CASE(EVENT_MODEMNG_CARD_REMOVE);
        ENUM_CASE(EVENT_MODEMNG_CARD_AVAILABLE);
        ENUM_CASE(EVENT_MODEMNG_CARD_UNAVAILABLE);
        ENUM_CASE(EVENT_MODEMNG_CARD_ERROR);
        ENUM_CASE(EVENT_MODEMNG_CARD_FSERROR);
        ENUM_CASE(EVENT_MODEMNG_CARD_SLOW);
        ENUM_CASE(EVENT_MODEMNG_CARD_CHECKING);
        ENUM_CASE(EVENT_MODEMNG_CARD_FORMAT);
        ENUM_CASE(EVENT_MODEMNG_CARD_FORMATING);
        ENUM_CASE(EVENT_MODEMNG_CARD_FORMAT_SUCCESSED);
        ENUM_CASE(EVENT_MODEMNG_CARD_FORMAT_FAILED);
        ENUM_CASE(EVENT_MODEMNG_CARD_READ_ONLY);
        ENUM_CASE(EVENT_MODEMNG_CARD_MOUNT_FAILED);
        ENUM_CASE(EVENT_MODEMNG_RESET);
        ENUM_CASE(EVENT_MODEMNG_MODESWITCH);
        ENUM_CASE(EVENT_MODEMNG_MODEOPEN);
        ENUM_CASE(EVENT_MODEMNG_MODECLOSE);
        ENUM_CASE(EVENT_MODEMNG_START_REC);
        ENUM_CASE(EVENT_MODEMNG_STOP_REC);
        ENUM_CASE(EVENT_MODEMNG_SWITCH_LIVEVIEW);
        ENUM_CASE(EVENT_MODEMNG_LIVEVIEW_UPORDOWN);
        ENUM_CASE(EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS);
        ENUM_CASE(EVENT_MODEMNG_START_PIV);
        ENUM_CASE(EVENT_MODEMNG_SMILE_START_PIV);
        ENUM_CASE(EVENT_MODEMNG_START_EMRREC);
        ENUM_CASE(EVENT_MODEMNG_SCREEN_DORMANT);
        ENUM_CASE(EVENT_MODEMNG_SETTING);
        ENUM_CASE(EVENT_MODEMNG_SETTING_LANGUAGE);
        ENUM_CASE(EVENT_MODEMNG_POWEROFF);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_PLAY);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_PROGRESS);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_PAUSE);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_RESUME);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_SEEK);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_SEEKPAUSE);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_FINISHED);
        ENUM_CASE(EVENT_MODEMNG_PLAYBACK_ABNORMAL);
        ENUM_CASE(EVENT_MODETEST_START_RECORD);
        ENUM_CASE(EVENT_MODETEST_STOP_RECORD);
        ENUM_CASE(EVENT_MODETEST_PLAY_RECORD);
        ENUM_CASE(EVENT_MODEMNG_START_UPFILE);
        ENUM_CASE(EVENT_MODEMNG_UPFILE_SUCCESSED);
        ENUM_CASE(EVENT_MODEMNG_UPFILE_FAIL);
        ENUM_CASE(EVENT_MODEMNG_UPFILE_FAIL_FILE_ERROR);
        ENUM_CASE(EVENT_MODEMNG_UVC_MODE_START);
        ENUM_CASE(EVENT_MODEMNG_UVC_MODE_STOP);
        ENUM_CASE(EVENT_MODEMNG_STORAGE_MODE_PREPAREDEV);
        ENUM_CASE(EVENT_MODEMNG_PHOTO_SET);
        ENUM_CASE(EVENT_MODEMNG_FOCUS);
        ENUM_CASE(EVENT_MODEMNG_PHOTO_STARTPIVSTAUE);
        ENUM_CASE(EVENT_MODEMNG_RTSP_INIT);
        ENUM_CASE(EVENT_MODEMNG_RTSP_SWITCH);
        ENUM_CASE(EVENT_MODEMNG_RTSP_DEINIT);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STARTSTATU);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STOPSTATU);
        ENUM_CASE(EVENT_MODEMNG_RECODER_SPLITREC);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STARTEVENTSTAUE);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STOPEVENTSTAUE);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STARTEMRSTAUE);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STOPEMRSTAUE);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STARTPIVSTAUE);
        ENUM_CASE(EVENT_MODEMNG_RECODER_STOPPIVSTAUE);
#ifdef SERVICES_SPEECH_ON
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_STARTREC);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_STOPREC);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_OPENFRONT);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_OPENREAR);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_CLOSESCREEN);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_OPENSCREEN);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_EMRREC);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_PIV);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_CLOSEWIFI);
        ENUM_CASE(EVENT_MODEMNG_SPEECHMNG_OPENWIFI);
        ENUM_CASE(EVENT_MODEMNG_START_SPEECH);
        ENUM_CASE(EVENT_MODEMNG_STOP_SPEECH);
#endif
        ENUM_CASE(EVENT_MODEMNG_BUTT);
    }
#undef ENUM_CASE
    return "UNKNOWN MODE EVENT";
}

const char *event_topic_get_name(uint32_t topic)
{
	/* */
#define ENUM_CASE(x) case x: return #x

	switch (APPCOMM_MODULE_ID(topic))
	{
#ifdef SERVICES_ADAS_ON
	case APP_MOD_ADASMNG:
		return adas_event_topic_get_name(topic);
#endif
	case APP_MOD_FILEMNG:
		return file_event_topic_get_name(topic);
#ifdef ADC_ON
	case APP_MOD_GAUGEMNG:
		return gauge_event_topic_get_name(topic);
#endif
#ifdef CONFIG_GSENSOR_ON
	case APP_MOD_GSENSORMNG:
		return gsensor_event_topic_get_name(topic);
#endif
#ifdef KEY_ON
	case APP_MOD_KEYMNG:
		return key_event_topic_get_name(topic);
#endif
#ifdef SERVICES_VIDEO_MD_ON
	case APP_MOD_MD:
		return md_event_topic_get_name(topic);
#endif
#ifdef SERVICES_PHOTO_ON
	case APP_MOD_PHOTOMNG:
		return photo_event_topic_get_name(topic);
#endif
#ifdef SERVICES_PLAYER_ON
	case APP_MOD_PLAYBACKMNG:
		return playback_event_topic_get_name(topic);
#endif
	case APP_MOD_RECMNG:
		return record_event_topic_get_name(topic);
#ifdef SERVICES_SPEECH_ON
	case APP_MOD_SPEECHMNG:
		return speech_event_topic_get_name(topic);
#endif
	case APP_MOD_USBMNG:
		return usb_event_topic_get_name(topic);
#ifdef APPLICATION_NET_ON
	case APP_MOD_NETCTRL:
		return netctrl_event_topic_get_name(topic);
#endif
	// case APP_MOD_MEDIA:
	// 	return media_event_topic_get_name(topic);
	case APP_MOD_MODEMNG:
		return mode_event_topic_get_name(topic);
	case APP_MOD_UI:
		return ui_event_topic_get_name(topic);
	case APP_MOD_STORAGEMNG:
		return storage_event_topic_get_name(topic);
		/* Note that default is not added here because when adding new event code,
		* if forget to add case, the compiler will automatically report a warning.
		*/
	}
	CVI_LOGE("UNKNOWN EVENT: %d, MODULE: %d", topic, APPCOMM_MODULE_ID(topic));

#undef ENUM_CASE

	return "EVENT_UNKNOWN";
}

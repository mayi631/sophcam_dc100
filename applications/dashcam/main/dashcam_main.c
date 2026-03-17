#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <ucontext.h>
#include <semaphore.h>
#include <malloc.h>
#include "system.h"
#include <sys/reboot.h>
#include "media_init.h"
#include "param.h"
#include "sysutils_eventhub.h"
#include "mode.h"
#include "storagemng.h"
#include "recordmng.h"
#ifdef SERVICES_PHOTO_ON
#include "photomng.h"
#endif
#include "filemng.h"
#include "usbctrl.h"
#include "gaugemng.h"
#include "tempermng.h"
#include "netctrl.h"
#include "ledmng.h"
#ifndef CHIP_184X
#include "cvi_msg_client.h"
#endif
#ifdef CONFIG_GSENSOR_ON
#include "gsensormng.h"
#endif
#ifdef SERVICES_LIVEVIEW_ON
#include "volmng.h"
#endif
#ifdef GPS_ON
#include "gpsmng.h"
#endif
#ifdef WIFI_ON
#include "wifimng.h"
#endif
#ifdef ADC_ON
#include "gaugemng.h"
#endif
#ifdef WATCHDOG_ON
#include "watchdogmng.h"
#include "hal_watchdog.h"
#endif
#ifdef SERVICES_SPEECH_ON
#include "speechmng.h"
#include "speech.h"
#endif
#ifdef SERVICES_PLAYER_ON
#include "playbackmng.h"
#include "player_service.h"
#endif
#ifdef SERVICES_ADAS_ON
#include "adasmng.h"
#endif

// #include "stacktrace.h"
// #include <mcheck.h>
// #define ENABLE_MTRACE 1

#ifdef ENABLE_ISP_PQ_TOOL
#include "media_dump.h"
#endif

#ifdef CHIP_184X
#include "cvi_sys.h"
#endif

static sem_t s_PowerOffSem; /** power off semaphore */
static pthread_t s_DelayedThread;
static pthread_t s_DelayedThread_Ao;
static MODEMNG_EXIT_MODE_E s_ExitMode = MODEMNG_EXIT_MODE_BUTT; /** exit mode */

void Sample_HandleSig(int32_t signo)
{
	sem_post(&s_PowerOffSem);
	CVI_LOGE("Sample_HandleSig signal number %d\n", signo);
	exit(signo);
}

void signal_process()
{
	signal(SIGINT, Sample_HandleSig);
	signal(SIGTERM, Sample_HandleSig);
	signal(SIGSEGV, Sample_HandleSig);
	signal(SIGABRT, Sample_HandleSig);
}

static int32_t ExitModeCallback(MODEMNG_EXIT_MODE_E enExitMode)
{
	s_ExitMode = enExitMode; /** exit mode */
	sem_post(&s_PowerOffSem);
	return 0;
}

int32_t ModuleDelayedStart()
{
	pthread_detach(pthread_self());
	int32_t s32Ret = 0;
#ifdef KEY_ON
	KEYMNG_CFG_S stKeyCfg;
	s32Ret = PARAM_GetKeyMngCfg(&stKeyCfg);
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Get key cfg");
	s32Ret = KEYMNG_Init(stKeyCfg);
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Key init");
#endif
#ifdef ADC_ON
	GAUGEMNG_CFG_S stGaugeCfg;
	s32Ret = PARAM_GetGaugeMngCfg(&stGaugeCfg);
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "get gauge cfg");
	s32Ret = GAUGEMNG_Init(&stGaugeCfg);
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "gauge init");
	s32Ret = GAUGEMNG_RegisterEvent();
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "gauge register event");
#endif
#ifdef CONFIG_LED_ON
	s32Ret = LEDMNG_Init();
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Led init");
#endif
	s32Ret = MODEMNG_TEST_MAIN_Create();
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Test mode");

#ifdef GPS_ON
	s32Ret = GPSMNG_Init();
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Gps init");

	s32Ret = GPSMNG_Start();
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Gps start");
#endif

#ifdef CONFIG_GSENSOR_ON
	GSENSORMNG_CFG_S GsensorParam;
	PARAM_GetGsensorParam(&GsensorParam);
	s32Ret = GSENSORMNG_Init(&GsensorParam);
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Gsensor init");
#endif

#ifdef WIFI_ON
	PARAM_WIFI_S WifiParam;
	s32Ret = PARAM_GetWifiParam(&WifiParam);

	if (true == WifiParam.Enable) {
		s32Ret |= WIFIMNG_Start(WifiParam.WifiCfg, WifiParam.WifiDefaultSsid);
	}
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "Wifi Init");
#endif

#ifdef WATCHDOG_ON
	int32_t s32FeedTime_s = 10; /**10s periodly feed dog*/
	s32Ret = WATCHDOGMNG_Init(s32FeedTime_s);
	APPCOMM_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "WATCHDOGMNG init");
#endif

#ifdef ENABLE_ISP_PQ_TOOL
	OSAL_FS_System("/etc/uhubon.sh host");
	OSAL_FS_System("/etc/uhubon.sh device");
	OSAL_FS_System("/etc/run_usb.sh probe rndis");
	OSAL_FS_System("/etc/run_usb.sh start");
	OSAL_FS_System("ifconfig usb0 192.168.0.103 up netmask 255.255.255.0");
#endif

	pthread_exit(0);
	return s32Ret;
}

int32_t ModuleDelayedStartThread(void)
{
	int32_t s32Ret = 0;

	s32Ret = pthread_create(&s_DelayedThread, NULL, (void *)ModuleDelayedStart, NULL);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	return s32Ret;
}

static int32_t ModuleAoStart()
{
	pthread_detach(pthread_self());

	int32_t s32Ret = 0;
	s32Ret = MEDIA_AoInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
	// pthread_exit(0);

	s32Ret = MEDIA_PlayBootSound();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = MEDIA_AoDeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	pthread_exit(0);
	return s32Ret;
}

int32_t ModuleAoStartThread(void)
{
	int32_t s32Ret = 0;

	s32Ret = pthread_create(&s_DelayedThread_Ao, NULL, (void *)ModuleAoStart, NULL);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	return s32Ret;
}

static int32_t module_init(void)
{
	int32_t s32Ret = 0;

	// set log level
	//  release set CVI_LOG_INFO
	//  CVI_LOG_SET_LEVEL(CVI_LOG_INFO);
	CVI_LOGD("main app starting...\n");

	s32Ret = PARAM_Ini2bin();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = PARAM_Init();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = SYSTEM_SetDefaultDateTime();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	#ifdef CHIP_184X
	s32Ret = CVI_SYS_Init();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
	#else
	s32Ret = CVI_MSG_Init();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
	#endif

	// s32Ret = ModuleAoStartThread();
	// APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = EVENTHUB_Init();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
	STORAGEMNG_RegisterEvent();
	RECORDMNG_RegisterEvent();
#ifdef SERVICES_PHOTO_ON
	POHTOMNG_RegisterEvent();
#endif
#ifdef SERVICES_PLAYER_ON
	PLAYBACKMNG_RegisterEvent();
#endif
#ifdef SERVICES_ADAS_ON
	ADASMNG_RegisterEvent();
#endif
	MODEMNG_RegisterEvent();
	// USBCTRL_RegisterEvent();

	SYSTEM_STARTUP_SRC_E enStartupSrc;
	SYSTEM_GetStartupWakeupSource(&enStartupSrc);
	CVI_LOGD("=============================enStartupSrc = %d\n", enStartupSrc);
	if (enStartupSrc == SYSTEM_STARTUP_SRC_GSENSORWAKEUP) {
		MODEMNG_SetParkingRec(true);
	} else {
		MODEMNG_SetParkingRec(false);
	}

#ifdef ENABLE_ISP_PQ_TOOL
	bool en = false;
	MEDIA_DUMP_GetSizeStatus(&en);
	PARAM_CFG_S sysparams;
	PARAM_GetParam(&sysparams);
	if (en == true) {
		for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
			sysparams.MediaComm.Rtsp.ChnAttrs[i].BindVencId = 0 + i * 4;
		}
	} else {
		for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
			sysparams.MediaComm.Rtsp.ChnAttrs[i].BindVencId = 1 + i * 4;
		}
	}
	PARAM_SetParam(&sysparams);
#endif

	MODEMNG_CONFIG_S stModemngCfg;
	stModemngCfg.pfnExitCB = ExitModeCallback;
	s32Ret = MODEMNG_Init(&stModemngCfg);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = ModuleDelayedStartThread();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	// open uvc mode
	// USB_SetMode(USB_MODE_UVC);
#ifdef CONFIG_APPLICATION_NET
	s32Ret = NETCTRL_Init();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef SERVICES_SPEECH_ON
	SPEECHMNG_PARAM_S SpeechParam;
	PARAM_GetSpeechParam(&SpeechParam);
	MAPI_ACAP_ATTR_S AiAttr = {0};
	PARAM_GetAiParam(&AiAttr);
	SpeechParam.AiNumPerFrm = AiAttr.u32PtNumPerFrm;
	s32Ret = SPEECHMNG_Init(&SpeechParam);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
	return 0;
}

static int32_t System_Exit()
{
	switch (s_ExitMode) {
	case MODEMNG_EXIT_MODE_POWEROFF: {
		CVI_LOGI("###### POWEROFF #####\n\n");
		reboot(RB_POWER_OFF);
		break;
	}
	case MODEMNG_EXIT_MODE_REBOOT: {
		CVI_LOGI("###### reboot #####\n\n");
		reboot(RB_AUTOBOOT);
		break;
	}
	default:
		CVI_LOGI("s_ExitMode error\n\n");
		reboot(RB_AUTOBOOT);
		break;
	}
	return 0;
}

static int32_t module_deinit(void)
{
	int32_t s32Ret = 0;

#ifdef SCREEN_ON
	s32Ret = HAL_COMM_SCREEN_Deinit(HAL_SCREEN_IDXS_0);
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

	s32Ret = MODEMNG_Deinit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#ifdef KEY_ON
	s32Ret = KEYMNG_DeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef CONFIG_LED_ON
	s32Ret = LEDMNG_DeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif
	s32Ret = MODEMNG_TEST_MAIN_Destroy();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = EVENTHUB_DeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = PARAM_Deinit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

	s32Ret = System_Exit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);

#ifdef CONFIG_APPLICATION_NET
	s32Ret = NETCTRL_DeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef WATCHDOG_ON
	s32Ret = WATCHDOGMNG_DeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

#ifdef SERVICES_SPEECH_ON
	s32Ret = SPEECHMNG_DeInit();
	APPCOMM_CHECK_RETURN(s32Ret, s32Ret);
#endif

	return 0;
}

int32_t main(int32_t argc, char *argv[])
{
	int32_t s32Ret = 0;
	CVI_LOG_OPEN();
#ifdef ENABLE_MTRACE
	setenv("MALLOC_TRACE", "/mnt/data/mem.txt", 1);
	mtrace();
#endif
	signal(SIGPIPE, SIG_IGN);
	// signal_process();
	// mallopt(M_TRIM_THRESHOLD, 1024);
	/** init semaphore */
	sem_init(&s_PowerOffSem, 0, 0);

	module_init();

	while ((0 != sem_wait(&s_PowerOffSem)) && (errno == EINTR))
		;

	module_deinit();
	CVI_LOG_CLOSE();
	return s32Ret;
}

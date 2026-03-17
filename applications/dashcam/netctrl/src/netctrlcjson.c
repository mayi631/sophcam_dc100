#include <stdio.h>
#include <sys/statfs.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <netinet/tcp.h>
#include "netctrlinner.h"
#include "filemng.h"
#include "cJSON.h"

static int32_t flag_socket = WIFI_APP_DISCONNECT;
static pthread_t g_websocket_thread;
static char *FoRStat = "front";
static int32_t flag_file = 0;
static double EV_value = -1;
static int32_t EV_Manual = 0;
static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

static void cJson_common(httpd_conn *hc, int32_t state, char *info)
{
	cJSON *cJson_app = NULL;
	char *res = NULL;
	cJson_app = cJSON_CreateObject();
	cJSON_AddNumberToObject(cJson_app, "result", state);
	cJSON_AddStringToObject(cJson_app, "info", info);
	res = cJSON_Print(cJson_app);
	NET_AddResponse(hc, res, strlen(res));
	if (res != NULL) {
		free(res);
	}
	cJSON_Delete(cJson_app);
}

static void cJson_itemcommon(httpd_conn *hc, int32_t state, cJSON *info)
{
	cJSON *cJson_app = NULL;
	char *res = NULL;
	cJson_app = cJSON_CreateObject();
	cJSON_AddNumberToObject(cJson_app, "result", state);
	cJSON_AddItemToObject(cJson_app, "info", info);
	res = cJSON_Print(cJson_app);
	NET_AddResponse(hc, res, strlen(res));
	if (res != NULL) {
		free(res);
	}
	cJSON_Delete(cJson_app);
}

static cJSON *cJson_str_variable(int32_t cnt, ...)
{
	cJSON *cJson_app = NULL;
	cJson_app = cJSON_CreateObject();
	char *s1 = NULL;
	char *s2 = NULL;
	int32_t num = 0;
	va_list ap;
	va_start(ap, cnt);
	for (int32_t i = 0; i < cnt; i++) {
		if (i % 2 == 0) {
			s1 = va_arg(ap, char *);
			num++;
		} else {
			s2 = va_arg(ap, char *);
			num++;
		}
		if (num == 2) {
			cJSON_AddStringToObject(cJson_app, s1, s2);
			num = 0;
		}
	}
	return cJson_app;
}

static cJSON *cJson_num_variable(int32_t cnt, ...)
{
	cJSON *cJson_app = NULL;
	cJson_app = cJSON_CreateObject();
	char *s1 = NULL;
	int32_t d = 0;
	int32_t num = 0;
	va_list ap;
	va_start(ap, cnt);
	for (int32_t i = 0; i < cnt; i++) {
		if (i % 2 == 0) {
			s1 = va_arg(ap, char *);
			num++;
		} else {
			d = va_arg(ap, int32_t);
			num++;
		}
		if (num == 2) {
			cJSON_AddNumberToObject(cJson_app, s1, d);
			num = 0;
		}
	}
	return cJson_app;
}

// 6.1.1 获取产品信息 http://(IP)/app/getproductinfo
int32_t APP_GetProductInfo(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJSON *cjson_info = cJson_str_variable(8, "model", "cardv", "company", "sophgo", "soc", "sophgo",
										   "sp", "cv");
	cJson_itemcommon(hc, result, cjson_info);
	return result;
}

// 6.1.2 获取设备信息 http://(IP)/app/getdeviceattr
int32_t APP_GetDeviceAttr(httpd_conn *hc, char *fun, char *str)
{
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	int32_t result = 0;
	// 连接APP，需从录像模式进入
	if (MODEMNG_GetCurWorkMode() != WORK_MODE_MOVIE) {
		MESSAGE_S Msg = {0};
		int32_t count = 0;
		Msg.topic = EVENT_MODEMNG_MODESWITCH;
		Msg.arg1 = WORK_MODE_MOVIE;
		NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
			return result;
		}
		// 等待切换录像模式
		while (MODEMNG_GetCurWorkMode() != WORK_MODE_MOVIE) {
			usleep(100 * 1000);
			if (count++ > 40) {
				CVI_LOGE("change mode fail");
				result = WIFIAPP_FALSE;
				cJson_common(hc, result, "set fail.");
				return result;
			}
		}
	}
	flag_socket = WIFI_APP_DISCONNECT;
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_CONNECT;
	EVENTHUB_Publish(&stEvent);

	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_APPCONNECT_SETTING;
	stEvent.arg1 = WIFI_APP_SETTING_OUT;
	EVENTHUB_Publish(&stEvent);

	char *wifiname = NULL;
	cJSON *cjson_info = cJSON_CreateObject();

	PARAM_WIFI_S wifipar;
	PARAM_GetWifiParam(&wifipar);
	wifiname = wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID;

	char szCmd[192] = {'\0'};
	char szCmd_s[192] = {'\0'};
	snprintf(szCmd, sizeof(szCmd), "%s", "mac=`ifconfig -a " HAL_WIFI_INTERFACE_NAME " | head -1 | awk '{ print $5 }' `;  echo ${mac}\n");
	snprintf(szCmd_s, sizeof(szCmd), "%s", "mac=`ifconfig -a " HAL_WIFI_INTERFACE_NAME " | head -1 | awk '{ print $5 }' | tr -d ':' `;  echo ${mac:8:4}\n");
	char szMacAddrResult[24] = {0};
	char szMacAddrResult_s[24] = {0};
	FILE *fp = popen(szCmd, "r");
	FILE *fp_s = popen(szCmd_s, "r");
	if (NULL == fp || NULL == fp_s) {
		perror("popen failed\n");
	} else {
		fgets(szMacAddrResult, sizeof(szMacAddrResult), fp);
		fgets(szMacAddrResult_s, sizeof(szMacAddrResult_s), fp_s);
		pclose(fp);
		pclose(fp_s);
	}
	szMacAddrResult[strlen(szMacAddrResult) - 1] = '\0';
	szMacAddrResult_s[strlen(szMacAddrResult_s) - 1] = '\0';
	if (strcmp(wifiname, wifipar.WifiDefaultSsid) == 0) {

		strcat(wifiname, szMacAddrResult_s);
	}

	int32_t camnum = 1;
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	if (((param.ViewWin.Current >> 1) & 0x1) == 1) {
		camnum = 2;
	}

	cJSON_AddStringToObject(cjson_info, "uuid", szMacAddrResult);
	cJSON_AddStringToObject(cjson_info, "softver", "CVITEK20210901");
	cJSON_AddStringToObject(cjson_info, "ssid", wifiname);
	cJSON_AddStringToObject(cjson_info, "bssid", szMacAddrResult);
	cJSON_AddNumberToObject(cjson_info, "camnum", camnum);
	cJSON_AddNumberToObject(cjson_info, "curcamid", 0);
	cJSON_AddNumberToObject(cjson_info, "wifireboot", 1);
	cJson_itemcommon(hc, result, cjson_info);

	return result;
}

// 6.1.3 获取媒体信息 http://(IP)/app/getmediainfo
int32_t APP_GetMediaInfo(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJSON *cjson_test = NULL;
	cJSON *cjson_info = NULL;
	char *appbuff = NULL;
	char RtspName_F[128] = "rtsp://";
	char RtspName_R[128] = "rtsp://";
	strcat(RtspName_F, HAL_WIFI_IP);
	strcat(RtspName_F, "/");
	strcat(RtspName_R, HAL_WIFI_IP);
	strcat(RtspName_R, "/");
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	PARAM_MEDIA_COMM_S meida_param;
	PARAM_GetMediaComm(&meida_param);
	strcat(RtspName_F, meida_param.Rtsp.ChnAttrs[0].Name);
	if (MAX_CAMERA_INSTANCES > 1) {
		strcat(RtspName_R, meida_param.Rtsp.ChnAttrs[1].Name);
	}
	cjson_test = cJSON_CreateObject();
	cjson_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(cjson_test, "result", 0);
	if (strcmp(FoRStat, "rear") == 0 && ((param.ViewWin.Current >> 1) & 0x1) == 1) {
		cJSON_AddStringToObject(cjson_info, "rtsp", RtspName_R);
	} else {
		cJSON_AddStringToObject(cjson_info, "rtsp", RtspName_F);
		FoRStat = "front";
	}
	cJSON_AddStringToObject(cjson_info, "transport", "udp");
	cJSON_AddNumberToObject(cjson_info, "port", 6035);
	// if (MAX_CAMERA_INSTANCES > 1) {
	//     cJSON* cjson_rtsps = NULL;
	//     cjson_rtsps = cJSON_CreateArray();
	//     cJSON_AddItemToArray(cjson_rtsps, cJSON_CreateString(RtspName_F));
	//     cJSON_AddItemToArray(cjson_rtsps, cJSON_CreateString(RtspName_R));
	//     cJSON_AddItemToObject(cjson_info, "rtsps", cjson_rtsps);
	// }
	cJSON_AddNumberToObject(cjson_info, "page", 1);
	cJSON_AddItemToObject(cjson_test, "info", cjson_info);

	appbuff = cJSON_Print(cjson_test);
	NET_AddResponse(hc, appbuff, strlen(appbuff));
	cJSON_Delete(cjson_test);
	if (appbuff != NULL) {
		free(appbuff);
	}

	return result;
}

// 6.1.4 获取SD卡信息 http://(IP)/app/getsdinfo
int32_t APP_GetSDInfo(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJSON *cjson_info = NULL;
	if (MODEMNG_GetCardState() == CARD_STATE_AVAILABLE) {
		unsigned long long rspace = 0;
		unsigned long long totalspace = 0;
		struct statfs diskInfo;
		STG_DEVINFO_S sd_param = {0};
		PARAM_GetStgInfoParam(&sd_param);
		statfs(sd_param.aszMntPath, &diskInfo);
		unsigned long long blocksize = diskInfo.f_bsize;
		rspace = diskInfo.f_bfree * blocksize;
		totalspace = diskInfo.f_blocks * blocksize;
		totalspace = (totalspace >> 20); // 0: B 10: KB 20: MB 30: GB
		rspace = (rspace >> 20);		 // 0: B 10: KB 20: MB 30: GB
		cjson_info = cJson_num_variable(6, "status", 0, "free", (int32_t)rspace, "total", (int32_t)totalspace);
	} else if (MODEMNG_GetCardState() == CARD_STATE_UNAVAILABLE) {
		cjson_info = cJson_num_variable(2, "status", 1);
	} else {
		cjson_info = cJson_num_variable(2, "status", 2);
	}
	cJson_itemcommon(hc, result, cjson_info);

	return result;
}

// 6.1.6 设置系统时间 http://(IP)/app/setsystime?date=20200628235650
int32_t APP_SetSysTime(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
	} else {
		cJson_common(hc, result, "set fail.");
		result = WIFIAPP_FALSE;
		return result;
	}
	SYSTEM_TM_S pstDateTime = {0};
	int32_t count = sscanf(fun, "%4d%2d%2d%2d%2d%2d", &pstDateTime.s32year, &pstDateTime.s32mon, &pstDateTime.s32mday,
						   &pstDateTime.s32hour, &pstDateTime.s32min, &pstDateTime.s32sec);
	if (count != 6) {
		cJson_common(hc, result, "set fail.");
		result = WIFIAPP_FALSE;
		return result;
	}
	SYSTEM_SetDateTime(&pstDateTime);
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.1.7 设置时区 http://(IP)/app/settimezone
int32_t APP_SetTimeZone(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.1.8 获取当前录像时长 http://(IP)/app/getrecduration
int32_t APP_GetRecDuration(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	// cJSON* cjson_info;
	// uint32_t time = 0;
	// cjson_info = cJson_num_variable(2, "duration", time);
	// cJson_itemcommon(hc, result, cjson_info);
	return result;
}

// 6.1.9 设置界面 http://(IP)/app/setting
int32_t APP_Setting(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_APPCONNECT_SETTING;

	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
		if (strcmp(fun, "exit") == 0) {
			stEvent.arg1 = WIFI_APP_SETTING_OUT;
		} else {
			stEvent.arg1 = WIFI_APP_SETTING_IN;
		}
	} else {
		stEvent.arg1 = WIFI_APP_SETTING_IN;
	}

	EVENTHUB_Publish(&stEvent);
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.1.10 回放界面 http://(IP)/app/playback
int32_t APP_Playback(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_APPCONNECT_SETTING;

	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
		if (strcmp(fun, "exit") == 0) {
			stEvent.arg1 = WIFI_APP_SETTING_OUT;
		} else {
			stEvent.arg1 = WIFI_APP_SETTING_IN;
		}
	} else {
		stEvent.arg1 = WIFI_APP_SETTING_IN;
	}

	EVENTHUB_Publish(&stEvent);
	if (flag_file == 1) {
		flag_file = 0; // 恢复标志位，如果非异常断联，则不重新扫描文件
		NETCTRLINNER_ScanFile();
	}
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.2.1 修改wifi名称 http://(IP)/app/setwifi?wifissid=wahaha
int32_t APP_SetWiFissid(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t s32Ret = 0;
	PARAM_WIFI_S wifipar;
	s32Ret = PARAM_GetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("Api passphrase cmd PARAM_GetWifiParam faile\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fail.");
		return result;
	}
	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
		memcpy(wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID, fun, HAL_WIFI_SSID_LEN);
	}
	s32Ret = PARAM_SetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("SetWiFipassphrase PARAM_SetWifiParam fail\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fail.");
		return result;
	}
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.2.2 修改wifi密码 http://(IP)/app/setwifi?wifipwd=81sd112
int32_t APP_SetWiFipwd(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t s32Ret = 0;
	PARAM_WIFI_S wifipar;

	s32Ret = PARAM_GetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("Api passphrase cmd PARAM_GetWifiParam faile\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fail.");
		return result;
	}

	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
		memcpy(wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord, fun, HAL_WIFI_PASSWORD_LEN);
	}

	s32Ret = PARAM_SetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("SetWiFipassphrase PARAM_SetWifiParam fail\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fail.");
		return result;
	}
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.2.1-2 修改wifi http://(IP)/app/setwifi
int32_t APP_SetWiFi(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	if (fun != NULL) {
		if (strstr(fun, "wifissid") != NULL) {
			result = APP_SetWiFissid(hc, fun, str);
		} else if (strstr(fun, "wifipwd") != NULL) {
			result = APP_SetWiFipwd(hc, fun, str);
		}
	} else {
		result = WIFIAPP_FALSE;
	}
	return result;
}

// 6.2.3 格式化SD卡 http://(IP)/app/sdformat
int32_t APP_SDFormat(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	uint32_t u32ModeState = 0;
	int32_t count = 0;
	MESSAGE_S Msg = {0};

	// 如果录像还没停止，先停止录像
	MODEMNG_GetModeState(&u32ModeState);
	if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
		(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

		if (0 != s32Result || 0 != sendresult) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
			cJson_common(hc, result, "set fail.");
			return result;
		}
		usleep(100 * 1000);
	}
	NETCTRLINNER_StopTimer();
	// 开始格式化SD卡
	NETCTRLINNER_SetSdState(WIFI_SD_FORMAT_INIT);
	Msg.topic = EVENT_MODEMNG_CARD_FORMAT;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != s32Result || 0 != sendresult) {
		CVI_LOGE("MODEMNG_Format failed\n");
		cJson_common(hc, result, "set fail.");
	} else {
		while (NETCTRLINNER_GetSdState() && count++ < 20) {
			usleep(100 * 1000);
		}
		if (count >= 20 || NETCTRLINNER_GetSdState() == WIFI_SD_FORMAT_FAIL) {
			result = WIFIAPP_FALSE;
			cJson_common(hc, result, "set fail.");
		} else {
			cJson_common(hc, result, "set success.");
		}
	}
	NETCTRLINNER_StartTimer();
	return result;
}

// 6.2.4 恢复出厂设置 http://(IP)/app/reset
int32_t APP_Reset(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_DEFAULT;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != s32Result || 0 != sendresult) {
		CVI_LOGE("send message fail\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fail.");
	} else {
		cJson_common(hc, result, "set success.");
	}
	return result;
}

// 6.2.5 获取摄像头信息 http://(IP)/app/getcamerainfo
int32_t APP_GetCameraInfo(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	char *appbuff = NULL;
	cJSON *cjson_app = NULL;
	cJSON *cjson_info = NULL;
	cjson_app = cJSON_CreateObject();
	cjson_info = cJSON_CreateObject();
	cJSON_AddNumberToObject(cjson_app, "result", 0);
	cJSON_AddNumberToObject(cjson_info, "camnum", MAX_CAMERA_INSTANCES);

	// 获取可用摄像头数
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	char *cam_num = NULL;
	if (((param.ViewWin.Current >> 1) & 0x1) == 1) {
		cam_num = "0,1";
	} else {
		cam_num = "0";
	}
	cJSON_AddStringToObject(cjson_info, "online", cam_num);

	// 当前摄像头
	if (strcmp(FoRStat, "rear") == 0) {
		cJSON_AddNumberToObject(cjson_info, "curcamid", 1);
	} else {
		cJSON_AddNumberToObject(cjson_info, "curcamid", 0);
	}

	cJSON_AddItemToObject(cjson_app, "info", cjson_info);

	appbuff = cJSON_Print(cjson_app);
	NET_AddResponse(hc, appbuff, strlen(appbuff));
	cJSON_Delete(cjson_app);
	if (appbuff != NULL) {
		free(appbuff);
	}
	return result;
}

// 6.2.6 修改摄像头参数 http://(IP)/app/setparamvalue?param=switchcam&value=1
int32_t APP_ParamSwitchCam(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	char *cam_id = NULL;
	cam_id = strrchr(str, '=') + 1;
	if (strcmp(cam_id, "1") == 0) {
		FoRStat = "rear";
	} else {
		FoRStat = "front";
	}
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.2.7 修改录像状态 http://(IP)/app/setparamvalue?param=rec&value=1
int32_t APP_ParamRec(httpd_conn *hc, char *fun, char *str)
{
	int32_t pstr = 0;
	int32_t result = 0;
	int32_t sendresult = 0;
	int32_t s32Result = 0;
	uint32_t u32ModeState = 0;
	if (str != NULL) {
		str = strrchr(str, '=') + 1;
		sscanf(str, "%d", &pstr);
	}
	MESSAGE_S Msg = {0};
	MODEMNG_GetModeState(&u32ModeState);
	// 检测参数是否正确
	if ((0 != pstr) && (1 != pstr)) {
		CVI_LOGE("Startandstopmovierecord error, par parameter error\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fali.");
		return result;
	}

	// 检测sd卡时候正常
	int32_t CardStatus = MODEMNG_GetCardState();
	if (CardStatus != CARD_STATE_AVAILABLE) {
		CVI_LOGE("sd card no insert");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fali.");
		return result;
	}

	if (0 == pstr) {
		// 0 means stop record
		if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
			(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
			Msg.topic = EVENT_MODEMNG_STOP_REC;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		}
	} else if (1 == pstr) {
		// 1 means start record
		if ((u32ModeState != MEDIA_MOVIE_STATE_REC) &&
			(u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC)) {
			Msg.topic = EVENT_MODEMNG_START_REC;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		}
	}

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fali.");
	} else {
		cJson_common(hc, result, "set success.");
	}

	return result;
}

// 6.2 修改分辨率 http://(IP)/app/setparamvalue?param=rec_resolution&value=1
int32_t APP_ParamRes(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	if (str != NULL) {
		str = strrchr(str, '=') + 1;
		sscanf(str, "%d", &pstr);
	}
	PARAM_MENU_S menuparam;
	PARAM_GetMenuParam(&menuparam);
	if (menuparam.VideoSize.Current == (uint32_t)pstr) {
		cJson_common(hc, result, "set success.");
		return result;
	}
	if (pstr == 1)
		pstr = MEDIA_VIDEO_SIZE_1920X1080P25;
	else if (pstr == 0)
		pstr = MEDIA_VIDEO_SIZE_2560X1440P25;
	else {
		result = WIFIAPP_FALSE;
		return result;
	}

	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_VIDEO_SIZE;
	Msg.arg2 = pstr;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fali.");
	} else {
		cJson_common(hc, result, "set success.");
		NETCTRLINNER_UiUpdate();
	}

	usleep(500 * 1000); // 切分辨率需要初始化系统，耗时较长，等待500ms是为了等待数据写入缓存，防止APP快速操作引起数据混乱
	return result;
}

// 6.2 修改录像时长 http://(IP)/app/setparamvalue?param=rec_split_duration&value=0
int32_t APP_ParamSpl(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_VIDEO_LOOP;
	if (str != NULL) {
		str = strrchr(str, '=') + 1;
		sscanf(str, "%d", &pstr);
	}
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	if ((0 != pstr) && (1 != pstr) && (2 != pstr)) {
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fali.");
	} else {
		if (param.VideoLoop.Current != (uint32_t)pstr) {
			Msg.arg2 = pstr;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

			if (0 != sendresult || 0 != s32Result) {
				CVI_LOGE("send message fail");
				result = WIFIAPP_FALSE;
				cJson_common(hc, result, "set fali.");
			} else {
				NETCTRLINNER_UiUpdate();
				cJson_common(hc, result, "set success.");
			}
		} else {
			cJson_common(hc, result, "set success.");
		}
	}
	return result;
}

// 6.2 声音录制开关 http://(IP)/app/setparamvalue?param=mic&value=0
int32_t APP_ParamMic(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
	if (str != NULL) {
		str = strrchr(str, '=') + 1;
		sscanf(str, "%d", &pstr);
	}
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	if ((0 != pstr) && (1 != pstr)) {
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fali.");
	} else {
		if (param.AudioEnable.Current != (uint32_t)pstr) {
			Msg.arg2 = pstr;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

			if (0 != sendresult || 0 != s32Result) {
				CVI_LOGE("send message fail");
				result = WIFIAPP_FALSE;
				cJson_common(hc, result, "set fali.");
			} else {
				NETCTRLINNER_UiUpdate();
				cJson_common(hc, result, "set success.");
			}
		} else {
			cJson_common(hc, result, "set success.");
		}
	}
	return result;
}

// 6.2 曝光补偿的设置 http://(IP)/app/setparamvalue?param=ev&value=0
int32_t APP_ParamEv(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t value = 0;
	if (str != NULL) {
		str = strrchr(str, '=') + 1;
		sscanf(str, "%d", &pstr);
	}
	if (pstr == 0)
		value = EV_value / 3.98;
	else if (pstr == 1)
		value = EV_value / 3.16;
	else if (pstr == 2)
		value = EV_value / 2.5;
	else if (pstr == 3)
		value = EV_value / 1.99;
	else if (pstr == 4)
		value = EV_value / 1.58;
	else if (pstr == 5)
		value = EV_value / 1.26;
	else if (pstr == 6)
		value = EV_value;
	else if (pstr == 7)
		value = EV_value * 1.26;
	else if (pstr == 8)
		value = EV_value * 1.58;
	else if (pstr == 9)
		value = EV_value * 1.99;
	else if (pstr == 10)
		value = EV_value * 2.5;
	else if (pstr == 11)
		value = EV_value * 3.16;
	else if (pstr == 12)
		value = EV_value * 3.98;
	else {
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "set fail.");
		return result;
	}
	EV_Manual = 1;
	VI_PIPE ViPipe = 0;
	ISP_EXPOSURE_ATTR_S expAttr;
	CVI_ISP_GetExposureAttr(ViPipe, &expAttr);
	expAttr.enOpType = OP_TYPE_MANUAL;
	expAttr.bByPass = 0;
	expAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
	expAttr.stManual.u32ExpTime = value;
	CVI_ISP_SetExposureAttr(ViPipe, &expAttr);
	cJson_common(hc, result, "set success.");
	return result;
}

// 6.2 修改设备参数 http://(IP)/app/setparamvalue?param=()&value=()
int32_t APP_SetParamValue(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
		if (strcmp(fun, "switchcam") == 0) {
			result = APP_ParamSwitchCam(hc, fun, str);
		} else if (strcmp(fun, "rec") == 0) {
			result = APP_ParamRec(hc, fun, str);
		} else if (strcmp(fun, "rec_resolution") == 0) {
			result = APP_ParamRes(hc, fun, str);
		} else if (strcmp(fun, "rec_split_duration") == 0) {
			result = APP_ParamSpl(hc, fun, str);
		} else if (strcmp(fun, "mic") == 0) {
			result = APP_ParamMic(hc, fun, str);
		} else if (strcmp(fun, "ev") == 0) {
			result = APP_ParamEv(hc, fun, str);
		}
	} else {
		result = WIFIAPP_FALSE;
	}
	return result;
}

// 6.2.8 拍照 http://(IP)/app/snapshot
int32_t APP_SnapShot(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	MESSAGE_S Msg = {0};
	int32_t s32Result = 0;
	int32_t sendresult = 0;

	if(MODEMNG_GetCardState() != CARD_STATE_AVAILABLE){
		CVI_LOGE("Card is not available\n");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "snapshot fail.");
		return result;
	}

	Msg.topic = EVENT_MODEMNG_START_PIV;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
		cJson_common(hc, result, "snapshot fail.");
	} else {
		cJson_common(hc, result, "snapshot success.");
	}
	return result;
}

// 6.2.10 查看锁挡状态 http://(IP)/app/getlockvideostatus
int32_t APP_GetLockVideoStatus(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJSON *INFO = cJSON_CreateArray();
	// if (MODEMNG_GetEmrState() == true) {
	//     cJSON_AddNumberToObject(INFO, "status", 1);
	// } else {
	//     cJSON_AddNumberToObject(INFO, "status", 0);
	// }
	cJson_itemcommon(hc, result, INFO);
	return result;
}

// 6.2.11 重启WiFi http://(IP)/app/wifireboot
int32_t APP_WiFiReboot(httpd_conn *hc, char *fun, char *str)
{
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	int32_t result = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_WIFI_STATUS;
	Msg.arg2 = 0;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("ReconnectWiFi wifi stop fail");
		cJson_common(hc, result, "set fail.");
		return sendresult;
	}

	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_WIFI_STATUS;
	Msg.arg2 = 1;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		cJson_common(hc, result, "set fail.");
		return sendresult;
	}
	cJson_common(hc, result, "set success.");
	return sendresult;
}

// 6.3.1 获取文件列表 http://(IP)/app/getfilelist
int32_t APP_GetFileListAll(httpd_conn *hc, char *fun, char *str)
{
	flag_file = 1; // 设置标志位，如果异常断联，需要重新扫描文件
	char fpathname[FILEMNG_PATH_MAX_LEN];
	int32_t result = 0;
	char *appbuff = NULL;
	uint32_t u32ModeState = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	uint32_t pu32FileObjCnt = 0;
	MESSAGE_S Msg = {0};
	struct stat filestat;
	PARAM_FILEMNG_S stCfg = {0};

	MODEMNG_GetModeState(&u32ModeState);

	if (u32ModeState != MEDIA_MOVIE_STATE_VIEW) {
		CVI_LOGE("on_startrec_click \n");
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
			cJson_common(hc, result, "set fail.");
			return result;
		}
		usleep(100 * 1000); // 等待停止录像
	}
	OSAL_FS_System("echo 3 > /proc/sys/vm/drop_caches"); // 清除缓存
	NETCTRLINNER_StopTimer();
	PARAM_GetFileMngParam(&stCfg);
	cJSON *cJson_app = cJSON_CreateObject();
	cJSON *cjson_info = cJSON_CreateArray();

	/*
	 * add info
	 */
	for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
		for (int32_t j = 0; j < FILEMNG_DIR_BUTT; j++) {
			if (stCfg.FileMng.dir_param[i].dirname[j][0] != '\0') {
				pu32FileObjCnt = FILEMNG_GetDirFileCnt(i, j);
				cJSON *cjson_dirs = cJSON_CreateObject();
				cJSON *cjson_files = cJSON_CreateArray();
				cJSON_AddStringToObject(cjson_dirs, "folder", stCfg.FileMng.dir_param[i].dirname[j]);
				cJSON_AddNumberToObject(cjson_dirs, "count", pu32FileObjCnt);
				if (j == FILEMNG_DIR_PHOTO) {
					for (uint32_t n = 0; n < pu32FileObjCnt; n++) {
						cJSON *cjson_file = cJSON_CreateObject();
						FILEMNG_GetFileNameByFileInx(i, j, n, &fpathname, 1);
						if (strlen(fpathname) == 0) {
							CVI_LOGE("fpathname is null");
							cJSON_Delete(cJson_app); // 释放内存
							result = WIFIAPP_FALSE;
						}
						cJSON_AddStringToObject(cjson_file, "name", fpathname);

						result = stat(fpathname, &filestat);
						// cJSON_AddNumberToObject(cjson_file, "duration",0);
						cJSON_AddNumberToObject(cjson_file, "size", filestat.st_size / 1024);
						cJSON_AddNumberToObject(cjson_file, "createtime", filestat.st_ctime);
						cJSON_AddNumberToObject(cjson_file, "type", 1);
						cJSON_AddItemToArray(cjson_files, cjson_file);
					}
				} else {
					for (uint32_t n = 0; n < pu32FileObjCnt; n++) {
						cJSON *cjson_file = cJSON_CreateObject();
						FILEMNG_GetFileNameByFileInx(i, j, n, &fpathname, 1);
						if (strlen(fpathname) == 0) {
							CVI_LOGE("fpathname is null");
							result = WIFIAPP_FALSE;
						}
						cJSON_AddStringToObject(cjson_file, "name", fpathname);

						result = stat(fpathname, &filestat);
						cJSON_AddNumberToObject(cjson_file, "size", filestat.st_size / 1024);
						cJSON_AddNumberToObject(cjson_file, "createtime", filestat.st_ctime);
						cJSON_AddNumberToObject(cjson_file, "type", 2);
						cJSON_AddItemToArray(cjson_files, cjson_file);
					}
				}
				cJSON_AddItemToObject(cjson_dirs, "files", cjson_files);
				cJSON_AddItemToArray(cjson_info, cjson_dirs);
			}
		}
	}

	cJSON_AddNumberToObject(cJson_app, "result", result);
	cJSON_AddItemToObject(cJson_app, "info", cjson_info);

	appbuff = cJSON_Print(cJson_app);
	NET_AddResponse(hc, appbuff, strlen(appbuff));
	cJSON_Delete(cJson_app);
	if (appbuff != NULL) {
		free(appbuff);
	}
	NETCTRLINNER_StartTimer();
	return result;
}

// 6.3.2 获取文件列表 http://(IP)/app/getfilelist?folder=loop&start=0&end=99
int32_t APP_GetFileListPage(httpd_conn *hc, char *func, char *param)
{
	flag_file = 1; // 设置标志位，如果异常断联，需要重新扫描文件
	int32_t result = 0;
	if (func == NULL || param == NULL) {
		result = WIFIAPP_FALSE;
		return result;
	}
	char *folder = strrchr(func, '=') + 1;
	uint32_t start = 0, end = 0;
	sscanf(param, "start=%d&end=%d", &start, &end);
	uint32_t u32ModeState = 0;
	uint32_t pu32FileObjCnt = 0;
	MESSAGE_S Msg = {0};
	struct stat filestat;
	char *file_buff = NULL;

	PARAM_FILEMNG_S stCfg = {0};
	MODEMNG_GetModeState(&u32ModeState);
	if (u32ModeState != MEDIA_MOVIE_STATE_VIEW) {
		CVI_LOGE("on_startrec_click \n");
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		if (NETCTRLINNER_SendSyncMsg(&Msg, &result) != 0 || result != 0) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
			cJson_common(hc, result, "set fail.");
			return result;
		}
		usleep(100 * 1000); // 等待停止录像
	}

	OSAL_FS_System("echo 3 > /proc/sys/vm/drop_caches"); // 清除缓存
	NETCTRLINNER_StopTimer();

	PARAM_GetFileMngParam(&stCfg);
	cJSON *cJson_app = cJSON_CreateObject();
	cJSON *cjson_info = cJSON_CreateArray();
	/*
	 * add info
	 */
	cJSON *cjson_dirs = cJSON_CreateObject();
	cJSON *cjson_files = cJSON_CreateArray();
	FILEMNG_DIR_E srcdir = FILEMNG_DIR_BUTT;
	if (strcmp(folder, "emr") == 0) {
		srcdir = FILEMNG_DIR_EMR;
	} else if (strcmp(folder, "loop") == 0) {
		srcdir = FILEMNG_DIR_NORMAL;
	} else if (strcmp(folder, "event") == 0) {
		srcdir = FILEMNG_DIR_PHOTO;
	} else {
		cJSON_Delete(cJson_app);
		result = WIFIAPP_FALSE;
		goto StartTimer;
	}
	cJSON_AddStringToObject(cjson_dirs, "folder", folder);

	uint32_t needCnt = end - start + 1;
	uint32_t totalfileCnt = 0;
	uint32_t uesdCnt = 0;
	for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
		pu32FileObjCnt = FILEMNG_GetDirFileCnt(i, srcdir);
		uesdCnt = needCnt > (pu32FileObjCnt - start) ? (pu32FileObjCnt - start) : needCnt;
		totalfileCnt += uesdCnt;
		needCnt -= uesdCnt;
		char(*fpathname)[FILEMNG_PATH_MAX_LEN] = malloc(uesdCnt * sizeof(*fpathname));
		FILEMNG_GetFileNameByFileInx(i, srcdir, start, fpathname, uesdCnt);
		char filepath[FILEMNG_PATH_MAX_LEN];
    	FILEMNG_GetFilePath(i, srcdir, filepath);
		for (uint32_t n = 0; n < uesdCnt; n++) {
			cJSON *cjson_file = cJSON_CreateObject();
			if (srcdir == FILEMNG_DIR_PHOTO) {
				cJSON_AddNumberToObject(cjson_file, "type", 1);
			} else {
				cJSON_AddNumberToObject(cjson_file, "type", 2);
			}
			cJSON_AddStringToObject(cjson_file, "name", fpathname[n]);
			int  file_length = FILEMNG_PATH_MAX_LEN *2;
			char fullfilepath[file_length];
    		sprintf(fullfilepath, "%s/%s", filepath, fpathname[n]);
			result = stat(fullfilepath, &filestat);
			cJSON_AddNumberToObject(cjson_file, "size", filestat.st_size / 1024);
			cJSON_AddNumberToObject(cjson_file, "createtime", filestat.st_ctime);
			cJSON_AddItemToArray(cjson_files, cjson_file);
		}
		free(fpathname);
	}

	cJSON_AddNumberToObject(cjson_dirs, "count", totalfileCnt);
	cJSON_AddItemToObject(cjson_dirs, "files", cjson_files);
	cJSON_AddItemToArray(cjson_info, cjson_dirs);
	cJSON_AddNumberToObject(cJson_app, "result", result);
	cJSON_AddItemToObject(cJson_app, "info", cjson_info);
	file_buff = cJSON_Print(cJson_app);
	cJSON_Delete(cJson_app);
	NET_AddResponse(hc, file_buff, strlen(file_buff));
	if (file_buff != NULL) {
		free(file_buff);
	}

StartTimer:
	NETCTRLINNER_StartTimer();
	return result;
}

// 6.3.1 获取文件列表 http://(IP)/app/getfilelist?folder=loop&start=0&end=99 获取所有,包含movie,photo
int32_t APP_GetFileList(httpd_conn *hc, char *fun, char *str)
{
	int result = 0;
	if (fun == NULL) {
		result = APP_GetFileListAll(hc, fun, str);
	} else {
		result = APP_GetFileListPage(hc, fun, str);
	}
	return result;
}

// 6.3.3 获取缩略图 http://192.168.169.1/app/getthumbnail?20250416_234421_00_0N.mov
int32_t APP_GetThumbNail(httpd_conn *hc, char *fun, char *str)
{
	int32_t ret = 0;
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
	fun = strrchr(fun, '=') + 1;
	char *dot = strrchr(fun, '.'); // 找到最后一个 '.' 的位置
	FILEMNG_DIR_E srcdir = FILEMNG_DIR_BUTT;
	int32_t cam_id = -1;
	if (dot != NULL && dot - fun >= 2) { // 确保有足够的字符
		char *folder = dot - 1;// 倒数第一个字符
		if (*folder == 'E') {
			srcdir = FILEMNG_DIR_EMR;
		} else if (*folder == 'N') {
			srcdir = FILEMNG_DIR_NORMAL;
		} else if (*folder == 'P') {
			srcdir = FILEMNG_DIR_PHOTO;
		} else {
			ret = WIFIAPP_FALSE;
			return ret;
		}
		cam_id = *(dot - 2) - '0'; // 将字符转换为整数
    } else {
        printf("Invalid fun format.\n");
    }

	char filepath[FILEMNG_PATH_MAX_LEN];
    FILEMNG_GetFilePath(cam_id, srcdir, filepath);
	int  file_length = FILEMNG_PATH_MAX_LEN *2;
    char fullfilepath[file_length];
    sprintf(fullfilepath, "%s/%s", filepath, fun);

	THUMBNAIL_PACKET_S packet = {0};
	THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;
	ret = THUMBNAIL_EXTRACTOR_Create(&viewer_handle);
	ret = THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, fullfilepath, &packet);
	uint32_t size = packet.size;
	if (size == 0) {
		CVI_LOGE("Get Thumbnail faile!\n");
		THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
		return WIFIAPP_FALSE;
	}

	if (0 != ret) {
		THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
		THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
		ret = WIFIAPP_FALSE;
	} else {
		NET_AddResponse(hc, (char *)(packet.data), packet.size);
	}

	THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
	THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
#endif
	return ret;
}

// 6.3.5 文件删除 http://192.168.169.1/app/deletefile?file=emr/video/202012121859.mp4
int32_t APP_DeleteFile(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	fun = strrchr(fun, '=') + 1;
	FILEMNG_DelFile(0, fun);
	cJson_common(hc, result, "delete success.");
	return 0;
}

// 6.4.1 查询当前所有设置项的参数列表 http://(IP)/app/getparamitems
int32_t APP_GetParamItems(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJSON *rec_resolution = cJSON_CreateObject();
	cJSON *rec_split_duration = cJSON_CreateObject();
	cJSON *rec = cJSON_CreateObject();
	cJSON *mic = cJSON_CreateObject();
	cJSON *ev = cJSON_CreateObject();
	cJSON *INFO = cJSON_CreateArray();
	cJSON *res_item = cJSON_CreateArray();
	cJSON *spl_item = cJSON_CreateArray();
	cJSON *recst_item = cJSON_CreateArray();
	cJSON *mic_item = cJSON_CreateArray();
	cJSON *ev_item = cJSON_CreateArray();
	cJSON *res_index = cJSON_CreateArray();
	cJSON *spl_index = cJSON_CreateArray();
	cJSON *recst_index = cJSON_CreateArray();
	cJSON *mic_index = cJSON_CreateArray();
	cJSON *ev_index = cJSON_CreateArray();

	// 分辨率
	cJSON_AddStringToObject(rec_resolution, "name", "rec_resolution");
	cJSON_AddItemToArray(res_item, cJSON_CreateString("1080p"));
	cJSON_AddItemToArray(res_item, cJSON_CreateString("1440p"));
	cJSON_AddItemToObject(rec_resolution, "items", res_item);
	cJSON_AddItemToArray(res_index, cJSON_CreateNumber(1));
	cJSON_AddItemToArray(res_index, cJSON_CreateNumber(0));
	cJSON_AddItemToObject(rec_resolution, "index", res_index);

	// 录像时间
	cJSON_AddStringToObject(rec_split_duration, "name", "rec_split_duration");
	cJSON_AddItemToArray(spl_item, cJSON_CreateString("1 min"));
	cJSON_AddItemToArray(spl_item, cJSON_CreateString("3 min"));
	cJSON_AddItemToArray(spl_item, cJSON_CreateString("5 min"));
	cJSON_AddItemToObject(rec_split_duration, "items", spl_item);
	cJSON_AddItemToArray(spl_index, cJSON_CreateNumber(0));
	cJSON_AddItemToArray(spl_index, cJSON_CreateNumber(1));
	cJSON_AddItemToArray(spl_index, cJSON_CreateNumber(2));
	cJSON_AddItemToObject(rec_split_duration, "index", spl_index);

	// 录像状态
	cJSON_AddStringToObject(rec, "name", "rec");
	cJSON_AddItemToArray(recst_item, cJSON_CreateString("off"));
	cJSON_AddItemToArray(recst_item, cJSON_CreateString("on"));
	cJSON_AddItemToObject(rec, "items", recst_item);
	cJSON_AddItemToArray(recst_index, cJSON_CreateNumber(0));
	cJSON_AddItemToArray(recst_index, cJSON_CreateNumber(1));
	cJSON_AddItemToObject(rec, "index", recst_index);

	// 声音录制开光
	cJSON_AddStringToObject(mic, "name", "mic");
	cJSON_AddItemToArray(mic_item, cJSON_CreateString("off"));
	cJSON_AddItemToArray(mic_item, cJSON_CreateString("on"));
	cJSON_AddItemToObject(mic, "items", mic_item);
	cJSON_AddItemToArray(mic_index, cJSON_CreateNumber(0));
	cJSON_AddItemToArray(mic_index, cJSON_CreateNumber(1));
	cJSON_AddItemToObject(mic, "index", mic_index);

	// 曝光补偿
	cJSON_AddStringToObject(ev, "name", "ev");
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("-2"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("-1.67"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("-1.33"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("-1"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("-0.67"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("-0.33"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("0"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("0.33"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("0.67"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("1"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("1.33"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("1.67"));
	cJSON_AddItemToArray(ev_item, cJSON_CreateString("2"));
	cJSON_AddItemToObject(ev, "items", ev_item);
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(0));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(1));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(2));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(3));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(4));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(5));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(6));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(7));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(8));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(9));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(10));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(11));
	cJSON_AddItemToArray(ev_index, cJSON_CreateNumber(12));
	cJSON_AddItemToObject(ev, "index", ev_index);

	cJSON_AddItemToArray(INFO, rec_resolution);
	cJSON_AddItemToArray(INFO, rec_split_duration);
	cJSON_AddItemToArray(INFO, rec);
	cJSON_AddItemToArray(INFO, mic);
	cJSON_AddItemToArray(INFO, ev);

	cJson_itemcommon(hc, result, INFO);
	return result;
}

// 6.4.2 查询所有设置项的当前值 http://(IP)/app/getparamvalue?param=all
int32_t APP_GetParamValue(httpd_conn *hc, char *fun, char *str)
{
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_CONNECT;
	EVENTHUB_Publish(&stEvent);

	int32_t result = 0;
	PARAM_MENU_S menuparam;
	PARAM_GetMenuParam(&menuparam);

	// 录像状态
	int32_t status = 0;
	uint32_t u32ModeState = 0;
	MODEMNG_GetModeState(&u32ModeState);
	if ((u32ModeState != MEDIA_MOVIE_STATE_REC) &&
		(u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC)) {
		status = 0;
	} else if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
			   (u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
		status = 1;
	}

	cJSON *rec = cJSON_CreateObject();
	cJSON *INFO = cJSON_CreateArray();
	cJSON_AddStringToObject(rec, "name", "rec");
	cJSON_AddNumberToObject(rec, "value", status);
	cJSON_AddItemToArray(INFO, rec);

	if (fun != NULL) {
		fun = strrchr(fun, '=') + 1;
		if (strcmp(fun, "rec") == 0) {
			cJSON_DetachItemFromObject(rec, "name");
			cJson_itemcommon(hc, result, rec);
			return result;
		}
	}

	// 分辨率
	int32_t resolution;
	if (menuparam.VideoSize.Current == 1)
		resolution = 1;
	else
		resolution = 0;

	// 循环播放时间
	int32_t LoopingVideo;
	if (menuparam.VideoLoop.Current == 0)
		LoopingVideo = 0;
	else if (menuparam.VideoLoop.Current == 1)
		LoopingVideo = 1;
	else
		LoopingVideo = 2;

	// 声音录制开关
	int32_t SoundRecord = 0;
	if (menuparam.AudioEnable.Current == 0)
		SoundRecord = 0;
	else
		SoundRecord = 1;

	// 曝光补偿
	int32_t evflage = 0;
	double value = 0;

	MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
	int32_t yuv_status = 0;
	MAPI_VCAP_GetSensorPipeAttr(Syshdl->sns[0], &yuv_status);
	if (0 == yuv_status) {
		CVI_LOGE("stViPipeAttr.bYuvBypassPath is true, yuv sensor skip isp ops");
	} else {
		ISP_EXP_INFO_S expInfo;
		memset(&expInfo, 0, sizeof(ISP_EXP_INFO_S));
		CVI_ISP_QueryExposureInfo(0, &expInfo);
		value = expInfo.u32ExpTime;
	}

	if (EV_Manual == 0) {
		evflage = 6;
		EV_value = value;
	} else {
		double scale = EV_value / value;
		if (scale > 3.5)
			evflage = 0;
		else if (scale >= 2.8 && scale < 3.5)
			evflage = 1;
		else if (scale >= 2.2 && scale < 2.8)
			evflage = 2;
		else if (scale >= 1.8 && scale < 2.2)
			evflage = 3;
		else if (scale >= 1.4 && scale < 1.8)
			evflage = 4;
		else if (scale >= 1.1 && scale < 1.4)
			evflage = 5;
		else if (scale >= 0.9 && scale < 1.1)
			evflage = 6;
		else if (scale >= 0.7 && scale < 0.9)
			evflage = 7;
		else if (scale >= 0.55 && scale < 0.7)
			evflage = 8;
		else if (scale >= 0.45 && scale < 0.55)
			evflage = 9;
		else if (scale >= 0.36 && scale < 0.45)
			evflage = 10;
		else if (scale >= 0.28 && scale < 0.36)
			evflage = 11;
		else
			evflage = 12;
	}

	cJSON *rec_resolution = cJSON_CreateObject();
	cJSON *rec_split_duration = cJSON_CreateObject();
	cJSON *mic = cJSON_CreateObject();
	cJSON *ev = cJSON_CreateObject();

	cJSON_AddStringToObject(rec_resolution, "name", "rec_resolution");
	cJSON_AddNumberToObject(rec_resolution, "value", resolution);
	cJSON_AddStringToObject(rec_split_duration, "name", "rec_split_duration");
	cJSON_AddNumberToObject(rec_split_duration, "value", LoopingVideo);
	cJSON_AddStringToObject(mic, "name", "mic");
	cJSON_AddNumberToObject(mic, "value", SoundRecord);
	cJSON_AddStringToObject(ev, "name", "ev");
	cJSON_AddNumberToObject(ev, "value", evflage);

	cJSON_AddItemToArray(INFO, rec_resolution);
	cJSON_AddItemToArray(INFO, rec_split_duration);
	cJSON_AddItemToArray(INFO, mic);
	cJSON_AddItemToArray(INFO, ev);

	cJson_itemcommon(hc, result, INFO);
	return result;
}

int32_t APP_GetAdasItems(httpd_conn *hc, char *fun, char *str)
{
	int32_t result = 0;
	cJSON *INFO = cJSON_CreateArray();
	cJson_itemcommon(hc, result, INFO);
	return result;
}

static int32_t app_socket_write(int32_t new_fd)
{
	char *sd_appbuff = NULL;
	char *cam_appbuff = NULL;
	int32_t r = 0;
	PARAM_MENU_S param;
	time_t tick;
	tick = time(NULL);
	cJSON *cjson_sd = cJSON_CreateObject();
	cJSON *cjson_sd_info = cJSON_CreateObject();
	cJSON *cjson_cam = cJSON_CreateObject();
	cJSON *cjson_cam_info = cJSON_CreateObject();

	cJSON_AddStringToObject(cjson_sd, "msgid", "sd");
	cJSON_AddNumberToObject(cjson_sd_info, "status", 0);
	cJSON_AddItemToObject(cjson_sd, "info", cjson_sd_info);
	cJSON_AddNumberToObject(cjson_sd, "time", tick);
	sd_appbuff = cJSON_Print(cjson_sd);

	cJSON_AddStringToObject(cjson_cam, "msgid", "cam_plugin");
	cJSON_AddNumberToObject(cjson_cam_info, "action", 0);
	cJSON_AddNumberToObject(cjson_cam_info, "curcamid", 0);
	cJSON_AddNumberToObject(cjson_cam_info, "camnum", MAX_CAMERA_INSTANCES);
	cJSON_AddItemToObject(cjson_cam, "info", cjson_cam_info);
	cJSON_AddNumberToObject(cjson_cam, "time", tick);
	cam_appbuff = cJSON_Print(cjson_cam);

	PARAM_GetMenuParam(&param);
	int32_t CardStatus = MODEMNG_GetCardState();
	;
	int32_t card_status = MODEMNG_GetCardState();
	;
	uint32_t CamStatus = param.ViewWin.Current;
	uint32_t cam_status = param.ViewWin.Current;

	while (flag_socket == WIFI_APP_CONNECTTED) {
		CardStatus = MODEMNG_GetCardState();
		PARAM_GetMenuParam(&param);
		CamStatus = param.ViewWin.Current;
		// 检测SD卡状态
		if (card_status != CardStatus) {
			card_status = CardStatus;
			if (sd_appbuff != NULL) {
				free(sd_appbuff);
			}
			if (card_status == CARD_STATE_AVAILABLE) {
				tick = time(NULL);
				cJSON_ReplaceItemInObject(cjson_sd_info, "status", cJSON_CreateNumber(0));
				cJSON_ReplaceItemInObject(cjson_sd, "time", cJSON_CreateNumber(tick));
				sd_appbuff = cJSON_Print(cjson_sd);
			} else if (card_status == CARD_STATE_UNAVAILABLE) {
				tick = time(NULL);
				cJSON_ReplaceItemInObject(cjson_sd_info, "status", cJSON_CreateNumber(1));
				cJSON_ReplaceItemInObject(cjson_sd, "time", cJSON_CreateNumber(tick));
				sd_appbuff = cJSON_Print(cjson_sd);
			} else {
				tick = time(NULL);
				cJSON_ReplaceItemInObject(cjson_sd_info, "status", cJSON_CreateNumber(2));
				cJSON_ReplaceItemInObject(cjson_sd, "time", cJSON_CreateNumber(tick));
				sd_appbuff = cJSON_Print(cjson_sd);
			}
			CVI_LOGI("sd_appbuff = %s\n", sd_appbuff);
		}
		// 发送sd卡状态, 放置在循环外，也作为保活机制
		r = send(new_fd, sd_appbuff, strlen(sd_appbuff), 0);
		if (r < 0) {
			CVI_LOGI("app socket keep alive fail\n");
		}
		// 检测前后路状态
		if (cam_status != CamStatus) {
			cam_status = CamStatus;
			if (cam_appbuff != NULL) {
				free(cam_appbuff);
			}
			if (((cam_status >> 1) & 0x1) != 1) {
				tick = time(NULL);
				cJSON_ReplaceItemInObject(cjson_cam_info, "camnum", cJSON_CreateNumber(1));
				cJSON_ReplaceItemInObject(cjson_cam_info, "action", cJSON_CreateNumber(0));
				cJSON_ReplaceItemInObject(cjson_cam_info, "curcamid", cJSON_CreateNumber(0));
				cJSON_ReplaceItemInObject(cjson_cam, "time", cJSON_CreateNumber(tick));
				cam_appbuff = cJSON_Print(cjson_cam);
			} else {
				tick = time(NULL);
				cJSON_ReplaceItemInObject(cjson_cam_info, "camnum", cJSON_CreateNumber(2));
				cJSON_ReplaceItemInObject(cjson_cam_info, "action", cJSON_CreateNumber(1));
				if (strcmp(FoRStat, "front") == 0) {
					cJSON_ReplaceItemInObject(cjson_cam_info, "curcamid", cJSON_CreateNumber(0));
				} else {
					cJSON_ReplaceItemInObject(cjson_cam_info, "curcamid", cJSON_CreateNumber(1));
				}
				cJSON_ReplaceItemInObject(cjson_cam, "time", cJSON_CreateNumber(tick));
				cam_appbuff = cJSON_Print(cjson_cam);
			}
			CVI_LOGI("cam_appbuff = %s\n", cam_appbuff);
			// 发送前后路状态
			r = send(new_fd, cam_appbuff, strlen(cam_appbuff), 0);
			if (r < 0) {
				CVI_LOGI("app socket keep alive fail\n");
			}
		}

		usleep(1500 * 1000);
	}

	if (sd_appbuff != NULL) {
		free(sd_appbuff);
	}
	if (cam_appbuff != NULL) {
		free(cam_appbuff);
	}
	cJSON_Delete(cjson_sd);
	cJSON_Delete(cjson_cam);
	return r;
}

static int32_t app_socket_SetKeepAlive(int32_t sockfd)
{
	int32_t s32Ret = 0;
	int32_t keepalive = 1, keepidle = 3, keepinterval = 1, keepcount = 3;
	s32Ret = setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
	if (s32Ret < 0) {
		return s32Ret;
	}

	s32Ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle, sizeof(keepidle));
	if (s32Ret < 0) {
		return s32Ret;
	}

	s32Ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
	if (s32Ret < 0) {
		return s32Ret;
	}

	s32Ret = setsockopt(sockfd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
	if (s32Ret < 0) {
		return s32Ret;
	}

	return s32Ret;
}

void *app_socket(void *arg)
{
	int32_t port = 6035, s32Ret = 0, f = -1;
	struct sockaddr_in s;
	int32_t v = 1;
	int32_t new_fd = 0;
	struct sockaddr_in stRemoteAddr = {0};
	socklen_t socklen = 0;

	/*创建套接字*/
	if ((f = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0)) < 0) {
		CVI_LOGE("socket() error in tcp_listen\n");
		return 0;
	}

	/*设置socket的可选参数 SO_REUSEPORT SO_REUSEADDR*/
	setsockopt(f, SOL_SOCKET, SO_REUSEADDR, (void *)&v, sizeof(int32_t));
	app_socket_SetKeepAlive(f);

	s.sin_family = AF_INET;
	s.sin_addr.s_addr = htonl(INADDR_ANY);
	s.sin_port = htons(port);

	/*绑定socket*/
	if (bind(f, (struct sockaddr *)&s, sizeof(s))) {
		CVI_LOGE("bind() error in tcp_bind with %s\n", strerror(errno));
		return 0;
	}

	/*监听*/
	if (listen(f, 2) < 0) {
		CVI_LOGE("listen() error in tcp_listen.\n");
		return 0;
	}

	/*超时时间设置*/
	fd_set server_fds;
	struct timeval timeout;

	/*接收*/
	while (NETCTRLINNER_APPConnetState() == WIFI_APP_CONNECTTED) {
		CVI_LOGI("really to socket\n");
		FD_ZERO(&server_fds);
		FD_SET(f, &server_fds);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		s32Ret = select(f + 1, &server_fds, NULL, NULL, &timeout);
		if (s32Ret < 0) {
			if (EINTR == errno || EAGAIN == errno) {
				CVI_LOGE(" [select err: %s]\n", strerror(errno));
				continue;
			}
			CVI_LOGE("listen thread: select error=%s\n", strerror(errno));
			break;
		} else if (0 == s32Ret) {
			continue;
		} else {
			if (FD_ISSET(f, &server_fds)) {
				new_fd = accept(f, (void *)&stRemoteAddr, &socklen);
				flag_socket = WIFI_APP_CONNECTTED;
				app_socket_write(new_fd);
				CVI_LOGI("enter close new_fd\n");
				close(new_fd);
			}
		}
	}
	close(f);
	return (void *)0;
}

int32_t custom_register_app(const char *cmd, void *cb)
{
	int ret = 0;
	NET_WIFIAPPMAPTO_APP_S stCgiCmd;

	if (cmd != NULL) {
		snprintf(stCgiCmd.cmd, strlen(cmd) + 1, cmd);
	} else {
		CVI_LOGE("error, cmd is NULL\n");
		ret = -1;
		goto EXIT;
	}
	if (cb == NULL) {
		ret = -1;
		goto EXIT;
	}
	stCgiCmd.callback = (SYSCALL_CMD_TO_APP)cb;
	RegisterCgiCmd_App(&stCgiCmd);

EXIT:
	return ret;
}

void NETCTRLINNER_CMDRegister(void)
{
	CVI_LOGE("enter: %s \n", __func__);
	custom_register_app("getproductinfo", (void *)APP_GetProductInfo);		   // 6.1.1 获取产品信息
	custom_register_app("getdeviceattr", (void *)APP_GetDeviceAttr);		   // 6.1.2 获取设备信息
	custom_register_app("getmediainfo", (void *)APP_GetMediaInfo);			   // 6.1.3 获取媒体信息
	custom_register_app("getsdinfo", (void *)APP_GetSDInfo);				   // 6.1.4 获取SD卡信息
	custom_register_app("setsystime", (void *)APP_SetSysTime);				   // 6.1.6 设置系统时间
	custom_register_app("settimezone", (void *)APP_SetTimeZone);			   // 6.1.7 设置时区
	custom_register_app("getrecduration", (void *)APP_GetRecDuration);		   // 6.1.8 获取当前录像时长
	custom_register_app("setting", (void *)APP_Setting);					   // 6.1.9 设置界面
	custom_register_app("playback", (void *)APP_Playback);					   // 6.1.10 回放界面
	custom_register_app("setparamvalue", (void *)APP_SetParamValue);		   // 6.2   设备参数的设置
	custom_register_app("setwifi", (void *)APP_SetWiFi);					   // 6.2.1-2 修改wifi
	custom_register_app("sdformat", (void *)APP_SDFormat);					   // 6.2.3 格式化SD卡
	custom_register_app("reset", (void *)APP_Reset);						   // 6.2.4 恢复出厂设置
	custom_register_app("getcamerainfo", (void *)APP_GetCameraInfo);		   // 6.2.5 获取摄像头信息
	custom_register_app("snapshot", (void *)APP_SnapShot);					   // 6.2.8 拍照
	custom_register_app("getlockvideostatus", (void *)APP_GetLockVideoStatus); // 6.2.10 查看锁挡状态
	custom_register_app("wifireboot", (void *)APP_WiFiReboot);				   // 6.2.11 重启WiFi
	custom_register_app("getfilelist", (void *)APP_GetFileList);			   // 6.3.1 获取文件列表
	custom_register_app("getthumbnail", (void *)APP_GetThumbNail);			   // 6.3.4 获取缩略图
	custom_register_app("deletefile", (void *)APP_DeleteFile);				   // 6.3.5 删除文件
	custom_register_app("getparamvalue", (void *)APP_GetParamValue);		   // 6.4.2 查询所有设置项的当前值
	custom_register_app("getparamitems", (void *)APP_GetParamItems);		   // 6.4.1 查询当前所有设置项的参数列表
	custom_register_app("getadasitems", (void *)APP_GetAdasItems);
}

int32_t NETCTRLINNER_InitCMDSocket(void)
{
	pthread_mutex_lock(&gMutex);
	flag_socket = WIFI_APP_CONNECTTED;
	if (g_websocket_thread == 0) {
		pthread_create(&g_websocket_thread, NULL, app_socket, NULL);
	}
	pthread_mutex_unlock(&gMutex);
	return 0;
}

int32_t NETCTRLINNER_DeInitCMDSocket(void)
{
	pthread_mutex_lock(&gMutex);
	flag_socket = WIFI_APP_DISCONNECT;
	if (g_websocket_thread != 0) {
		pthread_join(g_websocket_thread, NULL);
		g_websocket_thread = 0;
	}
	pthread_mutex_unlock(&gMutex);
	flag_file = 0; // 复位文件异常退出检测标志
	return 0;
}

int32_t NETCTRLINNER_GetFlagFile(void)
{
	return flag_file;
}
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

// photo mode command
#define WIFIAPP_CMD_CAPTURE 1001
#define WIFIAPP_CMD_CAPTURESIZE 1002
#define WIFIAPP_CMD_FREE_PIC_NUM 1003

// movie mode command
#define WIFIAPP_CMD_RECORD 2001
#define WIFIAPP_CMD_MOVIE_REC_SIZE 2002
#define WIFIAPP_CMD_CYCLIC_REC 2003
#define WIFIAPP_CMD_MOVIE_WDR 2004
#define WIFIAPP_CMD_MOVIE_EV 2005
#define WIFIAPP_CMD_MOTION_DET 2006
#define WIFIAPP_CMD_MOVIE_AUDIO 2007
#define WIFIAPP_CMD_DATEIMPRINT 2008
#define WIFIAPP_CMD_MAX_RECORD_TIME 2009
#define WIFIAPP_CMD_MOVIE_LIVEVIEW_SIZE 2010
#define WIFIAPP_CMD_MOVIE_GSENSOR_SENS 2011
#define WIFIAPP_CMD_SET_AUTO_RECORDING 2012
#define WIFIAPP_CMD_MOVIE_REC_BITRATE 2013
#define WIFIAPP_CMD_MOVIE_LIVEVIEW_BITRATE 2014
#define WIFIAPP_CMD_MOVIE_LIVEVIEW_START 2015
#define WIFIAPP_CMD_MOVIE_RECORDING_TIME 2016
#define WIFIAPP_CMD_MOVIE_REC_TRIGGER_RAWENC 2017
#define WIFIAPP_CMD_MOVIE_GET_RAWENC_JPG 2018
#define WIFIAPP_CMD_MOVIE_GET_LIVEVIEW_FMT 2019
#define WIFIAPP_CMD_MOVIE_CONTRAST 2020
#define WIFIAPP_CMD_TWOWAY_AUDIO 2021
#define WIFIAPP_CMD_TWOWAY_AUDIO_SAMPLERATE 2022
#define WIFIAPP_CMD_FLIP_MIRROR 2023
#define WIFIAPP_CMD_QUALITYSET 2024
#define WIFIAPP_CMD_MOVIE_BRC_ADJUST 2025
#define WIFIAPP_CMD_MODECHANGE 3001
#define WIFIAPP_CMD_QUERY 3002
#define WIFIAPP_CMD_SET_SSID 3003
#define WIFIAPP_CMD_SET_PASSPHRASE 3004
#define WIFIAPP_CMD_SET_DATE 3005
#define WIFIAPP_CMD_SET_TIME 3006
#define WIFIAPP_CMD_POWEROFF 3007
#define WIFIAPP_CMD_LANGUAGE 3008
#define WIFIAPP_CMD_TVFORMAT 3009
#define WIFIAPP_CMD_FORMAT 3010
#define WIFIAPP_CMD_SYSRESET 3011
#define WIFIAPP_CMD_VERSION 3012
#define WIFIAPP_CMD_FWUPDATE 3013
#define WIFIAPP_CMD_QUERY_CUR_STATUS 3014
#define WIFIAPP_CMD_FILELIST 3015
#define WIFIAPP_CMD_HEARTBEAT 3016
#define WIFIAPP_CMD_DISK_FREE_SPACE 3017
#define WIFIAPP_CMD_RECONNECT_WIFI 3018
#define WIFIAPP_CMD_GET_BATTERY 3019
#define WIFIAPP_CMD_NOTIFY_STATUS 3020
#define WIFIAPP_CMD_SAVE_MENUINFO 3021
#define WIFIAPP_CMD_GET_HW_CAP 3022
#define WIFIAPP_CMD_REMOVE_USER 3023
#define WIFIAPP_CMD_GET_CARD_STATUS 3024
#define WIFIAPP_CMD_GET_DOWNLOAD_URL 3025
#define WIFIAPP_CMD_GET_UPDATEFW_PATH 3026
#define WIFIAPP_CMD_UPLOAD_FILE 3027
#define WIFIAPP_CMD_SET_PIP_STYLE 3028
#define WIFIAPP_CMD_GET_SSID_PASSPHRASE 3029
#define WIFIAPP_CMD_QUERY_MOVIE_SIZE 3030
#define WIFIAPP_CMD_QUERY_MENUITEM 3031
#define WIFIAPP_CMD_SEND_SSID_PASSPHRASE 3032
#define WIFIAPP_CMD_SET_WIFI_CONNECT_MODE 3033
#define WIFIAPP_CMD_AUTO_TEST_CMD_DONE 3034
#define WIFIAPP_CMD_APP_STARTUP 3035
#define WIFIAPP_CMD_APP_SESSION_CLOSE 3036
#define WIFIAPP_CMD_GET_MODE_STAUTS 3037
#define WIFIAPP_CMD_WIFIAP_SEARCH 3038
#define WIFIAPP_CMD_THUMB 4001
#define WIFIAPP_CMD_SCREEN 4002
#define WIFIAPP_CMD_DELETE_ONE 4003
#define WIFIAPP_CMD_DELETE_ALL 4004
#define WIFIAPP_CMD_MOVIE_FILE_INFO 4005
#define WIFIAPP_CMD_UPLOAD 5001
#define WIFIAPP_CMD_UPLOAD_AUDIO 5002
#define WIFIAPP_CMD_PARK_MONITOR 8050
#define WIFIAPP_CMD_LASPE_TIME 8051
#define WIFIAPP_CMD_ROADCAM_START 8567
#define WIFIAPP_CMD_PHONE_APP 9090
#define WIFIAPP_CMD_SENSOR_NUM 9095

#define DEF_XML_HEAD "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n"
#define DEF_XML_STATE "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n<Function>\r\n<Cmd>3020</Cmd>\r\n<Status>%d</Status>\r\n</Function>"
#define DEF_XML_RET "<Function>\r\n<Cmd>%d</Cmd>\r\n<Status>%d</Status>\r\n</Function>"
#define DEF_XML_STR "<Function>\r\n<Cmd>%d</Cmd>\r\n<Status>%d</Status>\r\n<String>Width:%d,  Height:%d,  Length:%.f  sec</String>\r\n</Function>"
#define DEF_XML_VALUE "<Function>\r\n<Cmd>%d</Cmd>\r\n<Status>%d</Status>\r\n<Value>%ld</Value>\r\n</Function>"
#define DEF_XML_VERSION "<Function>\r\n<Cmd>%d</Cmd>\r\n<Status>%d</Status>\r\n<String>%s</String>\r\n</Function>"
#define DEF_XML_CMD_CUR_STS "<Cmd>%d</Cmd>\r\n<Status>%d</Status>\r\n"
#define DEF_XML_TEST "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n<Function>\r\n<Cmd>2002</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>2003</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>2004</Cmd>\r\n<Status>0</Status>\r\n<Cmd>2005</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>2006</Cmd>\r\n<Status>0</Status>\r\n<Cmd>2007</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>2008</Cmd>\r\n<Status>%d</Status>\r\n"
#define DEF_XML_TEST2 "<Cmd>2009</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>2010</Cmd>\r\n<Status>3</Status>\r\n<Cmd>2011</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>2012</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>3007</Cmd>\r\n<Status>0</Status>\r\n<Cmd>3008</Cmd>\r\n<Status>%d</Status>\r\n<Cmd>3009</Cmd>\r\n<Status>1</Status>\r\n<Cmd>3028</Cmd>\r\n<Status>%d</Status>\r\n</Function>"
#define DEF_XML_GETStREAM "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n<LIST>\r\n<MovieLiveViewLink>rtsp://192.168.1.254/%s</MovieLiveViewLink>\r\n</LIST>\r\n"
#define DEF_XML_GETMOVECAP "<Item>\r\n<Name>%s</Name>\r\n<Index>%d</Index>\r\n<Size>%d*%d</Size>\r\n<FrameRate>%d</FrameRate>\r\n<Type>%d</Type>\r\n</Item>\r\n"
#define DEF_XML_VALUE_LONG "<Function>\r\n<Cmd>%d</Cmd>\r\n<Status>%d</Status>\r\n<Value>%llu</Value>\r\n</Function>"
#define DEF_XML_ENABLE "<List>\r\n<SSID>%s</SSID>\r\n<PASSPHRASE>%s</PASSPHRASE>\r\n</List>"
#define DEF_XML_LISTAL_FILE "<ALLFile>\r\n<File>\r\n<NAME>%s</NAME>\r\n<FPATH>%s</FPATH>\r\n<SIZE>%lld</SIZE>\r\n<TIMECODE>%ld</TIMECODE>\r\n<TIME>%s</TIME>\r\n<ATTR>%d</ATTR>\r\n</File>\r\n</ALLFile>\r\n"
#define DEF_XML_QUER_LIST "<Option>\r\n<Index>%d</Index>\r\n<Id>%s</Id>\r\n</Option>\r\n"
#define DEF_XML_CMD_CUR_STS_TEST "<Cmd>%s</Cmd>\r\n<Status>%s</Status>\r\n"

static int32_t flag_socket = WIFI_APP_DISCONNECT;
static pthread_t g_websocket_thread;
static char *xmlbuff_file;
static int32_t file_size = 10 * 256;
static int32_t flag_file = 0;
static int32_t cam_status = 0;
static double EV_value = -1;
static int32_t EV_Manual = 0;
static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
extern NET_WIFIAPPMAPTO_S g_custom_list[CMDKEYWORDSIZE];

unsigned long XML_Snprintf(char *xmlbuff, char *buf, int32_t size, const char *fmt, ...)
{
	unsigned long len = 0;
	va_list marker;
	va_start(marker, fmt);
	len = vsnprintf(buf, size, fmt, marker);
	strcat(xmlbuff, buf);
	va_end(marker);

	return len;
}

/* This is a generic XML template.
** Format is:
** <?xml version=\"1.0\" encoding=\"UTF-8\" ?>
** <Function>
** <Cmd>cmd</Cmd>
** <Status>result</Status>
** </Function>
*/
void XML_Currency(httpd_conn *hc, int32_t cmd, int32_t result)
{
	char *buf = NULL;
	char xmlbuff[LBUFFSIZE];

	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, sizeof("<Function>\r\n"), "<Function>\r\n");
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_CMD_CUR_STS, cmd, result);
	XML_Snprintf(xmlbuff, buf, sizeof("</Function>\r\n"), "</Function>\r\n");
	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}
}

/* This is a generic XML template.
** Format is:
** <?xml version=\"1.0\" encoding=\"UTF-8\" ?>
** <Function>
** <Cmd>cmd</Cmd>
** <Status>result</Status>
** <Value>FreePicNum</Value>
** </Function>
*/
void XML_Valuecurrency(httpd_conn *hc, int32_t cmd, int32_t result, unsigned long FreePicNum)
{
	char *buf = NULL;
	char xmlbuff[LBUFFSIZE];

	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	if (0 == result) {
		memset(xmlbuff, 0, sizeof(xmlbuff));
		XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
		XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_VALUE, cmd, result, FreePicNum);
		NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);
	}

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}
}

void XML_Longvaluecurrency(httpd_conn *hc, int32_t cmd, int32_t result, unsigned long long FreePicNum)
{
	char *buf = NULL;
	char xmlbuff[LBUFFSIZE];

	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	if (0 == result) {
		memset(xmlbuff, 0, sizeof(xmlbuff));
		XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
		XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_VALUE_LONG, cmd, result, FreePicNum);
		NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);
	}

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}
}

// Command: http://192.168.1.254/?custom=1&cmd=1001
int32_t XML_GetPicture(httpd_conn *hc, char *str)
{
	char *buf = NULL;
	char xmlbuff[XBUFFSIZE];
	int32_t result = 0;
	int32_t FreePicNum = 0;
	CVI_DTCF_DIR_E Dirs = 12;
	uint32_t FileObjCnt = 0;
	char fpathname[255];
	char *filename = NULL;
	int32_t ret = 0;
	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	ret = FILEMNG_SetSearchScope(&Dirs, 1, &FileObjCnt);
	if (0 == ret) {
		ret = FILEMNG_GetFileByIndex(0, fpathname, 255);
		if (0 == ret) {
			if (NULL == fpathname) {
				CVI_LOGE("cmd 1001 fpathname is null");
				result = WIFIAPP_FALSE;
			} else {
				filename = (strrchr(fpathname, '/') + 1);
				FreePicNum = FileObjCnt;
				if (NULL == filename) {
					CVI_LOGE("cmd 1001 filename is null");
					result = WIFIAPP_FALSE;
				}
			}
		}
	} else {
		result = WIFIAPP_FALSE;
	}

	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, sizeof("<Function>\r\n"), "<Function>\r\n");
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_CMD_CUR_STS, WIFIAPP_CMD_CAPTURE, result);
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<File>\r\n<NAME>%s</NAME>\r\n<FPATH>%s</FPATH>\r\n</File>\r\n", filename, fpathname);
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<FREEPICNUM>%d</FREEPICNUM>\r\n", FreePicNum);
	XML_Snprintf(xmlbuff, buf, sizeof("</Function>\r\n"), "</Function>\r\n");

	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=1002&par=0
int32_t XML_SetCapture(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	sscanf(str, "%d", &pstr);
	uint32_t videosize = 0;
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	PARAM_GetVideoSizeEnum(param.VideoSize.Current, &videosize);
	MESSAGE_S Msg = {0};
	if (0 == pstr) {
		if (MEDIA_VIDEO_SIZE_1920X1080P25 == videosize) {
			Msg.topic = EVENT_MODEMNG_SETTING;
			Msg.arg1 = PARAM_MENU_VIDEO_SIZE;
			Msg.arg2 = MEDIA_VIDEO_SIZE_2560X1440P25;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
			result = WIFIAPP_OK;
		}
	} else if (1 == pstr) {
		if (MEDIA_VIDEO_SIZE_2560X1440P25 == videosize) {
			Msg.topic = EVENT_MODEMNG_SETTING;
			Msg.arg1 = PARAM_MENU_VIDEO_SIZE;
			Msg.arg2 = MEDIA_VIDEO_SIZE_1920X1080P25;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
			result = WIFIAPP_OK;
		}
	} else {
		result = WIFIAPP_FALSE;
	}

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}
	XML_Currency(hc, WIFIAPP_CMD_CAPTURESIZE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=1003
int32_t XML_GetFreePicture(httpd_conn *hc, char *str)
{
	unsigned long FreePicNum = 100;
	int32_t result = 0;

	XML_Valuecurrency(hc, WIFIAPP_CMD_FREE_PIC_NUM, result, FreePicNum);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2001&par=1
int32_t XML_StartandStopMovieRecord(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	uint32_t u32ModeState = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	sscanf(str, "%d", &pstr);
	MESSAGE_S Msg = {0};
	MODEMNG_GetModeState(&u32ModeState);

	if ((0 != pstr) && (1 != pstr)) {
		CVI_LOGE("Startandstopmovierecord error, par parameter error\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_RECORD, result);

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
	}

	XML_Currency(hc, WIFIAPP_CMD_RECORD, result);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2002&par=1
int32_t XML_SetMovieRecord(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	sscanf(str, "%d", &pstr);
	if (pstr < 0 || pstr > 11) {
		result = WIFIAPP_FALSE;
	} else {
		Msg.topic = EVENT_MODEMNG_SETTING;
		Msg.arg1 = PARAM_MENU_VIDEO_SIZE;
		Msg.arg2 = pstr;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
		} else {
			NETCTRLINNER_UiUpdate();
		}
	}
	cam_status = 0;
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_REC_SIZE, result);
	sleep(1); // 切分辨率需要初始化系统，耗时较长，等待1s是为了等待数据写入缓存，防止APP快速操作引起数据混乱
	return result;
}

// Command : http://192.168.1.254/?custom=1&cmd=2003&par=1
int32_t XML_SetCyclicRecordValue(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_VIDEO_LOOP;
	sscanf(str, "%d", &pstr);
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	if ((0 != pstr) && (1 != pstr) && (2 != pstr)) {
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_CYCLIC_REC, result);

		return result;
	}

	if (param.VideoLoop.Current != (uint32_t)pstr) {
		Msg.arg2 = pstr;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
		} else {
			NETCTRLINNER_UiUpdate();
		}
	}

	XML_Currency(hc, WIFIAPP_CMD_CYCLIC_REC, result);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2004&par=0
int32_t XML_EdisableMoviehdr(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);
	if (0 == pstr) {
		// stop
		result = WIFIAPP_OK;
	} else if (1 == pstr) {
		// up
		result = WIFIAPP_OK;
	} else {
		CVI_LOGE("EdisableMoviehdr error, par parameter error\n");
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_MOVIE_WDR, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2005&par=0
int32_t XML_SetMovieEvValue(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);
	int32_t value = 0;
	if (pstr == 12)
		value = EV_value / 3.98;
	else if (pstr == 11)
		value = EV_value / 3.16;
	else if (pstr == 10)
		value = EV_value / 2.5;
	else if (pstr == 9)
		value = EV_value / 1.99;
	else if (pstr == 8)
		value = EV_value / 1.58;
	else if (pstr == 7)
		value = EV_value / 1.26;
	else if (pstr == 6)
		value = EV_value;
	else if (pstr == 5)
		value = EV_value * 1.26;
	else if (pstr == 4)
		value = EV_value * 1.58;
	else if (pstr == 3)
		value = EV_value * 1.99;
	else if (pstr == 2)
		value = EV_value * 2.5;
	else if (pstr == 1)
		value = EV_value * 3.16;
	else if (pstr == 0)
		value = EV_value * 3.98;
	else {
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_MOVIE_EV, result);
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

	XML_Currency(hc, WIFIAPP_CMD_MOVIE_EV, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2006&par=1
int32_t XML_EdisableMotionDetection(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);
	if (0 == pstr) {
		// stop
		result = WIFIAPP_OK;
	} else if (1 == pstr) {
		// up
		result = WIFIAPP_OK;
	} else {
		CVI_LOGE("EdisableMotiondetection error, par parameter error\n");
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_MOTION_DET, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2007&par=1
int32_t XML_EdisableMovieAudio(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
	PARAM_MENU_S menuparam;
	PARAM_GetMenuParam(&menuparam);

	if ((0 != pstr) && (1 != pstr)) {
		CVI_LOGE("EdisableMotiondetection error, par parameter error\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_MOVIE_AUDIO, result);
		return result;
	}

	if (menuparam.AudioEnable.Current != (uint32_t)pstr) {
		Msg.arg2 = pstr;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
		} else {
			NETCTRLINNER_UiUpdate();
		}
	}

	XML_Currency(hc, WIFIAPP_CMD_MOVIE_AUDIO, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2008&par=1
int32_t XML_EdisableDateInprint(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	sscanf(str, "%d", &pstr);

	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);

	if ((0 != pstr) && (1 != pstr)) {
		CVI_LOGE("XML_Edisabledateinprint error, par parameter error\n");
		result = WIFIAPP_FALSE;
	} else if (param.Osd.Current == (uint32_t)pstr) {
		result = WIFIAPP_OK;
	} else {
		Msg.topic = EVENT_MODEMNG_SETTING;
		Msg.arg1 = PARAM_MENU_OSD_STATUS;
		Msg.arg2 = pstr;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		result = sendresult;
	}

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_DATEIMPRINT, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2009
int32_t XML_GetMovieMaxRecordTime(httpd_conn *hc)
{
	int32_t result = 0;
	unsigned long FreePicNum = 0;
	PARAM_MEDIA_COMM_S param;
	PARAM_GetMediaComm(&param);
	PARAM_RECORD_CHN_ATTR_S *recattr = &param.Record.ChnAttrs[0];
	if (NULL == recattr) {
		result = WIFIAPP_FALSE;
	} else {
		FreePicNum = (recattr->SplitTime) / 1000;
	}
	XML_Valuecurrency(hc, WIFIAPP_CMD_MAX_RECORD_TIME, result, FreePicNum);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2010&par=2
int32_t XML_ChangeLiveviewSize(httpd_conn *hc, char *str)
{
	unsigned long result = 0;
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_LIVEVIEW_SIZE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2011&par=2
int32_t XML_EDisableMovieGSensorSensitivity(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	sscanf(str, "%d", &pstr);
	if ((pstr < 0) || (pstr > 3)) {
		CVI_LOGE("EDisablemovieGSensorsensitivity parm error, par = %d\n", pstr);
		result = WIFIAPP_FALSE;
	} else {
		Msg.topic = EVENT_MODEMNG_SETTING;
		Msg.arg1 = PARAM_MENU_GSENSOR;
		Msg.arg2 = pstr;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
		} else {
			NETCTRLINNER_UiUpdate();
		}
	}

	XML_Currency(hc, WIFIAPP_CMD_MOVIE_GSENSOR_SENS, result);

	return result;
}

// Command:http://192.168.1.254/?custom=1&cmd=2012&par=1
int32_t XML_SetAutoRecording(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);

	PARAM_MEDIA_COMM_S param;
	PARAM_GetMediaComm(&param);

	if (0 == pstr) {
		if (true == param.Record.ChnAttrs[0].Enable) {
			param.Record.ChnAttrs[0].Enable = false;
		}
	} else if (1 == pstr) {
		if (false == param.Record.ChnAttrs[0].Enable) {
			param.Record.ChnAttrs[0].Enable = true;
		}
	} else {
		CVI_LOGE("XML_Edisabledateinprint error, par parameter error\n");
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_SET_AUTO_RECORDING, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2013&str=400
int32_t XML_SetMovieRecordBitrate(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_REC_BITRATE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2014&str=300
int32_t XML_SetMovieLiveViewBitrate(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_LIVEVIEW_BITRATE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2015&par=1
int32_t XML_StartandStopMovieLiveview(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	char rtspname[128] = APP_RTSP_NAME;
	int s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);
	if (pstr == 1) {
		Msg.topic = EVENT_MODEMNG_RTSP_INIT;
		Msg.arg1 = (int)MAX_CAMERA_INSTANCES;
		memcpy(Msg.aszPayload, rtspname, 128);
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		cam_status = 0;
	} else if (pstr == 0) {
		Msg.topic = EVENT_MODEMNG_RTSP_DEINIT;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	}
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_LIVEVIEW_START, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2016
// int32_t XML_GetMovieRecordingTime(httpd_conn *hc, char *str)
// {
// int32_t result = 0;
// uint32_t u32ModeState = 0;
// int32_t value = 0;
// MODEMNG_GetModeState(&u32ModeState);
// if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
//     (u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
//     value = 1;
// }
// XML_Valuecurrency(hc, WIFIAPP_CMD_MOVIE_RECORDING_TIME, result, value);
// return result;
// }

// Command: http://192.168.1.254/?custom=1&cmd=2017
int32_t XML_TriggerRAWencode(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	MESSAGE_S Msg = {0};
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	Msg.topic = EVENT_MODEMNG_START_PIV;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_REC_TRIGGER_RAWENC, result);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2018
int32_t XML_GetRAWencodeJPEG(httpd_conn *hc, char *str)
{
	int32_t ret = 0;
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
	CVI_DTCF_DIR_E Dirs = 12;
	uint32_t FileObjCnt = 0;
	THUMBNAIL_PACKET_S packet = {0};
	THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;
	char fpathname[255];
	FILEMNG_SetSearchScope(&Dirs, 1, &FileObjCnt);
	FILEMNG_GetFileByIndex(0, fpathname, 255);
	if (NULL == fpathname) {
		CVI_LOGE("cmd 2018 fpathname is null");
		ret = WIFIAPP_FALSE;
	} else {
		ret = THUMBNAIL_EXTRACTOR_Create(&viewer_handle);
		ret = THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, fpathname, &packet);
		uint32_t size = packet.size;
		if (size == 0) {
			CVI_LOGE("GetRAWencodeJPEG Get Thumbnail faile!\n");
			THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
			XML_Currency(hc, WIFIAPP_CMD_THUMB, ret);
			ret = WIFIAPP_FALSE;
		}
	}

	if (WIFIAPP_OK != ret) {
		XML_Currency(hc, WIFIAPP_CMD_THUMB, ret);
	} else {
		NET_AddResponse(hc, (char *)(packet.data), packet.size);
	}

	if (NULL != fpathname) {
		THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
		THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
	}
#endif

	return ret;
}

// Command: http://192.168.1.254/?custom=1&cmd=2019
int32_t XML_GetStreamingAddr(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	char *buf = NULL;
	char xmlbuff[LBUFFSIZE];

	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}
	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_GETStREAM) + sizeof(APP_RTSP_NAME), DEF_XML_GETStREAM, APP_RTSP_NAME);
	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2023&par=0
int32_t XML_SetMovieFliphorMirror(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	int32_t pstr = 0;
	uint32_t u32ModeState = 0;
	MESSAGE_S Msg = {0};
	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);

	if ((0 == pstr) || (1 == pstr)) {
		MODEMNG_GetModeState(&u32ModeState);
		if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
			(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
			Msg.topic = EVENT_MODEMNG_STOP_REC;
			Msg.arg1 = MEDIA_MOVIE_STATE_MENU;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		}

		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("stop rec send message fail");
			result = WIFIAPP_FALSE;
		} else {
			if (param.CamMirror.Current != (uint32_t)pstr) {
				Msg.topic = EVENT_MODEMNG_SETTING;
				Msg.arg1 = PARAM_MENU_REARCAM_MIRROR;
				Msg.arg2 = (uint32_t)pstr;
				sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

				if (0 != sendresult || 0 != s32Result) {
					CVI_LOGE("set mirror send message fail");
					result = WIFIAPP_FALSE;
				}
			}
		}
	} else {
		CVI_LOGE("SetmovieFliphorMirror fail, str = %d", pstr);
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_FLIP_MIRROR, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=2025&par=8000
int32_t XML_MovieBitRateAdjust(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_MOVIE_BRC_ADJUST, result);

	return result;
}

/* Command: http://192.168.1.254/?custom=1&cmd=3001&par=2
** You do not need to do the specific action of switching mode through the command
** and success is returned directly.
*/
int32_t XML_ChangeSystemMode(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	XML_Currency(hc, WIFIAPP_CMD_MODECHANGE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3002
int32_t XML_QueryCurrentSupportCommand(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	char *buf = NULL;
	int32_t size = sizeof(DEF_XML_HEAD) + sizeof("<Function>\r\n") * 2 + sizeof("<Cmd>1000</Cmd>\r\n") * CMDKEYWORDSIZE;
	int32_t result = 0;
	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}
	char xmlbuff[size];
	memset(xmlbuff, 0, size);
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, sizeof("<Function>\r\n"), "<Function>\r\n");
	for (int32_t i = 0; i < CMDKEYWORDSIZE; i++) {
		if (g_custom_list[i].cmd != 0) {
			XML_Snprintf(xmlbuff, buf, sizeof("<Cmd>1000</Cmd>\r\n"), "<Cmd>%d</Cmd>\r\n", g_custom_list[i].cmd);
		}
		result = WIFIAPP_FALSE;
	}
	XML_Snprintf(xmlbuff, buf, sizeof("</Function>"), "</Function>");
	NET_AddResponse(hc, xmlbuff, size);
	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command: http://192.168.1.254/ ?custom=1&cmd=3003&str=ABCDE
int32_t XML_SetWiFiSSID(httpd_conn *hc, char *str)
{
	int32_t s32Ret = 0;
	int32_t result = 0;
	PARAM_WIFI_S wifipar;
	s32Ret = PARAM_GetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("Api cmd PARAM_GetWifiParam faile\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_SSID, result);
		return result;
	}

	if ((strlen(str) - 1) >= NET_WIFI_SSID_LEN) {
		CVI_LOGE("WiFiSSID to long, Not added mac addr\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_SSID, result);
		return result;
	}

	memcpy(wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID, str, HAL_WIFI_SSID_LEN);

	s32Ret = PARAM_SetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("SetWiFiSSID PARAM_SetWifiParam fail\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_SSID, result);
		return result;
	}
	XML_Currency(hc, WIFIAPP_CMD_SET_SSID, result);

	return result;
}

// Command: http://192.168.1.254/ ?custom=1&cmd=3004&str=876543
int32_t XML_SetWiFiPassPhrase(httpd_conn *hc, char *str)
{
	int32_t s32Ret = 0;
	int32_t result = 0;
	PARAM_WIFI_S wifipar;
	s32Ret = PARAM_GetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("Api passphrase cmd PARAM_GetWifiParam faile\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_PASSPHRASE, result);
		return result;
	}

	if ((strlen(str) - 1) >= NET_WIFI_PASS_LEN) {
		CVI_LOGE("wifipassphrase to long, Not added mac addr\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_PASSPHRASE, result);
		return result;
	}

	memcpy(wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord, str, HAL_WIFI_PASSWORD_LEN);

	s32Ret = PARAM_SetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("SetWiFipassphrase PARAM_SetWifiParam fail\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_PASSPHRASE, result);
		return result;
	}

	XML_Currency(hc, WIFIAPP_CMD_SET_PASSPHRASE, result);

	return result;
}

// Command: http://192.168.1.254/ ?custom=1&cmd=3005&str=2014-03-21
int32_t XML_SetSystemDate(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	char sExecStr[LBUFFSIZE];
	char datebuf[LBUFFSIZE] = "date -s ";
	memset(sExecStr, 0, sizeof(sExecStr));
	strcat(datebuf, str);
	sprintf(sExecStr, "%s", datebuf);
	OSAL_FS_System(sExecStr);

	XML_Currency(hc, WIFIAPP_CMD_SET_DATE, result);

	return result;
}

// Command: http://192.168.1.254/? custom=1&cmd=3006&str=17:10:30
int32_t XML_SetSystemTime(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	char sExecStr[LBUFFSIZE];
	char datebuf[LBUFFSIZE] = "date -s ";
	memset(sExecStr, 0, sizeof(sExecStr));
	strcat(datebuf, str);
	sprintf(sExecStr, "%s", datebuf);
	OSAL_FS_System(sExecStr);

	XML_Currency(hc, WIFIAPP_CMD_SET_TIME, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3007&par=1
int32_t XML_SetSystemAutoPowerOffTime(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_POWEROFF, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3008&par=1
int32_t XML_SelectLanguage(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	sscanf(str, "%d", &pstr);
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_LANGUAGE;
	Msg.arg2 = pstr;

	if ((0 != pstr) && (1 != pstr)) {
		s32Result = 0;
		CVI_LOGE("str or par fail, not equal to 0 or 1\n");
		XML_Currency(hc, WIFIAPP_CMD_LANGUAGE, result);

		return result;
	}

	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_LANGUAGE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3009&par=1
int32_t XML_SetTVformat(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_TVFORMAT, result);

	return result;
}

/* Currently, formatting flash memory is not supported, only SD car */
// Command: http://192.168.1.254/?custom=1&cmd=3010&par=0
int32_t XML_CommandWouldFormatStorage(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t s32Ret = 0;
	int32_t pstr = 0;
	uint32_t u32ModeState = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	int32_t count = 0;
	MESSAGE_S Msg = {0};
	sscanf(str, "%d", &pstr);

	if ((0 != pstr) && (1 != pstr)) {
		CVI_LOGE("commandwouldformatstorage error, par parameter error\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_FORMAT, result);

		return result;
	}

	MODEMNG_GetModeState(&u32ModeState);
	if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
		(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
			XML_Currency(hc, WIFIAPP_CMD_FORMAT, result);
			return result;
		}
		usleep(100 * 1000);
	}
	NETCTRLINNER_StopTimer();
	NETCTRLINNER_SetSdState(WIFI_SD_FORMAT_INIT);
	if (0 == pstr) {
		result = WIFIAPP_FALSE;
	} else if (1 == pstr) {
		Msg.topic = EVENT_MODEMNG_CARD_FORMAT;
		// lable
		s32Ret = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != s32Ret) {
			CVI_LOGE("MODEMNG_Format failed\n");
			result = WIFIAPP_FALSE;
		} else {
			while (NETCTRLINNER_GetSdState() && count++ < 20) {
				usleep(100 * 1000);
			}
			if (count >= 20 || NETCTRLINNER_GetSdState() == WIFI_SD_FORMAT_FAIL) {
				result = WIFIAPP_FALSE;
			}
		}
	}

	XML_Currency(hc, WIFIAPP_CMD_FORMAT, result);
	NETCTRLINNER_StartTimer();
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3011
int32_t XML_ResetallSettingTodeFault(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	int32_t sendresult = 0;
	int32_t s32Result = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_DEFAULT;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("netctrl MODEMNG_SetDefaultParam failed\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SYSRESET, result);
		return result;
	}
	cam_status = 0;
	XML_Currency(hc, WIFIAPP_CMD_SYSRESET, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3012
int32_t XML_GetProjectVersion(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	char *buf = NULL;
	char xmlbuff[LBUFFSIZE];
	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}
	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_VERSION, WIFIAPP_CMD_VERSION, result, VERSION_NUM);
	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3013&str=generic_0.1.8&1
// 默认为Flash升级，添加&1改为SD升级
int32_t XML_StartToupdateFirmware(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	if (NULL == str) {
		CVI_LOGE("Parameter cannot be empty");
		result = -1;
		XML_Currency(hc, WIFIAPP_CMD_FWUPDATE, result);
		return result;
	}
	char version[128] = {0};
	int32_t pstr = 0;
	MESSAGE_S Msg = {0};
	int32_t s32Result = 0;
	int32_t sendresult = 0;

	if (NULL != strchr(str, '&')) {
		sscanf(strchr(str, '&') + 1, "%d", &pstr);
		memcpy(version, str, strchr(str, '&') - str);
	} else {
		memcpy(version, str, strlen(str) + 1);
	}

	Msg.topic = EVENT_MODEMNG_MODESWITCH;
	Msg.arg1 = WORK_MODE_UPDATE;
	memcpy(Msg.aszPayload, version, 128);
	Msg.arg2 = pstr;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_FWUPDATE, result);
		return result;
	}
	Msg.topic = EVENT_MODEMNG_START_UPFILE;
	memcpy(Msg.aszPayload, version, 128);
	Msg.arg2 = pstr;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_FWUPDATE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3014
int32_t XML_QueryaAllSettingCommandStatus(httpd_conn *hc, char *str)
{
	char *buf = NULL;
	int32_t vflage = false;
	int32_t evflage = true;
	int32_t lflage = true;
	int32_t ltflage = false;
	int32_t rflage = true;
	int32_t recflage = false;
	uint32_t aflage = false;
	uint32_t osdflage = false;
	int32_t FreePicNum;
	int32_t result = 0;
	double value = 0;
	if (NULL == buf) {
		buf = (char *)malloc(OBUFFSIZE);
	}
	char xmlbuff[OBUFFSIZE];
	memset(xmlbuff, 0, OBUFFSIZE);

	MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
	int32_t status = 0;
	MAPI_VCAP_GetSensorPipeAttr(Syshdl->sns[0], &status);
	if (0 == status) {
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
			evflage = 12;
		else if (scale >= 2.8 && scale < 3.5)
			evflage = 11;
		else if (scale >= 2.2 && scale < 2.8)
			evflage = 10;
		else if (scale >= 1.8 && scale < 2.2)
			evflage = 9;
		else if (scale >= 1.4 && scale < 1.8)
			evflage = 8;
		else if (scale >= 1.1 && scale < 1.4)
			evflage = 7;
		else if (scale >= 0.9 && scale < 1.1)
			evflage = 6;
		else if (scale >= 0.7 && scale < 0.9)
			evflage = 5;
		else if (scale >= 0.55 && scale < 0.7)
			evflage = 4;
		else if (scale >= 0.45 && scale < 0.55)
			evflage = 3;
		else if (scale >= 0.36 && scale < 0.45)
			evflage = 2;
		else if (scale >= 0.28 && scale < 0.36)
			evflage = 1;
		else
			evflage = 0;
	}

	PARAM_CFG_S param;
	PARAM_GetParam(&param);
	aflage = param.Menu.AudioEnable.Current;
	osdflage = param.Menu.Osd.Current;
	rflage = param.MediaComm.Record.ChnAttrs[0].Enable;
	ltflage = param.Menu.VideoLoop.Current;
	vflage = param.CamCfg[0].CamMediaInfo.CurMediaMode;

	PARAM_RECORD_CHN_ATTR_S *recattr = &param.MediaComm.Record.ChnAttrs[0];
	FreePicNum = (recattr->SplitTime) / 1000;
	recflage = param.DevMng.Gsensor.enSensitity;
	lflage = param.Menu.Language.Current;
	if (0 == result) {

		memset(xmlbuff, 0, sizeof(xmlbuff));
		XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_TEST), DEF_XML_TEST, vflage, ltflage, evflage,
					 aflage, osdflage);
		XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_TEST2), DEF_XML_TEST2, FreePicNum, recflage, rflage, lflage, cam_status);
		NET_AddResponse(hc, xmlbuff, OBUFFSIZE);
	}

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

/* Command: http://192.168.1.254/?custom=1&cmd=3015
** Attr is the user group permission for the file:
** 7: rwx 6: rw 5: rx 4: r 3: wx 2: w 1: x
*/
int32_t XML_ListAllFileCreatTime(httpd_conn *hc, char *str)
{
	flag_file = 1;
	char *buf = NULL;
	char *filename = NULL;
	char nowtime[24];
	char fpathname[255];
	int32_t result = 0;
	int32_t bufflen = 0;
	int32_t totalbufflen = 0;
	uint32_t u32ModeState = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	uint32_t u32DirCount = 0;
	uint32_t u32Index = 0;
	uint32_t pu32FileObjCnt = 0;
	MESSAGE_S Msg = {0};
	struct stat filebuf;
	struct tm *lt;
	CVI_DTCF_DIR_E aenDirs[DTCF_DIR_BUTT];
	PARAM_FILEMNG_S stCfg = {0};

	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	MODEMNG_GetModeState(&u32ModeState);

	if (u32ModeState != MEDIA_MOVIE_STATE_VIEW) {
		CVI_LOGE("on_startrec_click \n");
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
			XML_Currency(hc, WIFIAPP_CMD_FWUPDATE, result);
			return result;
		}
		usleep(100 * 1000); // 等待停止录像
	}
	OSAL_FS_System("echo 3 > /proc/sys/vm/drop_caches"); // 清除缓存
	NETCTRLINNER_StopTimer();
	PARAM_GetFileMngParam(&stCfg);
	for (u32Index = 0; u32Index < DTCF_DIR_BUTT; u32Index++) {
		if (0 < strnlen(stCfg.FileMngDtcf.aszDirNames[u32Index], CVI_DIR_LEN_MAX)) {
			aenDirs[u32DirCount++] = u32Index;
		}
	}
	FILEMNG_SetSearchScope(aenDirs, u32DirCount, &pu32FileObjCnt);

	bufflen += pu32FileObjCnt;
	totalbufflen = (sizeof(DEF_XML_HEAD) + sizeof("<LIST>\r\n") + bufflen * BUFFSIZE + sizeof("</LIST>"));
	if (NULL != xmlbuff_file && file_size < totalbufflen) {
		free(xmlbuff_file);
		xmlbuff_file = (char *)malloc(totalbufflen);
		file_size = totalbufflen;
	}
	memset(xmlbuff_file, 0, file_size);

	XML_Snprintf(xmlbuff_file, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff_file, buf, sizeof("<LIST>\r\n"), "<LIST>\r\n");

	for (uint32_t i = 0; i < pu32FileObjCnt; i++) {
		int32_t attr = 0;
		FILEMNG_GetFileByIndex(i, fpathname, 255);
		if (NULL == fpathname) {
			CVI_LOGE("cmd 3015 fpathname is null");
			result = WIFIAPP_FALSE;
		} else {
			filename = (strrchr(fpathname, '/') + 1);
		}

		if (NULL == filename) {
			CVI_LOGE("cmd 3015 filename is null");
			result = WIFIAPP_FALSE;
		} else {
			result = stat(fpathname, &filebuf);
			lt = localtime(&filebuf.st_ctime);
			memset(nowtime, 0, sizeof(nowtime));
			strftime(nowtime, 24, "%Y/%m/%d %H:%M:%S", lt);

			if (filebuf.st_mode & S_IRGRP) {
				attr += 4;
			}
			if (filebuf.st_mode & S_IWGRP) {
				attr += 2;
			}
			if (filebuf.st_mode & S_IXGRP) {
				attr += 1;
			}
		}

		if (WIFIAPP_OK != result) {
			XML_Currency(hc, WIFIAPP_CMD_FILELIST, result);
			return result;
		} else {
			char datebuf[LBUFFSIZE] = "A:";
			strcat(datebuf, fpathname);
			XML_Snprintf(xmlbuff_file, buf, BUFFSIZE, DEF_XML_LISTAL_FILE, filename, datebuf, filebuf.st_size,
						 filebuf.st_ctime, nowtime, 32);
		}
	}

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}

	XML_Snprintf(xmlbuff_file, buf, sizeof("</LIST>"), "</LIST>");
	NET_AddResponse(hc, xmlbuff_file, strlen(xmlbuff_file));

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}
	NETCTRLINNER_StartTimer();
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3016&par=0
int32_t XML_CommandSmartPhoneCheck(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	XML_Currency(hc, WIFIAPP_CMD_HEARTBEAT, result);
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_CONNECT;
	EVENTHUB_Publish(&stEvent);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3017
int32_t XML_GetDiskFreeSpace(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	unsigned long long FreePicNum = 0;
	int32_t result = 0;
	STG_FS_INFO_S stFSInfo = {0};

	result = STORAGEMNG_GetFSInfo(&stFSInfo);
	if (0 != result) {
		CVI_LOGE("get fs info failed\n");
		return -1;
	}
	if (0 != result) {
		FreePicNum = -1;
	} else {
		FreePicNum = stFSInfo.u64AvailableSize; // unit bytes
	}

	XML_Longvaluecurrency(hc, WIFIAPP_CMD_DISK_FREE_SPACE, result, FreePicNum);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3018
int32_t XML_ReconnectWiFi(httpd_conn *hc, char *str)
{
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_WIFI_STATUS;
	Msg.arg2 = 0;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("ReconnectWiFi wifi stop fail");
		return sendresult;
	}

	Msg.topic = EVENT_MODEMNG_SETTING;
	Msg.arg1 = PARAM_MENU_WIFI_STATUS;
	Msg.arg2 = 1;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("ReconnectWiFi wifi start fail");
		return sendresult;
	}
	return sendresult;
}

/* Current no power query, without this function, defaults to 100%, that is, 0 (full) is returned
** Command: http://192.168.1.254/?custom=1&cmd=3019 */
int32_t XML_GetbatteryLevel(httpd_conn *hc, char *str)
{
	unsigned long FreePicNum = 0;
	int32_t result = 0;
	XML_Valuecurrency(hc, WIFIAPP_CMD_GET_BATTERY, result, FreePicNum);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3021
int32_t XML_SaveMenuInformation(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	PARAM_SetSaveFlg();
	XML_Currency(hc, WIFIAPP_CMD_SAVE_MENUINFO, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3022
int32_t XML_GetHardwareCapacity(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	unsigned long FreePicNum = 0;
	int32_t result = 0;
	XML_Valuecurrency(hc, WIFIAPP_CMD_GET_HW_CAP, result, FreePicNum);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3023
int32_t XML_RemoveLastuser(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_REMOVE_USER, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3024
int32_t XML_GetCardsSatus(httpd_conn *hc, char *str)
{
	unsigned long FreePicNum = 0;
	int32_t result = 0;
	FreePicNum = MODEMNG_GetCardState();
	if (10 <= FreePicNum) {
		result = WIFIAPP_FALSE;
	}

	XML_Valuecurrency(hc, WIFIAPP_CMD_GET_CARD_STATUS, result, FreePicNum);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3025
int32_t XML_GetDownLoadURL(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3026
int32_t XML_GetUpdateFWpath(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	return result;
}

// Command:3027
// exaple:up.html
int32_t XML_UpBinDateFWpath(httpd_conn *hc, char *str)
{
#define READ_DATA_SIZE 1024 * 1024
#define MD5_SIZE 16
#define MD5_STR_LEN (MD5_SIZE * 2)
	char file_path[64];
	snprintf(file_path, 64, "mnt/sd/%s", CVI_GETUPFILENAME());
	char md5_str[MD5_STR_LEN + 1];
	int32_t i;
	int32_t fd;
	int32_t ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	int32_t result = 0;
	MD5_CTX_S md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd) {
		perror("open");
		result = -1;
	}

	// init md5
	MD5_Init(&md5);
	while (1) {
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret) {
			perror("read");
			result = -1;
		}
		MD5_Update(&md5, data, ret);
		if (0 == ret || ret < READ_DATA_SIZE) {
			break;
		}
	}
	close(fd);
	MD5_Final(&md5, md5_value);

	for (i = 0; i < MD5_SIZE; i++) {
		snprintf(md5_str + i * 2, 2 + 1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

	if (strcmp(str, md5_str) == 0) {
		result = 0;
	} else {
		CVI_LOGE("up bin file md5 error");
		result = -1;
	}
	XML_Currency(hc, WIFIAPP_CMD_UPLOAD_FILE, result);
	return 0;
}

// Command: http://192.168.1.254/?custom=1&cmd=3028&par=1
int32_t XML_SetUpPIPStyle(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	int32_t pstr = 0;
	int32_t s32Result = 0;
	char rtspname[128] = APP_RTSP_NAME;
	sscanf(str, "%d", &pstr);

	PARAM_MENU_S param;
	PARAM_GetMenuParam(&param);
	if (((param.ViewWin.Current >> pstr) & 0x1) != 1) {
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_SET_PIP_STYLE, result);
		return result;
	}
	int32_t sendresult = 0;
	cam_status = pstr;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_RTSP_SWITCH;
	Msg.arg1 = (int)pstr;
	Msg.arg2 = (int)MAX_CAMERA_INSTANCES;
	memcpy(Msg.aszPayload, rtspname, 128);
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}
	XML_Currency(hc, WIFIAPP_CMD_SET_PIP_STYLE, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3029
int32_t XML_GetSSIDandPassPhrase(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t s32Ret = 0;
	int32_t result = 0;
	PARAM_WIFI_S wifipar;
	char *buf = NULL;
	char xmlbuff[LBUFFSIZE];
	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}
	s32Ret = PARAM_GetWifiParam(&wifipar);
	if (0 != s32Ret) {
		CVI_LOGE("Api GetSSIDandpassphrase cmd PARAM_GetWifiParam faile\n");
		result = WIFIAPP_FALSE;
		XML_Currency(hc, WIFIAPP_CMD_GET_SSID_PASSPHRASE, result);
		return result;
	}

	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_ENABLE,
				 wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID,
				 wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord);
	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3030
int32_t XML_GetMovieSizeCapacity(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	char *buf = NULL;
	char xmlbuff[TBUFFSIZE];
	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, sizeof("<LIST>\r\n"), "<LIST>\r\n");
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_GETMOVECAP, "1920X1080P", 2, 1920, 1080, 25, 4);
	XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_GETMOVECAP, "2560X1440P", 6, 2560, 1440, 25, 4);
	XML_Snprintf(xmlbuff, buf, sizeof("/<LIST>"), "</LIST>");
	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=3031&str=all
int32_t XML_QueryMenuItem(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	char *buf = NULL;
	char xmlbuff[TBUFFSIZE];

	typedef struct _CMDARRAY {
		int32_t cmd;
		char *cmdname;
	} CMDARRAY;
	CMDARRAY cmdarray[] = {{WIFIAPP_CMD_MOVIE_WDR, "MOVIE_WDR"},
						   {WIFIAPP_CMD_MOTION_DET, "MOVIE_MOTION_DET"},
						   {WIFIAPP_CMD_MOVIE_AUDIO, "MOVIE_AUDIO"},
						   {WIFIAPP_CMD_DATEIMPRINT, "MOVIE_DATEIMPRINT"},
						   {WIFIAPP_CMD_MOVIE_GSENSOR_SENS, "GSENSOR"},
						   {WIFIAPP_CMD_SET_AUTO_RECORDING, "WIFI_AUTO_RECORDING"},
						   {WIFIAPP_CMD_LANGUAGE, "LANGUAGE"},
						   {WIFIAPP_CMD_SET_PIP_STYLE, "PIP_VIEW"},
						   {WIFIAPP_CMD_TVFORMAT, "TV_MODE"},
						   {WIFIAPP_CMD_PARK_MONITOR, "PARK_MONITOR"}};

	CMDARRAY cmdary[] = {{WIFIAPP_CMD_CYCLIC_REC, "MOVIE_CYCLIC_REC"},
						 {WIFIAPP_CMD_MOVIE_EV, "EV"},
						 {WIFIAPP_CMD_MOVIE_REC_SIZE, "REC_SIZE"}};

	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}
	int32_t cmdarray_num = sizeof(cmdarray) / sizeof(cmdarray[0]);
	int32_t cmdary_num = sizeof(cmdary) / sizeof(cmdary[0]);
	char name[8] = {0};
	memset(xmlbuff, 0, sizeof(xmlbuff));
	XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
	XML_Snprintf(xmlbuff, buf, sizeof("<LIST>\r\n"), "<LIST>\r\n");
	for (int32_t cmdid = 0; cmdid < cmdarray_num; cmdid++) {
		if (WIFIAPP_CMD_MOVIE_GSENSOR_SENS == cmdarray[cmdid].cmd) {
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdarray[cmdid].cmd, cmdarray[cmdid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "关");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "低");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 2, "中");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 3, "高");
			XML_Snprintf(xmlbuff, buf, sizeof("</MenuList>\r\n</Item>\r\n"), "</MenuList>\r\n</Item>\r\n");
		} else if (WIFIAPP_CMD_SET_PIP_STYLE == cmdarray[cmdid].cmd) {
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdarray[cmdid].cmd, cmdarray[cmdid].cmdname);
			for (int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
				sprintf(name, "cam_%d", i);
				XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, i, name);
			}
			XML_Snprintf(xmlbuff, buf, sizeof("</MenuList>\r\n</Item>\r\n"), "</MenuList>\r\n</Item>\r\n");
		} else if (WIFIAPP_CMD_LANGUAGE == cmdarray[cmdid].cmd) {
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdarray[cmdid].cmd, cmdarray[cmdid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "EN");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "China");
			XML_Snprintf(xmlbuff, buf, sizeof("</MenuList>\r\n</Item>\r\n"), "</MenuList>\r\n</Item>\r\n");
		} else if (WIFIAPP_CMD_TVFORMAT == cmdarray[cmdid].cmd) {
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdarray[cmdid].cmd, cmdarray[cmdid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "NTSC");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "PAL");
			XML_Snprintf(xmlbuff, buf, sizeof("</MenuList>\r\n</Item>\r\n"), "</MenuList>\r\n</Item>\r\n");
		} else {
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdarray[cmdid].cmd, cmdarray[cmdid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "OFF");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "ON");
			XML_Snprintf(xmlbuff, buf, sizeof("</MenuList>\r\n</Item>\r\n"), "</MenuList>\r\n</Item>\r\n");
		}
	}
	for (int32_t cmdrid = 0; cmdrid < cmdary_num; cmdrid++) {
		switch (cmdary[cmdrid].cmd) {
		case WIFIAPP_CMD_CAPTURESIZE:
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdary[cmdrid].cmd, cmdary[cmdrid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "1440P");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "1080P");
			break;
		case WIFIAPP_CMD_CYCLIC_REC:
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdary[cmdrid].cmd, cmdary[cmdrid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "1分钟");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "3分钟");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 2, "5分钟");
			break;
		case WIFIAPP_CMD_MOVIE_EV:
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdary[cmdrid].cmd, cmdary[cmdrid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "+2.0");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "+1.6");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 2, "+1.3");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 3, "+1.0");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 4, "+0.6");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 5, "+0.3");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 6, "+0.0");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 7, "-0.3");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 8, "-0.6");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 9, "-1.0");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 10, "-1.3");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 11, "-1.6");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 12, "-2.0");
			break;
		case WIFIAPP_CMD_MOVIE_REC_SIZE:
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, "<Item>\r\n<Cmd>%d</Cmd>\r\n<Name>%s</Name>\r\n<MenuList>\r\n",
						 cmdary[cmdrid].cmd, cmdary[cmdrid].cmdname);
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 0, "1080P");
			XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_QUER_LIST, 1, "1440P");
			break;
		default:
			break;
		}

		XML_Snprintf(xmlbuff, buf, sizeof("</MenuList>\r\n</Item>\r\n"), "</MenuList>\r\n</Item>\r\n");
	}

	XML_Snprintf(xmlbuff, buf, sizeof("<CHK>A6B7</CHK>\r\n</LIST>"), "<CHK>A6B7</CHK>\r\n</LIST>");

	NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}

	return result;
}

// Command : http://192.168.1.254/?custom=1&cmd=3032&str=SSID:passphrase
int32_t XML_SendSSIDandPassPhrase(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_SEND_SSID_PASSPHRASE, result);

	return result;
}

// Command :http://192.168.1.254/?custom=1&cmd=3033&par=1
int32_t XML_SetWiFiConnectionMode(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	// int32_t pstr = 0;
	// sscanf(str,"%d", &pstr);
	XML_Currency(hc, WIFIAPP_CMD_SET_WIFI_CONNECT_MODE, result);

	return result;
}

// Command :http://192.168.1.254/?custom=1&cmd=3034
int32_t XML_AutoTestDone(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	XML_Currency(hc, WIFIAPP_CMD_AUTO_TEST_CMD_DONE, result);

	return result;
}

// Command :http://192.168.1.254/?custom=1&cmd=3035
int32_t XML_StartAppSession(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	XML_Currency(hc, WIFIAPP_CMD_APP_STARTUP, result);
	return result;
}

// Command :http://192.168.1.254/?custom=1&cmd=3036
int32_t XML_AppSessionClose(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_RTSP_DEINIT;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}
	XML_Currency(hc, WIFIAPP_CMD_APP_SESSION_CLOSE, result);
	cam_status = 0;
	return result;
}

// Command :http://192.168.1.254/?custom=1&cmd=3037
int32_t XML_QueryCurrentDVRmode(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	int32_t mode;
	mode = MODEMNG_GetCurWorkMode();
	if (mode == 0) {
		result = 0;
	} else if (mode == 1) {
		result = 4;
	} else if (mode == 2) {
		result = 3;
	} else {
		result = 5;
	}
	XML_Currency(hc, WIFIAPP_CMD_GET_MODE_STAUTS, result);

	return result;
}

// Command :http://192.168.1.254/?custom=1&cmd=3038
int32_t XML_SacnWifiAp(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
	return result;
}

// Command: http://192.168.1.254/NOVATEK/MOVIE/2014_0321_011922_002.MOV?custom=1&cmd=4001
int32_t XML_GetThumbnail(httpd_conn *hc, char *str)
{
	int32_t ret = 0;
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
	THUMBNAIL_PACKET_S packet = {0};
	THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;
	ret = THUMBNAIL_EXTRACTOR_Create(&viewer_handle);
	ret = THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, str, &packet);
	uint32_t size = packet.size;
	if (size == 0) {
		CVI_LOGE("Get Thumbnail faile!\n");
		THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
		XML_Currency(hc, WIFIAPP_CMD_THUMB, ret);
		return WIFIAPP_FALSE;
	}

	if (0 != ret) {
		XML_Currency(hc, WIFIAPP_CMD_THUMB, ret);
	} else {
		NET_AddResponse(hc, (char *)(packet.data), packet.size);
	}

	THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
	THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
#endif

	return ret;
}

// Command : http://192.168.1.254/NOVATEK/MOVIE/2014_0321_011922_002.MOV?custom=1&cmd=4002
int32_t XML_GetScreennail(httpd_conn *hc, char *str)
{
	int32_t ret = 0;
#ifdef COMPONENTS_THUMBNAIL_EXTRACTOR_ON
	THUMBNAIL_PACKET_S packet = {0};
	THUMBNAIL_EXTRACTOR_HANDLE_T viewer_handle = NULL;
	ret = THUMBNAIL_EXTRACTOR_Create(&viewer_handle);
	ret = THUMBNAIL_EXTRACTOR_GetThumbnail(viewer_handle, str, &packet);
	uint32_t size = packet.size;
	if (size == 0) {
		CVI_LOGE("Get Thumbnail faile!\n");
		THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
		XML_Currency(hc, WIFIAPP_CMD_THUMB, ret);
		return WIFIAPP_FALSE;
	}

	if (0 != ret) {
		XML_Currency(hc, WIFIAPP_CMD_THUMB, ret);
	} else {
		NET_AddResponse(hc, (char *)(packet.data), packet.size);
	}

	THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
	THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
#endif

	return ret;
}

// Command:Http://192.168.1.254/?custom=1&cmd=4003&str=A:PATH/2024_02_20_100736_00.MOV
int32_t XML_DeleteOneFile(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	if (str == NULL) {
		CVI_LOGI("File path is null\n");
		result = -1;
		XML_Currency(hc, WIFIAPP_CMD_DELETE_ONE, result);
		return result;
	}
	char *sFileName = NULL;
	sFileName = strchr(str, ':') + 1;
	FILEMNG_RemoveFile(sFileName);
	XML_Currency(hc, WIFIAPP_CMD_DELETE_ONE, result);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=4004
int32_t XML_DeleteAll(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t i = 0;
	char datebuf[LBUFFSIZE] = {0};

	STG_DEVINFO_S sd_param = {0};
	PARAM_GetStgInfoParam(&sd_param);
	PARAM_FILEMNG_S file_param = {0};
	PARAM_GetFileMngParam(&file_param);

	for (i = 0; i < DTCF_DIR_BUTT; i++) {
		if (file_param.FileMngDtcf.aszDirNames[i][0] != '\0') {
			memset(datebuf, 0, sizeof(datebuf));
			strcat(datebuf, "rm -rf ");
			strcat(datebuf, sd_param.aszMntPath);
			strcat(datebuf, file_param.FileMngDtcf.szRootDir);
			strcat(datebuf, "/");
			strcat(datebuf, file_param.FileMngDtcf.aszDirNames[i]);
			strcat(datebuf, "/");
			if (i == DTCF_DIR_PHOTO_FRONT || i == DTCF_DIR_PHOTO_REAR) {
				strcat(datebuf, "*.JPG");
			} else {
				strcat(datebuf, "*.*");
			}
			OSAL_FS_System(datebuf);
		}
	}
	XML_Currency(hc, WIFIAPP_CMD_DELETE_ALL, result);

	return result;
}

// Command:http://192.168.1.254/NOVATEK/MOVIE/2014_0321_011922_002.MOV?custom=1&cmd=4005
int32_t XML_GetMovieFileInformation(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter: %s \n", __func__);
	int32_t result = 0;
#ifdef SERVICES_PLAYER_ON
	void *player = NULL;
	char *buf = NULL;
	char xmlbuff[TBUFFSIZE];
	if (NULL == buf) {
		buf = (char *)malloc(BUFFSIZE);
	}

	memset(xmlbuff, 0, sizeof(xmlbuff));

	PLAYER_Create(&player);
	PLAYER_SetDataSource(player, str);
	PLAYER_MEDIA_INFO_S info = {};

	result = PLAYER_GetMediaInfo(player, &info);
	if (0 != result) {
		XML_Currency(hc, WIFIAPP_CMD_MOVIE_FILE_INFO, result);
	} else {
		XML_Snprintf(xmlbuff, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
		XML_Snprintf(xmlbuff, buf, LBUFFSIZE, DEF_XML_STR, WIFIAPP_CMD_MOVIE_FILE_INFO, result, info.width, info.height, info.duration_sec);
		NET_AddCgiResponse(strlen(xmlbuff), hc, "%s", xmlbuff);
	}

	if (NULL != buf) {
		free(buf);
		buf = NULL;
	}
#endif

	return result;
}

// Command:http://192.168.1.254/?custom=1&cmd=5001&par=69e2f0e02ccdef024104dc3ecc1b7eec
// exaple:up.html
int32_t XML_UpdateFile(httpd_conn *hc, char *str)
{
#define READ_DATA_SIZE 1024 * 1024
#define MD5_SIZE 16
#define MD5_STR_LEN (MD5_SIZE * 2)
	char file_path[64];
	snprintf(file_path, 64, "mnt/sd/%s", CVI_GETUPFILENAME());
	char md5_str[MD5_STR_LEN + 1];
	int32_t i;
	int32_t fd;
	int32_t ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX_S md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd) {
		perror("open");
		return -1;
	}

	// init md5
	MD5_Init(&md5);
	while (1) {
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret) {
			perror("read");
			return -1;
		}
		MD5_Update(&md5, data, ret);
		if (0 == ret || ret < READ_DATA_SIZE) {
			break;
		}
	}
	close(fd);
	MD5_Final(&md5, md5_value);

	for (i = 0; i < MD5_SIZE; i++) {
		snprintf(md5_str + i * 2, 2 + 1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

	if (strcmp(str, md5_str) == 0) {
		return 0;
	} else {
		CVI_LOGE("up file md5 error");
		return -1;
	}
}

// Command:http://192.168.1.254/?custom=1&cmd=5002&par=69e2f0e02ccdef024104dc3ecc1b7eec
// exaple:up.html
int32_t XML_UpdateAudioFile(httpd_conn *hc, char *str)
{
#define READ_DATA_SIZE 1024 * 1024
#define MD5_SIZE 16
#define MD5_STR_LEN (MD5_SIZE * 2)
	char file_path[64];
	snprintf(file_path, 64, "mnt/sd/%s", CVI_GETUPFILENAME());
	char md5_str[MD5_STR_LEN + 1];
	int32_t i;
	int32_t fd;
	int32_t ret;
	unsigned char data[READ_DATA_SIZE];
	unsigned char md5_value[MD5_SIZE];
	MD5_CTX_S md5;

	fd = open(file_path, O_RDONLY);
	if (-1 == fd) {
		perror("open");
		return -1;
	}

	// init md5
	MD5_Init(&md5);
	while (1) {
		ret = read(fd, data, READ_DATA_SIZE);
		if (-1 == ret) {
			perror("read");
			return -1;
		}
		MD5_Update(&md5, data, ret);
		if (0 == ret || ret < READ_DATA_SIZE) {
			break;
		}
	}
	close(fd);
	MD5_Final(&md5, md5_value);

	for (i = 0; i < MD5_SIZE; i++) {
		snprintf(md5_str + i * 2, 2 + 1, "%02x", md5_value[i]);
	}
	md5_str[MD5_STR_LEN] = '\0'; // add end

	if (strcmp(str, md5_str) == 0) {
		return 0;
	} else {
		CVI_LOGE("up audio file md5 error");
		return -1;
	}
}

// Command: http://192.168.1.254/?custom=1&cmd=8050&par=0
int32_t XML_ParkingMonitoring(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	sscanf(str, "%d", &pstr);

	if (0 == pstr) {
		PARAM_SetMenuParam(0, PARAM_MENU_PARKING, MENU_PARKING_OFF);
	} else if (1 == pstr) {
		PARAM_SetMenuParam(0, PARAM_MENU_PARKING, MENU_PARKING_ON);
	} else {
		CVI_LOGE("Parkingmonitoring parm error, par = %d\n", pstr);
		result = WIFIAPP_FALSE;
	}

	XML_Currency(hc, WIFIAPP_CMD_PARK_MONITOR, result);

	return result;
}

int32_t XML_MediaLapseTime(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	int32_t pstr = 0;
	uint32_t u32ModeState = 0;
	int32_t s32Result = 0;
	int32_t sendresult = 0;
	sscanf(str, "%d", &pstr);
	MESSAGE_S Msg = {0};
	MODEMNG_GetModeState(&u32ModeState);

	if ((pstr < 0) || (pstr > 3)) {
		if (4 == pstr) {
			if (u32ModeState == MEDIA_MOVIE_STATE_REC) {
				Msg.topic = EVENT_MODEMNG_STOP_REC;
				sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
				if (0 != sendresult || 0 != s32Result) {
					CVI_LOGE("send message fail");
					result = WIFIAPP_FALSE;
					XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
					return result;
				}
				usleep(100 * 1000);
			}

			memset(&Msg, 0, sizeof(MESSAGE_S));
			Msg.topic = EVENT_MODEMNG_SETTING;
			Msg.arg1 = PARAM_MENU_LAPSE_TIME;
			Msg.arg2 = 0;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
			if (0 != sendresult || 0 != s32Result) {
				CVI_LOGE("send message fail");
				result = WIFIAPP_FALSE;
				XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
				return result;
			}

			memset(&Msg, 0, sizeof(MESSAGE_S));
			Msg.topic = EVENT_MODEMNG_MODESWITCH;
			Msg.arg1 = WORK_MODE_MOVIE;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
			if (0 != sendresult || 0 != s32Result) {
				CVI_LOGE("send message fail");
				result = WIFIAPP_FALSE;
				XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
				return result;
			}
			usleep(10 * 1000);

			memset(&Msg, 0, sizeof(MESSAGE_S));
			Msg.topic = EVENT_MODEMNG_SETTING;
			Msg.arg1 = PARAM_MENU_LAPSE_TIME;
			Msg.arg2 = MEDIA_VIDEO_LAPSETIME_OFF;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
			if (0 != sendresult || 0 != s32Result) {
				CVI_LOGE("send message fail");
				result = WIFIAPP_FALSE;
				XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
				return result;
			}

			memset(&Msg, 0, sizeof(MESSAGE_S));
			Msg.topic = EVENT_MODEMNG_START_REC;
			sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
			if (0 != sendresult || 0 != s32Result) {
				CVI_LOGE("send message fail");
				result = WIFIAPP_FALSE;
				XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
				return result;
			}
		} else {
			result = WIFIAPP_FALSE;
		}
		XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
		return result;
	}

	if (0 == pstr) {
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
		}
		XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
		return result;
	}

	if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
		(u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
		Msg.topic = EVENT_MODEMNG_STOP_REC;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("lapse send message fail");
			result = WIFIAPP_FALSE;
			XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);
			return result;
		}
	}

	if (WORK_MODE_LAPSE != MODEMNG_GetCurWorkMode()) {
		Msg.topic = EVENT_MODEMNG_MODESWITCH;
		Msg.arg1 = WORK_MODE_LAPSE;
		Msg.arg2 = pstr;
		sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
		if (0 != sendresult || 0 != s32Result) {
			CVI_LOGE("send message fail");
			result = WIFIAPP_FALSE;
		}
		usleep(2 * 1000 * 1000);
	}

	XML_Currency(hc, WIFIAPP_CMD_LASPE_TIME, result);

	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=8567
int32_t XML_RoadCamStart(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_CONNECT;
	EVENTHUB_Publish(&stEvent);
	XML_Currency(hc, WIFIAPP_CMD_ROADCAM_START, result);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=9090
int32_t XML_PhoneApp(httpd_conn *hc, char *str)
{
	CVI_LOGE("enter XML_PhoneApp\n");
	int32_t result = 0;
	int32_t sendresult = 0;
	int s32Result = 0;
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_CONNECT;
	EVENTHUB_Publish(&stEvent);
	char rtspname[128] = APP_RTSP_NAME;
	MESSAGE_S Msg = {0};
	Msg.topic = EVENT_MODEMNG_RTSP_INIT;
	Msg.arg1 = (int)MAX_CAMERA_INSTANCES;
	memcpy(Msg.aszPayload, rtspname, 128);
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
	if (0 != sendresult || 0 != s32Result) {
		CVI_LOGE("send message fail");
		result = WIFIAPP_FALSE;
	}
	XML_Currency(hc, WIFIAPP_CMD_PHONE_APP, result);
	return result;
}

// Command: http://192.168.1.254/?custom=1&cmd=9095
int32_t XML_SensorNum(httpd_conn *hc, char *str)
{
	int32_t result = 0;
	XML_Valuecurrency(hc, WIFIAPP_CMD_SENSOR_NUM, result, MAX_CAMERA_INSTANCES);
	return result;
}

static int app_socket_write(int new_fd)
{
	char xmlbuff[LBUFFSIZE];
	int r = 0;
	int CardStatus = 0;
	int card_status = -1;
	uint32_t RecStatus = 0;
	uint32_t rec_status = 10;
	memset(xmlbuff, 0, sizeof(xmlbuff));

	while (flag_socket == WIFI_APP_CONNECTTED) {
		CardStatus = MODEMNG_GetCardState();
		MODEMNG_GetModeState(&RecStatus);
		if (card_status != CardStatus) {
			card_status = CardStatus;
			if (card_status == CARD_STATE_AVAILABLE) {
				sprintf(xmlbuff, DEF_XML_STATE, 9); // 在手机APP协议中，状态9表示SD卡已插入
			} else {
				sprintf(xmlbuff, DEF_XML_STATE, 10); // 在手机APP协议中，状态10表示SD卡未插入
			}
			r = send(new_fd, xmlbuff, sizeof(xmlbuff), 0);
		} else if (rec_status != RecStatus) {
			rec_status = RecStatus;
			if (rec_status == MEDIA_MOVIE_STATE_REC || rec_status == MEDIA_MOVIE_STATE_LAPSE_REC) {
				sprintf(xmlbuff, DEF_XML_STATE, 1); // 在手机APP协议中，状态1表示正在录像
			} else {
				sprintf(xmlbuff, DEF_XML_STATE, 2); // 在手机APP协议中，状态2表示停止录像
			}
			r = send(new_fd, xmlbuff, sizeof(xmlbuff), 0);
		} else {
			usleep(500 * 1000);
		}
	}
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
	int32_t port = 3333, s32Ret = 0, f = -1;
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
				close(new_fd);
			}
		}
	}
	close(f);
	return (void *)0;
}

static int32_t custom_register(const int32_t cmd, void *cb)
{
	NET_WIFIAPPMAPTO_S stCgiCmd;
	int32_t s32Ret = 0;

	if (cmd != 0) {
		stCgiCmd.cmd = cmd;
	} else {
		CVI_LOGE("error, cmd is NULL\n");
		s32Ret = -1;
		goto EXIT;
	}

	if (cb == NULL) {
		s32Ret = -1;
		goto EXIT;
	}
	stCgiCmd.callback = (NET_SYSCALL_CMD_TO_CALLBACK)cb;
	NET_RegisterCgiCmd(&stCgiCmd);
EXIT:
	return s32Ret;
}

void NETCTRLINNER_CMDRegister(void)
{
	CVI_LOGE("enter: %s \n", __func__);
	custom_register(WIFIAPP_CMD_CAPTURE, (void *)XML_GetPicture);
	custom_register(WIFIAPP_CMD_CAPTURESIZE, (void *)XML_SetCapture);
	custom_register(WIFIAPP_CMD_FREE_PIC_NUM, (void *)XML_GetFreePicture);
	custom_register(WIFIAPP_CMD_RECORD, (void *)XML_StartandStopMovieRecord);
	custom_register(WIFIAPP_CMD_MOVIE_REC_SIZE, (void *)XML_SetMovieRecord);
	custom_register(WIFIAPP_CMD_CYCLIC_REC, (void *)XML_SetCyclicRecordValue);
	custom_register(WIFIAPP_CMD_MOVIE_WDR, (void *)XML_EdisableMoviehdr);
	custom_register(WIFIAPP_CMD_MOVIE_EV, (void *)XML_SetMovieEvValue);
	custom_register(WIFIAPP_CMD_MOTION_DET, (void *)XML_EdisableMotionDetection);
	custom_register(WIFIAPP_CMD_MOVIE_AUDIO, (void *)XML_EdisableMovieAudio);
	custom_register(WIFIAPP_CMD_DATEIMPRINT, (void *)XML_EdisableDateInprint);
	custom_register(WIFIAPP_CMD_MAX_RECORD_TIME, (void *)XML_GetMovieMaxRecordTime);
	custom_register(WIFIAPP_CMD_MOVIE_LIVEVIEW_SIZE, (void *)XML_ChangeLiveviewSize);
	custom_register(WIFIAPP_CMD_MOVIE_GSENSOR_SENS, (void *)XML_EDisableMovieGSensorSensitivity);
	custom_register(WIFIAPP_CMD_SET_AUTO_RECORDING, (void *)XML_SetAutoRecording);
	custom_register(WIFIAPP_CMD_MOVIE_REC_BITRATE, (void *)XML_SetMovieRecordBitrate);
	custom_register(WIFIAPP_CMD_MOVIE_LIVEVIEW_BITRATE, (void *)XML_SetMovieLiveViewBitrate);
	custom_register(WIFIAPP_CMD_MOVIE_LIVEVIEW_START, (void *)XML_StartandStopMovieLiveview);
	// custom_register(WIFIAPP_CMD_MOVIE_RECORDING_TIME, (void *)XML_GetMovieRecordingTime);
	custom_register(WIFIAPP_CMD_MOVIE_REC_TRIGGER_RAWENC, (void *)XML_TriggerRAWencode);
	custom_register(WIFIAPP_CMD_MOVIE_GET_RAWENC_JPG, (void *)XML_GetRAWencodeJPEG);
	custom_register(WIFIAPP_CMD_MOVIE_GET_LIVEVIEW_FMT, (void *)XML_GetStreamingAddr);
	custom_register(WIFIAPP_CMD_FLIP_MIRROR, (void *)XML_SetMovieFliphorMirror);
	custom_register(WIFIAPP_CMD_MOVIE_BRC_ADJUST, (void *)XML_MovieBitRateAdjust);
	custom_register(WIFIAPP_CMD_MODECHANGE, (void *)XML_ChangeSystemMode);
	custom_register(WIFIAPP_CMD_SET_SSID, (void *)XML_SetWiFiSSID);
	custom_register(WIFIAPP_CMD_SET_PASSPHRASE, (void *)XML_SetWiFiPassPhrase);
	custom_register(WIFIAPP_CMD_SET_DATE, (void *)XML_SetSystemDate);
	custom_register(WIFIAPP_CMD_SET_TIME, (void *)XML_SetSystemTime);
	custom_register(WIFIAPP_CMD_POWEROFF, (void *)XML_SetSystemAutoPowerOffTime);
	custom_register(WIFIAPP_CMD_LANGUAGE, (void *)XML_SelectLanguage);
	custom_register(WIFIAPP_CMD_TVFORMAT, (void *)XML_SetTVformat);
	custom_register(WIFIAPP_CMD_FORMAT, (void *)XML_CommandWouldFormatStorage);
	custom_register(WIFIAPP_CMD_SET_PIP_STYLE, (void *)XML_SetUpPIPStyle);
	custom_register(WIFIAPP_CMD_QUERY_MENUITEM, (void *)XML_QueryMenuItem);
	custom_register(WIFIAPP_CMD_SEND_SSID_PASSPHRASE, (void *)XML_SendSSIDandPassPhrase);
	custom_register(WIFIAPP_CMD_SET_WIFI_CONNECT_MODE, (void *)XML_SetWiFiConnectionMode);
	custom_register(WIFIAPP_CMD_QUERY, (void *)XML_QueryCurrentSupportCommand);
	custom_register(WIFIAPP_CMD_SYSRESET, (void *)XML_ResetallSettingTodeFault);
	custom_register(WIFIAPP_CMD_VERSION, (void *)XML_GetProjectVersion);
	custom_register(WIFIAPP_CMD_FWUPDATE, (void *)XML_StartToupdateFirmware);
	custom_register(WIFIAPP_CMD_QUERY_CUR_STATUS, (void *)XML_QueryaAllSettingCommandStatus);
	custom_register(WIFIAPP_CMD_FILELIST, (void *)XML_ListAllFileCreatTime);
	custom_register(WIFIAPP_CMD_DISK_FREE_SPACE, (void *)XML_GetDiskFreeSpace);
	custom_register(WIFIAPP_CMD_RECONNECT_WIFI, (void *)XML_ReconnectWiFi);
	custom_register(WIFIAPP_CMD_GET_BATTERY, (void *)XML_GetbatteryLevel);
	custom_register(WIFIAPP_CMD_SAVE_MENUINFO, (void *)XML_SaveMenuInformation);
	custom_register(WIFIAPP_CMD_GET_HW_CAP, (void *)XML_GetHardwareCapacity);
	custom_register(WIFIAPP_CMD_REMOVE_USER, (void *)XML_RemoveLastuser);
	custom_register(WIFIAPP_CMD_UPLOAD_FILE, (void *)XML_UpBinDateFWpath);
	custom_register(WIFIAPP_CMD_AUTO_TEST_CMD_DONE, (void *)XML_AutoTestDone);
	custom_register(WIFIAPP_CMD_GET_CARD_STATUS, (void *)XML_GetCardsSatus);
	custom_register(WIFIAPP_CMD_GET_DOWNLOAD_URL, (void *)XML_GetDownLoadURL);
	custom_register(WIFIAPP_CMD_GET_UPDATEFW_PATH, (void *)XML_GetUpdateFWpath);
	custom_register(WIFIAPP_CMD_GET_SSID_PASSPHRASE, (void *)XML_GetSSIDandPassPhrase);
	custom_register(WIFIAPP_CMD_QUERY_MOVIE_SIZE, (void *)XML_GetMovieSizeCapacity);
	custom_register(WIFIAPP_CMD_APP_STARTUP, (void *)XML_StartAppSession);
	custom_register(WIFIAPP_CMD_APP_SESSION_CLOSE, (void *)XML_AppSessionClose);
	custom_register(WIFIAPP_CMD_GET_MODE_STAUTS, (void *)XML_QueryCurrentDVRmode);
	custom_register(WIFIAPP_CMD_WIFIAP_SEARCH, (void *)XML_SacnWifiAp);
	custom_register(WIFIAPP_CMD_HEARTBEAT, (void *)XML_CommandSmartPhoneCheck);
	custom_register(WIFIAPP_CMD_DELETE_ONE, (void *)XML_DeleteOneFile);
	custom_register(WIFIAPP_CMD_DELETE_ALL, (void *)XML_DeleteAll);
	custom_register(WIFIAPP_CMD_THUMB, (void *)XML_GetThumbnail);
	custom_register(WIFIAPP_CMD_MOVIE_FILE_INFO, (void *)XML_GetMovieFileInformation);
	custom_register(WIFIAPP_CMD_SCREEN, (void *)XML_GetScreennail);
	custom_register(WIFIAPP_CMD_PARK_MONITOR, (void *)XML_ParkingMonitoring);
	custom_register(WIFIAPP_CMD_UPLOAD, (void *)XML_UpdateFile);
	custom_register(WIFIAPP_CMD_UPLOAD_AUDIO, (void *)XML_UpdateAudioFile);
	custom_register(WIFIAPP_CMD_LASPE_TIME, (void *)XML_MediaLapseTime);
	custom_register(WIFIAPP_CMD_ROADCAM_START, (void *)XML_RoadCamStart);
	custom_register(WIFIAPP_CMD_PHONE_APP, (void *)XML_PhoneApp);
	custom_register(WIFIAPP_CMD_SENSOR_NUM, (void *)XML_SensorNum);
}

int32_t NETCTRLINNER_InitCMDSocket(void)
{
	pthread_mutex_lock(&gMutex);
	flag_socket = WIFI_APP_CONNECTTED;
	if (g_websocket_thread == 0) {
		pthread_create(&g_websocket_thread, NULL, app_socket, NULL);
	}
	if (xmlbuff_file == NULL) {
		xmlbuff_file = malloc(file_size);
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
	if (xmlbuff_file != NULL) {
		free(xmlbuff_file);
		xmlbuff_file = NULL;
	}
	flag_file = 0; // 复位文件异常退出检测标志
	pthread_mutex_unlock(&gMutex);
	return 0;
}

int32_t NETCTRLINNER_GetFlagFile(void)
{
	return flag_file;
}
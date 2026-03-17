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

#define DEF_XML_HEAD    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n"
#define DEF_CGI_LISTAL_FILE "<file>\r\n<name>%s</name>\r\n<format>%s</format>\r\n<size>%lld</size>\r\n<attr>%s</attr>\r\n<time>%s</time>\r\n</file>\r\n"
#define DEF_CGI_CNT_FILE	"<amount>%d</amount>\r\n"
#define DEF_CGI_STS_TEST    "%d\n%s\n"
#define DEF_CGI_LOG_TEST    "%s=%s\n"

static int32_t flag_socket = WIFI_APP_DISCONNECT;
static pthread_t g_websocket_thread;
static char *cgibuff_file;
static int32_t file_size = 10 * 256;
static int32_t flag_file = 0;
static char *client_ip = NULL;
static char *FoRStat = "front";
static double EV_value = -1;
static int32_t EV_Manual = 0;
static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;

unsigned long CGI_Snprintf(char *cgibuff, char *buf, int32_t size, const char *fmt, ...)
{
    unsigned long len = 0;
    va_list marker;
    va_start(marker, fmt);
    len = vsnprintf(buf, size, fmt, marker);
    strcat(cgibuff, buf);
    va_end(marker);

    return len;
}

void CGI_Currency(httpd_conn *hc, char *cmd, char *str, int32_t ret, char *log)
{
    char *buf = NULL;
    char cgibuff[LBUFFSIZE];

    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    char *status = "OK";
    if (ret != 0) status = "Fail";
    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, ret, status);

    if (ret != 0 && log != NULL) {
        CGI_Snprintf(cgibuff, buf, strlen(log), log);
    } else if (cmd != NULL && str != NULL) {
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, cmd, str);
    }

    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, ret, status);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }
}

/*cgi.1 视频分辨率的设置，url = /cgi-bin/Config.cgi?action=set&property=Videores&value=…(1440p25, 1080p25, 720p25)*/
int32_t CGI_SetMovieRecord(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t pstr = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};
    //分辨率设置
    if (strcmp(str, "720P25")==0) {pstr = 0;}
    else if (strcmp(str, "1080P25")==0) {pstr = 2;}
    else if (strcmp(str, "1440P25")==0) {pstr = 6;}
    else {
        CVI_LOGE("error input\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "error input\r\n");
        return result;
    }

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
            CGI_Currency(hc, NULL, NULL, result, "send message fail\r\n");
            return result;
        } else {
            NETCTRLINNER_UiUpdate();
        }
    }
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 录影分段时间设置，url = /cgi-bin/Config.cgi?action=set&property=LoopingVideo&value=…(1MIN, 3MIN, 5MIN)*/
int32_t CGI_SetCyclicRecordValue(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t pstr = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};
    //录影分段设置
    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = PARAM_MENU_VIDEO_LOOP;

    if (strcmp(str,"1MIN") == 0) {
        pstr = 0;
    } else if (strcmp(str,"3MIN") == 0) {
        pstr = 1;
    } else if (strcmp(str,"5MIN") == 0) {
        pstr = 2;
    } else {
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "error input\r\n");
        return result;
    }

    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);

    if (param.VideoLoop.Current != (uint32_t)pstr) {
        Msg.arg2 = pstr;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
    }

    if (0 != sendresult || 0 != s32Result) {
        CVI_LOGE("send message fail");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "start rec: send message fail\r\n");
        return result;
    }

    NETCTRLINNER_UiUpdate();
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 碰撞锁定的灵敏度的设置, url = /cgi-bin/Config.cgi?action=set&property=GSensor&value=…(OFF, Middle, High)*/
int32_t CGI_SetGSensorSensitivity(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t pstr = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};

    if (strcmp(str, "OFF")==0) {pstr = 0;}
    else if (strcmp(str, "Low")==0) {pstr = 1;}
    else if (strcmp(str, "Middle")==0) {pstr = 2;}
    else if (strcmp(str, "High")==0) {pstr = 3;}
    else {
        CVI_LOGE("error input\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "error input\r\n");
        return result;
    }

    if ((pstr < 0) || (pstr > 3)) {
        CVI_LOGE("EDisablemovieGSensorsensitivity parm error, par = %d\n", pstr);
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "EDisablemovieGSensorsensitivity parm error\r\n");
    } else {
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_GSENSOR;
        Msg.arg2 = pstr;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
        if (0 != sendresult || 0 != s32Result) {
            CVI_LOGE("send message fail");
            result = WIFIAPP_FALSE;
            CGI_Currency(hc, NULL, NULL, result, "send message fail\r\n");
            return result;
        } else {
            NETCTRLINNER_UiUpdate();
        }
    }
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 缩时录影的设置, url = /cgi-bin/Config.cgi?action=set&property=VideoTimeLapse&value=…(OFF, 1fps/s, 2fps/s, 5fps/s)*/
int32_t CGI_SetVideoTimeLapse(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t pstr = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};

    //缩时录影设置
    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = PARAM_MENU_LAPSE_TIME;

    if (strcmp(str,"OFF") == 0){
        pstr = 0;
    }else if (strcmp(str,"1fps/s") == 0){
        pstr = 1;
    }else if (strcmp(str,"2fps/s") == 0){
        pstr = 2;
    }else if (strcmp(str,"5fps/s") == 0){
        pstr = 3;
    }else {
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "error input\r\n");
        return result;
    }
    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    if (param.LapseTime.Current != (uint32_t)pstr) {
        Msg.arg2 = pstr;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
    }

    if (0 != sendresult || 0 != s32Result) {
        CVI_LOGE("send message fail");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "start rec: send message fail\r\n");
        return result;
    }
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 曝光设定的设置, url = /cgi-bin/Config.cgi?action=set&property=Exposure&value=…(EVN200, EVN167, EVN133,
EVN100, EVN067, EVN033, EV0, EVP033, EVP067, EVP100, EVP133, EVP167, EVP200)*/
int32_t CGI_SetMovieEvValue(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t value = 0;

    if (strcmp(str,"EVN200") == 0)      value = EV_value / 3.98;
    else if (strcmp(str,"EVN167") == 0) value = EV_value / 3.16;
    else if (strcmp(str,"EVN133") == 0) value = EV_value / 2.5;
    else if (strcmp(str,"EVN100") == 0) value = EV_value / 1.99;
    else if (strcmp(str,"EVN067") == 0) value = EV_value / 1.58;
    else if (strcmp(str,"EVN033") == 0) value = EV_value / 1.26;
    else if (strcmp(str,"EV0") == 0)    value = EV_value;
    else if (strcmp(str,"EVP033") == 0) value = EV_value * 1.26;
    else if (strcmp(str,"EVP067") == 0) value = EV_value * 1.58;
    else if (strcmp(str,"EVP100") == 0) value = EV_value * 1.99;
    else if (strcmp(str,"EVP133") == 0) value = EV_value * 2.5;
    else if (strcmp(str,"EVP167") == 0) value = EV_value * 3.16;
    else if (strcmp(str,"EVP200") == 0) value = EV_value * 3.98;
    else {
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "input false\r\n");
    }

    VI_PIPE ViPipe = 0;
    ISP_EXPOSURE_ATTR_S  expAttr;
    CVI_ISP_GetExposureAttr(ViPipe, &expAttr);
    expAttr.enOpType = OP_TYPE_MANUAL;
    expAttr.bByPass = 0;
    expAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
    expAttr.stManual.u32ExpTime = value;
    CVI_ISP_SetExposureAttr(ViPipe, &expAttr);

    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 格式化SD卡, url = /cgi-bin/Config.cgi?action=set&property=SD0&value=format*/
int32_t CGI_CommandWouldFormatStorage(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    uint32_t u32ModeState = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    int32_t count = 0;
    MESSAGE_S Msg = {0};

    MODEMNG_GetModeState(&u32ModeState);
    if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
        (u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
        Msg.topic = EVENT_MODEMNG_STOP_REC;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

        if (0 != sendresult || 0 != s32Result) {
            CVI_LOGE("send message fail\n");
            result = WIFIAPP_FALSE;
            CGI_Currency(hc, NULL, NULL, result, "send message fail\r\n");
            return result;
        }
        usleep(100*1000);
    }
    NETCTRLINNER_StopTimer();
    NETCTRLINNER_SetSdState(WIFI_SD_FORMAT_INIT);
    Msg.topic = EVENT_MODEMNG_CARD_FORMAT;
    sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

    if (0 != s32Result || 0 != sendresult) {
        CVI_LOGE("MODEMNG_Format failed\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "CVI_MODE_Format failed\r\n");
    } else {
        while(NETCTRLINNER_GetSdState() && count ++ < 20) {
            usleep(100 * 1000);
        }
        if (count >= 20 || NETCTRLINNER_GetSdState() == WIFI_SD_FORMAT_FAIL) {
            result = WIFIAPP_FALSE;
            CGI_Currency(hc, NULL, NULL, result, "CVI_MODE_Format failed\r\n");
        } else {
            CGI_Currency(hc, NULL, NULL, result, NULL);
        }
    }
    NETCTRLINNER_StartTimer();
    return result;
}

/*cgi 删除文件, url = cgi-bin/Config.cgi?action=del&property=$SD$Normal$F$DT180718-101134F.MOV*/
int32_t CGI_DeleteOneFile(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    uint32_t i = 0;
    for (i = 0; i < strlen(str); i++) {
        if (str[i] == '$') {
            str[i] = '/';
        }
    }
    FILEMNG_DelFile(0, str);// 0:represent the front camera
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 设置wifi账号, url = cgi-bin/Config.cgi?action=set&property=Net.WIFI_AP.SSID&value=\(ssid)*/
static int32_t CGI_SetSSID(httpd_conn *hc, char *str)
{
    int32_t s32Ret = 0;
    int32_t result = 0;
    PARAM_WIFI_S wifipar;

    s32Ret = PARAM_GetWifiParam(&wifipar);
    if (0 != s32Ret) {
        CVI_LOGE("Api passphrase cmd PARAM_GetWifiParam faile\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "Api passphrase cmd PARAM_GetWifiParam faile\r\n");
        return result;
    }

    memcpy(wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID, str, HAL_WIFI_SSID_LEN);

    s32Ret = PARAM_SetWifiParam(&wifipar);
    if (0 != s32Ret) {
        CVI_LOGE("SetWiFipassphrase PARAM_SetWifiParam fail\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "SetWiFipassphrase PARAM_SetWifiParam fail\r\n");
        return result;
    }
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 获取wifi账号, url = cgi-bin/Config.cgi?action=get&property=Net.WIFI_AP.SSID*/
static int32_t CGI_GetSSID(httpd_conn *hc, char *str)
{
    int32_t s32Ret = 0;
    int32_t result = 0;
    PARAM_WIFI_S wifipar;
    char *buf = NULL;
    char cgibuff[LBUFFSIZE];

    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    s32Ret = PARAM_GetWifiParam(&wifipar);

    if (0 != s32Ret) {
        CVI_LOGE("Api GetSSIDandpassphrase cmd PARAM_GetWifiParam faile\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "Api GetSSIDandpassphrase cmd PARAM_GetWifiParam faile\r\n");
        return result;
    }

    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, result, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Net.WIFI_AP.SSID", wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 对wifi名称进行操作*/
int32_t CGI_SetGetSSID(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    CVI_LOGI("enter CVI_CGI_SetGetSSIDandpassphrase, str = %s\n", str);
    //连接APP，需从录像模式进入
    if (MODEMNG_GetCurWorkMode() != WORK_MODE_MOVIE) {
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_MOVIE;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
        if(0 != sendresult || 0 != s32Result) {
            CVI_LOGE("send message fail");
            result = WIFIAPP_FALSE;
        }
        usleep(300 * 1000);
    }
    if (str != NULL) {
        result = CGI_SetSSID(hc, str);
    } else {
        result = CGI_GetSSID(hc, str);
    }

    return result;
}

/*cgi 设置wifi密码, url = cgi-bin/Config.cgi?action=set&property=Net.WIFI_AP.CryptoKey&value=\(pwd)*/
static int32_t CGI_SetCryptoKey(httpd_conn *hc, char *str)
{
    CVI_LOGI("enter CVI_CGI_SetSSIDandPassPhrase\n");
    int32_t result = 0;
    int32_t s32Ret = 0;
    PARAM_WIFI_S wifipar;

    s32Ret = PARAM_GetWifiParam(&wifipar);
    if (0 != s32Ret) {
        CVI_LOGI("Api passphrase cmd PARAM_GetWifiParam faile\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "Api passphrase cmd PARAM_GetWifiParam faile\r\n");
        return result;
    }

    memcpy(wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord, str, HAL_WIFI_PASSWORD_LEN);

    s32Ret = PARAM_SetWifiParam(&wifipar);
    if (0 != s32Ret) {
        CVI_LOGI("SetWiFipassphrase PARAM_SetWifiParam fail\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "SetWiFipassphrase PARAM_SetWifiParam fail\r\n");
        return result;
    }
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 获取wifi账号和密码, url = cgi-bin/Config.cgi?action=get&property=Net.WIFI_AP.CryptoKey*/
static int32_t CGI_GetCryptoKey(httpd_conn *hc, char *str)
{
    int32_t s32Ret = 0;
    int32_t result = 0;
    PARAM_WIFI_S wifipar;
    char *buf = NULL;
    char cgibuff[LBUFFSIZE];

    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    s32Ret = PARAM_GetWifiParam(&wifipar);

    if (0 != s32Ret) {
        CVI_LOGE("Api GetSSIDandpassphrase cmd PARAM_GetWifiParam faile\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "Api GetSSIDandpassphrase cmd PARAM_GetWifiParam faile\r\n");
        return result;
    }

    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, result, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Net.WIFI_AP.CryptoKey", wifipar.WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 对wifi密码进行操作*/
int32_t CGI_SetGetCryptoKey(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CVI_LOGE("enter CGI_SetGetCryptoKey, str = %s\n", str);
    if (str != NULL) {
        result = CGI_SetCryptoKey(hc, str);
    } else {
        result = CGI_GetCryptoKey(hc, str);
    }

    return result;
}

/*cgi 恢复出厂设置, url = cgi-bin/Config.cgi?action=set&property=FactoryReset&value=NO*/
int32_t CGI_ResetAllSettingTodeFault(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t sendresult = 0;
    int32_t s32Result = 0;
    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = PARAM_MENU_DEFAULT;
    sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
    FoRStat = "front";
    if (0 != sendresult || 0 != s32Result) {
        CVI_LOGE("netctrl MODEMNG_SetDefaultParam failed\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "netctrl CVI_PARAM_SetDefaultParam failed\r\n");
    } else {
        CGI_Currency(hc, NULL, NULL, result, NULL);
    }
    return result;
}

/*cgi 版本值获取, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.FWversion*/
int32_t CGI_GetProjectVersion(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CGI_Currency(hc, "Camera.Menu.FWversion", VERSION_NUM, result, NULL);
    return result;
}

/*cgi 设备的唯一标志, url = /cgi-bin/Config.cgi?action=get&property=Camera.menu.UUID*/
int32_t CGI_GetMenuUUID(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    char szCmd[192] = {'\0'};
    snprintf(szCmd, sizeof(szCmd), "%s", "mac=`ifconfig -a "HAL_WIFI_INTERFACE_NAME" | head -1 | awk '{ print $5 }' `;  echo ${mac}\n");
    char szMacAddrResult[24] = {0};
    FILE* fp = popen(szCmd, "r");
    if (NULL == fp) {
        perror("popen failed\n");
    } else {
        fgets(szMacAddrResult, sizeof(szMacAddrResult), fp);
        pclose(fp);
    }
    szMacAddrResult[strlen(szMacAddrResult) - 1] = '\0';
    CGI_Currency(hc, "Camera.menu.UUID", szMacAddrResult, result, NULL);
    client_ip = inet_ntoa(hc->client_addr.sa_in.sin_addr);
    CVI_LOGI("enter ip = %s\n", client_ip);
    EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(EVENT_S));
    stEvent.topic = EVENT_NETCTRL_CONNECT;
    EVENTHUB_Publish(&stEvent);
    return result;
}

/*cgi 获取设备信息, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.devid*/
int32_t CGI_GetDevid(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

// /*cgi.27 获取文件清单, url = /cgi-bin/Config.cgi/?action=dir&property=ALL&format=all&count=20&from=0*/
// int32_t CGI_ListAllFileCreatTime(httpd_conn *hc, char *str)
// {
//     CVI_LOGE("enter CVI_CGI_Listallfilecreattime\n");
//     flag_file = 1;
//     char *buf = NULL;
//     char nowtime[24];
//     char fpathname[255];
//     int32_t result = 0;
//     int32_t vbufflen = 0;
//     int32_t pbufflen = 0;
//     int32_t totalbufflen = 0;
//     FILEMNG_DIR_E aenDirs = 0;
//     uint32_t u32ModeState = 0;
//     int32_t s32Result = 0;
//     int32_t sendresult = 0;
//     uint32_t pu32FileObjCnt = 0;
//     MESSAGE_S Msg = {0};
//     struct stat filebuf;
//     struct tm *lt;

//     if (NULL == buf) {
//         buf = (char *)malloc(BUFFSIZE);
//     }

//     MODEMNG_GetModeState(&u32ModeState);

//     if (u32ModeState != MEDIA_MOVIE_STATE_VIEW) {
//         CVI_LOGE("on_startrec_click \n");
//         Msg.topic = EVENT_MODEMNG_STOP_REC;
//         sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
//         usleep(100 * 1000);
//     }
//     if (strcmp(str, "Normal") == 0) {
//         aenDirs = 2;
//     } else if (strcmp(str, "Event") == 0) {
//         aenDirs = 0;
//     } else if (strcmp(str, "Photo") == 0) {
//         aenDirs = 12;
//     } else if (strcmp(str, "Normal_rear") == 0) {
//         aenDirs = 8;
//     } else if (strcmp(str, "Event_rear") == 0) {
//         aenDirs = 6;
//     } else if (strcmp(str, "Photo_rear") == 0) {
//         aenDirs = 13;
//     } else {
//         return 0;
//     }

//     char dir[16] = "<" ;
//     char dir_l[16] = "</" ;
//     strcat(dir, str);strcat(dir_l, str);
//     strcat(dir, ">\r\n");strcat(dir_l, ">\r\n");
//     FILEMNG_SetSearchScope(&aenDirs, 1, &pu32FileObjCnt);
//     vbufflen = pu32FileObjCnt;
//     totalbufflen = (sizeof(DEF_XML_HEAD) + sizeof(dir) * 2 +sizeof("list\r\n") * 2 + (vbufflen + pbufflen) * BUFFSIZE);
//     if(cgibuff_file != NULL && file_size < totalbufflen) {
//         free(cgibuff_file);
//         cgibuff_file = malloc(totalbufflen);
//         file_size = totalbufflen;
//     }
//     memset(cgibuff_file, 0, file_size);
//     CGI_Snprintf(cgibuff_file, buf, sizeof(DEF_XML_HEAD), DEF_XML_HEAD);
//     CGI_Snprintf(cgibuff_file, buf, sizeof(dir), dir);
//     FILEMNG_SetSearchScope(&aenDirs, 1, &pu32FileObjCnt);

//     for(uint32_t i = 0; i < pu32FileObjCnt; i++) {
//         FILEMNG_GetFileByIndex(i, fpathname, 255);
//         if (NULL == fpathname) {
//             CVI_LOGE("fpathname is null");
//             result = WIFIAPP_FALSE;
//             return result;
//         }
//         result = stat(fpathname, &filebuf);
//         lt = localtime(&filebuf.st_ctime);
//         memset(nowtime, 0, sizeof(nowtime));
//         strftime(nowtime, 24, "%Y-%m-%d %H:%M:%S", lt);
//         if (WIFIAPP_OK != result) {
//             CGI_Currency(hc, NULL, NULL, result, NULL);
//             return result;
//         } else {
//             char *format = NULL;
//             format = (strrchr(fpathname, '.') + 1);
//             CGI_Snprintf(cgibuff_file, buf, BUFFSIZE, DEF_CGI_LISTAL_FILE, fpathname, format, filebuf.st_size, "RW", nowtime);
//         }
//     }

//     if (0 != sendresult || 0 != s32Result) {
//         CVI_LOGE("send message fail");
//         result = WIFIAPP_FALSE;
//         return result;
//     }

//     CGI_Snprintf(cgibuff_file, buf, BUFFSIZE, DEF_CGI_CNT_FILE, pu32FileObjCnt);
//     CGI_Snprintf(cgibuff_file, buf, sizeof(dir_l), dir_l);
//     NET_AddResponse(hc, cgibuff_file, strlen(cgibuff_file));

//     if (NULL != buf) {
//         free(buf);
//         buf =NULL;
//     }

//     return result;
// }

// Command : http://192.168.1.1/thumb/Normal/F/FILE211229-163306F.MOV
int32_t CGI_GetScreennail(httpd_conn *hc, char *str)
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
        CGI_Currency(hc, NULL, NULL, WIFIAPP_FALSE, NULL);
        return WIFIAPP_FALSE;
    }

    if (0 != ret) {
        CGI_Currency(hc, NULL, NULL, WIFIAPP_FALSE, NULL);
    } else {
        NET_AddResponse(hc, (char *)(packet.data), packet.size);
    }

    THUMBNAIL_EXTRACTOR_Destroy(&viewer_handle);
    THUMBNAIL_EXTRACTOR_ClearPacket(&packet);
#endif
    return ret;
}

/*cgi 获取可运行设备编号, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.DeviceCapability*/
int32_t CGI_DeviceCapability(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 设置当前时间, url = /cgi-bin/Config.cgi?action=set&property=TimeSettings&value=2023$10$17$19$24$01*/
int32_t CGI_SetSystemTime(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    char sExecStr[LBUFFSIZE];
    char datebuf[LBUFFSIZE] = "date -s ";

    char date[32] = {0};
    strcat(date,"\"");
    strcat(date,str);
    int32_t cnt = 0;
    for(size_t i=0;i<strlen(date)+1;i++){
        if (date[i] == '$') {
            if (cnt < 2) {
                date[i] = '-';
            } else if (cnt == 2) {
                date[i] = ' ';
            } else {
                date[i] = ':';
            }
            cnt++;
        }
    }

    memset(sExecStr, 0, sizeof(sExecStr));
    strcat(datebuf, date);
    strcat(datebuf, "\"");
    sprintf(sExecStr, "%s", datebuf);
    system(sExecStr);
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 电量信息, url = /cgi-bin/Config.cgi?action=get&property=Camera.Battery.Level*/
int32_t CGI_BatteryLevel(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 获取当前设备的状态, url = /cgi-bin/Config.cgi?action=get&property=Camera.Preview.MJPEG.status.**/
int32_t CGI_GetMJPEGStatus(httpd_conn *hc, char *str)
{
    int32_t result = 0;

    //获取录像状态
    uint32_t u32ModeState = 0;
    MODEMNG_GetModeState(&u32ModeState);
    if ((u32ModeState != MEDIA_MOVIE_STATE_REC) &&
        (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC)) {
        CGI_Currency(hc, "Camera.Preview.MJPEG.status.record", "Standy", result, NULL);
    } else if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
            (u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
        CGI_Currency(hc, "Camera.Preview.MJPEG.status.record", "Recording", result, NULL);
    }

    return result;
}

/*cgi 获取当前设备的状态, url = /cgi-bin/Config.cgi?action=get&property=Camera.Preview.**/
int32_t CGI_GetPreviewStatus(httpd_conn *hc, char *str)
{
    int32_t result = 0;

    char *buf = NULL;
    char *way = "4";
    uint32_t enSns = 1;

    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    if (((param.ViewWin.Current >> 1) & 0x1) == 1 ) {
        enSns = 2;
    }

    if (enSns == 1) {
        way = "4";
    } else {
        way = "4/5";
    }
    char cgibuff[LBUFFSIZE];
    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Preview.RTSP.av", way);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Preview.Source.1.Camid", FoRStat);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 打开录像, url = /cgi-bin/Config.cgi?action=set&property=Video&value=recordon*/
int32_t CGI_Startmovierecord(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    uint32_t u32ModeState = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};
    MODEMNG_GetModeState(&u32ModeState);

    int32_t CardStatus = MODEMNG_GetCardState();
    if (CardStatus != CARD_STATE_AVAILABLE) {
        CVI_LOGE("sd card no insert");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, NULL);
        return result;
    }

   if ((u32ModeState != MEDIA_MOVIE_STATE_REC) &&
        (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC)) {
        Msg.topic = EVENT_MODEMNG_START_REC;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg,&s32Result);
    }

    if (0 != sendresult || 0 != s32Result) {
        CVI_LOGE("send message fail");
        result = WIFIAPP_FALSE;
    }

    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 关闭录像, url = /cgi-bin/Config.cgi?action=set&property=Video&value=recordoff*/
int32_t CGI_Stopmovierecord(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    uint32_t u32ModeState = 0;
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};
    MODEMNG_GetModeState(&u32ModeState);

    int32_t CardStatus = MODEMNG_GetCardState();
    if (CardStatus != CARD_STATE_AVAILABLE) {
        CVI_LOGE("sd card no insert");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, NULL);
        return result;
    }

   if ((u32ModeState == MEDIA_MOVIE_STATE_REC) ||
        (u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
        Msg.topic = EVENT_MODEMNG_STOP_REC;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);
    }

    if (0 != sendresult || 0 != s32Result) {
        CVI_LOGE("send message fail");
        result = WIFIAPP_FALSE;
    }

    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 拍照, url = /cgi-bin/Config.cgi?action=set&property=Video&value=capture*/
int32_t CGI_getpictureend(httpd_conn *hc, char *str)
{
    int32_t result = 0;

    if (MODEMNG_GetCardState() == 0) {
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, " No SD\r\n");
        return result;
    }

    MESSAGE_S Msg = {0};
    int32_t s32Result = 0;
    int32_t sendresult = 0;
    Msg.topic = EVENT_MODEMNG_START_PIV;
	sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

    if (0 != sendresult || 0 != s32Result) {
        CVI_LOGE("send message fail");
        result = WIFIAPP_FALSE;
    }

    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 更改录像, url = /cgi-bin/Config.cgi?action=set&property=Video&value=*/
int32_t CGI_MovieRecord(httpd_conn *hc, char *str)
{
    int32_t result = 0;

    if (strcmp(str, "recordon") == 0) {
        CGI_Startmovierecord(hc, str);
    } else if (strcmp(str, "recordoff") == 0) {
        CGI_Stopmovierecord(hc, str);
    } else if (strcmp(str, "capture") == 0) {
        CGI_getpictureend(hc, str);
    } else {
        result = WIFIAPP_FALSE;
    }

    CGI_Currency(hc, NULL, NULL, result, NULL);
    return result;
}

/*cgi ota版本号, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.otaversion*/
int32_t CGI_OtaVersion(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi SDInfo, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.SDInfo*/
int32_t CGI_SDInfo(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    char *CarStat = NULL;
    char *buf = NULL;
    char cgibuff[LBUFFSIZE];
    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    if (MODEMNG_GetCardState() == CARD_STATE_AVAILABLE) {
        CarStat = "ON";
    } else {
        CarStat = "OFF";
    }

    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, "Get SD Status\n");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "SD0INFO", "1");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.SDInfo", CarStat);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi CardInfo, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.CardInfo.**/
int32_t CGI_CardInfo(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    char *buf = NULL;
    char cgibuff[LBUFFSIZE];
    char total[16];
    char remain[16];
    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");

    unsigned long FreePicNum = 0;
    FreePicNum = MODEMNG_GetCardState();
    CVI_LOGE("FreePicNum = %ld\n", FreePicNum);
    if (FreePicNum == CARD_STATE_AVAILABLE) {
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.CardStatus", "NORMAL");
        unsigned long long rspace = 0;
        unsigned long long totalspace = 0;
        struct statfs diskInfo;
        STG_DEVINFO_S sd_param = {0};
        PARAM_GetStgInfoParam(&sd_param);
        statfs(sd_param.aszMntPath, &diskInfo);
        unsigned long long blocksize = diskInfo.f_bsize;
        rspace = diskInfo.f_bfree * blocksize;
        totalspace = diskInfo.f_blocks * blocksize;
        totalspace = (totalspace >> 30); //0: B 10: KB 20: MB 30: GB
        if (totalspace <= 16) totalspace = 16;
        else if (totalspace <= 32) totalspace = 32;
        else totalspace = 64;
        rspace = (rspace >> 30); //0: B 10: KB 20: MB 30: GB
        CVI_LOGE("totalspace = %lld, rspace = %lld", totalspace, rspace);
        sprintf(total, " %lld" , totalspace);
        sprintf(remain, " %lld" , rspace);
        strcat(total, "G"); strcat(remain, "G");
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.total", total);
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.remain", remain);
    } else if (FreePicNum > 0) {
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.CardStatus", "NEED_FORMAT");
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.total", "0");
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.remain", "0");
    } else {
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.CardStatus", "NONE");
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.total", "0");
        CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.CardInfo.remain", "0");
    }
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi EmmcInfo, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.EmmcInfo.**/
int32_t CGI_EmmcInfo(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    // CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}


/*cgi 是否可以录像, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.VideoresCapability*/
int32_t CGI_VideoresCapability(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    CGI_Currency(hc, "Camera.Menu.VideoresCapability", "ON", result, NULL);

    return result;
}

/*cgi 获取相机目录, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.**/
int32_t CGI_GetCamMenu(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);

    //分辨率
    char *resolution;
    uint32_t videosize = 0;
    PARAM_GetVideoSizeEnum(menuparam.VideoSize.Current, &videosize);
    if (videosize == 0) resolution = "720p";
    else if (videosize == 2) resolution = "1080p";
    else resolution = "1440p";

    //循环播放时间
    char *LoopingVideo;
    if (menuparam.VideoLoop.Current == 0) LoopingVideo = "1MIN";
    else if (menuparam.VideoLoop.Current == 1) LoopingVideo = "3MIN";
    else  LoopingVideo = "5MIN";

    //曝光
    char *EV = NULL;
    double value = 0;

    MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
    int32_t status = 0;
    MAPI_VCAP_GetSensorPipeAttr(Syshdl->sns[0], &status);
    if (0 == status) {
        CVI_LOGE("stViPipeAttr.bYuvBypassPath is true, yuv sensor skip isp ops");
    } else {
        ISP_EXP_INFO_S expInfo;
        memset(&expInfo,0,sizeof(ISP_EXP_INFO_S));
        CVI_ISP_QueryExposureInfo(0, &expInfo);
        value = expInfo.u32ExpTime;
    }

    if (EV_Manual == 0) {
        EV = "EV0";
        EV_value = value;
    } else {
        double scale = EV_value / value;
        if (scale > 3.5) EV = "EVN200";
        else if (scale >= 2.8 && scale < 3.5) EV = "EVN167";
        else if (scale >= 2.2 && scale < 2.8) EV = "EVN133";
        else if (scale >= 1.8 && scale < 2.2) EV = "EVN100";
        else if (scale >= 1.4 && scale < 1.8) EV = "EVN067";
        else if (scale >= 1.1 && scale < 1.4) EV = "EVN033";
        else if (scale >= 0.9 && scale < 1.1) EV = "EV0";
        else if (scale >= 0.7 && scale < 0.9) EV = "EVP033";
        else if (scale >= 0.55 && scale < 0.7) EV = "EVP067";
        else if (scale >= 0.45 && scale < 0.55) EV = "EVP100";
        else if (scale >= 0.36 && scale < 0.45) EV = "EVP133";
        else if (scale >= 0.28 && scale < 0.36) EV = "EVP167";
        else EV = "EVP200";
    }

    //GSensor灵敏度
    char *gsensor = "OFF";
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    if (pstParamCtx->bInit == true) {
        if (pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity == 0) gsensor = "OFF";
        else if (pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity == 1) gsensor = "Low";
        else if (pstParamCtx->pstCfg->DevMng.Gsensor.enSensitity == 2) gsensor = "Meddle";
        else gsensor = "High";
    }

    //录音开关
    char *SoundRecord = "ON";
    if (menuparam.AudioEnable.Current == 0) SoundRecord = "OFF";
    else SoundRecord = "ON";

    //前后路
    char *enSns = "OFF";
    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    if (((param.ViewWin.Current >> 1) & 0x1) == 1 ) {
        enSns = "ON";
    }

    //sd卡状态
    char *CarStat = NULL;
    if (MODEMNG_GetCardState() == CARD_STATE_AVAILABLE) {
        CarStat = "ON";
    } else {
        CarStat = "OFF";
    }

     //初始化buf
    char *buf = NULL;
    char cgibuff[LBUFFSIZE*14];
    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.VideoRes", resolution);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.LoopingVideo", LoopingVideo);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.EV", EV);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.GSensor", gsensor);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.FWversion", VERSION_NUM);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.SoundRecord", SoundRecord);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.menuXML", "ON");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.RearStarus", enSns);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.SDInfo", CarStat);
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 设置曝光值, /cgi-bin/Config.cgi?action=set&property=EV&value=…(EV0)*/
int32_t CGI_SetEV(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t value = 0;
    if (strcmp(str,"EVN200") == 0)      value = EV_value / 3.98;
    else if (strcmp(str,"EVN167") == 0) value = EV_value / 3.16;
    else if (strcmp(str,"EVN133") == 0) value = EV_value / 2.5;
    else if (strcmp(str,"EVN100") == 0) value = EV_value / 1.99;
    else if (strcmp(str,"EVN067") == 0) value = EV_value / 1.58;
    else if (strcmp(str,"EVN033") == 0) value = EV_value / 1.26;
    else if (strcmp(str,"EV0") == 0) value = EV_value;
    else if (strcmp(str,"EVP033") == 0) value = EV_value * 1.26;
    else if (strcmp(str,"EVP067") == 0) value = EV_value * 1.58;
    else if (strcmp(str,"EVP100") == 0) value = EV_value * 1.99;
    else if (strcmp(str,"EVP133") == 0) value = EV_value * 2.5;
    else if (strcmp(str,"EVP167") == 0) value = EV_value * 3.16;
    else if (strcmp(str,"EVP200") == 0) value = EV_value * 3.98;
    else {
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, WIFIAPP_FALSE, NULL);
        return result;
    }
    EV_Manual = 1;
    VI_PIPE ViPipe = 0;
    ISP_EXPOSURE_ATTR_S  expAttr;
    CVI_ISP_GetExposureAttr(ViPipe, &expAttr);
    expAttr.enOpType = OP_TYPE_MANUAL;
    expAttr.bByPass = 0;
    expAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
    expAttr.stManual.u32ExpTime = value;
    CVI_ISP_SetExposureAttr(ViPipe, &expAttr);
    CGI_Currency(hc, NULL, NULL, result, NULL);
    return 0;
}

/*cgi 声音录制的开关的设置, /cgi-bin/Config.cgi?action=set&property=SoundRecord&value=…(ON, OFF)*/
int32_t CGI_SetEdisableMovieAudio(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    int32_t pstr = 0;

    if (strcmp(str, "OFF")==0) pstr = 0;
    else if (strcmp(str, "ON")==0) pstr = 1;
    else{
        CVI_LOGE("error input\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "error input\r\n");
        return result;
    }

    int32_t s32Result = 0;
    int32_t sendresult = 0;
    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);

    if (menuparam.AudioEnable.Current != (uint32_t)pstr) {
        Msg.arg2 = pstr;
        sendresult = NETCTRLINNER_SendSyncMsg(&Msg, &s32Result);

        if (0 != sendresult || 0 != s32Result) {
            CVI_LOGE("send message fail");
            result = WIFIAPP_FALSE;
            CGI_Currency(hc, NULL, NULL, result, "send message fail\r\n");
            return result;
        } else {
            NETCTRLINNER_UiUpdate();
        }
    }
    CGI_Currency(hc, NULL, NULL, result, NULL);

    return result;
}

/*cgi 获取录音开关, url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.SoundRecord*/
int32_t CGI_GetEdisableMovieAudio(httpd_conn *hc, char *str)
{
    int32_t s32Ret = 0;
    int32_t result = 0;
    PARAM_MENU_S menuparam;
    s32Ret = PARAM_GetMenuParam(&menuparam);

    if (0 != s32Ret) {
        CVI_LOGE(" %s ,fail  \n", __func__);
        CVI_LOGE("Api PARAM_GetMenuParam faile\n");
        result = WIFIAPP_FALSE;
        CGI_Currency(hc, NULL, NULL, result, "Api PARAM_GetMenuParam faile\r\n");
        return result;
    }

    char *ret= NULL;
    if (menuparam.AudioEnable.Current == 0) ret = "OFF";
    else ret = "ON";
    CGI_Currency(hc, "Camera.Menu.SoundRecord", ret, result, NULL);

    return result;
}

/*cgi 心跳包, url = /cgi-bin/Config.cgi?action=set&property=Heartbeat*/
int32_t CGI_Heartbeat(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(EVENT_S));
    stEvent.topic = EVENT_NETCTRL_CONNECT;
    EVENTHUB_Publish(&stEvent);
    CGI_Currency(hc, NULL, NULL, result, NULL);
    return result;
}

/*cgi APP获取设置列表, url = /cammenu.xml*/
int32_t CGI_Cammenu(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    char *buf = NULL;
    char cgibuff[LBUFFSIZE*25] =
    "<?xml version=\"1.0\"?>\r\n\
    <camera xmlns=\"urn:schemas-ait-dvr:device-1-0\">\r\n\
	<version>0.1</version>\r\n\
	<menu title=\"VIDEO RESOLUTION\" id=\"Videores\">\r\n\
		<item id=\"1440P25\">1440P</item>\r\n\
        <item id=\"1080P25\">1080P</item>\r\n\
	</menu>\r\n\
	<menu title=\"LOOPING VIDEO\" id=\"LoopingVideo\">\r\n\
		<!-- <item id=\"OFF\">Off</item> -->\r\n\
		<item id=\"1MIN\">1 min</item>\r\n\
		<item id=\"3MIN\">3 min</item>\r\n\
		<item id=\"5MIN\">5 min</item>\r\n\
	</menu>\r\n\
	<menu title=\"EXPOSURE VALUE\" id=\"EV\">\r\n\
		<item id=\"EVN200\">-2</item>\r\n\
		<item id=\"EVN167\">-1.67</item>\r\n\
		<item id=\"EVN133\">-1.33</item>\r\n\
		<item id=\"EVN100\">-1</item>\r\n\
		<item id=\"EVN067\">-0.67</item>\r\n\
		<item id=\"EVN033\">-0.33</item>\r\n\
		<item id=\"EV0\">0</item>\r\n\
		<item id=\"EVP033\">0.33</item>\r\n\
		<item id=\"EVP067\">0.67</item>\r\n\
		<item id=\"EVP100\">1</item>\r\n\
		<item id=\"EVP133\">1.33</item>\r\n\
		<item id=\"EVP167\">1.67</item>\r\n\
		<item id=\"EVP200\">2</item>\r\n\
	</menu>\r\n\
    <menu title=\"GSENSOR SENSITIVITY\" id=\"GSensor\">\r\n\
		<item id=\"OFF\">OFF</item>\r\n\
        <item id=\"Low\">Low</item>\r\n\
        <item id=\"Middle\">Middle</item>\r\n\
        <item id=\"High\">High</item>\r\n\
	</menu>\r\n\
	<menu title=\"RECORD WITH AUDIO\" id=\"SoundRecord\">\r\n\
		<item id=\"OFF\">Off</item>\r\n\
		<item id=\"ON\">ON</item>\r\n\
	</menu>\r\n\
	<menu title=\"SOUND INDICATOR\" id=\"SoundIndicator\">\r\n\
		<item id=\"OFF\">Off</item>\r\n\
		<item id=\"ON\">On</item>\r\n\
	</menu>\r\n\
	<menu title=\"SYNC TIME\" id=\"SyncTime\">\r\n\
	</menu>\r\n\
	<menu title=\"FORMAT SD\" id=\"SD0\">\r\n\
	</menu>	\r\n\
	<menu title=\"RESET MACHINE\" id=\"FactoryReset\">\r\n\
	</menu>	\r\n\
    </camera>\r\n";

    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);
    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 进入或退出设置界面，url = /cgi-bin/Config.cgi?action=set&property=Setting&value=*/
int32_t CGI_Setting(httpd_conn *hc, char *str)
{
    CGI_Currency(hc, NULL, NULL, 0, NULL);
    return 0;
}

/*cgi 进入或退出回放界面，url = /cgi-bin/Config.cgi?action=set&property=Playback&value=*/
int32_t CGI_Playback(httpd_conn *hc, char *str)
{
    int32_t result = 0;
	EVENT_S stEvent;
	memset(&stEvent, 0, sizeof(EVENT_S));
	stEvent.topic = EVENT_NETCTRL_APPCONNECT_SETTING;

    if (str != NULL) {
		str = strrchr(str, '=') + 1;
		if (strcmp(str, "exit") == 0) {
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

    CGI_Currency(hc, NULL, NULL, 0, NULL);
    return result;
}

/*cgi 查看有几路RTSP，url = /cgi-bin/Config.cgi?action=get&property=Camera.Preview.RTSP.av*/
int32_t CGI_PreviewRTSP(httpd_conn *hc, char *str)
{
    int32_t result = 0;

    char *buf = NULL;
    char *way = "4";
    uint32_t enSns = 1;

    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    if (((param.ViewWin.Current >> 1) & 0x1) == 1 ) {
        enSns = 2;
    }

    if (enSns == 1) {
        way = "4";
    } else {
        way = "4/5";
    }
    char cgibuff[LBUFFSIZE];
    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Preview.RTSP.av", way);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 查看后路状态，url = /cgi-bin/Config.cgi?action=get&property=Camera.Menu.RearStarus*/
int32_t CGI_RearStarus(httpd_conn *hc, char *str)
{
    int32_t result = 0;
    char *buf = NULL;
    char *RearStarus = "OFF";

    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    if (((param.ViewWin.Current >> 1) & 0x1) == 1 ) {
        RearStarus = "OK";
    }

    char cgibuff[LBUFFSIZE];
    if (NULL == buf) {
        buf = (char *)malloc(BUFFSIZE);
    }
    memset(cgibuff, 0, sizeof(cgibuff));
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_STS_TEST, 0, "OK");
    CGI_Snprintf(cgibuff, buf, LBUFFSIZE, DEF_CGI_LOG_TEST, "Camera.Menu.RearStarus", RearStarus);
    NET_AddCgiResponse(strlen(cgibuff), hc, "%s", cgibuff);

    if (NULL != buf) {
        free(buf);
        buf = NULL;
    }

    return result;
}

/*cgi 设置前后路状态，url = /cgi-bin/Config.cgi?action=setcamid&property=Camera.Preview.Source.1.Camid&value=front*/
int32_t CGI_SourceCamid(httpd_conn *hc, char *str)
{
    FoRStat = str;
    CGI_Currency(hc, NULL, NULL, 0, NULL);
    return 0;
}

/*cgi 开启或关闭串口流，url = /cgi-bin/Config.cgi?action=set&property=StreamStatus&value=ON*/
int32_t CGI_StreamStatus(httpd_conn *hc, char *str)
{
    CGI_Currency(hc, NULL, NULL, 0, NULL);
    EVENT_S stEvent;
    memset(&stEvent, 0, sizeof(EVENT_S));
    stEvent.topic = EVENT_NETCTRL_CONNECT;
    EVENTHUB_Publish(&stEvent);
    return 0;
}

void* app_socket_udp(void *arg)
{
    int32_t sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int32_t CamNumStatus = 0;
    int32_t camnum_status = -1;
    int32_t CardStatus = 0;
    int32_t card_status = -1;
    int32_t flag = 0;
    struct sockaddr_in sock_addr = {0};
	sock_addr.sin_family = AF_INET;                         // 设置地址族为IPv4
	sock_addr.sin_port = htons(49142);						// 设置地址的端口号信息
    sock_addr.sin_addr.s_addr = inet_addr(client_ip);
    int ret = 0;
    char sendbuf[128];
    bind(sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));

    while (flag_socket == WIFI_APP_CONNECTTED){
        memset(sendbuf, 0, sizeof(sendbuf));

        //检测摄像头个数
        PARAM_MENU_S param;
        PARAM_GetMenuParam(&param);
        CamNumStatus = param.ViewWin.Current;
        if (camnum_status != CamNumStatus) {
            camnum_status = CamNumStatus;
            flag = 1;
            if ((((camnum_status >> 1) & 0x1) == 1 )) {
                strcat(sendbuf, "CAM_NUM_CHANGE = 2\r\n");
            } else {
                strcat(sendbuf, "CAM_NUM_CHANGE = 1\r\n");
            }
        }

        //检测SD卡状态
        CardStatus = MODEMNG_GetCardState();
        if (card_status != CardStatus) {
            card_status = CardStatus;
            flag = 1;
            if (card_status == CARD_STATE_AVAILABLE) {
                strcat(sendbuf, "SD_STATUS = ON\r\n");
            } else {
                strcat(sendbuf, "SD_STATUS = OFF\r\n");
            }
        }

        if (flag == 1) {
            ret = sendto(sockfd, sendbuf, sizeof(sendbuf), 0, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
            CVI_LOGI("enter ret = %d, sendbuf = %s\n", ret, sendbuf);
            flag = 0;
        }

        usleep(500*1000);
    }
    close(sockfd);

    return (void *)0;
}

static int32_t custom_register_cgi(const char *cmd, void *cb)
{
    int32_t ret = 0;
    NET_WIFIAPPMAPTO_CGI_S stCgiCmd;

    if (cmd != NULL) {
        snprintf(stCgiCmd.cmd, strlen(cmd)+1, cmd);
    } else {
        CVI_LOGE("error, cmd is NULL\n");
        ret = -1;
        goto EXIT;
    }
    if (cb == NULL) {
        ret = -1;
        goto EXIT;
    }
    stCgiCmd.callback = (NET_SYSCALL_CMD_TO_CALLBACK)cb;
    NET_RegisterCgiCmd_CGI(&stCgiCmd);

EXIT:
    return ret;
}

void NETCTRLINNER_CMDRegister(void)
{
    CVI_LOGE("enter: %s \n", __func__);
    custom_register_cgi("Videores", (void *)CGI_SetMovieRecord);                           /*cgi 视频分辨率的设置和获取*/
    custom_register_cgi("LoopingVideo", (void *)CGI_SetCyclicRecordValue);                 /*cgi 录影分段时间设置和获取*/
    custom_register_cgi("GSensor", (void *)CGI_SetGSensorSensitivity);                     /*cgi 碰撞锁定的灵敏度的设置和获取*/
    custom_register_cgi("VideoTimeLapse", (void *)CGI_SetVideoTimeLapse);                  /*cgi 缩时录音的设置和获取*/
    custom_register_cgi("Exposure", (void *)CGI_SetMovieEvValue);                          /*cgi 曝光设定的设置和获取*/
    custom_register_cgi("SD0", (void *)CGI_CommandWouldFormatStorage);                     /*cgi 格式化SD卡*/
    custom_register_cgi("DEL", (void *)CGI_DeleteOneFile);                                 /*cgi 删除文件*/
    custom_register_cgi("Net.WIFI_AP.SSID", (void *)CGI_SetGetSSID);                       /*cgi 对wifi名称进行操作*/
    custom_register_cgi("Net.WIFI_AP.CryptoKey", (void *)CGI_SetGetCryptoKey);             /*cgi 对wifi密码进行操作*/
    custom_register_cgi("FactoryReset", (void *)CGI_ResetAllSettingTodeFault);             /*cgi 恢复出厂设置*/
    custom_register_cgi("Camera.Menu.FWVersion", (void *)CGI_GetProjectVersion);           /*cgi 版本值获取*/
    custom_register_cgi("Camera.menu.UUID", (void *)CGI_GetMenuUUID);                      /*cgi 设备的唯一标志*/
    custom_register_cgi("Preview.MJPEG.status.*", (void *)CGI_GetMJPEGStatus);             /*cgi 获取当前设备的状态*/
    custom_register_cgi("Camera.Preview.*", (void *)CGI_GetPreviewStatus);                 /*cgi 获取当前设备的状态*/
    custom_register_cgi("Camera.Menu.*", (void *)CGI_GetCamMenu);                          /*cgi 获取相机目录*/
    custom_register_cgi("SoundRecord", (void *)CGI_SetEdisableMovieAudio);                 /*cgi 设置录音开关*/
    custom_register_cgi("Camera.Menu.SoundRecord", (void *)CGI_GetEdisableMovieAudio);     /*cgi 获取录音开关*/
    custom_register_cgi("Heartbeat", (void *)CGI_Heartbeat);                               /*cgi 心跳包*/
    custom_register_cgi("Video", (void *)CGI_MovieRecord);                                 /*cgi 更改录像*/
    custom_register_cgi("Camera.Menu.otaversion", (void *)CGI_OtaVersion);                 /*cgi ota版本号*/
    custom_register_cgi("Camera.Menu.SDInfo", (void *)CGI_SDInfo);                         /*cgi SDInfo*/
    custom_register_cgi("Camera.Menu.CardInfo.*", (void *)CGI_CardInfo);                   /*cgi CardInfo*/
    custom_register_cgi("Camera.Menu.EmmcInfo.*", (void *)CGI_EmmcInfo);                   /*cgi EmmcInfo*/
    custom_register_cgi("Camera.Menu.VideoresCapability", (void *)CGI_VideoresCapability); /*cgi 是否可以录像*/
    custom_register_cgi("Camera.Menu.devid", (void *)CGI_GetDevid);                        /*cgi 获取设备信息*/
    // custom_register_cgi("dir", (void *)CGI_ListAllFileCreatTime);                          /*cgi 获取文件清单*/
    custom_register_cgi("thumb", (void *)CGI_GetScreennail);                               /*cgi 获取文件缩略图*/
    custom_register_cgi("Camera.Menu.DeviceCapability", (void *)CGI_DeviceCapability);     /*cgi 获取可运行设备编号*/
    custom_register_cgi("Camera.Battery.Level", (void *)CGI_BatteryLevel);                 /*cgi 电量信息*/
    custom_register_cgi("TimeSettings", (void *)CGI_SetSystemTime);                        /*cgi 设置当前时间*/
    custom_register_cgi("NULL", (void *)CGI_Heartbeat);                                    /*cgi 空函数，不回复*/
    custom_register_cgi("EV", (void *)CGI_SetEV);                                          /*cgi 设置曝光量*/
    custom_register_cgi("cammenu", (void *)CGI_Cammenu);                                   /*cgi 设置参数界面*/
    custom_register_cgi("Setting", (void *)CGI_Setting);                                   /*cgi 进入或退出设置界面*/
    custom_register_cgi("Playback", (void *)CGI_Playback);                                 /*cgi 进入或退出回放界面*/
    custom_register_cgi("Camera.Preview.RTSP.av", (void *)CGI_PreviewRTSP);                /*cgi 查看有几路RTSP*/
    custom_register_cgi("Camera.Menu.RearStarus", (void *)CGI_RearStarus);                 /*cgi 查看后路状态*/
    custom_register_cgi("Camera.Preview.Source.1.Camid", (void *)CGI_SourceCamid);         /*cgi 设置前后路*/
    custom_register_cgi("StreamStatus", (void *)CGI_StreamStatus);                         /*cgi 开启或关闭串口流*/
    custom_register_cgi("Camera.Menu.SD0", (void *)CGI_SDInfo);
}

int32_t NETCTRLINNER_InitCMDSocket(void)
{
    pthread_mutex_lock(&gMutex);
    flag_socket = WIFI_APP_CONNECTTED;
    if (g_websocket_thread == 0) {
        pthread_create(&g_websocket_thread, NULL, app_socket_udp, NULL);
    }
    if (cgibuff_file == NULL) {
        cgibuff_file = malloc(file_size);
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
    if (cgibuff_file != NULL)
    {
        free(cgibuff_file);
        cgibuff_file = NULL;
    }
    flag_file = 0;          //复位文件异常退出检测标志
    pthread_mutex_unlock(&gMutex);
    return 0;
}

int32_t NETCTRLINNER_GetFlagFile(void)
{
    return flag_file;
}
#include <string.h>
#include "param.h"
#include "param_ini2bin.h"
#include "ini.h"
#include "param_printf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

extern PARAM_ACCESS g_ParamAccess;

static int32_t PARAM_LoadStgInfo(const char *file, STG_DEVINFO_S *Stg)
{
    char tmp_port[16] = {0};
    char tmp_devpath[16] = {0};
    char tmp_mntpath[16] = {0};

    INI_GetString("storage", "dev_port", "", tmp_port, 16, file);
    INI_GetString("storage", "dev_path", "", tmp_devpath, 16, file);
    INI_GetString("storage", "mnt_path", "", tmp_mntpath, 16, file);

    printf("%s %s %s\n", tmp_port, tmp_devpath, tmp_mntpath);

    memcpy(Stg->aszDevPort, tmp_port, 16);
    memcpy(Stg->aszDevPath, tmp_devpath, 16);
    memcpy(Stg->aszMntPath, tmp_mntpath, 16);

    return 0;
}

static int32_t PARAM_LoadWifiInfo(const char *file, PARAM_WIFI_S *Wifi)
{
    char tmp_ssid[32] = {0};
    char tmp_password[64] = {0};
    long int enable = 0;
    long int mode = 0;
    long int ssid_hide = 0;
    long int channel = 0;

    enable = INI_GetLong("wifi_config", "enable", 0, file);
    mode = INI_GetLong("wifi_config", "mode", 0, file);
    ssid_hide = INI_GetLong("wifi_config", "ssid_hide", 0, file);
    channel = INI_GetLong("wifi_config", "channel", 0, file);
    INI_GetString("wifi_config", "ssid", "", tmp_ssid, 16, file);
    INI_GetString("wifi_config", "password", "", tmp_password, 16, file);

    printf("%s: %ld %ld %ld %ld password:%s\n", tmp_ssid, enable, mode, ssid_hide, channel, tmp_password);

    Wifi->Enable = enable;
    Wifi->WifiCfg.enMode = mode;
    if(mode == 1) {
        Wifi->WifiCfg.unCfg.stApCfg.bHideSSID = ssid_hide;
        Wifi->WifiCfg.unCfg.stApCfg.s32Channel = channel;
        memcpy(Wifi->WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID, tmp_ssid, 32);
        memcpy(Wifi->WifiDefaultSsid, tmp_ssid, 32);
        memcpy(Wifi->WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord, tmp_password, 64);
    } else {
        printf("now only suport ap mode");
    }

    return 0;
}

static int32_t PARAM_LoadPWMInfo(const char *file, PARAM_PWM_S *PWM)
{
    long int enable = 0;
    long int group = 0;
    long int channel = 0;
    long int period = 0;
    long int duty_cycle = 0;

    enable = INI_GetLong("pwm_config", "enable", 0, file);
    group = INI_GetLong("pwm_config", "group", 0, file);
    channel = INI_GetLong("pwm_config", "channel", 0, file);
    period = INI_GetLong("pwm_config", "period", 0, file);
    duty_cycle = INI_GetLong("pwm_config", "duty_cycle", 0, file);

    printf("PWM : group(%ld) channel(%ld) period(%ld) duty_cycle(%ld)\n", group, channel, period, duty_cycle);

    PWM->Enable = enable;
    PWM->PWMCfg.group = group;
    PWM->PWMCfg.channel = channel;
    PWM->PWMCfg.period = period;
    PWM->PWMCfg.duty_cycle = duty_cycle;

    return 0;
}

static int32_t PARAM_LoadGsensorInfo(const char *file, GSENSORMNG_CFG_S *Gsensor)
{
   int32_t  enable = 0;
    long int enSensitity = 0;
    long int sampleRate = 0;
   int32_t  level = 0;

    enable = INI_GetLong("gsensor_config", "enable", 0, file);
    enSensitity = INI_GetLong("gsensor_config", "enSensitity", 0, file);
    sampleRate = INI_GetLong("gsensor_config", "u32SampleRate", 0, file);
    level = INI_GetLong("gsensor_config", "level", 0, file);
    printf("enSensitity = %ld u32SampleRate = %ld level = %d \n", enSensitity, sampleRate, level);

    Gsensor->gsensor_enable = enable;
    Gsensor->enSensitity = enSensitity;
    Gsensor->gsensor_level = level;
    Gsensor->stAttr.u32SampleRate = sampleRate;

    return 0;
}

static int32_t PARAM_Load_KeyInfo(const char *file, KEYMNG_CFG_S *key)
{
    long int key_cnt = 0;
    char typebuffer[64] = {0};
    char idbuffer[64] = {0};
    char enablebuffer[64] = {0};
    char timebuffer[64] = {0};
    char giridbuffer[64] = {0};

    key_cnt = INI_GetLong("keymng.key", "key_cnt", 0, file);
    key->stKeyCfg.u32KeyCnt = key_cnt;
    for (int32_t  i = 0; i < key_cnt; i++) {
        memset(typebuffer, 0, sizeof(typebuffer));
        memset(idbuffer, 0, sizeof(idbuffer));
        memset(enablebuffer, 0, sizeof(enablebuffer));
        memset(timebuffer, 0, sizeof(timebuffer));
        snprintf(typebuffer, sizeof(typebuffer), "key_type%d", i);
        snprintf(idbuffer, sizeof(idbuffer), "key_id%d", i);
        snprintf(enablebuffer, sizeof(enablebuffer), "longkey_enable%d", i);
        snprintf(timebuffer, sizeof(timebuffer), "longkey_time%d", i);

        key->stKeyCfg.astKeyAttr[i].enType = INI_GetLong("keymng.key", typebuffer, 0, file);
        key->stKeyCfg.astKeyAttr[i].s32Id = INI_GetLong("keymng.key", idbuffer, 0, file);
        key->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.bLongClickEnable = INI_GetLong("keymng.key", enablebuffer, 0, file);
        key->stKeyCfg.astKeyAttr[i].unAttr.stClickKeyAttr.u32LongClickTime_msec = INI_GetLong("keymng.key", timebuffer, 0, file);
    }

    /* Key Configure */
    key->stGrpKeyCfg.bEnable = INI_GetLong("keymng.grpkey", "enable", 0, file);
    for (int32_t  i = 0; i < KEYMNG_KEY_NUM_EACH_GRP; i++) {
        memset(giridbuffer, 0, sizeof(giridbuffer));
        snprintf(giridbuffer, sizeof(giridbuffer), "key_idx%d", i);
        key->stGrpKeyCfg.au32GrpKeyIdx[i] = INI_GetLong("keymng.grpkey", giridbuffer, 0, file);
    }

    return 0;
}

static int32_t PARAM_LoadGaugeInfo(const char *file, GAUGEMNG_CFG_S *Gauge)
{
    long int LowLevel = 0;
    long int UltraLowLevel = 0;
    long int ADCChannelVbat = 0;
    long int USBChargerDetectGPIO = 0;

    LowLevel = INI_GetLong("gaugemng", "LowLevel", 0, file);
    UltraLowLevel = INI_GetLong("gaugemng", "UltraLowLevel", 0, file);
    ADCChannelVbat = INI_GetLong("gaugemng", "ADCChannelVbat", 0, file);
    USBChargerDetectGPIO = INI_GetLong("gaugemng", "USBChargerDetectGPIO", 0, file);
    printf("LowLevel = %ld UltraLowLevel = %ld, ADCChannelVbat=%ld, USBChargerDetectGPIO=%ld\n", LowLevel, UltraLowLevel, ADCChannelVbat, USBChargerDetectGPIO);

    Gauge->s32LowLevel = LowLevel;
    Gauge->s32UltraLowLevel = UltraLowLevel;
    Gauge->s32ADCChannelVbat = ADCChannelVbat;
    Gauge->s32USBChargerDetectGPIO = USBChargerDetectGPIO;

    return 0;
}

static int32_t PARAM_LoadFlashLedInfo(const char *file, PARAM_FLASH_LED_S *FlashLed)
{
    long int gpio_num = 0;
    long int pulse = 0;
    long int thres = 0;
    gpio_num = INI_GetLong("flash_led_config", "gpio_num", 0, file);
    pulse = INI_GetLong("flash_led_config", "pulse", 0, file);
    thres = INI_GetLong("flash_led_config", "thres", 0, file);
    printf("flash_led:%ld %ld %ld\n", gpio_num, pulse, thres);
    FlashLed->GpioNum = gpio_num;
    FlashLed->Pulse = pulse;
    FlashLed->Thres = thres;
    return 0;
}

static int32_t PARAM_LoadDevmng(const char *file, PARAM_DEVMNG_S *DevMng)
{
   int32_t  s32Ret = 0;

    s32Ret = PARAM_LoadStgInfo(file, &DevMng->Stg);
    s32Ret = PARAM_LoadWifiInfo(file, &DevMng->Wifi);
    s32Ret = PARAM_LoadPWMInfo(file, &DevMng->PWM);
    s32Ret = PARAM_Load_KeyInfo(file, &DevMng->stkeyMngCfg);
    s32Ret = PARAM_LoadGsensorInfo(file, &DevMng->Gsensor);
    s32Ret = PARAM_LoadGaugeInfo(file, &DevMng->GaugeCfg);
    s32Ret = PARAM_LoadFlashLedInfo(file, &DevMng->FlashLed);

    return s32Ret;
}

int32_t  INI_PARAM_LoadDevmngCfg(PARAM_DEVMNG_S *DevMng)
{
    printf("\n---enter: %s\n", __func__);

    uint32_t i = 0;
    char filepath[PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_devmng")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadDevmng(filepath, DevMng);
            break;
        }
    }

    return 0;
}

static const char *wifi_mode_to_str(HAL_WIFI_MODE_E mode)
{
    switch(mode) {
        case HAL_WIFI_MODE_STA: return "STA";
        case HAL_WIFI_MODE_AP: return "AP";
        default: return "UNKNOWN";
    }
}

static const char *wifi_sta_mode_to_str(HAL_WIFI_STA_MODE_E mode)
{
    switch(mode) {
        case HAL_WIFI_STA_MODE_COMMON: return "COMMON";
        case HAL_WIFI_STA_MODE_SENIOR: return "SENIOR";
        default: return "UNKNOWN";
    }
}

static const char *key_type_to_str(KEYMNG_KEY_TYPE_E type)
{
    switch(type) {
        case KEYMNG_KEY_TYPE_CLICK: return "CLICK";
        case KEYMNG_KEY_TYPE_HOLD: return "HOLD";
        default: return "UNKNOWN";
    }
}

static const char *stg_dev_state_to_str(STG_DEV_STATE_E state)
{
    switch(state) {
        case STG_DEV_STATE_UNPLUGGED: return "UNPLUGGED";
        case STG_DEV_STATE_CONNECTING: return "CONNECTING";
        case STG_DEV_STATE_CONNECTED: return "CONNECTED";
        case STG_DEV_STATE_IDLE: return "IDLE";
        default: return "UNKNOWN";
    }
}

static const char *stg_state_to_str(STG_STATE_E state)
{
    switch(state) {
        case STG_STATE_DEV_UNPLUGGED: return "DEV_UNPLUGGED";
        case STG_STATE_DEV_CONNECTING: return "DEV_CONNECTING";
        case STG_STATE_DEV_ERROR: return "DEV_ERROR";
        case STG_STATE_FS_CHECKING: return "FS_CHECKING";
        case STG_STATE_FS_CHECK_FAILED: return "FS_CHECK_FAILED";
        case STG_STATE_FS_EXCEPTION: return "FS_EXCEPTION";
        case STG_STATE_MOUNTED: return "MOUNTED";
        case STG_STATE_UNMOUNTED: return "UNMOUNTED";
        case STG_STATE_MOUNT_FAILED: return "MOUNT_FAILED";
        case STG_STATE_FORMATING: return "FORMATING";
        case STG_STATE_FORMAT_SUCCESSED: return "FORMAT_SUCCESSED";
        case STG_STATE_FORMAT_FAILED: return "FORMAT_FAILED";
        case STG_STATE_READ_ONLY: return "READ_ONLY";
        case STG_STATE_IDLE: return "IDLE";
        default: return "UNKNOWN";
    }
}

static const char *stg_fs_type_to_str(STG_FS_TYPE_E type)
{
    switch(type) {
        case STG_FS_TYPE_FAT32: return "FAT32";
        case STG_FS_TYPE_EXFAT: return "EXFAT";
        case STG_FS_TYPE_UNKNOWN: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

static const char *gsensor_sensitivity_to_str(HAL_GSENSOR_SENSITITY_E sen)
{
    switch(sen) {
        case HAL_GSENSOR_SENSITITY_OFF: return "OFF";
        case HAL_GSENSOR_SENSITITY_LOW: return "LOW";
        case HAL_GSENSOR_SENSITITY_MIDDLE: return "MIDDLE";
        case HAL_GSENSOR_SENSITITY_HIGH: return "HIGH";
        default: return "UNKNOWN";
    }
}

void print_param_devmng(const PARAM_DEVMNG_S *devmng)
{
    if(devmng == NULL) {
        printf("[DevMng] Error: NULL pointer received.\n");
        return;
    }
    printf("[DevMng]\n");
    printf("Stg:\n");
    printf("devState: %d (%s)\n", devmng->Stg.devState, stg_dev_state_to_str(devmng->Stg.devState));
    printf("stgState: %d (%s)\n", devmng->Stg.stgState, stg_state_to_str(devmng->Stg.stgState));
    printf("fsType: %s\n", devmng->Stg.fsType);
    printf("fsType_e: %d (%s)\n", devmng->Stg.fsType_e, stg_fs_type_to_str(devmng->Stg.fsType_e));
    printf("workMode: %s\n", devmng->Stg.workMode);
    printf("aszDevPort: %s\n", devmng->Stg.aszDevPort);
    printf("speedClass: %s\n", devmng->Stg.speedClass);
    printf("speedGrade: %s\n", devmng->Stg.speedGrade);
    printf("aszDevType: %s\n", devmng->Stg.aszDevType);
    printf("aszDevPath: %s\n", devmng->Stg.aszDevPath);
    printf("aszMntPath: %s\n", devmng->Stg.aszMntPath);
    printf("aszErrCnt: %s\n", devmng->Stg.aszErrCnt);
    printf("Wifi:\n");
    printf("Enable: %s\n", devmng->Wifi.Enable ? "ON" : "OFF");
    printf("WifiDefaultSsid: %s\n", devmng->Wifi.WifiDefaultSsid);
    printf("WifiCfg.enMode: %d (%s)\n", devmng->Wifi.WifiCfg.enMode, wifi_mode_to_str(devmng->Wifi.WifiCfg.enMode));
    if(devmng->Wifi.WifiCfg.enMode == HAL_WIFI_MODE_AP) {
        printf("WifiCfg.unCfg.stApCfg:\n");
        printf("bHideSSID: %s\n", devmng->Wifi.WifiCfg.unCfg.stApCfg.bHideSSID ? "TRUE" : "FALSE");
        printf("s32Channel: %d\n", devmng->Wifi.WifiCfg.unCfg.stApCfg.s32Channel);
        printf("stCfg.szWiFiSSID: %s\n", devmng->Wifi.WifiCfg.unCfg.stApCfg.stCfg.szWiFiSSID);
        printf("stCfg.szWiFiPassWord: %s\n", devmng->Wifi.WifiCfg.unCfg.stApCfg.stCfg.szWiFiPassWord);
    } else if(devmng->Wifi.WifiCfg.enMode == HAL_WIFI_MODE_STA) {
        printf("WifiCfg.unCfg.stStaCfg:\n");
        printf("enStaMode: %d (%s)\n", devmng->Wifi.WifiCfg.unCfg.stStaCfg.enStaMode,
               wifi_sta_mode_to_str(devmng->Wifi.WifiCfg.unCfg.stStaCfg.enStaMode));
        if(devmng->Wifi.WifiCfg.unCfg.stStaCfg.enStaMode == HAL_WIFI_STA_MODE_COMMON) {
            printf("unCfg.stCommonCfg.stCfg.szWiFiSSID: %s\n",
                   devmng->Wifi.WifiCfg.unCfg.stStaCfg.unCfg.stCommonCfg.stCfg.szWiFiSSID);
            printf("unCfg.stCommonCfg.stCfg.szWiFiPassWord: %s\n",
                   devmng->Wifi.WifiCfg.unCfg.stStaCfg.unCfg.stCommonCfg.stCfg.szWiFiPassWord);
        } else if(devmng->Wifi.WifiCfg.unCfg.stStaCfg.enStaMode == HAL_WIFI_STA_MODE_SENIOR) {
            printf("unCfg.stSeniorCfg.stCfg.szWiFiSSID: %s\n",
                   devmng->Wifi.WifiCfg.unCfg.stStaCfg.unCfg.stSeniorCfg.stCfg.szWiFiSSID);
            printf("unCfg.stSeniorCfg.stCfg.szWiFiPassWord: %s\n",
                   devmng->Wifi.WifiCfg.unCfg.stStaCfg.unCfg.stSeniorCfg.stCfg.szWiFiPassWord);
        }
    }
    printf("KeyMng:\n");
    printf("stKeyCfg.u32KeyCnt: %d\n", devmng->stkeyMngCfg.stKeyCfg.u32KeyCnt);
    for(uint32_t i = 0; i < devmng->stkeyMngCfg.stKeyCfg.u32KeyCnt; i++) {
        const KEYMNG_KEY_ATTR_S *key_attr = &devmng->stkeyMngCfg.stKeyCfg.astKeyAttr[i];
        printf("stKeyCfg.astKeyAttr[%d]:\n", i);
        printf("enType: %d (%s)\n", key_attr->enType, key_type_to_str(key_attr->enType));
        printf("s32Id: %d\n", key_attr->s32Id);
        if(key_attr->enType == KEYMNG_KEY_TYPE_CLICK) {
            printf("unAttr.stClickKeyAttr:\n");
            printf("bLongClickEnable: %s\n", key_attr->unAttr.stClickKeyAttr.bLongClickEnable ? "TRUE" : "FALSE");
            printf("u32LongClickTime_msec: %d ms\n", key_attr->unAttr.stClickKeyAttr.u32LongClickTime_msec);
        } else if(key_attr->enType == KEYMNG_KEY_TYPE_HOLD) {
            printf("unAttr.stHoldKeyAttr: (No additional attributes)\n");
        }
    }
    printf("stGrpKeyCfg.bEnable: %s\n", devmng->stkeyMngCfg.stGrpKeyCfg.bEnable ? "TRUE" : "FALSE");
    if(devmng->stkeyMngCfg.stGrpKeyCfg.bEnable) {
        printf("      stGrpKeyCfg.au32GrpKeyIdx: [");
        for(int i = 0; i < KEYMNG_KEY_NUM_EACH_GRP; i++) {
            printf("%d", devmng->stkeyMngCfg.stGrpKeyCfg.au32GrpKeyIdx[i]);
            if(i < KEYMNG_KEY_NUM_EACH_GRP - 1) printf(", ");
        }
        printf("]\n");
    }
    printf("PWM:\n");
    printf("Enable: %s\n", devmng->PWM.Enable ? "ON" : "OFF");
    printf("PWMCfg.group: %d\n", devmng->PWM.PWMCfg.group);
    printf("PWMCfg.channel: %d\n", devmng->PWM.PWMCfg.channel);
    printf("PWMCfg.period: %d\n", devmng->PWM.PWMCfg.period);
    printf("PWMCfg.duty_cycle: %d\n", devmng->PWM.PWMCfg.duty_cycle);
    printf("Gsensor:\n");
    printf("gsensor_level: %d\n", devmng->Gsensor.gsensor_level);
    printf("gsensor_enable: %d\n", devmng->Gsensor.gsensor_enable);
    printf("enSensitity: %d (%s)\n", devmng->Gsensor.enSensitity,
           gsensor_sensitivity_to_str(devmng->Gsensor.enSensitity));
    printf("stAttr.u32SampleRate: %d kps\n", devmng->Gsensor.stAttr.u32SampleRate);
    printf("GaugeCfg:\n");
    printf("s32LowLevel: %d%%\n", devmng->GaugeCfg.s32LowLevel);
    printf("s32UltraLowLevel: %d%%\n", devmng->GaugeCfg.s32UltraLowLevel);
    printf("s32ADCChannelVbat: %d\n", devmng->GaugeCfg.s32ADCChannelVbat);
    printf("s32USBChargerDetectGPIO: %d\n", devmng->GaugeCfg.s32USBChargerDetectGPIO);
    printf("stFlashLed:\n");
    printf("GpioNum: %d\n", devmng->FlashLed.GpioNum);
    printf("Pulse: %d ms\n", devmng->FlashLed.Pulse);
    printf("Thres: %d%%\n", devmng->FlashLed.Thres);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

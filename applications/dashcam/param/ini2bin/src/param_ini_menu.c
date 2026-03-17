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

#define print_valueset(name, vs)                                                                                       \
    _Generic(vs,                                                                                                       \
        const PARAM_VALUESET_S *: print_valueset_common(name, vs),                                                     \
        const PARAM_VALUESET_2X_S *: print_valueset_common(name, vs),                                                  \
        const PARAM_VALUESET_4X_S *: print_valueset_common(name, vs),                                                  \
        default: (void)printf("[print_valueset] Unsupported type for %s\n", name))
extern PARAM_ACCESS g_ParamAccess;

#define APP_PARAM_LoadValueSet(file, valueset)                  \
    do {                                                        \
        return PARAM_LoadValueSet(file, valueset, #valueset, 1);   \
    } while (0)

#define APP_PARAM_LoadValueSetX(file, valueset, multi)                  \
    do {                                                        \
        return PARAM_LoadValueSet(file, valueset, #valueset, multi);   \
    } while (0)

static int32_t PARAM_LoadValueSet(const char *file, void *param, const char *section, int multi)
{
    long int i = 0;
    long int num = 0;
    long int cur = 0;
    long int value = 0;
    char tmp_desc[PARAM_MENU_ITEM_DESC_LEN] = {0};
    char tmp_key_desc[32] = {0};
    char tmp_key_value[32] = {0};
    PARAM_VALUESET_S *ValueSet = NULL;
    PARAM_VALUESET_2X_S *ValueSet2X = NULL;
    PARAM_VALUESET_4X_S *ValueSet4X = NULL;

    if(multi == 1){
        ValueSet = param;
    }else if(multi == 2){
        ValueSet2X = param;
    }else if(multi == 4){
        ValueSet4X = param;
    }

    printf("file: %s, section: %s\n", file, section);

    num = INI_GetLong(section, "num", 0, file);
    cur = INI_GetLong(section, "current", 0, file);
    printf("%s: %ld %ld\n", __func__, num, cur);

    if(multi == 1){
        ValueSet->ItemCnt = num;
        ValueSet->Current = cur;
    }else if(multi == 2){
        ValueSet2X->ItemCnt = num;
        ValueSet2X->Current = cur;
    }else if(multi == 4){
        ValueSet4X->ItemCnt = num;
        ValueSet4X->Current = cur;
    }

    for (i = 0; i < num; i++) {
        memset(tmp_desc, 0, sizeof(tmp_desc));
        memset(tmp_key_desc, 0, sizeof(tmp_key_desc));
        memset(tmp_key_value, 0, sizeof(tmp_key_value));

        snprintf(tmp_key_desc, sizeof(tmp_key_desc), "description%ld", i);
        snprintf(tmp_key_value, sizeof(tmp_key_value), "value%ld", i);

        INI_GetString(section, tmp_key_desc, "", tmp_desc, PARAM_MENU_ITEM_DESC_LEN, file);
        value = INI_GetLong(section, tmp_key_value, 0, file);
        printf("%s: %ld\n", tmp_desc, value);

        if(multi == 1){
            ValueSet->Items[i].Value = value;
            memcpy(ValueSet->Items[i].Desc, tmp_desc, PARAM_MENU_ITEM_DESC_LEN);
        }else if(multi == 2){
            ValueSet2X->Items[i].Value = value;
            memcpy(ValueSet2X->Items[i].Desc, tmp_desc, PARAM_MENU_ITEM_DESC_LEN);
        }else if(multi == 4){
            ValueSet4X->Items[i].Value = value;
            memcpy(ValueSet4X->Items[i].Desc, tmp_desc, PARAM_MENU_ITEM_DESC_LEN);
        }
    }

    return 0;
}


static int32_t PARAM_LoadUserDataSet(const char *file, void *param, const char *section)
{
    long int bBootFirst = 0;
    unsigned char num = false;

    PARAM_USER_MENU_S *ValueSet = param;
    printf("file: %s, section: %s\n", file, section);

    bBootFirst = INI_GetLong(section, "bBootFirst", 0, file);
    num = (unsigned char)bBootFirst;
    printf("%s: %ld: %d\n", __func__, bBootFirst, num);

    ValueSet->bBootFirst = num;


    return 0;
}

static int32_t PARAM_LoadVideoSize(const char *file, PARAM_VALUESET_S *video_size)
{
    APP_PARAM_LoadValueSet(file, video_size);
    return 0;
}

static int32_t PARAM_LoadVideoLoop(const char *file, PARAM_VALUESET_S *video_loop)
{
    APP_PARAM_LoadValueSet(file, video_loop);
    return 0;
}

static int32_t PARAM_LoadVideoCodec(const char *file, PARAM_VALUESET_S *video_codec)
{
    APP_PARAM_LoadValueSet(file, video_codec);
    return 0;
}

static int32_t PARAM_LoadLapseTime(const char *file, PARAM_VALUESET_S *lapse_time)
{
    APP_PARAM_LoadValueSet(file, lapse_time);
    return 0;
}

static int32_t PARAM_LoadAudioEnable(const char *file, PARAM_VALUESET_S *audio_enable)
{
    APP_PARAM_LoadValueSet(file, audio_enable);
    return 0;
}

static int32_t PARAM_LoadOsdEnable(const char *file, PARAM_VALUESET_S *osd_enable)
{
    APP_PARAM_LoadValueSet(file, osd_enable);
    return 0;
}

static int32_t PARAM_LoadScreenDormant(const char *file, PARAM_VALUESET_S *screen_dormant)
{
    APP_PARAM_LoadValueSet(file, screen_dormant);
    return 0;
}

static int32_t PARAM_LoadKeyToneEnable(const char *file, PARAM_VALUESET_S *key_tone)
{
    APP_PARAM_LoadValueSet(file, key_tone);
    return 0;
}

static int32_t PARAM_LoadFatigueEnable(const char *file, PARAM_VALUESET_S *fatigue_driving)
{
    APP_PARAM_LoadValueSet(file, fatigue_driving);
    return 0;
}

static int32_t PARAM_LoadSpeedStampEnable(const char *file, PARAM_VALUESET_S *speed_stamp)
{
    APP_PARAM_LoadValueSet(file, speed_stamp);
    return 0;
}

static int32_t PARAM_LoadGPSStampEnable(const char *file, PARAM_VALUESET_S *GPS_stamp)
{
    APP_PARAM_LoadValueSet(file, GPS_stamp);
    return 0;
}

static int32_t PARAM_LoadSpeedUnitEnable(const char *file, PARAM_VALUESET_S *Speed_Unit)
{
    APP_PARAM_LoadValueSet(file, Speed_Unit);
    return 0;
}
static int32_t PARAM_LoadRearCamMirrorEnable(const char *file, PARAM_VALUESET_S *RearCam_Mirror)
{
    APP_PARAM_LoadValueSet(file, RearCam_Mirror);
    return 0;
}

static int32_t PARAM_LoadTimeFormatEnable(const char *file, PARAM_VALUESET_S *Time_Format)
{
    APP_PARAM_LoadValueSet(file, Time_Format);
    return 0;
}
static int32_t PARAM_LoadTimeZoneEnable(const char *file, PARAM_VALUESET_S *Time_Zone)
{
    APP_PARAM_LoadValueSet(file, Time_Zone);
    return 0;
}

static int32_t PARAM_LoadFrequenceEnable(const char *file, PARAM_VALUESET_S *Frequence)
{
    APP_PARAM_LoadValueSet(file, Frequence);
    return 0;
}

static int32_t PARAM_LoadParkingEnable(const char *file, PARAM_VALUESET_S *Parking)
{
    APP_PARAM_LoadValueSet(file, Parking);
    return 0;
}

static int32_t PARAM_LoadUserData(const char *file, PARAM_USER_MENU_S *UserData)
{
    char *section = "UserData";
    PARAM_LoadUserDataSet(file, UserData, section);
    return 0;
}

static int32_t PARAM_LoadCarNumStampEnable(const char *file, PARAM_VALUESET_S *carnum_stamp)
{
    APP_PARAM_LoadValueSet(file, carnum_stamp);
    return 0;
}

static int32_t PARAM_LoadRecLoopEnable(const char *file, PARAM_VALUESET_S *rec_loop)
{
    APP_PARAM_LoadValueSet(file, rec_loop);
    return 0;
}

static int32_t PARAM_LoadPhotoSize(const char *file, PARAM_VALUESET_S *photo_size)
{
    APP_PARAM_LoadValueSet(file, photo_size);
    return 0;
}

static int32_t PARAM_LoadLightFrequence(const char *file, PARAM_VALUESET_S *light_frequence)
{
    APP_PARAM_LoadValueSet(file, light_frequence);
    return 0;
}

static int32_t PARAM_LoadPowerOff(const char *file, PARAM_VALUESET_S *power_off)
{
    APP_PARAM_LoadValueSet(file, power_off);
    return 0;
}

static int32_t PARAM_LoadFaceDet(const char *file, PARAM_VALUESET_S *face_det)
{
    APP_PARAM_LoadValueSet(file, face_det);
    return 0;
}

static int32_t PARAM_LoadFaceSmile(const char *file, PARAM_VALUESET_S *face_smile)
{
    APP_PARAM_LoadValueSet(file, face_smile);
    return 0;
}

static int32_t PARAM_LoadActionAudio(const char *file, PARAM_VALUESET_S *action_audio)
{
    APP_PARAM_LoadValueSet(file, action_audio);
    return 0;
}

static int32_t PARAM_LoadFlashLed(const char *file, PARAM_VALUESET_S *flash_led)
{
    APP_PARAM_LoadValueSet(file, flash_led);
    return 0;
}

static int32_t PARAM_LoadPhotoQuality(const char *file, PARAM_VALUESET_S *photo_quality)
{
    APP_PARAM_LoadValueSet(file, photo_quality);
    return 0;
}

static int32_t PARAM_LoadIspEffect(const char *file, PARAM_VALUESET_4X_S *isp_effect)
{
    APP_PARAM_LoadValueSetX(file, isp_effect, 4);
    return 0;
}

static int32_t PARAM_LoadLanguage(const char *file, PARAM_VALUESET_2X_S *language)
{
    APP_PARAM_LoadValueSetX(file, language, 2);
    return 0;
}

static int32_t PARAM_LoadAutoScreenOff(const char *file, PARAM_VALUESET_S *auto_screen_off)
{
    APP_PARAM_LoadValueSet(file, auto_screen_off);
    return 0;
}

static int32_t PARAM_LoadMenu(const char *file, PARAM_MENU_S *Menu)
{
   int32_t  s32Ret = 0;
    s32Ret = PARAM_LoadVideoSize(file, &Menu->VideoSize);
    s32Ret |= PARAM_LoadVideoLoop(file, &Menu->VideoLoop);
    s32Ret |= PARAM_LoadVideoCodec(file, &Menu->VideoCodec);
    s32Ret |= PARAM_LoadLapseTime(file, &Menu->LapseTime);
    s32Ret |= PARAM_LoadAudioEnable(file, &Menu->AudioEnable);
    s32Ret |= PARAM_LoadOsdEnable(file, &Menu->Osd);
    s32Ret |= PARAM_LoadScreenDormant(file, &Menu->ScreenDormant);
    s32Ret |= PARAM_LoadKeyToneEnable(file, &Menu->KeyTone);
    s32Ret |= PARAM_LoadFatigueEnable(file, &Menu->FatigueDirve);
    s32Ret |= PARAM_LoadSpeedStampEnable(file, &Menu->SpeedStamp);
    s32Ret |= PARAM_LoadGPSStampEnable(file, &Menu->GPSStamp);
    s32Ret |= PARAM_LoadSpeedUnitEnable(file, &Menu->SpeedUnit);
    s32Ret |= PARAM_LoadRearCamMirrorEnable(file, &Menu->CamMirror);
    s32Ret |= PARAM_LoadTimeFormatEnable(file, &Menu->TimeFormat);
    s32Ret |= PARAM_LoadTimeZoneEnable(file, &Menu->TimeZone);
    s32Ret |= PARAM_LoadFrequenceEnable(file, &Menu->Frequence);
    s32Ret |= PARAM_LoadParkingEnable(file, &Menu->Parking);
    s32Ret |= PARAM_LoadUserData(file, &Menu->UserData);
    s32Ret |= PARAM_LoadCarNumStampEnable(file, &Menu->CarNumStamp);
    s32Ret |= PARAM_LoadRecLoopEnable(file, &Menu->RecLoop);
    s32Ret |= PARAM_LoadPhotoSize(file, &Menu->PhotoSize);
    s32Ret |= PARAM_LoadLightFrequence(file, &Menu->LightFrequence);
    s32Ret |= PARAM_LoadPowerOff(file, &Menu->PowerOff);
    s32Ret |= PARAM_LoadFaceDet(file, &Menu->FaceDet);
    s32Ret |= PARAM_LoadFaceSmile(file, &Menu->FaceSmile);
    s32Ret |= PARAM_LoadActionAudio(file, &Menu->ActionAudio);
    s32Ret |= PARAM_LoadFlashLed(file, &Menu->FlashLed);
    s32Ret |= PARAM_LoadPhotoQuality(file, &Menu->PhotoQuality);
    s32Ret |= PARAM_LoadIspEffect(file, &Menu->IspEffect);
    s32Ret |= PARAM_LoadLanguage(file, &Menu->Language);
    s32Ret |= PARAM_LoadAutoScreenOff(file, &Menu->AutoScreenOff);
    if (s32Ret != 0) {
        printf("load error\n");
    }
    return 0;
}

int32_t  INI_PARAM_LoadMenuCfg(PARAM_MENU_S *MenuMng)
{
    printf("\n---enter: %s\n", __func__);

    uint32_t i = 0;
    char filepath[PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_menu")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadMenu(filepath, MenuMng);
            break;
        }
    }

    return 0;
}

static void print_valueset_common(const char *name, const void *vs)
{
    if(vs == NULL) {
        printf("[%s] %s: vs is NULL\n", __func__, name);
        return;
    }
    // 强制转换为任意一种VALUESET类型（布局一致即可）
    const PARAM_VALUESET_S *p_vs = (const PARAM_VALUESET_S *)vs;
    // 统一打印逻辑（与原三个函数完全一致）
    printf("  %s: ItemCnt=%u, Current=%u\n", name, p_vs->ItemCnt, p_vs->Current);
    for(uint32_t i = 0; i < p_vs->ItemCnt; i++) {
        const char *desc = p_vs->Items[i].Desc ? p_vs->Items[i].Desc : "";
        printf("    Item%u: Value=%d, Desc=%s\n", i, p_vs->Items[i].Value, desc);
    }
}

void print_param_menu(const PARAM_MENU_S *menu)
{
    if(menu == NULL) {
        printf("print_param_menu: menu is NULL\n");
        return;
    }
    print_valueset("VideoSize", &menu->VideoSize);
    print_valueset("VideoLoop", &menu->VideoLoop);
    print_valueset("VideoCodec", &menu->VideoCodec);
    print_valueset("LapseTime", &menu->LapseTime);
    print_valueset("AudioEnable", &menu->AudioEnable);
    print_valueset("Osd", &menu->Osd);
    print_valueset("PwmBri", &menu->PwmBri);
    print_valueset("ViewWin", &menu->ViewWin);
    print_valueset("LcdContrl", &menu->LcdContrl);
    print_valueset("ScreenDormant", &menu->ScreenDormant);
    print_valueset("KeyTone", &menu->KeyTone);
    print_valueset("FatigueDirve", &menu->FatigueDirve);
    print_valueset("SpeedStamp", &menu->SpeedStamp);
    print_valueset("GPSStamp", &menu->GPSStamp);
    print_valueset("SpeedUnit", &menu->SpeedUnit);
    print_valueset("CamMirror", &menu->CamMirror);
    print_valueset("TimeFormat", &menu->TimeFormat);
    print_valueset("TimeZone", &menu->TimeZone);
    print_valueset("Frequence", &menu->Frequence);
    print_valueset("Parking", &menu->Parking);
    print_valueset("PhotoSize", &menu->PhotoSize);
    print_valueset("PhotoQuality", &menu->PhotoQuality);
    print_valueset("MotionDet", &menu->MotionDet);
    print_valueset("CarNumStamp", &menu->CarNumStamp);
    print_valueset("RecLoop", &menu->RecLoop);
    print_valueset("LightFrequence", &menu->LightFrequence);
    print_valueset("FaceDet", &menu->FaceDet);
    print_valueset("FaceSmile", &menu->FaceSmile);
    print_valueset("PowerOff", &menu->PowerOff);
    print_valueset("ActionAudio", &menu->ActionAudio);
    print_valueset("FlashLed", &menu->FlashLed);
    print_valueset("IspEffect", &menu->IspEffect);
    print_valueset("Language", &menu->Language);
    print_valueset("AutoScreenOff", &menu->AutoScreenOff);
    printf("UserData:\n");
    printf("bBootFirst: %u\n", menu->UserData.bBootFirst);
    printf("cUserCarName: %s\n", menu->UserData.cUserCarName);
    printf("u32UserCarNum: ");
    for(uint32_t i = 0; i < 8; i++) {
        printf("%u ", menu->UserData.u32UserCarNum[i]);
    }
    printf("\n");
    printf("==========================\n");
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

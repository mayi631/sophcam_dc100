#include <string.h>
#include "param.h"
#include "param_ini2bin.h"
#include "ini.h"
// #include "media_init.h"
#include "mode.h"
#include "param_printf.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


extern PARAM_ACCESS g_ParamAccess;
// /* common func */
// static uint32_t HexString2Dec(const unsigned char *line, uint32_t len)
// {
//     unsigned char c;
//     unsigned char ch;
//     uint32_t value = 0;

//     if (len == 0) {
//         return -1;
//     }
//     for (value = 0; len--; line++) {
//         ch = *line;
//         if ((ch >= '0') && (ch <= '9')) {
//             value = value * 16 + (ch - '0');
//             continue;
//         }

//         c = (unsigned char) (ch | 0x20);
//         if (c >= 'a' && c <= 'f') {
//             value = value * 16 + (c - 'a' + 10);
//             continue;
//         }
//         return -1;
//     }
//     return value;
// }


/* common config start */
static int32_t PARAM_LoadMode(const char *filename, uint32_t *mode)
{
    char tmp_mode[16] = {0};

    INI_GetString("work_mode", "poweron_mode", "",
        tmp_mode, 16, filename);
    printf("%s: poweron_mode: %s\n", __func__, tmp_mode);

    if(strcmp(tmp_mode, "record") == 0) {
        *mode = WORK_MODE_MOVIE;
    } else if(strcmp(tmp_mode, "photo") == 0) {
        *mode = WORK_MODE_PHOTO;
    } else if(strcmp(tmp_mode, "playback") == 0) {
        *mode = WORK_MODE_PLAYBACK;
    } else if(strcmp(tmp_mode, "usbcam") == 0) {
        *mode = WORK_MODE_USBCAM;
    } else if(strcmp(tmp_mode, "usb") == 0) {
        *mode = WORK_MODE_USBCAM;
    } else {
        printf("error mode: %s\n", tmp_mode);
        return -1;
    }

    printf("work_mode index: %u\n", *mode);

    return 0;
}

static int32_t PARAM_LoadVO(const char *file, PARAM_DISP_ATTR_S *Vo)
{
    long int i = 0;
    long int vo_cnt = 0;
    long int width = 0;
    long int height = 0;
    long int rotate = 0;
    long int enIntfSync = 0;
    long int frame_fmt = 0;
    char tmp_section[32] = {0};
    long int fps = 25;

    vo_cnt = INI_GetLong("vo_config", "vo_cnt", 0, file);
    printf("%s: vo_cnt: %ld\n", __func__, vo_cnt);

    for (i = 0; i < vo_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "vo%ld", i);
        width = INI_GetLong(tmp_section, "width", 0, file);
        height = INI_GetLong(tmp_section, "height", 0, file);
        rotate = INI_GetLong(tmp_section, "rotate", 0, file);
        fps = INI_GetLong(tmp_section, "fps", 0, file);
        enIntfSync = INI_GetLong(tmp_section, "enIntfSync", 36, file);// default VO_OUTPUT_USER
        frame_fmt = INI_GetLong(tmp_section, "frame_fmt", 19, file);

        Vo->Width = width;
        Vo->Height = height;
        Vo->Rotate = rotate;
        Vo->Fps = fps;
        Vo->EnIntfSync = enIntfSync;
        Vo->frame_fmt = frame_fmt;

        printf("%ld %ld %ld %ld %ld %ld\n", width, height, rotate, fps, enIntfSync, frame_fmt);
    }

    return 0;
}

static int32_t PARAM_LoadWindow(const char *file, PARAM_WND_ATTR_S *Window)
{
#ifdef SERVICES_LIVEVIEW_ON
    long int i = 0;
    long int window_cnt = 0;
    long int enable = 0;
    long int used_crop = 0;
    long int small_win_enable = 0;
    long int bind_vproc_id = 0;
    long int bind_vproc_chn_id = 0;
    long int x = 0;
    long int y = 0;
    long int width = 0;
    long int s_x = 0;
    long int s_y = 0;
    long int height = 0;
    long int s_width = 0;
    long int s_height = 0;
    long int onestep = 0;
    long int ystep = 0;
    long int mirror = 0;
    long int filp = 0;
    char tmp_section[32] = {0};

    window_cnt = INI_GetLong("window_config", "window_cnt", 0, file);
    printf("%s: window_cnt: %ld\n", __func__, window_cnt);
    Window->WndCnt = window_cnt;

    for (i = 0; i < window_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "window%ld", i);

        enable = INI_GetLong(tmp_section, "enable", 0, file);
        used_crop = INI_GetLong(tmp_section, "used_crop", 0, file);
        small_win_enable = INI_GetLong(tmp_section, "small_win_enable", 0, file);
        bind_vproc_id = INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        bind_vproc_chn_id = INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
        x = INI_GetLong(tmp_section, "x", 0, file);
        y = INI_GetLong(tmp_section, "y", 0, file);
        width = INI_GetLong(tmp_section, "width", 0, file);
        height = INI_GetLong(tmp_section, "height", 0, file);
        s_x = INI_GetLong(tmp_section, "s_x", 0, file);
        s_y = INI_GetLong(tmp_section, "s_y", 0, file);
        s_width = INI_GetLong(tmp_section, "s_width", 0, file);
        s_height = INI_GetLong(tmp_section, "s_height", 0, file);
        onestep = INI_GetLong(tmp_section, "onestep", 0, file);
        ystep = INI_GetLong(tmp_section, "ystep", 0, file);
        mirror = INI_GetLong(tmp_section, "mirror", 0, file);
        filp = INI_GetLong(tmp_section, "filp", 0, file);

        Window->Wnds[i].WndEnable = enable;
        Window->Wnds[i].UsedCrop = used_crop;
        Window->Wnds[i].SmallWndEnable = small_win_enable;
        Window->Wnds[i].BindVprocId = bind_vproc_id;
        Window->Wnds[i].BindVprocChnId = bind_vproc_chn_id;
        Window->Wnds[i].WndX = x;
        Window->Wnds[i].WndY = y;
        Window->Wnds[i].WndWidth = width;
        Window->Wnds[i].WndHeight = height;
        Window->Wnds[i].WndsX = s_x;
        Window->Wnds[i].WndsY = s_y;
        Window->Wnds[i].WndsWidth = s_width;
        Window->Wnds[i].WndsHeight = s_height;
        Window->Wnds[i].OneStep = onestep;
        Window->Wnds[i].yStep = ystep;
        Window->Wnds[i].WndMirror = mirror;
        Window->Wnds[i].WndFilp = filp;

        printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
            enable, used_crop, small_win_enable, bind_vproc_id,
            bind_vproc_chn_id, x, y, width, height, s_x, s_y,
            s_width, s_height);
    }
#endif
    return 0;
}

static int32_t PARAM_LoadFacep(const char *file, PARAM_FACEP_ATTR_S *Facep)
{
#ifdef SERVICES_FACEP_ON
    long int i = 0;
    long int facep_cnt = 0;
    long int fd_enable = 0;
    long int fattr_enable = 0;
    long int in_vpss_grp = 0;
    long int in_vpss_chn = 0;
    long int in_width = 0;
    long int in_height = 0;
    long int model_width = 0;
    long int model_height = 0;
    long int osd_id = 0;
    long int osd_mirror = 0;

    char tmp_section[32] = {0};
    char model_fd_path[MODEL_PATH_LEN] = {0};
    char model_fattr_path[MODEL_PATH_LEN] = {0};
    float model_fd_thres = 0;

    facep_cnt = INI_GetLong("facep_config", "facep_cnt", 0, file);
    printf("%s: facep_cnt: %ld\n", __func__, facep_cnt);
    Facep->facep_cnt = facep_cnt;

    for (i = 0; i < facep_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "facep%ld", i);

        fd_enable = INI_GetLong(tmp_section, "fd_enable", 0, file);
        fattr_enable = INI_GetLong(tmp_section, "fattr_enable", 0, file);
        in_vpss_grp = INI_GetLong(tmp_section, "in_vpss_grp", 0, file);
        in_vpss_chn = INI_GetLong(tmp_section, "in_vpss_chn", 0, file);
        in_width = INI_GetLong(tmp_section, "in_width", 0, file);
        in_height = INI_GetLong(tmp_section, "in_height", 0, file);
        model_width = INI_GetLong(tmp_section, "model_width", 0, file);
        model_height = INI_GetLong(tmp_section, "model_height", 0, file);
        model_fd_thres = INI_GetFloat(tmp_section, "model_fd_thres", 0.f, file);
        osd_id = INI_GetLong(tmp_section, "osd_id", 0, file);
        osd_mirror = INI_GetLong(tmp_section, "osd_mirror", 0, file);
        INI_GetString(tmp_section, "model_fd_path", "", model_fd_path, sizeof(model_fd_path), file);
        INI_GetString(tmp_section, "model_fattr_path", "", model_fattr_path, sizeof(model_fattr_path), file);

        Facep->sv_param[i].fd_enable = fd_enable;
        Facep->sv_param[i].fattr_enable = fattr_enable;
        Facep->sv_param[i].in_vpss_grp = in_vpss_grp;
        Facep->sv_param[i].in_vpss_chn = in_vpss_chn;
        Facep->sv_param[i].in_width = in_width;
        Facep->sv_param[i].in_height = in_height;
        Facep->sv_param[i].model_width = model_width;
        Facep->sv_param[i].model_height = model_height;
        Facep->sv_param[i].model_fd_thres = model_fd_thres;
        Facep->sv_param[i].osd_id = osd_id;
        Facep->sv_param[i].osd_mirror = osd_mirror;
        strncpy(Facep->sv_param[i].model_fd_path, model_fd_path, sizeof(Facep->sv_param[i].model_fd_path));
        strncpy(Facep->sv_param[i].model_fattr_path, model_fattr_path, sizeof(Facep->sv_param[i].model_fattr_path));

        printf("facep%ld %ld %ld %ld %ld %ld %ld\n",
            i, in_vpss_grp, in_vpss_chn, in_width, in_height,
            model_width, model_height);
        printf("fd:%s thres:%f enable:%ld\n", model_fd_path, model_fd_thres, fd_enable);
        printf("fattr:%s enable:%ld\n", model_fattr_path, fattr_enable);
        printf("osd id:%ld mirror:%ld\n", osd_id, osd_mirror);
    }
#endif
    return 0;
}

static int32_t PARAM_LoadAi(const char *file, MAPI_ACAP_ATTR_S *Ai)
{
    // uint32_t i = 0;
    // long int ai_cnt = 0;
    long int sample_rate = 0;
    long int channel = 0;
    long int num_per_frm = 0;
    long int isVquOn = 0;
    long int audio_channel = 0;
    long int volume = 0;
    //long int resample = 0;
    char tmp_section[16] = {0};

    // ai_cnt = INI_GetLong("ai_config", "ai_cnt", 0, file);
    // printf("%s: ai_cnt: %ld\n", __func__, ai_cnt);
    // Ai->ChnCnt = ai_cnt;

    //for (i = 0; i < ai_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "ai");
        sample_rate = INI_GetLong(tmp_section, "sample_rate", 0, file);
        channel = INI_GetLong(tmp_section, "channel", 0, file);
        num_per_frm = INI_GetLong(tmp_section, "num_per_frm", 0, file);
        isVquOn = INI_GetLong(tmp_section, "isVquOn", 0, file);
        audio_channel = INI_GetLong(tmp_section, "audio_channel", 0, file);
        volume = INI_GetLong(tmp_section, "volume", 20, file);
        //resample = INI_GetLong(tmp_section, "resample", 0, file);

        Ai->enSampleRate = sample_rate;
        Ai->channel = channel;
        Ai->u32PtNumPerFrm = num_per_frm;
        Ai->bVqeOn = isVquOn;
        Ai->AudioChannel = audio_channel;
        Ai->volume = volume;
        //Ai->resample = resample;

        printf("%ld %ld %ld\n", sample_rate, channel, num_per_frm);
    //}

    return 0;
}

static int32_t PARAM_LoadAenc(const char *file, MAPI_AENC_ATTR_S *Aenc)
{
    long int format = 0;
    long int sample_rate = 0;
    long int num_per_frm = 0;
    long int channel = 0;
    char tmp_section[16] = {0};

    snprintf(tmp_section, sizeof(tmp_section), "aenc");
    format = INI_GetLong(tmp_section, "format", 0, file);
    sample_rate = INI_GetLong(tmp_section, "sample_rate", 0, file);
    num_per_frm = INI_GetLong(tmp_section, "num_per_frm", 0, file);
    channel = INI_GetLong(tmp_section, "channel", 0, file);

    Aenc->enAencFormat = format;
    Aenc->src_samplerate = sample_rate;
    Aenc->u32PtNumPerFrm = num_per_frm;
    Aenc->channels = channel;

    printf("anec param: format=%ld, sample_rate=%ld, num_per_frm=%ld, channel=%ld\n",
            format, sample_rate, num_per_frm, channel);

    return 0;
}

static int32_t PARAM_LoadAo(const char *file, MAPI_AO_ATTR_S *Ao)
{
    // uint32_t i = 0;
    // long int ao_cnt = 0;
    long int sample_rate = 0;
    long int channel = 0;
    long int num_per_frm = 0;
    long int audio_channel = 0;
    long int volume = 0;
    long int PowerPinId = 0;
    char tmp_section[16] = {0};

    // ao_cnt = INI_GetLong("ao_config", "ao_cnt", 0, file);
    // printf("%s: ao_cnt: %ld\n", __func__, ao_cnt);
    // Ao->ChnCnt = ao_cnt;

    // for (i = 0; i < ao_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "ao");
        sample_rate = INI_GetLong(tmp_section, "sample_rate", 0, file);
        channel = INI_GetLong(tmp_section, "channel", 0, file);
        num_per_frm = INI_GetLong(tmp_section, "num_per_frm", 0, file);
        audio_channel = INI_GetLong(tmp_section, "audio_channel", 0, file);
        volume = INI_GetLong(tmp_section, "volume", 32, file);
        PowerPinId = INI_GetLong(tmp_section, "PowerPinId", 32, file);

        Ao->enSampleRate = sample_rate;
        Ao->channels = channel;
        Ao->u32PtNumPerFrm = num_per_frm;
        Ao->AudioChannel = audio_channel;
        Ao->volume = volume;
        Ao->u32PowerPinId = PowerPinId;

        printf("%ld %ld %ld %ld\n", sample_rate, channel, num_per_frm, PowerPinId);
    // }

    return 0;
}

#ifdef SERVICES_SPEECH_ON
static int32_t PARAM_LoadSpeech(const char *file, SPEECHMNG_PARAM_S *Speech)
{
    long int enable = 0;
    long int sample_rate = 0;
    long int bitWidth = 0;
    char model_path[128] = "";
    char tmp_section[16] = {0};

    snprintf(tmp_section, sizeof(tmp_section), "speech");
    enable = INI_GetLong(tmp_section, "enable", 0, file);
    sample_rate = INI_GetLong(tmp_section, "sample_rate", 0, file);
    bitWidth = INI_GetLong(tmp_section, "bitWidth", 0, file);
    INI_GetString(tmp_section, "model_path", "", model_path, sizeof(model_path), file);

    Speech->enable = enable;
    Speech->SampleRate = sample_rate;
    Speech->BitWidth = bitWidth;
    strncpy(Speech->ModelPath, model_path, sizeof(Speech->ModelPath));

    printf("%ld %ld %s\n", sample_rate, bitWidth, model_path);

    return 0;
}
#endif

static int32_t PARAM_LoadOsd(const char *file, PARAM_MEDIA_OSD_ATTR_S *Osd)
{
    long int i = 0, j = 0;
    long int osd_cnt = 0;
    // display params
    long int enable = 0;
    long int osdc_enable = 0;
    long int bind_mode = 0;
    long int mod_hdl = 0;
    long int chn_hdl = 0;
    long int coordinate = 0;
    long int start_x = 0;
    long int start_y = 0;
    long int batch = 0;
    long int display_num = 0;
    // content params
    long int type = 0;
    long int rgn_type = 0;
    long int color = 0;
    // -time
    long int time_fmt = 0;
    long int width = 0;
    long int height = 0;
    long int bg_color = 0;
    // -str
    char str[MAPI_OSD_MAX_STR_LEN] = {0};
    // -circle
    // -bmp
    long int pixel_fmt = 0;

    char tmp_section[64] = {0};

    osd_cnt = INI_GetLong("osd_config", "osd_cnt", 0, file);
    osdc_enable = INI_GetLong("osd_config", "osdc_enable", 0, file);
    printf("%s: osd_cnt: %ld, osdc_enable: %ld\n", __func__, osd_cnt, osdc_enable);
    Osd->OsdCnt = osd_cnt;
    Osd->bOsdc = osdc_enable;

    for (i = 0; i < osd_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "osd_content%ld", i);
        type = INI_GetLong(tmp_section, "type", 0, file);
        // TODO: char converter to uint32_t: "0xffff" --> 65535
        color = INI_GetLong(tmp_section, "color", 0, file);
        display_num = INI_GetLong(tmp_section, "display_num", 0, file);
        Osd->OsdAttrs[i].u32DispNum = display_num;
        Osd->OsdAttrs[i].stContent.enType = type;
        Osd->OsdAttrs[i].stContent.u32Color = color;

        if (type == 0) {
            // time
            time_fmt = INI_GetLong(tmp_section, "time_fmt", 0, file);
            width = INI_GetLong(tmp_section, "width", 0, file);
            height = INI_GetLong(tmp_section, "height", 0, file);
            // TODO: char converter to uint32_t: "0xffff" --> 65535
            bg_color = INI_GetLong(tmp_section, "bg_color", 0, file);
            Osd->OsdAttrs[i].stContent.stTimeContent.enTimeFmt = time_fmt;
            Osd->OsdAttrs[i].stContent.stTimeContent.stFontSize.u32Width = width;
            Osd->OsdAttrs[i].stContent.stTimeContent.stFontSize.u32Height = height;
            Osd->OsdAttrs[i].stContent.stTimeContent.u32BgColor = bg_color;
        } else if (type == 1) {
            // string
            // TODO: get str
            char string[MAPI_OSD_MAX_STR_LEN] = {0};
            width = INI_GetLong(tmp_section, "width", 0, file);
            height = INI_GetLong(tmp_section, "height", 0, file);
            // TODO: char converter to uint32_t: "0xffff" --> 65535
            bg_color = INI_GetLong(tmp_section, "bg_color", 0, file);
            INI_GetString(tmp_section, "string", "", string, MAPI_OSD_MAX_STR_LEN, file);
            snprintf(Osd->OsdAttrs[i].stContent.stStrContent.szStr, MAPI_OSD_MAX_STR_LEN, "%s", string);
            Osd->OsdAttrs[i].stContent.stStrContent.stFontSize.u32Width = width;
            Osd->OsdAttrs[i].stContent.stStrContent.stFontSize.u32Height = height;
            Osd->OsdAttrs[i].stContent.stStrContent.u32BgColor = bg_color;
        } else if (type == 2) {
            // bmp
            pixel_fmt = INI_GetLong(tmp_section, "pixel_fmt", 0, file);
            Osd->OsdAttrs[i].stContent.stBitmapContent.enPixelFormat = pixel_fmt;
        } else if (type == 3) {
            // circle
            width = INI_GetLong(tmp_section, "width", 0, file);
            height = INI_GetLong(tmp_section, "height", 0, file);
            Osd->OsdAttrs[i].stContent.stCircleContent.u32Width = width;
            Osd->OsdAttrs[i].stContent.stCircleContent.u32Height = height;
        } else if (type == 4) {
            // object
            width = INI_GetLong(tmp_section, "width", 0, file);
            height = INI_GetLong(tmp_section, "height", 0, file);
            Osd->OsdAttrs[i].stContent.stObjectContent.u32Width = width;
            Osd->OsdAttrs[i].stContent.stObjectContent.u32Height = height;
        } else {
            printf("[Error] no such osd type\n");
            return -1;
        }
        printf("%ld %ld %ld %ld %ld %ld %s %ld %ld\n",
                type, color, time_fmt, width, height, bg_color, str, pixel_fmt, display_num);
        for (j = 0 ;j < display_num; j++) {
            memset(tmp_section, 0, sizeof(tmp_section));
            snprintf(tmp_section, sizeof(tmp_section), "osd_content%ld.osd_display%ld", i,j);
            enable = INI_GetLong(tmp_section, "enable", 0, file);
            bind_mode = INI_GetLong(tmp_section, "bind_mode", 0, file);
            mod_hdl = INI_GetLong(tmp_section, "mod_hdl", 0, file);
            chn_hdl = INI_GetLong(tmp_section, "chn_hdl", 0, file);
            coordinate = INI_GetLong(tmp_section, "coordinate", 0, file);
            start_x = INI_GetLong(tmp_section, "start_x", 0, file);
            start_y = INI_GetLong(tmp_section, "start_y", 0, file);
            batch = INI_GetLong(tmp_section, "batch", 0, file);
            rgn_type = INI_GetLong(tmp_section, "rgn_type", 0, file);

            Osd->OsdAttrs[i].astDispAttr[j].bShow = enable;
            Osd->OsdAttrs[i].astDispAttr[j].enBindedMod = bind_mode;
            Osd->OsdAttrs[i].astDispAttr[j].ModHdl = mod_hdl;
            Osd->OsdAttrs[i].astDispAttr[j].ChnHdl = chn_hdl;
            Osd->OsdAttrs[i].astDispAttr[j].enCoordinate = coordinate;
            Osd->OsdAttrs[i].astDispAttr[j].stStartPos.s32X = start_x;
            Osd->OsdAttrs[i].astDispAttr[j].stStartPos.s32Y = start_y;
            Osd->OsdAttrs[i].astDispAttr[j].u32Batch = batch;
            Osd->OsdAttrs[i].astDispAttr[j].enRgnCmprType = rgn_type;

            printf("%ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
                enable, bind_mode, mod_hdl, chn_hdl, coordinate,
                start_x, start_y, batch, rgn_type);
        }

    }
    return 0;
}

static int32_t PARAM_LoadRecord(const char *file, PARAM_RECORD_ATTR_S *Record)
{
    long int i = 0;
    long int rec_cnt = 0;
    long int enable = 0;
    long int bind_venc_id = 0;
    long int sub_video_en = 0;
    long int sub_bind_venc_id = 0;
    long int audio_status = 0;
    long int file_type = 0;
    long int split_time = 0;
    long int pre_time = 0;
    long int post_time = 0;
    float timelapse_recorder_fps = 0;
    long int timelapse_recorder_gop = 0;
    long int memory_buffer_sec = 0;
    long int pre_alloc_unit = 0;
    float normal_extend_video_buffer_sec = -1;
    float event_extend_video_buffer_sec = -1;
    float extend_other_buffer_sec = -1;
    float short_file_ms = -1;
    char model[32] = {0};
    char tmp_section[32] = {0};
    long int focus_pos = 0;
    long int focus_pos_lock = 0;

    rec_cnt = INI_GetLong("record_config", "rec_cnt", 0, file);
    printf("%s: rec_cnt: %ld\n", __func__, rec_cnt);
    Record->ChnCnt = rec_cnt;

    for (i = 0; i < rec_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "record%ld", i);
        enable = INI_GetLong(tmp_section, "enable", 0, file);
        sub_video_en = INI_GetLong(tmp_section, "sub_video_en", 0, file);
        bind_venc_id = INI_GetLong(tmp_section, "bind_venc_id", 0, file);
        audio_status = INI_GetLong(tmp_section, "audio_status", 0, file);
        file_type = INI_GetLong(tmp_section, "file_type", 0, file);
        split_time = INI_GetLong(tmp_section, "split_time", 0, file);
        pre_time = INI_GetLong(tmp_section, "pre_time", 0, file);
        post_time = INI_GetLong(tmp_section, "post_time", 0, file);
        timelapse_recorder_fps = INI_GetFloat(tmp_section, "timelapse_recorder_fps", 0.f, file);
        timelapse_recorder_gop = INI_GetLong(tmp_section, "timelapse_recorder_gop", 0, file);
        memory_buffer_sec = INI_GetLong(tmp_section, "memory_buffer_sec", 0, file);
        pre_alloc_unit = INI_GetLong(tmp_section, "pre_alloc_unit", 0, file);
        normal_extend_video_buffer_sec = INI_GetFloat(tmp_section, "normal_extend_video_buffer_sec", -1, file);
        event_extend_video_buffer_sec = INI_GetFloat(tmp_section, "event_extend_video_buffer_sec", -1, file);
        extend_other_buffer_sec = INI_GetFloat(tmp_section, "extend_other_buffer_sec", -1, file);
        short_file_ms = INI_GetFloat(tmp_section, "short_file_ms", -1, file);
        focus_pos = INI_GetFloat(tmp_section, "focus_pos", 0, file);
        focus_pos_lock = INI_GetFloat(tmp_section, "focus_pos_lock", 0, file);
        INI_GetString(tmp_section, "devmodel", "", model, 31, file);

        if(sub_video_en == 1){
            char tmp_sub_section[32] = {0};
            snprintf(tmp_sub_section, sizeof(tmp_sub_section), "record%ld_sub", i);
            sub_bind_venc_id = INI_GetLong(tmp_sub_section, "sub_bind_venc_id", -1, file);
        }

        Record->ChnAttrs[i].Enable = enable;
        Record->ChnAttrs[i].Subvideoen = sub_video_en;
        Record->ChnAttrs[i].SubBindVencId = sub_bind_venc_id;
        Record->ChnAttrs[i].AudioStatus = audio_status;
        Record->ChnAttrs[i].BindVencId = bind_venc_id;
        Record->ChnAttrs[i].FileType = file_type;
        Record->ChnAttrs[i].SplitTime = split_time;
        Record->ChnAttrs[i].PreTime = pre_time;
        Record->ChnAttrs[i].PostTime = post_time;
        Record->ChnAttrs[i].TimelapseFps = timelapse_recorder_fps;
        Record->ChnAttrs[i].TimelapseGop = timelapse_recorder_gop;
        Record->ChnAttrs[i].MemoryBufferSec = memory_buffer_sec;
        Record->ChnAttrs[i].PreallocUnit = pre_alloc_unit;
        Record->ChnAttrs[i].NormalExtendVideoBufferSec = normal_extend_video_buffer_sec;
        Record->ChnAttrs[i].EventExtendVideoBufferSec = event_extend_video_buffer_sec;
        Record->ChnAttrs[i].ExtendOtherBufferSec = extend_other_buffer_sec;
        Record->ChnAttrs[i].ShortFileMs = short_file_ms;
        Record->ChnAttrs[i].FocusPos = focus_pos;
        Record->ChnAttrs[i].FocusPosLock = focus_pos_lock;
        strncpy(Record->ChnAttrs[i].devmodel, model, sizeof(Record->ChnAttrs[i].devmodel) - 1);

        printf("%ld %ld %ld %ld %ld %ld %ld %f %ld %ld %f %f %f %f\n",
            enable, audio_status, bind_venc_id, file_type, split_time, pre_time,
            post_time, timelapse_recorder_fps, timelapse_recorder_gop, pre_alloc_unit,
            normal_extend_video_buffer_sec, event_extend_video_buffer_sec, extend_other_buffer_sec, short_file_ms);
    }

    return 0;
}

static int32_t PARAM_LoadRtsp(const char *file, PARAM_RTSP_ATTR_S *Rtsp)
{
    long int i = 0;
    long int rtsp_cnt = 0;
    long int enable = 0;
    long int bind_venc_id = 0;
    long int max_connections = 0;
    long int timeout = 0;
    long int port = 0;
    char name[32] = {0};
    char tmp_section[32] = {0};

    rtsp_cnt = INI_GetLong("rtsp_config", "rtsp_cnt", 0, file);
    port = INI_GetLong("rtsp_config", "port", 554, file);
    timeout = INI_GetLong("rtsp_config", "timeout", 120, file);
    printf("%s: rtsp_cnt: %ld %ld %ld\n", __func__, rtsp_cnt, port, timeout);
    //Rtsp->ChnCnt = rtsp_cnt;

    for (i = 0; i < rtsp_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "rtsp%ld", i);
        enable = INI_GetLong(tmp_section, "enable", 0, file);
        bind_venc_id = INI_GetLong(tmp_section, "bind_venc_id", 0, file);
        max_connections = INI_GetLong(tmp_section, "max_conn", 0, file);
        INI_GetString(tmp_section, "name", "", name, 31, file);
        if(strlen(name) == 0){
            snprintf(name, sizeof(name), "cvi_cam_%ld", i);
        }

        Rtsp->ChnAttrs[i].Enable = enable;
        Rtsp->ChnAttrs[i].BindVencId = bind_venc_id;
        Rtsp->ChnAttrs[i].MaxConn = max_connections;
        Rtsp->ChnAttrs[i].Timeout = timeout;
        Rtsp->ChnAttrs[i].Port = port;
        strncpy(Rtsp->ChnAttrs[i].Name, name, sizeof(Rtsp->ChnAttrs[i].Name) - 1);

        printf("%ld %ld\n", enable, bind_venc_id);
    }

    return 0;
}

static int32_t PARAM_LoadPiv(const char *file, PARAM_PIV_ATTR_S *Piv)
{
    long int i = 0;
    long int piv_cnt = 0;
    long int bind_venc_id = 0;
    char tmp_section[32] = {0};

    piv_cnt = INI_GetLong("piv_config", "piv_cnt", 0, file);
    printf("%s: piv_cnt: %ld\n", __func__, piv_cnt);
    //Piv->ChnCnt = piv_cnt;

    for (i = 0; i < piv_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "piv%ld", i);
        bind_venc_id = INI_GetLong(tmp_section, "bind_venc_id", 0, file);

        Piv->ChnAttrs[i].BindVencId = bind_venc_id;
        printf("%ld\n", bind_venc_id);
    }
    return 0;
}

static int32_t PARAM_LoadThm(const char *file, PARAM_THUMBNAIL_ATTR_S *Thm)
{
    long int i = 0;
    long int thm_cnt = 0;
    long int bind_venc_id = 0;
    char tmp_section[32] = {0};

    thm_cnt = INI_GetLong("thm_config", "thm_cnt", 0, file);
    printf("%s: thm_cnt: %ld\n", __func__, thm_cnt);

    for (i = 0; i < thm_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "thm%ld", i);
        bind_venc_id = INI_GetLong(tmp_section, "bind_venc_id", 0, file);

        Thm->ChnAttrs[i].BindVencId = bind_venc_id;

        printf("%ld\n", bind_venc_id);
    }
    return 0;
}

static int32_t PARAM_LoadSubPic(const char *file, PARAM_SUB_PIC_ATTR_S *SubPic)
{
    long int i = 0;
    long int sub_pic_cnt = 0;
    long int bind_venc_id = 0;
    char tmp_section[32] = {0};

    sub_pic_cnt = INI_GetLong("sub_pic_config", "sub_pic_cnt", 0, file);
    printf("%s: sub_pic_cnt: %ld\n", __func__, sub_pic_cnt);

    for (i = 0; i < sub_pic_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "sub_pic%ld", i);
        bind_venc_id = INI_GetLong(tmp_section, "bind_venc_id", 0, file);

        SubPic->ChnAttrs[i].BindVencId = bind_venc_id;

        printf("%ld\n", bind_venc_id);
    }
    return 0;
}


static int32_t PARAM_LoadPhoto(const char *file, PARAM_PHOTO_ATTR_S *Photo)
{
    long int i = 0;
    char tmp_section[32] = {0};
    long int ph_cnt = 0;

    ph_cnt = INI_GetLong("photo_config", "photo_cnt", 0, file);
    Photo->photoid = 0;
    Photo->VprocDev_id = INI_GetLong("photo_config", "vprocdev_id", 0, file);
    for (i = 0; i < ph_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "photo%ld", i);
        Photo->ChnAttrs[i].Enable = INI_GetLong(tmp_section, "enable", 0, file);
        Photo->ChnAttrs[i].BindVencId = INI_GetLong(tmp_section, "bind_venc_id", 0, file);
        Photo->ChnAttrs[i].BindVcapId = INI_GetLong(tmp_section, "bind_vcap_id", 0, file);
        Photo->ChnAttrs[i].EnableDumpRaw = INI_GetLong(tmp_section, "enable_dump_raw", 0, file);
    }

    printf("photo config %u\n", Photo->VprocDev_id);
    return 0;
}

static int32_t PARAM_LoadVideoMd(const char *file, PARAM_MD_ATTR_S *MDConfig)
{
    long int i = 0;
    char tmp_section[32] = {0};
    long int md_cnt = 0;
    long int motionSensitivity = 0;

    md_cnt = INI_GetLong("motionDet_config", "md_cnt", 0, file);
    motionSensitivity = INI_GetLong("motionDet_config", "motionSensitivity", 0, file);
    MDConfig->motionSensitivity = motionSensitivity;
    for (i = 0; i < md_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "md%ld", i);
        MDConfig->ChnAttrs[i].Enable = INI_GetLong(tmp_section, "enable", 0, file);
        MDConfig->ChnAttrs[i].BindVprocId = INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        MDConfig->ChnAttrs[i].BindVprocChnId = INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
    }

    printf("md motionSensitivity = %d\n", MDConfig->motionSensitivity);
    return 0;
}

#ifdef SERVICES_ADAS_ON
static int32_t PARAM_LoadADAS(const char *file, PARAM_ADAS_ATTR_S *ADASConfig)
{
    int32_t i = 0;
    char tmp_section[32] = {0};
    int32_t adas_cnt = 0;
    int32_t width = 0;
    int32_t height = 0;
    char car_model_path[128] = "";
    char lane_model_path[128] = "";
    float fps = 0;

    adas_cnt = INI_GetLong("adas_config", "adas_cnt", 0, file);
    width = INI_GetLong("adas_config", "model_vb_width", 0, file);
    height = INI_GetLong("adas_config", "model_vb_heigt", 0, file);
    fps = INI_GetFloat("adas_config", "model_fps", 0.f, file);
    INI_GetString("adas_config", "car_model_path", "", car_model_path, sizeof(car_model_path), file);
    INI_GetString("adas_config", "lane_model_path", "", lane_model_path, sizeof(lane_model_path), file);

    ADASConfig->adas_cnt = adas_cnt;
    ADASConfig->stADASModelAttr.fps = fps;
    ADASConfig->stADASModelAttr.width = width;
    ADASConfig->stADASModelAttr.height = height;
    strncpy(ADASConfig->stADASModelAttr.CarModelPath, car_model_path, sizeof(ADASConfig->stADASModelAttr.CarModelPath));
    strncpy(ADASConfig->stADASModelAttr.LaneModelPath, lane_model_path, sizeof(ADASConfig->stADASModelAttr.LaneModelPath));
    for (i = 0; i < adas_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "adas%d", i);
        ADASConfig->ChnAttrs[i].Enable = INI_GetLong(tmp_section, "enable", 0, file);
        ADASConfig->ChnAttrs[i].BindVprocId = INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        ADASConfig->ChnAttrs[i].BindVprocChnId = INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
    }

    printf("adas_cnt = %d\n", adas_cnt);
    return 0;
}
#endif

#ifdef SERVICES_QRCODE_ON
static int32_t PARAM_LoadQRCode(const char *file, PARAM_QRCODE_ATTR_S *QRCodeConfig)
{
    long int i = 0;
    char tmp_section[32] = {0};
    long int qrcode_cnt = 0;

    qrcode_cnt = INI_GetLong("qrcode_config", "qrcode_cnt", 0, file);
    QRCodeConfig->qrcode_cnt = qrcode_cnt;
    for (i = 0; i < qrcode_cnt; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "qrcode%ld", i);
        QRCodeConfig->ChnAttrs[i].Enable = INI_GetLong(tmp_section, "enable", 0, file);
        QRCodeConfig->ChnAttrs[i].BindVprocId = INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        QRCodeConfig->ChnAttrs[i].BindVprocChnId = INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
    }

    printf("md qrcode_cnt = %ld\n", qrcode_cnt);
    return 0;
}
#endif

static int32_t PARAM_LoadMediaComm(const char *filename, PARAM_MEDIA_COMM_S *MediaComm)
{
   int32_t  s32Ret = 0;

    printf("enter: %s, filename: %s\n", __func__, filename);

    s32Ret = PARAM_LoadMode(filename, &MediaComm->PowerOnMode);
    s32Ret = PARAM_LoadVO(filename, &MediaComm->Vo);
    s32Ret = PARAM_LoadWindow(filename, &MediaComm->Window);
    s32Ret = PARAM_LoadAi(filename, &MediaComm->Ai);
    s32Ret = PARAM_LoadAenc(filename, &MediaComm->Aenc);
    s32Ret = PARAM_LoadAo(filename, &MediaComm->Ao);
    s32Ret = PARAM_LoadRecord(filename, &MediaComm->Record);
    s32Ret = PARAM_LoadRtsp(filename, &MediaComm->Rtsp);
    s32Ret = PARAM_LoadPiv(filename, &MediaComm->Piv);
    s32Ret = PARAM_LoadThm(filename, &MediaComm->Thumbnail);
    s32Ret = PARAM_LoadSubPic(filename, &MediaComm->SubPic);
    s32Ret = PARAM_LoadPhoto(filename, &MediaComm->Photo);
    s32Ret = PARAM_LoadVideoMd(filename, &MediaComm->Md);
    s32Ret = PARAM_LoadFacep(filename, &MediaComm->Facep);
#ifdef SERVICES_SPEECH_ON
    s32Ret = PARAM_LoadSpeech(filename, &MediaComm->Speech);
#endif
#ifdef SERVICES_ADAS_ON
    s32Ret = PARAM_LoadADAS(filename, &MediaComm->ADAS);
#endif
#ifdef SERVICES_QRCODE_ON
    s32Ret = PARAM_LoadQRCode(filename, &MediaComm->QRCODE);
#endif
    return s32Ret;
}

int32_t  INI_PARAM_LoadMediaCommCfg(PARAM_MEDIA_COMM_S *MediaComm)
{
    printf("\n---enter: %s\n", __func__);
    uint32_t i = 0;
    char filepath[PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_media_comm")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a media comm file
            PARAM_LoadMediaComm(filepath, MediaComm);
            break;
        }
    }

    if(i >= g_ParamAccess.module_num) {
        printf("error , can not find media comm file\n");
        return -1;
    }

    return 0;
}
/* common config end */


/* special config begin */
static int32_t PARAM_LoadMediaMode(const char *file, uint32_t *MediaMode)
{
   int32_t  s32Ret = 0;
    char tmp_section[16] = {0};
    INI_GetString("common", "media_mode", "", tmp_section,
                        PARAM_MODULE_NAME_LEN, file);

    s32Ret = INI_PARAM_MediaString2Uint(MediaMode, tmp_section);
    if(s32Ret == -1) {
        return -1;
    }
    printf("%s: file: %s modename: %d\n", __func__, file, *MediaMode);
    return 0;
}

static int32_t PARAM_LoadSns(const char *file, PARAM_MEDIA_SNS_ATTR_S *Sns, int32_t index)
{
    long int enable = 0;
    long int id = 0;
    long int wdrmode = 0;
    long int i2cid = 0;
    long int i2caddr = 0;
    long int hwsync = 0;
    long int cam_clk_id = 0;
    long int rst_gpio_inx = 0;
    long int rst_gpio_pin = 0;
    long int rst_gpio_pol = 0;
    long int mipidev = 0;
    long int mclk_en = 0;
    long int lane_switch_pin = 0;
    long int lane_switch_pin_pol = 0;
    int32_t  i = 0;
    char tmp_section[16] = {0};
    long int tmpid = 0;
    uint32_t sensortype = 0;
    char sensor_config[64] = {0};
    uint32_t parse_sensor_switch_config = 0;
    MAPI_VCAP_SENSOR_ATTR_T *sns_chn_attr_ptr = &Sns->SnsChnAttr;
    snprintf(sensor_config, sizeof(sensor_config), "sensor_config%d", index);

    enable = INI_GetLong(sensor_config, "enable", 0, file);
    Sns->SnsEnable = enable;

PARSE_SENSOR_SWITCH:
    id = INI_GetLong(sensor_config, "id", 0, file);
    wdrmode = INI_GetLong(sensor_config, "wdrmode", 0, file);
    i2cid = INI_GetLong(sensor_config, "i2cid", 0, file);
    i2caddr = INI_GetLong(sensor_config, "i2caddr", 0, file);
    hwsync = INI_GetLong(sensor_config, "hwsync", 0, file);
    cam_clk_id = INI_GetLong(sensor_config, "cam_clk_id", 0, file);
    rst_gpio_inx = INI_GetLong(sensor_config, "rst_gpio_inx", 0, file);
    rst_gpio_pin = INI_GetLong(sensor_config, "rst_gpio_pin", 0, file);
    rst_gpio_pol = INI_GetLong(sensor_config, "rst_gpio_pol", 0, file);
    mipidev = INI_GetLong(sensor_config, "mipidev", 0, file);
    sensortype = INI_GetLong(sensor_config, "sensortype", 0, file);
    mclk_en = INI_GetLong(sensor_config, "mclk_en", 0, file);
    lane_switch_pin = INI_GetLong(sensor_config, "lane_switch_pin", 0, file);
    lane_switch_pin_pol = INI_GetLong(sensor_config, "lane_switch_pin_pol", 0, file);


    printf("%s: %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
        __func__, enable, id, wdrmode, i2cid, i2caddr, hwsync, mclk_en,
        lane_switch_pin, lane_switch_pin_pol);

    sns_chn_attr_ptr->u8SnsId = id;
    sns_chn_attr_ptr->u8WdrMode = wdrmode;
    sns_chn_attr_ptr->u8I2cBusId = i2cid;
    sns_chn_attr_ptr->u8I2cSlaveAddr = i2caddr;
    sns_chn_attr_ptr->u8HwSync = hwsync;
    sns_chn_attr_ptr->u8MipiDev = mipidev;
    sns_chn_attr_ptr->u8CamClkId = cam_clk_id;
    sns_chn_attr_ptr->u8RstGpioInx = rst_gpio_inx;
    sns_chn_attr_ptr->u8RstGpioPin = rst_gpio_pin;
    sns_chn_attr_ptr->u8RstGpioPol = rst_gpio_pol;
    sns_chn_attr_ptr->u32sensortype = sensortype;
    sns_chn_attr_ptr->bMclkEn = mclk_en;
    sns_chn_attr_ptr->u32LaneSwitchPin = lane_switch_pin;
    sns_chn_attr_ptr->u32LaneSwitchPinPol = lane_switch_pin_pol;
    for (i = 0; i < 5; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "laneid%d", i);
        tmpid = INI_GetLong(sensor_config, tmp_section, 0, file);
        printf(" lanid%d : %ld", i, tmpid);
        sns_chn_attr_ptr->as8LaneId[i] = tmpid;
    }
    for (i = 0; i < 5; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "swap%d", i);
        tmpid = INI_GetLong(sensor_config, tmp_section, 0, file);
        printf(" swap%d : %ld", i, tmpid);
        sns_chn_attr_ptr->as8PNSwap[i] = tmpid;
    }
    printf("\n");

    for (i = 0; i< VI_MAX_PIPE_NUM; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        sprintf(tmp_section, "apipe%d", i);
        sns_chn_attr_ptr->aPipe[i] = INI_GetLong(sensor_config, tmp_section, -1, file);
    }
    printf("\n");

    if(parse_sensor_switch_config){
        return 0;
    }
    memset(sensor_config, 0, sizeof(sensor_config));
    snprintf(sensor_config, sizeof(sensor_config), "sensor_switch_config%d", index);
    enable = INI_GetLong(sensor_config, "enable", 0, file);
    printf("sensor_switch_config: %ld\n", enable);
    if(enable){
        sns_chn_attr_ptr = &Sns->SnsSwitchChnAttr;
        parse_sensor_switch_config = 1;
        goto PARSE_SENSOR_SWITCH;
    }
    return 0;
}

static int32_t PARAM_LoadVcap(const char *file, PARAM_MEDIA_VACP_ATTR_S *Vcap, int32_t index)
{
    long int enable = 0;
    long int id = 0;
    long int width = 0;
    long int height = 0;
    long int fmt = 0;
    long int cpress = 0;
    long int mirror = 0;
    long int filp = 0;
    long int vbcnt = 0;
    float fps = 0;
    char vcap_config[64] = {0};
    uint32_t parse_vcap_switch_config = 0;
    MAPI_VCAP_CHN_ATTR_T *vcap_chn_attr_ptr = &Vcap->VcapChnAttr;
    snprintf(vcap_config, sizeof(vcap_config), "vcap_config%d", index);

    enable = INI_GetLong(vcap_config, "enable", 0, file);
    id = INI_GetLong(vcap_config, "id", 0, file);
    Vcap->VcapEnable = enable;
    Vcap->VcapId = id;

PARSE_VCAP_SWITCH:
    width = INI_GetLong(vcap_config, "width", 0, file);
    height = INI_GetLong(vcap_config, "height", 0, file);
    fmt = INI_GetLong(vcap_config, "fmt", 0, file);
    cpress = INI_GetLong(vcap_config, "cpress", 0, file);
    mirror = INI_GetLong(vcap_config, "mirror", 0, file);
    filp = INI_GetLong(vcap_config, "filp", 0, file);
    fps = INI_GetFloat(vcap_config, "fps", 0.f, file);
    vbcnt = INI_GetLong(vcap_config, "chn_vbcnt", 0, file);
    long int scene_mode = 0;
    scene_mode = INI_GetLong(vcap_config, "scene_mode", 0, file);

    printf("%s: %ld %ld %ld %ld %ld %ld %ld %ld %f %ld\n",
        __func__, enable, id, width, height, fmt, cpress, mirror, filp, fps, scene_mode);

    vcap_chn_attr_ptr->u32Width = width;
    vcap_chn_attr_ptr->u32Height = height;
    vcap_chn_attr_ptr->enPixelFmt = fmt;
    vcap_chn_attr_ptr->enCompressMode = cpress;
    vcap_chn_attr_ptr->bMirror = mirror;
    vcap_chn_attr_ptr->bFlip = filp;
    vcap_chn_attr_ptr->f32Fps = fps;
    vcap_chn_attr_ptr->vbcnt = vbcnt;
    vcap_chn_attr_ptr->scenemode = scene_mode;

    if(parse_vcap_switch_config){
        return 0;
    }
    memset(vcap_config, 0, sizeof(vcap_config));
    snprintf(vcap_config, sizeof(vcap_config), "vcap_switch_config%d", index);
    enable = INI_GetLong(vcap_config, "enable", 0, file);
    printf("vcap_switch_config: %ld\n", enable);
    if(enable){
        vcap_chn_attr_ptr = &Vcap->VcapSwitchChnAttr;
        parse_vcap_switch_config = 1;
        goto PARSE_VCAP_SWITCH;
    }

    return 0;
}

static int32_t PARAM_LoadVproc(const char *file, PARAM_MEDIA_VPROC_ATTR_S *Vproc)
{
    long int vproc_cnt = 0;
    vproc_cnt = INI_GetLong("vproc_config", "vproc_cnt", 0, file);
    printf("%s: vproc_cnt:%ld\n",__func__, vproc_cnt);
    Vproc->VprocCnt = vproc_cnt;
    long int grp_id = 0;
    long int grp_enable = 0;
    long int vpss_dev = 0;
    long int width = 0;
    long int height = 0;
    long int fmt = 0;
    long int srcfps = 0;
    long int dstfps = 0;
    long int chn_num = 0;
    long int bind_enable = 0;
    long int src_mod_id = 0;
    long int src_dev_id = 0;
    long int src_chn_id = 0;
    long int dst_mod_id = 0;
    long int dst_dev_id = 0;
    long int dst_chn_id = 0;
    long int crop_enable = 0;
    long int crop_coordinate = 0;
    long int crop_x = 0;
    long int crop_y = 0;
    long int crop_w = 0;
    long int crop_h = 0;
    for (int32_t i = 0; i < vproc_cnt; i++) {
        char tmp_section[16] = {0};
        snprintf(tmp_section, sizeof(tmp_section), "vproc%d", i);
        printf("section: %s\n", tmp_section);

        grp_id = INI_GetLong(tmp_section, "grp_id", 0, file);
        grp_enable = INI_GetLong(tmp_section, "grp_enable", 0, file);
        vpss_dev = INI_GetLong(tmp_section, "vpss_dev", 0, file);
        width = INI_GetLong(tmp_section, "width", 0, file);
        height = INI_GetLong(tmp_section, "height", 0, file);
        fmt = INI_GetLong(tmp_section, "fmt", 0, file);
        srcfps = INI_GetLong(tmp_section, "srcfps", 0, file);
        dstfps = INI_GetLong(tmp_section, "dstfps", 0, file);
        bind_enable = INI_GetLong(tmp_section, "bind_enable", 0, file);
        src_mod_id = INI_GetLong(tmp_section, "src_mod_id", 0, file);
        src_dev_id = INI_GetLong(tmp_section, "src_dev_id", 0, file);
        src_chn_id = INI_GetLong(tmp_section, "src_chn_id", 0, file);
        dst_mod_id = INI_GetLong(tmp_section, "dst_mod_id", 0, file);
        dst_dev_id = INI_GetLong(tmp_section, "dst_dev_id", 0, file);
        dst_chn_id = INI_GetLong(tmp_section, "dst_chn_id", 0, file);
        chn_num = INI_GetLong(tmp_section, "chn_num", 0, file);

        crop_enable = INI_GetLong(tmp_section, "crop_enable", 0, file);
        crop_coordinate = INI_GetLong(tmp_section, "crop_coordinate", 0, file);
        crop_x = INI_GetLong(tmp_section, "crop_x", 0, file);
        crop_y = INI_GetLong(tmp_section, "crop_y", 0, file);
        crop_w = INI_GetLong(tmp_section, "crop_w", 0, file);
        crop_h = INI_GetLong(tmp_section, "crop_h", 0, file);

        Vproc->VprocGrpAttr[i].Vprocid = grp_id;
        Vproc->VprocGrpAttr[i].VprocEnable = grp_enable;
        Vproc->VprocGrpAttr[i].VpssDev = vpss_dev;
        Vproc->VprocGrpAttr[i].BindEnable = bind_enable;
        Vproc->VprocGrpAttr[i].stSrcChn.enModId = src_mod_id;
        Vproc->VprocGrpAttr[i].stSrcChn.s32DevId = src_dev_id;
        Vproc->VprocGrpAttr[i].stSrcChn.s32ChnId = src_chn_id;
        Vproc->VprocGrpAttr[i].stDestChn.enModId = dst_mod_id;
        Vproc->VprocGrpAttr[i].stDestChn.s32DevId = dst_dev_id;
        Vproc->VprocGrpAttr[i].stDestChn.s32ChnId = dst_chn_id;
        Vproc->VprocGrpAttr[i].ChnCnt = chn_num;

        Vproc->VprocGrpAttr[i].VpssGrpAttr.u32MaxW = width;
        Vproc->VprocGrpAttr[i].VpssGrpAttr.u32MaxH = height;
        Vproc->VprocGrpAttr[i].VpssGrpAttr.enPixelFormat = fmt;
        Vproc->VprocGrpAttr[i].VpssGrpAttr.stFrameRate.s32SrcFrameRate = srcfps;
        Vproc->VprocGrpAttr[i].VpssGrpAttr.stFrameRate.s32DstFrameRate = dstfps;
        Vproc->VprocGrpAttr[i].VpssGrpAttr.u8VpssDev = vpss_dev;

        Vproc->VprocGrpAttr[i].VpssCropInfo.bEnable = crop_enable > 0 ? true : false;
        Vproc->VprocGrpAttr[i].VpssCropInfo.enCropCoordinate = crop_coordinate;
        Vproc->VprocGrpAttr[i].VpssCropInfo.stCropRect.s32X = crop_x;
        Vproc->VprocGrpAttr[i].VpssCropInfo.stCropRect.s32Y = crop_y;
        Vproc->VprocGrpAttr[i].VpssCropInfo.stCropRect.u32Width = crop_w;
        Vproc->VprocGrpAttr[i].VpssCropInfo.stCropRect.u32Height = crop_h;

        printf("%s: grp:%ld %ld %ld %ld\n"
            "%ld %ld %ld %ld %ld %ld\n"
            "%ld %ld %ld %ld %ld %ld\n"
            "%ld %ld %ld %ld %ld %ld\n",
            __func__, grp_id, grp_enable, vpss_dev, bind_enable,
            src_mod_id, src_dev_id, src_chn_id, dst_mod_id, dst_dev_id, dst_chn_id,
            width, height, fmt, srcfps, dstfps, chn_num,
            crop_enable, crop_coordinate, crop_x, crop_y, crop_w, crop_h);

        for (int32_t j = 0; j < chn_num; j++) {
            snprintf(tmp_section, sizeof(tmp_section), "vproc%d_chn%d", i, j);

            long int chn_id = 0;
            long int chn_enable = 0;
            long int chn_width = 0;
            long int chn_height = 0;
            long int chn_vfmt = 0;
            long int chn_fmt = 0;
            long int chn_depth = 0;
            long int chn_srcfps = 0;
            long int chn_dstfps = 0;
            long int chn_mirror = 0;
            long int chn_filp = 0;
            long int chn_vbcnt = 0;
            long int lowdelay_cnt = 0;
            long int aspect_ratio_mode = 0;
            long int aspect_ratio_rect_x = 0;
            long int aspect_ratio_rect_y = 0;
            long int aspect_ratio_rect_w = 0;
            long int aspect_ratio_rect_h = 0;
            long int aspect_ratio_bg_color_en = 0;
            long int aspect_ratio_bg_color = 0;
            long int sbm_enable = 0;
            long int sbm_buf_line = 0;
            long int sbm_buf_num = 0;
            long int chn_crop_enable = 0;
            long int chn_crop_coordinate = 0;
            long int chn_crop_x = 0;
            long int chn_crop_y = 0;
            long int chn_crop_w = 0;
            long int chn_crop_h = 0;
            long int chn_rotation = 0;

            chn_id = INI_GetLong(tmp_section, "chn_id", 0, file);
            chn_enable = INI_GetLong(tmp_section, "chn_enable", 0, file);
            chn_width = INI_GetLong(tmp_section, "chn_width", 0, file);
            chn_height = INI_GetLong(tmp_section, "chn_height", 0, file);
            chn_vfmt = INI_GetLong(tmp_section, "chn_vmt", 0, file);
            chn_fmt = INI_GetLong(tmp_section, "chn_fmt", 0, file);
            chn_depth = INI_GetLong(tmp_section, "chn_depth", 0, file);
            chn_srcfps = INI_GetLong(tmp_section, "chn_srcfps", 0, file);
            chn_dstfps = INI_GetLong(tmp_section, "chn_dstfps", 0, file);
            chn_mirror = INI_GetLong(tmp_section, "chn_mirror", 0, file);
            chn_filp = INI_GetLong(tmp_section, "chn_filp", 0, file);
            chn_vbcnt = INI_GetLong(tmp_section, "chn_vbcnt", 0, file);
            lowdelay_cnt = INI_GetLong(tmp_section, "lowdelay_cnt", 0, file);
            aspect_ratio_mode = INI_GetLong(tmp_section, "aspect_ratio_mode", 0, file);
            aspect_ratio_rect_x = INI_GetLong(tmp_section, "aspect_ratio_rect_x", 0, file);
            aspect_ratio_rect_y = INI_GetLong(tmp_section, "aspect_ratio_rect_y", 0, file);
            aspect_ratio_rect_w = INI_GetLong(tmp_section, "aspect_ratio_rect_w", 0, file);
            aspect_ratio_rect_h = INI_GetLong(tmp_section, "aspect_ratio_rect_h", 0, file);
            aspect_ratio_bg_color_en = INI_GetLong(tmp_section, "aspect_ratio_bg_color_en", 0, file);
            aspect_ratio_bg_color = INI_GetLong(tmp_section, "aspect_ratio_bg_color", 0, file);
            sbm_enable = INI_GetLong(tmp_section, "sbm_enable", 0, file);
            sbm_buf_line = INI_GetLong(tmp_section, "sbm_buf_line", 0, file);
            sbm_buf_num = INI_GetLong(tmp_section, "sbm_buf_num", 0, file);

            chn_crop_enable = INI_GetLong(tmp_section, "chn_crop_enable", 0, file);
            chn_crop_coordinate = INI_GetLong(tmp_section, "chn_crop_coordinate", 0, file);
            chn_crop_x = INI_GetLong(tmp_section, "chn_crop_x", 0, file);
            chn_crop_y = INI_GetLong(tmp_section, "chn_crop_y", 0, file);
            chn_crop_w = INI_GetLong(tmp_section, "chn_crop_w", 0, file);
            chn_crop_h = INI_GetLong(tmp_section, "chn_crop_h", 0, file);
            chn_rotation = INI_GetLong(tmp_section, "chn_rotation", 0, file);

            printf("chn:%ld %ld %ld %ld %ld %ld %ld %ld\n"
                "%ld %ld %ld %ld %ld\n"
                "%ld %ld %ld %ld %ld %ld %ld\n",
                chn_id, chn_width, chn_height, chn_vfmt, chn_fmt, chn_depth, chn_srcfps, chn_dstfps,
                chn_enable, chn_mirror, chn_filp, chn_vbcnt, lowdelay_cnt,
                aspect_ratio_mode, aspect_ratio_rect_x, aspect_ratio_rect_y, aspect_ratio_rect_w, aspect_ratio_rect_h,
                aspect_ratio_bg_color_en, aspect_ratio_bg_color);

            printf("chn crop: %ld %ld %ld %ld %ld %ld\n",
                chn_crop_enable, chn_crop_coordinate, chn_crop_x, chn_crop_y, chn_crop_w, chn_crop_h);

            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VprocChnid = chn_id;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VprocChnEnable = chn_enable;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.u32Width = chn_width;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.u32Height = chn_height;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.enVideoFormat = chn_vfmt;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.enPixelFormat = chn_fmt;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.u32Depth = chn_depth;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stFrameRate.s32SrcFrameRate = chn_srcfps;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stFrameRate.s32DstFrameRate = chn_dstfps;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.bMirror = chn_mirror;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.bFlip = chn_filp;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VprocChnVbCnt = chn_vbcnt;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VprocChnLowDelayCnt = lowdelay_cnt;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.enMode = aspect_ratio_mode;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.stVideoRect.s32X = aspect_ratio_rect_x;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.stVideoRect.s32Y = aspect_ratio_rect_y;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.stVideoRect.u32Width = aspect_ratio_rect_w;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.stVideoRect.u32Height = aspect_ratio_rect_h;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.bEnableBgColor = aspect_ratio_bg_color_en;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnAttr.stAspectRatio.u32BgColor = aspect_ratio_bg_color;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssBufWrap.bEnable = sbm_enable;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssBufWrap.u32BufLine = sbm_buf_line;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssBufWrap.u32WrapBufferSize = sbm_buf_num;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnCropInfo.bEnable = chn_crop_enable > 0 ? true : false;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnCropInfo.enCropCoordinate = chn_crop_coordinate;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnCropInfo.stCropRect.s32X = chn_crop_x;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnCropInfo.stCropRect.s32Y = chn_crop_y;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnCropInfo.stCropRect.u32Width = chn_crop_w;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].VpssChnCropInfo.stCropRect.u32Height = chn_crop_h;
            Vproc->VprocGrpAttr[i].VprocChnAttr[j].enRotation = chn_rotation;
        }
    }

    return 0;
}

static int32_t PARAM_LoadVenc(const char *file, PARAM_MEDIA_VENC_ATTR_S *Venc)
{
    long int i = 0;
    long int chn_num = 0;

    long int enable = 0;
    long int id = 0;
    long int sbm_enable = 0;
    long int bind_vproc_id = 0;
    long int bind_vproc_chn_id = 0;
    long int venc_bind_mode = 0;
    long int width = 0;
    long int height = 0;
    long int codec = 0;
    long int pixel_fmt = 0;
    long int gop = 0;
    long int profile = 0;
    float fps = 0;
    long int rate_ctrl_mode = 0;
    long int bit_rate = 0;
    long int iqp = 0;
    long int pqp = 0;
    long int min_iqp = 0;
    long int max_iqp = 0;
    long int min_qp = 0;
    long int max_qp = 0;
    long int initialDelay = 0;
    long int thrdLv = 0;
    long int statTime = 0;
    long int change_pos = 0;
    long int jpeg_quality = 0;
    long int single_EsBuf = 0;
    long int bitstream_bufSize = 0;
    long int datafifo_len = 0;
    char tmp_section[16] = {0};
    long int src_framerate = 0;
    long int dst_framerate = 0;
    long int aspectRatio_Flag = 0;
    long int overscan_Flag = 0;
    long int videoSignalType_Flag = 0;
    long int video_Format = 0;
    long int videoFull_Flag = 0;
    long int colourDescription_Flag = 0;
    long int firstFrameStartQp = 0;
    long int maxBitRate = 0;
    long int gop_mode = 0;
    long int maxIprop = 0;
    long int minIprop = 0;
    long int minStillPercent = 0;
    long int maxStillQP = 0;
    long int avbrPureStillThr = 0;
    long int motionSensitivity = 0;
    long int bgDeltaQp = 0;
    long int rowQpDelta = 0;
    long int ipqpDelta = 0;

    chn_num = INI_GetLong("venc_config", "chn_num", 0, file);
    printf("%s: chn_num: %ld\n", __func__, chn_num);
    // Venc->ChnCnt = chn_num;

    for (i = 0; i < chn_num; i++) {
        snprintf(tmp_section, sizeof(tmp_section), "venc_chn%ld", i);
        printf("section: %s\n", tmp_section);

        enable = INI_GetLong(tmp_section, "enable", 0, file);
        id = INI_GetLong(tmp_section, "id", 0, file);
        sbm_enable = INI_GetLong(tmp_section, "sbm_enable", 0, file);
        bind_vproc_id = INI_GetLong(tmp_section, "bind_vproc_id", 0, file);
        bind_vproc_chn_id = INI_GetLong(tmp_section, "bind_vproc_chn_id", 0, file);
        venc_bind_mode = INI_GetLong(tmp_section, "venc_bind_mode", 0, file);
        width = INI_GetLong(tmp_section, "width", 0, file);
        height = INI_GetLong(tmp_section, "height", 0, file);
        codec = INI_GetLong(tmp_section, "codec", 0, file);
        pixel_fmt = INI_GetLong(tmp_section, "pixel_fmt", 0, file);
        gop = INI_GetLong(tmp_section, "gop", 0, file);
        profile = INI_GetLong(tmp_section, "profile", 0, file);
        fps = INI_GetFloat(tmp_section, "fps", 0.f, file);
        rate_ctrl_mode = INI_GetLong(tmp_section, "rate_ctrl_mode", 0, file);
        bit_rate = INI_GetLong(tmp_section, "bit_rate", 0, file);
        iqp = INI_GetLong(tmp_section, "iqp", 0, file);
        pqp = INI_GetLong(tmp_section, "pqp", 0, file);
        min_iqp = INI_GetLong(tmp_section, "min_iqp", 0, file);
        max_iqp = INI_GetLong(tmp_section, "max_iqp", 0, file);
        min_qp = INI_GetLong(tmp_section, "min_qp", 0, file);
        max_qp = INI_GetLong(tmp_section, "max_qp", 0, file);
        change_pos = INI_GetLong(tmp_section, "change_pos", 0, file);
        jpeg_quality = INI_GetLong(tmp_section, "jpeg_quality", 0, file);
        single_EsBuf = INI_GetLong(tmp_section, "single_EsBuf", 0, file);
        bitstream_bufSize = INI_GetLong(tmp_section, "bufSize", 0, file);
        datafifo_len = INI_GetLong(tmp_section, "datafifoLen", 10, file);
        src_framerate = INI_GetLong(tmp_section, "src_framerate", 0, file);
        dst_framerate = INI_GetLong(tmp_section, "dst_framerate", 0, file);
        initialDelay = INI_GetLong(tmp_section, "initialdelay", 0, file);
        thrdLv = INI_GetLong(tmp_section, "thrdlv", 0, file);
        statTime = INI_GetLong(tmp_section, "stattime", 0, file);
        aspectRatio_Flag = INI_GetLong(tmp_section, "aspectRatio_Flag", 0, file);
        overscan_Flag = INI_GetLong(tmp_section, "overscan_Flag", 0, file);
        videoSignalType_Flag = INI_GetLong(tmp_section, "videoSignalType_Flag", 0, file);
        video_Format = INI_GetLong(tmp_section, "video_Format", 0, file);
        videoFull_Flag = INI_GetLong(tmp_section, "videoFull_Flag", 0, file);
        colourDescription_Flag = INI_GetLong(tmp_section, "colourDescription_Flag", 0, file);
        firstFrameStartQp = INI_GetLong(tmp_section, "firstFrameStartQp", 0, file);
        maxBitRate = INI_GetLong(tmp_section, "maxBitRate", 0, file);
        gop_mode = INI_GetLong(tmp_section, "gop_mode", 0, file);
        maxIprop = INI_GetLong(tmp_section, "maxIprop", 0, file);
        minIprop = INI_GetLong(tmp_section, "minIprop", 0, file);
        minStillPercent = INI_GetLong(tmp_section, "minStillPercent", 0, file);
        maxStillQP = INI_GetLong(tmp_section, "maxStillQP", 0, file);
        avbrPureStillThr = INI_GetLong(tmp_section, "avbrPureStillThr", 0, file);
        motionSensitivity = INI_GetLong(tmp_section, "motionSensitivity", 0, file);
        bgDeltaQp = INI_GetLong(tmp_section, "bgDeltaQp", 0, file);
        rowQpDelta = INI_GetLong(tmp_section, "rowQpDelta", 0, file);
        ipqpDelta = INI_GetLong(tmp_section, "ipqpDelta", 0, file);
        printf("%ld %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld %f %ld \
                %ld %ld %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld %ld \
                %ld %ld %ld %ld\n",
            enable, id, sbm_enable, bind_vproc_id, bind_vproc_chn_id, width,
            height, codec, pixel_fmt, gop, fps, rate_ctrl_mode,
            bit_rate, iqp, pqp, min_iqp, max_iqp, min_qp, max_qp,
            single_EsBuf, bitstream_bufSize, datafifo_len, src_framerate, dst_framerate,
            initialDelay, thrdLv, statTime, aspectRatio_Flag, overscan_Flag,
            videoSignalType_Flag, video_Format, videoFull_Flag, colourDescription_Flag);

        /* set float fps into src_framerate & dst_framerate */
        if (fps>0 && src_framerate==0 && dst_framerate==0) {
            long int fpsNum = (long int)fps, fpsDenom=0;
            if ((float)fpsNum != fps){
                fpsDenom = 100;
                float tmp = fps*fpsDenom;
                fpsNum=(long int)tmp;
                src_framerate = (fpsDenom<<16) + fpsNum;
                dst_framerate = src_framerate;
            } else {
                src_framerate = fpsNum;
                dst_framerate = fpsNum;
            }
            printf("float venc fps: %ld, %ld\n", src_framerate, dst_framerate);
        }

        Venc->VencChnAttr[i].VencChnEnable = enable;
        Venc->VencChnAttr[i].VencId = id;
        Venc->VencChnAttr[i].sbm_enable = sbm_enable;
        Venc->VencChnAttr[i].BindVprocId = bind_vproc_id;
        Venc->VencChnAttr[i].BindVprocChnId = bind_vproc_chn_id;
        Venc->VencChnAttr[i].bindMode = venc_bind_mode;
        Venc->VencChnAttr[i].framerate = fps;
        Venc->VencChnAttr[i].MapiVencAttr.width = width;
        Venc->VencChnAttr[i].MapiVencAttr.height = height;
        Venc->VencChnAttr[i].MapiVencAttr.codec = codec;
        Venc->VencChnAttr[i].MapiVencAttr.pixel_format = pixel_fmt;
        Venc->VencChnAttr[i].MapiVencAttr.gop = gop;
        Venc->VencChnAttr[i].MapiVencAttr.profile = profile;
        Venc->VencChnAttr[i].MapiVencAttr.rate_ctrl_mode = rate_ctrl_mode;
        Venc->VencChnAttr[i].MapiVencAttr.bitrate_kbps = bit_rate;
        Venc->VencChnAttr[i].MapiVencAttr.iqp = iqp;
        Venc->VencChnAttr[i].MapiVencAttr.pqp = pqp;
        Venc->VencChnAttr[i].MapiVencAttr.minIqp = min_iqp;
        Venc->VencChnAttr[i].MapiVencAttr.maxIqp = max_iqp;
        Venc->VencChnAttr[i].MapiVencAttr.minQp = min_qp;
        Venc->VencChnAttr[i].MapiVencAttr.maxQp = max_qp;
        Venc->VencChnAttr[i].MapiVencAttr.changePos = change_pos;
        Venc->VencChnAttr[i].MapiVencAttr.single_EsBuf = single_EsBuf;
        Venc->VencChnAttr[i].MapiVencAttr.bufSize = bitstream_bufSize;
        Venc->VencChnAttr[i].MapiVencAttr.datafifoLen = datafifo_len;
        Venc->VencChnAttr[i].MapiVencAttr.jpeg_quality = jpeg_quality;
        Venc->VencChnAttr[i].MapiVencAttr.src_framerate = src_framerate;
        Venc->VencChnAttr[i].MapiVencAttr.dst_framerate = dst_framerate;
        Venc->VencChnAttr[i].MapiVencAttr.initialDelay = initialDelay;
        Venc->VencChnAttr[i].MapiVencAttr.thrdLv = thrdLv;
        Venc->VencChnAttr[i].MapiVencAttr.statTime = statTime;
        Venc->VencChnAttr[i].MapiVencAttr.aspectRatioInfoPresentFlag = aspectRatio_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.overscanInfoPresentFlag = overscan_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.videoSignalTypePresentFlag = videoSignalType_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.videoFormat = video_Format;
        Venc->VencChnAttr[i].MapiVencAttr.videoFullRangeFlag = videoFull_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.colourDescriptionPresentFlag = colourDescription_Flag;
        Venc->VencChnAttr[i].MapiVencAttr.firstFrameStartQp = firstFrameStartQp;
        Venc->VencChnAttr[i].MapiVencAttr.maxBitRate = maxBitRate;
        Venc->VencChnAttr[i].MapiVencAttr.gop_mode = gop_mode;
        Venc->VencChnAttr[i].MapiVencAttr.maxIprop = maxIprop;
        Venc->VencChnAttr[i].MapiVencAttr.minIprop = minIprop;
        Venc->VencChnAttr[i].MapiVencAttr.minStillPercent = minStillPercent;
        Venc->VencChnAttr[i].MapiVencAttr.maxStillQP = maxStillQP;
        Venc->VencChnAttr[i].MapiVencAttr.avbrPureStillThr = avbrPureStillThr;
        Venc->VencChnAttr[i].MapiVencAttr.motionSensitivity = motionSensitivity;
        Venc->VencChnAttr[i].MapiVencAttr.bgDeltaQp = bgDeltaQp;
        Venc->VencChnAttr[i].MapiVencAttr.rowQpDelta = rowQpDelta;
        Venc->VencChnAttr[i].MapiVencAttr.ipqpDelta = ipqpDelta;
    }
    return 0;
}

static int32_t PARAM_LoadVB(const char *file, PARAM_MEDIA_VB_ATTR_S *Vb)
{
    long int i = 0;
    long int pool_cnt = 0;
    long int enable = 0;
    long int frame_width = 0;
    long int frame_height = 0;
    long int frame_fmt = 0;
    long int blk_size = 0;
    long int blk_cnt = 0;
    char tmp_section[32] = {0};

    pool_cnt = INI_GetLong("vb_config", "vbpool_cnt", 0, file);
    printf("%s: pool_cnt: %ld\n", __func__, pool_cnt);
    Vb->Poolcnt = pool_cnt;

    for (i = 0; i < pool_cnt; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vbpool%ld", i);
        enable = INI_GetLong(tmp_section, "enable", 0, file);
        blk_cnt = INI_GetLong(tmp_section, "blk_cnt", 0, file);

        Vb->Vbpool[i].is_frame = enable;
        if(enable) {
            frame_width = INI_GetLong(tmp_section, "frame_width", 0, file);
            frame_height = INI_GetLong(tmp_section, "frame_height", 0, file);
            frame_fmt = INI_GetLong(tmp_section, "frame_fmt", 0, file);

            Vb->Vbpool[i].vb_blk_size.frame.width = frame_width;
            Vb->Vbpool[i].vb_blk_size.frame.height = frame_height;
            Vb->Vbpool[i].vb_blk_size.frame.fmt = frame_fmt;
        } else {
            blk_size = INI_GetLong(tmp_section, "blk_size", 0, file);
            Vb->Vbpool[i].vb_blk_size.size = blk_size;
        }

        Vb->Vbpool[i].vb_blk_num = blk_cnt;

        printf("%ld %ld %ld %ld %ld %ld\n", enable, frame_width, frame_height, frame_fmt, blk_size, blk_cnt);
    }

    return 0;
}

static int32_t PARAM_LoadMediaSpec(const char *file, PARAM_MEDIA_SPEC_S *MediaMode, int32_t  index)
{
   int32_t  s32Ret = 0;
    s32Ret = PARAM_LoadMediaMode(file, &MediaMode->MediaMode);
    s32Ret |= PARAM_LoadSns(file, &MediaMode->SnsAttr, index);
    s32Ret |= PARAM_LoadVcap(file, &MediaMode->VcapAttr, index);
    s32Ret |= PARAM_LoadVproc(file, &MediaMode->VprocAttr);
    s32Ret |= PARAM_LoadVenc(file, &MediaMode->VencAttr);
    s32Ret |= PARAM_LoadVB(file, &MediaMode->Vb);
    s32Ret |= PARAM_LoadOsd(file, &MediaMode->Osd);
    return s32Ret;
}


static int32_t PARAM_LoadMediaCam(const char *comm_file, PARAM_CAM_CFG *CamCfg, int32_t index)
{
   int32_t  s32Ret = 0;
    long int enable = 0;
    long int cam_id = 0;
    long int osdshow = 0;
    uint32_t cur_mode = 0;
    long int count = 0;
    char tmp_section[16] = {0};

    uint32_t i = 0;
    uint32_t j = 0;
    char cam_name[PARAM_MODULE_NAME_LEN] = {0};
    char tmp_name[PARAM_MODULE_NAME_LEN] = {0};

    enable = INI_GetLong("camera", "enable", 0, comm_file);
    cam_id = INI_GetLong("camera", "cam_id", 0, comm_file);
    osdshow = INI_GetLong("camera", "osdshow", 0, comm_file);

    INI_GetString("camera", "cur_mode", "", tmp_section,
                        PARAM_MODULE_NAME_LEN, comm_file);

    s32Ret = INI_PARAM_MediaString2Uint(&cur_mode, tmp_section);
    if(s32Ret == -1) {
        return -1;
    }

    count = INI_GetLong("mediamode", "count", 0, comm_file);

    CamCfg->CamEnable = enable;
    CamCfg->CamMediaInfo.CamID = cam_id;
    CamCfg->CamMediaInfo.CurMediaMode = cur_mode;
    CamCfg->MediaModeCnt = count;

    printf("cam_id = %ld :%s: enable: %ld osdshow: %ld mode: %d count: %ld\n",
        cam_id, comm_file, enable, osdshow, cur_mode, count);

    memset(cam_name, 0, sizeof(cam_name));
    snprintf(cam_name, sizeof(cam_name), "config_media_cam%d", index);

    for (j = 0; j < g_ParamAccess.module_num; j++) {
        if (strstr(g_ParamAccess.modules[j].name, cam_name)) {
            memset(tmp_name, 0, sizeof(tmp_name));
            snprintf(tmp_name, sizeof(tmp_name), "%s%s",
                g_ParamAccess.modules[j].path, g_ParamAccess.modules[j].name);
            printf("cam_name = %s \n",tmp_name);
            s32Ret = PARAM_LoadMediaSpec(tmp_name, &CamCfg->MediaSpec[i++], index);
        }
    }

    return s32Ret;
}


int32_t  INI_PARAM_LoadMediaCamCfg(PARAM_CAM_CFG *CamCfg)
{
    printf("\n---enter: %s\n", __func__);

   int32_t  s32Ret = 0;
    uint32_t i = 0;
    uint32_t j = 0;
    char comm_name[PARAM_MODULE_NAME_LEN] = {0};
    char comm_file[PARAM_MODULE_NAME_LEN] = {0};

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        memset(comm_name, 0, sizeof(comm_name));
        snprintf(comm_name, sizeof(comm_name), "config_mediamode_cam%d", i);

        for (j = 0; j < g_ParamAccess.module_num; j++) {
            if (strstr(g_ParamAccess.modules[j].name, comm_name)) {
                memset(comm_file, 0, sizeof(comm_file));
                snprintf(comm_file, sizeof(comm_file), "%s%s",
                    g_ParamAccess.modules[j].path, g_ParamAccess.modules[j].name);
                break;
            }
        }

        s32Ret = PARAM_LoadMediaCam(comm_file, &CamCfg[i], i);
    }

    return s32Ret;
}
/* special config end */

static void print_param_vi_vpss_mode(const VI_VPSS_MODE_S *VpssMode)
{
    if(VpssMode == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("stVIVPSSMode:\n");
    for(uint32_t i = 0; i < VI_MAX_PIPE_NUM; i++) {
        printf("VpssDevId: %u\n", VpssMode->aenMode[i]);
    }
}
static void print_param_vpss_mode(const VPSS_MODE_S *VpssMode)
{
    if(VpssMode == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("stVPSSMode:\n");
    printf("enMode: %u\n", VpssMode->enMode);
    for(uint32_t i = 0; i < VPSS_DEVICE_NUM; i++) {
        printf("aenInput[%u]: %u\n", i, VpssMode->aenInput[i]);
    }
}
static void print_param_vpss(const PARAM_VPSS_ATTR_S *Vpss)
{
    if(Vpss == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Vpss:\n");
    printf("stVIVPSSMode:\n");
    print_param_vi_vpss_mode(&Vpss->stVIVPSSMode);
    printf("stVPSSMode:\n");
    print_param_vpss_mode(&Vpss->stVPSSMode);
}

static void print_param_vo(const PARAM_DISP_ATTR_S *Vo)
{
    if(Vo == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Vo:\n");
    printf("Width: %u\n", Vo->Width);
    printf("Height: %u\n", Vo->Height);
    printf("Rotate: %u\n", Vo->Rotate);
    printf("Fps: %d\n", Vo->Fps);
    printf("EnIntfSync: %u\n", Vo->EnIntfSync);
    printf("frame_fmt: %u\n", Vo->frame_fmt);
}

static void print_param_window(const PARAM_WND_ATTR_S *Window)
{
    if(Window == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Window:\n");
    printf("WndCnt: %u\n", Window->WndCnt);
}

static void print_param_ai(const MAPI_ACAP_ATTR_S *Ai)
{
    if(Ai == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Ai:\n");
    printf("enSampleRate: %u\n", Ai->enSampleRate);
    printf("channel: %u\n", Ai->channel);
    printf("u32PtNumPerFrm: %u\n", Ai->u32PtNumPerFrm);
    printf("bVqeOn: %d\n", Ai->bVqeOn);
    printf("AudioChannel: %u\n", Ai->AudioChannel);
    printf("volume: %d\n", Ai->volume);
}

static void print_param_aenc(const MAPI_AENC_ATTR_S *Aenc)
{
    if(Aenc == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Aenc:\n");
    printf("enAencFormat: %u\n", Aenc->enAencFormat);
    printf("u32PtNumPerFrm: %u\n", Aenc->u32PtNumPerFrm);
    printf("src_samplerate: %u\n", Aenc->src_samplerate);
    printf("channels: %u\n", Aenc->channels);
}

static void print_param_ao(const MAPI_AO_ATTR_S *Ao)
{
    if(Ao == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Ao:\n");
    printf("enSampleRate: %u\n", Ao->enSampleRate);
    printf("channels: %u\n", Ao->channels);
    printf("u32PtNumPerFrm: %u\n", Ao->u32PtNumPerFrm);
    printf("u32PowerPinId: %u\n", Ao->u32PowerPinId);
    printf("AudioChannel: %u\n", Ao->AudioChannel);
    printf("volume: %d\n", Ao->volume);
}

static void print_param_record(const PARAM_RECORD_ATTR_S *Record)
{
    if(Record == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Record:\n");
    printf("ChnCnt: %u\n", Record->ChnCnt);
    for(uint32_t i = 0; i < Record->ChnCnt; i++) {
        printf("ChnAttrs[%u]:\n", i);
        printf("Enable: %s\n", bool_to_str(Record->ChnAttrs[i].Enable));
        printf("Subvideoen: %s\n", bool_to_str(Record->ChnAttrs[i].Subvideoen));
        printf("AudioStatus: %s\n", bool_to_str(Record->ChnAttrs[i].AudioStatus));
        printf("SubBindVencId: %u\n", Record->ChnAttrs[i].SubBindVencId);
        printf("BindVencId: %u\n", Record->ChnAttrs[i].BindVencId);
        printf("FileType: %u\n", Record->ChnAttrs[i].FileType);
        printf("SplitTime: %llu\n", Record->ChnAttrs[i].SplitTime);
        printf("PreTime: %u\n", Record->ChnAttrs[i].PreTime);
        printf("PostTime: %u\n", Record->ChnAttrs[i].PostTime);
        printf("TimelapseFps: %f\n", Record->ChnAttrs[i].TimelapseFps);
        printf("TimelapseGop: %u\n", Record->ChnAttrs[i].TimelapseGop);
        printf("MemoryBufferSec: %u\n", Record->ChnAttrs[i].MemoryBufferSec);
        printf("PreallocUnit: %u\n", Record->ChnAttrs[i].PreallocUnit);
        printf("NormalExtendVideoBufferSec: %f\n", Record->ChnAttrs[i].NormalExtendVideoBufferSec);
        printf("EventExtendVideoBufferSec: %f\n", Record->ChnAttrs[i].EventExtendVideoBufferSec);
        printf("ExtendOtherBufferSec: %f\n", Record->ChnAttrs[i].ExtendOtherBufferSec);
        printf("ShortFileMs: %f\n", Record->ChnAttrs[i].ShortFileMs);
        printf("FocusPos: %u\n", Record->ChnAttrs[i].FocusPos);
        printf("FocusPosLock: %u\n", Record->ChnAttrs[i].FocusPosLock);
        printf("devmodel: %s\n", Record->ChnAttrs[i].devmodel);
    }
}
static void print_param_rtsp(const PARAM_RTSP_ATTR_S *Rtsp)
{
    printf("Rtsp:\n");
    printf("ChnCnt: %u\n", (uint32_t)MAX_RTSP_CNT);
    for(uint32_t i = 0; i < MAX_RTSP_CNT; i++) {

        printf("ChnAttrs[%u]:\n", i);
        printf("Enable: %s\n", bool_to_str(Rtsp->ChnAttrs[i].Enable));
        printf("BindVencId: %u\n", Rtsp->ChnAttrs[i].BindVencId);
        printf("MaxConn: %d\n", Rtsp->ChnAttrs[i].MaxConn);
        printf("Timeout: %d\n", Rtsp->ChnAttrs[i].Timeout);
        printf("Port: %d\n", Rtsp->ChnAttrs[i].Port);
        printf("Name: %s\n", Rtsp->ChnAttrs[i].Name);
    }
}

static void print_param_piv(const PARAM_PIV_ATTR_S *Piv)
{
    if(Piv == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Piv:\n");
    printf("ChnCnt: %u\n", (uint32_t)MAX_CAMERA_INSTANCES);
    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        printf("ChnAttrs[%u]:\n", i);
        printf("BindVencId: %u\n", Piv->ChnAttrs[i].BindVencId);
    }
}

static void print_param_thumbnail(const PARAM_THUMBNAIL_ATTR_S *Thumbnail)
{
    if(Thumbnail == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Thumbnail:\n");
    printf("ChnCnt: %u\n", (uint32_t)MAX_CAMERA_INSTANCES);
    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        printf("ChnAttrs[%u]:\n", i);
        printf("BindVencId: %u\n", Thumbnail->ChnAttrs[i].BindVencId);
    }
}
static void print_param_sub_pic(const PARAM_SUB_PIC_ATTR_S *SubPic)
{
    if(SubPic == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("SubPic:\n");
    printf("ChnCnt: %u\n", (uint32_t)MAX_CAMERA_INSTANCES);
    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        printf("ChnAttrs[%u]:\n", i);
        printf("BindVencId: %u\n", SubPic->ChnAttrs[i].BindVencId);
    }
}

static void print_param_photo(const PARAM_PHOTO_ATTR_S *Photo)
{
    if(Photo == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Photo:\n");
    printf("photoid: %u\n", Photo->photoid);
    printf("VprocDev_id: %u\n", Photo->VprocDev_id);
    printf("ChnCnt: %u\n", (uint32_t)MAX_CAMERA_INSTANCES);
    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        printf("ChnAttrs[%u]:\n", i);
        printf("Enable: %s\n", bool_to_str(Photo->ChnAttrs[i].Enable));
        printf("BindVencId: %u\n", Photo->ChnAttrs[i].BindVencId);
        printf("BindVcapId: %u\n", Photo->ChnAttrs[i].BindVcapId);
        printf("EnableDumpRaw: %u\n", Photo->ChnAttrs[i].EnableDumpRaw);
    }
}

static void print_param_md(const PARAM_MD_ATTR_S *Md)
{
    if(Md == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Md:\n");
    printf("motionSensitivity: %u\n", Md->motionSensitivity);
    printf("ChnCnt: %u\n", (uint32_t)MAX_CAMERA_INSTANCES);
    for(uint32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        printf("ChnAttrs[%u]:\n", i);
        printf("Enable: %s\n", bool_to_str(Md->ChnAttrs[i].Enable));
        printf("BindVprocId: %u\n", Md->ChnAttrs[i].BindVprocId);
        printf("BindVprocChnId: %u\n", Md->ChnAttrs[i].BindVprocChnId);
    }
}

static void print_param_facep(const PARAM_FACEP_ATTR_S *Facep)
{
    if(Facep == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("Facep:\n");
    printf("facep_cnt: %d\n", Facep->facep_cnt);
}

static void printf_media_sns_attr(const PARAM_MEDIA_SNS_ATTR_S *SnsAttr)
{
    printf("SnsEnable: %s\n", bool_to_str(SnsAttr->SnsEnable));
    printf("SnsChnAttr:\n");
    printf("u8SnsId: %d\n", SnsAttr->SnsChnAttr.u8SnsId);
    printf("u8WdrMode: %d\n", SnsAttr->SnsChnAttr.u8WdrMode);
    printf("u8I2cBusId: %d\n", SnsAttr->SnsChnAttr.u8I2cBusId);
    printf("u8I2cSlaveAddr: %d\n", SnsAttr->SnsChnAttr.u8I2cSlaveAddr);
    printf("u8HwSync: %d\n", SnsAttr->SnsChnAttr.u8HwSync);
    printf("u8MipiDev: %d\n", SnsAttr->SnsChnAttr.u8MipiDev);
    printf("u8CamClkId: %d\n", SnsAttr->SnsChnAttr.u8CamClkId);
    printf("u8RstGpioInx: %d\n", SnsAttr->SnsChnAttr.u8RstGpioInx);
    printf("u8RstGpioPin: %d\n", SnsAttr->SnsChnAttr.u8RstGpioPin);
    printf("u8RstGpioPol: %d\n", SnsAttr->SnsChnAttr.u8RstGpioPol);
    printf("bMclkEn: %s\n", bool_to_str(SnsAttr->SnsChnAttr.bMclkEn));
    printf("bHsettlen: %s\n", bool_to_str(SnsAttr->SnsChnAttr.bHsettlen));
    printf("u8Hsettle: %d\n", SnsAttr->SnsChnAttr.u8Hsettle);
    printf("u8Orien: %d\n", SnsAttr->SnsChnAttr.u8Orien);
}

static void printf_media_vcap_chn_attr(const MAPI_VCAP_CHN_ATTR_T *VcapChnAttr)
{
    printf("u32Width: %d\n", VcapChnAttr->u32Width);
    printf("u32Height: %d\n", VcapChnAttr->u32Height);
    printf("enPixelFmt: %d\n", VcapChnAttr->enPixelFmt);
    printf("enCompressMode: %d\n", VcapChnAttr->enCompressMode);
    printf("bMirror: %s\n", bool_to_str(VcapChnAttr->bMirror));
    printf("bFlip: %s\n", bool_to_str(VcapChnAttr->bFlip));
    printf("f32Fps: %f\n", VcapChnAttr->f32Fps);
    printf("vbcnt: %d\n", VcapChnAttr->vbcnt);
#ifdef __CV184X__
    printf("scenemode: %d\n", VcapChnAttr->scenemode);
#endif
}

static void printf_media_vcap_attr(const PARAM_MEDIA_VACP_ATTR_S *VcapAttr)
{
    printf("VcapEnable: %s\n", bool_to_str(VcapAttr->VcapEnable));
    printf("VcapId: %d\n", VcapAttr->VcapId);
    printf("VcapChnAttr:\n");
    printf_media_vcap_chn_attr(&VcapAttr->VcapChnAttr);
    printf("VcapSwitchChnAttr:\n");
    printf_media_vcap_chn_attr(&VcapAttr->VcapSwitchChnAttr);
}

static void printf_media_ext_vproc_chn_attr(const MEDIA_EXT_VPROC_CHN_ATTR_S *ExtChnAttr)
{
    printf("VprocChnid: %d\n", ExtChnAttr->ChnAttr.BindVprocChnId);
}

static void printf_media_vproc_chn_attr(const MEDIA_VPROC_CHN_ATTR_S *VprocChnAttr)
{
    printf("VprocChnid: %d\n", VprocChnAttr->VprocChnid);
    printf("VprocChnEnable: %s\n", bool_to_str(VprocChnAttr->VprocChnEnable));
    printf("VprocChnVbCnt: %d\n", VprocChnAttr->VprocChnVbCnt);
    printf("VprocChnLowDelayCnt: %d\n", VprocChnAttr->VprocChnLowDelayCnt);
    printf("enRotation: %d\n", VprocChnAttr->enRotation);

    printf("VprocChnAttr:\n");
    printf("u32Width: %d\n", VprocChnAttr->VpssChnAttr.u32Width);
    printf("u32Height: %d\n", VprocChnAttr->VpssChnAttr.u32Height);
    printf("enVideoFormat: %d\n", VprocChnAttr->VpssChnAttr.enVideoFormat);
    printf("enPixelFormat: %d\n", VprocChnAttr->VpssChnAttr.enPixelFormat);
    printf("bMirror: %s\n", bool_to_str(VprocChnAttr->VpssChnAttr.bMirror));
    printf("bFlip: %s\n", bool_to_str(VprocChnAttr->VpssChnAttr.bFlip));
    printf("u32Depth: %d\n", VprocChnAttr->VpssChnAttr.u32Depth);
    printf("stAspectRatio->enMode: %d\n", VprocChnAttr->VpssChnAttr.stAspectRatio.enMode);
    printf("stAspectRatio->bEnableBgColor: %s\n", bool_to_str(VprocChnAttr->VpssChnAttr.stAspectRatio.bEnableBgColor));
    printf("stAspectRatio->u32BgColor: %d\n", VprocChnAttr->VpssChnAttr.stAspectRatio.u32BgColor);
    printf("stVideoRect->u32Width: %d\n", VprocChnAttr->VpssChnAttr.stAspectRatio.stVideoRect.u32Width);
    printf("stVideoRect->u32Height: %d\n", VprocChnAttr->VpssChnAttr.stAspectRatio.stVideoRect.u32Height);
    printf("stVideoRect->u32X: %d\n", VprocChnAttr->VpssChnAttr.stAspectRatio.stVideoRect.s32X);
    printf("stVideoRect->u32Y: %d\n", VprocChnAttr->VpssChnAttr.stAspectRatio.stVideoRect.s32Y);

    printf("VpssBufWrap:\n");
    printf("VpssBufWrap->bEnable: %s\n", bool_to_str(VprocChnAttr->VpssBufWrap.bEnable));
    printf("VpssBufWrap->u32BufLine: %d\n", VprocChnAttr->VpssBufWrap.u32BufLine);
    printf("VpssBufWrap->u32WrapBufferSize: %d\n", VprocChnAttr->VpssBufWrap.u32WrapBufferSize);

    printf("VpssChnCropInfo:\n");
    printf("bEnable: %s\n", bool_to_str(VprocChnAttr->VpssChnCropInfo.bEnable));
    printf("enCropCoordinate: %d\n", VprocChnAttr->VpssChnCropInfo.enCropCoordinate);
    printf("stCropRect->u32Width: %d\n", VprocChnAttr->VpssChnCropInfo.stCropRect.u32Width);
    printf("stCropRect->u32Height: %d\n", VprocChnAttr->VpssChnCropInfo.stCropRect.u32Height);
    printf("stCropRect->u32X: %d\n", VprocChnAttr->VpssChnCropInfo.stCropRect.s32X);
    printf("stCropRect->u32Y: %d\n", VprocChnAttr->VpssChnCropInfo.stCropRect.s32Y);
}

static void printf_media_vproc_grp_attr(const PARAM_MEDIA_VPROC_GRP_ATTR_S *VprocGrpAttr)
{
    printf("VprocEnable: %s\n", bool_to_str(VprocGrpAttr->VprocEnable));
    printf("BindEnable: %s\n", bool_to_str(VprocGrpAttr->BindEnable));
    printf("Vprocid: %d\n", VprocGrpAttr->Vprocid);
    printf("VpssDev: %d\n", VprocGrpAttr->VpssDev);
    printf("ChnCnt: %d\n", VprocGrpAttr->ChnCnt);
    printf("stSrcChn:\n");
    printf("enModId: %d\n", VprocGrpAttr->stSrcChn.enModId);
    printf("s32DevId: %d\n", VprocGrpAttr->stSrcChn.s32DevId);
    printf("s32ChnId: %d\n", VprocGrpAttr->stSrcChn.s32ChnId);
    printf("stDestChn:\n");
    printf("enModId: %d\n", VprocGrpAttr->stDestChn.enModId);
    printf("s32DevId: %d\n", VprocGrpAttr->stDestChn.s32DevId);
    printf("s32ChnId: %d\n", VprocGrpAttr->stDestChn.s32ChnId);
    printf("VpssGrpAttr:\n");
    printf("u32MaxW: %d\n", VprocGrpAttr->VpssGrpAttr.u32MaxW);
    printf("u32MaxH: %d\n", VprocGrpAttr->VpssGrpAttr.u32MaxH);
    printf("enPixelFormat: %u\n", VprocGrpAttr->VpssGrpAttr.enPixelFormat);
    printf("stFrameRate:\n");
    printf("s32SrcFrameRate: %d\n", VprocGrpAttr->VpssGrpAttr.stFrameRate.s32SrcFrameRate);
    printf("s32DstFrameRate: %d\n", VprocGrpAttr->VpssGrpAttr.stFrameRate.s32DstFrameRate);
    printf("u8VpssDev: %d\n", VprocGrpAttr->VpssGrpAttr.u8VpssDev);
    printf("VpssCropInfo:\n");
    printf("bEnable: %s\n", bool_to_str(VprocGrpAttr->VpssCropInfo.bEnable));
    printf("enCropCoordinate: %d\n", VprocGrpAttr->VpssCropInfo.enCropCoordinate);
    printf("stCropRect:\n");
    printf("u32Width: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.u32Width);
    printf("u32Height: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.u32Height);
    printf("u32X: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.s32X);
    printf("u32Y: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.s32Y);
    printf("VprocChnAttr:\n");
    for(uint32_t index = 0; index < VprocGrpAttr->ChnCnt; index++) {
        printf("VprocChnAttr[%d]:\n", index);
        printf_media_vproc_chn_attr(&VprocGrpAttr->VprocChnAttr[index]);
    }
}

static void printf_media_vproc_attr(const PARAM_MEDIA_VPROC_ATTR_S *VprocAttr)
{
    printf("      VprocCnt: %d\n", VprocAttr->VprocCnt);
    for(uint32_t index = 0; index < VprocAttr->VprocCnt; index++) {
        printf("      VprocGrpAttr[%d]:\n", index);
        const PARAM_MEDIA_VPROC_GRP_ATTR_S *VprocGrpAttr = &VprocAttr->VprocGrpAttr[index];
        printf_media_vproc_grp_attr(VprocGrpAttr);
        printf("VpssCropInfo:\n");
        printf("u32CropW: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.u32Width);
        printf("u32CropH: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.u32Height);
        printf("u32CropX: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.s32X);
        printf("u32CropY: %d\n", VprocGrpAttr->VpssCropInfo.stCropRect.s32Y);
    }
}

static void printf_media_venc_chn_attr(const MEDIA_VENC_CHN_ATTR_S *VencChnAttr)
{
    printf("VencChnEnable: %s\n", bool_to_str(VencChnAttr->VencChnEnable));
    printf("VencId: %d\n", VencChnAttr->VencId);
    printf("sbm_enable: %d\n", VencChnAttr->sbm_enable);
    printf("BindVprocId: %d\n", VencChnAttr->BindVprocId);
    printf("BindVprocChnId: %d\n", VencChnAttr->BindVprocChnId);
    printf("framerate: %f\n", VencChnAttr->framerate);
    printf("bindMode: %d\n", VencChnAttr->bindMode);
    printf("MapiVencAttr:\n");
    printf("codec: %d\n", VencChnAttr->MapiVencAttr.codec);
    printf("width: %d\n", VencChnAttr->MapiVencAttr.width);
    printf("height: %d\n", VencChnAttr->MapiVencAttr.height);
    printf("pixel_format: %d\n", VencChnAttr->MapiVencAttr.pixel_format);
    printf("gop: %d\n", VencChnAttr->MapiVencAttr.gop);
    printf("profile: %d\n", VencChnAttr->MapiVencAttr.profile);
    printf("rate_ctrl_mode: %d\n", VencChnAttr->MapiVencAttr.rate_ctrl_mode);
    printf("bitrate_kbps: %d\n", VencChnAttr->MapiVencAttr.bitrate_kbps);
    printf("iqp: %d\n", VencChnAttr->MapiVencAttr.iqp);
    printf("pqp: %d\n", VencChnAttr->MapiVencAttr.pqp);
    printf("minIqp: %d\n", VencChnAttr->MapiVencAttr.minIqp);
    printf("maxIqp: %d\n", VencChnAttr->MapiVencAttr.maxIqp);
    printf("minQp: %d\n", VencChnAttr->MapiVencAttr.minQp);
    printf("maxQp: %d\n", VencChnAttr->MapiVencAttr.maxQp);
    printf("changePos: %d\n", VencChnAttr->MapiVencAttr.changePos);
    printf("jpeg_quality: %d\n", VencChnAttr->MapiVencAttr.jpeg_quality);
    printf("single_EsBuf: %d\n", VencChnAttr->MapiVencAttr.single_EsBuf);
    printf("bufSize: %d\n", VencChnAttr->MapiVencAttr.bufSize);
    printf("datafifoLen: %d\n", VencChnAttr->MapiVencAttr.datafifoLen);
    printf("src_framerate: %d\n", VencChnAttr->MapiVencAttr.src_framerate);
    printf("dst_framerate: %d\n", VencChnAttr->MapiVencAttr.dst_framerate);
    printf("initialDelay: %d\n", VencChnAttr->MapiVencAttr.initialDelay);
    printf("thrdLv: %d\n", VencChnAttr->MapiVencAttr.thrdLv);
    printf("statTime: %d\n", VencChnAttr->MapiVencAttr.statTime);
    printf("firstFrameStartQp: %d\n", VencChnAttr->MapiVencAttr.firstFrameStartQp);
    printf("maxBitRate: %d\n", VencChnAttr->MapiVencAttr.maxBitRate);
    printf("gop_mode: %d\n", VencChnAttr->MapiVencAttr.gop_mode);
    printf("maxIprop: %d\n", VencChnAttr->MapiVencAttr.maxIprop);
    printf("minIprop: %d\n", VencChnAttr->MapiVencAttr.minIprop);
    printf("minStillPercent: %d\n", VencChnAttr->MapiVencAttr.minStillPercent);
    printf("maxStillQP: %d\n", VencChnAttr->MapiVencAttr.maxStillQP);
    printf("avbrPureStillThr: %d\n", VencChnAttr->MapiVencAttr.avbrPureStillThr);
    printf("motionSensitivity: %d\n", VencChnAttr->MapiVencAttr.motionSensitivity);
    printf("bgDeltaQp: %d\n", VencChnAttr->MapiVencAttr.bgDeltaQp);
    printf("rowQpDelta: %d\n", VencChnAttr->MapiVencAttr.rowQpDelta);
}

static void printf_media_venc_attr(const PARAM_MEDIA_VENC_ATTR_S *VencAttr)
{
    for(uint32_t index = 0; index < MAX_VENC_CNT; index++) {
        printf("VencChnAttr[%d]:\n", index);
        printf_media_venc_chn_attr(&VencAttr->VencChnAttr[index]);
    }
}

static void printf_media_vb_chn_attr(const MAPI_MEDIA_SYS_VB_POOL_T *Vbpool)
{
    printf("vb_blk_size: %d\n", Vbpool->vb_blk_size.size);
    if(Vbpool->is_frame) {
        printf("width: %d\n", Vbpool->vb_blk_size.frame.width);
        printf("height: %d\n", Vbpool->vb_blk_size.frame.height);
        printf("fmt: %d\n", Vbpool->vb_blk_size.frame.fmt);
    }
    printf("vb_blk_num: %d\n", Vbpool->vb_blk_num);
}

static void printf_media_vb_attr(const PARAM_MEDIA_VB_ATTR_S *VbAttr)
{
    printf("Poolcnt: %d\n", VbAttr->Poolcnt);
    for(uint32_t index = 0; index < VbAttr->Poolcnt; index++) {
        printf("Vbpool[%d]:\n", index);
        printf_media_vb_chn_attr(&VbAttr->Vbpool[index]);
    }
}

static void printf_media_osd_disp_attr(const MAPI_OSD_DISP_ATTR_S *OsdDispAttr)
{
    printf("bShow: %s\n", bool_to_str(OsdDispAttr->bShow));
    printf("enBindedMod: %d\n", OsdDispAttr->enBindedMod);
    printf("ModHdl: %d\n", OsdDispAttr->ModHdl);
    printf("ChnHdl: %d\n", OsdDispAttr->ChnHdl);
    printf("enCoordinate: %d\n", OsdDispAttr->enCoordinate);
    printf("stStartPos: (%d, %d)\n", OsdDispAttr->stStartPos.s32X, OsdDispAttr->stStartPos.s32Y);
    printf("u32Batch: %d\n", OsdDispAttr->u32Batch);
    printf("enRgnCmprType: %d\n", OsdDispAttr->enRgnCmprType);
    printf("stFontSize: (%d, %d)\n", OsdDispAttr->stTimeContent.stFontSize.u32Width,
           OsdDispAttr->stTimeContent.stFontSize.u32Height);
    printf("u32BgColor: 0x%x\n", OsdDispAttr->stTimeContent.u32BgColor);
    printf("szStr: %s\n", OsdDispAttr->stStrContent.szStr);
    printf("stFontSize: (%d, %d)\n", OsdDispAttr->stStrContent.stFontSize.u32Width,
           OsdDispAttr->stStrContent.stFontSize.u32Height);
    printf("u32BgColor: 0x%x\n", OsdDispAttr->stStrContent.u32BgColor);
    printf("u32Width: %d\n", OsdDispAttr->stCircleContent.u32Width);
    printf("u32Height: %d\n", OsdDispAttr->stCircleContent.u32Height);
    printf("enPixelFormat: %d\n", OsdDispAttr->stBitmapContent.enPixelFormat);
    printf("u32Width: %d\n", OsdDispAttr->stBitmapContent.u32Width);
    printf("u32Height: %d\n", OsdDispAttr->stBitmapContent.u32Height);
    printf("pData: %p\n", OsdDispAttr->stBitmapContent.pData);
    printf("maxlen: %d\n", OsdDispAttr->maxlen);
    printf("u64BitmapPhyAddr: 0x%llx\n", OsdDispAttr->u64BitmapPhyAddr);
    printf("pBitmapVirAddr: %p\n", OsdDispAttr->pBitmapVirAddr);
}

static void printf_media_osd_content(const MAPI_OSD_CONTENT_S *stContent)
{
    printf("enOsdType: %d\n", stContent->enType);
    printf("u32FontColor: 0x%x\n", stContent->u32Color);
}

static void printf_media_osd_chn_attr(const MAPI_OSD_ATTR_S *OsdAttr)
{
    printf("u32DispNum: %d\n", OsdAttr->u32DispNum);
    printf("bFlip: %s\n", bool_to_str(OsdAttr->bFlip));
    printf("bMirror: %s\n", bool_to_str(OsdAttr->bMirror));
    for(uint32_t index = 0; index < OsdAttr->u32DispNum; index++) {
        printf("astDispAttr[%d]:\n", index);
        printf_media_osd_disp_attr(&OsdAttr->astDispAttr[index]);
    }
    printf("stContent:\n");
    printf_media_osd_content(&OsdAttr->stContent);
}

static void printf_media_osd_attr(const PARAM_MEDIA_OSD_ATTR_S *OsdAttr)
{
    printf("bOsdc: %s\n", bool_to_str(OsdAttr->bOsdc));
    printf("OSDCnt: %d\n", OsdAttr->OsdCnt);
    for(uint32_t index = 0; index < (uint32_t)OsdAttr->OsdCnt; index++) {
        printf("OsdAttrs[%d]:\n", index);
        printf_media_osd_chn_attr(&OsdAttr->OsdAttrs[index]);
    }
}

static void printf_MediaSpec(const PARAM_MEDIA_SPEC_S *MediaSpec)
{
    printf("MediaSpec:\n");
    printf("MediaMode: %d\n", MediaSpec->MediaMode);
    printf_media_sns_attr(&MediaSpec->SnsAttr);
    printf("VcapAttr:\n");
    printf_media_vcap_attr(&MediaSpec->VcapAttr);
    printf("VprocAttr:\n");
    printf_media_vproc_attr(&MediaSpec->VprocAttr);
    printf("VencAttr:\n");
    printf_media_venc_attr(&MediaSpec->VencAttr);
    printf("VbAttr:\n");
    printf_media_vb_attr(&MediaSpec->Vb);
    printf("OsdAttr:\n");
    printf_media_osd_attr(&MediaSpec->Osd);
}

void print_param_cam_cfg(const PARAM_CAM_CFG *cam)
{
    printf("CamCfg:\n");
    printf("CamEnable: %s\n", bool_to_str(cam->CamEnable));
    printf("CamIspEnable: %s\n", bool_to_str(cam->CamIspEnable));
    printf("CamID: %d\n", cam->CamMediaInfo.CamID);
    printf("MediaModeCnt: %d\n", cam->MediaModeCnt);
    for(uint32_t index = 0; index < cam->MediaModeCnt; index++) {
        printf_MediaSpec(&cam->MediaSpec[index]);
    }
}

void print_param_media_comm(const PARAM_MEDIA_COMM_S *MediaComm)
{
    if(MediaComm == NULL) {
        printf("[MEDIA] Error: Null pointer received.\n");
        return;
    }
    printf("[Media Configuration (PARAM_MEDIA_COMM_S)]\n");
    printf("PowerOnMode: %u\n", MediaComm->PowerOnMode);
    printf("Vpss:\n");
    print_param_vpss(&MediaComm->Vpss);
    printf("Vo:\n");
    print_param_vo(&MediaComm->Vo);
    printf("Window:\n");
    print_param_window(&MediaComm->Window);
    printf("Ai:\n");
    print_param_ai(&MediaComm->Ai);
    printf("Aenc:\n");
    print_param_aenc(&MediaComm->Aenc);
    printf("Ao:\n");
    print_param_ao(&MediaComm->Ao);
    printf("Record:\n");
    print_param_record(&MediaComm->Record);
    printf("Rtsp:\n");
    print_param_rtsp(&MediaComm->Rtsp);
    printf("Piv:\n");
    print_param_piv(&MediaComm->Piv);
    printf("Thumbnail:\n");
    print_param_thumbnail(&MediaComm->Thumbnail);
    printf("SubPic:\n");
    print_param_sub_pic(&MediaComm->SubPic);
    printf("Photo:\n");
    print_param_photo(&MediaComm->Photo);
    printf("Md:\n");
    print_param_md(&MediaComm->Md);
#ifdef SERVICES_SPEECH_ON
    printf("Speech:\n");
    print_param_speech(&MediaComm->Speech);
#endif
#ifdef SERVICES_ADAS_ON
    printf("ADAS:\n");
    print_param_adas(&MediaComm->ADAS);
#endif
#ifdef SERVICES_QRCODE_ON
    printf("QRCODE:\n");
    print_param_qrcode(&MediaComm->QRCODE);
#endif
    printf("Facep:\n");
    print_param_facep(&MediaComm->Facep);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

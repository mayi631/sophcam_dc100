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
static char* PARAM_GetVideoModeName(int32_t  vidoemode)
{
    switch (vidoemode)
    {
        case UVC_STREAM_FORMAT_YUV420:
            return "yuv420";
        case UVC_STREAM_FORMAT_MJPEG:
            return "mjpeg";
        case UVC_STREAM_FORMAT_H264:
            return "h264";
        default:
            return "mjpeg";
    }
}

static int32_t PARAM_LoadUsbMode(const char *filepath, PARAM_USB_MODE_S *UsbModeCfg)
{
    long int usb_mode = 0;
    long int count = 0;
    long int defmode = 0;
    long int videomode = 0;
    long int bitrate = 0;
    long int vcaphdl = 0;
    long int vprochdl = 0;
    long int venchdl = 0;
    long int acaphdl = 0;
    long int bufcnt = 0;
    long int bufsize = 0;
    char tmp_section[128] = {0};
    long int i = 0, j = 0;

    usb_mode = INI_GetLong("common", "usb_mode", 0, filepath);
    UsbModeCfg->UsbWorkMode = usb_mode;
    INI_GetString("uvc", "dev_path", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->UvcParam.UvcCfg.szDevPath, sizeof(tmp_section), "%s", tmp_section);
    for (i = 0; i < UVC_STREAM_FORMAT_BUTT; i++) {
        count = INI_GetLong(PARAM_GetVideoModeName(i), "count", 0, filepath);
        defmode = INI_GetLong(PARAM_GetVideoModeName(i), "video_defmode", 0, filepath);
        UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].u32Cnt = count;
        UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].enDefMode = defmode;
        for (j = 0; j < count && j < UVC_VIDEOMODE_BUTT; j++) {
            videomode = INI_GetLong(PARAM_GetVideoModeName(i), "video_mode0", 0, filepath);
            bitrate = INI_GetLong(PARAM_GetVideoModeName(i), "video_bitrate0", 0, filepath);
            UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].astModes[i].u32BitRate = bitrate;
            UsbModeCfg->UvcParam.UvcCfg.stDevCap.astFmtCaps[i].astModes[i].enMode = videomode;
        }
    }
    vcaphdl = INI_GetLong("datasource", "vcap_id", 0, filepath);
    vprochdl = INI_GetLong("datasource", "vporc_id", 0, filepath);
    venchdl = INI_GetLong("datasource", "vporc_chn_id", 0, filepath);
    acaphdl = INI_GetLong("datasource", "acap_id", 0, filepath);
    UsbModeCfg->UvcParam.VcapId = vcaphdl;
    UsbModeCfg->UvcParam.VprocId = vprochdl;
    UsbModeCfg->UvcParam.VprocChnId = venchdl;
    UsbModeCfg->UvcParam.AcapId = acaphdl;
    bufsize = INI_GetLong("buffer", "buffer_size", 0, filepath);
    bufcnt = INI_GetLong("buffer", "buffer_count", 0, filepath);
    UsbModeCfg->UvcParam.UvcCfg.stBufferCfg.u32BufCnt = bufcnt;
    UsbModeCfg->UvcParam.UvcCfg.stBufferCfg.u32BufSize = bufsize;

    memset(tmp_section, 0, sizeof(tmp_section));
    INI_GetString("storage", "dev_path", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->StorageCfg.szDevPath, sizeof(tmp_section), "%s", tmp_section);

    memset(tmp_section, 0, sizeof(tmp_section));
    INI_GetString("storage", "sysfile", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->StorageCfg.szSysFile, sizeof(tmp_section), "%s", tmp_section);

    memset(tmp_section, 0, sizeof(tmp_section));
    INI_GetString("storage", "usb_state_proc", "", tmp_section, sizeof(tmp_section), filepath);
    snprintf(UsbModeCfg->StorageCfg.szProcFile, sizeof(tmp_section), "%s", tmp_section);

    printf("%s------->usb_mode: %ld, szDevPath: %s\n", __func__, usb_mode, UsbModeCfg->UvcParam.UvcCfg.szDevPath);
    printf("%s------->bufcnt: %ld, bufsize: %ld\n", __func__, bufcnt, bufsize);
    printf("%s------->szProcFile: %s\n", __func__, UsbModeCfg->StorageCfg.szProcFile);

    return 0;
}

static int32_t PARAM_LoadVpss(const char *file, PARAM_VPSS_ATTR_S *Vpss)
{
    long int i = 0;
    long int mode = 0;
    long int input = 0;
    long int pipe_mode = 0;
    long int vi_vpss_pipe = 0;
    char tmp_section[32] = {0};

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++){
        snprintf(tmp_section, sizeof(tmp_section), "vi_vpss_mode%ld", i);
        pipe_mode = INI_GetLong(tmp_section, "pipe_mode", 0, file);
        vi_vpss_pipe = INI_GetLong(tmp_section, "vi_vpss_pipe", 0, file);
        Vpss->stVIVPSSMode.aenMode[vi_vpss_pipe] = pipe_mode;
        printf("vivpssmode:vi_vpss_pipe:%ld, pipe_mode:%ld \n", vi_vpss_pipe, pipe_mode);
    }

    mode = INI_GetLong("vpss_mode", "mode", 0, file);
    Vpss->stVPSSMode.enMode = mode;
    printf("vpssmode-------------> %ld \n", mode);
    for(i = 0; i < VPSS_IP_NUM; i++) {
        memset(tmp_section, 0, sizeof(tmp_section));
        snprintf(tmp_section, sizeof(tmp_section), "vpss_mode.dev%ld", i);
        input = INI_GetLong(tmp_section, "input", 0, file);
        Vpss->stVPSSMode.aenInput[i] = input;
        printf("Dev%ld input-------> %ld \n", i, input);
    }

    return 0;
}

static int32_t PARAM_LoadMediaMode(const char *filepath, PARAM_MODE_S *MediaModeCfg)
{
    long int cam_num = 0;
    PARAM_LoadVpss(filepath, &(MediaModeCfg->Vpss));
    cam_num = INI_GetLong("common", "cam_num", 0, filepath);
    MediaModeCfg->CamNum = cam_num;
    for (int32_t  i = 0; i < cam_num; i++) {
        long int camid = 0;
        uint32_t curmediamode = 0;
        char tmp[16] = {0};
        char tmp_section[16] = {0};
        snprintf(tmp, sizeof(tmp), "config%d", i);
        camid = INI_GetLong(tmp, "cam_id", 0, filepath);
        INI_GetString(tmp, "media_mode", "", tmp_section,
                        PARAM_MODULE_NAME_LEN, filepath);
        INI_PARAM_MediaString2Uint(&curmediamode, tmp_section);
        MediaModeCfg->CamMediaInfo[i].CamID = camid;
        MediaModeCfg->CamMediaInfo[i].CurMediaMode = curmediamode;
        printf("%s: cam_num: %ld, camid: %ld, curmediamode: %d\n", __func__, cam_num, camid, curmediamode);
    }

    return 0;
}

static int32_t PARAM_LoadWorkMode(const char *filepath, PARAM_WORK_MODE_S *WorkModeCfg)
{

    char mode_name[PARAM_MODULE_NAME_LEN] = {0};

    INI_GetString("common", "work_mode", "", mode_name, PARAM_MODULE_NAME_LEN, filepath);

    if(strcmp(mode_name, "record") == 0) {
        PARAM_LoadMediaMode(filepath, &WorkModeCfg->RecordMode);
    } else if(strcmp(mode_name, "photo") == 0) {
        PARAM_LoadMediaMode(filepath, &WorkModeCfg->PhotoMode);
    } else if(strcmp(mode_name, "playback") == 0) {

    } else if(strcmp(mode_name, "usbcam") == 0) {

    } else if(strcmp(mode_name, "usb") == 0) {
        PARAM_LoadUsbMode(filepath, &WorkModeCfg->UsbMode);
    } else {
        printf("error mode: %s\n", mode_name);
        return -1;
    }
    printf("%s: work_mode: %s\n", __func__, mode_name);
    return 0;
}

int32_t  INI_PARAM_LoadWorkModeCfg(PARAM_WORK_MODE_S *WorkModeCfg)
{
    printf("\n---enter: %s\n", __func__);
    uint32_t i = 0;
    char filepath[PARAM_MODULE_NAME_LEN];

    for (i = 0; i < g_ParamAccess.module_num; i++) {
        if (strstr(g_ParamAccess.modules[i].name, "config_workmode")) {
            memset(filepath, 0, sizeof(filepath));
            snprintf(filepath, sizeof(filepath), "%s%s",
                g_ParamAccess.modules[i].path, g_ParamAccess.modules[i].name);
            // find a workmode file
            PARAM_LoadWorkMode(filepath, WorkModeCfg);
        }
    }
    // WorkMode->ModeCnt = j;
    return 0;
}

static void print_param_vi_vpss_mode(const VI_VPSS_MODE_S *vi_vpss_mode) {
    if (vi_vpss_mode == NULL) return;

    printf("VI VPSS Mode:\n");
    for (int32_t i = 0; i < VI_MAX_PIPE_NUM; i++) {
        printf("Pipe %d: %s\n", i, vi_vpss_mode->aenMode[i] == VI_OFFLINE_VPSS_OFFLINE ? "Offline" : "Online");
    }
}

static void print_param_vpss_mode(const VPSS_MODE_S *vpss_mode) {
    if (vpss_mode == NULL) return;

    printf("VPSS Mode: %s\n", vpss_mode->enMode == VPSS_MODE_SINGLE ? "Single" : "Dual");
    printf("VPSS Input:\n");
    for (int32_t i = 0; i < VPSS_DEVICE_NUM; i++) {
        printf("Device %d: %s\n", i, vpss_mode->aenInput[i] == VPSS_INPUT_MEM ? "Memory" : "ISP");
    }
}

static void print_param_vpss_attr(const PARAM_VPSS_ATTR_S *vpss_attr) {
    if (vpss_attr == NULL) return;
    printf("VPSS Attribute:\n");
    print_param_vi_vpss_mode(&vpss_attr->stVIVPSSMode);
    print_param_vpss_mode(&vpss_attr->stVPSSMode);
}

static void print_param_mode(const PARAM_MODE_S *mode, const char *mode_name) {
    if (mode == NULL) return;
    printf("[%s Mode Configuration]\n", mode_name);
    printf("Camera Number: %u\n", mode->CamNum);
    print_param_vpss_attr(&mode->Vpss);
    for (uint32_t i = 0; i < mode->CamNum; i++) {
        printf("Camera %d: ID %u, Current Media Mode %u\n", i, mode->CamMediaInfo[i].CamID, mode->CamMediaInfo[i].CurMediaMode);
    }
}

static const char* uvc_stream_format_to_str(UVC_STREAM_FORMAT_E fmt) {
    switch (fmt) {
        case UVC_STREAM_FORMAT_MJPEG: return "MJPEG";
        case UVC_STREAM_FORMAT_BUTT:  return "BUTT";
        case UVC_STREAM_FORMAT_H264:  return "H264";
        default:                      return "Unknown";
    }
}

static void uvc_videomode_get_info(UVC_VIDEOMODE_E mode, uint32_t *width, uint32_t *height, uint32_t *fps) {
    if (width == NULL || height == NULL || fps == NULL) return;
    switch (mode) {
        case UVC_VIDEOMODE_VGA30:
            *width = 640;  *height = 360;  *fps = 30; break;
        case UVC_VIDEOMODE_720P30:
            *width = 1280; *height = 720;  *fps = 30; break;
        case UVC_VIDEOMODE_1080P30:
            *width = 1920; *height = 1080; *fps = 30; break;
        case UVC_VIDEOMODE_4K30:
            *width = 3840; *height = 2160; *fps = 30; break;
        default:
            *width = 0; *height = 0; *fps = 0; break;
    }
}

static const char* uvc_videomode_to_str(UVC_VIDEOMODE_E mode) {
    switch (mode) {
        case UVC_VIDEOMODE_VGA30:  return "VGA30 (640x360@30fps)";
        case UVC_VIDEOMODE_720P30: return "720P30 (1280x720@30fps)";
        case UVC_VIDEOMODE_1080P30:return "1080P30 (1920x1080@30fps)";
        case UVC_VIDEOMODE_4K30:   return "4K30 (3840x2160@30fps)";
        default:                   return "Unknown Mode";
    }
}

static void print_param_uvc_param(const PARAM_UVC_PARAM_S *uvc_param) {
    if (uvc_param == NULL) {
        printf("UVC Parameter is NULL!\n");
        return;
    }
    printf("UVC Parameter:\n");
    printf("Vcap ID: %u\n", uvc_param->VcapId);
    printf("Vproc ID: %u\n", uvc_param->VprocId);
    printf("Vproc Chn ID: %u\n", uvc_param->VprocChnId);
    printf("Acap ID: %u\n", uvc_param->AcapId);
    printf("UVC Device Path: \"%s\"\n", uvc_param->UvcCfg.szDevPath);
    printf("UVC Device Capabilities:\n");

    for (UVC_STREAM_FORMAT_E fmt = 0; fmt < UVC_STREAM_FORMAT_BUTT; fmt++) {
        const UVC_FORMAT_CAP_S *fmt_cap = &uvc_param->UvcCfg.stDevCap.astFmtCaps[fmt];
        if (fmt_cap->u32Cnt == 0 || fmt_cap->u32Cnt > UVC_VIDEOMODE_BUTT) {
            continue;
        }
        printf("Stream Format: %s\n", uvc_stream_format_to_str(fmt));
        printf("Default Video Mode: %s (enum value: %u)\n", uvc_videomode_to_str(fmt_cap->enDefMode),
               fmt_cap->enDefMode);
        printf("Supported Video Modes (%u modes):\n", fmt_cap->u32Cnt);
        for (uint32_t i = 0; i < fmt_cap->u32Cnt; i++) {
            const UVC_VIDEOATTR_S *mode = &fmt_cap->astModes[i];
            uint32_t width, height, fps;
            if (mode->enMode >= UVC_VIDEOMODE_BUTT) {
                printf("Mode %u: Invalid videomode (enum value: %u), Bitrate: %u\n",
                       i, mode->enMode, mode->u32BitRate);
                continue;
            }
            uvc_videomode_get_info(mode->enMode, &width, &height, &fps);
            printf("Mode %u: %s, Bitrate: %u\n", i, uvc_videomode_to_str(mode->enMode), mode->u32BitRate);
            printf("Resolution: %ux%u, FrameRate: %u fps\n", width, height, fps);
        }
    }

    printf("UVC Buffer Configuration:\n");
    printf("Buffer Count: %u\n", uvc_param->UvcCfg.stBufferCfg.u32BufCnt);
    printf("Buffer Size: %u bytes\n", uvc_param->UvcCfg.stBufferCfg.u32BufSize);
}

static void print_usb_storage_cfg(const USB_STORAGE_CFG_S *usb_cfg) {
    if (usb_cfg == NULL) return;
    printf("USB Storage Configuration:\n");
    printf("Proc File: \"%s\"\n", usb_cfg->szProcFile);
    printf("Sys File: \"%s\"\n", usb_cfg->szSysFile);
    printf("Dev Path: \"%s\"\n", usb_cfg->szDevPath);
}

void print_param_workmode(const PARAM_WORK_MODE_S *work_mode) {
    if (work_mode == NULL) return;
    printf("[Work Mode Configuration]\n");
    // 打印记录模式
    print_param_mode(&work_mode->RecordMode, "Record");
    // 打印照片模式
    print_param_mode(&work_mode->PhotoMode, "Photo");
    // 打印USB模式
    printf("USB Work Mode: %u\n", work_mode->UsbMode.UsbWorkMode);
    print_param_uvc_param(&work_mode->UsbMode.UvcParam);
    print_usb_storage_cfg(&work_mode->UsbMode.StorageCfg);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

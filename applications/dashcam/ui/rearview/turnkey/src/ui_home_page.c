#include "ui_common.h"
#include "ui_windowmng.h"
#include "netctrl.h"
#include "netctrlinner.h"

enum _Loop_Time_E{
    MEDIA_VIDEO_LOOP_60000_MSEC = 60000,
    MEDIA_VIDEO_LOOP_180000_MSEC = 180000,
    MEDIA_VIDEO_LOOP_300000_MSEC = 300000,
    MEDIA_VIDEO_LOOP_BUTTOM
} Loop_Time_E;
static uint32_t timer_id = 0;
static uint32_t time_handle = 0;
static uint32_t app_setting_flag = WIFI_APP_SETTING_OUT;

static bool ui_home_cardstatus(void)
{
    uint32_t type = 0x0;

    if (ui_winmng_getwinisshow(UI_HOME_PAGE) == false) {
        CVI_LOGW("ui_home_page no open !\n");
        return false;
    }

    switch(MODEMNG_GetCardState()) {
        case CARD_STATE_REMOVE:
            type = MSG_EVENT_ID_NO_CARD;
            break;
        case CARD_STATE_AVAILABLE:
             if (wrnmsg_window_isopen() == true &&
                ui_wrnmsg_get_type() != (ret_t)MSG_EVENT_APP_CONNECT_SUCCESS) {
                ui_winmng_finishwin(UI_WRNMSG_PAGE);
            }
            return true;
            break;
        case CARD_STATE_ERROR:
            type = MSG_EVENT_ID_SDCARD_ERROR;
            break;
        case CARD_STATE_FSERROR:
        case CARD_STATE_UNAVAILABLE:
            type = MSG_EVENT_ID_SDCARD_NEED_FORMAT;
            break;
        case CARD_STATE_SLOW:
            type = MSG_EVENT_ID_SDCARD_SLOW;
            break;
        case CARD_STATE_CHECKING:
            type = MSG_EVENT_ID_SDCARD_CHECKING;
            break;
        case CARD_STATE_READ_ONLY:
            type = MSG_EVENT_ID_SDCARD_READ_ONLY;
            break;
        case CARD_STATE_MOUNT_FAILED:
            type = MSG_EVENT_ID_SDCARD_MOUNT_FAILED;
            break;
        default:
            CVI_LOGE("value is invalid\n");
            MODEMNG_SetParkingRec(false);
            return false;
            //break;
    }
    MODEMNG_SetParkingRec(false);
    ui_wrnmsg_update_type(type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);

    return false;
}

int32_t  UI_Home_OnReceiveMsgResult(EVENT_S* pstEvent)
{
    // int32_t  s32Ret = 0;
    (void)pstEvent;
    close_busying_page();
    return 0;
}

ret_t on_systime_update(const timer_info_t* timer)
{
    static uint8_t u8status = 0;
    uint32_t u32ModeState = 0;
    widget_t* win = WIDGET(timer->ctx);
    widget_t* video_image = widget_lookup(win, "home_record", TRUE);
    widget_t* sos_image_widget = widget_lookup(win, "sos_image", TRUE);
    MODEMNG_GetModeState(&u32ModeState);

    if (u32ModeState == MEDIA_MOVIE_STATE_VIEW) {
        widget_set_visible(video_image, FALSE, FALSE);
        if (MODEMNG_GetEmrState() == false) {
            image_base_set_image(sos_image_widget, "lock_black");
        }
        return RET_REPEAT;
    } else if (u32ModeState == MEDIA_MOVIE_STATE_REC ||
        u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC) {
        if (MODEMNG_GetEmrState() == false) {
            image_base_set_image(video_image, "home_record_normal");
            image_base_set_image(sos_image_widget, "lock_black");
        } else {
            image_base_set_image(video_image, "home_recsos");
            image_base_set_image(sos_image_widget, "lock_yellow");
        }
    }
    widget_set_visible(video_image, u8status, FALSE);
    u8status = 1 - u8status;

    return RET_REPEAT;
}

static ret_t on_home_page_low_battery(void* ctx, event_t* e)
{
    ui_wrnmsg_update_type(e->type);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);
    return RET_OK;
}

static ret_t on_home_page_move(void* ctx, event_t* e)
{
    pointer_event_t* evt = (pointer_event_t*)e;
    static int32_t  cury = 0, prey = 0;
    if (MODEMNG_GetInProgress() == true) {
        return RET_OK;
    }

    uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
    uint32_t enWind = (curWind >> 16) & 0xFFFF;
    uint32_t enSns = (curWind & 0xFFFF);
    uint32_t n = enSns;
    uint32_t count = 0;
    while(n){n = (n >> 1); count++;};
    if(enWind == enSns && count > 1){
        return RET_OK;
    }

    cury = evt->y;
    for(int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++){
        if(((enWind >> i) & 0x1) == 1){
            if (prey != 0) {
                if(cury - prey > 0) {
                    MODEMNG_LiveViewUp(i);
                } else if(cury - prey < 0) {
                    MODEMNG_LiveViewDown(i);
                }
            }
            break;
        }
    }
    prey = cury;
    return RET_OK;
}

struct timespec t_open = {0};
struct timespec t_start = {0};
struct timespec t_stop = {0};

static ret_t on_startrec_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    MESSAGE_S Msg = {0};
    uint32_t u32ModeState = 0;
    MODEMNG_GetModeState(&u32ModeState);

    if (u32ModeState == MEDIA_MOVIE_STATE_VIEW) {
        if(ui_home_cardstatus() == true) {
            Msg.topic = EVENT_MODEMNG_START_REC;
        }
        clock_gettime(CLOCK_BOOTTIME, &t_start);
    } else {
        clock_gettime(CLOCK_BOOTTIME, &t_stop);
        if (1000 * (t_stop.tv_sec - t_open.tv_sec) + (t_stop.tv_nsec - t_open.tv_nsec) / 1000000  > 800) {
            if (t_start.tv_sec == 0 || (1000 * (t_stop.tv_sec - t_start.tv_sec) + (t_stop.tv_nsec - t_start.tv_nsec) / 1000000  > 800)) {
                Msg.topic = EVENT_MODEMNG_STOP_REC;
            }
            else
                return RET_OK;
        } else {
            return RET_OK;
        }
    }
    MODEMNG_SendMessage(&Msg);
    return RET_OK;
}

static ret_t on_setup_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    MESSAGE_S Msg = {0};
    if (MODEMNG_GetInProgress() == true) {
        return RET_OK;
    }
    if (timer_id != 0) {
        timer_remove(timer_id);
        timer_id = 0;
    }
    if(time_handle != 0) {
        timer_remove(time_handle);
        time_handle = 0;
    }
    Msg.topic = EVENT_MODEMNG_STOP_REC;
    Msg.arg1 = MEDIA_MOVIE_STATE_MENU;
    MODEMNG_SendMessage(&Msg);
    ui_winmng_startwin(UI_SET_PAGE, false);

    return RET_OK;
}

static ret_t on_lvmodechange_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;

    uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
    if(curWind == 0){
        return RET_OK;
    }
    MESSAGE_S Msg = {0};
    Msg.topic = EVENT_MODEMNG_SWITCH_LIVEVIEW;
    uint32_t enWind = (curWind >> 16) & 0xFFFF;
    uint32_t enSns = (curWind & 0xFFFF);

RETRY:
    if(enWind == enSns){
        enWind = 0x1;
    }else if((enWind << 1) > enSns){
        enWind = enSns;
    }else{
        enWind = (enWind << 1);
    }

    if((enWind & enSns) == 0){
        goto RETRY;
    }

    Msg.arg1 = ((enWind << 16) | enSns);

    MODEMNG_SendMessage(&Msg);
    return RET_OK;
}

static ret_t on_photo_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    MESSAGE_S Msg = {0};
#ifdef SERVICES_PHOTO_ON
    Msg.topic = EVENT_MODEMNG_MODESWITCH;
    Msg.arg1 = WORK_MODE_PHOTO;
    MODEMNG_SendMessage(&Msg);
#else
    if (ui_home_cardstatus() == true) {
        Msg.topic = EVENT_MODEMNG_START_PIV;
        MODEMNG_SendMessage(&Msg);
    } else {
        CVI_LOGE("the card is not exist\n");
    }
#endif

    return RET_OK;
}

static ret_t on_playback_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    if(ui_home_cardstatus()) {
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_MODESWITCH;
        Msg.arg1 = WORK_MODE_PLAYBACK;
        MODEMNG_SendMessage(&Msg);
    }

    return RET_OK;
}

static ret_t on_sos_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;

    uint32_t u32ModeState = 0;
    MODEMNG_GetModeState(&u32ModeState);
    if (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC) {
        if(ui_home_cardstatus()) {
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_START_EMRREC;
            MODEMNG_SendMessage(&Msg);
        }
    }

    return RET_OK;
}

static ret_t on_audio_click(void* ctx, event_t* e)
{
    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
    MESSAGE_S Msg = {0};
    if (WORK_MODE_MOVIE == s32CurMode) {
        widget_t* widget_main = WIDGET(ctx);
        widget_t* audio_btm_image_widget = widget_lookup(widget_main, "audio_btm_image", TRUE);

        PARAM_MENU_S menuparam;
        PARAM_GetMenuParam(&menuparam);
        if(menuparam.AudioEnable.Current == 0) {
            Msg.topic = EVENT_MODEMNG_SETTING;
            Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
            Msg.arg2 = 1;
            MODEMNG_SendMessage(&Msg);
            image_base_set_image(audio_btm_image_widget, "audio_mic");
        } else {
            Msg.topic = EVENT_MODEMNG_SETTING;
            Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
            Msg.arg2 = 0;
            MODEMNG_SendMessage(&Msg);
            image_base_set_image(audio_btm_image_widget, "audio_mic_no");
        }
    }

    return RET_OK;
}

static ret_t on_wifi_click(void* ctx, event_t* e)
{
#ifdef WIFI_ON
    widget_t* widget_main = WIDGET(ctx);
    widget_t* wifi_btm_image_widget = widget_lookup(widget_main, "wifi_btm_image", TRUE);

    PARAM_WIFI_S WifiParam = {0};
    MESSAGE_S Msg = {0};
    int32_t  s32Ret = 0;
    s32Ret = PARAM_GetWifiParam(&WifiParam);
    if (0 != s32Ret) {
        return RET_FAIL;
    }
    if (WifiParam.Enable) {
        open_busying_page();
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_WIFI_STATUS;
        Msg.arg2 = 0;
        UICOMM_SendAsyncMsg(&Msg,ui_winmng_receivemsgresult);
        image_base_set_image(wifi_btm_image_widget, "wifi_off");
    } else {
        open_busying_page();
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_WIFI_STATUS;
        Msg.arg2 = 1;
        UICOMM_SendAsyncMsg(&Msg,ui_winmng_receivemsgresult);
        image_base_set_image(wifi_btm_image_widget, "wifi_on");
    }
#endif
    return RET_OK;
}

#ifdef SERVICES_SPEECH_ON
static ret_t on_speech_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
    MESSAGE_S Msg = {0};
    if (WORK_MODE_MOVIE == s32CurMode) {
        widget_t* widget_main = WIDGET(ctx);
        widget_t* speech_btm_image_widget = widget_lookup(widget_main, "speech_btm_image", TRUE);

        SPEECHMNG_PARAM_S SpeechParam = {0};
        PARAM_GetSpeechParam(&SpeechParam);
        if (SpeechParam.enable == 0) {
            Msg.topic = EVENT_MODEMNG_START_SPEECH;
            MODEMNG_SendMessage(&Msg);
            image_base_set_image(speech_btm_image_widget, "speech_white");
            SpeechParam.enable = 1;
            PARAM_SetSpeechParam(&SpeechParam);
        } else {
            Msg.topic = EVENT_MODEMNG_STOP_SPEECH;
            MODEMNG_SendMessage(&Msg);
            image_base_set_image(speech_btm_image_widget, "speech_stop");
            SpeechParam.enable = 0;
            PARAM_SetSpeechParam(&SpeechParam);
        }
    }
    return RET_OK;
}

static void Set_BTM_Speech_Image(widget_t* ctx)
{
    widget_t* widget_main = WIDGET(ctx);
    char *res_image_buf[] = {"speech_stop", "speech_white"};

    SPEECHMNG_PARAM_S SpeechParam = {0};
    PARAM_GetSpeechParam(&SpeechParam);
    image_base_set_image(widget_main, res_image_buf[SpeechParam.enable]);
}
#endif

#ifdef SERVICES_ADAS_LABEL_LANE_ON
static int32_t values_lane[8][4];
static ret_t on_paint_lane(void* ctx, event_t* e) {
    paint_event_t* evt = (paint_event_t*)e;
    widget_t* widget = WIDGET(evt->e.target);
    int32_t bool_draw = widget_get_prop_int(widget, "bool_draw", 0);
    if (bool_draw) {
        canvas_t* c = evt->c;
        vgcanvas_t* vg = lcd_get_vgcanvas(c->lcd);
        color_t bg = color_init(0xff, 0, 0, 0xff);
        if (vg != NULL) {
            vgcanvas_save(vg);
            vgcanvas_set_line_width(vg, 8);
            vgcanvas_set_stroke_color(vg, bg);

            int32_t count = widget_get_prop_int(widget, "count", 0);
            for(int32_t i = 0; i < count; ++i) {
                vgcanvas_begin_path(vg);
                int32_t start_x = values_lane[i][0];
                int32_t start_y = values_lane[i][1];
                int32_t end_x = values_lane[i][2];
                int32_t end_y = values_lane[i][3];
                // printf(" x1, y1: %d, %d, x2, y2: %d, %d\n",start_x, start_y, end_x, end_y);
                vgcanvas_move_to(vg, start_x, start_y);
                vgcanvas_line_to(vg, end_x, end_x);
                vgcanvas_stroke(vg);
            }

            vgcanvas_restore(vg);
        }
    }

    return RET_OK;
}
#endif

#if defined(SERVICES_ADAS_LABEL_LANE_ON) || defined(SERVICES_ADAS_LABEL_CAR_ON)
static int32_t draw_transfer(int32_t camid, int32_t source_x1, int32_t source_y1, int32_t source_x2, int32_t source_y2, int32_t *dst)
{
    PARAM_ADAS_ATTR_S ADASAttr = {0};
    PARAM_GetADASConfigParam(&ADASAttr);
    int32_t grp_id = ADASAttr.ChnAttrs[camid].BindVprocId;
    int32_t bind_chn_id = ADASAttr.ChnAttrs[camid].BindVprocChnId;

    PARAM_DISP_ATTR_S  disp_attr;
    PARAM_GetVoParam(&disp_attr);
    long vo_width = (long)disp_attr.Width;
    long vo_height = (long)disp_attr.Height;

    PARAM_MEDIA_SPEC_S params;
    PARAM_GetMediaMode(camid, &params);
    uint32_t vi_width = params.VcapAttr.VcapChnAttr.u32Width;
    uint32_t vi_height = params.VcapAttr.VcapChnAttr.u32Height;
    uint32_t source_width = params.VprocAttr.VprocChnAttr[bind_chn_id].VpssChnAttr.u32Width;
    uint32_t source_height = params.VprocAttr.VprocChnAttr[bind_chn_id].VpssChnAttr.u32Height;
    int32_t chn_id = -1;
    for (int32_t j = 0; j < MAPI_VPROC_MAX_CHN_NUM; j++) {
        if((params.VprocAttr.VprocChnAttr[j].VpssChnAttr.u32Width == vo_width) && (params.VprocAttr.VprocChnAttr[j].VpssChnAttr.u32Height == vo_height))
        {
            chn_id = params.VprocAttr.VprocChnAttr[j].VprocChnid;
            break;
        }
    }
    if (chn_id == -1) {
        CVI_LOGE("chn_id %d is nonvalid\n", chn_id);
        return -1;
    }

    int32_t vi_x1 = source_x1 * vi_width;
    int32_t vi_x2 = source_x2 * vi_width;
    int32_t vi_y1 = source_y1 * vi_height;
    int32_t vi_y2 = source_y2 * vi_height;

    VPSS_CROP_INFO_S stCropInfo = {0};
    int32_t s32Ret = 0;
    s32Ret = CVI_VPSS_GetChnCrop(grp_id, chn_id, &stCropInfo);
    if (s32Ret != 0) {
        CVI_LOGE("CVI_VPSS_GetChnCrop fail with %#x\n", s32Ret);
        return s32Ret;
    }
    int32_t s32X = stCropInfo.stCropRect.s32X;
    int32_t s32Y = stCropInfo.stCropRect.s32Y;
    int32_t u32Width = stCropInfo.stCropRect.u32Width;
    int32_t u32Height = stCropInfo.stCropRect.u32Height;
    int32_t vi_min_x = s32X * source_width;
    int32_t vi_min_y = s32Y * source_height;

    int32_t new_x1 = vi_x1 - vi_min_x;
    int32_t new_x2 = vi_x2 - vi_min_x;
    int32_t new_y1 = vi_y1 - vi_min_y;
    int32_t new_y2 = vi_y2 - vi_min_y;

    dst[0] = ceil(new_x1 * vo_width/ (u32Width * source_width));
    dst[1] = ceil(new_y1 * vo_height/ (u32Height * source_height));
    dst[2] = ceil(new_x2 * vo_width/ (u32Width * source_width));
    dst[3] = ceil(new_y2 * vo_height/ (u32Height * source_height));

    return 0;
}
#endif

#ifdef SERVICES_ADAS_LABEL_CAR_ON
static int32_t values_car[8][4];
static ret_t on_paint_car(void* ctx, event_t* e) {
    paint_event_t* evt = (paint_event_t*)e;
    widget_t* widget = WIDGET(evt->e.target);
    int32_t bool_draw = widget_get_prop_int(widget, "bool_draw", 0);
    if (bool_draw) {
        canvas_t* c = evt->c;
        vgcanvas_t* vg = lcd_get_vgcanvas(c->lcd);
        if (vg != NULL) {
            vgcanvas_save(vg);
            color_t bg = color_init(0, 0xff, 0, 0xff);
            vgcanvas_set_stroke_color(vg, bg);

            int32_t count = widget_get_prop_int(widget, "count", 0);
            for (int32_t i = 0; i < count; ++i) {
                vgcanvas_begin_path(vg);
                float x = (float)values_car[i][0];
                float y = (float)values_car[i][1];
                float w = (float)values_car[i][2];
                float h = (float)values_car[i][3];
                // printf(" x1, y1: %f, %f, x2, y2: %f, %f\n",x, y, w, h);
                vgcanvas_rect(vg, x, y, w, h);
                vgcanvas_stroke(vg);
            }
            vgcanvas_restore(vg);
        }
    }

    return RET_OK;
}
#endif

#ifdef CONFIG_PWM_ON
static ret_t on_slider_changed(void* ctx, event_t* e)
{
    #ifdef CONFIG_PWM_ON
    (void)e;
    int32_t  ret = 0;
    widget_t* widget = WIDGET(ctx);
    uint32_t slider_value = widget_get_value(widget);
    CVI_LOGD("slider_value = %d\n", slider_value);
    if(slider_value < 10)
    {
        slider_value = 10;
    }
    ret = HAL_PWM_Set_Percent(slider_value);
    if(-1 == ret) {
        CVI_LOGD("error the rate, need input correct parm\n");
    }
    PARAM_PWM_S Param;
    ret = PARAM_GetPWMParam(&Param);
    if(ret == 0) {
        Param.PWMCfg.duty_cycle = Param.PWMCfg.period * slider_value / 100;
        HAL_SCREEN_COMM_SetLuma(HAL_SCREEN_IDXS_0, Param.PWMCfg);
    } else {
        CVI_LOGE("%s : PARAM_GetPWMParam failed\n",__func__);
    }
    ret = PARAM_SetPWMParam(&Param);
    if(ret != 0) {
        CVI_LOGE("%s : PARAM_SetPWMParam failed\n",__func__);
    }
    #endif
    return RET_OK;
}
#endif

#ifdef ADC_ON
static ret_t progress_bar_on_timer(const timer_info_t* timer)
{
    #ifdef ADC_ON
    widget_t* progress_bar = (widget_t*)timer->ctx;
    USB_STATE_E penState = USB_STATE_OUT;
    if(NULL == progress_bar || NULL == progress_bar->name) {
        CVI_LOGE("progress_bar is null !\n");
        return RET_REPEAT;
    }

    USB_GetState(&penState);
    progress_bar_set_value(progress_bar, USB_STATE_INSERT == penState ? 100 : GAUGEMNG_GetPercentage());
    #endif
    return RET_REPEAT;
}
#endif

static void Set_LoopTime_Image(widget_t* ctx)
{
    widget_t* widget_main = WIDGET(ctx);
    char *loop_image_buf[] = {"one_min", "three_min", "five_min"};
    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    image_base_set_image(widget_main, loop_image_buf[param.VideoLoop.Current]);
}

static void Set_Res_Image(widget_t* ctx)
{
    widget_t* widget_main = WIDGET(ctx);
    char *res_image_buf[] = {"1080P", "1440P", "1660P"};
    int32_t  value = 0;
    uint32_t videosize = 0;
    PARAM_MENU_S param;
    PARAM_GetMenuParam(&param);
    PARAM_GetVideoSizeEnum(param.VideoSize.Current, &videosize);
    switch (videosize) {
        case MEDIA_VIDEO_SIZE_1920X1080P25:
            value = 0;
            break;
        case MEDIA_VIDEO_SIZE_2560X1440P25:
            value = 1;
            break;
        case MEDIA_VIDEO_SIZE_2560X1600P25:
            value = 2;
            break;
        default:
            CVI_LOGE("value is invalid");
            break;
    }
    image_base_set_image(widget_main, res_image_buf[value]);
}

static void Set_BTM_Audio_Image(widget_t* ctx)
{
    widget_t* widget_main = WIDGET(ctx);
    char *res_image_buf[] = {"audio_mic_no", "audio_mic"};

    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    image_base_set_image(widget_main, res_image_buf[menuparam.AudioEnable.Current]);
}

static void Set_BTM_Wifi_Image(widget_t* ctx)
{
#ifdef WIFI_ON
    widget_t* widget_main = WIDGET(ctx);
    char *res_image_buf[] = {"wifi_off", "wifi_on"};

    PARAM_WIFI_S WifiParam = {0};
    PARAM_GetWifiParam(&WifiParam);
    if (WifiParam.Enable) {
        image_base_set_image(widget_main, res_image_buf[1]);
    } else {
        image_base_set_image(widget_main, res_image_buf[0]);
    }

#endif
    return;
}

void Set_ProgressBbar(widget_t* ctx)
{
    #ifdef ADC_ON
    widget_t* widget_main = WIDGET(ctx);
    USB_STATE_E penState = USB_STATE_OUT;
    USB_GetState(&penState);
    progress_bar_set_value(widget_main, penState == USB_STATE_INSERT ? 100 : GAUGEMNG_GetPercentage());
    progress_bar_set_show_text(widget_main, true);
    if(time_handle == 0) {
        time_handle = widget_add_timer(widget_main, progress_bar_on_timer, 3000);
    }
    #endif
}

static ret_t init_widget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);
    (void)ctx;

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (tk_str_eq(name, "home_record")) {
            widget_set_visible(widget, FALSE, FALSE);
        } else if (tk_str_eq(name, "home_startrec_button")) {
            widget_on(widget, EVT_CLICK, on_startrec_click, win);
        } else if (tk_str_eq(name, "home_setup_button")) {
            widget_on(widget, EVT_CLICK, on_setup_click, win);
        } else if (tk_str_eq(name, "home_videochange_button")) {
            widget_on(widget, EVT_CLICK, on_lvmodechange_click, win);
        } else if (tk_str_eq(name, "home_photo_button")) {
            widget_on(widget, EVT_CLICK, on_photo_click, win);
        } else if (tk_str_eq(name, "home_playback_button")) {
            widget_on(widget, EVT_CLICK, on_playback_click, win);
        } else if (tk_str_eq(name, "home_sos_button")) {
            widget_on(widget, EVT_CLICK, on_sos_click, win);
        } else if (tk_str_eq(name, "home_audio_button")) {
            widget_on(widget, EVT_CLICK, on_audio_click, win);
        } else if (tk_str_eq(name, "slider")) {
#ifdef CONFIG_PWM_ON
            HAL_SCREEN_PWM_S Attr = HAL_SCREEN_COMM_GetLuma(HAL_SCREEN_IDXS_0);
            slider_set_value(widget, Attr.duty_cycle * 100 /Attr.period);
            widget_on(widget, EVT_VALUE_CHANGED, on_slider_changed, widget);
#endif
        } else if (tk_str_eq(name, "home_wifi_button")) {
            widget_on(widget, EVT_CLICK, on_wifi_click, win);
        }else if (tk_str_eq(name, "res_image")) {
            Set_Res_Image(widget);
        } else if (tk_str_eq(name, "loop_image")) {
            Set_LoopTime_Image(widget);
        // } else if (tk_str_eq(name, "progress_bar")) {
        //     Set_ProgressBbar(widget);
        } else if (tk_str_eq(name, "audio_btm_image")) {
            Set_BTM_Audio_Image(widget);
        } else if (tk_str_eq(name, "wifi_btm_image")) {
            Set_BTM_Wifi_Image(widget);
        } else if (tk_str_eq(name, "home_speech_button")) {
#ifdef SERVICES_SPEECH_ON
            widget_set_visible(widget, TRUE, FALSE);
            widget_on(widget, EVT_CLICK, on_speech_click, win);
#endif
        } else if (tk_str_eq(name, "speech_btm_image")) {
#ifdef SERVICES_SPEECH_ON
            Set_BTM_Speech_Image(widget);
            widget_set_visible(widget, TRUE, FALSE);
#endif
        }
    }

    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

static ret_t on_window_close(void* ctx, event_t* e) {
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    return RET_OK;
}

/**
 * 初始化
 */

ret_t ui_home_open(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    if (win) {
        timer_id = widget_add_timer(win, on_systime_update, 1000);
        widget_on(win, EVT_POINTER_MOVE, on_home_page_move, win);
        widget_on(win, EVT_WINDOW_CLOSE, on_window_close, win);
        widget_on(win, EVT_LOW_BATTERY, on_home_page_low_battery, win);
        init_children_widget(win);
        if (ui_home_cardstatus() == true) {
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_START_REC;
            MODEMNG_SendMessage(&Msg);
            clock_gettime(CLOCK_BOOTTIME, &t_open);
        } else {
            MODEMNG_SetModeState(MEDIA_MOVIE_STATE_VIEW);
        }
    }

    return RET_OK;
}

ret_t ui_home_close(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    if (timer_id != 0) {
        timer_remove(timer_id);
        timer_id = 0;
    }
    if(time_handle != 0) {
        timer_remove(time_handle);
        time_handle = 0;
    }
    return RET_OK;
}

int32_t  ui_homepage_eventcb(void *argv, EVENT_S *msg)
{
    MESSAGE_S Msg = {0};
    uint32_t u32ModeState = 0;
    MODEMNG_GetModeState(&u32ModeState);
    // CVI_LOGD("u32ModeState == %d\n", u32ModeState);
    // CVI_LOGD("ui home eventcb will process message topic(%x)\n", msg->topic);
    switch(msg->topic) {
        case EVENT_MODEMNG_MODEOPEN:
        {
            ui_winmng_startwin(UI_HOME_PAGE, false);
            break;
        }
        case EVENT_MODEMNG_MODECLOSE:
        {
            ui_winmng_closeallwin();
            break;
        }
        case EVENT_MODEMNG_CARD_FSERROR:
        case EVENT_MODEMNG_CARD_SLOW:
        case EVENT_MODEMNG_CARD_CHECKING:
        case EVENT_MODEMNG_CARD_UNAVAILABLE:
        case EVENT_MODEMNG_CARD_ERROR:
        case EVENT_MODEMNG_CARD_READ_ONLY:
        case EVENT_MODEMNG_CARD_MOUNT_FAILED:
        case EVENT_MODEMNG_CARD_REMOVE:
        {
            uint32_t type = ui_wrnmsg_get_type();
            if ((u32ModeState != MEDIA_MOVIE_STATE_MENU) ||
                (type == EVT_FORMAT_PROCESS)) {
                ui_home_cardstatus();
            }
            break;
        }
        case EVENT_MODEMNG_CARD_AVAILABLE:
        {
            if (app_setting_flag == WIFI_APP_SETTING_IN) {
                break;
            }
        }
        case EVENT_MODEMNG_RESET:
        {
            if (u32ModeState != MEDIA_MOVIE_STATE_MENU && ui_home_cardstatus() == true) {
                Msg.topic = EVENT_MODEMNG_START_REC;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        // case EVENT_FILEMNG_SCAN_COMPLETED:
        // {
        //     Msg.topic = EVENT_FILEMNG_SCAN_COMPLETED;
        //     MODEMNG_SendMessage(&Msg);

        //     // if (u32ModeState != MEDIA_MOVIE_STATE_MENU && ui_home_cardstatus() == true) {
        //     //     Msg.topic = EVENT_MODEMNG_START_REC;
        //     //     MODEMNG_SendMessage(&Msg);
        //     // }
        //     break;
        // }
        case EVENT_GSENSORMNG_COLLISION:
        {
            CVI_LOGD("EVENT_GSENSORMNG_COLLISION received! \n");
            if ((u32ModeState != MEDIA_MOVIE_STATE_MENU) &&
                ui_home_cardstatus() && MODEMNG_GetEmrState() == false) {
                Msg.topic = EVENT_MODEMNG_START_EMRREC;
                MODEMNG_SendMessage(&Msg);
            }
            break;
        }
        case EVENT_MODEMNG_RECODER_STARTPIVSTAUE:
        {
            if (msg->arg1 == 0)  {
                VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_PHOTO_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=false,
                };
                VOICEPLAY_Push(&stVoice, 0);
            }
            break;
        }
        case EVENT_MODEMNG_RECODER_STARTSTATU:
        {
            if (msg->arg1 == 0)  {
            /*    VOICEPLAY_VOICE_S stVoice=
                {
                    .au32VoiceIdx={UI_VOICE_REC_IDX},
                    .u32VoiceCnt=1,
                    .bDroppable=false,
                };
                VOICEPLAY_Push(&stVoice, 0);*/
            }
            break;
        }
        case EVENT_MODEMNG_RECODER_STOPSTATU:
        {
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case EVENT_MODEMNG_RECODER_SPLITREC:
        {
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case EVENT_MODEMNG_RECODER_STARTEVENTSTAUE:
        {
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case EVENT_MODEMNG_RECODER_STOPEVENTSTAUE:
        {
            CVI_LOGD("msg->arg1 = %d msg->aszPayload = %s\n", msg->arg1, msg->aszPayload);
            break;
        }
        case EVENT_MODEMNG_CARD_FORMAT_SUCCESSED:
        {
            if (u32ModeState == MEDIA_MOVIE_STATE_MENU) {
                ui_winmng_startwin(UI_HOME_PAGE, TRUE);
            }
            return 0;
        }
        case EVENT_NETCTRL_APPCONNECT_SUCCESS:
        {
            ui_wrnmsg_update_type(MSG_EVENT_APP_CONNECT_SUCCESS);
            ui_winmng_startwin(UI_WRNMSG_PAGE, false);
            break;
        }
        case EVENT_NETCTRL_APPDISCONNECT:
        {
            app_setting_flag = WIFI_APP_SETTING_OUT;
            ui_wrnmsg_update_type(MSG_EVENT_APP_DISCONNECT);
            ui_winmng_finishwin(UI_WRNMSG_PAGE);
            if (msg->arg1 == 1) {
                NETCTRLINNER_ScanFile();
            }
            if (MODEMNG_GetCurWorkMode() == WORK_MODE_MOVIE && ui_home_cardstatus() == true) {
                uint32_t u32ModeState = 0;
                MODEMNG_GetModeState(&u32ModeState);
                if ((u32ModeState != MEDIA_MOVIE_STATE_REC) ||
                    (u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC)) {
                    MESSAGE_S Msg = {0};
                    Msg.topic = EVENT_MODEMNG_START_REC;
                    MODEMNG_SendMessage(&Msg);
                }
            }
            break;
        }
        case EVENT_NETCTRL_APPCONNECT_SETTING:
        {
            app_setting_flag = msg->arg1;
            break;
        }
        case EVENT_NETCTRL_UIUPDATE:
        {
            widget_t* topwin = window_manager_get_top_main_window(window_manager());
            if (topwin != NULL) {
                widget_t* audio_btm_image_widget = widget_lookup(topwin, "audio_btm_image", TRUE);
                widget_t* res_image_widget = widget_lookup(topwin, "res_image", TRUE);
                widget_t* loop_image_widget = widget_lookup(topwin, "loop_image", TRUE);
                Set_BTM_Audio_Image(audio_btm_image_widget);
                Set_Res_Image(res_image_widget);
                Set_LoopTime_Image(loop_image_widget);
            }
            break;
        }
#ifdef SERVICES_ADAS_LABEL_CAR_ON
        case EVENT_ADASMNG_LABEL_CAR:
        {
            widget_t* topwin = window_manager_get_top_main_window(window_manager());
            if (topwin != NULL) {
                widget_t* widget = canvas_widget_create(topwin, 0, 0, topwin->w, topwin->h);
                if (widget) {
                    widget_on(widget, EVT_PAINT, on_paint_car, NULL);
                    widget_set_prop_int(widget, "count", msg->arg1);
                    for (int32_t i = 0; i < msg->arg1; ++i) {
                        int32_t offset = i << 4;
                        int32_t x1 = *((int32_t*)(msg->aszPayload + 0 + offset));
                        int32_t y1 = *((int32_t*)(msg->aszPayload + 4+ offset));
                        int32_t x2 = *((int32_t*)(msg->aszPayload + 8+ offset));
                        int32_t y2 = *((int32_t*)(msg->aszPayload + 12+ offset));
                        int32_t dst[4];
                        if (draw_transfer(msg->arg2, x1, y1, x2, y2, dst) != 0)
                            continue;
                        values_car[i][0] = dst[0];
                        values_car[i][1] = dst[1];
                        values_car[i][2] = dst[2] - dst[0];
                        values_car[i][3] = dst[3] - dst[1];
                    }
                    widget_set_prop_int(widget, "bool_draw", 1);
                    // widget_invalidate(widget, NULL);
                    usleep(15 * 1000);
                    widget_destroy(widget);
                }
            }
            break;
        }
#endif
#ifdef SERVICES_ADAS_LABEL_LANE_ON
        case EVENT_ADASMNG_LABEL_LANE:
        {
            widget_t* topwin = window_manager_get_top_main_window(window_manager());
            if (topwin != NULL) {
                widget_t* widget = canvas_widget_create(topwin, 0, 0, topwin->w, topwin->h);
                if (widget) {
                    widget_on(widget, EVT_PAINT, on_paint_lane, NULL);
                    widget_set_prop_int(widget, "count", msg->arg1);
                    for (int32_t i = 0; i < msg->arg1; ++i) {
                        int32_t offset = i << 4;
                        int32_t x0 = *((int32_t*)(msg->aszPayload + 0 + offset));
                        int32_t y0 = *((int32_t*)(msg->aszPayload + 4 + offset));
                        int32_t x1 = *((int32_t*)(msg->aszPayload + 8 + offset));
                        int32_t y1 = *((int32_t*)(msg->aszPayload + 12 + offset));
                        int32_t dst[4];
                        if (draw_transfer(msg->arg2, x0, y0, x1, y1, dst) != 0)
                            continue;
                        values_lane[i][0] = dst[0];
                        values_lane[i][1] = dst[1];
                        values_lane[i][2] = dst[2];
                        values_lane[i][3] = dst[3];
                    }
                    widget_set_prop_int(widget, "bool_draw", 1);
                    // widget_invalidate(widget, NULL);
                    usleep(20 * 1000);
                    widget_destroy(widget);
                }
            }
            break;
        }
#endif
        default:
            break;
    }
    return 0;
}

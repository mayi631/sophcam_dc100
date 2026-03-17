#include <stdio.h>
#include "ui_windowmng.h"
#include "param.h"
#include "mode.h"
#include "hal_pwm.h"
#include "usb.h"
typedef struct tagUI_MENU_SEL_S
{
    char     name[32];
} UI_MENU_SEL_S;

typedef struct tagUI_OPTION_SEL_S
{
    uint8_t           cnt;
    UI_MENU_SEL_S     *optionname;
} UI_OPTION_SEL_S;

enum _MENU_ITEM_ID
{
    MENU_ITEM_ID_RESOLUTION = 0,
    MENU_ITEM_ID_LOOPTIME,
    MENU_ITEM_ID_LAPSE,
    MENU_ITEM_ID_AUDIO,
    MENU_ITEM_ID_OSD,
    MENU_ITEM_ID_CODEC,
    MENU_ITEM_ID_SCREENDORMANT,
    MENU_ITEM_PWM_BRI,
    MENU_ITEM_VIEW_WIN,
    MENU_ITEM_ID_FORMAT,
    MENU_ITEM_SETTING_TIME,
    MENU_ITEM_SETTING_UVCMODE,
    MENU_ITEM_ID_MAX
};

static UI_MENU_SEL_S ui_item[] = {
    { "STRID_RESOLUTION"        },
    { "STRID_SPLITTIME"         },
    { "STRID_LAPSE_REC_TIME"    },
    { "STRID_AUDIO"             },
    { "STRID_OSD"               },
    { "STRID_VIDEO_ENCODE"      },
    { "STRID_SCREEN_DORMANT"    },
    { "STRID_PWM_BRI"           },
    { "STRID_VIEW_WIN"          },
    { "STRID_FORMAT_SD"         },
    { "STRID_SETTING_TIME"      },
    { "STRID_USB_MODE"     }
};

static UI_MENU_SEL_S ui_optionres[] = {
    //{ "STRID_1600P"              },
    { "STRID_1440P"              },
    { "STRID_1080P"              },
    //{ "STRID_1296P"              },
    //{ "STRID_720P"               },
    //{ "STRID_4K"                 }
};

static UI_MENU_SEL_S ui_phoptionres[] = {
    { "STRID_VGA"              },
    { "STRID_3M"              },
    { "STRID_5M"              },
    { "STRID_8M"               },
    { "STRID_10M"                 },
    { "STRID_12M"                 },
};

static UI_MENU_SEL_S ui_optionbirpwm[] = {
    { "STRID_LCD_LOW"               },
    { "STRID_LCD_MID"               },
    { "STRID_LCD_HIGH"              },
};

static UI_MENU_SEL_S ui_optionscreendormant[] = {
    { "STRID_OFF"               },
    { "STRID_1MIN"              },
    { "STRID_3MIN"              },
    { "STRID_5MIN"              }
};

static UI_MENU_SEL_S ui_optionviewwin[] = {
    { "STRID_FRONT_WIN"              },
    { "STRID_BACK_WIN"               },
    { "STRID_DOUBLE_WIN"             },
};

static UI_MENU_SEL_S ui_optionloop[] = {
    { "STRID_1MIN"              },
    { "STRID_3MIN"              },
    { "STRID_5MIN"              }
};

static UI_MENU_SEL_S ui_optionlapse[] = {
    { "STRID_OFF"               },
    { "STRID_1S"                },
    { "STRID_2S"                },
    { "STRID_3S"                }
};

static UI_MENU_SEL_S ui_optionaudio[] = {
    { "STRID_OFF"               },
    { "STRID_ON"                }
};

static UI_MENU_SEL_S ui_optionosd[] = {
    { "STRID_OFF"               },
    { "STRID_ON"                }
};

static UI_MENU_SEL_S ui_optioncodec[] = {
    { "STRID_H264"               },
    { "STRID_H265"               }
};

static UI_MENU_SEL_S ui_optionmode[] = {
    { "STRID_OFF"              },
    { "STRID_UVC"              },
    { "STRID_USB_STORAGE"      }
};

static UI_OPTION_SEL_S option[] = {
    {sizeof(ui_optionres) / sizeof(ui_optionres[0]), ui_optionres},
    {sizeof(ui_optionloop) / sizeof(ui_optionloop[0]), ui_optionloop},
    {sizeof(ui_optionlapse) / sizeof(ui_optionlapse[0]), ui_optionlapse},
    {sizeof(ui_optionaudio) / sizeof(ui_optionaudio[0]), ui_optionaudio},
    {sizeof(ui_optionosd) / sizeof(ui_optionosd[0]), ui_optionosd},
    {sizeof(ui_optioncodec) / sizeof(ui_optioncodec[0]), ui_optioncodec},
    {sizeof(ui_optionscreendormant) / sizeof(ui_optionscreendormant[0]), ui_optionscreendormant},
    {sizeof(ui_optionbirpwm) / sizeof(ui_optionbirpwm[0]), ui_optionbirpwm},
    {sizeof(ui_optionviewwin) / sizeof(ui_optionviewwin[0]), ui_optionviewwin},
    {MENU_ITEM_ID_FORMAT, NULL},
    {MENU_ITEM_SETTING_TIME, NULL},
    {sizeof(ui_optionmode) / sizeof(ui_optionmode[0]), ui_optionmode},
};

static const char* ui_optionwidget[] = {
    "back_button",
    "left_button",
    "right_button",
    "top_view",
    "list_item",
    "list_item1",
    "list_item2",
    "list_item3",
    "list_item4",
    "list_item5",
    "list_item6",
    "list_item7",
    "list_item8",
    "list_item9",
    "list_item10",
    "list_item11"
};

typedef ret_t (*UISet_CallBack)(void* ctx, event_t* e);

typedef struct tagUI_SELOPTION_CALLBACK_S {
    UISet_CallBack ui_seloption_EVT;
    bool ui_seloption_lookup;
    bool ui_seloption_tr;
    bool ui_seloption_set_click;
    bool ui_seloption_set_sensitive;
} UI_SELOPTION_CALLBACK_S;

typedef struct tagUI_MEU_CALLBACK_S {
    UISet_CallBack ui_widget_on;
    bool ui_set_video;
	bool ui_set_label;
} UI_MEU_CALLBACK_S;

typedef struct tagUI_CLICK_CALLBACK_S {
    uint32_t type;
    UISet_CallBack ui_seloption;
} UI_CLICK_CALLBACK_S;

typedef struct tagUI_OPTIONUPDATE_CALLBACK_S {
    bool ui_set_update_video;
    bool ui_set_update_lable;
    bool ui_set_update_init;
} UI_OPTIONUPDATE_CALLBACK_S;

static const UI_OPTIONUPDATE_CALLBACK_S s_astOptionUpdateWinCallBack[] = {
    {false, false, false},
    {false, false, false},
    {false, false, false},
    {false, false, false},
    {true, false, true}, // list_item
    {false, true, true},
	{false, true, true},
	{false, true, true},
	{false, true, true},
	{false, true, true},
	{false, true, true},
	{false, true, true},
	{false, true, false}, // list_item8
	{false, false, false},
	{false, false, false},
	{false, false, true},
};

static uint8_t type = 0;
static int32_t  selOpt = 0;

static void set_option_videosize2itemcnt(uint32_t viedosize, uint32_t *itemcnt)
{
    if (viedosize == MEDIA_VIDEO_SIZE_2560X1440P25){
        *itemcnt = 0;
    } else if (viedosize == MEDIA_VIDEO_SIZE_1920X1080P25){
        *itemcnt = 1;
    } else {
        CVI_LOGE("media video size faile !\n");
    }
}

void set_option_usbmodeitemcnt(uint32_t mode, uint32_t *itemcnt)
{
    if (mode == WORK_MODE_UVC){
        *itemcnt = 0;
    } else if (mode == WORK_MODE_STORAGE){
        *itemcnt = 1;
    } else {
        CVI_LOGE("Usb mode faile !\n");
    }
}

static void set_option_itemcnt2videosize(uint32_t *viedosize, int32_t  itemcnt)
{
    if (itemcnt == 0){
        *viedosize = MEDIA_VIDEO_SIZE_2560X1440P25;
    } else if (itemcnt == 1){
        *viedosize = MEDIA_VIDEO_SIZE_1920X1080P25;
    } else {
        CVI_LOGE("media video size faile !\n");
    }
}

static char* get_optionevent(const char* name)
{
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    if (tk_str_eq(name, "list_item1")) {
        return ui_optionloop[menuparam.VideoLoop.Current].name;
    } else if (tk_str_eq(name, "list_item2")) {
        return ui_optionlapse[menuparam.LapseTime.Current].name;
    } else if (tk_str_eq(name, "list_item3")) {
        return ui_optionaudio[menuparam.AudioEnable.Current].name;
    } else if (tk_str_eq(name, "list_item4")) {
        return ui_optionosd[menuparam.Osd.Current].name;
    } else if (tk_str_eq(name, "list_item5")) {
        return ui_optioncodec[menuparam.VideoCodec.Current].name;
    } else if (tk_str_eq(name, "list_item6")) {
        return ui_optionscreendormant[menuparam.ScreenDormant.Current].name;
    } else if (tk_str_eq(name, "list_item7")) {
        return ui_optionbirpwm[menuparam.PwmBri.Current].name;
    } else if (tk_str_eq(name, "list_item8")) {
        uint32_t index = 0;
        uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
        uint32_t enWind = (curWind >> 16) & 0xFFFF;
        uint32_t enSns = (curWind & 0xFFFF);
        if((enWind == enSns) && (enSns != 1)){
            index = MAX_CAMERA_INSTANCES;
        }else{
            for(int32_t  i = 0; i < MAX_CAMERA_INSTANCES; i++){
                if(((enWind >> i) & 0x1) == 1){
                    index = i;
                    break;
                }
            }
        }

        return ui_optionviewwin[index].name;
    }

    return 0;
}

static void set_option_label(widget_t* widget, char *str)
{
    widget_t* label = widget_lookup(widget, "label", TRUE);
    widget_set_tr_text(label, str);
}

static ret_t update_optionwidget(void* ctx, const void* iter)
{
    (void)ctx;
    widget_t* widget = WIDGET(iter);
    int32_t  arrsize = sizeof(ui_optionwidget)/sizeof(ui_optionwidget[0]);
    if (widget->name != NULL) {
        const char* name = widget->name;
        int32_t  id = 0;
        if (arrsize > 0) {
            for (id = 0; id < arrsize; id++) {
                if (tk_str_eq(name, ui_optionwidget[id])) {
                    if (s_astOptionUpdateWinCallBack[id].ui_set_update_video) {
                        int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
                        if (WORK_MODE_MOVIE == s32CurMode) {
                            uint32_t cnt = 0;
                            uint32_t videosize = 0;
                            PARAM_MENU_S menuparam;
                            PARAM_GetMenuParam(&menuparam);
                            PARAM_GetVideoSizeEnum(menuparam.VideoSize.Current, &videosize);
                            set_option_videosize2itemcnt(videosize, &cnt);
                            set_option_label(widget, ui_optionres[cnt].name);
                        } else if (WORK_MODE_PHOTO == s32CurMode) {
                            PARAM_MEDIA_COMM_S menMediaComm;
                            PARAM_GetMediaPhotoSize(&menMediaComm);
                            set_option_label(widget, ui_phoptionres[menMediaComm.Photo.photoid].name);
                        }
                    }

                    if (s_astOptionUpdateWinCallBack[id].ui_set_update_lable) {
                        set_option_label(widget, get_optionevent(name));
                    }
                }
            }
        }
    }

    return RET_OK;
}

static void update_item_optionname(widget_t* widget) {
  widget_foreach(widget, update_optionwidget, widget);
}

static ret_t on_optionback_click(void* ctx, event_t* e)
{
    widget_t* win = WIDGET(ctx);
    (void)e;

    window_close(win);
    ui_winmng_finishwin(UI_OPTION_PAGE);
    update_item_optionname(window_manager_get_top_window(window_manager()));

    return RET_OK;
}

static ret_t set_selimage(widget_t* widget, int32_t  selopt)
{
    widget_t* image = widget_lookup(widget, "image", TRUE);
    int32_t  index = widget_index_of(widget);
    if(index == selopt) {
        widget_set_visible(image, TRUE, FALSE);
        return RET_OK;
    }

    widget_set_visible(image, FALSE, FALSE);
    return RET_OK;
}

static ret_t init_optionupdate(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    (void )ctx;
    int32_t  id = 0;
    if (widget->name != NULL) {
        const char* name = widget->name;
        int32_t  arrsize = sizeof(ui_optionwidget)/sizeof(ui_optionwidget[0]);
        if (arrsize > 0) {
            for (id = 0; id < arrsize; id++) {
                if (tk_str_eq(name, ui_optionwidget[id])) {
                    if (s_astOptionUpdateWinCallBack[id].ui_set_update_init) {
                        set_selimage(widget, selOpt);
                    }
                }
            }
        }
    }

    return RET_OK;
}

static ret_t update_optionssel(widget_t* widget)
{
    widget_foreach(widget, init_optionupdate, widget);

    return RET_OK;
}

static ret_t on_optionres_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    int32_t  msgarg2 = 0;
    if (index != selOpt) {
        int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
        if (WORK_MODE_MOVIE == s32CurMode) {
            uint32_t videosize = 0;
            set_option_itemcnt2videosize(&videosize, index);
            msgarg2 = videosize;
        } else if (WORK_MODE_PHOTO == s32CurMode) {
            msgarg2 = index;
        }
        open_busying_page();
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_VIDEO_SIZE;
        Msg.arg2 = msgarg2;
        UICOMM_SendAsyncMsg(&Msg, ui_winmng_receivemsgresult);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }

    return RET_OK;
}

static ret_t on_optionuvc_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    int32_t  a_usbmode = WORK_MODE_BUTT;
    USB_POWER_SOURCE_E enPowerState = USB_POWER_SOURCE_BUTT;
    USB_CheckPower_Soure(&enPowerState);
    if (index != selOpt) {
        MESSAGE_S Msg = {0};

        if (0 == index) {
            CVI_LOGI("usb close");
            return RET_FAIL;
        } else {
            if (USB_POWER_SOURCE_PC == enPowerState) {
                if ((WORK_MODE_UVC != MODEMNG_GetCurWorkMode()) && (WORK_MODE_STORAGE != MODEMNG_GetCurWorkMode())) {
                    if (1 == index) {
                        a_usbmode = WORK_MODE_UVC;
                    } else {
                        a_usbmode = WORK_MODE_STORAGE;
                    }
                    Msg.topic = EVENT_MODEMNG_MODESWITCH;
                    Msg.arg1 = a_usbmode;
                    UICOMM_SendAsyncMsg(&Msg, ui_winmng_receivemsgresult);
                }
            }
            update_optionssel(win);
        }
    } else {
        CVI_LOGI("Option sel is same!\n");
    }

    return RET_OK;
}

static ret_t on_optionlooptime_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    if (index != selOpt) {
        open_busying_page();
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_VIDEO_LOOP;
        Msg.arg2 = index;
        UICOMM_SendAsyncMsg(&Msg, ui_winmng_receivemsgresult);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }

    return RET_OK;
}

static ret_t on_optionlapse_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    MESSAGE_S Msg = {0};
    if (index != selOpt) {
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_LAPSE_TIME;
        Msg.arg2 = index;
        MODEMNG_SendMessage(&Msg);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }
    return RET_OK;
}

static ret_t on_optionaudio_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    MESSAGE_S Msg = {0};
    if (index != selOpt) {
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_AUDIO_STATUS;
        Msg.arg2 = index;
        MODEMNG_SendMessage(&Msg);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }
    return RET_OK;
}

static ret_t on_optionosd_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    MESSAGE_S Msg = {0};
    if (index != selOpt) {
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_OSD_STATUS;
        Msg.arg2 = index;
        MODEMNG_SendMessage(&Msg);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }
    return RET_OK;
}

static ret_t on_option_pwm_bri_click(void* ctx, event_t* e)
{
    #ifdef CONFIG_PWM_ON
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    if (index != selOpt) {
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_PWM_BRI_STATUS;
        Msg.arg2 = index;
        MODEMNG_SendMessage(&Msg);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }
    #endif
    return RET_OK;
}

static ret_t on_option_view_win_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);

    uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
    if(curWind == 0){
        return RET_OK;
    }

    uint32_t enWind = (curWind >> 16) & 0xFFFF;
    uint32_t enSns = (curWind & 0xFFFF);

    if(index < (int32_t)(sizeof(ui_optionviewwin) / sizeof(ui_optionviewwin[0])) - 1){
        enWind = (0x1 << index);
    }else{
        enWind = enSns;
    }

    if((enWind & enSns) != 0){
        if (index != selOpt) {
            MESSAGE_S Msg = {0};
            Msg.topic = EVENT_MODEMNG_SETTING;
            Msg.arg1 = PARAM_MENU_VIEW_WIN_STATUS;
            Msg.arg2 = ((enWind << 16) | enSns);
            MODEMNG_SendMessage(&Msg);
            selOpt = index;
            update_optionssel(win);
        } else {
            CVI_LOGI("Option sel is same!\n");
        }
    }
    return RET_OK;
}

static ret_t on_option_screendormant_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    if (index != selOpt) {
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_SCREENDORMANT;
        Msg.arg2 = index;
        MODEMNG_SendMessage(&Msg);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }
    return RET_OK;
}

static ret_t on_optioncodec_click(void* ctx, event_t* e)
{
    (void)e;
    widget_t* list_item = WIDGET(ctx);
    widget_t* win = widget_get_window(list_item);
    int32_t  index = widget_index_of(list_item);
    if (index != selOpt) {
        open_busying_page();
        MESSAGE_S Msg = {0};
        Msg.topic = EVENT_MODEMNG_SETTING;
        Msg.arg1 = PARAM_MENU_VIDEO_CODEC;
        Msg.arg2 = index;
        UICOMM_SendAsyncMsg(&Msg, ui_winmng_receivemsgresult);
        selOpt = index;
        update_optionssel(win);
    } else {
        CVI_LOGI("Option sel is same!\n");
    }
    return RET_OK;
}

static const UI_CLICK_CALLBACK_S s_astClickWinCallBack[] = {
    {MENU_ITEM_ID_RESOLUTION, on_optionres_click},
    {MENU_ITEM_ID_LOOPTIME, on_optionlooptime_click},
    {MENU_ITEM_ID_LAPSE, on_optionlapse_click},
    {MENU_ITEM_ID_AUDIO, on_optionaudio_click},
    {MENU_ITEM_ID_OSD, on_optionosd_click},
    {MENU_ITEM_ID_CODEC, on_optioncodec_click},
    {MENU_ITEM_ID_SCREENDORMANT, on_option_screendormant_click},
    {MENU_ITEM_PWM_BRI, on_option_pwm_bri_click},
    {MENU_ITEM_VIEW_WIN, on_option_view_win_click},
    {MENU_ITEM_ID_FORMAT, NULL},
    {MENU_ITEM_SETTING_TIME, NULL},
    {MENU_ITEM_SETTING_UVCMODE, on_optionuvc_click},
};

static ret_t set_seloption_click(widget_t* widget, uint32_t type)
{
    int32_t  arrsize = sizeof(s_astClickWinCallBack)/sizeof(s_astClickWinCallBack[0]);
    int32_t  id = 0;
    for (id = 0; id < arrsize; id++) {
        if (type == s_astClickWinCallBack[id].type) {
            if (s_astClickWinCallBack[id].ui_seloption != NULL) {
                widget_on(widget, EVT_CLICK, s_astClickWinCallBack[id].ui_seloption, widget);
            }
        }
    }

    return RET_OK;
}

static const UI_SELOPTION_CALLBACK_S s_astSELOPTIONWinCallBack[] = {
    {on_optionback_click, false, false, false, false},
    {on_optionback_click, false, false, false, false},
    {on_optionback_click, false, false, false, false},
    {NULL, true, true, false, false},
    {NULL, false, true, true, true}, // list_item
    {NULL, false, true, true, true},
    {NULL, false, true, true, true},
    {NULL, false, true, true, true},
    {NULL, false, true, true, true},
    {NULL, false, true, true, true},
    {NULL, false, true, true, true}, // list_item6
    {NULL, false, false, false, false},
    {NULL, false, false, false, false},
    {NULL, false, false, false, false},
    {NULL, false, false, false, false},
    {NULL, false, true, true, true}, // list_item11
};

static ret_t init_optionwidget(void* ctx, const void* iter)
{
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);
    (void )ctx;
    if (widget->name != NULL) {
        const char* name = widget->name;
        int32_t  index = widget_index_of(widget);

        int32_t  arrsize = sizeof(ui_optionwidget)/sizeof(ui_optionwidget[0]);
        int32_t  id = 0;
        for (id = 0; id < arrsize; id++) {
            if (tk_str_eq(name, ui_optionwidget[id])) {
                if (s_astSELOPTIONWinCallBack[id].ui_seloption_EVT != NULL) {
                    widget_on(widget, EVT_CLICK, s_astSELOPTIONWinCallBack[id].ui_seloption_EVT, win);
                }

                if (s_astSELOPTIONWinCallBack[id].ui_seloption_lookup) {
                    widget_t* label = widget_lookup(widget, "set_label", TRUE);
                    widget_set_tr_text(label, ui_item[type].name);
                }

                if (s_astSELOPTIONWinCallBack[id].ui_seloption_EVT == NULL && (!(s_astSELOPTIONWinCallBack[id].ui_seloption_lookup))) {

                    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
                    if (type == 0) {
                        if (WORK_MODE_MOVIE == s32CurMode) {
                            option[type].cnt = 2;
                        } else if (WORK_MODE_PHOTO == s32CurMode) {
                            option[type].cnt = 6;
                        }
                    }

                    if (index < option[type].cnt) {
                        if ((!(s_astSELOPTIONWinCallBack[id].ui_seloption_lookup)) && (s_astSELOPTIONWinCallBack[id].ui_seloption_tr)) {
                            if (WORK_MODE_MOVIE == s32CurMode) {
                                widget_set_tr_text(widget, option[type].optionname[index].name);
                            } else if (WORK_MODE_PHOTO == s32CurMode) {
                                PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
                                selOpt = pstParamCtx->pstCfg->MediaComm.Photo.photoid;
                                widget_set_tr_text(widget, ui_phoptionres[index].name);
                            }
                        }

                        if (s_astSELOPTIONWinCallBack[id].ui_seloption_set_click) {
                            set_seloption_click(widget, type);
                        }

                        if (s_astSELOPTIONWinCallBack[id].ui_seloption_set_sensitive) {
                            widget_set_sensitive(widget, TRUE);
                        }
                    }
                    set_selimage(widget, selOpt);
                }
            }
        }
    }
    return RET_OK;
}

static void init_optionchildren_widget(widget_t* widget) {
  widget_foreach(widget, init_optionwidget, widget);
}

ret_t open_option_window(void* ctx, event_t* e)
{
    CVI_LOGD("open_option_window\n");
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        init_optionchildren_widget(win);
        return RET_OK;
    }

    return RET_FAIL;
}

ret_t close_option_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    return RET_OK;
}

static ret_t on_setupback_click(void* ctx, event_t* e)
{
    (void)ctx;
    (void)e;
    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
    int32_t  backwinmode = UI_HOME_PAGE;
    if (WORK_MODE_MOVIE == s32CurMode) {
        backwinmode = UI_HOME_PAGE;
    } else if (WORK_MODE_PHOTO == s32CurMode) {
        backwinmode = UI_HOME_PHOTO_PAGE;
    }
    ui_winmng_finishwin(UI_SET_PAGE);
    ui_winmng_startwin(backwinmode, TRUE);
    return RET_OK;
}

static ret_t on_openresolutionoption_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    uint32_t cnt = 0;
    uint32_t videosize = 0;
    PARAM_GetVideoSizeEnum(menuparam.VideoSize.Current, &videosize);
    set_option_videosize2itemcnt(videosize, &cnt);
    selOpt = cnt;
    type = MENU_ITEM_ID_RESOLUTION;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_openo_uvc_click(void* ctx, event_t* e)
{
    (void)e;
    uint32_t cnt = 0;
    selOpt = cnt;
    type = MENU_ITEM_SETTING_UVCMODE;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_openloopoption_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.VideoLoop.Current;
    type = MENU_ITEM_ID_LOOPTIME;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_openlapseoption_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.LapseTime.Current;
    type = MENU_ITEM_ID_LAPSE;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_openaudiooption_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.AudioEnable.Current;
    type = MENU_ITEM_ID_AUDIO;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_openosdoption_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.Osd.Current;
    type = MENU_ITEM_ID_OSD;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_opencodecoption_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.VideoCodec.Current;
    type = MENU_ITEM_ID_CODEC;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_open_pwmbri_option_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.PwmBri.Current;
    type = MENU_ITEM_PWM_BRI;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_open_screendormant_click(void* ctx, event_t* e)
{
    (void)e;
    PARAM_MENU_S menuparam;
    PARAM_GetMenuParam(&menuparam);
    selOpt = menuparam.ScreenDormant.Current;
    type = MENU_ITEM_ID_SCREENDORMANT;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_open_viewwin_option_click(void* ctx, event_t* e)
{
    (void)e;
    uint32_t curWind = (uint32_t)PARAM_Get_View_Win();
    uint32_t enWind = (curWind >> 16) & 0xFFFF;
    uint32_t enSns = (curWind & 0xFFFF);
    if(enWind == enSns){
        selOpt = sizeof(ui_optionviewwin) / sizeof(ui_optionviewwin[0]) - 1;
    }else{
        selOpt = (enWind >> 1);
    }

    type = MENU_ITEM_VIEW_WIN;
    ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
    return RET_OK;
}

static ret_t on_openo_time_ption_click(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    ui_winmng_startwin(UI_SETTIME_PAGE, FALSE);
    return RET_FAIL;
}

static ret_t on_openoption_click(void* ctx, event_t* e)
{
    (void)e;
    (void)ctx;
    ui_winmng_startwin(UI_FORMAT_PAGE, false);
    return RET_FAIL;
}

static ret_t on_menu_page_down(void* ctx, event_t* e)
{
    return RET_OK;
}

static const UI_MEU_CALLBACK_S s_astMEUWinCallBack[] = {
    {on_setupback_click, false, false},
    {on_setupback_click, false, false},
    {on_setupback_click, false, false},
    {NULL, false, false},
    {on_openresolutionoption_click, true, false},
    {on_openloopoption_click, false, true},
    {on_openlapseoption_click, false, true},
    {on_openaudiooption_click, false, true},
    {on_openosdoption_click, false, true},
    {on_opencodecoption_click, false, true},
    {on_open_screendormant_click, false, true},
    {on_open_pwmbri_option_click, false, true},
    {on_open_viewwin_option_click, false, true},
    {on_openoption_click, false, false},
    {on_openo_time_ption_click, false, false},
    {on_openo_uvc_click, false, false},
};

static ret_t init_widget(void* ctx, const void* iter)
{
    (void)ctx;
    widget_t* widget = WIDGET(iter);
    widget_t* win = widget_get_window(widget);
    int32_t  arrsize = sizeof(ui_optionwidget)/sizeof(ui_optionwidget[0]);
    int32_t  id = 0;

    if (widget->name != NULL) {
        const char* name = widget->name;
        if (arrsize > 0) {
            for (id = 0; id < arrsize; id++) {
                if (tk_str_eq(name, ui_optionwidget[id])) {
                    if (s_astMEUWinCallBack[id].ui_widget_on != NULL) {
                        widget_on(widget, EVT_CLICK, s_astMEUWinCallBack[id].ui_widget_on, win);
                    }

                    if (s_astMEUWinCallBack[id].ui_set_video) {
                        int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
                        if (WORK_MODE_MOVIE == s32CurMode) {
                            uint32_t cnt = 0;
                            uint32_t videosize = 0;
                            PARAM_MENU_S menuparam;
                            PARAM_GetMenuParam(&menuparam);
                            PARAM_GetVideoSizeEnum(menuparam.VideoSize.Current, &videosize);
                            set_option_videosize2itemcnt(videosize, &cnt);
                            set_option_label(widget, ui_optionres[cnt].name);
                        } else if (WORK_MODE_PHOTO == s32CurMode) {
                            PARAM_MEDIA_COMM_S menMediaComm;
                            PARAM_GetMediaPhotoSize(&menMediaComm);
                            set_option_label(widget, ui_phoptionres[menMediaComm.Photo.photoid].name);
                        }


                    }

                    if (s_astMEUWinCallBack[id].ui_set_label) {
                        set_option_label(widget, get_optionevent(name));
                    }
                }
            }
        }
    }

    return RET_OK;
}

static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

ret_t open_menu_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        CVI_LOGI("ui_ide_option_page_open \n");
        window_manager_get_top_window(window_manager());
        widget_on(win, EVT_POINTER_DOWN, on_menu_page_down, win);
        init_children_widget(win);
        return RET_OK;
    }

    return RET_FAIL;
}

ret_t close_menu_window(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {

    }
    MODEMNG_SetModeState(MEDIA_MOVIE_STATE_VIEW);
    return RET_OK;
}
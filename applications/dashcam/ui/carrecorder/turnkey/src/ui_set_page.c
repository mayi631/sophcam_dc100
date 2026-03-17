#include <stdio.h>
#include "ui_windowmng.h"
#include "param.h"
#include "mode.h"

#define ONE_PAGE_BUTTONCNT  8
#define PAGE_LABEL_LEN      16
#define UI_GPS_INFO_LEN     128

typedef enum _MENU_ITEM_NUM {
    MENU_ITEM_NUM_DEFAULT = 0,
    MENU_ITEM_NUM_ONE = 1,
    MENU_ITEM_NUM_TWO,
    MENU_ITEM_NUM_THREE,
    MENU_ITEM_NUM_FOUR,
    MENU_ITEM_NUM_FIVE,
    MENU_ITEM_ID_MAX = 24,
    MENU_ITEM_BUTTON_MAX = 30,
    MENU_ITEM_DATE_TIME = 50,
    MENU_ITEM_GPS_INFO = 51,
    MENU_ITEM_DEV_INFO = 52,
    MENU_ITEM_DEFAULT = 53,
    MENU_ITEM_SD_FORMAT = 100,
} MENU_ITEM_NUM;

typedef enum _MENU_BUTTON_NAME {
    RESLUTION_BUTTON = 0,
    AUDIO_BUTTON,
    DATE_BUTTON,
    LOOPRECORDING_BUTTON,
    GSENSOR_BUTTON,
    FATIGUEDRIVING_BUTTON,
    SPEED_BUTTON,
    GPS_BUTTON,
    SPEEDUNIT_BUTTON,
    GPSINFO_BUTTON,
    REARCAMMIRROR_BUTTON,
    APP_BUTTON,
    WIFI_BUTTON,
    LANGUAGE_BUTTON,
    TIMEFOMAT_BUTTON,
    DATETIME_BUTTON,
    TIMEZONE_BUTTON,
    FREQUENCY_BUTTON,
    CLICKTONE_BUTTON,
    SCREENSAVER_BUTTON,
    PARKING_BUTTON,
    FORMAT_BUTTON,
    DEFAULT_BUTTON,
    INFO_BUTTON,
} MENU_BUTTON_NAME;

typedef struct _UI_PAGE_INFO {
    uint8_t button_cnt;
    uint8_t cur_button_id;
    uint8_t cur_page;
    uint8_t pre_page;
    uint8_t page_num;
    uint8_t close_flag;
    int32_t      option_curid[MENU_ITEM_ID_MAX];
} UI_PAGE_INFO_S;

typedef struct _UI_MENU_SEL_S {
    char name[32];
    int32_t   param;
} UI_MENU_SEL_S;

typedef struct _UI_OPTION_SEL_S {
    uint8_t           cnt;
    UI_MENU_SEL_S     *option_name;
    int32_t                option_menu;
} UI_OPTION_SEL_S;

static UI_MENU_SEL_S ui_resoultion[MENU_ITEM_NUM_THREE] = {
    { "STRID_1440P" , MEDIA_VIDEO_SIZE_2560X1440P30 },
    { "STRID_1080P" , MEDIA_VIDEO_SIZE_1920X1080P30 },
    { "STRID_720P"  , MEDIA_VIDEO_SIZE_1280X720P30  },
};
static UI_MENU_SEL_S ui_audio[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MEDIA_VIDEO_AUDIO_ON  },
    { "STRID_OFF" , MEDIA_VIDEO_AUDIO_OFF },
};
static UI_MENU_SEL_S ui_datestamp[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MEDIA_VIDEO_OSD_ON     },
    { "STRID_OFF" , MEDIA_VIDEO_OSD_OFF    },
};
static UI_MENU_SEL_S ui_optionloop[MENU_ITEM_NUM_THREE] = {
    { "STRID_1MIN" , MEDIA_VIDEO_LOOP_1MIN },
    { "STRID_3MIN" , MEDIA_VIDEO_LOOP_3MIN },
    { "STRID_5MIN" , MEDIA_VIDEO_LOOP_5MIN },
};
static UI_MENU_SEL_S ui_gensor[MENU_ITEM_NUM_FOUR] = {
    { "STRID_OFF"     , MENU_GENSOR_OFF },
    { "STRID_LOW"     , MENU_GENSOR_LOW },
    { "STRID_MIDDLE"  , MENU_GENSOR_MID },
    { "STRID_HIGH"    , MENU_GENSOR_HIGH },
};
static UI_MENU_SEL_S ui_fatiguedriving[MENU_ITEM_NUM_FIVE] = {
    { "STRID_OFF"  , MENU_FATIGUEDRIVE_OFF},
    { "STRID_1H"   , MENU_FATIGUEDRIVE_1HOUR},
    { "STRID_2H"   , MENU_FATIGUEDRIVE_2HOUR},
    { "STRID_3H"   , MENU_FATIGUEDRIVE_3HOUR},
    { "STRID_4H"   , MENU_FATIGUEDRIVE_4HOUR},
};
static UI_MENU_SEL_S ui_gpscontrol[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MENU_GPSSTAMP_ON},
    { "STRID_OFF" , MENU_GPSSTAMP_OFF},
};
static UI_MENU_SEL_S ui_speedunit[MENU_ITEM_NUM_TWO] = {
    { "KMH"  , MENU_SPEEDUNIT_KMH},
    { "MPH"  , MENU_SPEEDUNIT_MPH},
};
static UI_MENU_SEL_S ui_rearcammirror[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MENU_REARCAM_MIRROR_ON},
    { "STRID_OFF" , MENU_REARCAM_MIRROR_OFF},
};
static UI_MENU_SEL_S ui_wifi[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MENU_WIFI_ON},
    { "STRID_OFF" , MENU_WIFI_OFF},
};
static UI_MENU_SEL_S ui_language[MENU_ITEM_NUM_TWO] = {
    { "STRID_ZH"  , MENU_LANGUAGE_CHN},
    { "STRID_EN"  , MENU_LANGUAGE_ENG},
};
static UI_MENU_SEL_S ui_timezone[MENU_ITEM_NUM_FIVE] = {
    { "CHN"  , MENU_TIME_ZONE_CHN},
    { "ENG"  , MENU_TIME_ZONE_ENG},
    { "UNI"  , MENU_TIME_ZONE_UNI},
    { "JAP"  , MENU_TIME_ZONE_JAP},
    { "EGP"  , MENU_TIME_ZONE_EGP},
};
static UI_MENU_SEL_S ui_clicktone[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MEDIA_AUDIO_KEYTONE_ON},
    { "STRID_OFF" , MEDIA_AUDIO_KEYTONE_OFF},
};
static UI_MENU_SEL_S ui_parking[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MENU_PARKING_ON},
    { "STRID_OFF" , MENU_PARKING_OFF},
};
static UI_MENU_SEL_S ui_speedmark[MENU_ITEM_NUM_TWO] = {
    { "STRID_ON"  , MENU_SPEEDSTAMP_ON},
    { "STRID_OFF" , MENU_SPEEDSTAMP_OFF},
};
static UI_MENU_SEL_S ui_timefomat[MENU_ITEM_NUM_TWO] = {
    { "STRID_12H"  , MENU_TIME_FORMAT_12},
    { "STRID_24H"  , MENU_TIME_FORMAT_24},
};
static UI_MENU_SEL_S ui_frequency[MENU_ITEM_NUM_THREE] = {
    { "STRID_OFF"   , MENU_FREQUENCY_OFF},
    { "STRID_50HZ"  , MENU_FREQUENCY_50},
    { "STRID_60HZ"  , MENU_FREQUENCY_60},
};
static UI_MENU_SEL_S ui_screensaver[MENU_ITEM_NUM_FOUR] = {
    { "STRID_OFF"  , MENU_SCREENDORMANT_OFF},
    { "STRID_1MIN" , MENU_SCREENDORMANT_1MIN},
    { "STRID_3MIN" , MENU_SCREENDORMANT_3MIN},
    { "STRID_5MIN" , MENU_SCREENDORMANT_5MIN},
};
// identical page label name
static UI_MENU_SEL_S ui_sdformat[MENU_ITEM_SD_FORMAT] = {
    { "STRID_FORMAT_SD_AT_ONCE" , 0},
};
// GPS info
static UI_MENU_SEL_S ui_gpsinfo[MENU_ITEM_GPS_INFO] = {
    { "STRID_GPS_INFO" , 0},
};
static UI_MENU_SEL_S ui_devinfo[MENU_ITEM_DEV_INFO] = {
    { "STRID_ABOUT" , 0},
};
static UI_MENU_SEL_S ui_defalut[MENU_ITEM_DEFAULT] = {
    { "STRID_RESET_FACTORY_AT_ONCE" , 0},
};
static UI_OPTION_SEL_S option[MENU_ITEM_ID_MAX] = {
    // page 1
    {MENU_ITEM_NUM_THREE ,  ui_resoultion   ,  PARAM_MENU_VIDEO_SIZE    },
    {MENU_ITEM_NUM_TWO   ,  ui_audio        ,  PARAM_MENU_AUDIO_STATUS  },
    {MENU_ITEM_NUM_TWO   ,  ui_datestamp    ,  PARAM_MENU_OSD_STATUS    },    //  date button
    {MENU_ITEM_NUM_THREE ,  ui_optionloop   ,  PARAM_MENU_VIDEO_LOOP    },    //  looprecording
    {MENU_ITEM_NUM_FOUR  ,  ui_gensor       ,  PARAM_MENU_GSENSOR       },
    {MENU_ITEM_NUM_FIVE  ,  ui_fatiguedriving, PARAM_MENU_FATIGUE_DRIVE },    //  疲劳驾驶
    {MENU_ITEM_NUM_TWO   ,  ui_speedmark    ,  PARAM_MENU_SPEED_STAMP   },    //  speed buttom
    {MENU_ITEM_NUM_TWO   ,  ui_gpscontrol   ,  PARAM_MENU_GPS_STAMP     },
    //page 2
    {MENU_ITEM_NUM_TWO   ,  ui_speedunit    ,  PARAM_MENU_SPEED_UNIT    },    // 速度单位
    {MENU_ITEM_GPS_INFO  ,  ui_gpsinfo      ,  0                            },    // gpsinfo
    {MENU_ITEM_NUM_TWO   ,  ui_rearcammirror,  PARAM_MENU_REARCAM_MIRROR},    // rearcammirror 后路镜像
    {0                   ,  NULL            ,  0                            },    // app
    {MENU_ITEM_NUM_TWO   ,  ui_wifi         ,  PARAM_MENU_WIFI_STATUS   },
    {MENU_ITEM_NUM_TWO   ,  ui_language     ,  PARAM_MENU_LANGUAGE      },
    {MENU_ITEM_NUM_TWO   ,  ui_timefomat    ,  PARAM_MENU_TIME_FORMAT   },    // timefomat
    {MENU_ITEM_DATE_TIME ,  NULL            ,  0                            },    // datetime
    //page 3
    {MENU_ITEM_NUM_FIVE  ,  ui_timezone     ,  PARAM_MENU_TIME_ZONE     },    // timezone
    {MENU_ITEM_NUM_THREE ,  ui_frequency    ,  PARAM_MENU_FREQUENCY     },    // frequency
    {MENU_ITEM_NUM_TWO   ,  ui_clicktone    ,  PARAM_MENU_KEYTONE       },    // 按键声
    {MENU_ITEM_NUM_FOUR  ,  ui_screensaver  ,  PARAM_MENU_SCREENDORMANT },    // screensaver
    {MENU_ITEM_NUM_TWO   ,  ui_parking      ,  PARAM_MENU_PARKING       },
    {MENU_ITEM_SD_FORMAT ,  ui_sdformat     ,  0                            },    // format SD
    {MENU_ITEM_DEFAULT   ,  ui_defalut      ,  0                            },    // reset
    {MENU_ITEM_DEV_INFO  ,  ui_devinfo      ,  0                            },    // about
};

static UI_PAGE_INFO_S s_page_info = {0};

static ret_t init_widget(void* ctx, const void* iter)
{
    (void)ctx;
    widget_t* widget = WIDGET(iter);
    // widget_t* win = widget_get_window(widget);

    if (widget->name != NULL) {
        const char* name = widget_get_type(widget);
        if (tk_str_eq(name, WIDGET_TYPE_BUTTON)) {
            s_page_info.button_cnt++;
        }
    }

    return RET_OK;
}
static void set_page_label(widget_t* widget, int32_t  curpage, int32_t  pagenum)
{
    char str[PAGE_LABEL_LEN];
    widget_t* label = widget_lookup(widget, "page_label", TRUE);
    snprintf(str, PAGE_LABEL_LEN, "%d/%d", curpage, pagenum);
    CVI_LOGD("set_page_label = %s\n",str);
    widget_set_text_utf8(label, str);
}
static void init_children_widget(widget_t* widget) {
  widget_foreach(widget, init_widget, widget);
}

static ret_t on_set_page_key_down(void* ctx, event_t* e)
{
    key_event_t* evt = (key_event_t*)e;
    widget_t* win = WIDGET(ctx);
    if(evt->alt == TRUE) {
        CVI_LOGD("Ignore long key press ! \n");
        return RET_OK;
    }

    if (ui_winmng_getwinisshow(UI_WRNMSG_PAGE) == true && evt->key != UI_KEY_POWER) {
        ui_winmng_finishwin(UI_WRNMSG_PAGE);
        return 0;
    }

    widget_t* scroll_view = widget_lookup(win, "scroll_view", TRUE);

    CVI_LOGD("on_set_page_key_down keyid ==== %d w-> %d\n", evt->key, scroll_view->h);
    if (evt->key == UI_KEY_POWER) {
        s_page_info.close_flag = TRUE;
        ui_winmng_startwin(UI_HOME_PAGE, TRUE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
        CVI_LOGD("on_set_page_key_down UI_KEY_MENU\n");
        s_page_info.close_flag = FALSE;
        if (option[s_page_info.cur_button_id].cnt <= MENU_ITEM_BUTTON_MAX) {
            // 普通按键菜单
            ui_winmng_startwin(UI_OPTION_PAGE, FALSE);
        } else if (option[s_page_info.cur_button_id].cnt == MENU_ITEM_DATE_TIME) {
            // 时间设置菜单
            ui_winmng_startwin(UI_DATETIME_PAGE, FALSE);
        } else {
            // 信息显示类菜单
            ui_winmng_startwin(UI_IDE_OPTION_PAGE, FALSE);
        }
        return RET_STOP;
    } else if (evt->key == UI_KEY_UP) {

        if (s_page_info.cur_button_id == 0) {
            s_page_info.cur_button_id = s_page_info.button_cnt;
        }
        s_page_info.cur_button_id--;

        s_page_info.cur_page = (s_page_info.cur_button_id / ONE_PAGE_BUTTONCNT);
        CVI_LOGD("key up s_page_info.cur_page = %d \n", s_page_info.cur_page);
        if (s_page_info.cur_page != s_page_info.pre_page) {
            scroll_view_scroll_to(scroll_view, 0, (scroll_view->h * s_page_info.cur_page), TK_ANIMATING_TIME);
        }
        s_page_info.pre_page = s_page_info.cur_page;

        set_page_label(win, s_page_info.cur_page + 1, s_page_info.page_num );
        widget_t* button = widget_get_child(scroll_view, s_page_info.cur_button_id);
        widget_set_prop_bool(button, WIDGET_STATE_FOCUSED, TRUE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        s_page_info.cur_button_id++;
        if (s_page_info.cur_button_id > s_page_info.button_cnt - 1) {
            s_page_info.cur_button_id = 0;
        }

        s_page_info.cur_page = (s_page_info.cur_button_id / ONE_PAGE_BUTTONCNT);
        CVI_LOGD("key down s_page_info.cur_page = %d \n", s_page_info.cur_page);
        if (s_page_info.cur_page != s_page_info.pre_page) {
            scroll_view_scroll_to(scroll_view, 0, (scroll_view->h * s_page_info.cur_page), TK_ANIMATING_TIME);
        }

        s_page_info.pre_page = s_page_info.cur_page;

        set_page_label(win, s_page_info.cur_page + 1, s_page_info.page_num);
        widget_t* button = widget_get_child(scroll_view, s_page_info.cur_button_id);
        widget_set_prop_bool(button, WIDGET_STATE_FOCUSED, TRUE);
        return RET_STOP;
    }

    return RET_OK;
}

ret_t ui_set_page_open(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_set_page_key_down, win);
        widget_t* scroll_view = widget_lookup(win, "scroll_view", TRUE);

        init_children_widget(scroll_view);
        s_page_info.page_num  = s_page_info.button_cnt / ONE_PAGE_BUTTONCNT;
        s_page_info.pre_page = 0;
        scroll_view_set_virtual_h(scroll_view, scroll_view->h * s_page_info.page_num);
        widget_focus_first(scroll_view);
    }
    return RET_OK;
}

ret_t ui_set_page_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        if (s_page_info.close_flag == TRUE) {
            s_page_info.cur_button_id = 0;
            s_page_info.button_cnt = 0;
        }

        MODEMNG_SetModeState(MEDIA_MOVIE_STATE_VIEW);
    }
    return RET_OK;
}


/**
 * optin page code
 *
 * 每个按键边长100
 * 总宽度720
*/
#define WINDOW_WIGHT    720
#define WINDOW_HIGHT    320
#define BUTTON_SIZE     100

static widget_t* option_button[MENU_ITEM_NUM_FIVE];
PARAM_MENU_S menu_param = {0};
static bool creat_button_flag = false;

void option_setuiLanguage(int32_t  param)
{
    switch (param)
    {
        case MENU_LANGUAGE_CHN:
            locale_info_change(locale_info(), "zh", "CN");
            break;
        case MENU_LANGUAGE_ENG:
            locale_info_change(locale_info(), "en", "US");
            break;
        default:
            break;
    }
}

static int32_t  option_OnReceiveMsgResult(EVENT_S* pstEvent)
{
    // int32_t  s32Ret = 0;
    (void)pstEvent;
    ui_winmng_finishwin(UI_OPTION_PAGE);
    return 0;
}

static int32_t  option_OnReceiveMsgDefaultResult(EVENT_S* pstEvent)
{
    // int32_t  s32Ret = 0;
    (void)pstEvent;
    // PARAM_GetMenuParam(&menu_param);
    // option_setuiLanguage(menu_param.Language.Current);
    ui_wrnmsg_update_type(MSG_EVENT_ID_SETTING_DEFUALT_FINISH);
    ui_winmng_startwin(UI_WRNMSG_PAGE, false);

    return 0;
}

static void save_option_to_param(void)
{
    int32_t  curIndx = s_page_info.cur_button_id;
    int32_t  *array = s_page_info.option_curid;
    MESSAGE_S Msg = {0};

    if (option[curIndx].cnt == 0 || option[curIndx].option_name == NULL) {
        CVI_LOGD("option param is invalid ! save_option_to_param failed !\n");
        return;
    }

    Msg.topic = EVENT_MODEMNG_SETTING;
    Msg.arg1 = option[curIndx].option_menu;
    Msg.arg2 = option[curIndx].option_name[array[curIndx]].param;
    UICOMM_SendAsyncMsg(&Msg, option_OnReceiveMsgResult);

    // PARAM_SetMenuParam(0, option[curIndx].option_menu, option[curIndx].option_name[array[curIndx]].param);
    widget_set_prop_bool(option_button[array[curIndx]], WIDGET_STATE_FOCUSED, TRUE);

    return;
}

static void option_find_defaut_id(int32_t  ini_param)
{
    int32_t  i;
    for (i = 0; i < option[s_page_info.cur_button_id].cnt; i++) {
        if (ini_param == option[s_page_info.cur_button_id].option_name[i].param) {
            s_page_info.option_curid[s_page_info.cur_button_id] = i;
            break;
        }
    }
    return;
}
/**
 * 读取参数并focus
**/
static ret_t default_focus_buttons_check(void)
{
    PARAM_DEVMNG_S dev_param = {0};
    PARAM_GetDevParam(&dev_param);
    PARAM_GetMenuParam(&menu_param);

    switch(s_page_info.cur_button_id)
    {
        case RESLUTION_BUTTON:
        {
            MEDIA_VIDEO_SIZE_E size;
            PARAM_GetVideoSizeEnum(menu_param.VideoSize.Current, &size);
            option_find_defaut_id(size);
            break;
        }
        case AUDIO_BUTTON:
            option_find_defaut_id(menu_param.AudioEnable.Current);
            break;
        case LOOPRECORDING_BUTTON:
            option_find_defaut_id(menu_param.VideoLoop.Current);
            break;
        case DATE_BUTTON:
            option_find_defaut_id(menu_param.Osd.Current);
            break;
        case GSENSOR_BUTTON:
            option_find_defaut_id(dev_param.Gsensor.enSensitity);
            break;
        case FATIGUEDRIVING_BUTTON:
            option_find_defaut_id(menu_param.FatigueDirve.Current);
            break;
        case SPEED_BUTTON:
            option_find_defaut_id(menu_param.SpeedStamp.Current);
            break;
        case GPS_BUTTON:
            option_find_defaut_id(menu_param.GPSStamp.Current);
            break;
        case SPEEDUNIT_BUTTON:
            option_find_defaut_id(menu_param.SpeedUnit.Current);
            break;
        case REARCAMMIRROR_BUTTON:
            option_find_defaut_id(menu_param.CamMirror.Current);
            break;
        case WIFI_BUTTON:
            option_find_defaut_id(dev_param.Wifi.Enable);
            break;
        case LANGUAGE_BUTTON:
            option_find_defaut_id(menu_param.Language.Current);
            break;
        case TIMEFOMAT_BUTTON:
            option_find_defaut_id(menu_param.TimeFormat.Current);
            break;
        case TIMEZONE_BUTTON:
            option_find_defaut_id(menu_param.TimeZone.Current);
            break;
        case FREQUENCY_BUTTON:
            option_find_defaut_id(menu_param.Frequence.Current);
            break;
        case CLICKTONE_BUTTON:
            option_find_defaut_id(menu_param.KeyTone.Current);
            break;
        case SCREENSAVER_BUTTON:
            option_find_defaut_id(menu_param.ScreenDormant.Current);
            break;
        case PARKING_BUTTON:
            option_find_defaut_id(menu_param.Parking.Current);
            break;
        default:
            break;
    }

    return RET_OK;
}
/**
 * 根据菜单号创建button
**/
static ret_t create_buttons_on_option_page(widget_t* win)
{
    int32_t  button_coordinate;
    int32_t  button_interval;
    int32_t  curIndx = s_page_info.cur_button_id;
    int32_t  *array = s_page_info.option_curid;

    if (option[curIndx].cnt == 0) {
        CVI_LOGD("This option has no button! stop create_buttons_on_option_page \n");
        return RET_FAIL;
    }

    button_interval = (WINDOW_WIGHT - (BUTTON_SIZE * option[curIndx].cnt)) /
                      (option[curIndx].cnt + 1);
    for (int32_t  i = 0; i < option[curIndx].cnt; i++) {
        button_coordinate = (i + 1) * button_interval + i * BUTTON_SIZE + 50;
        option_button[i] = button_create(win, button_coordinate, 110, BUTTON_SIZE ,BUTTON_SIZE);
        widget_set_tr_text(option_button[i], option[curIndx].option_name[i].name);
        widget_set_style_color(option_button[i], "normal:bg_color", 0xFFFFFFFF);
        widget_set_style_color(option_button[i], "focused:bg_color", 0XFFFF3600);
        widget_set_focusable(option_button[i], true);
    }
    default_focus_buttons_check();
    widget_set_prop_bool(option_button[array[curIndx]], WIDGET_STATE_FOCUSED, TRUE);

    CVI_LOGD("creat %d buttons success! \n", option[curIndx].cnt);
    creat_button_flag = true; // 防止多次创建button

    return RET_OK;
}

static ret_t init_optionwidget(void* ctx, const void* iter)
{
    ret_t ret;
    widget_t* win = WIDGET(iter);
    (void )ctx;
    if (creat_button_flag == false) {
        ret = create_buttons_on_option_page(win);
        if (ret == RET_FAIL) {
            CVI_LOGD("create_buttons_on_option_page failed %d \n", __LINE__);
        }
    }

    return RET_OK;
}
static void init_optionchildren_widget(widget_t* widget)
{
    widget_foreach(widget, init_optionwidget, widget);
}

static ret_t on_option_page_key_down(void* ctx, event_t* e)
{
    int32_t  curIndx = s_page_info.cur_button_id;
    int32_t  *array = s_page_info.option_curid;  // 存储当前坐标

    CVI_LOGD("on_option_page_key_down \n");
    key_event_t* evt = (key_event_t*)e;
    if(evt->alt == TRUE) {
        CVI_LOGD("Ignore long key press ! \n");
        return RET_OK;
    }

    if (ui_winmng_getwinisshow(UI_WRNMSG_PAGE) == true && evt->key != UI_KEY_POWER) {
        ui_winmng_finishwin(UI_WRNMSG_PAGE);
        return 0;
    }

    if(evt->alt == TRUE) {
        CVI_LOGD("Ignore long key press ! \n");
        return RET_OK;
    }
    if (evt->key == UI_KEY_POWER) {
        // 不保存退出
        s_page_info.close_flag = FALSE;
        ui_winmng_finishwin(UI_OPTION_PAGE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
        // 保存设置并退出
        save_option_to_param();
        return RET_STOP;
    } else if (evt->key == UI_KEY_UP) {
        // key up
        array[curIndx]--;
        if (array[curIndx] <= 0) {
            array[curIndx] = 0;
        }
        widget_set_prop_bool(option_button[array[curIndx]], WIDGET_STATE_FOCUSED, TRUE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        // key down
        array[curIndx]++;
        if (array[curIndx] >= option[curIndx].cnt - 1) {
            array[curIndx] = option[curIndx].cnt - 1;
        }
        widget_set_prop_bool(option_button[array[curIndx]], WIDGET_STATE_FOCUSED, TRUE);
        return RET_STOP;
    }
    return RET_OK;
}

ret_t ui_option_page_open(void* ctx, event_t* e)
{
    CVI_LOGD("ui_option_page_open\n");
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_option_page_key_down, win);
        init_optionchildren_widget(win);
        return RET_OK;
    }
    return RET_FAIL;
}

ret_t ui_option_page_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        creat_button_flag = false;
        memset(s_page_info.option_curid, 0, sizeof(s_page_info.option_curid));
    }
    return RET_OK;
}
/**

**/
static widget_t* label_head;
static widget_t* label_body;
static bool creat_nobutton_flag = false;
/**
 * 处理特殊菜单功能
**/
static ret_t deal_option_page_no_button(void)
{
    int32_t  ret;
    int32_t  s32Ret;
    int32_t  curIndx = s_page_info.cur_button_id;
    uint32_t msgtype = 0;

    msgtype = ui_wrnmsg_get_type();

    switch (curIndx)
    {
        case APP_BUTTON:
            break;
        case GPSINFO_BUTTON:
            break;
        case FORMAT_BUTTON:
            if (ui_format_cardstatus() != true) {
                return RET_OK;
            }
            if (msgtype != MSG_EVENT_ID_FORMAT_PROCESS) {
                MESSAGE_S Msg = {0};
                Msg.topic = EVENT_MODEMNG_CARD_FORMAT;
                s32Ret = MODEMNG_SendMessage(&Msg);
                if(0 != s32Ret) {
                    CVI_LOGE("MODEMNG_Format failed\n");
                    return RET_OK;
                }
                break;
            }
        case INFO_BUTTON:
            break;
        case DEFAULT_BUTTON:
            {
                MESSAGE_S Msg = {0};
                Msg.topic = EVENT_MODEMNG_SETTING;
                Msg.arg1 = PARAM_MENU_DEFAULT;
                UICOMM_SendAsyncMsg(&Msg, option_OnReceiveMsgDefaultResult);
                break;
            }
    }

    return RET_OK;
}
/**
 * 根据菜单号创建特殊菜单
**/
static ret_t creat_text_label(widget_t* win, char *head, char *body)
{
    // label head
    label_head = label_create(win, 286, 36, 260, 40);
    widget_set_style_color(label_head, "normal:bg_color", 0x00000000);
    widget_set_style_color(label_head, "normal:text_color", 0xFFFFFFFF);
    widget_set_style_int(label_head, "font_size", 30);
    widget_set_tr_text(label_head, head);
    // label body
    label_body = label_create(win, 116, 140, 600, 120);
    widget_set_style_color(label_body, "normal:bg_color", 0x00000000);
    widget_set_style_color(label_body, "normal:text_color", 0xFFFFFFFF);
    widget_set_style_int(label_body, "font_size", 30);
    label_set_length(label_body, -1);
    label_set_line_wrap(label_body, true);
    label_set_word_wrap(label_body, true);
    widget_set_tr_text(label_body, body);

    return RET_OK;
}
static ret_t creat_option_page_no_button(widget_t* win)
{
    int32_t  curIndx = s_page_info.cur_button_id;

    switch (curIndx)
    {
        case APP_BUTTON:
            break;
        case GPSINFO_BUTTON:
            break;
        case FORMAT_BUTTON:
            creat_text_label(win, "STRID_FORMAT_SD", option[curIndx].option_name[MENU_ITEM_NUM_DEFAULT].name);
            break;
        case INFO_BUTTON:
            creat_text_label(win, "STRID_ABOUT", option[curIndx].option_name[MENU_ITEM_NUM_DEFAULT].name);
            break;
        case DEFAULT_BUTTON:
            creat_text_label(win, "STRID_RESET_SYSTEM", option[curIndx].option_name[MENU_ITEM_NUM_DEFAULT].name);
            break;
    }
    creat_nobutton_flag = true;
    return RET_OK;
}
static ret_t on_identical_page_key_down(void* ctx, event_t* e)
{
    uint32_t msgtype = 0;
    CVI_LOGD("on_identical_page_key_down \n");

    key_event_t* evt = (key_event_t*)e;
    if(evt->alt == TRUE) {
        CVI_LOGD("Ignore long key press ! \n");
        return RET_OK;
    }
    msgtype = ui_wrnmsg_get_type();
    if (ui_winmng_getwinisshow(UI_WRNMSG_PAGE) == true) {
        if (msgtype == MSG_EVENT_ID_FORMAT_PROCESS) {
            return RET_STOP;
        }
        if (msgtype != MSG_EVENT_ID_FORMAT_PROCESS) {
            ui_winmng_finishwin(UI_WRNMSG_PAGE);
            return RET_STOP;
        }
    }

    if (evt->key == UI_KEY_POWER) {
        ui_winmng_finishwin(UI_IDE_OPTION_PAGE);
        return RET_STOP;
    } else if (evt->key == UI_KEY_MENU) {
        deal_option_page_no_button();
        return RET_STOP;
    } else if (evt->key == UI_KEY_UP) {
        return RET_STOP;
    } else if (evt->key == UI_KEY_DOWN) {
        return RET_STOP;
    }

    return RET_OK;
}
static ret_t init_identicalwidget(void* ctx, const void* iter)
{
    ret_t ret;
    widget_t* win = WIDGET(iter);
    (void )ctx;

    if (creat_nobutton_flag == false) {
        CVI_LOGD("creat_option_page_no_button \n");
        ret = creat_option_page_no_button(win);
        if (ret == RET_FAIL) {
            CVI_LOGD("creat_option_page_no_button failed %d \n", __LINE__);
        }
    }

    return RET_OK;
}
static void init_identical_widget(widget_t* widget)
{
    widget_foreach(widget, init_identicalwidget, widget);
}

ret_t ui_ide_option_page_open(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        CVI_LOGD("ui_ide_option_page_open \n");
        widget_on(window_manager(), EVT_UI_KEY_DOWN, on_identical_page_key_down, win);
        init_identical_widget(win);
        return RET_OK;
    }

    return RET_OK;
}

ret_t ui_ide_option_page_close(void* ctx, event_t* e)
{
    (void)e;
    widget_t* win = WIDGET(ctx);
    if (win) {
        creat_nobutton_flag = false;
    }
    return RET_OK;
}
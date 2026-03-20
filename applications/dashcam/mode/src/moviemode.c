#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "modeinner.h"
#include "param.h"
#include "media_osd.h"
#include "ledmng.h"
//#ifdef USE_GUI_AWTK
#include "ui_common.h"
//#endif

#ifdef ENABLE_VIDEO_MD
#include "cvi_videomd.h"
#endif
#include "zoomp.h"
#include "media_disp.h"

int32_t MODEMNG_ResetMovieMode(PARAM_CFG_S *Param)
{
    int32_t s32Ret = 0;
#ifdef SERVICES_QRCODE_ON
    MEDIA_QRCodeDeInit();
#endif

#if defined (ENABLE_VIDEO_MD)
    MEDIA_VIDEOMD_DeInit();
#endif

#ifdef SERVICES_ADAS_ON
    s32Ret = MEDIA_ADASDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Init MEDIA_ADASDeInit");
#endif

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = MEDIA_RecordSerDeInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record deinit");
    }
#else
    s32Ret = MEDIA_RecordSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record deinit");
#endif

#ifdef SERVICES_RTSP_ON
    s32Ret = MEDIA_RtspSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Deinit rtsp ser");
#endif
    s32Ret = MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Stop osd");
#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = MEDIA_StopVideoInTask();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StopVideoInTask init");
#endif

    s32Ret = MEDIA_StopAudioInTask();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StopAudioInTask deinit");

    s32Ret = MEDIA_AiDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Ai deinit");

    s32Ret = MEDIA_AencDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Aenc deinit");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Deinit liveview ser");
    #endif

    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Deinit video");

    s32Ret = MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MEDIA_VencDeInit fail");

    ZOOMP_Reset();

    s32Ret = PARAM_SetParam(Param);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Setting video param");

    s32Ret = MEDIA_SensorSwitchInit(NULL, NULL);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "VI Switch");

    s32Ret = MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Init video");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Liveview ser init");
    #endif

    s32Ret = UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UIAPP_Start");

    s32Ret = MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Venc init");

    s32Ret = MEDIA_AiInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Ai init");

    s32Ret = MEDIA_AencInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Aenc init");

    s32Ret = MEDIA_StartAudioInTask();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StartAudioInTask init");
#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = MEDIA_StartVideoInTask();/*rtsp rec get substream thread*/
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StartVideoInTask init");
#endif

    s32Ret = MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Start osd");

#ifdef ENABLE_ISP_PQ_TOOL
    MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = MEDIA_RecordSerInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record init");
    }
#else
    s32Ret = MEDIA_RecordSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record init");
#endif

#ifdef SERVICES_RTSP_ON
    s32Ret = MEDIA_RtspSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Init rtsp ser");
#endif

#ifdef SERVICES_ADAS_ON
    s32Ret = MEDIA_ADASInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Init MEDIA_ADASInit");
#endif

#if defined (ENABLE_VIDEO_MD)
    MEDIA_VIDEOMD_Init();
#endif

#ifdef SERVICES_QRCODE_ON
    MEDIA_QRCodeInit();
#endif

    return s32Ret;
}

//MOVIE MODE
int32_t MODEMNG_OpenMovieMode(void)
{
    int32_t s32Ret = 0;
    uint32_t is_mode_proc_with_sensor = 0;

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = WORK_MODE_MOVIE;

    s32Ret = MODEMNG_SetCurModeMedia(WORK_MODE_MOVIE);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL,"Set Cur Mode Media");

    s32Ret = MEDIA_SensorSwitchInit(NULL, NULL);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"VI Switch");

    s32Ret = MEDIA_VI_VPSS_Mode_Init();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "VI VPSS mode init");

    MODEMNG_GetSensorState(&is_mode_proc_with_sensor);
    if(is_mode_proc_with_sensor){
        /* 不显示sensor重新初始化导致的反应帧 */
        s32Ret = MEDIA_DISP_ClearBuf();
        MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"DISP clear");

        s32Ret = MEDIA_DISP_Pause();
        MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"DISP pause");

        s32Ret = MEDIA_SensorInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "VI init");
    } else {
        /* 不显示vpss重新初始化导致的异常帧 */
        s32Ret = MEDIA_DISP_Pause();
        MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"DISP pause");
    }

    s32Ret = MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Video init");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Liveview init");
    #endif

    s32Ret = UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UIAPP_Start");

    s32Ret = MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Venc init");

    s32Ret = MEDIA_AiInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Ai init");

    s32Ret = MEDIA_AencInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Aenc init");

    s32Ret = MEDIA_StartAudioInTask();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StartAudioInTask init");

    #ifdef SERVICES_SUBVIDEO_ON
    s32Ret = MEDIA_StartVideoInTask();/*rtsp rec get substream thread*/
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StartVideoInTask init");
    #endif

    s32Ret = MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Start osd");
#ifdef SERVICES_ADAS_ON
    s32Ret = MEDIA_ADASInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Init MEDIA_ADASInit");
#endif

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = MEDIA_RecordSerInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record init");
    }
#else
    s32Ret = MEDIA_RecordSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record init");
#endif

    if(is_mode_proc_with_sensor){
        usleep(500 * 1000);
    }
    is_mode_proc_with_sensor = 0;
    MODEMNG_SetSensorState(is_mode_proc_with_sensor);
    s32Ret = MEDIA_DISP_Resume();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "DISP resume");

#ifdef SERVICES_RTSP_ON
    s32Ret = MEDIA_RtspSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Rtsp init");
#endif

#ifdef ENABLE_VIDEO_MD
    MEDIA_VIDEOMD_Init();
#endif

#ifdef SERVICES_QRCODE_ON
    MEDIA_QRCodeInit();
#endif

    /** open movie ui */
    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_MOVIE;
    s32Ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Publish");

    return s32Ret;
}

int32_t MODEMNG_CloseMovieMode(void)
{
    int32_t s32Ret = 0;
    uint32_t is_mode_proc_with_sensor = 0;

    /*stop rec*/
    MODEMNG_StopRec();
#ifdef SERVICES_QRCODE_ON
    MEDIA_QRCodeDeInit();
#endif

#if defined (ENABLE_VIDEO_MD)
    MEDIA_VIDEOMD_DeInit();
#endif
#ifdef SERVICES_ADAS_ON
    s32Ret = MEDIA_ADASDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Init MEDIA_ADASDeInit");
#endif

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        s32Ret = MEDIA_RecordSerDeInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record deinit");
    }
#else
    s32Ret = MEDIA_RecordSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record deinit");
#endif

#ifdef SERVICES_RTSP_ON
    s32Ret = MEDIA_RtspSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Rtsp deinit");
#endif
    s32Ret = MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Stop osd");

#ifdef SERVICES_SUBVIDEO_ON
    s32Ret = MEDIA_StopVideoInTask();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL,"StopVideoInTask init");
#endif

    s32Ret = MEDIA_StopAudioInTask();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "StopAudioInTask deinit");

    s32Ret = MEDIA_AiDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Ai deinit");

    s32Ret = MEDIA_AencDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Aenc deinit");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Liveview deinit");
    #endif

    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Deinit video");

    s32Ret = MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MEDIA_VencDeInit fail");

    MODEMNG_GetSensorState(&is_mode_proc_with_sensor);
    if(is_mode_proc_with_sensor){
        s32Ret = MEDIA_SensorDeInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "VI deinit");
    }

    ZOOMP_DeInit();

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;

    /** close movie ui */
    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = WORK_MODE_MOVIE;
    s32Ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Publish");

    return s32Ret;
}

int32_t MODEMNG_StartEventRec(void)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    int32_t i = 0;
    int32_t isStartEventRec = 0;
    if((pstModeMngCtx->bInEmrRec != true)) {
        for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            if(SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval != 0){
                continue;
            }

            RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
            if (pstModeMngCtx->u32ModeState == MEDIA_MOVIE_STATE_VIEW) {
                RECORD_SERVICE_StartRecord(rs_hdl);
            }
        }
        for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            if(SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval != 0){
                continue;
            }

            RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
            RECORD_SERVICE_EventRecord(rs_hdl);
            isStartEventRec = 1;
        }
        if(isStartEventRec == 1){
            pstModeMngCtx->u32ModeState = MEDIA_MOVIE_STATE_REC;
            pstModeMngCtx->bInEmrRec = true;
        }
    }

    return 0;
}

int32_t MODEMNG_StartRec(void)
{
    uint32_t u32ModeState = 0;
    MODEMNG_GetModeState(&u32ModeState);
    if(u32ModeState == MEDIA_MOVIE_STATE_MENU){
        return 0;
    }
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    int32_t i = 0;
    if(pstModeMngCtx->u32ModeState != MEDIA_MOVIE_STATE_REC
    && pstModeMngCtx->u32ModeState != MEDIA_MOVIE_STATE_LAPSE_REC) {
        for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];

            if(SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval == 0) {
                RECORD_SERVICE_StartRecord(rs_hdl);
                pstModeMngCtx->u32ModeState = MEDIA_MOVIE_STATE_REC;
            } else {
                RECORD_SERVICE_StartTimelapseRecord(rs_hdl);
                pstModeMngCtx->u32ModeState = MEDIA_MOVIE_STATE_LAPSE_REC;
            }
        }
    } else {
        CVI_LOGI("Movie mode state is rec %u\n", pstModeMngCtx->u32ModeState);
    }
    if (pstModeMngCtx->bInParkingRec == true) {
        pstModeMngCtx->bInParkingRec = false;
        MODEMNG_StartEventRec();
    }
#ifdef CONFIG_LED_ON
    LEDMNG_Control(true);
#endif
    return 0;
}

int32_t MODEMNG_StopRec(void)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    int32_t i = 0;
    if ((pstModeMngCtx->u32ModeState == MEDIA_MOVIE_STATE_REC ||
        pstModeMngCtx->u32ModeState == MEDIA_MOVIE_STATE_LAPSE_REC)) {
        for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];

            if (SysMediaParams->SysServices.RecordParams[i].timelapse_recorder_gop_interval == 0) {
                RECORD_SERVICE_StopRecord(rs_hdl);
            } else {
                RECORD_SERVICE_StopTimelapseRecord(rs_hdl);
            }
        }
        pstModeMngCtx->u32ModeState = MEDIA_MOVIE_STATE_VIEW;
        pstModeMngCtx->bInEmrRec = false;
    }
#ifdef CONFIG_LED_ON
    LEDMNG_Control(false);
#endif
    return 0;
}

int32_t MODEMNG_StartPiv(void)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    int32_t s32Ret = 0;
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        char filename[FILEMNG_PATH_MAX_LEN] = {0};
        s32Ret = FILEMNG_GenerateFileName(i, FILEMNG_DIR_PHOTO, FILE_TYPE_SUFFIX[RECORD_SERVICE_FILE_TYPE_JPEG], filename, FILEMNG_PATH_MAX_LEN);
        if(0 == s32Ret){
            s32Ret = RECORD_SERVICE_PivCapture(rs_hdl, filename);
            RECORD_SERVICE_WaitPivFinish(rs_hdl);
        }
    }

    return s32Ret;
}


static int32_t MODEMNG_LiveViewAdjustFocus(uint32_t wndid , char* ratio)
{
#ifdef SERVICES_LIVEVIEW_ON
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    PARAM_WND_ATTR_S WndParam;
    PARAM_GetWndParam(&WndParam);

    if(wndid < WndParam.WndCnt && WndParam.Wnds[wndid].WndEnable == true){
        // LIVEVIEWMNG_AdjustFocus(wndid , ratio);
        for(int i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
            RECORD_SERVICE_AdjustFocus(rs_hdl, ratio);
        }
    }
#endif

    return 0;
}

int32_t MODEMNG_LiveViewUp(uint32_t viewwin)
{
#ifdef SERVICES_LIVEVIEW_ON
    PARAM_WND_ATTR_S WndParam;
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    PARAM_GetWndParam(&WndParam);

    if(viewwin < WndParam.WndCnt && WndParam.Wnds[viewwin].WndEnable == true && WndParam.Wnds[viewwin].SmallWndEnable == false){
        LIVEVIEWMNG_MoveUp(viewwin);
        LIVEVIEW_SERVICE_GetParam(SysMediaParams->SysServices.LvHdl, viewwin, &WndParam.Wnds[viewwin]);
        PARAM_SetWndParam(&WndParam);
    }
#endif

    return 0;
}

int32_t MODEMNG_LiveViewDown(uint32_t viewwin)
{
#ifdef SERVICES_LIVEVIEW_ON
    PARAM_WND_ATTR_S WndParam;
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    PARAM_GetWndParam(&WndParam);
    if(viewwin < WndParam.WndCnt && WndParam.Wnds[viewwin].WndEnable == true && WndParam.Wnds[viewwin].SmallWndEnable == false){
        LIVEVIEWMNG_MoveDown(viewwin);
        LIVEVIEW_SERVICE_GetParam(SysMediaParams->SysServices.LvHdl, viewwin, &WndParam.Wnds[viewwin]);
        PARAM_SetWndParam(&WndParam);
    }
#endif

    return 0;
}

static int32_t MODEMNG_APP_RTSP_INIT(uint32_t id, uint8_t *name)
{
#ifdef SERVICES_RTSP_ON
    MEDIA_APP_RTSP_Init(id, (char *)name);
#endif
    return 0;
}

static int32_t MODEMNG_APP_RTSP_SWITCH(uint32_t value, uint32_t id, uint8_t *name)
{
#ifdef SERVICES_RTSP_ON
    MEDIA_SwitchRTSPChanel(value, id, (char *)name);
#endif
    return 0;
}

static int32_t MODEMNG_APP_RTSP_DEINIT()
{
#ifdef SERVICES_RTSP_ON
    MEDIA_APP_RTSP_DeInit();
#endif
    return 0;
}

int32_t MODEMNG_LiveViewMirror(uint32_t CamID, bool en)
{
#ifdef SERVICES_LIVEVIEW_ON
    PARAM_WND_ATTR_S WndParam;
    PARAM_GetWndParam(&WndParam);
    uint32_t val = (CamID << 1) | (en & 0x1);
    LIVEVIEWMNG_Mirror(val);
    WndParam.Wnds[CamID].WndMirror = en;
    PARAM_SetWndParam(&WndParam);
#endif

    return 0;
}

static void MODEMNG_SetMediaVideoSize(uint32_t CamID, int32_t value)
{
    PARAM_CFG_S *param_ptr = (PARAM_CFG_S *)malloc(sizeof(PARAM_CFG_S));
    if(param_ptr == NULL){
        CVI_LOGE("malloc failed");
        return;
    }

    PARAM_GetParam(param_ptr);

    if((uint32_t)value < param_ptr->Menu.VideoSize.ItemCnt){
        CVI_LOGI("use item:%d", value);
    }else{
        CVI_LOGE("error video size item: %d !", value);
        return;
    }

    int32_t media_mode = MEDIA_Size2VideoMediaMode(param_ptr->Menu.VideoSize.Items[value].Value, -1,param_ptr->Menu.VideoSize.Items[value].Desc);
    if(media_mode < 0){
        CVI_LOGE("error media mode: %d with size(%d)", media_mode, param_ptr->Menu.VideoSize.Items[value].Value);
        return;
    }else{
        CVI_LOGI("switch to %d\n", media_mode);
    }
    param_ptr->CamCfg[CamID].CamMediaInfo.CurMediaMode = media_mode;
    param_ptr->WorkModeCfg.RecordMode.CamMediaInfo[CamID].CurMediaMode = media_mode;
    MODEMNG_ResetMovieMode(param_ptr);
    if(param_ptr != NULL){
        free(param_ptr);
        param_ptr = NULL;
    }

    PARAM_SetMenuParam(CamID, PARAM_MENU_VIDEO_SIZE, value);
}

static void MODEMNG_SetMediaRecEn(uint32_t value)
{
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        uint32_t val = (1 << i) & value;
        val = (val > 0)?1:0;
        PARAM_SetMenuParam(i, PARAM_MENU_REC_INX, val);
    }
}

static void MODEMNG_SetGsensorSetSensitity(uint32_t CamID, int32_t value)
{
#ifdef CONFIG_GSENSOR_ON
    GSENSORMNG_MenuSetSensitity(value);
    PARAM_SetMenuParam(CamID, PARAM_MENU_GSENSOR, value);
#endif
}

static void MODEMNG_SetMediaLoopTime(int32_t value)
{
    int32_t i = 0;

#ifdef ENABLE_ISP_PQ_TOOL
    bool en = false;
    MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        MEDIA_RecordSerDeInit();
    }
#else
    MEDIA_RecordSerDeInit();
#endif

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        RECORD_SERVICE_PARAM_S *RecParam = &MEDIA_GetCtx()->SysServices.RecordParams[i];
        switch (value) {
            case MEDIA_VIDEO_LOOP_1MIN:
                RecParam->recorder_split_interval_ms = 60000; //msec
                break;
            case MEDIA_VIDEO_LOOP_3MIN:
                RecParam->recorder_split_interval_ms = 180000; //msec
                break;
            case MEDIA_VIDEO_LOOP_5MIN:
                RecParam->recorder_split_interval_ms = 300000; //msec
                break;
            default:
                CVI_LOGE("value is invalid");
                break;
        }
        PARAM_SetMenuParam(i, PARAM_MENU_VIDEO_LOOP, value);
    }

#ifdef ENABLE_ISP_PQ_TOOL
    MEDIA_DUMP_GetSizeStatus(&en);
    if (en == false) {
        MEDIA_RecordSerInit();
    }
#else
    MEDIA_RecordSerInit();
#endif

}

static void MODEMNG_SetRecordMode(bool Enable_Lapse)
{
    uint32_t i = 0, s32Ret = 0;
    uint32_t j = 0, k = 0;
    // MAPI_VCODEC_E ori_codec = MAPI_VCODEC_H264;
    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    uint32_t BindVprocId = 0;
    uint32_t BindVprocChnId = 0;

    s32Ret = MEDIA_RecordSerDeInit();
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret,"Record deinit");
    s32Ret = MEDIA_VencDeInit();
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "MEDIA_VencDeInit fail");
    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret, "MEDIA_VideoDeInit fail");

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            for(j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++){
                if(pstParamCtx->pstCfg->CamCfg[i].CamMediaInfo.CurMediaMode == pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].MediaMode) {
                    for (k = 0; k < MAX_VENC_CNT; k++) {
                        if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].VencChnEnable == true) {
                            BindVprocId = pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].BindVprocId;
                            BindVprocChnId = pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].BindVprocChnId;
                            if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].VencId == 0) {
                                if (Enable_Lapse) {
                                    // ori_codec = pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].MapiVencAttr.codec;
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].MapiVencAttr.codec = MAPI_VCODEC_H264; //H264
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].sbm_enable = 0;
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VprocAttr.VprocGrpAttr[BindVprocId].VprocChnAttr[BindVprocChnId].VpssBufWrap.bEnable = 0;
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VprocAttr.VprocGrpAttr[BindVprocId].VprocChnAttr[BindVprocChnId].VprocChnEnable = true;
                                } else {
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].MapiVencAttr.codec = MAPI_VCODEC_H265;
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].sbm_enable = 1;
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VprocAttr.VprocGrpAttr[BindVprocId].VprocChnAttr[BindVprocChnId].VpssBufWrap.bEnable = 1;
                                    pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VprocAttr.VprocGrpAttr[BindVprocId].VprocChnAttr[BindVprocChnId].VprocChnEnable = false;
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    s32Ret = MEDIA_VideoInit();
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret,"vproc init");
    s32Ret = MEDIA_VencInit();
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret,"Venc init");
    s32Ret = MEDIA_RecordSerInit();
    MODEMNG_CHECK_EXPR_WITHOUT_RETURN(s32Ret,"Record init");

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            RECORD_SERVICE_PARAM_S *RecParam = &MEDIA_GetCtx()->SysServices.RecordParams[i];
            RECORD_SERVICE_HANDLE_T RecdHdl = MEDIA_GetCtx()->SysServices.RecordHdl[i];
            if (Enable_Lapse) {
                // pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].MapiVencAttr.codec = ori_codec;
                // pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VencAttr.VencChnAttr[k].sbm_enable = 1;
                // pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].VprocAttr.VprocGrpAttr[BindVprocId].VprocChnAttr[BindVprocChnId].VpssBufWrap.bEnable = 1;
                RecParam->timelapse_recorder_gop_interval = 1; //默认时间间隔1s
                RecParam->rec_mode = 1;
            } else {
                RecParam->timelapse_recorder_gop_interval = 0;
                RecParam->rec_mode = 0;
            }
            RECORD_SERVICE_UpdateParam(RecdHdl, RecParam);
        }
    }
}


static void LapseRecord_SetLapseTime(int32_t value)
{
    uint32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            RECORD_SERVICE_PARAM_S *RecParam = &MEDIA_GetCtx()->SysServices.RecordParams[i];
            RECORD_SERVICE_HANDLE_T RecdHdl = MEDIA_GetCtx()->SysServices.RecordHdl[i];
            RecParam->timelapse_recorder_gop_interval = value; //sec
            RECORD_SERVICE_UpdateParam(RecdHdl, RecParam);
        }
    }
}

void MODEMNG_SetMediaLapseTime(int32_t value)
{
    int32_t i = 0, gop = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            RECORD_SERVICE_PARAM_S *RecParam = &MEDIA_GetCtx()->SysServices.RecordParams[i];
            RECORD_SERVICE_HANDLE_T RecdHdl = MEDIA_GetCtx()->SysServices.RecordHdl[i];
            switch (value) {
                case MEDIA_VIDEO_LAPSETIME_OFF:
                    gop = 0;
                    break;
                case MEDIA_VIDEO_LAPSETIME_1S:
                    gop = 1; //sec
                    break;
                case MEDIA_VIDEO_LAPSETIME_2S:
                    gop = 2; //sec
                    break;
                case MEDIA_VIDEO_LAPSETIME_3S:
                    gop = 3; //sec
                    break;
                default:
                    CVI_LOGE("value is invalid");
                    break;
            }
            RecParam->timelapse_recorder_gop_interval = gop; //sec
            // MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
            if(gop > 0){
                RecParam->rec_mode = 1;
                // pstModeMngCtx->CurWorkMode = WORK_MODE_LAPSE;
            }else {
                RecParam->rec_mode = 0;
                // pstModeMngCtx->CurWorkMode = WORK_MODE_MOVIE;
            }
            RECORD_SERVICE_UpdateParam(RecdHdl, RecParam);
            PARAM_SetMenuParam(i, PARAM_MENU_LAPSE_TIME, value);
        }
    }
}

static void MODEMNG_SetMediaAudio(int32_t value)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    int32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
    #ifdef SERVICES_RTSP_ON
        RTSP_SERVICE_HANDLE_T rtsp_hdl = SysMediaParams->SysServices.RtspHdl[i];
    #endif
        switch (value) {
            case MEDIA_VIDEO_AUDIO_OFF:
                RECORD_SERVICE_StartMute(rs_hdl);
            #ifdef SERVICES_RTSP_ON
                RTSP_SERVICE_StartMute(rtsp_hdl);
            #endif
                break;
            case MEDIA_VIDEO_AUDIO_ON:
                RECORD_SERVICE_StopMute(rs_hdl);
            #ifdef SERVICES_RTSP_ON
                RTSP_SERVICE_StopMute(rtsp_hdl);
            #endif
                break;
            default:
                CVI_LOGE("value is invalid");
                break;
        }

        PARAM_SetMenuParam(i, PARAM_MENU_AUDIO_STATUS, value);
    }
}

static void MODEMNG_SetMediaOsd(int32_t value)
{
    uint32_t i = 0, z = 0;

    PARAM_MEDIA_OSD_ATTR_S OsdParam;
    PARAM_GetOsdParam(&OsdParam);
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for(int32_t j = 0; j < OsdParam.OsdCnt; j++){
            for(z = 0; z < OsdParam.OsdAttrs[j].u32DispNum; z++){
                if (OsdParam.OsdAttrs[j].astDispAttr[z].u32Batch == i &&
                    OsdParam.OsdAttrs[j].stContent.enType == MAPI_OSD_TYPE_TIME){
                    MAPI_OSD_Show(j, z, value);
                    OsdParam.OsdAttrs[j].astDispAttr[z].bShow = value;
                }
            }
        }
    }

    PARAM_CONTEXT_S *pstParamCtx = PARAM_GetCtx();
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        for (uint32_t j = 0; ((j < pstParamCtx->pstCfg->CamCfg[i].MediaModeCnt)&&(j < PARAM_MEDIA_CNT)); j++) {
            for (int32_t k = 0; k < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdCnt; k++) {
                for(z = 0; z < pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].u32DispNum; z++){
                    if (pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].stContent.enType == MAPI_OSD_TYPE_TIME){
                        pstParamCtx->pstCfg->CamCfg[i].MediaSpec[j].Osd.OsdAttrs[k].astDispAttr[z].bShow = value;
                    }
                }
            }
        }
    }

    PARAM_SetOsdParam(&OsdParam);
    PARAM_SetMenuParam(0, PARAM_MENU_OSD_STATUS, value);
}

static void MODEMNG_SetMenuSpeedStamp(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_SPEED_STAMP, value);
}

static void MODEMNG_SetMenuGpsStamp(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_GPS_STAMP, value);
}
#if CONFIG_PWM_ON
static void MODEMNG_SetMenuPwmBri(int32_t value)
{
    int32_t  pwm_level = 0, ret = 0;
    PARAM_SetMenuParam(0, PARAM_MENU_PWM_BRI_STATUS, value);
    switch(value) {
        case MEDIA_PWM_BRI_LOW:
            pwm_level = 10;
            break;
        case MEDIA_PWM_BRI_MID:
            pwm_level = 55;
            break;
        case MEDIA_PWM_BRI_HIGH:
            pwm_level = 95;
            break;
        default:
            break;
    }
    ret = HAL_PWM_Set_Percent(pwm_level);
    if(-1 == ret) {
        CVI_LOGD("error the rate, need input correct parm\n");
    }
    PARAM_PWM_S Param;
    ret = PARAM_GetPWMParam(&Param);
    if(ret == 0) {
        if (Param.Enable == false) {
            CVI_LOGD("pwm is disable\n");
            return;
        }
        Param.PWMCfg.duty_cycle = Param.PWMCfg.period * pwm_level / 100;
        HAL_SCREEN_COMM_SetLuma(HAL_SCREEN_IDXS_0, Param.PWMCfg);
        ret = PARAM_SetPWMParam(&Param);
        if(ret != 0) {
            CVI_LOGE("%s : PARAM_SetPWMParam failed\n",__func__);
        }
    } else {
        CVI_LOGE("%s : PARAM_GetPWMParam failed\n",__func__);
    }
}
#endif
static void MODEMNG_SetMenuFrequency(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_FREQUENCY, value);
    MEDIA_SetAntiFlicker();
}

static void CVI_MODEMNG_SetMenuTimeFormat(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_TIME_FORMAT, value);
}

static void MODEMNG_SetMenuCarNumStamp(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_CARNUM, value);
    MEDIA_UpdateCarNumOsd();
}

static void MODEMNG_SetMenuCarNumOsd(uint8_t *car_name)
{
    const uint8_t *CarNum = car_name;
    char *string_carnum_stamp = {0};
    string_carnum_stamp = (char *)CarNum;
    CVI_LOGD("in move mode :string_carnum_stamp= %s, car_name = %s\n", string_carnum_stamp, car_name);
    PARAM_SetOsdCarNameParam(string_carnum_stamp);
    MEDIA_UpdateCarNumOsd();
}

void MODEMNG_SetMenuMotionDet(int32_t value)
{
#ifdef ENABLE_VIDEO_MD
    for(int32_t i = 0; i < MAX_CAMERA_INSTANCES; i++){
        PARAM_SetMenuParam(i, PARAM_MENU_MOTION_DET, value);
        CVI_MOTION_DETECT_SetState(i, value);
    }
#endif
}

#if 0
void MODEMNG_SetMediaVencFormat(int32_t value)
{
    int32_t i = 0;
    MAPI_VENC_CHN_ATTR_T attr[MAX_CAMERA_INSTANCES];
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    if(SysMediaParams->SysModeAttr.ModeState == MEDIA_MOVIE_STATE_REC) {
        CVI_LOGE("Movie mode is rec not set value\n");
        return;
    }
    /* reset movie mode */
    MODE_CHECK_RET_NOR(MEDIA_RecordSerDeInit());
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            MAPI_VENC_HANDLE_T *VencHdl = &MEDIA_GetCtx()->SysHandle.venchdl[i][0];
            if (*VencHdl != NULL) {
                MAPI_VENC_GetAttr(*VencHdl, &attr[i]);
                int32_t rc = MAPI_VENC_DeinitChn(*VencHdl);
                if(rc != 0) {
                    CVI_LOGE("VENC_Deinit failed\n");
                    break;
                }
            } else {
                CVI_LOGE("venc is not init! \n");
            }
        }
    }
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            switch (value) {
                case MEDIA_VIDEO_VENCTYPE_H264:
                    attr[i].codec = MAPI_VCODEC_H264;
                    break;
                case MEDIA_VIDEO_VENCTYPE_H265:
                    attr[i].codec = MAPI_VCODEC_H265;
                    break;
                default:
                    CVI_LOGE("value is invalid");
                    break;
            }
            PARAM_SetMenuParam(i, PARAM_MENU_VIDEO_CODEC, value);
        }
    }
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            if(MAPI_VENC_InitChn(&MEDIA_GetCtx()->SysHandle.venchdl[i][0], &attr[i]) != 0) {
                CVI_LOGE("VENC Init Channel failed!\n");
                break;
            }
        }
    }

    MODE_CHECK_RET_NOR(MEDIA_RecordSerInit());

}
#else
void MODEMNG_SetMediaVencFormat(int32_t value)
{
    int32_t i = 0;
    /* reset movie mode */
    PARAM_CFG_S param;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (MEDIA_Is_CameraEnabled(i) == true) {
            PARAM_SetMenuParam(i, PARAM_MENU_VIDEO_CODEC, value);
        }
    }
    PARAM_GetParam(&param);
    MODEMNG_ResetMovieMode(&param);
}
#endif

int32_t isStopRec(int32_t arg1) {
    switch(arg1)
    {
        case PARAM_MENU_VIDEO_SIZE:
        case PARAM_MENU_VIDEO_LOOP:
        case PARAM_MENU_GSENSOR:
        case PARAM_MENU_VIDEO_CODEC:
        case PARAM_MENU_RECORD_MODE:
        case PARAM_MENU_LAPSE_TIME:
        case PARAM_MENU_LAPSE_RECORD_TIME:
        case PARAM_MENU_LANGUAGE:
        case PARAM_MENU_PARKING:
        case PARAM_MENU_KEYTONE:
        case PARAM_MENU_OSD_STATUS:
        case PARAM_MENU_SPEED_STAMP:
        case PARAM_MENU_GPS_STAMP:
        case PARAM_MENU_PWM_BRI_STATUS:
        case PARAM_MENU_VIEW_WIN_STATUS:
        case PARAM_MENU_TIME_FORMAT:
        case PARAM_MENU_DEFAULT:
        case PARAM_MENU_FREQUENCY:
        case PARAM_MENU_REARCAM_MIRROR:
        case PARAM_MENU_SCREENDORMANT:
        case PARAM_MENU_REC_INX:
        case PARAM_MENU_CARNUM:
        case PARAM_MENU_SET_CARNUM_OSD:
            return 1;
        case PARAM_MENU_WIFI_STATUS:
        case PARAM_MENU_AUDIO_STATUS:
            return  0;
        default:
            CVI_LOGE("not support param type(%d)\n\n", arg1);
            return -1;
    }
}
static void MODEMNG_SetScreenDormant(int32_t value)
{
    uint32_t s32Ret = 0, time_sec = 0;
    TIMEDTASK_ATTR_S s_ScreenDormantAttr = {0};
    switch (value)
    {
    case MENU_SCREENDORMANT_OFF:
        time_sec = 0;
        break;
    case MENU_SCREENDORMANT_1MIN:
        time_sec = 60;
        break;
    case MENU_SCREENDORMANT_3MIN:
        time_sec = 180;
        break;
    case MENU_SCREENDORMANT_5MIN:
        time_sec = 300;
        break;
    default:
        time_sec = 0;
        break;
    }
    s_ScreenDormantAttr.u32Time_sec = time_sec;
    s_ScreenDormantAttr.bEnable = (bool)(s_ScreenDormantAttr.u32Time_sec > 0U ? true : false);
    s_ScreenDormantAttr.periodic = false;

    PWRCTRL_TASK_E enPwrCtrlType = PWRCTRL_TASK_SCREENDORMANT;
    s32Ret = POWERCTRL_SetTaskAttr(enPwrCtrlType, &s_ScreenDormantAttr);
    if (s32Ret) {
        CVI_LOGE("SetTaskAttr screen_dormant failed");
        return;
    }

    PARAM_SetMenuParam(0, PARAM_MENU_SCREENDORMANT, value);
}

static void MODEMNG_SetRearCamMirror(uint32_t camid, int32_t value)
{
    MODEMNG_LiveViewMirror(camid, value);
    PARAM_SetMenuParam(camid, PARAM_MENU_REARCAM_MIRROR, value);
}

static void MODEMNG_SetMenuLanguage(int32_t value)
{
    int32_t s32Ret = 0;
    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_SETTING_LANGUAGE;
    stEvent.arg1 = value;
    s32Ret = EVENTHUB_Publish(&stEvent);
    if (s32Ret != 0) {
        CVI_LOGE("Publish EVENT_MODEMNG_SETTING_LANGUAGE failed !\n");
        return;
    }
    PARAM_SetMenuParam(0, PARAM_MENU_LANGUAGE, value);
}

static void MODEMNG_SetMenuKeyTone(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_KEYTONE, value);
}

static void MODEMNG_SetMenuParking(int32_t value)
{
    PARAM_SetMenuParam(0, PARAM_MENU_PARKING, value);
}

#ifdef WIFI_ON
static void MODEMNG_SetMenuWifiStatus(int32_t value)
{
    PARAM_WIFI_S WifiParam = {0};
    int32_t s32Ret = 0;
    s32Ret = PARAM_GetWifiParam(&WifiParam);
    WifiParam.Enable = value;
    if (true == value) {
        s32Ret = WIFIMNG_Start(WifiParam.WifiCfg, WifiParam.WifiDefaultSsid);
    } else {
        s32Ret = WIFIMNG_Stop();
    }
    if (s32Ret != 0) {
        CVI_LOGE("MODEMNG_SetMenuWifiStatus failed !\n");
        return;
    }
    PARAM_SetWifiParam(&WifiParam);
}
#endif

int32_t MODEMNG_SetDefaultParam(void)
{
#ifdef SERVICES_LIVEVIEW_ON
    int32_t ret;
    PARAM_CFG_S param;
    PARAM_CFG_S param_stash = {0};

    // get AHD param
    PARAM_GetParam(&param);
    param_stash.MediaComm.Window.WndCnt = param.MediaComm.Window.WndCnt;
    for(uint32_t i = 0; i < param_stash.MediaComm.Window.WndCnt; i++){
        param_stash.MediaComm.Window.Wnds[i].WndEnable = param.MediaComm.Window.Wnds[i].WndEnable;
        param_stash.MediaComm.Window.Wnds[i].SmallWndEnable = param.MediaComm.Window.Wnds[i].SmallWndEnable;
    }

    // load default pararm
    ret = PARAM_LoadDefaultParamFromFlash(&param);
    MODEMNG_CHECK_RET(ret,MODE_EINVAL,"reset param");

    for(uint32_t i = 0; i < param.MediaComm.Window.WndCnt; i++){
        param.MediaComm.Window.Wnds[i].WndEnable = param_stash.MediaComm.Window.Wnds[i].WndEnable;
        param.MediaComm.Window.Wnds[i].SmallWndEnable = param_stash.MediaComm.Window.Wnds[i].SmallWndEnable;
    }

    // save param
    PARAM_SetParam(&param);
    PARAM_GetParam(&param);
    PARAM_SaveParam();

    MODEMNG_SetScreenDormant(param.Menu.ScreenDormant.Current);
#ifdef CONFIG_GSENSOR_ON
    GSENSORMNG_MenuSetSensitity(param.DevMng.Gsensor.enSensitity);
#endif

    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_SETTING_LANGUAGE;
    stEvent.arg1 = param.Menu.Language.Current;
    ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(ret,MODE_EINVAL,"set ui language");

    ret = MODEMNG_ResetMovieMode(&param);
    MODEMNG_CHECK_RET(ret,MODE_EINVAL,"reset movie mode");

    CVI_LOGI("reset param finish\n");
#endif

    return 0;
}

int32_t MODEMNG_StartMemoryBufferRec(void)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    int32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        RECORD_SERVICE_StartMemoryBuffer(rs_hdl);
    }

    return 0;
}

int32_t MODEMNG_StopMemoryBufferRec(void)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    int32_t i = 0;

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        RECORD_SERVICE_StopMemoryBuffer(rs_hdl);
    }

    return 0;
}

void MODEMNG_MovieSwitchSensor(void)
{
    uint32_t cam_id = 0;
    PARAM_MEDIA_SNS_ATTR_S sns_attr = {0};
    MAPI_VCAP_SENSOR_ATTR_T sns_chn_attr = {0};
    PARAM_MEDIA_VACP_ATTR_S vcap_attr = {0};
    MAPI_VCAP_CHN_ATTR_T vcap_chn_attr = {0};

    for (cam_id = 0; cam_id < MAX_CAMERA_INSTANCES; cam_id++) {
        if (!MEDIA_Is_CameraEnabled(cam_id)) {
            continue;
        }

        MODEMNG_SetSensorState(1);

        MODEMNG_CloseMovieMode();

        /* 交换前后摄参数 */
        PARAM_GetSensorParam(cam_id, &sns_attr);
        memcpy(&sns_chn_attr, &sns_attr.SnsSwitchChnAttr, sizeof(MAPI_VCAP_SENSOR_ATTR_T));
        memcpy(&sns_attr.SnsSwitchChnAttr, &sns_attr.SnsChnAttr, sizeof(MAPI_VCAP_SENSOR_ATTR_T));
        memcpy(&sns_attr.SnsChnAttr, &sns_chn_attr, sizeof(MAPI_VCAP_SENSOR_ATTR_T));
        PARAM_SetSensorParam(cam_id, &sns_attr);

        PARAM_GetVcapParam(cam_id, &vcap_attr);
        memcpy(&vcap_chn_attr, &vcap_attr.VcapSwitchChnAttr, sizeof(MAPI_VCAP_CHN_ATTR_T));
        memcpy(&vcap_attr.VcapSwitchChnAttr, &vcap_attr.VcapChnAttr, sizeof(MAPI_VCAP_CHN_ATTR_T));
        memcpy(&vcap_attr.VcapChnAttr, &vcap_chn_attr, sizeof(MAPI_VCAP_CHN_ATTR_T));
        PARAM_SetVcapParam(cam_id, &vcap_attr);

        /* 保存参数,切换拍照录像的时候保持不变 */
        MEDIA_SensorSwitchInit(&sns_attr, &vcap_attr);

        PARAM_WND_ATTR_S *WndParam = &PARAM_GetCtx()->pstCfg->MediaComm.Window;
        if(WndParam->Wnds[0].WndMirror == 0){
            CVI_LOGI("switch %d to %d", WndParam->Wnds[0].WndMirror, 1);
            WndParam->Wnds[0].WndMirror = 1;
        }else{
            CVI_LOGI("switch %d to %d", WndParam->Wnds[0].WndMirror, 0);
            WndParam->Wnds[0].WndMirror = 0;
        }
        PARAM_SetWndParam(WndParam);

        /* facep osd mirror */
        PARAM_FACEP_ATTR_S FaceParam = {0};
        PARAM_GetFaceParam(&FaceParam);
        FaceParam.sv_param[0].osd_mirror = !FaceParam.sv_param[0].osd_mirror;
        PARAM_SetFaceParam(&FaceParam);

        /* save to bin */
        PARAM_SetSaveFlg();

        MODEMNG_OpenMovieMode();
    }
}

static int32_t MODEMNG_SetFlashLed(int32_t value)
{
    int32_t s32Ret = 0;
    int32_t i = 0;
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();

    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        RECORD_SERVICE_HANDLE_T rs_hdl = SysMediaParams->SysServices.RecordHdl[i];
        CVI_LOGI("video flash: %d", value);
        if(value) {
            RECORD_SERVICE_SetFlashLed(rs_hdl, RECORD_SERVICE_FLASH_LED_MODE_NP);
        } else {
            RECORD_SERVICE_SetFlashLed(rs_hdl, RECORD_SERVICE_FLASH_LED_MODE_NC);
        }
    }

    return s32Ret;
}

static int32_t MODEMNG_SetVideoEffect(uint32_t value)
{
    int32_t s32Ret = 0;
    int32_t i = 0;
    MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
    PARAM_MENU_S menu_param = {0};
    char effect_bin_path[64] = "/mnt/data/bin/cvi_sdr_bin";

    PARAM_GetMenuParam(&menu_param);

    CVI_LOGI("set photo effect to %d\n", value);
    if (value >= menu_param.IspEffect.ItemCnt) {
        CVI_LOGE("not support photo effect(%d)\n", value);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        if(value){
            memset(effect_bin_path, 0, sizeof(effect_bin_path));
            snprintf(effect_bin_path, sizeof(effect_bin_path),
                    "/mnt/data/bin/%s", menu_param.IspEffect.Items[value].Desc);
        }else {
            /* use default effect bin */
        }

        s32Ret = MAPI_VCAP_SetEffect(Syshdl->sns[i], effect_bin_path);
        if (s32Ret != MAPI_SUCCESS) {
            CVI_LOGE("MAPI_VCAP_SetEffect failed\n");
            return -1;
        }

        PARAM_SetMenuParam(i, PARAM_MENU_ISP_EFFECT, value);
    }

    return s32Ret;
}

static int32_t MODEMNG_RecStatesSettingMsgProc(MESSAGE_S* pstMsg)
{
    int32_t s32Ret = 0;

    switch (pstMsg->arg1)
    {
        case PARAM_MENU_VIDEO_SIZE:
            MODEMNG_SetMediaVideoSize(0, pstMsg->arg2);
            break;

        case PARAM_MENU_VIDEO_LOOP:
            MODEMNG_SetMediaLoopTime(pstMsg->arg2);
            break;

        case PARAM_MENU_VIDEO_EFFECT:
            MODEMNG_SetVideoEffect(pstMsg->arg2);
            break;

        case PARAM_MENU_SENSOR_SWITCH:
            MODEMNG_MovieSwitchSensor();
            break;

        case PARAM_MENU_FLASH_LED:
            MODEMNG_SetFlashLed(pstMsg->arg2);
            break;

        case PARAM_MENU_GSENSOR:
            MODEMNG_SetGsensorSetSensitity(0, pstMsg->arg2);
            break;

        case PARAM_MENU_VIDEO_CODEC:
            MODEMNG_SetMediaVencFormat(pstMsg->arg2);
            break;

        case PARAM_MENU_RECORD_MODE:
            MODEMNG_SetRecordMode(pstMsg->arg2);
            break;

        case PARAM_MENU_LAPSE_TIME:
            MODEMNG_SetMediaLapseTime(pstMsg->arg2);
            break;

        case PARAM_MENU_LAPSE_RECORD_TIME:
            LapseRecord_SetLapseTime(pstMsg->arg2);
            break;

        case PARAM_MENU_WIFI_STATUS:
#ifdef WIFI_ON
            MODEMNG_SetMenuWifiStatus(pstMsg->arg2);
#endif
            break;

        case PARAM_MENU_AUDIO_STATUS:
            MODEMNG_SetMediaAudio(pstMsg->arg2);
            break;

        case PARAM_MENU_LANGUAGE:
            MODEMNG_SetMenuLanguage(pstMsg->arg2);
            break;

        case PARAM_MENU_PARKING:
            MODEMNG_SetMenuParking(pstMsg->arg2);
            break;

        case PARAM_MENU_KEYTONE:
            MODEMNG_SetMenuKeyTone(pstMsg->arg2);
            break;

        case PARAM_MENU_OSD_STATUS:
            MODEMNG_SetMediaOsd(pstMsg->arg2);
            break;

        case PARAM_MENU_SPEED_STAMP:
            MODEMNG_SetMenuSpeedStamp(pstMsg->arg2);
            break;

        case PARAM_MENU_GPS_STAMP:
            MODEMNG_SetMenuGpsStamp(pstMsg->arg2);
            break;

        case PARAM_MENU_PWM_BRI_STATUS:
        #if CONFIG_PWM_ON
            MODEMNG_SetMenuPwmBri(pstMsg->arg2);
        #endif
            break;

        case PARAM_MENU_VIEW_WIN_STATUS:
            MODEMNG_LiveViewSwitch(pstMsg->arg2);
            break;

        case PARAM_MENU_TIME_FORMAT:
            CVI_MODEMNG_SetMenuTimeFormat(pstMsg->arg2);
            break;

        case PARAM_MENU_DEFAULT:
            MODEMNG_SetDefaultParam();
            break;

        case PARAM_MENU_FREQUENCY:
            MODEMNG_SetMenuFrequency(pstMsg->arg2);
            break;

        case PARAM_MENU_REARCAM_MIRROR:
            MODEMNG_SetRearCamMirror(pstMsg->aszPayload[0], pstMsg->arg2);
            break;

        case PARAM_MENU_SCREENDORMANT:
            MODEMNG_SetScreenDormant(pstMsg->arg2);
            break;

        case PARAM_MENU_REC_INX:
            MODEMNG_SetMediaRecEn(pstMsg->arg2);
            break;
        case PARAM_MENU_CARNUM: //stamp
            MODEMNG_SetMenuCarNumStamp(pstMsg->arg2);
            break;

        case PARAM_MENU_SET_CARNUM_OSD:
            MODEMNG_SetMenuCarNumOsd(pstMsg->aszPayload);
            break;
        default:
            CVI_LOGE("not support param type(%d)\n\n", pstMsg->arg1);
            return -1;
    }

    return s32Ret;
}

/** Movie Mode message process */
int32_t MODEMNG_MovieModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    /** check parameters */
    STATE_S* pstStateAttr = (STATE_S*)pvArg;
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return PROCESS_MSG_RESULTE_OK;
    }

    CVI_LOGI("MODEMNG_MovieModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));
    (void)pstStateAttr;

    switch (pstMsg->topic) {
    case EVENT_STORAGEMNG_DEV_UNPLUGED: {
        MODEMNG_StopRec();
        // MODEMNG_StartMemoryBufferRec();
        return PROCESS_MSG_UNHANDLER;
            }
        case EVENT_MODEMNG_POWEROFF:
            {
                MODEMNG_StopRec();
                return PROCESS_MSG_UNHANDLER;
            }
        case EVENT_SENSOR_PLUG_STATUS:
        {
#ifdef SERVICES_LIVEVIEW_ON
            PARAM_CFG_S Param;
            PARAM_GetParam(&Param);
            MODEMNG_StopRec();
            int32_t snsid = pstMsg->aszPayload[1];
            int32_t mode = pstMsg->aszPayload[0];
            uint32_t lastmode = Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode;
            if (SENSOR_PLUG_IN == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug in\n", snsid);
                CVI_LOGD("sensor %d resolution=%d\n", snsid, mode);
                Param.CamCfg[snsid].CamMediaInfo.CurMediaMode = MEDIA_Res2RecordMediaMode(mode);
                Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode = MEDIA_Res2RecordMediaMode(mode);
                Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode = MEDIA_Res2PhotoMediaMode(mode);
                Param.CamCfg[snsid].CamEnable = true;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = true;
                if(lastmode != Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode){
                    MAPI_VCAP_SetAhdMode(snsid, mode);
                #ifndef RESET_MODE_AHD_HOTPLUG_ON
                    MODEMNG_ResetMovieMode(&Param);
                #endif
                }
            } else if (SENSOR_PLUG_OUT == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug out\n", snsid);
                Param.CamCfg[snsid].CamEnable = false;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = false;
            }
        #ifdef RESET_MODE_AHD_HOTPLUG_ON
            MODEMNG_ResetMovieMode(&Param);
        #endif
            PARAM_SetParam(&Param);
            MODEMNG_MonitorStatusNotify(pstMsg);
#endif
            return PROCESS_MSG_RESULTE_OK;
        }
        case EVENT_RECMNG_EVENTREC_END:
        case EVENT_RECMNG_EMRREC_END:
            {
                MODEMNG_SetEmrState(false);
                return PROCESS_MSG_UNHANDLER;
            }
        case EVENT_MODEMNG_START_REC:
            {
                MODEMNG_StartRec();
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_STOP_REC:
            {
                MODEMNG_StopRec();
                if (MEDIA_MOVIE_STATE_MENU == pstMsg->arg1) {
                    MODEMNG_SetModeState(MEDIA_MOVIE_STATE_MENU);
                }
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_SETTING:
            {
                if (pstModeMngCtx->u32ModeState == MEDIA_MOVIE_STATE_REC && isStopRec(pstMsg->arg1)) {
                    MODEMNG_StopRec();
                }

                MODEMNG_RecStatesSettingMsgProc(pstMsg);
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_RTSP_INIT:
            {
                MODEMNG_APP_RTSP_INIT(pstMsg->arg1, pstMsg->aszPayload);
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_RTSP_SWITCH:
            {
                MODEMNG_APP_RTSP_SWITCH(pstMsg->arg1, pstMsg->arg2, pstMsg->aszPayload);
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_RTSP_DEINIT:
            {
                MODEMNG_APP_RTSP_DEINIT();
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_LIVEVIEW_UPORDOWN:
            {
                PARAM_MENU_S param;
                PARAM_GetMenuParam(&param);
                if (0 == pstMsg->arg1) {
                    MODEMNG_LiveViewDown(param.ViewWin.Current);
                } else {
                    MODEMNG_LiveViewUp(param.ViewWin.Current);
                }
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_LIVEVIEW_ADJUSTFOCUS:
            {
                MODEMNG_LiveViewAdjustFocus(pstMsg->arg1 , (char *)pstMsg->aszPayload);
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_START_PIV:
            {
                MODEMNG_StartPiv();
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_START_EMRREC:
            {
                MODEMNG_StartEventRec();
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_SENSOR_STATE:
            {
                MODEMNG_SetSensorState(pstMsg->arg1);
                return PROCESS_MSG_RESULTE_OK;
            }
        default:
            return PROCESS_MSG_UNHANDLER;
            break;
        }

    return PROCESS_MSG_RESULTE_OK;
}

int32_t MODEMNG_MovieStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stMovieState =
    {
        WORK_MODE_MOVIE,
        MODEEMNG_STATE_REC,
        MODEMNG_OpenMovieMode,
        MODEMNG_CloseMovieMode,
        MODEMNG_MovieModeMsgProc,
        NULL
    };
    stMovieState.argv = &stMovieState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stMovieState,
                              (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add NormalRec state");
    return s32Ret;
}

/** deinit movie mode */
int32_t MODEMNG_MovieStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODEMNG_CloseMovieMode();
    return s32Ret;
}

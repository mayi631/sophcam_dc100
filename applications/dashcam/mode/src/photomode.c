#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "media_init.h"
#include "modeinner.h"
#include "param.h"
#include "ledmng.h"
//#ifdef USE_GUI_AWTK
#include "ui_common.h"
//#endif

#include "photo_service.h"
#include "zoomp.h"
#include "media_disp.h"

void smile_callback_fun(void);

int32_t MODEMNG_ResetPhotoMode(PARAM_CFG_S *Param)
{
    int32_t s32Ret = 0;

    #ifdef SERVICES_FACEP_ON
    FACEP_SERVICE_Pause();
    #endif

    s32Ret = MEDIA_PhotoSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record deinit");

    s32Ret = MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Stop osd");

    s32Ret = MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "Venc  deinit");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Liveview deinit");
    #endif

    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Video deinit");

    ZOOMP_Reset();

    s32Ret = PARAM_SetParam(Param);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Setting video param");

    s32Ret = MEDIA_SensorSwitchInit(NULL, NULL);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "VI Switch");

    s32Ret = MEDIA_VideoInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Video init");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Liveview init");
    #endif

    s32Ret = MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Venc init");

    s32Ret = UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UIAPP_Start");

    s32Ret = MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Start osd");

    s32Ret = MEDIA_PhotoSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Photo init");

    #ifdef SERVICES_FACEP_ON
    FACEP_SERVICE_Resume();
    #endif

    return s32Ret;
}

//PHOTO MODE
int32_t MODEMNG_OpenPhotoMode(void)
{
    int32_t s32Ret = 0;
    uint32_t is_mode_proc_with_sensor = 0;
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = WORK_MODE_PHOTO;

    s32Ret = MODEMNG_SetCurModeMedia(WORK_MODE_PHOTO);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Set Cur Mode Media");

    s32Ret = MEDIA_SensorSwitchInit(NULL, NULL);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"VI Switch");

    s32Ret = MEDIA_VI_VPSS_Mode_Init();
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"VI VPSS mode init");

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

    s32Ret = MEDIA_VencInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Venc init");

    s32Ret = UIAPP_Start();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "UIAPP_Start");

    /* sensor没有重新初始化，提前恢复DISP，避免卡顿 */
    if(!is_mode_proc_with_sensor){
        usleep(50 * 1000);
        s32Ret = MEDIA_DISP_Resume();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "DISP resume");
    }

    /* 600ms */
    s32Ret = MEDIA_StartOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Start osd");

    s32Ret = MEDIA_PhotoSerInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Photo init");

    #ifdef SERVICES_FACEP_ON
    s32Ret = MEDIA_FacepInit(smile_callback_fun);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Facep init");
    #endif

    /* sensor重新初始化，延迟恢复DISP，去除反应帧 */
    if(is_mode_proc_with_sensor){
        usleep(500 * 1000);
        s32Ret = MEDIA_DISP_Resume();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "DISP resume");
    }
    is_mode_proc_with_sensor = 0;
    MODEMNG_SetSensorState(is_mode_proc_with_sensor);

    /** open photo ui */
    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODEOPEN;
    stEvent.arg1 = WORK_MODE_PHOTO;
    s32Ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Publish");

    return s32Ret;
}

int32_t MODEMNG_ClosePhotoMode(void)
{
    int32_t s32Ret = 0;
    uint32_t is_mode_proc_with_sensor = 0;

    #ifdef SERVICES_FACEP_ON
    s32Ret = MEDIA_FacepDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Facep deinit");
    #endif

    s32Ret = MEDIA_PhotoSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Record deinit");

    s32Ret = MEDIA_StopOsd();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Stop osd");

    #ifdef SERVICES_LIVEVIEW_ON
    s32Ret = MEDIA_LiveViewSerDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Liveview deinit");
    #endif

    MODEMNG_GetSensorState(&is_mode_proc_with_sensor);
    if(is_mode_proc_with_sensor){
        s32Ret = MEDIA_SensorDeInit();
        MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "VI deinit");
    }

    s32Ret = MEDIA_VideoDeInit();
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "Video deinit");

    s32Ret = MEDIA_VencDeInit();
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "Venc deinit");

    ZOOMP_DeInit();

    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    pstModeMngCtx->CurWorkMode = WORK_MODE_BUTT;

    /** close photo ui */
    EVENT_S stEvent;
    stEvent.topic = EVENT_MODEMNG_MODECLOSE;
    stEvent.arg1 = WORK_MODE_PHOTO;
    s32Ret = EVENTHUB_Publish(&stEvent);
    MODEMNG_CHECK_RET(s32Ret,MODE_EINVAL,"Publish");

    return s32Ret;
}

int32_t MODEMNG_StartPhoto(void)
{
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    int32_t i = 0;
    int32_t s32Ret = 0;
    PHOTO_SERVICE_FLASH_LED_MODE_E flash_mode = 0;
    PARAM_MENU_S menu_param = {0};
    for(i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        PHOTO_SERVICE_HANDLE_T ph_hdl = SysMediaParams->SysServices.PhotoHdl[i];
        char filename[FILEMNG_PATH_MAX_LEN] = {0};
        s32Ret = FILEMNG_GenerateFileName(i, FILEMNG_DIR_PHOTO, ".jpg", filename, FILEMNG_PATH_MAX_LEN);
        if(0 == s32Ret){
            PARAM_GetMenuParam(&menu_param);
            flash_mode = (PHOTO_SERVICE_FLASH_LED_MODE_E)menu_param.FlashLed.Items[menu_param.FlashLed.Current].Value;
            if(flash_mode == PHOTO_SERVICE_FLASH_LED_MODE_NP ||
                flash_mode == PHOTO_SERVICE_FLASH_LED_MODE_AUTO){
                /* pre flash */
                s32Ret = PHOTO_SERVICE_SetFlashLed(ph_hdl, flash_mode);
                if(s32Ret == 0){
                    usleep(165 * 1000);
                    PHOTO_SERVICE_SetFlashLed(ph_hdl, PHOTO_SERVICE_FLASH_LED_MODE_NC);
                    usleep(396 * 1000);
                    /* main flash */
                    PHOTO_SERVICE_SetFlashLed(ph_hdl, PHOTO_SERVICE_FLASH_LED_MODE_NP);
                    /* wait to stable */
                    usleep(100 * 1000);
                }
            }

            s32Ret = PHOTO_SERVICE_PivCapture(ph_hdl, filename);
            PHOTO_SERVICE_WaitPivFinish(ph_hdl);

            if(flash_mode == PHOTO_SERVICE_FLASH_LED_MODE_NP ||
                flash_mode == PHOTO_SERVICE_FLASH_LED_MODE_AUTO){
                PHOTO_SERVICE_SetFlashLed(ph_hdl, PHOTO_SERVICE_FLASH_LED_MODE_NC);
            }
        }
    }

    return s32Ret;
}

static int32_t MODEMNG_AdjustFocus(uint32_t wndid , char* ratio)
{
#ifdef SERVICES_LIVEVIEW_ON
    MEDIA_PARAM_INIT_S *SysMediaParams = MEDIA_GetCtx();
    PARAM_WND_ATTR_S WndParam;
    PARAM_GetWndParam(&WndParam);

    if(wndid < WndParam.WndCnt && WndParam.Wnds[wndid].WndEnable == true){
        LIVEVIEWMNG_AdjustFocus(wndid , ratio);
        for(int i = 0; i < MAX_CAMERA_INSTANCES; i++) {
            if (!MEDIA_Is_CameraEnabled(i)) {
                continue;
            }

            PHOTO_SERVICE_HANDLE_T ph_hdl = SysMediaParams->SysServices.PhotoHdl[i];
            PHOTO_SERVICE_AdjustFocus(ph_hdl, ratio);
        }
    }

#endif

    return 0;
}

int32_t MODEMNG_PLiveViewUp(uint32_t viewwin)
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

int32_t MODEMNG_PLiveViewDown(uint32_t viewwin)
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

void MODEMNG_PhotoSetMediaSize(uint32_t CamID, int32_t value)
{
    PARAM_CFG_S *param_ptr = (PARAM_CFG_S *)malloc(sizeof(PARAM_CFG_S));
    if(param_ptr == NULL){
        CVI_LOGE("malloc failed");
        return;
    }

    PARAM_GetParam(param_ptr);

    if((uint32_t)value < param_ptr->Menu.PhotoSize.ItemCnt){
        CVI_LOGI("use item:%d", value);
    }else{
        CVI_LOGE("error photo size item: %d !", value);
        return;
    }

    int32_t media_mode = MEDIA_Size2PhotoMediaMode(param_ptr->Menu.PhotoSize.Items[value].Value, -1);
    if(media_mode < 0){
        CVI_LOGE("error media mode: %d with size(%d)", media_mode, param_ptr->Menu.PhotoSize.Items[value].Value);
        return;
    }else{
        CVI_LOGI("switch to %d\n", media_mode);
    }
    param_ptr->CamCfg[CamID].CamMediaInfo.CurMediaMode = media_mode;
    param_ptr->WorkModeCfg.PhotoMode.CamMediaInfo[CamID].CurMediaMode = media_mode;

    MODEMNG_ResetPhotoMode(param_ptr);
    if(param_ptr != NULL){
        free(param_ptr);
        param_ptr = NULL;
    }

    PARAM_SetMenuParam(CamID, PARAM_MENU_PHOTO_SIZE, value);
}

void MODEMNG_PhotoSwitchSensor(void)
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

        MODEMNG_ClosePhotoMode();

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

        MODEMNG_OpenPhotoMode();
    }
}


static int32_t MODEMNG_SetPhotoEffect(uint32_t value)
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

#ifdef SERVICES_FACEP_ON
static int32_t MODEMNG_SetFaceDet(int32_t value)
{
    int32_t s32Ret = 0;
    int32_t i = 0;
    PARAM_FACEP_ATTR_S *facep_atrr = &PARAM_GetCtx()->pstCfg->MediaComm.Facep;
    MEDIA_PARAM_INIT_S *media_params = MEDIA_GetCtx();
    FACEP_SERVICE_HANDLE_T hdl = 0;

    CVI_LOGI("set face det state to %d\n", value);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        if(value == 0){
            CVI_LOGI("open face det");
            facep_atrr->sv_param[i].fd_enable = 1;
            hdl = media_params->SysServices.FacepHdl[i];
            FACEP_SERVICE_Enable(hdl, FACEP_FUNC_ID_FD);
        }else{
            CVI_LOGI("close face det");
            facep_atrr->sv_param[i].fd_enable = 0;
            hdl = media_params->SysServices.FacepHdl[i];
            FACEP_SERVICE_Disable(hdl, FACEP_FUNC_ID_FD);
        }
        /* 0:open, 1:close */
        PARAM_SetMenuParam(i, PARAM_MENU_FACE_DET, value);
    }

    return s32Ret;
}

static int32_t MODEMNG_SetFaceSmile(int32_t value)
{
    int32_t s32Ret = 0;
    int32_t i = 0;
    PARAM_FACEP_ATTR_S *facep_atrr = &PARAM_GetCtx()->pstCfg->MediaComm.Facep;
    MEDIA_PARAM_INIT_S *media_params = MEDIA_GetCtx();
    FACEP_SERVICE_HANDLE_T hdl = 0;

    CVI_LOGI("set face attr state to %d\n", value);

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        if(value == 0){
            CVI_LOGI("open face smile");
            facep_atrr->sv_param[i].fattr_enable = 1;
            hdl = media_params->SysServices.FacepHdl[i];
            FACEP_SERVICE_Enable(hdl, FACEP_FUNC_ID_FATTR);
        }else{
            CVI_LOGI("close face smile");
            facep_atrr->sv_param[i].fattr_enable = 0;
            hdl = media_params->SysServices.FacepHdl[i];
            FACEP_SERVICE_Disable(hdl, FACEP_FUNC_ID_FATTR);
        }
        /* 0:open, 1:close */
        PARAM_SetMenuParam(i, PARAM_MENU_FACE_SMILE, value);
    }

    return s32Ret;
}
#endif

static int32_t MODEMNG_SetFlashLed(int32_t value)
{
    int32_t i = 0;
    int32_t s32Ret = 0;
    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        PARAM_SetMenuParam(i, PARAM_MENU_FLASH_LED, value);
    }
    return s32Ret;
}

static int32_t MODEMNG_SetPhotoQuality(int32_t value)
{
    int32_t i = 0;
    int32_t s32Ret = 0;

    PARAM_MENU_S menu_param = {0};
    MEDIA_PARAM_INIT_S *MediaParams = MEDIA_GetCtx();

    for (i = 0; i < MAX_CAMERA_INSTANCES; i++) {
        if (!MEDIA_Is_CameraEnabled(i)) {
            continue;
        }

        PARAM_GetMenuParam(&menu_param);

        CVI_LOGI("set to %d\n", value);
        if ((uint32_t)value >= menu_param.PhotoQuality.ItemCnt) {
            CVI_LOGE("not support (%d)\n", value);
            return -1;
        }

        s32Ret = PHOTO_SERVICE_SetQuality(MediaParams->SysServices.PhotoHdl[i],
                                menu_param.PhotoQuality.Items[value].Value);
        if(s32Ret) {
            CVI_LOGE("set quality failed");
            return -1;
        }

        PARAM_SetMenuParam(i, PARAM_MENU_PHOTO_QUALITY, value);
    }
    return s32Ret;
}

static int32_t MODEMNG_PhotoStatesSettingMsgProc(MESSAGE_S* pstMsg)
{
    int32_t s32Ret = 0;

    switch (pstMsg->arg1)
    {
        case PARAM_MENU_PHOTO_SIZE:
            MODEMNG_PhotoSetMediaSize(0, pstMsg->arg2);
            break;

        case EVENT_MODEMNG_PHOTO_SET:
            MODEMNG_SetModeState(MEDIA_MOVIE_STATE_MENU);
            break;

        case PARAM_MENU_PHOTO_EFFECT:
            MODEMNG_SetPhotoEffect(pstMsg->arg2);
            break;

        case PARAM_MENU_SENSOR_SWITCH:
            MODEMNG_PhotoSwitchSensor();
            break;
        #ifdef SERVICES_FACEP_ON
        case PARAM_MENU_FACE_DET:
            MODEMNG_SetFaceDet(pstMsg->arg2);
            break;

        case PARAM_MENU_FACE_SMILE:
            MODEMNG_SetFaceSmile(pstMsg->arg2);
            break;
        #endif

        case PARAM_MENU_FLASH_LED:
            MODEMNG_SetFlashLed(pstMsg->arg2);
            break;

        case PARAM_MENU_PHOTO_QUALITY:
            MODEMNG_SetPhotoQuality(pstMsg->arg2);
            break;

        default:
            CVI_LOGE("not support param type(%d)\n\n", pstMsg->arg1);
            return -1;
    }

    return s32Ret;
}

int32_t MODEMNG_PhotoModeMsgProc(MESSAGE_S* pstMsg, void* pvArg, uint32_t* pStateID)
{
    /** check parameters */
    STATE_S* pstStateAttr = (STATE_S*)pvArg;
    MODEMNG_S* pstModeMngCtx = MODEMNG_GetModeCtx();
    (void)pstStateAttr;

    if (pstModeMngCtx->bSysPowerOff == true) {
        CVI_LOGI("power off ignore msg id: %x\n", pstMsg->topic);
        return PROCESS_MSG_RESULTE_OK;
    }

    CVI_LOGI("MODEMNG_PhotoModeMsgProc:%s\n", event_topic_get_name(pstMsg->topic));

    switch (pstMsg->topic) {
    case EVENT_SENSOR_PLUG_STATUS: {
#ifdef SERVICES_LIVEVIEW_ON
            PARAM_CFG_S Param;
            PARAM_GetParam(&Param);
            int32_t snsid = pstMsg->aszPayload[1];
            int32_t mode = pstMsg->aszPayload[0];
            uint32_t lastmode = Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode;
            if (SENSOR_PLUG_IN == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug in\n", snsid);
                CVI_LOGD("sensor %d resolution=%d\n", snsid, mode);
                Param.CamCfg[snsid].CamMediaInfo.CurMediaMode = MEDIA_Res2PhotoMediaMode(mode);
                Param.WorkModeCfg.RecordMode.CamMediaInfo[snsid].CurMediaMode = MEDIA_Res2RecordMediaMode(mode);
                Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode = MEDIA_Res2PhotoMediaMode(mode);
                Param.CamCfg[snsid].CamEnable = true;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = true;
                if(lastmode != Param.WorkModeCfg.PhotoMode.CamMediaInfo[snsid].CurMediaMode){
                    MAPI_VCAP_SetAhdMode(snsid, mode);
                #ifndef RESET_MODE_AHD_HOTPLUG_ON
                    MODEMNG_ResetPhotoMode(&Param);
                #endif
                }
            } else if (SENSOR_PLUG_OUT == pstMsg->arg1) {
                CVI_LOGD("sensor %d plug out\n", snsid);
                Param.CamCfg[snsid].CamEnable = false;
                Param.MediaComm.Window.Wnds[snsid].WndEnable = false;
            }
        #ifdef RESET_MODE_AHD_HOTPLUG_ON
            MODEMNG_ResetPhotoMode(&Param);
        #endif
            PARAM_SetParam(&Param);
            MODEMNG_MonitorStatusNotify(pstMsg);
#endif
            return PROCESS_MSG_RESULTE_OK;
        }
        case EVENT_MODEMNG_SETTING:
            {
                MODEMNG_PhotoStatesSettingMsgProc(pstMsg);
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
                MODEMNG_AdjustFocus(pstMsg->arg1 , (char *)pstMsg->aszPayload);
                return PROCESS_MSG_RESULTE_OK;
            }
        case EVENT_MODEMNG_START_PIV:
            {
                MODEMNG_StartPhoto();
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

int32_t MODEMNG_PhotoStatesInit(const STATE_S* pstBase)
{
    int32_t s32Ret = 0;

    static STATE_S stPhotoState =
    {
        WORK_MODE_PHOTO,
        MODEEMNG_STATE_PHOTO,
        MODEMNG_OpenPhotoMode,
        MODEMNG_ClosePhotoMode,
        MODEMNG_PhotoModeMsgProc,
        NULL
    };
    stPhotoState.argv = &stPhotoState;
    s32Ret = HFSM_AddState(MODEMNG_GetModeCtx()->pvHfsmHdl,
                              &stPhotoState,
                              (STATE_S*)pstBase);
    MODEMNG_CHECK_RET(s32Ret, MODE_EINVAL, "HFSM add NormalRec state");

    return s32Ret;
}

/** deinit Photo mode */
int32_t MODEMNG_PhotoStatesDeinit(void)
{
    int32_t s32Ret = 0;
    MODEMNG_ClosePhotoMode();
    return s32Ret;
}

void smile_callback_fun(void){
    EVENT_S stEvent;
    CVI_S32 ret = 0;
    CVI_LOGI("in");

    stEvent.topic = EVENT_MODEMNG_SMILE_START_PIV;
    stEvent.arg1 = WORK_MODE_PHOTO;
    ret = EVENTHUB_Publish(&stEvent);
    if(ret != 0){
        CVI_LOGE("EVENTHUB_Publish");
    }

    MODEMNG_StartPhoto();
}

#include <stdio.h>
#include <stdlib.h>

#include "liveview.h"
#include "cvi_log.h"
#include "mapi.h"
#include "osal.h"
#include "cvi_vpss.h"
#include "zoomp.h"

typedef struct __lv_condext {
	lv_param_t param;

	volatile uint32_t shutdown;

	MAPI_WND_HANDLE_T wnd[DISP_MAX_WND_NUM];
	pthread_mutex_t lv_mutex;
	pthread_cond_t lv_cond;
	OSAL_TASK_HANDLE_S lv_task;

	// event task
	OSAL_TASK_HANDLE_S event_task;
	MQ_ENDPOINT_HANDLE_t mq_ep;
} lv_context_t, *lv_context_handle_t;

typedef int32_t (*lv_cmd_cb_t)(MQ_MSG_S *msg, void *userdate);

typedef struct _lv_cmd_desc {
	lv_cmd_cb_t cb;
	uint32_t flags;
} lv_cmd_desc_t;

static uint8_t move_up_flag = 0;
static uint8_t move_down_flag = 0;

static void lv_cmd_movewndpost(lv_context_handle_t lv, int32_t wnd, bool add)
{
	lv_param_handle_t param = &lv->param;
	if (param->LiveviewService[wnd].wnd_attr.UsedCrop == true) {
		VPSS_CROP_INFO_S attr;
		VPSS_GRP_ATTR_S stGrpAttr;
		CHECK_RET(CVI_VPSS_GetGrpAttr(param->LiveviewService[wnd].wnd_attr.BindVprocId, &stGrpAttr));
		CHECK_RET(CVI_VPSS_GetChnCrop(param->LiveviewService[wnd].wnd_attr.BindVprocId, param->LiveviewService[wnd].wnd_attr.BindVprocChnId, &attr));
		if (add == true) {
			attr.stCropRect.s32Y = attr.stCropRect.s32Y + (param->LiveviewService[wnd].wnd_attr.OneStep);
		} else {
			attr.stCropRect.s32Y = attr.stCropRect.s32Y - (param->LiveviewService[wnd].wnd_attr.OneStep);
		}

		if ((attr.stCropRect.s32Y >= 0) && ((attr.stCropRect.s32Y + attr.stCropRect.u32Height) <= stGrpAttr.u32MaxH)) {
			if (add == true) {
				param->LiveviewService[wnd].wnd_attr.yStep++;
			} else {
				param->LiveviewService[wnd].wnd_attr.yStep--;
			}
		}
	}
}

static void lv_cmd_switchwndmod(cmd_liveview_t cmdid, lv_context_handle_t lv, uint32_t idset)
{
	lv_param_handle_t param = &lv->param;
	switch (cmdid) {
	case CMD_LIVEVIEW_SWITCH:
		for (uint32_t i = 0; i < param->WndCnt * 2; i += 2) {
			uint32_t smallEn = (0x1 << i) & idset;
			uint32_t wndEn = (0x1 << (i + 1)) & idset;
			param->LiveviewService[i / 2].wnd_attr.SmallWndEnable = smallEn > 0 ? true : false;
			param->LiveviewService[i / 2].wnd_attr.WndEnable = wndEn > 0 ? true : false;
		}
		break;
	case CMD_LIVEVIEW_MOVEUP:
		if (param->LiveviewService[idset].wnd_attr.SmallWndEnable == false) {
			lv_cmd_movewndpost(lv, idset, false);
			move_up_flag = 1;
		}
		break;
	case CMD_LIVEVIEW_MOVEDOWN:
		if (param->LiveviewService[idset].wnd_attr.SmallWndEnable == false) {
			lv_cmd_movewndpost(lv, idset, true);
			move_down_flag = 1;
		}
		break;
	default:
		CVI_LOGE("cmdid %d is illegal\n", cmdid);
		break;
	}
}
static void lv_cmd_adjustwndfocus(lv_context_handle_t lv, int32_t wnd, char *ratio)
{
	lv_param_handle_t param = &lv->param;

	if (param->LiveviewService[wnd].wnd_attr.UsedCrop == true) {
		param->LiveviewService[wnd].wnd_attr.ratio = atof(ratio);
		CVI_LOGD("focus ratio:%f", param->LiveviewService[wnd].wnd_attr.ratio);
	}
}

static int32_t lv_cmd_cb_adjustfocus(MQ_MSG_S *msg, void *userdata)
{
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	lv_cmd_adjustwndfocus(lv, msg->arg2, msg->payload);
	return 0;
}

// static int32_t lv_set_vprocattr(MAPI_VPROC_HANDLE_T vproc_hdl, LIVEVIEW_SERVICE_WNDATTR_S wnd_attr)
// {
//     int32_t ret = 0;

//     VPSS_CHN_ATTR_S stChnAttr;
//     ret = MAPI_VPROC_GetChnAttr(vproc_hdl, wnd_attr.BindVprocChnId, &stChnAttr);
//     if (ret != 0) {
//         CVI_LOGE("MAPI_VPROC_GetChnAttr failed\n");
//         return ret;
//     }
//     stChnAttr.bMirror = wnd_attr.WndMirror;
//     stChnAttr.bFlip = wnd_attr.WndFilp;

//     ret = MAPI_VPROC_SetChnAttr(vproc_hdl, wnd_attr.BindVprocChnId, &stChnAttr);
//     if (ret != 0) {
//         CVI_LOGE("MAPI_VPROC_SetChnAttr failed\n");
//         return ret;
//     }
//     return ret;
// }

static void lv_set_wndattrmirror(lv_context_handle_t lv, int32_t id, int32_t en)
{
	lv->param.LiveviewService[id].wnd_attr.WndMirror = en;
}

static void lv_set_wndattrfilp(lv_context_handle_t lv, int32_t id, int32_t en)
{
	lv->param.LiveviewService[id].wnd_attr.WndFilp = en;
}

static void lv_cmd_wnd_mirrorfilp(cmd_liveview_t cmdid, uint32_t attr_en, lv_context_handle_t lv)
{
	lv_param_handle_t param = &lv->param;
	uint32_t wndIndex = attr_en >> 1;
	uint32_t val = attr_en & 0x1;
	switch (cmdid) {
	case CMD_LIVEVIEW_MIRROR:
		param->LiveviewService[wndIndex].wnd_attr.WndMirror = val;
		lv_set_wndattrmirror(lv, wndIndex, val);
		break;
	case CMD_LIVEVIEW_FILP:
		param->LiveviewService[wndIndex].wnd_attr.WndFilp = val;
		lv_set_wndattrfilp(lv, wndIndex, val);
		break;
	default:
		CVI_LOGE("cmdid %d is illegal\n", cmdid);
		break;
	}
}

static int32_t lv_cmd_cb_shutdown(MQ_MSG_S *msg, void *userdata)
{
	UNUSED(msg);
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	lv->shutdown = 1;
	// TODO: send ACK
	return 0;
}

static int32_t lv_cmd_cb_switchwndmod(MQ_MSG_S *msg, void *userdata)
{
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	int32_t cmd_id = msg->arg1;
	lv_cmd_switchwndmod(cmd_id, lv, msg->arg2);
	// TODO: send ACK
	return 0;
}

static int32_t lv_cmd_cb_movewndup(MQ_MSG_S *msg, void *userdata)
{
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	int32_t cmd_id = msg->arg1;
	lv_cmd_switchwndmod(cmd_id, lv, msg->arg2);
	// TODO: send ACK
	return 0;
}

static int32_t lv_cmd_cb_movewnddown(MQ_MSG_S *msg, void *userdata)
{
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	int32_t cmd_id = msg->arg1;
	lv_cmd_switchwndmod(cmd_id, lv, msg->arg2);
	// TODO: send ACK
	return 0;
}

static int32_t lv_cmd_cb_mirror(MQ_MSG_S *msg, void *userdata)
{
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	int32_t cmd_id = msg->arg1;
	int32_t en = msg->arg2;
	lv_cmd_wnd_mirrorfilp(cmd_id, en, lv);
	// TODO: send ACK
	return 0;
}

static int32_t lv_cmd_cb_filp(MQ_MSG_S *msg, void *userdata)
{
	lv_context_handle_t lv = (lv_context_handle_t)userdata;
	int32_t cmd_id = msg->arg1;
	int32_t en = msg->arg2;
	lv_cmd_wnd_mirrorfilp(cmd_id, en, lv);
	// TODO: send ACK
	return 0;
}

static lv_cmd_desc_t lv_cmd_tbl[] = {
	{NULL, 0},					 /* 0x00    INVALID */
	{lv_cmd_cb_shutdown, 0},	 /* 0x01    SHUTDOWN */
	{lv_cmd_cb_switchwndmod, 0}, /* 0x02    switch WNDMOD */
	{lv_cmd_cb_movewndup, 0},	 /* 0x03    moveup */
	{lv_cmd_cb_movewnddown, 0},	 /* 0x04    movedown WNDMOD */
	{lv_cmd_cb_mirror, 0},		 /* 0x05    WND MIRROR */
	{lv_cmd_cb_filp, 0},		 /* 0x06    WND FILP */
	{lv_cmd_cb_adjustfocus, 0},	 /* 0x07    ADJUST FOCUS */
};

static int32_t lv_mq_cb(MQ_ENDPOINT_HANDLE_t ep, MQ_MSG_S *msg, void *ep_arg)
{
	UNUSED(ep);
	int32_t ret = 0;
	lv_context_handle_t lv = (lv_context_handle_t)ep_arg;
	pthread_mutex_lock(&lv->lv_mutex);
#if 0
    printf("lv_mq_cb: rx, target_id = %08x, len = %d, ep_arg = %p\n", msg->target_id, msg->len, ep_arg);
    printf("lv_mq_cb:     arg1 = 0x%08x, arg2 = 0x%08x\n", msg->arg1, msg->arg2);
    printf("lv_mq_cb:     seq_no = 0x%04x, time = %lu\n", msg->seq_no, msg->crete_time);
    if (msg->len > (int32_t)MQ_MSG_HEADER_LEN + 4) {
        printf("lv_mq_cb:     payload [%02x %02x %02x %02x]\n", msg->payload[0], msg->payload[1],
               msg->payload[2], msg->payload[3]);
    }
#endif

	int32_t cmd_id = msg->arg1;
	CVI_LOG_ASSERT(cmd_id >= 0 && cmd_id < (int32_t)(sizeof(lv_cmd_tbl) / sizeof(lv_cmd_desc_t)),
				   "cmd_id %d out of range\n", cmd_id);

	if (lv_cmd_tbl[cmd_id].cb == NULL) {
		CVI_LOGE("cmd_id %d not handled\n", cmd_id);
		pthread_cond_signal(&lv->lv_cond);
		pthread_mutex_unlock(&lv->lv_mutex);
		return -1;
	}

	ret = lv_cmd_tbl[cmd_id].cb(msg, ep_arg);
	if (ret != 0) {
		CVI_LOGE("cb %d failed! \n", cmd_id);
	}
	pthread_cond_signal(&lv->lv_cond);
	pthread_mutex_unlock(&lv->lv_mutex);

	return ret;
}

static void lv_event_task_entry(void *arg)
{
	lv_context_handle_t lv = (lv_context_handle_t)arg;
	lv_param_handle_t p = &lv->param;

	UNUSED(lv);
	UNUSED(p);

	// start mq
	MQ_ENDPOINT_CONFIG_S mq_config;
	mq_config.name = "lv_mq";
	mq_config.id = MQ_ID(CMD_CLIENT_ID_LIVEVIEW, 0);
	mq_config.recv_cb = lv_mq_cb;
	mq_config.recv_cb_arg = (void *)lv;
	int32_t rc = MQ_CreateEndpoint(&mq_config, &lv->mq_ep);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("MQ_CreateEndpoint failed\n");
		exit(-1);
	}

	while (!lv->shutdown) {
		OSAL_TASK_Sleep(10000); // 10 ms
	}

	// cleanup mq
	MQ_DestroyEndpoint(lv->mq_ep);
}

int32_t lv_start_event_task(lv_context_handle_t lv)
{
	OSAL_TASK_ATTR_S ta;
	ta.name = "lv_event";
	ta.entry = lv_event_task_entry;
	ta.param = (void *)lv;
	ta.priority = OSAL_TASK_PRI_NORMAL;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	int32_t rc = OSAL_TASK_Create(&ta, &lv->event_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("lv_event task create failed, %d\n", rc);
		return -1;
	}

	return 0;
}

int32_t lv_stop_event_task(lv_context_handle_t lv)
{
	int32_t rc = OSAL_TASK_Join(lv->event_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("lv_event task join failed, %d\n", rc);
		return -1;
	}
	OSAL_TASK_Destroy(&lv->event_task);

	return 0;
}

static void lv_set_toVoVprocChnAttr(lv_context_handle_t lv)
{
	lv_param_handle_t p = &lv->param;
	uint32_t i = 0;

	for (i = 0; i < p->WndCnt; i++) {
		if (p->LiveviewService[i].wnd_attr.WndEnable == true) {
			VPSS_CHN_ATTR_S stChnAttr;
			CHECK_RET(MAPI_VPROC_GetChnAttr(p->LiveviewService[i].vproc_hdl, p->LiveviewService[i].wnd_attr.BindVprocChnId, &stChnAttr));
			VPSS_GRP_ATTR_S pstGrpAttr = {0};
			CHECK_RET(CVI_VPSS_GetGrpAttr(p->LiveviewService[i].wnd_attr.BindVprocId, &pstGrpAttr));
			if (p->LiveviewService[i].wnd_attr.SmallWndEnable == false) {
				stChnAttr.u32Width = p->LiveviewService[i].wnd_attr.WndWidth;
				stChnAttr.u32Height = p->LiveviewService[i].wnd_attr.WndHeight;
			} else {
				stChnAttr.u32Width = p->LiveviewService[i].wnd_attr.WndsWidth;
				stChnAttr.u32Height = p->LiveviewService[i].wnd_attr.WndsHeight;
			}
			stChnAttr.bFlip = p->LiveviewService[i].wnd_attr.WndFilp;
			stChnAttr.bMirror = p->LiveviewService[i].wnd_attr.WndMirror;
			// CVI_LOGE(" w = %d  h = %d  w = %d  h = %d \n", stChnAttr.u32Width, stChnAttr.u32Height, p->LiveviewService[i].wnd_attr.WndWidth, p->LiveviewService[i].wnd_attr.WndHeight);
			CHECK_RET(MAPI_VPROC_SetChnAttr(p->LiveviewService[i].vproc_hdl,
											p->LiveviewService[i].wnd_attr.BindVprocChnId, &stChnAttr));

			if (p->LiveviewService[i].wnd_attr.UsedCrop == true) {
				VPSS_CROP_INFO_S grp_crop_info = {0};
				RECT_S crop_in = {0}, crop_out = {0};

				CHECK_RET(MAPI_VPROC_GetGrpCrop(p->LiveviewService[i].vproc_hdl, &grp_crop_info));

				CVI_LOGI("p->LiveviewService[i].wnd_attr.ratio:%f", p->LiveviewService[i].wnd_attr.ratio);
				if(((!grp_crop_info.bEnable) && p->LiveviewService[i].wnd_attr.ratio < 2)
					|| p->LiveviewService[i].wnd_attr.ratio > ZOOM_MAX_RADIO){
					CVI_LOGI("don't need to crop: %d", grp_crop_info.bEnable);
					return;
				}

				if(grp_crop_info.bEnable){
					crop_in.s32X = grp_crop_info.stCropRect.s32X;
					crop_in.s32Y = grp_crop_info.stCropRect.s32Y;
					crop_in.u32Width = grp_crop_info.stCropRect.u32Width;
					crop_in.u32Height = grp_crop_info.stCropRect.u32Height;
				}else{
					crop_in.s32X = 0;
					crop_in.s32Y = 0;
					crop_in.u32Width = pstGrpAttr.u32MaxW;
					crop_in.u32Height = pstGrpAttr.u32MaxH;
				}

				if(!ZOOMP_Is_Init()){
					ZOOMP_Init(crop_in);
				}

				if(ZOOMP_GetCropInfo(crop_in, &crop_out, p->LiveviewService[i].wnd_attr.ratio)){
					return;
				}

				if(crop_out.u32Width < 64 || crop_out.u32Height < 64){
					CVI_LOGE("crop is too small");
					return;
				}

				grp_crop_info.bEnable = true;
				grp_crop_info.enCropCoordinate = VPSS_CROP_ABS_COOR;
				grp_crop_info.stCropRect = crop_out;
				CHECK_RET(MAPI_VPROC_SetGrpCrop(p->LiveviewService[i].vproc_hdl, &grp_crop_info));
			}
		}
	}
}

static void lv_video_task_entry(void *arg)
{
	lv_context_handle_t lv = (lv_context_handle_t)arg;

	while (!lv->shutdown) {
		pthread_mutex_lock(&lv->lv_mutex);
		lv_set_toVoVprocChnAttr(lv);
		pthread_cond_wait(&lv->lv_cond, &lv->lv_mutex);
		pthread_mutex_unlock(&lv->lv_mutex);
	}
}

static int32_t lv_start_video_task(lv_context_handle_t lv)
{
	OSAL_TASK_ATTR_S ta;
	ta.name = "liveview";
	ta.entry = lv_video_task_entry;
	ta.param = (void *)lv;
	ta.priority = OSAL_TASK_PRI_NORMAL;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	int32_t rc = OSAL_TASK_Create(&ta, &lv->lv_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("lv_video task create failed, %d\n", rc);
		return -1;
	}

	return 0;
}

int32_t lv_stop_video_task(lv_context_handle_t lv)
{

	int32_t rc = OSAL_TASK_Join(lv->lv_task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("lv_video task join failed, %d\n", rc);
		return -1;
	}
	OSAL_TASK_Destroy(&lv->lv_task);

	return 0;
}

int32_t LIVEVIEW_SERVICE_GetStepY(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, int32_t *lastStep)
{
	(void)hdl;
	(void)wndId;
	(void)lastStep;
	// if (hdl == NULL) {
	//     return -1;
	// }
	// LIVEVIEW_SERVICE_WNDATTR_S *WndParam;
	// WndParam = (LIVEVIEW_SERVICE_WNDATTR_S *)malloc(sizeof(LIVEVIEW_SERVICE_WNDATTR_S));
	// if(LIVEVIEW_SERVICE_GetParam(hdl, wndId, WndParam) != 0) {
	//     free(WndParam);
	//     return -1;
	// } else {
	//     lastStep = (int32_t*)WndParam->yStep;
	// }

	// CVI_LOGD("LIVEVIEW_SERVICE_GetStepY lastStep = (%d)\n", (int32_t)lastStep);

	// free(WndParam);
	return 0;
}

int32_t LIVEVIEW_SERVICE_SetStepY(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, int32_t step, int32_t *lastStep)
{
	(void)hdl;
	(void)wndId;
	(void)lastStep;
	(void)step;
	// if (hdl == NULL) {
	//     return -1;
	// }

	// PARAM_WND_ATTR_S WndParam;
	// PARAM_GetWndParam(&WndParam);
	// WndParam.Wnds[wndId].yStep = step;
	// lastStep = (int32_t*)step;
	// PARAM_SetWndParam(&WndParam);

	// CVI_LOGD("LIVEVIEW_SERVICE_SetStepY lastStep = (%d)\n", (int32_t)lastStep);
	return 0;
}

int32_t LIVEVIEW_SERVICE_AddStepY(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, int32_t step, int32_t *lastStep)
{
	(void)hdl;
	(void)wndId;
	(void)lastStep;
	(void)step;
	// if(hdl == NULL) {
	//     return -1;
	// }

	// PARAM_WND_ATTR_S WndParam;
	// PARAM_GetWndParam(&WndParam);
	// WndParam.Wnds[wndId].yStep = (WndParam.Wnds[wndId].yStep + step);
	// lastStep = (int32_t*)WndParam.Wnds[wndId].yStep;
	// PARAM_SetWndParam(&WndParam);
	// CVI_LOGD("LIVEVIEW_SERVICE_AddStepY lastStep = (%d)\n", (int32_t)lastStep);

	return 0;
}

int32_t LIVEVIEW_SERVICE_Create(LIVEVIEW_SERVICE_HANDLE_T *hdl, LIVEVIEW_SERVICE_PARAM_S *params)
{
	lv_context_handle_t lv = (lv_context_handle_t)calloc(sizeof(lv_context_t), 1);
	lv->param = *params;
	lv_param_handle_t p = &lv->param;
	uint32_t i = 0;

	for (i = 0; i < p->WndCnt; i++) {
		p->LiveviewService[i].wnd_attr.ratio = 1; // 初始化focus ratio为1
	}

	p->hVbPool = VB_INVALID_POOLID;
	p->vproc_id = -1;

	pthread_mutex_init(&lv->lv_mutex, NULL);
	pthread_condattr_t condattr;
	pthread_condattr_init(&condattr);
	pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&lv->lv_cond, &condattr);
	pthread_condattr_destroy(&condattr);

	// start send frame to wnd grp
	lv_start_video_task(lv);

	// start recvie cmd
	lv_start_event_task(lv);

	*hdl = (LIVEVIEW_SERVICE_HANDLE_T)lv;

	return 0;
}

int32_t LIVEVIEW_SERVICE_Destroy(LIVEVIEW_SERVICE_HANDLE_T hdl)
{
	lv_context_handle_t lv = (lv_context_handle_t)hdl;

	// uint32_t i = 0;
	// send shutdown to self
	lv->shutdown = 1;

	// wait for exit
	while (!lv->shutdown) {
		OSAL_TASK_Sleep(20000);
	}
	pthread_mutex_lock(&lv->lv_mutex);
	pthread_cond_signal(&lv->lv_cond);
	pthread_mutex_unlock(&lv->lv_mutex);
	CVI_LOGI("LiveView Service destroy\n");

	// stop event task
	CHECK_RET(lv_stop_event_task(lv));

	// stop video task
	CHECK_RET(lv_stop_video_task(lv));

	pthread_cond_destroy(&lv->lv_cond);
	pthread_mutex_destroy(&lv->lv_mutex);
	CVI_LOGI("LiveView Service destroy end\n");
	free(lv);
	return 0;
}

int32_t LIVEVIEW_SERVICE_GetParam(LIVEVIEW_SERVICE_HANDLE_T hdl, int32_t wndId, LIVEVIEW_SERVICE_WNDATTR_S *WndParam)
{
	lv_context_handle_t lv = (lv_context_handle_t)hdl;

	lv_param_handle_t p = &lv->param;

	memcpy(WndParam, &p->LiveviewService[wndId].wnd_attr, sizeof(LIVEVIEW_SERVICE_WNDATTR_S));

	return 0;
}

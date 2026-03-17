#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include "cvi_log.h"
#include "mapi.h"
#include "osal.h"
#include "rtsp_service.h"
#include "audio_service.h"
#include "rtsp.h"
#ifdef SERVICES_SUBVIDEO_ON
#include "subvideo_service.h"
#endif

typedef struct RTSP_SER_ATTR {
	int32_t id;
	char rtsp_name[MAX_RTSP_NAME_LEN];
	int32_t max_conn;
	int32_t timeout;
	int32_t port;
	int32_t auth_en;
	char username[MAX_RTSP_NAME_LEN];
	char password[MAX_RTSP_NAME_LEN];
	RTSP_VIDEO_FORMAT_E video_codec;
	int32_t audio_en;
	RTSP_AUDIO_FORMAT_E audio_codec;
	RTSP_SERVICE_CALLBACK *rtsp_play;
	void *rtsp_play_arg;
	RTSP_SERVICE_CALLBACK *rtsp_teardown;
	void *rtsp_teardown_arg;

	float framerate;
	int32_t bitrate_kbps;
	int32_t audio_sample_rate;
	int32_t audio_channels;
	int32_t audio_pernum;
	MAPI_VENC_HANDLE_T venc_hdl;
} RTSP_SER_ATTR_S;

typedef struct RTSP_SERVICE_CONTEXT {
	int32_t ref;
	int32_t mute;
	char *mute_data;
	RTSP_SER_ATTR_S attr;
	OSAL_MUTEX_HANDLE_S mutex;
	OSAL_TASK_HANDLE_S video_task;
	int32_t video_exit;
	void *rtsp_ser;
} RTSP_SERVICE_CONTEXT_S;

static RTSP_SERVICE_CONTEXT_S *gstRtspSerCtx[RTSP_INSTANCE_NUM];
static OSAL_MUTEX_S rtsp_ctx_mutex = OSAL_MUTEX_INITIALIZER_R;

#ifndef SERVICES_SUBVIDEO_ON
static int32_t rtsp_service_get_venc_stream(MAPI_VENC_HANDLE_T vhdl, VENC_STREAM_S *stream)
{
	int32_t ret = 0;
	if (vhdl == NULL) {
		CVI_LOGE("vhdl is null");
		return -1;
	}

	ret = MAPI_VENC_GetStreamTimeWait(vhdl, stream, 1000);
	if (ret != 0) {
		CVI_LOGE("RS[%p]: MAPI_VENC_GetStreamTimeWait failed", vhdl);
		return -1;
	}

	if (stream->u32PackCount <= 0 || stream->u32PackCount > 8) {
		CVI_LOGE("RS[%p]: MAPI_VENC_GetStreamTimeWait failed, stream->u32PackCount:%d", vhdl, stream->u32PackCount);
		MAPI_VENC_ReleaseStream(vhdl, stream);
		return -1;
	}

	return 0;
}

static void rtsp_service_video_task(void *arg)
{
	RTSP_SERVICE_CONTEXT_S *c = arg;
	MAPI_VENC_HANDLE_T vhdl = c->attr.venc_hdl;
	VENC_STREAM_S stream;
	CVI_LOGD("rtsp start get data");

#ifdef RTSP_NO_AUDIO
	int32_t audio_null_data_len = 64;
    uint8_t *audio_null_data = (uint8_t *)malloc(64);
    if (audio_null_data == NULL){
        CVI_LOGD( "Malloc failed. audio_null_data == NULL. \n");
        return ;
    }
	memset(audio_null_data, 0, audio_null_data_len);
#endif
	while (!c->video_exit) {
		memset(&stream, 0x0, sizeof(VENC_STREAM_S));
		int32_t ret = rtsp_service_get_venc_stream(vhdl, &stream);
		if (ret != 0) {
			OSAL_TASK_Sleep(20 * 1000);
			continue;
		}
		RTSP_FRAME_S frame;
		memset(&frame, 0x0, sizeof(RTSP_FRAME_S));
		frame.type = FRAME_TYPE_VIDEO;
		bool iskey = 0;

		for (unsigned i = 0; i < stream.u32PackCount; i++) {
			VENC_PACK_S *ppack;
			ppack = &stream.pstPack[i];
			frame.data[i] = ppack->pu8Addr + ppack->u32Offset;
			frame.len[i] = ppack->u32Len - ppack->u32Offset;
			frame.vi_pts[i] = stream.pstPack[i].u64PTS;
			MAPI_VENC_GetStreamStatus(vhdl, ppack, &iskey);
			frame.iskey[i] = iskey;
		}
		RTSP_SendFrame(c->rtsp_ser, &frame);
		MAPI_VENC_ReleaseStream(vhdl, &stream);
		#ifdef RTSP_NO_AUDIO
		memset(&frame, 0x0, sizeof(RTSP_FRAME_S));
		// send audio null data
		frame.type = FRAME_TYPE_AUDIO;
		frame.data[0] = audio_null_data;
		frame.iskey[0] = 1;
		frame.len[0] = audio_null_data_len;
		RTSP_SendFrame(c->rtsp_ser, &frame);
		#endif
	}

#ifdef RTSP_NO_AUDIO
    if (audio_null_data) {
        free(audio_null_data);
    }
#endif
}

static int32_t rtsp_service_start_video_task(RTSP_SERVICE_CONTEXT_S *c)
{
	OSAL_TASK_ATTR_S ta;
	c->video_exit = 0;
	ta.name = c->attr.rtsp_name;
	ta.entry = rtsp_service_video_task;
	ta.param = (void *)c;
	ta.priority = OSAL_TASK_PRI_RT_MID;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	OSAL_TASK_Create(&ta, &c->video_task);
	return 0;
}

static int32_t rtsp_service_stop_video_task(RTSP_SERVICE_CONTEXT_S *c)
{
	c->video_exit = 1;
	OSAL_TASK_Join(c->video_task);
	OSAL_TASK_Destroy(&c->video_task);
	return 0;
}
#else
static void rtsp_service_video_cb(const VENC_STREAM_S *stream, void *arg){
    RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)arg;
    MAPI_VENC_HANDLE_T vhdl = ctx->attr.venc_hdl;
    if(ctx == NULL || stream == NULL || vhdl == NULL){
        return;
    }

    RTSP_FRAME_S frame;
	memset(&frame, 0x0, sizeof(RTSP_FRAME_S));
    frame.type = FRAME_TYPE_VIDEO;
    bool iskey = 0;
    for (unsigned i = 0; i < stream->u32PackCount; i++) {
        VENC_PACK_S *ppack;
        ppack = &stream->pstPack[i];
        frame.data[i] = ppack->pu8Addr + ppack->u32Offset;
        frame.len[i] = ppack->u32Len - ppack->u32Offset;
        frame.vi_pts[i] = stream->pstPack[i].u64PTS;
        MAPI_VENC_GetStreamStatus(vhdl, ppack, &iskey);
        frame.iskey[i] = iskey;
    }
    RTSP_SendFrame(ctx->rtsp_ser, &frame);
    return;
}

static int32_t rtsp_service_add_video_cb(RTSP_SERVICE_HANDLE_T *hdl){
    RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
    if(ctx == NULL){
        return -1;
    }
    CVI_LOGI("rtsp add %s video cb", ctx->attr.rtsp_name);
    VIDEO_SERVICR_CallbackUnSet(ctx->attr.id,ctx->attr.rtsp_name);
    VIDEO_SERVICR_CallbackSet(ctx->attr.id,ctx->attr.rtsp_name, rtsp_service_video_cb, hdl);
    MAPI_VENC_RequestIDR(ctx->attr.venc_hdl);/*请求I帧*/
    return 0;
}

static int32_t rtsp_service_remove_video_cb(RTSP_SERVICE_HANDLE_T *hdl){
    RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
    if(ctx == NULL){
        return -1;
    }
    CVI_LOGI("rtsp remove %s video cb", ctx->attr.rtsp_name);
    VIDEO_SERVICR_CallbackUnSet(ctx->attr.id,ctx->attr.rtsp_name);
    return 0;
}
#endif

#ifndef RTSP_NO_AUDIO
static void rtsp_service_audio_cb(const AUDIO_FRAME_S *audio_frame, const AEC_FRAME_S *aec_frame, void *arg)
{
	(void)aec_frame;
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)arg;
	if (ctx == NULL) {
		return;
	}

	RTSP_FRAME_S frame;
	memset(&frame, 0x0, sizeof(RTSP_FRAME_S));
	frame.type = FRAME_TYPE_AUDIO;
	if (ctx->mute == 1) {
		frame.data[0] = (unsigned char *)ctx->mute_data;
	} else {
		frame.data[0] = audio_frame->u64VirAddr[0];
	}
	frame.len[0] = audio_frame->u32Len * 2;
	frame.vi_pts[0] = audio_frame->u64TimeStamp;
	RTSP_SendFrame(ctx->rtsp_ser, &frame);
	return;
}

static int32_t rtsp_service_add_audio_cb(RTSP_SERVICE_HANDLE_T *hdl)
{
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
	if (ctx == NULL) {
		return -1;
	}
	CVI_LOGD("rtsp add %s mute %d", ctx->attr.rtsp_name, ctx->mute);
	AUDIO_SERVICR_ACAP_CallbackUnset(ctx->attr.rtsp_name);
	AUDIO_SERVICR_ACAP_CallbackSet(ctx->attr.rtsp_name, rtsp_service_audio_cb, hdl);
	return 0;
}

static int32_t rtsp_service_remove_audio_cb(RTSP_SERVICE_HANDLE_T *hdl)
{
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
	if (ctx == NULL) {
		return -1;
	}
	CVI_LOGD("rtsp remove %s mute %d", ctx->attr.rtsp_name, ctx->mute);
	AUDIO_SERVICR_ACAP_CallbackUnset(ctx->attr.rtsp_name);
	return 0;
}
#endif

static void rtsp_service_start_media_by_name(char *name)
{
	if (name == NULL || strlen(name) <= 0) {
		return;
	}

	OSAL_MUTEX_Lock(&rtsp_ctx_mutex);
	for (int32_t i = 0; i < RTSP_INSTANCE_NUM; i++) {
		RTSP_SERVICE_CONTEXT_S *c = gstRtspSerCtx[i];
		if (c) {
			OSAL_MUTEX_Lock(c->mutex);
			if (strcmp(c->attr.rtsp_name, name) == 0) {
				if (c->ref == 0) {
					c->attr.rtsp_play(c->ref, c->attr.rtsp_play_arg);
					#ifdef SERVICES_SUBVIDEO_ON
					rtsp_service_add_video_cb((void *)c);
					#else
					rtsp_service_start_video_task(c);
					#endif
					#ifndef RTSP_NO_AUDIO
					rtsp_service_add_audio_cb((void *)c);
					#endif
				}
				c->ref++;
				CVI_LOGD("session %s start play %d", name, c->ref);
				OSAL_MUTEX_Unlock(c->mutex);
				break;
			}
			OSAL_MUTEX_Unlock(c->mutex);
		}
	}
	OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
}

static void rtsp_service_stop_media_by_name(char *name)
{
	if (name == NULL || strlen(name) <= 0) {
		return;
	}
	OSAL_MUTEX_Lock(&rtsp_ctx_mutex);
	for (int32_t i = 0; i < RTSP_INSTANCE_NUM; i++) {
		RTSP_SERVICE_CONTEXT_S *c = gstRtspSerCtx[i];
		if (c) {
			OSAL_MUTEX_Lock(c->mutex);
			if (strcmp(c->attr.rtsp_name, name) == 0) {
				c->ref--;
				if (c->ref == 0) {
					#ifdef SERVICES_SUBVIDEO_ON
                    rtsp_service_remove_video_cb((void *)c);
					#else
					rtsp_service_stop_video_task(c);
					#endif
					#ifndef RTSP_NO_AUDIO
					rtsp_service_remove_audio_cb((void *)c);
					#endif
					c->attr.rtsp_teardown(c->ref, c->attr.rtsp_teardown_arg);
				}
				OSAL_MUTEX_Unlock(c->mutex);
				break;
			}
			OSAL_MUTEX_Unlock(c->mutex);
		}
	}
	OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
}

static void rtsp_service_event_cb(RTSP_EVENT_S *e)
{
	if (e) {
		switch (e->e) {
		case RTSP_EVENT_CLI_CONNECT: {
			CVI_LOGD("rtsp_service: recv %s CONNECT %s Event", e->rtsp_name, e->cli_ipaddr);
			rtsp_service_start_media_by_name(e->rtsp_name);
		} break;
		case RTSP_EVENT_CLI_DISCONNECT: {
			CVI_LOGD("rtsp_service: recv %s DISCONNECT %s Event", e->rtsp_name, e->cli_ipaddr);
			rtsp_service_stop_media_by_name(e->rtsp_name);
		} break;
		default:
			break;
		}
	}
}

static int32_t rtsp_service_create(RTSP_SERVICE_HANDLE_T *hdl, RTSP_SER_ATTR_S *attr)
{
	OSAL_MUTEX_Lock(&rtsp_ctx_mutex);
	if (gstRtspSerCtx[attr->id] != NULL) {
		OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
		CVI_LOGI("%s is exist", attr->rtsp_name);
		return -1;
	}
	CVI_LOGD("rtsp_service init start %s", attr->rtsp_name);
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)malloc(sizeof(RTSP_SERVICE_CONTEXT_S));
	if (ctx == NULL) {
		OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
		return -1;
	}
	memset(ctx, 0x0, sizeof(RTSP_SERVICE_CONTEXT_S));
	memcpy(&ctx->attr, attr, sizeof(RTSP_SER_ATTR_S));
	ctx->mute = ((attr->audio_en == 1) ? 0 : 1);
	OSAL_MUTEX_ATTR_S mutex_attr;
	mutex_attr.name = "rtsp_mutex";
	mutex_attr.type = PTHREAD_MUTEX_NORMAL;
	OSAL_MUTEX_Create(&mutex_attr, &ctx->mutex);
	CVI_LOGD("rtsp_service init mute %d", ctx->mute);

	ctx->mute_data = (char *)malloc(attr->audio_pernum * 2);
	memset(ctx->mute_data, 0x0, attr->audio_pernum * 2);

	RTSP_INFO_S rtsp_info;
	memset(&rtsp_info, 0x0, sizeof(RTSP_INFO_S));
	RTSP_MEDIA_INFO_S media_info;
	memset(&media_info, 0x0, sizeof(RTSP_MEDIA_INFO_S));

	strcpy(rtsp_info.username, attr->username);
	strcpy(rtsp_info.password, attr->password);
	rtsp_info.max_conn = attr->max_conn;
	rtsp_info.timeout = attr->timeout;
	rtsp_info.port = attr->port;
	CVI_LOGD("rtsp_service init rtsp_info %d %d %d", attr->port, attr->max_conn, attr->timeout);

	media_info.id = attr->id;
	strcpy(media_info.rtsp_name, attr->rtsp_name);
	media_info.framerate = attr->framerate;
	media_info.bitrate_kbps = attr->bitrate_kbps;
	media_info.video_codec = attr->video_codec;
	media_info.audio_codec = attr->audio_codec;
	media_info.audio_channels = attr->audio_channels;
	media_info.audio_pernum = attr->audio_pernum;
	media_info.audio_sample_rate = attr->audio_sample_rate;
	media_info.rtp_port = 10000;
	media_info.video_port = 20000;
	media_info.audio_port = 21000;
	media_info.cb = rtsp_service_event_cb;
	// media_info.rbuf_inx = 0;
	// media_info.rbuf[FRAME_TYPE_VIDEO] = NULL;
	// media_info.rbuf[FRAME_TYPE_AUDIO] = NULL;
	CVI_LOGD("rtsp_service init media_info %f %d %d", attr->framerate, attr->video_codec, attr->audio_pernum);

	RTSP_Create(&ctx->rtsp_ser, &rtsp_info, &media_info);
	if (ctx->rtsp_ser == NULL) {
		free(ctx->mute_data);
		OSAL_MUTEX_Destroy(ctx->mutex);
		free(ctx);
		OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
		return -1;
	}

	CVI_LOGI("rtsp name %d %s create success", attr->id, ctx->attr.rtsp_name);
	gstRtspSerCtx[attr->id] = ctx;
	*hdl = (RTSP_SERVICE_HANDLE_T *)ctx;
	OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
	return 0;
}

static void rtsp_service_destroy(RTSP_SERVICE_HANDLE_T hdl)
{
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
	if (ctx) {
		OSAL_MUTEX_Lock(&rtsp_ctx_mutex);
		if (gstRtspSerCtx[ctx->attr.id] == NULL || gstRtspSerCtx[ctx->attr.id] != ctx) {
			CVI_LOGI("%s is no exist", ctx->attr.rtsp_name);
			OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
			return;
		}
		RTSP_Destroy(ctx->rtsp_ser);
		OSAL_MUTEX_Unlock(&rtsp_ctx_mutex);
		while (ctx->ref > 0) {
			rtsp_service_stop_media_by_name(ctx->attr.rtsp_name);
		}
		free(ctx->mute_data);
		gstRtspSerCtx[ctx->attr.id] = NULL;
		CVI_LOGD("gstRtspSerCtx reset NULL %d", ctx->attr.id);
		OSAL_MUTEX_Destroy(ctx->mutex);
		free(ctx);
	}
}

static int32_t rtsp_service_update_param(RTSP_SERVICE_HANDLE_T hdl, RTSP_SER_ATTR_S *attr)
{
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
	memcpy(&ctx->attr, attr, sizeof(RTSP_SER_ATTR_S));
	return 0;
}

static int32_t rtsp_service_set_mute(RTSP_SERVICE_HANDLE_T hdl, int32_t mute)
{
	RTSP_SERVICE_CONTEXT_S *ctx = (RTSP_SERVICE_CONTEXT_S *)hdl;
	ctx->mute = mute;
	return 0;
}

static RTSP_VIDEO_FORMAT_E rtsp_service_video_format(RTSP_VIDEO_CODEC_E codec)
{
	switch (codec) {
	case RTSP_VIDEO_CODEC_H264:
		return RTSP_VIDEO_H264;
		break;
	case RTSP_VIDEO_CODEC_H265:
		return RTSP_VIDEO_H265;
		break;
	case RTSP_VIDEO_CODEC_MJPEG:
		return RTSP_VIDEO_MJPEG;
		break;
	default:
		return RTSP_VIDEO_H264;
		break;
	}
}

static RTSP_AUDIO_FORMAT_E rtsp_service_audio_format(RTSP_AUDIO_CODEC_E codec)
{
	switch (codec) {
	case RTSP_AUDIO_CODEC_NONE:
		return RTSP_AUDIO_BUTT;
		break;
	case RTSP_AUDIO_CODEC_PCM:
		return RTSP_AUDIO_PCM;
		break;
	case RTSP_AUDIO_CODEC_AAC:
		return RTSP_AUDIO_AAC;
		break;
	default:
		return RTSP_AUDIO_BUTT;
		break;
	}
}
static void rtsp_service_param2attr(RTSP_SERVICE_PARAM_S *param, RTSP_SER_ATTR_S *attr)
{
	memset(attr, 0x0, sizeof(RTSP_SER_ATTR_S));
	attr->id = param->rtsp_id;
	strncpy(attr->rtsp_name, param->rtsp_name, sizeof(attr->rtsp_name) - 1);
	attr->max_conn = param->max_conn;
	if (attr->max_conn == 0) {
		attr->max_conn = RTSP_INSTANCE_NUM;
	}
	attr->timeout = param->timeout;
	if (attr->timeout == 0) {
		attr->timeout = 120;
	}
	attr->video_codec = rtsp_service_video_format(param->video_codec);
	attr->framerate = param->framerate;
	attr->bitrate_kbps = param->bitrate_kbps;
	attr->audio_codec = rtsp_service_audio_format(param->audio_codec);
	if (RTSP_AUDIO_CODEC_NONE != param->audio_codec) {
		attr->audio_en = 1;
	}
	attr->audio_channels = param->audio_channels;
	attr->audio_sample_rate = param->audio_sample_rate;
	attr->audio_pernum = param->audio_pernum;
	CVI_LOGD("video bitrate %d", attr->bitrate_kbps);
	CVI_LOGD("audio codec %d chn %d rate %d", attr->audio_codec, attr->audio_channels, attr->audio_sample_rate);

	attr->rtsp_play = param->rtsp_play;
	attr->rtsp_play_arg = param->rtsp_play_arg;
	attr->rtsp_teardown = param->rtsp_teardown;
	attr->rtsp_teardown_arg = param->rtsp_teardown_arg;
	attr->venc_hdl = param->venc_hdl;
	attr->port = param->port;
	if (attr->port == 0) {
		attr->port = 554;
	}
	attr->auth_en = 0;
	snprintf(attr->username, sizeof(attr->username), "%s", "root");
	snprintf(attr->password, sizeof(attr->password), "%s", "123456");
}

int32_t RTSP_SERVICE_Create(RTSP_SERVICE_HANDLE_T *hdl, RTSP_SERVICE_PARAM_S *param)
{
	if (param == NULL) {
		*hdl = NULL;
		return -1;
	}
	RTSP_SER_ATTR_S attr;
	rtsp_service_param2attr(param, &attr);
	#ifdef RTSP_NO_AUDIO
    attr.audio_en          = CVI_FALSE;
    attr.audio_codec       = RTSP_AUDIO_PCM;
    attr.audio_sample_rate = 8000;
    attr.audio_channels    = 1;
    attr.audio_pernum      = 320;
	#endif
	return rtsp_service_create(hdl, &attr);
}

int32_t RTSP_SERVICE_Destroy(RTSP_SERVICE_HANDLE_T hdl)
{
	if (hdl == NULL) {
		return -1;
	}
	rtsp_service_destroy(hdl);
	return 0;
}

int32_t RTSP_SERVICE_UpdateParam(RTSP_SERVICE_HANDLE_T hdl, RTSP_SERVICE_PARAM_S *param)
{
	if (hdl == NULL) {
		return -1;
	}
	RTSP_SER_ATTR_S attr;
	rtsp_service_param2attr(param, &attr);
	return rtsp_service_update_param(hdl, &attr);
}

int32_t RTSP_SERVICE_StartMute(RTSP_SERVICE_HANDLE_T hdl)
{
	if (hdl == NULL) {
		return -1;
	}
	return rtsp_service_set_mute(hdl, 1);
}

int32_t RTSP_SERVICE_StopMute(RTSP_SERVICE_HANDLE_T hdl)
{
	if (hdl == NULL) {
		return -1;
	}
	return rtsp_service_set_mute(hdl, 0);
}

int32_t RTSP_SERVICE_StartStop(uint32_t value, char *name)
{
	if (value == 0) {
		rtsp_service_stop_media_by_name(name);
	} else if (value == 1) {
		rtsp_service_start_media_by_name(name);
	}
	return 0;
}

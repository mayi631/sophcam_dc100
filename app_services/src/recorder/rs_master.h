#ifndef __REC_MASTER_H__
#define __REC_MASTER_H__

#include "osal.h"

#include "rs_define.h"
#include "recorder.h"
#include "record_service.h"
#include "filesync.h"

/* rec callback set */
typedef struct REC_CALLBACK_S {
	void *pfnRequestFileNames; /* callback  request file names */
	void *pfnNormalRecCb;
	void *pfnLapseRecCb;
	void *pfnEventRecCb;
	void *pfnGetSubtitleCb;
	void *pfnGetGPSInfoCb;
} REC_CALLBACK_S;

typedef struct _PHOTO_SERVICE_HANDLE_S {
	MAPI_VPROC_HANDLE_T rec_vproc;
	MAPI_VPROC_HANDLE_T sub_rec_vproc;
	MAPI_VPROC_HANDLE_T thumbnail_vproc;
	MAPI_VENC_HANDLE_T rec_venc_hdl;
	MAPI_VENC_HANDLE_T sub_rec_venc_hdl;
	MAPI_VENC_HANDLE_T thumbnail_venc_hdl;
	uint32_t thumbnail_bufsize;
	MAPI_VENC_HANDLE_T piv_venc_hdl;
	uint32_t piv_bufsize;
	int32_t vproc_chn_id_venc;
	CVI_S32 sub_vproc_chn_id_venc;
	int32_t vproc_chn_id_thumbnail;
	RECORD_SERVICE_VENC_BIND_MODE_E venc_bind_mode;
	volatile bool venc_rec_start;
} PHOTO_SERVICE_MAPI_HANDLES_S;

/* record attribute param */
typedef struct REC_ATTR_S {
	RECORD_SERVICE_HANDLE_T rs;
	RECORDER_TYPE_E enRecType; /* record type */
	union {
		RECORDER_NORMAL_ATTR_S stNormalRecAttr; /* normal record attribute */
		RECORDER_LAPSE_ATTR_S stLapseRecAttr;	/* lapse record attribute */
	} unRecAttr;

	RECORDER_SPLIT_ATTR_S stSplitAttr;						  /* record split attribute */
	uint32_t u32StreamCnt;									  /*  stream cnt */
	RECORDER_STREAM_ATTR_S astStreamAttr[REC_STREAM_MAX_CNT]; /*  array of stream attr */
	uint32_t u32PreRecTimeSec;								  /*  pre record time */
	uint32_t u32PostRecTimeSec;								  /*  post record time */
	REC_CALLBACK_S stCallback;
	int32_t s32RecPresize;
	int32_t s32SnapPresize;
	int32_t s32MemRecPreSec;
	uint8_t enable_record_on_start;
	uint8_t enable_perf_on_start;
	uint8_t enable_debug_on_start;
	uint8_t enable_subtitle;
	uint8_t enable_thumbnail;
	uint8_t enable_subvideo;
	int32_t recorder_file_type;
#define CS_PARAM_MAX_FILENAME_LEN (128)
	char recorder_save_dir_base[CS_PARAM_MAX_FILENAME_LEN];
	PHOTO_SERVICE_MAPI_HANDLES_S handles;
	float short_file_ms;
	char devmodel[32];
	char mntpath[32];

	char normal_dir_type[2];
	char park_dir_type[2];
	char event_dir_type[2];
	char snap_dir_type;
	uint32_t flash_led_gpio;
	uint32_t flash_led_pulse;
} REC_ATTR_T;

void *master_create(int32_t id, REC_ATTR_T *attr);
int32_t master_destroy(int32_t id);
int32_t master_update_attr(int32_t id, REC_ATTR_T *attr);
int32_t master_start_normal_rec(int32_t id);
int32_t master_stop_normal_rec(int32_t id);
int32_t master_start_lapse_rec(int32_t id);
int32_t master_stop_lapse_rec(int32_t id);
int32_t master_start_event_rec(int32_t id);
int32_t master_stop_event_rec(int32_t id);
int32_t master_start_mem_rec(int32_t id);
int32_t master_stop_mem_rec(int32_t id);
int32_t master_set_mute(int32_t id);
int32_t master_cancle_mute(int32_t id);
int32_t master_snap(int32_t id, char *file_name);
void master_waitsnap_finish(int32_t id);

#endif // __CS_VIDEO_H__

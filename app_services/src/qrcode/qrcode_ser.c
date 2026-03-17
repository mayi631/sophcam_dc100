#include "osal.h"
#include "qrcode_ser.h"
#include "qrcode.h"

typedef struct qr_scanner_context {
	QRCODE_SERVICE_PARAM_S attr;
	int32_t exitflag;
	OSAL_TASK_HANDLE_S task;
} qr_scanner_context_s;

static void qrcode_scanner_task(void *arg)
{
	qr_scanner_context_s *qr = (qr_scanner_context_s *)arg;
	char result[QRCODE_RESULT_MAX_CNT][QRCODE_RESULT_MAX_LEN];
	uint32_t cnt = 0;
	while (!qr->exitflag) {
		VIDEO_FRAME_INFO_S frame = {0};
		int32_t ret = MAPI_VPROC_GetChnFrame(qr->attr.vproc, qr->attr.vproc_chnid, &frame);
		if (ret != 0) {
			CVI_LOGE("QR[%d]: MAPI_VPROC_GetChnFrame failed", ret);
			OSAL_TASK_Sleep(50 * 1000);
			continue;
		}

		if (qr->attr.w * qr->attr.h < frame.stVFrame.u32Length[0]) {
			CVI_LOGE("Ydata mismatch %u < %u!!!", qr->attr.w * qr->attr.h, frame.stVFrame.u32Length[0]);
			MAPI_ReleaseFrame(&frame);
			OSAL_TASK_Sleep(50 * 1000);
			continue;
		}

		if (MAPI_FrameMmap(&frame, 0) == 0) {
			cnt = 0;
			qrcode_decode(frame.stVFrame.pu8VirAddr[0], result, &cnt);
			MAPI_FrameMunmap(&frame);
		}
		MAPI_ReleaseFrame(&frame);

		if (cnt > 0) {
			for (uint32_t i = 0; i < cnt; i++) {
				CVI_LOGI("result[%d] : %s", i, result[i]);
			}
		}

		OSAL_TASK_Sleep(250 * 1000);
	}
}

int32_t QRCode_Service_Create(QRCODE_SERVICE_HANDLE_T *hdl, QRCODE_SERVICE_PARAM_S *attr)
{
	CVI_LOGI("QRCode_Service_Create start");
	int32_t rc = 0;
	qr_scanner_context_s *qr = (qr_scanner_context_s *)malloc(sizeof(qr_scanner_context_s));
	if (qr == NULL) {
		CVI_LOGE("out of mem for qr %lu", sizeof(qr_scanner_context_s));
		return -1;
	}
	memset(qr, 0x0, sizeof(qr_scanner_context_s));

	qrcode_attr_s qrattr;
	qrattr.w = attr->w;
	qrattr.h = attr->h;
	rc = qrcode_init(&qrattr);
	if (rc != 0) {
		free(qr);
		return -1;
	}

	memcpy(&qr->attr, attr, sizeof(QRCODE_SERVICE_PARAM_S));

	OSAL_MUTEX_ATTR_S qr_ta;
	qr_ta.name = "qr";
	qr_ta.entry = qrcode_scanner_task;
	qr_ta.param = (void *)qr;
	qr_ta.priority = OSAL_TASK_PRI_NORMAL;
	qr_ta.detached = false;
	qr_ta.stack_size = 256 * 1024;
	rc = OSAL_TASK_Create(&qr_ta, &qr->task);
	if (rc != OSAL_SUCCESS) {
		CVI_LOGE("qrcode_scanner_task create failed, %d", rc);
		qrcode_deinit();
		free(qr);
		return -1;
	}
	*hdl = qr;
	CVI_LOGI("QRCode_Service_Create end");
	return 0;
}

int32_t QRCode_Service_Destroy(QRCODE_SERVICE_HANDLE_T hdl)
{
	if (hdl) {
		qr_scanner_context_s *ctx = (qr_scanner_context_s *)hdl;
		ctx->exitflag = 1;
		int32_t rc = OSAL_TASK_Join(ctx->task);
		if (rc != OSAL_SUCCESS) {
			CVI_LOGE("qr_scanner task join failed, %d", rc);
			return -1;
		}
		OSAL_TASK_Destroy(&ctx->task);
		qrcode_deinit();
		free(ctx);
	}
	return 0;
}

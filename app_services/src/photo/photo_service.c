#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <fcntl.h>
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include "dtcf.h"

#include "cvi_ae.h"
#include "cvi_log.h"
#include "exif.h"
#include "photo_service.h"
#include "zoomp.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

/* 曝光程序常量定义 */
#define EXPOSURE_PROGRAM_NOT_DEFINED 0
#define EXPOSURE_PROGRAM_MANUAL 1
#define EXPOSURE_PROGRAM_NORMAL_PROGRAM 2
#define EXPOSURE_PROGRAM_APERTURE_PRIORITY 3
#define EXPOSURE_PROGRAM_SHUTTER_PRIORITY 4
#define EXPOSURE_PROGRAM_CREATIVE_PROGRAM 5
#define EXPOSURE_PROGRAM_ACTION_PROGRAM 6
#define EXPOSURE_PROGRAM_PORTRAIT_MODE 7
#define EXPOSURE_PROGRAM_LANDSCAPE_MODE 8

/* 测光模式常量定义 */
#define METERING_MODE_UNKNOWN 0
#define METERING_MODE_AVERAGE 1
#define METERING_MODE_CENTER_WEIGHTED_AVERAGE 2
#define METERING_MODE_SPOT 3
#define METERING_MODE_MULTI_SEGMENT 4
#define METERING_MODE_PARTIAL 5

/* 白平衡常量定义 */
#define WHITE_BALANCE_AUTO 0
#define WHITE_BALANCE_MANUAL 1

/* 闪光灯常量定义 */
#define FLASH_NO_FLASH 0
#define FLASH_FIRED 1

/**
 * @brief 设置照片 EXIF 扩展参数
 * @details 在 EXIF_MakeExifParam 初始化默认参数后，调用此函数设置更详细的
 *          相机信息、曝光参数等。设置完成后会将 parame 标志设为 true。
 *
 * @param[out] exifInfo EXIF 信息结构体指针
 * @param[in] imgWidth 图像宽度
 * @param[in] imgHeight 图像高度
 *
 * @note 内部使用固定的曝光参数值
 */
static void phs_set_exif_params(EXIF_FILE_INFO_S* exifInfo)
{
    if (exifInfo == NULL) {
        CVI_LOGE("exifInfo is NULL");
        return;
    }

    /* 设置已手动配置标志 */
    exifInfo->parame = 1;
    /* ==================== 相机信息 ==================== */
    /* 照相机制造商 */
    strcpy(exifInfo->Make, " ");
    /* 照相机型号 */
    strcpy(exifInfo->Model, "DC309");
    /* 软件版本 */
    strcpy(exifInfo->Version, "1.0.2");
    /* 版权信息 */
    strcpy(exifInfo->CopyRight, "Photo by Camera");

    /* ==================== 图像基本信息 ==================== */
    exifInfo->Orientation = 1; /* 正常方向 */
    exifInfo->ColorSpace = 1; /* sRGB */

    /* ==================== 光圈值 ==================== */
    exifInfo->FNumberNum = 28;
    exifInfo->FNumberDen = 10;
    exifInfo->ApertureFNumber = 28; /* f/2.8 */

    /* ==================== 曝光时间 ==================== */
    exifInfo->ExposureTimeNum = 1;
    exifInfo->ExposureTimeDen = 500; /* 1/500s */

    /* ==================== ISO 感光度 ==================== */
    exifInfo->ISOSpeedRatings[0] = 100;
    exifInfo->ISOSpeedRatings[1] = 0;

    /* ==================== 焦距 ==================== */
    exifInfo->FocalLengthNum = 5000; /* 50mm */
    exifInfo->FocalLengthDen = 100;

    /* ==================== 测光模式 ==================== */
    exifInfo->MeteringMode = METERING_MODE_SPOT; /* 点测光 */

    /* ==================== 白平衡 ==================== */
    exifInfo->WhiteBalance = WHITE_BALANCE_AUTO; /* 自动 */

    /* ==================== 闪光灯 ==================== */
    exifInfo->Flash = FLASH_NO_FLASH; /* 无闪光 */

    /* ==================== 曝光补偿 ==================== */
    exifInfo->ExposureBiasNum = 0;
    exifInfo->ExposureBiasDen = 10;

    /* ==================== 曝光程序 ==================== */
    exifInfo->ExposureProgram = EXPOSURE_PROGRAM_APERTURE_PRIORITY; /* 光圈优先 */

    //     /* ==================== 主体距离 ==================== */
    //     exifInfo->SubjectDistanceNum = -1;
    //     exifInfo->SubjectDistanceDen = 1;

    /* ==================== 分辨率信息 ==================== */
    exifInfo->XResolutionNum = 300;
    exifInfo->XResolutionDen = 1;
    exifInfo->YResolutionNum = 300;
    exifInfo->YResolutionDen = 1;
    exifInfo->RUnit = 1; /* 英寸 */

    /* ==================== 亮度 ==================== */
    exifInfo->BrightnessNum = 100;
    exifInfo->BrightnessDen = 10;
}

typedef struct thumbnail_buf_s {
    void* buf;
    uint32_t size;
    uint32_t actsize;
} thumbnail_buf_t;

static thumbnail_buf_t g_snap_thumbnail_buf[MAX_CONTEXT_CNT];
static thumbnail_buf_t g_sub_pic_buf[MAX_CONTEXT_CNT];

static phs_context_t gstPhsCtx[MAX_CONTEXT_CNT];

static PHOTO_SERVICE_ATTR_S gst_cvi_phs_attr[MAX_CONTEXT_CNT];

static int32_t phs_get_venc_stream(MAPI_VENC_HANDLE_T vhdl, VENC_STREAM_S *stream)
{
	int32_t ret = MAPI_VENC_GetStreamTimeWait(vhdl, stream, 1000);
	if (ret != 0) {
		CVI_LOGE("[%p]: MAPI_VENC_GetStreamTimeWait failed", vhdl);
		return -1;
	}

	if (stream->u32PackCount <= 0 || stream->u32PackCount > 8) {
		MAPI_VENC_ReleaseStream(vhdl, stream);
		return -1;
	}

	return 0;
}

static int32_t phs_get_id(PHOTO_SERVICE_HANDLE_T phs)
{
	for (int32_t i = 0; i < MAX_CONTEXT_CNT; i++) {
		if (gst_cvi_phs_attr[i].phs == phs) {
			return i;
		}
	}
	return MAX_CONTEXT_CNT;
}

static void APPATTR_2_PHOTOATTR(PHOTO_SERVICE_PARAM_S *param, PHOTO_SERVICE_ATTR_S *attr)
{
	attr->handles.photo_venc_hdl = param->photo_venc_hdl;
	attr->handles.photo_bufsize = param->photo_bufsize;
	attr->handles.enable_dump_raw = param->enable_dump_raw;
	if (attr->handles.photo_bufsize <= 0) {
		attr->handles.photo_bufsize = 1024 * 1024;
	}

	attr->handles.thumbnail_vproc = param->thumbnail_vproc;
	attr->handles.vproc_chn_id_thumbnail = param->vproc_chn_id_thumbnail;
	attr->handles.thumbnail_venc_hdl = param->thumbnail_venc_hdl;
	attr->handles.thumbnail_bufsize = param->thumbnail_bufsize;
	if (attr->handles.thumbnail_bufsize <= 0) {
		attr->handles.thumbnail_bufsize = 128 * 1024;
	}

	attr->handles.sub_pic_vproc = param->sub_pic_vproc;
	attr->handles.vproc_chn_id_sub_pic = param->vproc_chn_id_sub_pic;
	attr->handles.sub_pic_venc_hdl = param->sub_pic_venc_hdl;
	attr->handles.sub_pic_bufsize = param->sub_pic_bufsize;
	if (attr->handles.sub_pic_bufsize <= 0) {
		attr->handles.sub_pic_bufsize = 128 * 1024;
	}

	attr->handles.src_vproc = param->src_vproc;
	attr->handles.src_vproc_chn_id = param->src_vproc_chn_id;

	attr->handles.src_vcap = param->src_vcap;

	attr->handles.scale_vproc = param->scale_vproc;
	attr->handles.scale_vproc_chn_id = param->scale_vproc_chn_id;

	attr->flash_led_gpio = param->flash_led_gpio;
	attr->flash_led_pulse = param->flash_led_pulse;
	attr->flash_led_thres = param->flash_led_thres;

	attr->stCallback.pfnNormalPhsCb = param->cont_photo_event_cb;

	attr->s32SnapPresize = param->prealloclen;
}

static int32_t phs_write_snap_file(char *filename,
	uint8_t *pivdata, uint32_t pivlen,
	uint8_t *thmbdata, uint32_t thmblen,
	uint8_t *sub_pic_data, uint32_t sub_pic_len,
	uint32_t alignsize)
{
	char JPEG_SOI[2] = {0xFF, 0xD8};
	int32_t fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd < 0) {
		CVI_LOGE("open file fail, %s", filename);
		return -1;
	}

	if (0 > write(fd, JPEG_SOI, sizeof(JPEG_SOI))) {
		CVI_LOGE("write error");
		close(fd);
		return -1;
	}

	/* write thumbnail */
	if (thmblen < (0xFFFF - 10)) {
		char JFXX_header[10] = {
			// 10 = 2+2+5+1
			0xFF, 0xE0,								  // APP0 marker
			(thmblen + 8) >> 8, (thmblen + 8) & 0xFF, // Length of segment excluding APP0 marker
			0x4A, 0x46, 0x58, 0x58, 0x00,			  // Identifier,
			0x10									  // Thumbnail format, 1 means jpeg
		};

		if (0 > write(fd, JFXX_header, sizeof(JFXX_header))) {
			CVI_LOGE("write JFXX_header error");
			close(fd);
			return -1;
		}

		if (0 > write(fd, thmbdata, thmblen)) {
			CVI_LOGE("write thumbnail_data error");
			close(fd);
			return -1;
		}
	} else {
		CVI_LOGI("write thumbnail failed\n");
		char JFXX0_header[10] = { // 10 = 2+2+5+1
			0xFF, 0xE0, //APP0 marker
			0x00, 0x08, // Length of segment excluding APP0 marker
			0x4A, 0x46,0x58,0x58, 0x00, // Identifier,
			0x10 // Thumbnail format, 1 means jpeg
		};
		if (0 > write(fd, JFXX0_header, sizeof(JFXX0_header))) {
			CVI_LOGE("write JFXX_header error");
			close(fd);
			return -1;
                }
        }

        /* write EXIF (APP1) - 必须在 APP0 之后、APP2 之前，确保 thumbnail_extractor 能正确解析 */
        EXIF_FILE_INFO_S exifFileInfo;
        char exifData[1164];
        uint32_t totalLen = 0;

        EXIF_MakeExifParam(&exifFileInfo);
        /* 设置 EXIF 扩展参数 */
        phs_set_exif_params(&exifFileInfo);

        if (EXIF_MakeExifFile(exifData, &totalLen, &exifFileInfo) == 0) {
            if (0 > write(fd, exifData, totalLen)) {
                CVI_LOGE("write exif data error");
                close(fd);
                return -1;
            }
            CVI_LOGI("write exif data success, len:%u", totalLen);
        } else {
            CVI_LOGE("generate exif data failed");
        }

        /* write piv */
        CVI_LOGD("%d %X\n", pivlen, (pivlen >> 24) & 0xFF);
        unsigned char JPEG_LEN[8] = { 0xFF, 0xE2, 0x00, 0x06,
            (pivlen >> 24) & 0xFF, (pivlen >> 16) & 0xFF,
            (pivlen >> 8) & 0xFF, (pivlen >> 0) & 0xFF };
        if (0 > write(fd, JPEG_LEN, sizeof(JPEG_LEN))) {
            CVI_LOGE("write piv data end error");
            close(fd);
            return -1;
        }

        // skip SOI, 0xFF, 0xD8
        if (0 > write(fd, pivdata + 2, pivlen - 2)) {
            CVI_LOGE("write piv data error");
            close(fd);
            return -1;
        }

        /* write sub pic, not standard jpeg APPn */
	if (sub_pic_len < 0xFFFFFFFF) {
		char JFXX3_header[6] = {
			0xFF, 0xE3, //APP3 marker
			(sub_pic_len >> 24) & 0xFF, (sub_pic_len >> 16) & 0xFF,
			(sub_pic_len >> 8) & 0xFF,  (sub_pic_len >> 0) & 0xFF,
		};
		CVI_LOGD("phs_write_snap_file sub_pic_len:%d \n", sub_pic_len);
		CVI_LOGD("JFXX3_header[2]:0x%X \n", JFXX3_header[2]);
		CVI_LOGD("JFXX3_header[3]:0x%X \n", JFXX3_header[3]);
		CVI_LOGD("JFXX3_header[4]:0x%X \n", JFXX3_header[4]);
		CVI_LOGD("JFXX3_header[5]:0x%X \n", JFXX3_header[5]);
		if (0 > write(fd, JFXX3_header, sizeof(JFXX3_header))) {
			CVI_LOGE("write JFXX3_header error");
			close(fd);
			return -1;
		}

		if (0 > write(fd, sub_pic_data, sub_pic_len)) {
			CVI_LOGE("write sub pic data error");
			close(fd);
			return -1;
		}
	} else {
		CVI_LOGE("write sub pic failed\n");
	}

	// add useless msg in file
	char JPEG_END[2] = {0xFF, 0xD9};
	if (0 > write(fd, JPEG_END, sizeof(JPEG_END))) {
		CVI_LOGE("write piv data end error");
		close(fd);
		return -1;
	}

	if (pivlen + 8 < alignsize) {
		ftruncate(fd, alignsize);
	}
	fsync(fd);
	close(fd);
	return 0;
}

/**
    pulse: 1~16, see AW3641E spec(1-wire interface)
    time_ms: the time of open
*/
static int32_t photo_flash_led_by_gpio(uint32_t gpio_num, uint32_t pulse, uint32_t is_open){
	uint32_t i = 0;
	uint32_t j = -1;
	int32_t fd = -1;
	char path[64] = {0};

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0){
		CVI_LOGE("open failed");
		return -1;
	}
	snprintf(path, sizeof(path), "%d", gpio_num);
	write(fd, path, strlen(path));
	close(fd);

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio_num);
	fd = open(path, O_WRONLY);
	if(fd < 0){
		CVI_LOGE("open failed");
		return -1;
	}
	write(fd, "out", 3);
	close(fd);

	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio_num);
	fd = open(path, O_RDWR);
	if(fd < 0){
		CVI_LOGE("open failed");
		return -1;
	}

	if(is_open){
		for(i = 0; i < pulse; i++){
			lseek(fd, 0, SEEK_SET);
			write(fd, "0", 1);//2.3us
			for(j = 0; j < 500; j++);// 0.6us per 100
			lseek(fd, 0, SEEK_SET);
			write(fd, "1", 1);//2.3us
			for(j = 0; j < 500; j++);// 0.6us per 100
		}
	} else {
		write(fd, "0", 1);//2.3us
	}

	close(fd);
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if(fd < 0){
		CVI_LOGE("open failed");
		return -1;
	}
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%d", gpio_num);
	write(fd, path, strlen(path));
	close(fd);

	return 0;
}

static int32_t phs_dump_raw_callback(uint32_t ViPipe, VIDEO_FRAME_INFO_S* pstVideoFrame,
                                    uint32_t u32DataNum, void* pPrivateData)
{
	uint32_t i = 0;
	uint32_t frm_num = 1;
	char name[128] = {0};
	FILE *output;

	CVI_LOGI("ViPipe %d, u32DataNum %d, pPrivateData %s", ViPipe, u32DataNum, (char *)pPrivateData);

	if (!pstVideoFrame) {
		return -1;
	}

	if (pstVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
		frm_num = 2;
	}

	strncpy(name, (char *)pPrivateData, 128);
	name[127] = '\0';

	output = fopen(name, "wb");
	if (output == NULL) {
		CVI_LOGE("open %s fail!!!", name);
		return -1;
	}

	for (i = 0; i < frm_num; i++) {
		size_t image_size = pstVideoFrame[i].stVFrame.u32Length[0];
		unsigned char *ptr = calloc(1, image_size);

		MAPI_FrameMmap(&pstVideoFrame[i], false);
		memcpy(ptr, (const void *)pstVideoFrame[i].stVFrame.pu8VirAddr[0],
				pstVideoFrame[i].stVFrame.u32Length[0]);
		MAPI_FrameMunmap(&pstVideoFrame[i]);

		fwrite(ptr, image_size, 1, output);
		free(ptr);
	}

	fclose(output);

	return 0;
}

static int32_t phs_dump_raw(MAPI_VCAP_SENSOR_HANDLE_T sns_hdl, char *filename)
{
	int32_t ret = MAPI_SUCCESS;
	static char filename_buf[128] = {0};
	MAPI_VCAP_RAW_DATA_T stVCapRawData = {0};
	VI_DUMP_ATTR_S stDumpAttr = {0};

	memset(filename_buf, 0, sizeof(filename_buf));
	snprintf(filename_buf, sizeof(filename_buf), "%s.raw", filename);
	filename_buf[sizeof(filename_buf) - 1] = '\0';

	stVCapRawData.pPrivateData = (void*)filename_buf;
	stVCapRawData.pfn_VCAP_RawDataProc = (void *)phs_dump_raw_callback;

	ret = MAPI_VCAP_GetDumpRawAttr(sns_hdl, &stDumpAttr);
	if (ret != MAPI_SUCCESS) {
		CVI_LOGE("MAPI_VCAP_GetDumpRawAttr fail");
		return ret;
	}

	stDumpAttr.bEnable = 1;
	stDumpAttr.u32Depth = 0;
	stDumpAttr.enDumpType = VI_DUMP_TYPE_RAW;
	ret = MAPI_VCAP_SetDumpRawAttr(sns_hdl, &stDumpAttr);
	if (ret != MAPI_SUCCESS) {
		CVI_LOGE("MAPI_VCAP_SetDumpRawAttr fail");
		return ret;
	}

	ret = MAPI_VCAP_StartDumpRaw(sns_hdl, 1, &stVCapRawData);
	if (ret != MAPI_SUCCESS) {
		CVI_LOGE("MAPI_VCAP_StartDumpRaw fail");
		return ret;
	}

	/* wait dump raw finish */
	ret = MAPI_VCAP_StopDumpRaw(sns_hdl);
	if (ret != MAPI_SUCCESS) {
		CVI_LOGE("MAPI_VCAP_StopDumpRaw fail");
		return ret;
	}

	return MAPI_SUCCESS;
}

static void phs_thumb_task_entry(void *arg)
{
	phs_context_handle_t phs = (phs_context_handle_t)arg;
	PHOTO_SERVICE_ATTR_S *p = (PHOTO_SERVICE_ATTR_S *)phs->attr;
	VIDEO_FRAME_INFO_S frame = {0};
	VENC_STREAM_S stream = {0};
	uint8_t *thumbdata = NULL;
	uint32_t *thmbsize = 0;
	uint32_t bufsize = 0;
	int32_t thumbnail_flag = 0;
	while (!phs->shutdown) {
		if (phs->need_thumbnail == 0) {
			OSAL_TASK_Sleep(10 * 1000);
			continue;
		}

		pthread_mutex_lock(&phs->thumbnail_mutex);
		if ((phs->need_thumbnail & 0x1) == 1) {
			thumbdata = (uint8_t *)g_snap_thumbnail_buf[phs->id].buf;
			thmbsize = &g_snap_thumbnail_buf[phs->id].actsize;
			bufsize = g_snap_thumbnail_buf[phs->id].size;
			thumbnail_flag = 0;
		}
		*thmbsize = 0;

		MAPI_VENC_StartRecvFrame(p->handles.thumbnail_venc_hdl, -1);

		int32_t ret = MAPI_VPROC_GetChnFrame(p->handles.thumbnail_vproc, p->handles.vproc_chn_id_thumbnail, &frame);
		if (ret != 0) {
			CVI_LOGE("PHS[%d]: MAPI_VPROC_GetChnFrame failed", phs->id);
			goto END1;
		}

		ret = MAPI_VENC_SendFrame(p->handles.thumbnail_venc_hdl, &frame);
		if (ret != 0) {
			CVI_LOGE("PHS[%d]: MAPI_VENC_SendFrame failed", phs->id);
			goto END;
		}

		ret = MAPI_VENC_GetStream(p->handles.thumbnail_venc_hdl, &stream);
		if (ret != 0) {
			CVI_LOGE("PHS[%d]: MAPI_VENC_GetStream failed", phs->id);
			goto END;
		}

		CVI_LOGD("PHS[%d]: thumbnail buf size %u thmbsize %u\n", phs->id, bufsize, stream.pstPack[0].u32Len);
		if (bufsize < stream.pstPack[0].u32Len) {
			LOG_PHET(MAPI_VENC_ReleaseStream(p->handles.thumbnail_venc_hdl, &stream));
			goto END;
		}

		memcpy(thumbdata, stream.pstPack[0].pu8Addr, stream.pstPack[0].u32Len);
		*thmbsize = stream.pstPack[0].u32Len;
		LOG_PHET(MAPI_VENC_ReleaseStream(p->handles.thumbnail_venc_hdl, &stream));
	END:
		LOG_PHET(MAPI_ReleaseFrame(&frame));
	END1:
		MAPI_VENC_StopRecvFrame(p->handles.thumbnail_venc_hdl);
		if (thumbnail_flag == 0) {
			phs->need_thumbnail &= (~0x1);
		} else if (thumbnail_flag == 1) {
			phs->need_thumbnail &= (~0x2);
		}
		CVI_LOGD("PHS[%d] catch thumbnail %s", phs->id, (*thmbsize > 0) ? ("success") : ("failed"));
		pthread_mutex_unlock(&phs->thumbnail_mutex);
		OSAL_TASK_Sleep(10 * 1000);
	}
	CVI_LOGD("PHS[%d] exit", phs->id);
}

static void phs_sub_pic_task_entry(void *arg)
{
	phs_context_handle_t phs = (phs_context_handle_t)arg;
	PHOTO_SERVICE_ATTR_S *p = (PHOTO_SERVICE_ATTR_S *)phs->attr;
	VIDEO_FRAME_INFO_S frame = {0};
	VENC_STREAM_S stream = {0};
	uint8_t *sub_pic_data = NULL;
	uint32_t *sub_pic_size = 0;
	uint32_t bufsize = 0;
	int32_t sub_pic_flag = 0;
	while (!phs->shutdown) {
		if (phs->need_sub_pic == 0) {
			OSAL_TASK_Sleep(10 * 1000);
			continue;
		}

		pthread_mutex_lock(&phs->sub_pic_mutex);
		if ((phs->need_sub_pic & 0x1) == 1) {
			sub_pic_data = (uint8_t *)g_sub_pic_buf[phs->id].buf;
			sub_pic_size = &g_sub_pic_buf[phs->id].actsize;
			bufsize = g_sub_pic_buf[phs->id].size;
			sub_pic_flag = 0;
		}
		*sub_pic_size = 0;

		MAPI_VENC_StartRecvFrame(p->handles.sub_pic_venc_hdl, -1);

		CVI_LOGI("PHS[%d]: sub pic vproc %p vproc_chn_id %d", phs->id, p->handles.sub_pic_vproc, p->handles.vproc_chn_id_sub_pic);
		int32_t ret = MAPI_VPROC_GetChnFrame(p->handles.sub_pic_vproc, p->handles.vproc_chn_id_sub_pic, &frame);
		if (ret != 0) {
			CVI_LOGE("PHS[%d]: MAPI_VPROC_GetChnFrame failed", phs->id);
			goto END1;
		}

		ret = MAPI_VENC_SendFrame(p->handles.sub_pic_venc_hdl, &frame);
		if (ret != 0) {
			CVI_LOGE("PHS[%d]: MAPI_VENC_SendFrame failed", phs->id);
			goto END;
		}

		ret = MAPI_VENC_GetStream(p->handles.sub_pic_venc_hdl, &stream);
		if (ret != 0) {
			CVI_LOGE("PHS[%d]: MAPI_VENC_GetStream failed", phs->id);
			goto END;
		}

		CVI_LOGD("PHS[%d]: thumbnail buf size %u thmbsize %u\n", phs->id, bufsize, stream.pstPack[0].u32Len);
		if (bufsize < stream.pstPack[0].u32Len) {
			LOG_PHET(MAPI_VENC_ReleaseStream(p->handles.sub_pic_venc_hdl, &stream));
			goto END;
		}

		memcpy(sub_pic_data, stream.pstPack[0].pu8Addr, stream.pstPack[0].u32Len);
		*sub_pic_size = stream.pstPack[0].u32Len;
		LOG_PHET(MAPI_VENC_ReleaseStream(p->handles.sub_pic_venc_hdl, &stream));
	END:
		LOG_PHET(MAPI_ReleaseFrame(&frame));
	END1:
		MAPI_VENC_StopRecvFrame(p->handles.sub_pic_venc_hdl);
		if (sub_pic_flag == 0) {
			phs->need_sub_pic &= (~0x1);
		} else if (sub_pic_flag == 1) {
			phs->need_sub_pic &= (~0x2);
		}
		CVI_LOGD("PHS[%d] catch sub pic %s", phs->id, (*sub_pic_size > 0) ? ("success") : ("failed"));
		pthread_mutex_unlock(&phs->sub_pic_mutex);
		OSAL_TASK_Sleep(10 * 1000);
	}
	CVI_LOGD("PHS[%d] exit", phs->id);
}

static void phs_snap_report_error(PHOTO_SERVICE_EVENT_CALLBACK func, int32_t code)
{
	if(func != NULL){
		func(PHOTO_SERVICE_EVENT_PIV_ERROR, NULL, (void *)(&code));
	}
}

static void phs_snap_task_entry(void *arg)
{
	phs_context_handle_t phs = (phs_context_handle_t)arg;
	PHOTO_SERVICE_ATTR_S *p = (PHOTO_SERVICE_ATTR_S *)phs->attr;

	uint8_t *thumbnail_data = (uint8_t *)g_snap_thumbnail_buf[phs->id].buf;
	uint32_t *thumbnail_len = &g_snap_thumbnail_buf[phs->id].actsize;
	uint8_t *sub_pic_data = (uint8_t *)g_sub_pic_buf[phs->id].buf;
	uint32_t *sub_pic_len = &g_sub_pic_buf[phs->id].actsize;
	VIDEO_FRAME_INFO_S vcap_frame = {0};
	VENC_STREAM_S stream = {0};
	VPSS_GRP src_vproc_grp = 0;
	int32_t ret = 0;
	int32_t timeout_cnt = 0;

	src_vproc_grp = MAPI_VPROC_GetGrp(p->handles.src_vproc);
	CVI_LOGI("PHS[%d]: src_vproc_grp %d, src_vproc_chn_id %d", phs->id, src_vproc_grp, p->handles.src_vproc_chn_id);
	// ret = MAPI_VENC_BindVproc(p->handles.photo_venc_hdl, src_vproc_grp, p->handles.src_vproc_chn_id);
	// if (ret != MAPI_SUCCESS) {
	// 	CVI_LOGE("PHS[%d]: MAPI_VENC_BindVproc failed", phs->id);
	// }

	while (!phs->shutdown) {
		// OSAL_TASK_Sleep(20 * 1000);
		pthread_mutex_lock(&phs->piv_mutex);
		pthread_cond_wait(&phs->piv_cond, &phs->piv_mutex);
		if (phs->shutdown == 1) {
			CVI_LOGI("PHS[%d] shutdown", phs->id);
			phs->piv_finish = 1;
			pthread_mutex_unlock(&phs->piv_mutex);
			break;
		}

		if (p->stCallback.pfnNormalPhsCb) {
			((PHOTO_SERVICE_EVENT_CALLBACK)p->stCallback.pfnNormalPhsCb)(PHOTO_SERVICE_EVENT_PIV_START, phs->piv_filename, (void *)(&phs->id));
		}

		if (p->handles.photo_venc_hdl == NULL) {
			CVI_LOGE("null snapshot venc hdl");
			phs->piv_finish = 1;
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}

		ret = MAPI_VENC_StartRecvFrame(p->handles.photo_venc_hdl, -1);
		if (ret != MAPI_SUCCESS) {
			CVI_LOGE("PHS[%d]: MAPI_VENC_StartRecvFrame failed", phs->id);
			phs->piv_finish = 1;
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}

		ret = MAPI_VCAP_GetFrame(p->handles.src_vcap, &vcap_frame);
		if (ret != MAPI_SUCCESS) {
			CVI_LOGE("PHS[%d]: MAPI_VCAP_GetFrame failed", phs->id);
			phs->piv_finish = 1;
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}
		CVI_LOGI("PHS[%d]: vcap_frame width %d height %d", phs->id, vcap_frame.stVFrame.u32Width, vcap_frame.stVFrame.u32Height);

		ret = MAPI_VPROC_SendFrame(p->handles.src_vproc, &vcap_frame);
		if (ret != MAPI_SUCCESS) {
			CVI_LOGE("PHS[%d]: MAPI_VPROC_SendFrame failed", phs->id);
			phs->piv_finish = 1;
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}
		MAPI_VCAP_ReleaseFrame(p->handles.src_vcap, &vcap_frame);

		/* main picture */
		if (0 > phs_get_venc_stream(p->handles.photo_venc_hdl, &stream)) {
			CVI_LOGE("PHS[%d]: snapshot get venc stream fail", phs->id);
			MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
			phs->piv_finish = 1;
			*thumbnail_len = 0;
			*sub_pic_len = 0;
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}

		uint32_t len = stream.pstPack[0].u32Len;
		CVI_LOGI("PHS[%d] get venc stream success, len %u", phs->id, len);

		/* sub pic */
		CVI_LOGI("PHS[%d] start get sub pic", phs->id);
		*sub_pic_len = 0;
		phs->need_sub_pic |= 0x1;
		timeout_cnt = 0;
		while (*sub_pic_len == 0) {
			if (phs->shutdown == 1 || timeout_cnt++ > 20) {
				CVI_LOGI("PHS[%d] shutdown, timeout_cnt %d", phs->id, timeout_cnt);
				phs->piv_finish = 1;
				pthread_mutex_unlock(&phs->piv_mutex);
				break;
			}
			OSAL_TASK_Sleep(10 * 1000);
		}
		if (*sub_pic_len == 0) {
			phs->piv_finish = 1;
			MAPI_VENC_ReleaseStream(p->handles.photo_venc_hdl, &stream);
			MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}
		CVI_LOGI("PHS[%d] get sub pic success, len %u", phs->id, *sub_pic_len);

		/* thumbnail */
		CVI_LOGI("PHS[%d] start get thumbnail", phs->id);
		*thumbnail_len = 0;
		phs->need_thumbnail |= 0x1;
		timeout_cnt = 0;
		while (*thumbnail_len == 0) {
			if (phs->shutdown == 1 || timeout_cnt++ > 20) {
				CVI_LOGI("PHS[%d] shutdown, timeout_cnt %d", phs->id, timeout_cnt);
				phs->piv_finish = 1;
				pthread_mutex_unlock(&phs->piv_mutex);
				break;
			}
			OSAL_TASK_Sleep(10 * 1000);
		}
		if (*thumbnail_len == 0) {
			phs->piv_finish = 1;
			MAPI_VENC_ReleaseStream(p->handles.photo_venc_hdl, &stream);
			MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}
		CVI_LOGI("PHS[%d] get thumbnail success, len %u", phs->id, *thumbnail_len);

		/* packet and write file */
		if (phs_write_snap_file(phs->piv_filename,
				stream.pstPack[0].pu8Addr, len,
				thumbnail_data, *thumbnail_len,
				sub_pic_data, *sub_pic_len,
				phs->piv_prealloclen) < 0) {
			phs->piv_finish = 1;
			*thumbnail_len = 0;
			*sub_pic_len = 0;
			MAPI_VENC_ReleaseStream(p->handles.photo_venc_hdl, &stream);
			MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);
			phs_snap_report_error(p->stCallback.pfnNormalPhsCb, -1);
			pthread_mutex_unlock(&phs->piv_mutex);
			continue;
		}
		MAPI_VENC_ReleaseStream(p->handles.photo_venc_hdl, &stream);
		MAPI_VENC_StopRecvFrame(p->handles.photo_venc_hdl);

		/* dump raw */
		if (p->handles.enable_dump_raw) {
			phs_dump_raw(p->handles.src_vcap, phs->piv_filename);
		}

		if (p->stCallback.pfnNormalPhsCb) {
			((PHOTO_SERVICE_EVENT_CALLBACK)p->stCallback.pfnNormalPhsCb)(PHOTO_SERVICE_EVENT_PIV_END, phs->piv_filename, (void *)(&phs->id));
		}
		CVI_LOGD("PHS[%d] snap success", phs->id);
		phs->piv_finish = 1;
		*thumbnail_len = 0;
		*sub_pic_len = 0;
		pthread_mutex_unlock(&phs->piv_mutex);
	}

	CVI_LOGD("PHS[%d] exit", phs->id);
}

static void phs_state_task_entry(void *arg)
{

	phs_context_handle_t phs = (phs_context_handle_t)arg;
	while (!phs->shutdown) {
		// handle transition
		pthread_mutex_lock(&phs->state_mutex);
		if (phs->new_state != phs->cur_state) {
			phs->cur_state = phs->new_state;
			CVI_LOGI("PHS[%d]: PHS_STATE: change to 0x%x done", phs->id, phs->cur_state);
		}
		pthread_mutex_unlock(&phs->state_mutex);
		OSAL_TASK_Sleep(20 * 1000);
	}
	CVI_LOGD("PHS[%d] exit", phs->id);
}

static int32_t phs_start_task(phs_context_handle_t phs)
{
	OSAL_TASK_ATTR_S piv_ta;
	static char piv_name[16] = {0};
	snprintf(piv_name, sizeof(piv_name), "phs_piv_%d", phs->id);
	piv_ta.name = piv_name;
	piv_ta.entry = phs_snap_task_entry;
	piv_ta.param = (void *)phs;
	piv_ta.priority = OSAL_TASK_PRI_RT_MID;
	piv_ta.detached = false;
	piv_ta.stack_size = 256 * 1024;
	int32_t pc = OSAL_TASK_Create(&piv_ta, &phs->piv_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("phs_snap task create failed, %d", pc);
		return PHS_ERR_FAILURE;
	}

	OSAL_TASK_ATTR_S thumb_ta;
	static char thumb_name[16] = {0};
	snprintf(thumb_name, sizeof(thumb_name), "phs_thumb_%d", phs->id);
	thumb_ta.name = thumb_name;
	thumb_ta.entry = phs_thumb_task_entry;
	thumb_ta.param = (void *)phs;
	thumb_ta.priority = OSAL_TASK_PRI_NORMAL;
	thumb_ta.detached = false;
	thumb_ta.stack_size = 256 * 1024;
	pc = OSAL_TASK_Create(&thumb_ta, &phs->thumb_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("phs_thumb task create failed, %d", pc);
		return PHS_ERR_FAILURE;
	}

	OSAL_TASK_ATTR_S sub_pic_ta;
	static char sub_pic_name[16] = {0};
	snprintf(sub_pic_name, sizeof(sub_pic_name), "phs_sub_pic_%d", phs->id);
	sub_pic_ta.name = sub_pic_name;
	sub_pic_ta.entry = phs_sub_pic_task_entry;
	sub_pic_ta.param = (void *)phs;
	sub_pic_ta.priority = OSAL_TASK_PRI_NORMAL;
	sub_pic_ta.detached = false;
	sub_pic_ta.stack_size = 256 * 1024;
	pc = OSAL_TASK_Create(&sub_pic_ta, &phs->sub_pic_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("phs_sub_pic task create failed, %d", pc);
		return PHS_ERR_FAILURE;
	}

	static char state_name[16] = {0};
	snprintf(state_name, sizeof(state_name), "phs_state_%d", phs->id);
	OSAL_TASK_ATTR_S ta;
	ta.name = state_name;
	ta.entry = phs_state_task_entry;
	ta.param = (void *)phs;
	ta.priority = OSAL_TASK_PRI_RT_HIGH;
	ta.detached = false;
	ta.stack_size = 256 * 1024;
	pc = OSAL_TASK_Create(&ta, &phs->state_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("phs_state task create failed, %d", pc);
		return PHS_ERR_FAILURE;
	}

	return PHS_SUCCESS;
}

static void *phs_create(int32_t id, PHOTO_SERVICE_ATTR_S *attr)
{
	if (id >= MAX_CONTEXT_CNT) {
		return 0;
	}
	memset(&gstPhsCtx[id], 0x0, sizeof(gstPhsCtx[id]));
	phs_context_handle_t handle = &gstPhsCtx[id];
	handle->attr = attr;
	handle->id = id;

	if (g_snap_thumbnail_buf[id].size == 0) {
		g_snap_thumbnail_buf[id].buf = malloc(attr->handles.thumbnail_bufsize);
		if (g_snap_thumbnail_buf[id].buf == NULL) {
			CVI_LOGE("snap thumbnail %d %u malloc failed, OOM!!!", id, attr->handles.thumbnail_bufsize);
			return NULL;
		}
		g_snap_thumbnail_buf[id].size = attr->handles.thumbnail_bufsize;
		CVI_LOGD("snap thumbnail size %u", g_snap_thumbnail_buf[id].size);
	}

	if (g_sub_pic_buf[id].size == 0) {
		g_sub_pic_buf[id].buf = malloc(attr->handles.sub_pic_bufsize);
		if (g_sub_pic_buf[id].buf == NULL) {
			CVI_LOGE("sub pic buf %d %u malloc failed, OOM!!!", id, attr->handles.sub_pic_bufsize);
			return NULL;
		}
		g_sub_pic_buf[id].size = attr->handles.sub_pic_bufsize;
		CVI_LOGD("sub pic buf size %u", g_sub_pic_buf[id].size);
	}

	pthread_mutex_init(&handle->state_mutex, NULL);
	pthread_mutex_init(&handle->piv_mutex, NULL);
	pthread_condattr_t piv_condattr;
	pthread_condattr_init(&piv_condattr);
	pthread_condattr_setclock(&piv_condattr, CLOCK_MONOTONIC);
	pthread_cond_init(&handle->piv_cond, &piv_condattr);
	pthread_condattr_destroy(&piv_condattr);

	pthread_mutex_init(&handle->thumbnail_mutex, NULL);
	handle->cur_state = PHS_STATE_IDLE;
	handle->piv_prealloclen = attr->s32SnapPresize;

	phs_start_task(handle);

	uint32_t enable_states = 0;
	enable_states |= PHS_STATE_PHOTO_CREATE_EN;

	phs_enable_state(handle, enable_states);
	return (void *)handle;
}

int32_t PHOTO_SERVICE_Create(PHOTO_SERVICE_HANDLE_T *hdl, PHOTO_SERVICE_PARAM_S *param)
{
	if (param->photo_id >= MAX_CONTEXT_CNT) {
		return -1;
	}
	PHOTO_SERVICE_ATTR_S *attr = &gst_cvi_phs_attr[param->photo_id];
	APPATTR_2_PHOTOATTR(param, attr);
	attr->phs = phs_create(param->photo_id, attr);
	*hdl = attr->phs;
	return (*hdl != NULL) ? 0 : -1;
}

static int32_t phs_destroy(int32_t id)
{
	if (id >= MAX_CONTEXT_CNT) {
		return 0;
	}

	phs_context_handle_t handle = &gstPhsCtx[id];

	CVI_LOGD("PHS[%d] destroy start", id);
	phs_change_state(handle, PHS_STATE_IDLE);
	handle->shutdown = 1;
	int32_t pc = OSAL_TASK_Join(handle->state_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("phs_state task join failed, %d", pc);
		return PHS_ERR_FAILURE;
	}
	OSAL_TASK_Destroy(&handle->state_task);
	pc = OSAL_TASK_Join(handle->thumb_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("thumbnail task join failed, %d", pc);
		return PHS_ERR_FAILURE;
	}
	OSAL_TASK_Destroy(&handle->thumb_task);

	pthread_mutex_lock(&handle->piv_mutex);
	pthread_cond_signal(&handle->piv_cond);
	pthread_mutex_unlock(&handle->piv_mutex);
	pc = OSAL_TASK_Join(handle->piv_task);
	if (pc != OSAL_SUCCESS) {
		CVI_LOGE("phs_piv task join failed, %d", pc);
		return PHS_ERR_FAILURE;
	}
	OSAL_TASK_Destroy(&handle->piv_task);
	CVI_LOGD("PHS[%d] destroy start 1", id);
	pthread_mutex_destroy(&handle->state_mutex);
	pthread_mutex_destroy(&handle->piv_mutex);
	pthread_cond_destroy(&handle->piv_cond);
	pthread_mutex_destroy(&handle->thumbnail_mutex);

	if (g_snap_thumbnail_buf[id].buf) {
		free(g_snap_thumbnail_buf[id].buf);
		g_snap_thumbnail_buf[id].buf = NULL;
		CVI_LOGD("## free g_snap_thumbnail_buf %d %u", id, g_snap_thumbnail_buf[id].size);
	}
	g_snap_thumbnail_buf[id].actsize = 0;
	g_snap_thumbnail_buf[id].size = 0;

	if (g_sub_pic_buf[id].buf) {

		free(g_sub_pic_buf[id].buf);
		g_sub_pic_buf[id].buf = NULL;
		CVI_LOGD("## free g_sub_pic_buf %d %u", id, g_sub_pic_buf[id].size);
	}
	g_sub_pic_buf[id].actsize = 0;
	g_sub_pic_buf[id].size = 0;

	CVI_LOGE("PHS[%d]end", id);

	return pc;
}

int32_t PHOTO_SERVICE_Destroy(PHOTO_SERVICE_HANDLE_T hdl)
{
	return phs_destroy(phs_get_id(hdl));
}

int32_t PHOTO_SERVICE_AdjustFocus(PHOTO_SERVICE_HANDLE_T hdl , char* ratio)
{
	int32_t ret = 0;
	VPSS_CROP_INFO_S st_crop_info = {0};
	VPSS_GRP vpss_grp = 0;
	VPSS_GRP_ATTR_S st_grp_attr = {0};
	PHOTO_SERVICE_ATTR_S phattr = gst_cvi_phs_attr[phs_get_id(hdl)];
	float focus_ratio = 0;
	RECT_S crop_in = {0}, crop_out = {0};

	focus_ratio = atof(ratio);
	vpss_grp = MAPI_VPROC_GetGrp(phattr.handles.src_vproc);
	CVI_LOGI("vpss_grp %d focus_ratio %f", vpss_grp, focus_ratio);

	ret = MAPI_VPROC_GetGrpAttr(phattr.handles.src_vproc, &st_grp_attr);
	if(ret != MAPI_SUCCESS){
		CVI_LOGE("MAPI_VPROC_GetGrpAttr failed");
		return PHS_ERR_FAILURE;
	}

	CVI_LOGI("st_grp_attr.u32MaxW %d u32MaxH %d", st_grp_attr.u32MaxW, st_grp_attr.u32MaxH);

	ret = MAPI_VPROC_GetGrpCrop(phattr.handles.src_vproc, &st_crop_info);
	if(ret != MAPI_SUCCESS){
		CVI_LOGE("CVI_VPSS_GetGrpCrop failed");
		return PHS_ERR_FAILURE;
	}

	if(((!st_crop_info.bEnable) && focus_ratio < 2)
		|| focus_ratio > ZOOM_MAX_RADIO){
		CVI_LOGI("don't need to crop: %d", st_crop_info.bEnable);
		return PHS_ERR_FAILURE;
	}

	if(st_crop_info.bEnable){
		crop_in.s32X = st_crop_info.stCropRect.s32X;
		crop_in.s32Y = st_crop_info.stCropRect.s32Y;
		crop_in.u32Width = st_crop_info.stCropRect.u32Width;
		crop_in.u32Height = st_crop_info.stCropRect.u32Height;
	}else{
		crop_in.s32X = 0;
		crop_in.s32Y = 0;
		crop_in.u32Width = st_grp_attr.u32MaxW;
		crop_in.u32Height = st_grp_attr.u32MaxH;
	}

	if(!ZOOMP_Is_Init()){
		ZOOMP_Init(crop_in);
	}

	if(ZOOMP_GetCropInfo(crop_in, &crop_out, focus_ratio)){
		return PHS_ERR_FAILURE;
	}

	if(crop_out.u32Width < 64 || crop_out.u32Height < 64){
		CVI_LOGE("crop is too small");
		return PHS_ERR_FAILURE;
	}

	st_crop_info.bEnable = CVI_TRUE;
	st_crop_info.enCropCoordinate = VPSS_CROP_ABS_COOR;
	st_crop_info.stCropRect = crop_out;
	ret = MAPI_VPROC_SetGrpCrop(phattr.handles.src_vproc, &st_crop_info);
	if(ret != MAPI_SUCCESS){
		CVI_LOGE("CVI_VPSS_SetGrpCrop failed");
		return PHS_ERR_FAILURE;
	}

	return PHS_SUCCESS;
}

static int32_t phs_snap(int32_t id, char *file_name)
{
	int32_t ret = 0;
	if (id >= MAX_CONTEXT_CNT) {
		return 0;
	}
	phs_context_handle_t handle = &gstPhsCtx[id];

	pthread_mutex_lock(&handle->piv_mutex);
	memset(handle->piv_filename, 0, sizeof(handle->piv_filename));
	snprintf(handle->piv_filename, sizeof(handle->piv_filename), "%s", file_name);

	if (0 == strlen(handle->piv_filename)) {
		CVI_LOGE("get snap filename failed! \n");
		ret = -1;
	}
	handle->piv_finish = 0;
	pthread_mutex_unlock(&handle->piv_mutex);
	pthread_cond_signal(&handle->piv_cond);
	CVI_LOGD("%d snap_finish", id);
	return ret;
}

int32_t PHOTO_SERVICE_PivCapture(PHOTO_SERVICE_HANDLE_T hdl, char *file_name)
{
	if (!hdl) {
		return -1;
	}
	return phs_snap(phs_get_id(hdl), file_name);
}
static void phs_waitsnap_finish(int32_t id)
{
	if (id >= MAX_CONTEXT_CNT) {
		CVI_LOGE("PHOTO ID of fix photomode is error\n");
		return;
	}
	phs_context_handle_t handle = &gstPhsCtx[id];
	CVI_LOGD("%d waitsnap_start", id);
	while (!handle->piv_finish) {
		OSAL_TASK_Sleep(20 * 1000);
	}
	CVI_LOGD("%d waitsnap_finish", id);
}

void PHOTO_SERVICE_WaitPivFinish(PHOTO_SERVICE_HANDLE_T hdl)
{
	if (!hdl) {
		CVI_LOGE("photo service handle is null !\n");
		return;
	}
	phs_waitsnap_finish(phs_get_id(hdl));
}

int32_t PHOTO_SERVICE_SetFlashLed(PHOTO_SERVICE_HANDLE_T hdl, PHOTO_SERVICE_FLASH_LED_MODE_E mode)
{
	int32_t is_open = 0;
	int16_t lv = 0;
	VI_PIPE ViPipe = 0;
	PHOTO_SERVICE_ATTR_S phattr = gst_cvi_phs_attr[phs_get_id(hdl)];

	if(mode == PHOTO_SERVICE_FLASH_LED_MODE_NP){
		is_open = 1;
	}else if(mode == PHOTO_SERVICE_FLASH_LED_MODE_NC){
		is_open = 0;
	}else if(mode == PHOTO_SERVICE_FLASH_LED_MODE_AUTO){
		ViPipe = MAPI_VCAP_GetSensorPipe(phattr.handles.src_vcap);
		CVI_ISP_GetCurrentLvX100(ViPipe, &lv);
		CVI_LOGI("lv:%d thres:%d", lv, phattr.flash_led_thres);
		/* day */
		if(lv > phattr.flash_led_thres){
			return -1;
		} else { /* night */
			is_open = 1;
		}
	}
	CVI_LOGI("flash(%d %d %d)", phattr.flash_led_gpio, phattr.flash_led_pulse, is_open);
	photo_flash_led_by_gpio(phattr.flash_led_gpio, phattr.flash_led_pulse, is_open);
	return 0;
}

int32_t PHOTO_SERVICE_SetQuality(PHOTO_SERVICE_HANDLE_T hdl, uint32_t quality)
{
	int32_t ret = 0;
	VENC_JPEG_PARAM_S stJpegParam = {0};
	PHOTO_SERVICE_ATTR_S phattr = gst_cvi_phs_attr[phs_get_id(hdl)];

	ret = MAPI_VENC_GetJpegParam(phattr.handles.photo_venc_hdl, &stJpegParam);
	if (ret != MAPI_SUCCESS) {
		CVI_LOGE("VENC_GetJpegParam fail");
		return -1;
	} else {
		CVI_LOGI("Current Qfactor: %d", stJpegParam.u32Qfactor);
		stJpegParam.u32Qfactor = quality;
		CVI_LOGI("Set Qfactor: %d", stJpegParam.u32Qfactor);
		ret = MAPI_VENC_SetJpegParam(phattr.handles.photo_venc_hdl, &stJpegParam);
		if (ret != MAPI_SUCCESS) {
			CVI_LOGE("VENC_SetJpegParam fail");
			return -1;
		}
	}

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

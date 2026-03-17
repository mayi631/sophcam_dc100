#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include "cvi_comm_video.h"
#include "media_init.h"
#include "media_osd.h"
#include "cvi_vi.h"
#include "cvi_sys.h"
#include "system.h"
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"

#ifdef ENABLE_ISP_PQ_TOOL
#include "mode.h"
int32_t MEDIA_DUMP_SetDumpRawAttr(int32_t sns_id)
{
    int32_t s32Ret = 0;
    VI_DUMP_ATTR_S astDumpAttr[2];
    MAPI_VCAP_SENSOR_HANDLE_T g_sns_hdl = MEDIA_GetCtx()->SysHandle.sns[sns_id];
    memset(&astDumpAttr, 0, sizeof(VI_DUMP_ATTR_S) * 2);

    astDumpAttr[0].bEnable    = APP_TRUE;
    astDumpAttr[0].u32Depth   = 0;
    astDumpAttr[0].enDumpType = VI_DUMP_TYPE_RAW;

    s32Ret = MAPI_VCAP_SetDumpRawAttr(g_sns_hdl, &astDumpAttr[0]);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_VCAP_SetDumpRawAttr fail");

    s32Ret = MAPI_VCAP_GetDumpRawAttr(g_sns_hdl, &astDumpAttr[1]);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_VCAP_GetDumpRawAttr fail");
    if ((astDumpAttr[0].bEnable != astDumpAttr[1].bEnable) ||
        (astDumpAttr[0].u32Depth != astDumpAttr[1].u32Depth) ||
        (astDumpAttr[0].enDumpType != astDumpAttr[1].enDumpType)) {
        CVI_LOGE("Incorrect dump raw attr for sns[%d]\n", sns_id);
        return -1;
    }

    return 0;
}

static char* MEDIA_DUMP_GetSdConfig(void)
{
	static char path[128] = {0};
	STG_DEVINFO_S SDParam = {0};
	PARAM_GetStgInfoParam(&SDParam);
	memcpy(path, SDParam.aszMntPath, strlen(SDParam.aszMntPath));
	return path;
}

static void get_file_name_prefix(int32_t sns_id, char* prefix, uint32_t len)
{
	struct tm *t;
	time_t tt;

	ISP_PUB_ATTR_S stPubAttr;
	ISP_EXP_INFO_S stExpInfo;

	const char* pBayerStr = NULL;
	const char* pMode = NULL;

	MEDIA_SYSHANDLE_S *Syshdl = &MEDIA_GetCtx()->SysHandle;
    int32_t status = 0;
    MAPI_VCAP_GetSensorPipeAttr(Syshdl->sns[sns_id], &status);
    if (0 == status) {
        CVI_LOGE("stViPipeAttr.bYuvBypassPath is true, yuv sensor skip isp ops");
    }

	memset(&stPubAttr, 0, sizeof(ISP_PUB_ATTR_S));
	CVI_ISP_GetPubAttr(sns_id, &stPubAttr);

	memset(&stExpInfo, 0, sizeof(ISP_EXP_INFO_S));
	CVI_ISP_QueryExposureInfo(sns_id, &stExpInfo);

	switch(stPubAttr.enBayer) {
	case BAYER_BGGR:
		pBayerStr = "BGGR";
		break;
	case BAYER_GBRG:
		pBayerStr = "GBRG";
		break;
	case BAYER_GRBG:
		pBayerStr = "GRBG";
		break;
	case BAYER_RGGB:
		pBayerStr = "RGGB";
		break;
	default:
		pBayerStr = "XXOO";
		break;
	}

	if (stPubAttr.enWDRMode == WDR_MODE_NONE) {
		pMode = "Linear";
	} else if (stPubAttr.enWDRMode >= WDR_MODE_2To1_LINE) {
		pMode = "WDR";
	} else {
		pMode = "XXOO";
	}

	time(&tt);
	t = localtime(&tt);

	snprintf(prefix, len,
		"%dX%d_%s_%s_-color=%d_-bits=12_-frame=1_-hdr=%d_ISO=%d_%04d%02d%02d%02d%02d%02d",
		stPubAttr.enWDRMode == WDR_MODE_NONE ? stPubAttr.stWndRect.u32Width : stPubAttr.stWndRect.u32Width * 2,
		stPubAttr.stWndRect.u32Height,
		pBayerStr, pMode, stPubAttr.enBayer,
		stPubAttr.enWDRMode == WDR_MODE_NONE ? 0 : 1,
		stExpInfo.u32ISO,
		t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
		t->tm_hour, t->tm_min, t->tm_sec);
}

// static void dump_log(VI_PIPE ViPipe, char* prefix)
// {
// 	char name[128] = {0};
// 	FILE *fp = NULL;

// 	uint8_t *buf = NULL;
//     snprintf(name, 128, "%s%s.json", MEDIA_DUMP_GetSdConfig(), prefix);
// 	fp = fopen(name, "w");
// 	if (fp == NULL) {
// 		printf("open %s fail!!!\n", name);
// 		return;
// 	}

// 	VI_DUMP_REGISTER_TABLE_S reg_tbl;
// 	ISP_INNER_STATE_INFO_S pstInnerStateInfo;

// 	CVI_ISP_QueryInnerStateInfo(ViPipe, &pstInnerStateInfo);
// 	reg_tbl.MlscGainLut.RGain = pstInnerStateInfo.mlscGainTable.RGain;
// 	reg_tbl.MlscGainLut.GGain = pstInnerStateInfo.mlscGainTable.GGain;
// 	reg_tbl.MlscGainLut.BGain = pstInnerStateInfo.mlscGainTable.BGain;

// 	CVI_VI_DumpHwRegisterToFile(ViPipe, fp, &reg_tbl);
// 	fclose(fp);

// 	snprintf(name, 128, "%s%s.txt", MEDIA_DUMP_GetSdConfig(), prefix);
// 	fp = fopen(name, "w");
// 	if (fp == NULL) {
// 		printf("open %s fail!!!\n", name);
// 		return;
// 	}

// 	CVI_ISP_DumpFrameRawInfoToFile(ViPipe, fp);
// 	fclose(fp);

// 	uint32_t size = CVI_ISP_GetAWBDbgBinSize();

// 	if ((AE_SNAP_LOG_BUFF_SIZE + AE_LOG_BUFF_SIZE) > size) {
// 		size = AE_SNAP_LOG_BUFF_SIZE + AE_LOG_BUFF_SIZE;
// 	}

// 	if (AWB_SNAP_LOG_BUFF_SIZE > size) {
// 		size = AWB_SNAP_LOG_BUFF_SIZE;
// 	}

// 	buf = calloc(1, size);
// 	if (buf == NULL) {
// 		printf("calloc buf fail!!!\n");
// 		return;
// 	}

// 	snprintf(name, 128, "%s%s-awb.bin", MEDIA_DUMP_GetSdConfig(), prefix);
// 	fp = fopen(name, "w");
// 	if (fp == NULL) {
// 		printf("open %s fail!!!\n", name);
// 		return;
// 	}

// 	CVI_ISP_GetAWBDbgBinBuf(ViPipe, buf, CVI_ISP_GetAWBDbgBinSize());

// 	fwrite(buf, 1, CVI_ISP_GetAWBDbgBinSize(), fp);
// 	fclose(fp);

// 	snprintf(name, 128, "%s%s-awblog.txt", MEDIA_DUMP_GetSdConfig(), prefix);
// 	fp = fopen(name, "w");
// 	if (fp == NULL) {
// 		printf("open %s fail!!!\n", name);
// 		return;
// 	}

// 	CVI_ISP_GetAWBSnapLogBuf(ViPipe, buf, AWB_SNAP_LOG_BUFF_SIZE);

// 	fwrite(buf, 1, AWB_SNAP_LOG_BUFF_SIZE, fp);
// 	fclose(fp);

// 	snprintf(name, 128, "%s%s-aelog.txt", MEDIA_DUMP_GetSdConfig(), prefix);
// 	fp = fopen(name, "w");
// 	if (fp == NULL) {
// 		printf("open %s fail!!!\n", name);
// 		return;
// 	}

// 	CVI_ISP_GetAELogBuf(ViPipe, buf, (AE_SNAP_LOG_BUFF_SIZE + AE_LOG_BUFF_SIZE), AUTO_MODE);

// 	fwrite(buf, 1, (AE_SNAP_LOG_BUFF_SIZE + AE_LOG_BUFF_SIZE), fp);
// 	fclose(fp);

// 	free(buf);
// }

static int32_t MEDIA_RAWDATA_DumpCallback(uint32_t ViPipe, VIDEO_FRAME_INFO_S* pstVideoFrame,
                                    uint32_t u32DataNum, void* pPrivateData)
{
    uint32_t i = 0;
    uint32_t frm_num = 1;
	char prefix[128] = {0};
	char name[128] = {0};
	FILE *output;

    if (!pstVideoFrame) {
        return -1;
    }

    if (pstVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
         frm_num = 2;
    }

	get_file_name_prefix(ViPipe, prefix, 128);
	// dump_log(ViPipe, prefix);

	snprintf(name, 128, "%s%s.raw", MEDIA_DUMP_GetSdConfig(), prefix);

	output = fopen(name, "wb");
	if (output == NULL) {
		printf("open %s fail!!!\n", name);
		return -1;
	}

    for (i = 0; i < frm_num; i++) {
        size_t image_size = pstVideoFrame[i].stVFrame.u32Length[0];
        unsigned char *ptr = calloc(1, image_size);
        pstVideoFrame[i].stVFrame.pu8VirAddr[0] = CVI_SYS_Mmap(pstVideoFrame[i].stVFrame.u64PhyAddr[0],
                                                               pstVideoFrame[i].stVFrame.u32Length[0]);

        memcpy(ptr, (const void *)pstVideoFrame[i].stVFrame.pu8VirAddr[0],
               pstVideoFrame[i].stVFrame.u32Length[0]);
        CVI_SYS_Munmap((void *)pstVideoFrame[i].stVFrame.pu8VirAddr[0],
                       pstVideoFrame[i].stVFrame.u32Length[0]);

        fwrite(ptr, image_size, 1, output);
        free(ptr);
    }

	fclose(output);

	snprintf(name, 128, "%s%s.txt", MEDIA_DUMP_GetSdConfig(), prefix);

	output = fopen(name, "a");
	if (output == NULL) {
		printf("open %s fail!!!\n", name);
		return -1;
	}

	fprintf(output, "LE crop = %d,%d,%d,%d\n", pstVideoFrame[0].stVFrame.s16OffsetLeft,
		pstVideoFrame[0].stVFrame.s16OffsetTop,
		pstVideoFrame[0].stVFrame.u32Width,
		pstVideoFrame[0].stVFrame.u32Height);

	if (frm_num > 1) {
		fprintf(output, "SE crop = %d,%d,%d,%d\n", pstVideoFrame[1].stVFrame.s16OffsetLeft,
			pstVideoFrame[1].stVFrame.s16OffsetTop,
			pstVideoFrame[1].stVFrame.u32Width,
			pstVideoFrame[1].stVFrame.u32Height);
	}

	fclose(output);

    return 0;
}

int32_t MEDIA_DUMP_DumpRaw(const int32_t sns_id)
{
    int32_t s32Ret = 0;
    MAPI_VCAP_SENSOR_HANDLE_T g_sns_hdl = MEDIA_GetCtx()->SysHandle.sns[sns_id];
    MAPI_VCAP_RAW_DATA_T stVCapRawData;
    stVCapRawData.pPrivateData = (void*)__FUNCTION__;
    stVCapRawData.pfn_VCAP_RawDataProc = (void *)MEDIA_RAWDATA_DumpCallback;

    s32Ret = MAPI_VCAP_StartDumpRaw(g_sns_hdl, 1, &stVCapRawData);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_VCAP_StartDumpRaw fail");

    s32Ret = MAPI_VCAP_StopDumpRaw(g_sns_hdl);
    MEDIA_CHECK_RET(s32Ret, APP_MEDIA_EINVAL, "MAPI_VCAP_StopDumpRaw fail");

    return 0;
}

extern int32_t CVI_ISP_SetAELogName(const char *szName);
extern int32_t CVI_ISP_SetAELogPath(const char *szPath);
extern int32_t AWB_SetDumpLogPath(const char *szPath);
extern int32_t AWB_SetDumpLogName(const char *szPath);
extern int32_t AE_DumpLog(void);
extern int32_t AWB_DumpLog(void);
extern void AWB_DumpDbgBin(char sID);

int32_t save_frame_file(uint32_t Grp, uint32_t Chn, VIDEO_FRAME_INFO_S *pFrame, void *pPrivateData)
{
    char filename[128];
    snprintf(filename, 128, "%s_grp%d_chn%d", (char*)pPrivateData, Grp, Chn);
    (MAPI_SaveFramePixelData(pFrame, filename));
    return 0;
}

int32_t MEDIA_DUMP_DumpYuv(int32_t sns_id)
{
    VIDEO_FRAME_INFO_S vcap_frame;
    char filename[128];
    char prefix[128] = {0};
    MAPI_VCAP_SENSOR_HANDLE_T g_sns_hdl = MEDIA_GetCtx()->SysHandle.sns[sns_id];
    MAPI_VPROC_HANDLE_T vproc_hdl = MEDIA_GetCtx()->SysHandle.vproc[sns_id];
	PARAM_WORK_MODE_S Workmode = {0};
    PARAM_GetWorkModeParam(&Workmode);
    PARAM_VPSS_ATTR_S Vpssmode = {0};
    int32_t  s32CurMode = MODEMNG_GetCurWorkMode();
    if (WORK_MODE_MOVIE == s32CurMode) {
        memcpy(&Vpssmode,&Workmode.RecordMode.Vpss, sizeof(PARAM_VPSS_ATTR_S));
    } else if (WORK_MODE_PHOTO == s32CurMode) {
        memcpy(&Vpssmode, &Workmode.PhotoMode.Vpss, sizeof(PARAM_VPSS_ATTR_S));
    } else {
        CVI_LOGE("This mode has no vpss parameters!\n");
    }
    SYSTEM_TM_S stDateTime;
    SYSTEM_GetRTCDateTime(&stDateTime);
	char *LogPath = MEDIA_DUMP_GetSdConfig();
	if (LogPath[strlen(LogPath) - 1] == '/') {
		LogPath[strlen(LogPath) - 1] = '\0';
	}
    CVI_ISP_SetAELogPath(LogPath);
    AWB_SetDumpLogPath(LogPath);
    if (Vpssmode.stVIVPSSMode.aenMode[sns_id] == VI_OFFLINE_VPSS_OFFLINE ||
        Vpssmode.stVIVPSSMode.aenMode[sns_id] == VI_OFFLINE_VPSS_OFFLINE) {
        (MAPI_VCAP_GetFrame(g_sns_hdl, &vcap_frame));
        snprintf(filename, 128, "%s%04d%02d%02d-%02d%02d%02d-media_dump_%d", MEDIA_DUMP_GetSdConfig(), stDateTime.s32year, stDateTime.s32mon, stDateTime.s32mday, stDateTime.s32hour, stDateTime.s32min, stDateTime.s32sec, sns_id);
        (MAPI_SaveFramePixelData(&vcap_frame, filename));
        (MAPI_ReleaseFrame(&vcap_frame));
        CVI_LOGE("sns[%d] dump yuv frame pass\n", sns_id);
    } else {
        CVI_LOGE("sns[%d] dump yuv, aemode is online\n", sns_id);
        MAPI_DUMP_FRAME_CALLBACK_FUNC_T stCallbackFun;
        get_file_name_prefix(sns_id, prefix, 128);
        snprintf(filename, 128, "%s%s", MEDIA_DUMP_GetSdConfig(), prefix);
		// snprintf(filename, 128, "%s%04d%02d%02d-%02d%02d%02d-media_dump_%d", MEDIA_DUMP_GetSdConfig(), stDateTime.s32year, stDateTime.s32mon, stDateTime.s32mday, stDateTime.s32hour, stDateTime.s32min, stDateTime.s32sec, sns_id);
		stCallbackFun.pPrivateData = (void*)filename;
		stCallbackFun.pfunFrameProc = save_frame_file;
		(MAPI_VPROC_StartChnDump(vproc_hdl, 0, 1, &stCallbackFun));
		(MAPI_VPROC_StopChnDump(vproc_hdl, 0));
    }


    return 0;
}

void MEDIA_DUMP_GetSizeStatus(bool *en)
{
	static bool get = false;
	static bool status = false;
	uint32_t pq = 0;
	char *buf = NULL;
    char val[32] = {0};
    char *shellcmd = "fw_printenv pq";

    FILE* fp = NULL;
	if (get == true) {
		CVI_LOGE("already acquired !\n");
		*en = status;
		return;
	}
    fp = popen(shellcmd, "r");
    if (fp == NULL) {
        CVI_LOGI(" get pq  failed\n");
		status = false;
    } else {
		CVI_LOGI(" get pq  success\n");
		fgets(val, sizeof(val), fp);
		buf = strtok(val, "=");
		buf = strtok(NULL, "=");
		CVI_LOGI("pq buf=%s\n", buf);

		if (buf != NULL) {
			pq = strtoul(buf, NULL, 16);
			printf("strtol pq = %d\n", pq);
		}
    	pclose(fp);
		if (pq == 1) {
			status = true;
		} else {
			status = false;
		}
	}

	get = true;
	*en = status;
}

void MEDIA_DUMP_VIISPReset(uint8_t bInit)
{
	if (bInit == 1) {

		MEDIA_VideoInit();

		MEDIA_DispInit(true);

		MEDIA_LiveViewSerInit();

		MEDIA_VencInit();

		MEDIA_StartOsd();
#ifdef ENABLE_ISP_PQ_TOOL
		bool en = false;
    	MEDIA_DUMP_GetSizeStatus(&en);
		if (en == false) {
			MEDIA_RecordSerInit();
		}
#else
    	MEDIA_RecordSerInit();
#endif

		// MEDIA_RtspSerInit();
	} else if(bInit == 0) {
		VI_VPSS_MODE_S stVIVPSSMode;
		CVI_SYS_GetVIVPSSMode(&stVIVPSSMode);
		MEDIA_LiveViewSerDeInit();
#ifdef ENABLE_ISP_PQ_TOOL
		bool en = false;
		MEDIA_DUMP_GetSizeStatus(&en);
		if (en == false) {
			MEDIA_RecordSerDeInit();
		}
#else
		MEDIA_RecordSerDeInit();
#endif

		// MEDIA_RtspSerDeInit();

		MEDIA_StopOsd();

		MEDIA_VideoDeInit();

		// MEDIA_DispDeInit();

		CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	} else if (bInit == 2){

		MEDIA_LiveViewSerDeInit();
#ifdef ENABLE_ISP_PQ_TOOL
		bool en = false;
		MEDIA_DUMP_GetSizeStatus(&en);
		if (en == false) {
			MEDIA_RecordSerDeInit();
		}
#else
		MEDIA_RecordSerDeInit();
#endif

		// MEDIA_RtspSerDeInit();

		MEDIA_StopOsd();

		MEDIA_VideoDeInit();

		// MEDIA_DispDeInit();

		MEDIA_VbDeInit();

		MEDIA_VbInit();

		MEDIA_VideoInit();

		MEDIA_DispInit(true);

		MEDIA_LiveViewSerInit();

		MEDIA_VencInit();

		MEDIA_StartOsd();
#ifdef ENABLE_ISP_PQ_TOOL
		en = false;
    	MEDIA_DUMP_GetSizeStatus(&en);
		if (en == false) {
			MEDIA_RecordSerInit();
		}
#else
    	MEDIA_RecordSerInit();
#endif

		// MEDIA_RtspSerDeInit();
	}
}

void MEDIA_DUMP_ReplayInit(void)
{
	// typedef void (*raw_replay_vi_isp_init_callback)(uint8_t bInit);
	// extern void raw_replay_register_callback_fun(raw_replay_vi_isp_init_callback pfun);
	// raw_replay_register_callback_fun(MEDIA_DUMP_VIISPReset);
}

#endif

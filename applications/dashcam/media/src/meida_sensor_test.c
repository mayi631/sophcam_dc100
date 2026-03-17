
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>
#include <inttypes.h>

#include <fcntl.h>		/* low-level i/o */
#include "cvi_buffer.h"
#include "cvi_ae.h"
#include "cvi_awb.h"
#include "cvi_isp.h"
#include "cvi_sensor.h"
#include <time.h>
#ifndef CHIP_184X
#include "cvi_msg_client.h"
#endif
#include "media_sensor_test.h"
#include "cvi_vi.h"
#include "cvi_sys.h"
#include "appcomm.h"

#define DELAY_500MS() (usleep(500 * 1000))
#define AAA_LIMIT(var, min, max) ((var) = ((var) < (min)) ? (min) : (((var) > (max)) ? (max) : (var)))
#define AAA_ABS(a) ((a) > 0 ? (a) : -(a))
#define AAA_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AAA_MAX(a, b) ((a) > (b) ? (a) : (b))
#define AAA_DIV_0_TO_1(a) ((0 == (a)) ? 1 : (a))

#define _LOG_NONE       "\033[0m"
#define _LOG_RED        "\033[1;31m"
#define _LOG_YELLOW     "\033[1;33m"
#define _LOG_GREEN      "\033[1;32m"
#define _LOG_DEBUG      "\033[0m"

#define debug_log(fmt, arg...) printf(_LOG_DEBUG fmt _LOG_NONE, ##arg)
#define info_log(fmt, arg...)  printf(_LOG_GREEN fmt _LOG_NONE, ##arg)
#define warn_log(fmt, arg...)  printf(_LOG_YELLOW fmt _LOG_NONE, ##arg)
#define error_log(fmt, arg...) printf(_LOG_RED fmt _LOG_NONE, ##arg)

#ifndef AE_SE
#define AE_SE ISP_CHANNEL_SE
#endif

#ifndef AE_LE
#define AE_LE ISP_CHANNEL_LE
#endif

#ifndef AE_WDR_RATIO_BASE
#define AE_WDR_RATIO_BASE 64
#endif

#ifndef AE_GAIN_BASE
#define AE_GAIN_BASE 1024
#endif

#ifndef MAX_SENSOR_NUM
#define MAX_SENSOR_NUM   2
#endif

static AE_SENSOR_DEFAULT_S stSnsDft[MAX_SENSOR_NUM];

//static ISP_SENSOR_EXP_FUNC_S stSensorExpFunc[MAX_SENSOR_NUM];
//static AE_SENSOR_EXP_FUNC_S stExpFuncs[MAX_SENSOR_NUM];
//static ISP_SNS_OBJ_S *pstSnsObj[MAX_SENSOR_NUM];

static void AE_SetFpsTest(uint8_t sID, uint8_t fps);

typedef struct __SENSOR_INFO_S {
	bool bWDRMode;

	float fExpLineTime;

	uint32_t u32LExpTimeMin;
	uint32_t u32LExpTimeMax;

	uint32_t u32LExpLineMin;
	uint32_t u32LExpLineMax;

	uint32_t u32SExpTimeMin;
	uint32_t u32SExpTimeMax;

	uint32_t u32SExpLineMin;
	uint32_t u32SExpLineMax;
} _SENSOR_INFO_S;

static _SENSOR_INFO_S sensorInfo[MAX_SENSOR_NUM];

struct _SNS_EXP_MAX_S exp_mix_min = {
	.manual = 1,
	.ratio = { 256, 64, 64 },//max 256x
	.IntTimeMax = {0, 0, 0, 0},
	.IntTimeMin = {0, 0, 0, 0},
	.LFMaxIntTime = {0, 0, 0, 0},
};

static void getSensorInfo(uint8_t sID)
{
	ISP_PUB_ATTR_S stPubAttr;
	uint32_t s32Ret = APP_SUCCESS;

	memset(&stPubAttr, 0, sizeof(stPubAttr));

	CVI_ISP_GetPubAttr(sID, &stPubAttr);

	if (stPubAttr.enWDRMode != WDR_MODE_NONE) {
		sensorInfo[sID].bWDRMode = true;
	} else {
		sensorInfo[sID].bWDRMode = false;
	}

	uint8_t fps = stPubAttr.f32FrameRate;

	// s32Ret = CVI_SENSOR_SetSnsType(sID, g_enSnsType[sID]);
	if (s32Ret != APP_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", sID);
	}
	#ifndef CHIP_184X
	s32Ret = CVI_SENSOR_SetSnsFps(sID, fps, &stSnsDft[sID]);
	#else
	s32Ret = CVI_SNS_SetSnsFps(sID, fps, &stSnsDft[sID]);
	#endif
	if (s32Ret != APP_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sns fps failed!\n");
	}

	sensorInfo[sID].fExpLineTime = 1000000 / (float) (stSnsDft[sID].u32FullLinesStd * fps);

	if (stSnsDft[sID].stIntTimeAccu.f32Accuracy < 1) {
		sensorInfo[sID].fExpLineTime = sensorInfo[sID].fExpLineTime *
								stSnsDft[sID].stIntTimeAccu.f32Accuracy;
	}

	info_log("\nsensor: %d, fps: %d\n", sID, fps);

	info_log("sensor frame line: %d, line time: %f, f32Accuracy: %f\n",
		stSnsDft[sID].u32FullLinesStd, sensorInfo[sID].fExpLineTime,
		stSnsDft[sID].stIntTimeAccu.f32Accuracy);

	if (!sensorInfo[sID].bWDRMode) {

		sensorInfo[sID].u32LExpLineMin = stSnsDft[sID].u32MinIntTime;
		sensorInfo[sID].u32LExpLineMax = stSnsDft[sID].u32MaxIntTime;

		sensorInfo[sID].u32LExpTimeMin =  sensorInfo[sID].u32LExpLineMin *
										sensorInfo[sID].fExpLineTime + 1;
		sensorInfo[sID].u32LExpTimeMax =  sensorInfo[sID].u32LExpLineMax *
										sensorInfo[sID].fExpLineTime;

		info_log("sensor exposure time range: %d - %d, line range: %d - %d\n",
			sensorInfo[sID].u32LExpTimeMin, sensorInfo[sID].u32LExpTimeMax,
			sensorInfo[sID].u32LExpLineMin, sensorInfo[sID].u32LExpLineMax);

	} else {
		// s32Ret = CVI_SENSOR_SetSnsType(sID, g_enSnsType[sID]);
		if (s32Ret != APP_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", sID);
		}
		#ifndef CHIP_184X
		s32Ret = CVI_SENSOR_GetExpRatio(sID, &exp_mix_min);
		#else
		s32Ret = CVI_SNS_GetExpRatio(sID, &exp_mix_min);
		#endif
		if (s32Ret != APP_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "get exp ratio failed!\n");
		}
		sensorInfo[sID].u32LExpLineMin = exp_mix_min.IntTimeMin[0];
		sensorInfo[sID].u32SExpLineMin = exp_mix_min.IntTimeMin[0];

		sensorInfo[sID].u32SExpLineMax = exp_mix_min.IntTimeMax[0];

		sensorInfo[sID].u32LExpLineMax = stSnsDft[sID].u32FullLinesStd - exp_mix_min.IntTimeMax[0];

		sensorInfo[sID].u32LExpTimeMin = sensorInfo[sID].u32LExpLineMin *
										sensorInfo[sID].fExpLineTime + 1;
		sensorInfo[sID].u32LExpTimeMax = sensorInfo[sID].u32LExpLineMax *
										sensorInfo[sID].fExpLineTime;

		sensorInfo[sID].u32SExpTimeMin = sensorInfo[sID].u32LExpTimeMin;
		sensorInfo[sID].u32SExpTimeMax = sensorInfo[sID].u32SExpLineMax * sensorInfo[sID].fExpLineTime;

		info_log("sensor LE exposure time range: %d - %d, line range: %d - %d\n",
			sensorInfo[sID].u32LExpTimeMin, sensorInfo[sID].u32LExpTimeMax,
			sensorInfo[sID].u32LExpLineMin, sensorInfo[sID].u32LExpLineMax);

		info_log("sensor SE exposure time range: %d - %d, line range: %d - %d\n",
			sensorInfo[sID].u32SExpTimeMin, sensorInfo[sID].u32SExpTimeMax,
			sensorInfo[sID].u32SExpLineMin, sensorInfo[sID].u32SExpLineMax);
	}

	info_log("sensor Again max: %d, Dgain max: %d\n\n",
		stSnsDft[sID].u32MaxAgain, stSnsDft[sID].u32MaxDgain);
}

static void apply_sensor_default_blc(uint8_t sID)
{
	ISP_CMOS_BLACK_LEVEL_S stBlc;
	uint32_t s32Ret = APP_SUCCESS;

	memset(&stBlc, 0, sizeof(ISP_CMOS_BLACK_LEVEL_S));
	// s32Ret = CVI_SENSOR_SetSnsType(sID, g_enSnsType[sID]);
	if (s32Ret != APP_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", sID);
	}
	#ifndef CHIP_184X
	s32Ret = CVI_SENSOR_GetIspBlkLev(sID, &stBlc);
	#else
	s32Ret = CVI_SNS_GetIspBlkLev(sID, &stBlc);
	#endif
	if (s32Ret != APP_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "get sensor isp black level failed!\n");
	}
	CVI_ISP_SetBlackLevelAttr(sID, &stBlc.blcAttr);

	debug_log("apply sensor default blc enOpType:%d ISO = 100 R:%d Gr:%d Gb:%d B:%d\n",
		stBlc.blcAttr.enOpType,
		stBlc.blcAttr.stAuto.OffsetR[0],
		stBlc.blcAttr.stAuto.OffsetGr[0],
		stBlc.blcAttr.stAuto.OffsetGb[0],
		stBlc.blcAttr.stAuto.OffsetB[0]);
}

static void init_sensor_info(void)
{
	uint32_t s32Ret = APP_SUCCESS;

	for (int32_t i = 0; i < 1; i++) {
		// s32Ret = CVI_SENSOR_SetSnsType(i, g_enSnsType[i]);
		if (s32Ret != APP_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", i);
		}
		apply_sensor_default_blc(i);
		#ifndef CHIP_184X
		s32Ret = CVI_SENSOR_GetAeDefault(i, &stSnsDft[i]);
		#else
		s32Ret = CVI_SNS_GetAeDefault(i, &stSnsDft[i]);
		#endif
		if (s32Ret != APP_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "get sensor AE default failed!\n");
		}
		getSensorInfo(i);
	}
}

static uint32_t calcExpLine(uint8_t sID, uint32_t expTime)
{
	uint32_t expLine = 0;

	expLine = expTime / sensorInfo[sID].fExpLineTime;
	AAA_LIMIT(expLine, sensorInfo[sID].u32LExpLineMin, sensorInfo[sID].u32LExpLineMax);

	return expLine;
}

static void calcCenterG(uint8_t sID, uint16_t *LE, uint16_t *SE)
{
	uint16_t row, column, i;
	uint16_t RValue, GValue, BValue, maxValue;
	uint8_t centerRowStart, centerRowEnd, centerColumnStart, centerColumnEnd;
	uint32_t centerLuma[ISP_CHANNEL_MAX_NUM] = {0, 0};
	uint16_t centerCnt[ISP_CHANNEL_MAX_NUM] = {0, 0};

	ISP_AE_STATISTICS_S stAeStat;

	memset(&stAeStat, 0, sizeof(ISP_AE_STATISTICS_S));

	CVI_ISP_GetAEStatistics(sID, &stAeStat);

	centerRowStart = AE_ZONE_ROW / 2 - AE_ZONE_ROW / 4;
	centerRowEnd = AE_ZONE_ROW / 2 + AE_ZONE_ROW / 4;
	centerColumnStart = AE_ZONE_COLUMN / 2 - AE_ZONE_COLUMN / 4;
	centerColumnEnd = AE_ZONE_COLUMN / 2 + AE_ZONE_COLUMN / 4;

	for (i = 0; i < ISP_CHANNEL_MAX_NUM; i++) {
		for (row = 0; row < AE_ZONE_ROW; row++) {
			for (column = 0; column < AE_ZONE_COLUMN; column++) {

				if ((row >= centerRowStart && row <= centerRowEnd) &&
					(column >= centerColumnStart && column <= centerColumnEnd)) {

					RValue = stAeStat.au16FEZoneAvg[i][0][row][column][ISP_BAYER_CHN_R];
					GValue = (stAeStat.au16FEZoneAvg[i][0][row][column][ISP_BAYER_CHN_GR] +
						stAeStat.au16FEZoneAvg[i][0][row][column][ISP_BAYER_CHN_GB]) / 2;
					BValue = stAeStat.au16FEZoneAvg[i][0][row][column][ISP_BAYER_CHN_B];

					maxValue = AAA_MAX(RValue, GValue);
					maxValue = AAA_MAX(maxValue, BValue);
					centerCnt[i]++;
					centerLuma[i] += maxValue;
				}

			}
		}
	}

	*LE = centerLuma[ISP_CHANNEL_LE] / centerCnt[ISP_CHANNEL_LE];
	*SE = centerLuma[ISP_CHANNEL_SE] / centerCnt[ISP_CHANNEL_SE];
}

static void _print_ae_info(uint8_t sID)
{
	ISP_EXP_INFO_S stExpInfo;

	memset(&stExpInfo, 0, sizeof(stExpInfo));
	CVI_ISP_QueryExposureInfo(sID, &stExpInfo);

	uint16_t le, se;

	calcCenterG(sID, &le, &se);

	if (!sensorInfo[sID].bWDRMode) {
		info_log("\ntime: %u, iso: %u, AeL: %u\n", stExpInfo.u32ExpTime,
			stExpInfo.u32ISO, le);
		info_log("sensor LEexpT: %u, LEexpL: %u AG: %u, DG: %u, IG: %u\n\n",
			stExpInfo.u32ExpTime, calcExpLine(sID, stExpInfo.u32ExpTime),
			stExpInfo.u32AGain, stExpInfo.u32DGain, stExpInfo.u32ISPDGain);
	} else {
		info_log("\ntime: %u, iso: %u, AeL: %u, AeS: %u\n", stExpInfo.u32ExpTime,
			stExpInfo.u32ISO, le, se);
		info_log("sensor LEexpT: %u, LEexpL: %u, SEexpT: %u, SEexpL: %u, AG: %u, DG: %u, IG: %u\n\n",
			stExpInfo.u32ExpTime, calcExpLine(sID, stExpInfo.u32ExpTime),
			stExpInfo.u32ShortExpTime, calcExpLine(sID, stExpInfo.u32ShortExpTime),
			stExpInfo.u32AGain, stExpInfo.u32DGain, stExpInfo.u32ISPDGain);
	}
}

static void AE_SetManualExposureTest(uint8_t sID, uint8_t mode, uint32_t expTime, uint32_t ISONum)
{
	ISP_EXPOSURE_ATTR_S stExpAttr;

	memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

	CVI_ISP_GetExposureAttr(sID, &stExpAttr);

	stExpAttr.u8DebugMode = 0;

	if (mode == 0) {
		stExpAttr.bByPass = 1;
		printf("AE byPass!\n");
	} else if (mode == 1) {
		stExpAttr.bByPass = 0;
		stExpAttr.enOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enExpTimeOpType = OP_TYPE_AUTO;
		stExpAttr.stManual.enISONumOpType = OP_TYPE_AUTO;
		printf("AE Auto!\n");
	} else if (mode == 2) {
		stExpAttr.bByPass = 0;
		stExpAttr.enOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.enISONumOpType = OP_TYPE_MANUAL;
		stExpAttr.stManual.u32ExpTime = expTime;
		stExpAttr.stManual.enGainType = AE_TYPE_ISO;
		stExpAttr.stManual.u32ISONum = ISONum;
		printf("AE Manual!\n");
	}

	CVI_ISP_SetExposureAttr(sID, &stExpAttr);

	DELAY_500MS();

	_print_ae_info(sID);
}

static void AE_SetDebugMode(uint8_t sID, uint8_t item)
{
	ISP_EXPOSURE_ATTR_S stExpAttr;

	memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

	CVI_ISP_GetExposureAttr(sID, &stExpAttr);

	stExpAttr.u8DebugMode = item;

	CVI_ISP_SetExposureAttr(sID, &stExpAttr);
}

static void AE_SetManualGainTest(uint8_t sID, uint32_t again, uint32_t dgain, uint32_t ispDgain)
{
	ISP_EXPOSURE_ATTR_S stExpAttr;

	memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

	CVI_ISP_GetExposureAttr(sID, &stExpAttr);

	stExpAttr.bByPass = 0;
	stExpAttr.u8DebugMode = 0;
	stExpAttr.enOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enGainType = AE_TYPE_GAIN;
	stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.u32AGain = again;
	stExpAttr.stManual.u32DGain = dgain;
	stExpAttr.stManual.u32ISPDGain = ispDgain;

	CVI_ISP_SetExposureAttr(sID, &stExpAttr);

	DELAY_500MS();

	_print_ae_info(sID);
}

static void AE_SetFpsTest(uint8_t sID, uint8_t fps)
{
	ISP_PUB_ATTR_S stPubAttr;

	memset(&stPubAttr, 0, sizeof(stPubAttr));

	CVI_ISP_GetPubAttr(sID, &stPubAttr);

	stPubAttr.f32FrameRate = fps;

	info_log("\nset pipe: %d, fps: %d\n", sID, fps);

	CVI_ISP_SetPubAttr(sID, &stPubAttr);

	DELAY_500MS();

	getSensorInfo(sID);
}

static void AE_SetLSC(uint8_t sID, uint8_t enableLSC)
{
	VI_PIPE ViPipe = sID;

#if defined(ARCH_CV182X) || defined(CHIP_184X)
	ISP_MESH_SHADING_ATTR_S plscAttr;

	CVI_ISP_GetMeshShadingAttr(ViPipe, &plscAttr);
	plscAttr.Enable = enableLSC;
	CVI_ISP_SetMeshShadingAttr(ViPipe, &plscAttr);
#else
	ISP_DRC_ATTR_S pDRCAttr;

#define AE_LSCR_ENABLE  3
#define AWB_LSCR_ENABLE 4
	CVI_ISP_GetDRCAttr(ViPipe, &pDRCAttr);
	pDRCAttr.DRCMu[AE_LSCR_ENABLE] = enableLSC;
	pDRCAttr.DRCMu[AWB_LSCR_ENABLE] = enableLSC;
	CVI_ISP_SetDRCAttr(ViPipe, &pDRCAttr);
#endif

	if (enableLSC) {
		info_log("sID:%d LSC enalbe!\n", sID);
	} else {
		info_log("sID:%d LSC disable!\n", sID);
	}
}

static void AE_SetWDRManualRatio(uint8_t sID, uint16_t ratio)
{
	ISP_WDR_EXPOSURE_ATTR_S stWDRExpAttr;

	memset(&stWDRExpAttr, 0, sizeof(ISP_WDR_EXPOSURE_ATTR_S));

	CVI_ISP_GetWDRExposureAttr(sID, &stWDRExpAttr);

	stWDRExpAttr.enExpRatioType = OP_TYPE_MANUAL;
	stWDRExpAttr.au32ExpRatio[0] = ratio < 4 ? (4 * AE_WDR_RATIO_BASE) : (ratio * AE_WDR_RATIO_BASE);

	CVI_ISP_SetWDRExposureAttr(sID, &stWDRExpAttr);

	info_log("set WDR manual ratio: %d\n", stWDRExpAttr.au32ExpRatio[0]);

	if (ratio == 0) {

		printf("set max SE shutter time: %d\n", sensorInfo[sID].u32SExpTimeMax);

		ISP_EXP_INFO_S stExpInfo;

		memset(&stExpInfo, 0, sizeof(stExpInfo));
		CVI_ISP_QueryExposureInfo(sID, &stExpInfo);

		AE_SetManualExposureTest(sID, 2, 100000, stExpInfo.u32ISO);
	}
}

static void AE_GainLinearTest(uint8_t sID, uint32_t expTime, uint32_t StartISONum, uint32_t EndISONum)
{
#define RATIO_ERROR_DIFF	3

	uint16_t leLuma, seLuma;
	SNS_GAIN_S stDgain;
	SNS_GAIN_S stAgain;
	uint32_t s32Ret = APP_SUCCESS;

	uint32_t tempGain = 0;

	uint32_t iso, gain, preAgain = 0, preDgain = 0;
	uint32_t	isoTable[] = {100, 200, 400, 800, 1600, 3200, 6400,
		12800, 25600, 51200, 102400, 204800, 409600, 819200};
	uint16_t isoStep = 1, curLuma, preLuma = 0, isoTblSize;
	uint16_t i, lumaRatio, gainRatio;

	ISP_EXPOSURE_ATTR_S expAttr = { 0 };
	// VI_PIPE ViPipe = sID;

	CVI_ISP_GetExposureAttr(sID, &expAttr);

	isoTblSize = sizeof(isoTable) / sizeof(uint32_t);
	expAttr.bByPass = 0;
	expAttr.u8DebugMode = 0;
	expAttr.enOpType = OP_TYPE_MANUAL;
	expAttr.stManual.enGainType = AE_TYPE_GAIN;
	expAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
	expAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
	expAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
	expAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
	expAttr.stManual.u32ExpTime = expTime;

	if (EndISONum < StartISONum)
		EndISONum = StartISONum;

	StartISONum = AAA_MAX(StartISONum, 100);

	// s32Ret = CVI_SENSOR_SetSnsType(ViPipe, g_enSnsType[sID]);
	if (s32Ret != APP_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", sID);
	}
	for (iso = StartISONum; iso <= EndISONum; iso += isoStep) {
		for (i = 1 ; i < isoTblSize; ++i) {
			if (iso < isoTable[i]) {
				isoStep = (isoTable[i] - isoTable[i-1]) / 100;
				break;
			}
		}

		gain = (uint32_t) ((uint64_t) iso * (uint64_t) AE_GAIN_BASE) / 100;

		if (gain > stSnsDft[sID].u32MaxAgain && preAgain == stSnsDft[sID].u32MaxAgain) {
			stAgain.gain = stSnsDft[sID].u32MaxAgain;
			stDgain.gain = (uint64_t)gain * AE_GAIN_BASE / AAA_DIV_0_TO_1(stAgain.gain);
			tempGain = stDgain.gain;
			#ifndef CHIP_184X
			s32Ret = CVI_SENSOR_SetDgainCalc(sID, &stDgain);
			#else
			s32Ret = CVI_SNS_SetDgainCalc(sID, &stDgain);
			#endif
			if (s32Ret != APP_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor gain calc failed!\n");
			}
			if (stDgain.gain > tempGain) {
				error_log("\n\nWARN: The output Dgain(%d) can not bigger than", stDgain.gain);
				error_log(" the input Dgain(%d)!!!\n\n", tempGain);
			}
		} else {
			stAgain.gain = gain;
			stDgain.gain = AE_GAIN_BASE;
			stAgain.gain = AAA_MIN(stAgain.gain, stSnsDft[sID].u32MaxAgain);
			tempGain = stAgain.gain;
			#ifndef CHIP_184X
			s32Ret = CVI_SENSOR_SetAgainCalc(sID, &stAgain);
			#else
			s32Ret = CVI_SNS_SetAgainCalc(sID, &stAgain);
			#endif
			if (s32Ret != APP_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor gain calc failed!\n");
			}
			if (stAgain.gain > tempGain) {
				error_log("\n\nWARN: The output Again(%d) can not bigger than", stAgain.gain);
				error_log(" the input Again(%d)!!!\n\n", tempGain);
			}
		}

		if (stAgain.gain != preAgain || stDgain.gain != preDgain) {
			expAttr.stManual.u32AGain = stAgain.gain;
			expAttr.stManual.u32DGain = stDgain.gain;
			expAttr.stManual.u32ISPDGain = AE_GAIN_BASE;
			CVI_ISP_SetExposureAttr(sID, &expAttr);
			DELAY_500MS();
			calcCenterG(sID, &leLuma, &seLuma);
			curLuma = leLuma;
			lumaRatio = curLuma * 100 / AAA_DIV_0_TO_1(preLuma);
			gainRatio = (uint64_t)stAgain.gain * stDgain.gain * 100 / AAA_DIV_0_TO_1((uint64_t)preAgain * preDgain);
			if (AAA_ABS(lumaRatio - gainRatio) > RATIO_ERROR_DIFF)
				error_log("AG(%d):%u DG(%d):%u L:%u LR:%d GR:%d\n", stAgain.gainDb, stAgain.gain,
					stDgain.gainDb, stDgain.gain, curLuma, lumaRatio, gainRatio);
			else
				info_log("AG(%d):%u DG(%d):%u L:%u LR:%d GR:%d\n", stAgain.gainDb, stAgain.gain,
					stDgain.gainDb, stDgain.gain, curLuma, lumaRatio, gainRatio);
			preAgain = stAgain.gain;
			preDgain = stDgain.gain;
			preLuma = curLuma;
			if (stSnsDft[sID].u32MaxDgain > 1024 &&
				stDgain.gain >= stSnsDft[sID].u32MaxDgain) {
				break;
			} else if (stSnsDft[sID].u32MaxDgain == 1024 &&
				stAgain.gain >= stSnsDft[sID].u32MaxAgain) {
				break;
			}
		}
	}
}

static void AE_ShutterLinearTest(uint8_t sID, uint8_t fid, uint32_t startExpTime, uint32_t endExpTime)
{
	ISP_EXP_INFO_S stExpInfo;
	ISP_EXPOSURE_ATTR_S stExpAttr;
	ISP_WDR_EXPOSURE_ATTR_S stWDRExpAttr;

	memset(&stExpInfo, 0, sizeof(stExpInfo));
	memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));
	memset(&stWDRExpAttr, 0, sizeof(ISP_WDR_EXPOSURE_ATTR_S));

	uint32_t tmpExpTime[ISP_CHANNEL_MAX_NUM] = {0, 0};
	uint16_t curLuma[ISP_CHANNEL_MAX_NUM] = {0, 0};
	uint16_t preLuma[ISP_CHANNEL_MAX_NUM] = {0, 0};
	uint32_t curExpLine[ISP_CHANNEL_MAX_NUM] = {0, 0};
	uint32_t preExpLine[ISP_CHANNEL_MAX_NUM] = {0, 0};

	int16_t lumaRatio[ISP_CHANNEL_MAX_NUM] = {0, 0};
	int16_t expLineRatio[ISP_CHANNEL_MAX_NUM] = {0, 0};
	int16_t ratioDiff[ISP_CHANNEL_MAX_NUM] = {0, 0};

	bool loop = true;

	uint16_t leLuma, seLuma;

#define RATIO_ERROR_DIFF  3
#define WDR_TEST_RATIO    4

	startExpTime = AAA_MAX(sensorInfo[sID].u32LExpTimeMin, startExpTime);
	endExpTime = AAA_MIN(sensorInfo[sID].u32LExpTimeMax, endExpTime);

	info_log("LE T: %d - %d, L: %d - %d\n",
		sensorInfo[sID].u32LExpTimeMin,
		sensorInfo[sID].u32LExpTimeMax,
		sensorInfo[sID].u32LExpLineMin,
		sensorInfo[sID].u32LExpLineMax);

	if (sensorInfo[sID].bWDRMode) {

		info_log("SE T: %d - %d, L: %d - %d\n",
			sensorInfo[sID].u32SExpTimeMin,
			sensorInfo[sID].u32SExpTimeMax,
			sensorInfo[sID].u32SExpLineMin,
			sensorInfo[sID].u32SExpLineMax);
	}

	fid = (fid == 1 ? AE_SE : AE_LE);

	tmpExpTime[AE_LE] = tmpExpTime[AE_SE] = startExpTime;

	CVI_ISP_GetWDRExposureAttr(sID, &stWDRExpAttr);

	if (fid == AE_SE) {

		if (!sensorInfo[sID].bWDRMode) {
			error_log("not WDR mode...\n");
			return;
		}

		stWDRExpAttr.enExpRatioType = OP_TYPE_MANUAL;
		stWDRExpAttr.au32ExpRatio[0] = WDR_TEST_RATIO * AE_WDR_RATIO_BASE;

		tmpExpTime[AE_LE] = tmpExpTime[AE_SE] * WDR_TEST_RATIO;
		endExpTime = AAA_MIN(sensorInfo[sID].u32SExpTimeMax, endExpTime);

	} else {
		stWDRExpAttr.enExpRatioType = OP_TYPE_AUTO;
	}

	if (endExpTime < startExpTime) {
		error_log("test fail, endExpTime: %d < startExpTime: %d\n", endExpTime, startExpTime);
		return;
	}

	CVI_ISP_SetWDRExposureAttr(sID, &stWDRExpAttr);

	CVI_ISP_GetExposureAttr(sID, &stExpAttr);

	stExpAttr.bByPass = 0;
	stExpAttr.u8DebugMode = 0;
	stExpAttr.enOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enGainType = AE_TYPE_GAIN;
	stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.u32ExpTime = tmpExpTime[AE_LE];

	CVI_ISP_SetExposureAttr(sID, &stExpAttr);

	DELAY_500MS();

	CVI_ISP_QueryExposureInfo(sID, &stExpInfo);
	calcCenterG(sID, &leLuma, &seLuma);

	curExpLine[AE_LE] = calcExpLine(sID, stExpInfo.u32ExpTime);
	preExpLine[AE_LE] = curExpLine[AE_LE];
	curLuma[AE_LE] = leLuma;
	preLuma[AE_LE] = curLuma[AE_LE];

	if (sensorInfo[sID].bWDRMode) {
		curExpLine[AE_SE] = calcExpLine(sID, stExpInfo.u32ShortExpTime);
		preExpLine[AE_SE] = curExpLine[AE_SE];
		curLuma[AE_SE] = seLuma;
		preLuma[AE_SE] = curLuma[AE_SE];
	}

	tmpExpTime[fid] = startExpTime;

	while (loop) {

		if (tmpExpTime[fid] > endExpTime) {
			tmpExpTime[fid] = endExpTime;
			loop = false;
		}

		if (fid == AE_SE) {
			tmpExpTime[AE_LE] = tmpExpTime[AE_SE] * WDR_TEST_RATIO;
		}

		stExpAttr.stManual.u32ExpTime = tmpExpTime[AE_LE];
		CVI_ISP_SetExposureAttr(sID, &stExpAttr);

		DELAY_500MS();

		CVI_ISP_QueryExposureInfo(sID, &stExpInfo);
		calcCenterG(sID, &leLuma, &seLuma);

		//curExpLine[AE_LE] = calcExpLine(sID, stExpInfo.u32ExpTime);
		curExpLine[AE_LE] = calcExpLine(sID, tmpExpTime[AE_LE]);
		curLuma[AE_LE] = leLuma;

		lumaRatio[AE_LE] = curLuma[AE_LE] * 100 / AAA_DIV_0_TO_1(preLuma[AE_LE]);
		expLineRatio[AE_LE] = curExpLine[AE_LE] * 100 / AAA_DIV_0_TO_1(preExpLine[AE_LE]);

		ratioDiff[AE_LE] = abs(lumaRatio[AE_LE] - expLineRatio[AE_LE]);

		if (sensorInfo[sID].bWDRMode) {
			//curExpLine[AE_SE] = calcExpLine(sID, stExpInfo.u32ShortExpTime);
			curExpLine[AE_SE] = calcExpLine(sID, tmpExpTime[AE_SE]);
			curLuma[AE_SE] = seLuma;

			lumaRatio[AE_SE] = curLuma[AE_SE] * 100 / AAA_DIV_0_TO_1(preLuma[AE_SE]);
			expLineRatio[AE_SE] = curExpLine[AE_SE] * 100 / AAA_DIV_0_TO_1(preExpLine[AE_SE]);

			ratioDiff[AE_SE] = abs(lumaRatio[AE_SE] - expLineRatio[AE_SE]);
		}

		if (ratioDiff[AE_LE] >= RATIO_ERROR_DIFF
			|| ratioDiff[AE_SE] >= RATIO_ERROR_DIFF) {
			error_log("\nWARN: Not linear item:\n");
		}

		preLuma[AE_LE] = curLuma[AE_LE];
		preExpLine[AE_LE] = curExpLine[AE_LE];

		info_log("LE, L: %d, T: %d, E: %d, LR: %d, ER: %d\n",
			curLuma[AE_LE],
			stExpInfo.u32ExpTime,
			curExpLine[AE_LE],
			lumaRatio[AE_LE],
			expLineRatio[AE_LE]);

		if (sensorInfo[sID].bWDRMode) {
			preLuma[AE_SE] = curLuma[AE_SE];
			preExpLine[AE_SE] = curExpLine[AE_SE];

			info_log("SE, L: %d, T: %d, E: %d, LR: %d, ER: %d\n",
				curLuma[AE_SE],
				stExpInfo.u32ShortExpTime,
				curExpLine[AE_SE],
				lumaRatio[AE_SE],
				expLineRatio[AE_SE]);
		}

		do {
			tmpExpTime[fid] = tmpExpTime[fid] * 105 / 100; // 5%

			curExpLine[fid] = calcExpLine(sID, tmpExpTime[fid]);

		} while (curExpLine[fid] == preExpLine[fid]);
	}
}

static uint32_t gainLookup(uint8_t sID, uint8_t type, uint32_t index)
{
	SNS_GAIN_S stMaxGain = {
		.gain = 1024,
		.gainDb = 0,
	};
	SNS_GAIN_S stTempGain = {
		.gain = 0,
		.gainDb = 0,
	};
	uint32_t minGain = 1024;
	uint32_t s32Ret = APP_SUCCESS;

	if (index == 0) {
		return 1024;
	}
	// s32Ret = CVI_SENSOR_SetSnsType(sID, g_enSnsType[sID]);
	if (s32Ret != APP_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor type(%d) failed!\n", sID);
	}
	if (type == 0) {
		stMaxGain.gain = stSnsDft[sID].u32MaxAgain;
		#ifndef CHIP_184X
		s32Ret = CVI_SENSOR_SetAgainCalc(sID, &stMaxGain);
		#else
		s32Ret = CVI_SNS_SetAgainCalc(sID, &stMaxGain);
		#endif
		if (s32Ret != APP_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor max again calc failed!\n");
		}
	} else {
		stMaxGain.gain = stSnsDft[sID].u32MaxDgain;
		#ifndef CHIP_184X
		s32Ret = CVI_SENSOR_SetDgainCalc(sID, &stMaxGain);
		#else
		s32Ret = CVI_SNS_SetDgainCalc(sID, &stMaxGain);
		#endif
		if (s32Ret != APP_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "get sensor max dgain calc failed!\n");
		}
	}

	if (index >= stMaxGain.gainDb) {
		return stMaxGain.gain;
	}
	stTempGain.gainDb =stMaxGain.gainDb;

	while (1) {

		stTempGain.gain = (stMaxGain.gain + minGain) / 2;

		if (type == 0) {

			#ifndef CHIP_184X
		s32Ret = CVI_SENSOR_SetAgainCalc(sID, &stMaxGain);
		#else
		s32Ret = CVI_SNS_SetAgainCalc(sID, &stMaxGain);
		#endif
			if (s32Ret != APP_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "set sensor temp again calc failed!\n");
			}
		} else {
			#ifndef CHIP_184X
			s32Ret = CVI_SENSOR_SetDgainCalc(sID, &stMaxGain);
			#else
			s32Ret = CVI_SNS_SetDgainCalc(sID, &stMaxGain);
			#endif
			if (s32Ret != APP_SUCCESS) {
				CVI_TRACE_LOG(CVI_DBG_ERR, "get sensor temp dgain calc failed!\n");
			}
		}

		if (stTempGain.gainDb == index) {
			return stTempGain.gain;
		} else if (stTempGain.gainDb > index) {
			stMaxGain.gain = stTempGain.gain;
		} else {
			minGain = stTempGain.gain;
		}
	}
}

static void AE_GainTableLinearTest(uint8_t sID, uint8_t type, uint32_t startIndex, uint32_t endIndex)
{
#define RATIO_ERROR_DIFF	3

	uint16_t leLuma, seLuma;

	uint32_t preGain = 0, gain = AE_GAIN_BASE;
	uint16_t curLuma = 0, preLuma = 0;
	int32_t lumaRatio, gainRatio;

	ISP_EXP_INFO_S stExpInfo;
	ISP_EXPOSURE_ATTR_S stExpAttr;

	memset(&stExpInfo, 0, sizeof(ISP_EXP_INFO_S));
	memset(&stExpAttr, 0, sizeof(ISP_EXPOSURE_ATTR_S));

	CVI_ISP_GetExposureAttr(sID, &stExpAttr);

	stExpAttr.bByPass = 0;
	stExpAttr.u8DebugMode = 0;
	stExpAttr.enOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enGainType = AE_TYPE_GAIN;
	stExpAttr.stManual.enExpTimeOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enAGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enDGainOpType = OP_TYPE_MANUAL;
	stExpAttr.stManual.enISPDGainOpType = OP_TYPE_MANUAL;

	stExpAttr.stManual.u32ISPDGain = AE_GAIN_BASE;

	gain = gainLookup(sID, type, startIndex);

	if (type == 0) {
		stExpAttr.stManual.u32DGain = AE_GAIN_BASE;
		stExpAttr.stManual.u32AGain = gain;
	} else {
		stExpAttr.stManual.u32AGain = stSnsDft[sID].u32MaxAgain;
		stExpAttr.stManual.u32DGain = gain;
	}

	CVI_ISP_SetExposureAttr(sID, &stExpAttr);

	DELAY_500MS();

	CVI_ISP_QueryExposureInfo(sID, &stExpInfo);
	calcCenterG(sID, &leLuma, &seLuma);

	curLuma = leLuma;

	if (type == 0) {
		gain = stExpInfo.u32AGain;
	} else {
		gain = stExpInfo.u32DGain;
	}

	printf("start index: %d, gain: %d, luma: %d\n", startIndex, gain, curLuma);

	for (uint32_t i = startIndex + 1; i <= endIndex; i++) {

		preGain = gain;
		preLuma = curLuma;

		gain = gainLookup(sID, type, i);

		if (type == 0) {
			stExpAttr.stManual.u32DGain = AE_GAIN_BASE;
			stExpAttr.stManual.u32AGain = gain;
		} else {
			stExpAttr.stManual.u32AGain = stSnsDft[sID].u32MaxAgain;
			stExpAttr.stManual.u32DGain = gain;
		}

		CVI_ISP_SetExposureAttr(sID, &stExpAttr);

		DELAY_500MS();

		CVI_ISP_QueryExposureInfo(sID, &stExpInfo);
		calcCenterG(sID, &leLuma, &seLuma);

		curLuma = leLuma;

		if (type == 0) {
			gain = stExpInfo.u32AGain;
		} else {
			gain = stExpInfo.u32DGain;
		}

		lumaRatio = curLuma * 100 / AAA_DIV_0_TO_1(preLuma);
		gainRatio = gain * 100 / AAA_DIV_0_TO_1(preGain);

		if (abs(lumaRatio - gainRatio) >= RATIO_ERROR_DIFF ||
			curLuma <= preLuma) {
			error_log("Index: %u, AG:%u DG:%u L:%u LR:%d GR:%d\n", i, stExpInfo.u32AGain,
				stExpInfo.u32DGain, curLuma, lumaRatio, gainRatio);
		} else {
			info_log("Index: %u, AG:%u DG:%u L:%u LR:%d GR:%d\n", i, stExpInfo.u32AGain,
				stExpInfo.u32DGain, curLuma, lumaRatio, gainRatio);
		}
	}
}

static void AE_SetBlc(uint8_t sID, uint8_t type)
{
	int32_t r = 0, gr = 0, gb = 0, b = 0;
	ISP_BLACK_LEVEL_ATTR_S stBlackLevelAttr;

	memset(&stBlackLevelAttr, 0, sizeof(ISP_BLACK_LEVEL_ATTR_S));
	CVI_ISP_GetBlackLevelAttr(sID, &stBlackLevelAttr);

	switch (type) {
	case 0:
		stBlackLevelAttr.Enable = APP_FALSE;
		break;
	case 1:
		stBlackLevelAttr.Enable = APP_TRUE;
		stBlackLevelAttr.enOpType = OP_TYPE_AUTO;
		break;
	case 2:
		stBlackLevelAttr.Enable = APP_TRUE;
		stBlackLevelAttr.enOpType = OP_TYPE_MANUAL;
		printf("Please input offsetR,offsetGr,offsetGb,offsetB\n");
		scanf("%d %d %d %d", &r, &gr, &gb, &b);
		stBlackLevelAttr.stManual.OffsetR = (uint16_t) r;
		stBlackLevelAttr.stManual.OffsetGr = (uint16_t) gr;
		stBlackLevelAttr.stManual.OffsetGb = (uint16_t) gb;
		stBlackLevelAttr.stManual.OffsetB = (uint16_t) b;
		break;
	default:
		break;
	}

	printf("blc info, enable: %d, enOpType: %d, manual offsetR,Gr,Gb,B: %d, %d, %d, %d\n",
		stBlackLevelAttr.Enable,
		stBlackLevelAttr.enOpType,
		stBlackLevelAttr.stManual.OffsetR,
		stBlackLevelAttr.stManual.OffsetGr,
		stBlackLevelAttr.stManual.OffsetGb,
		stBlackLevelAttr.stManual.OffsetB);

	CVI_ISP_SetBlackLevelAttr(sID, &stBlackLevelAttr);
}

static void sensor_ae_test_init(void)
{
	memset(&stSnsDft, 0, sizeof(AE_SENSOR_DEFAULT_S) * MAX_SENSOR_NUM);
	memset(&sensorInfo, 0, sizeof(_SENSOR_INFO_S) * MAX_SENSOR_NUM);

	init_sensor_info();
}

int32_t sensor_ae_test(char *param)
{
	printf("ae test: param = %s\n", param);
	int32_t i = 0;
	char *p[5];
	char *temp;
	temp = strtok(param, "-");
    while(temp) {
        p[i] = temp;
        printf("p[i] == %s, i = %d\n", p[i], i);
        i++;
        temp = strtok(NULL, "-");
    }
	uint32_t sID = atoi(p[1]), item = atoi(p[0]), para1 = atoi(p[2]), para2 = atoi(p[3]), para3 = atoi(p[4]);
	uint32_t s32Ret = APP_SUCCESS;

	sensor_ae_test_init();

	printf("\n1:AE_SetManualExposureTest(sID, 0:bypss 1:auto 2:manu, time, iso)\n");
	printf("2:AE_SetDebugMode(sID, item)\n");
	printf("3:AE_SetManualGainTest(sID, AG, DG, IG)\n");
	printf("4:AE_SetFpsTest(sID, fps)\n");
	printf("5:AE_SetLSC(sID, enable)\n");
	printf("6:AE_SetWDRManualRatio(sid, ratio), ratio: 4 - 256, 0: set SE max shutter time.\n");
	printf("7:AE_GainLinearTest(sID, time, startISO, endISO)\n");
	printf("8:AE_ShutterLinearTest(sID, fid 0: LE 1: SE, startExpTime, endExpTime)\n");
	printf("9:AE_GainTableLinearTest(sID, type: again 0 dgain 1, startIndex, endIndex)\n");
	printf("10:AE_SetBlc(sID, type: 0:disable 1:auto, 2:manu)\n");
	printf("Item/sID/para1/para2/para3\n\n");

	// scanf("%d %d %d %d %d", &item, &sID, &para1, &para2, &para3);

	if (sID >= MAX_SENSOR_NUM) {
		error_log("sID out of range...\n");
		s32Ret = APP_FAILURE;
		return s32Ret;
	}

	switch (item) {
	case 1:
		AE_SetManualExposureTest(sID, para1, para2, para3);
		break;
	case 2:
		AE_SetDebugMode(sID, para1);
		break;
	case 3:
		AE_SetManualGainTest(sID, para1, para2, para3);
		break;
	case 4:
		AE_SetFpsTest(sID, para1);
		break;
	case 5:
		AE_SetLSC(sID, para1);
		break;
	case 6:
		AE_SetWDRManualRatio(sID, para1);
		break;
	case 7:
		AE_GainLinearTest(sID, para1, para2, para3);
		break;
	case 8:
		AE_ShutterLinearTest(sID, para1, para2, para3);
		break;
	case 9:
		AE_GainTableLinearTest(sID, para1, para2, para3);
		break;
	case 10:
		AE_SetBlc(sID, para1);
		break;
	default:
		break;
	}

	return s32Ret;
}

static int32_t sys_vi_init(void)
{
	return APP_SUCCESS;
}

int32_t _vi_get_chn_frame(uint8_t chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		FILE *output;
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		void *vir_addr;
		uint32_t plane_offset, u32LumaSize, u32ChromaSize;
		char img_name[128] = {0, };

		CVI_TRACE_LOG(CVI_DBG_WARN, "width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "sample_%d.yuv", chn);

		output = fopen(img_name, "wb");
		if (output == NULL) {
			memset(img_name, 0x0, sizeof(img_name));
			snprintf(img_name, sizeof(img_name), "/mnt/data/sample_%d.yuv", chn);
			output = fopen(img_name, "wb");
			if (output == NULL) {
				CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
				CVI_TRACE_LOG(CVI_DBG_ERR, "fopen fail\n");
				return APP_FAILURE;
			}
		}

		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = CVI_SYS_Mmap(stVideoFrame.stVFrame.u64PhyAddr[0], image_size);
		CVI_SYS_IonInvalidateCache(stVideoFrame.stVFrame.u64PhyAddr[0], vir_addr, image_size);
		plane_offset = 0;
		for (int32_t i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				CVI_TRACE_LOG(CVI_DBG_WARN,
					   "plane(%d): paddr(%#"PRIx64") vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				fwrite((void *)stVideoFrame.stVFrame.pu8VirAddr[i]
					, (i == 0) ? u32LumaSize : u32ChromaSize, 1, output);
			}
		}
		CVI_SYS_Munmap(vir_addr, image_size);

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\n");
		}

		fclose(output);
		return APP_SUCCESS;
	}
	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\n");
	return APP_FAILURE;
}

long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

static int32_t sensor_dump_yuv(void)
{
	int32_t loop = 0;
	uint32_t ok = 0, ng = 0;
	uint8_t  chn = 0;
	int32_t tmp;
	struct timespec start, end;

	CVI_TRACE_LOG(CVI_DBG_WARN, "Get frm from which chn(0~1): ");
	scanf("%d", &tmp);
	chn = tmp;
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111 is infinite: ");
	scanf("%d", &loop);
	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (_vi_get_chn_frame(chn) == APP_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
						(float)diff_in_us(start, end)/1000);
		} else
			++ng;
		//sleep(1);
		if (loop != 11111)
			loop--;
	}
	CVI_TRACE_LOG(CVI_DBG_WARN, "VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI yuv TEST-PASS\n");

	return APP_SUCCESS;
}

static int32_t sensor_flip_mirror(void)
{
	int32_t flip;
	int32_t mirror;
	int32_t chnID;
	int32_t pipeID;

	CVI_TRACE_LOG(CVI_DBG_WARN, "chn(0~1): ");
	scanf("%d", &chnID);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Flip enable/disable(1/0): ");
	scanf("%d", &flip);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Mirror enable/disable(1/0): ");
	scanf("%d", &mirror);
	pipeID = chnID;
	CVI_VI_SetChnFlipMirror(pipeID, chnID, flip, mirror);

	return APP_SUCCESS;
}

static int32_t sensor_dump_raw(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	struct timeval tv1;
	int32_t frm_num = 1, j = 0;
	uint32_t dev = 0, loop = 0;
	struct timespec start, end;
	int32_t s32Ret = APP_SUCCESS;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	CVI_TRACE_LOG(CVI_DBG_WARN, "To get raw dump from dev(0~1): ");
	scanf("%d", &dev);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Enable(%d), DumpType(%d):\n", attr.bEnable, attr.enDumpType);
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do (1~60)");
	scanf("%d", &loop);

	if (loop > 60)
		return s32Ret;

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		frm_num = 1;

		CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			FILE *output;
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0]
					= CVI_SYS_Mmap(stVideoFrame[j].stVFrame.u64PhyAddr[0]
					  , stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_TRACE_LOG(CVI_DBG_WARN, "paddr(%#"PRIx64") vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);
				CVI_SYS_Munmap((void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
						stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
				default:
				case BAYER_FORMAT_BG:
					snprintf(order_id, sizeof(order_id), "BG");
					break;
				case BAYER_FORMAT_GB:
					snprintf(order_id, sizeof(order_id), "GB");
					break;
				case BAYER_FORMAT_GR:
					snprintf(order_id, sizeof(order_id), "GR");
					break;
				case BAYER_FORMAT_RG:
					snprintf(order_id, sizeof(order_id), "RG");
					break;
				}

				snprintf(img_name, sizeof(img_name),
						"./vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						(long int)tv1.tv_sec, (long int)tv1.tv_usec);

				CVI_TRACE_LOG(CVI_DBG_WARN, "dump image %s\n", img_name);

				output = fopen(img_name, "wb");

				fwrite(ptr, image_size, 1, output);
				fclose(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\n",
					(float)diff_in_us(start, end) / 1000);

		loop--;
	}

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI raw TEST-PASS\n");

	return s32Ret;
}

static int32_t sensor_linear_wdr_switch(void)
{
	int32_t s32Ret = APP_SUCCESS;
	return s32Ret;
}

#ifdef ENABLE_LOAD_ISPD_SO

#include <dlfcn.h>

void load_ispd(void)
{
#define ISPD_LIBNAME "libcvi_ispd2.so"
#define ISPD_CONNECT_PORT 5566

	char *error = NULL;
	void *handle = NULL;

	dlerror();
	handle = dlopen(ISPD_LIBNAME, RTLD_NOW);

	if (handle) {
		void (*daemon_init)(uint32_t port);

		printf("Load dynamic library %s success\n", ISPD_LIBNAME);

		dlerror();
		daemon_init = dlsym(handle, "isp_daemon2_init");
		error = dlerror();

		if (error == NULL) {
			(*daemon_init)(ISPD_CONNECT_PORT);
		} else {
			printf("Run daemon initial fail, %s\n", error);
			dlclose(handle);
		}
	} else {
		error = dlerror();
		printf("dlopen: %s, error: %s\n", ISPD_LIBNAME, error);
	}
}
#endif

int32_t sensor_test(char *param)
{
	int32_t s32Ret = APP_SUCCESS;
	int32_t op = 5;

	s32Ret = sys_vi_init();
	if (s32Ret != APP_SUCCESS)
		return s32Ret;

#ifdef ENABLE_LOAD_ISPD_SO
	load_ispd();
#endif

	usleep(1000 * 1000);

	system("stty erase ^H");

	printf("---Basic------------------------------------------------\n");
	// printf("1: dump vi raw frame\n");
	// printf("2: dump vi yuv frame\n");
	// printf("3: set chn flip/mirror\n");
	// printf("4: linear wdr switch\n");
	printf("5: AE debug\n");
	// printf("255: exit\n");
	// fflush(stdin);
	// scanf("%d", &op);
	printf("op: %d\n", op);

	switch (op) {
	case 1:
		s32Ret = sensor_dump_raw();
		break;
	case 2:
		s32Ret = sensor_dump_yuv();
		break;
	case 3:
		s32Ret = sensor_flip_mirror();
		break;
	case 4:
		s32Ret = sensor_linear_wdr_switch();
		break;
	case 5:
		printf("sensor_test: param = %s\n", param);
		s32Ret = sensor_ae_test(param);
		break;
	default:
		break;
	}
	// if (s32Ret != APP_SUCCESS) {
	// 	CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\n", op, s32Ret);
	// 	break;
	// }

	// sys_vi_deinit();
	// CVI_MSG_Deinit();

	return s32Ret;
}


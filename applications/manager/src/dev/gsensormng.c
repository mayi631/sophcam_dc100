#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "gsensormng.h"
#include "sysutils_eventhub.h"
#include "osal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// #define CONFIG_GSENSOR_ON 1
#ifdef CONFIG_GSENSOR_ON
/** macro define */
#define GSENSORMNG_CHECK_INTERVAL (300) /**< gsensor collision check interval, unit ms */

/** GSENSORMNG context */
typedef struct tagGSENSORMNG_CONTEXT_S {
	bool bInitState;
	pthread_mutex_t Mutex;
	pthread_t ThreadCheckId;
	bool bThreadRun;
	GSENSORMNG_CFG_S stCfg;
} GSENSORMNG_CONTEXT_S;

static GSENSORMNG_CONTEXT_S s_stGSENSORMNGContext = {.bInitState = false, .Mutex = PTHREAD_MUTEX_INITIALIZER};

static int32_t GSENSORMNG_InParmValidChck(const GSENSORMNG_CFG_S *stCfg)
{
	/** parm check */
	APPCOMM_CHECK_POINTER(stCfg, -1);
	APPCOMM_CHECK_POINTER((stCfg->enSensitity >= HAL_GSENSOR_SENSITITY_OFF &&
						   stCfg->enSensitity < HAL_GSENSOR_SENSITITY_BUIT),
						  GSENSORMNG_EINVAL);

	return 0;
}

static int32_t GSENSORMNG_InternalParmInit(const GSENSORMNG_CFG_S *pstCfg)
{
	APPCOMM_CHECK_POINTER(pstCfg, -1);
	s_stGSENSORMNGContext.bInitState = false;
	s_stGSENSORMNGContext.bThreadRun = false;
	memcpy(&s_stGSENSORMNGContext.stCfg, pstCfg, sizeof(GSENSORMNG_CFG_S));
	return 0;
}

static void GSENSORMNG_GsensorHalParmwitch(const GSENSORMNG_CFG_S *pstMngCfg, HAL_GSENSOR_CFG_S *psthalCfg)
{
	psthalCfg->enSensitity = pstMngCfg->enSensitity;
	psthalCfg->stAttr.u32SampleRate = pstMngCfg->stAttr.u32SampleRate;
	psthalCfg->busnum = 2;
	return;
}
static short collision_threshold[4][3] = {
	{10000, 10000, 10000},
	{75, 75, 75}, //{115,115,115},          //500//900//8G
	{50, 50, 50}, //{85,85,85},             //200//600//4G
	{25, 25, 25}  //{75,75,75},            //100//200//2G
};

static int32_t g_GSensorSensitivity = 0;
static int32_t g_GSensorLevel = 0;
// static int32_t  old_x, old_y, old_z;
static int8_t old_x, old_y, old_z;
bool collision_flag = false;

// 碰撞检测
static bool GSENSORMNG_Check_Collision(int32_t x, int32_t y, int32_t z)
{

	bool isOverAccelerometer = false;
	bool mIsCrashWarning = false;
	int8_t n_x, n_y, n_z;
	int8_t abs_x, abs_y, abs_z;

	n_x = (int8_t)(x & 0xff); // 8bit 有符号整数
	n_y = (int8_t)(y & 0xff);
	n_z = (int8_t)(z & 0xff);

	abs_x = abs(n_x - old_x);
	abs_y = abs(n_y - old_y);
	abs_z = abs(n_z - old_z);

	// CVI_LOGD("XYZ : %d, %d, %d  OLD_XYZ : %d, %d, %d \n", x, y, z, old_x, old_y, old_z);
	// CVI_LOGD("abs XYZ : %d, %d, %d  N_XYZ : %d, %d, %d \n", abs_x, abs_y, abs_z, n_x, n_y, n_z);

	isOverAccelerometer = abs_x > collision_threshold[g_GSensorLevel][0] ||
						  abs_y > collision_threshold[g_GSensorLevel][1] ||
						  abs_z > collision_threshold[g_GSensorLevel][2];

	if (collision_flag && isOverAccelerometer) {
		mIsCrashWarning = true;
	} else {
		mIsCrashWarning = false;
	}

	old_x = n_x;
	old_y = n_y;
	old_z = n_z;
	collision_flag = true;

	if (mIsCrashWarning) {
		//        CVI_LOGD("XYZ : %d, %d, %d  OLD_XYZ : %d, %d, %d \n", x, y, x, old_x, old_y, old_z);
	}

	return mIsCrashWarning ? true : false;
}

static int32_t conver_radian(double radian)
{
	int32_t degree;
	float angle;

	angle = radian * 180 / pi;
	degree = (int32_t)angle;

	return degree;
}
// 翻车检测
static bool GSENSORMNG_Check_Rollover(int32_t x, int32_t y, int32_t z)
{
	bool mIsRolloverWarning = false;
	double xAxis = (double)x;
	double yAxis = (double)y;
	double zAxis = (double)z;

	// double rad1 = atan(sqrt(yAxis * yAxis + xAxis * xAxis) / zAxis);
	// double rad2 = atan(xAxis / sqrt(yAxis * yAxis + zAxis * zAxis));
	double rad3 = atan(yAxis / sqrt(xAxis * xAxis + zAxis * zAxis)); // 只用到y轴的夹角即可判断

	// int32_t degree_z = conver_radian(rad1);
	// int32_t degree_x = conver_radian(rad2);
	int32_t degree_y = conver_radian(rad3);

	if (abs(degree_y) < ROLLOVER_MIN_DEGREE && !mIsRolloverWarning) {
		mIsRolloverWarning = true;
	} else if (abs(degree_y) > ROLLOVER_MAX_DEGREE && !mIsRolloverWarning) {
		mIsRolloverWarning = false;
	}

	if (mIsRolloverWarning) {
		// CVI_LOGD("degree X Y Z : %d, %d, %d \n", degree_x, degree_y, degree_z);
	}
	return mIsRolloverWarning ? true : false;
}
static void GSENSORMNG_CollisionEventPublish(void)
{
	EVENT_S stEvent;
	memset(&stEvent, '\0', sizeof(stEvent));
	stEvent.topic = EVENT_GSENSORMNG_COLLISION;
	EVENTHUB_Publish(&stEvent);
	//    CVI_LOGD("collision event\n");
	return;
}

static void GSENSORMNG_CollisionCheck(void)
{
	HAL_GSENSOR_VALUE_S stCurValue;
	int32_t ret;

	if (0 == HAL_GSENSOR_GetCurValue(&stCurValue)) {
		ret = GSENSORMNG_Check_Collision(stCurValue.s16XDirValue, stCurValue.s16YDirValue, stCurValue.s16ZDirValue);
		if (ret == true) {
			GSENSORMNG_CollisionEventPublish();
		}
		ret = GSENSORMNG_Check_Rollover(stCurValue.s16XDirValue, stCurValue.s16YDirValue, stCurValue.s16ZDirValue);
		if (ret == true) {
			// CVI_LOGD("Check_Rollover is true\n");
		}
	}
	return;
}

static void *GSENSORMNG_CollisionCheckThread(void *pData)
{
	/** Set Task Name */
	prctl(PR_SET_NAME, "gsensor_level", 0, 0, 0);

	while (s_stGSENSORMNGContext.bThreadRun) {
		GSENSORMNG_CollisionCheck();
		cvi_usleep(GSENSORMNG_CHECK_INTERVAL * 1000);
	}

	CVI_LOGD("gsensor check thread exit\n");
	return NULL;
}

int32_t GSENSORMNG_Init(const GSENSORMNG_CFG_S *pstCfg)
{
	int32_t s32Ret;

	if (GSENSORMNG_InParmValidChck(pstCfg) != 0) {
		CVI_LOGE("parm check error\n");
		return GSENSORMNG_EINVAL;
	}
	g_GSensorSensitivity = pstCfg->enSensitity;
	g_GSensorLevel = pstCfg->enSensitity;
	MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);

	if (true == s_stGSENSORMNGContext.bInitState) {
		CVI_LOGE("gsensormng has already been started\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_EINITIALIZED;
	}

	extern HAL_GSENSOR_OBJ_S gsensorObj;
	s32Ret = HAL_GSENSOR_Register(&gsensorObj);
	if (s32Ret < 0) {
		CVI_LOGE("sensor obj register failed\n");
		return GSENSORMNG_EINTER;
	}

	HAL_GSENSOR_CFG_S stGsensorCfg;
	GSENSORMNG_GsensorHalParmwitch(pstCfg, &stGsensorCfg);
	s32Ret = HAL_GSENSOR_Init(&stGsensorCfg);

	if (0 != s32Ret) {
		CVI_LOGE("HAL_GSENSOR_Init Failed\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_EINTER;
	}
	// GSENSORMNG_OpenInterrupt(0);
	GSENSORMNG_InternalParmInit(pstCfg);

	/* Create Gsensor collision Check Task */
	s_stGSENSORMNGContext.bThreadRun = true;
	s32Ret = pthread_create(&s_stGSENSORMNGContext.ThreadCheckId, NULL, GSENSORMNG_CollisionCheckThread, NULL);
	if (0 != s32Ret) {
		CVI_LOGE("Create Collsion Check Thread Fail!\n");
		HAL_GSENSOR_DeInit();
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_ETHREAD;
	}
	CVI_LOGD("g_GSensorSensitivity = %d g_GSensorSensitivity = %d\n", g_GSensorSensitivity, g_GSensorSensitivity);

	s_stGSENSORMNGContext.bInitState = true;
	MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
	return 0;
}

void GSENSORMNG_MenuSetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity)
{
	g_GSensorLevel = enSensitity;
}

int32_t GSENSORMNG_SetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity)
{
	int32_t s32Ret;
	APPCOMM_CHECK_POINTER((enSensitity >= HAL_GSENSOR_SENSITITY_OFF &&
						   enSensitity < HAL_GSENSOR_SENSITITY_BUIT),
						  GSENSORMNG_EINVAL);

	MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
	if (false == s_stGSENSORMNGContext.bInitState) {
		CVI_LOGW("gsensor manage no init\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_ENOINIT;
	}

	s32Ret = HAL_GSENSOR_SetSensitity((HAL_GSENSOR_SENSITITY_E)enSensitity);
	if (0 != s32Ret) {
		CVI_LOGE("gsensor set Sensitity error\n");
		s32Ret = GSENSORMNG_EINTER;
	}
	MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
	return s32Ret;
}

int32_t GSENSORMNG_GetAttr(GSENSORMNG_ATTR_S *pstAttr)
{
	APPCOMM_CHECK_POINTER(pstAttr, GSENSORMNG_EINVAL);

	MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
	if (false == s_stGSENSORMNGContext.bInitState) {
		CVI_LOGW("gsensor manage no init\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_ENOINIT;
	}
	pstAttr->u32SampleRate = s_stGSENSORMNGContext.stCfg.stAttr.u32SampleRate;
	MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
	return 0;
}

int32_t GSENSORMNG_SetAttr(const GSENSORMNG_ATTR_S *pstAttr)
{
	HAL_GSENSOR_ATTR_S stGsensorAttr;
	int32_t s32Ret;
	APPCOMM_CHECK_POINTER(pstAttr, GSENSORMNG_EINVAL);

	MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
	if (false == s_stGSENSORMNGContext.bInitState) {
		CVI_LOGW("gsensor manage no init\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_ENOINIT;
	}
	stGsensorAttr.u32SampleRate = pstAttr->u32SampleRate;

	s32Ret = HAL_GSENSOR_SetAttr(&stGsensorAttr);
	if (0 != s32Ret) {
		CVI_LOGE("gsensor set gensor attr error\n");
		s32Ret = GSENSORMNG_EINTER;
	}
	MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
	return s32Ret;
}

int32_t GSENSORMNG_OpenInterrupt(int32_t IntNum)
{
	int32_t s32Ret;

	s32Ret = HAL_GSENSOR_OpenInterrupt(IntNum);
	if (0 != s32Ret) {
		CVI_LOGE("open gensor interrupt failed\n");
		s32Ret = GSENSORMNG_EINTER;
	}
	return s32Ret;
}

int32_t GSENSORMNG_DeInit(void)
{
	int32_t s32Ret;
	MUTEX_LOCK(s_stGSENSORMNGContext.Mutex);
	/** Destory Gsensor Check Task */
	if (false == s_stGSENSORMNGContext.bInitState) {
		CVI_LOGW("gsensor manage no init\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_ENOINIT;
	}
	s_stGSENSORMNGContext.bThreadRun = false;

	s32Ret = pthread_join(s_stGSENSORMNGContext.ThreadCheckId, NULL);
	if (0 != s32Ret) {
		CVI_LOGE("Join collsion Thread Fail!\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_ETHREAD;
	}

	/** Close hal gsensor */
	s32Ret = HAL_GSENSOR_DeInit();

	if (0 != s32Ret) {
		CVI_LOGE("HAL_GSENSOR_DeInit Fail!\n");
		MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
		return GSENSORMNG_EINTER;
	}
	s_stGSENSORMNGContext.bInitState = false;

	MUTEX_UNLOCK(s_stGSENSORMNGContext.Mutex);
	return 0;
}
#else
int32_t GSENSORMNG_Init(const GSENSORMNG_CFG_S *pstCfg)
{
	return 0;
}

int32_t GSENSORMNG_SetSensitity(HAL_GSENSOR_SENSITITY_E enSensitity)
{
	return 0;
}

int32_t GSENSORMNG_GetAttr(GSENSORMNG_ATTR_S *pstAttr)
{
	return 0;
}

int32_t GSENSORMNG_SetAttr(const GSENSORMNG_ATTR_S *pstAttr)
{
	return 0;
}

int32_t GSENSORMNG_DeInit(void)
{
	return 0;
}

#endif
int32_t GSENSORMNG_RegisterEvent(void)
{
	int32_t s32Ret = 0;
	s32Ret = EVENTHUB_RegisterTopic(EVENT_GSENSORMNG_COLLISION);
	if (0 != s32Ret) {
		CVI_LOGE("Register collision event fail\n");
		return GSENSORMNG_EREGISTEREVENT;
	}
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* #ifdef __cplusplus */
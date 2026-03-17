#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "cvi_vpss.h"
// #include "sample_comm.h"
#include "cvi_buffer.h"
#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_vpss.h"
#include "cvi_vo.h"
#include "cvi_isp.h"
#include "cvi_venc.h"
#include "cvi_vdec.h"
#include "cvi_gdc.h"
#include "cvi_region.h"
#include "cvi_bin.h"
#include "osal.h"
//#define CVI_LOG_LEVEL CVI_LOG_VERBOSE
#include "cvi_log.h"

#include "mapi.h"
#include "mapi_internal.h"

typedef struct MEM_INFO_S {
	uint64_t phy_addr;
	void *vir_addr;
	struct MEM_INFO_S *next;
} MEM_INFO_T;

typedef struct VbPoolMapping {
	void *vir_addr;
	VB_POOL pool_id;
	size_t size;
	uint64_t u64PhyAddr;
	struct VbPoolMapping *next;
} VbPoolMapping_T;

static VbPoolMapping_T *g_vb_info = NULL;
static OSAL_MUTEX_S vb_mutex = OSAL_MUTEX_INITIALIZER;
static MEM_INFO_T *g_mem_info = NULL;
static OSAL_MUTEX_S mem_mutex = OSAL_MUTEX_INITIALIZER;

static void COMM_SYS_Exit(void)
{
    CVI_VB_Exit();
    CVI_SYS_Exit();
}

static int COMM_SYS_Init(VB_CONFIG_S *pstVbConfig)
{
    CVI_S32 s32Ret = CVI_FAILURE;

    COMM_SYS_Exit();

    if (pstVbConfig == NULL) {
        CVI_LOGE("input parameter is null or VBPoolCnt = 0, it is invaild!\n");
        return MAPI_ERR_FAILURE;
    }

    s32Ret = CVI_SYS_Init();
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("SYS_Init failed!\n");
        return s32Ret;
    }

    s32Ret = CVI_VB_SetConfig(pstVbConfig);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VB_SetConf failed!\n");
        return s32Ret;
    }

    s32Ret = CVI_VB_Init();
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VB_Init failed!\n");
        return s32Ret;
    }

    return MAPI_SUCCESS;
}

/// pass resolution and blkcnt directly for now
/// and assuming yuv420 for now
/// TODO: refactor to attribute struct
int MAPI_Media_Init(MAPI_MEDIA_SYS_ATTR_T *attr)
{
    //struct sigaction sa;
    //memset(&sa, 0, sizeof(struct sigaction));
    //sigemptyset(&sa.sa_mask);
    //sa.sa_sigaction = _SYS_HandleSig;
    //sa.sa_flags = SA_SIGINFO|SA_RESETHAND;    // Reset signal handler to system default after signal triggered
    //sigaction(SIGINT, &sa, NULL);
    //sigaction(SIGTERM, &sa, NULL);

    VB_CONFIG_S      stVbConf;
    uint32_t sameVbSizeCnt = 0;
    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

    for (unsigned i = 0; i < attr->vb_pool_num; i++) {
        uint32_t blk_size;
        if (attr->vb_pool[i].is_frame) {
            blk_size = get_frame_size(
                    attr->vb_pool[i].vb_blk_size.frame.width,
                    attr->vb_pool[i].vb_blk_size.frame.height,
                    attr->vb_pool[i].vb_blk_size.frame.fmt);
        } else {
            blk_size = attr->vb_pool[i].vb_blk_size.size;
        }
        uint32_t blk_num = attr->vb_pool[i].vb_blk_num;
        uint32_t j = 0;
        bool sameVbSize = false;
        for (j = 0; j < i; j++) {
            if (stVbConf.astCommPool[j].u32BlkSize == blk_size) {
                sameVbSize = true;
                sameVbSizeCnt++;
                break;
            }
        }
        if (sameVbSize == true) {
            stVbConf.astCommPool[j].u32BlkCnt     += blk_num;
        } else {
            stVbConf.astCommPool[i - sameVbSizeCnt].u32BlkSize    = blk_size;
            stVbConf.astCommPool[i - sameVbSizeCnt].u32BlkCnt     = blk_num;
            stVbConf.astCommPool[i - sameVbSizeCnt].enRemapMode   = VB_REMAP_MODE_CACHED;
        }
        CVI_LOGI("VB pool[%d] BlkSize %d BlkCnt %d\n", i, blk_size, blk_num);
    }
    stVbConf.u32MaxPoolCnt        = attr->vb_pool_num - sameVbSizeCnt;

    int ret = MAPI_SUCCESS;
    CVI_S32 rc = COMM_SYS_Init(&stVbConf);
    if (rc != MAPI_SUCCESS) {
        CVI_LOGE("COMM_SYS_Init fail, rc = %#x\n", rc);
        ret = MAPI_ERR_FAILURE;
        goto error;
    }

	rc = CVI_SYS_SetVIVPSSMode(&attr->stVIVPSSMode);
	if (rc != CVI_SUCCESS) {
		CVI_LOGE("SYS_SetVIVPSSMode failed with %#x\n", rc);
		ret = MAPI_ERR_FAILURE;
		goto error;
	}

    #ifdef CHIP_184X
	rc = CVI_VPSS_SetMode(&attr->stVPSSMode);
    #else
    rc = CVI_SYS_SetVPSSModeEx(&attr->stVPSSMode);
    #endif
	if (rc != CVI_SUCCESS) {
		CVI_LOGE("CVI_VPSS_SetMode failed with %#x\n", rc);
		ret = MAPI_ERR_FAILURE;
		goto error;
	}

    return ret;

error:
    COMM_SYS_Exit();
    return ret;
}

int MAPI_Media_Deinit(void)
{
    COMM_SYS_Exit();
    return MAPI_SUCCESS;
}

int MAPI_SYS_VI_VPSS_Mode_Init(MAPI_MEDIA_SYS_ATTR_T *attr){
    CVI_S32 rc = MAPI_SUCCESS;

    rc = CVI_SYS_SetVIVPSSMode(&attr->stVIVPSSMode);
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("SYS_SetVIVPSSMode failed with %#x\n", rc);
        return MAPI_ERR_FAILURE;
    }

    #ifdef CHIP_184X
    rc = CVI_VPSS_SetMode(&attr->stVPSSMode);
    #else
    rc = CVI_SYS_SetVPSSModeEx(&attr->stVPSSMode);
    #endif
    if (rc != CVI_SUCCESS) {
        CVI_LOGE("CVI_VPSS_SetMode failed with %#x\n", rc);
        return MAPI_ERR_FAILURE;
    }

    return MAPI_SUCCESS;
}

int MAPI_SYS_VB_Init(MAPI_MEDIA_SYS_ATTR_T *attr){
    VB_CONFIG_S      stVbConf;
    uint32_t sameVbSizeCnt = 0;
    CVI_S32 s32Ret = CVI_SUCCESS;

    memset(&stVbConf, 0, sizeof(VB_CONFIG_S));

    for (unsigned i = 0; i < attr->vb_pool_num; i++) {
        uint32_t blk_size;
        if (attr->vb_pool[i].is_frame) {
            blk_size = get_frame_size(
                    attr->vb_pool[i].vb_blk_size.frame.width,
                    attr->vb_pool[i].vb_blk_size.frame.height,
                    attr->vb_pool[i].vb_blk_size.frame.fmt);
        } else {
            blk_size = attr->vb_pool[i].vb_blk_size.size;
        }
        uint32_t blk_num = attr->vb_pool[i].vb_blk_num;
        uint32_t j = 0;
        bool sameVbSize = false;
        for (j = 0; j < i; j++) {
            if (stVbConf.astCommPool[j].u32BlkSize == blk_size) {
                sameVbSize = true;
                sameVbSizeCnt++;
                break;
            }
        }
        if (sameVbSize == true) {
            stVbConf.astCommPool[j].u32BlkCnt     += blk_num;
        } else {
            stVbConf.astCommPool[i - sameVbSizeCnt].u32BlkSize    = blk_size;
            stVbConf.astCommPool[i - sameVbSizeCnt].u32BlkCnt     = blk_num;
            stVbConf.astCommPool[i - sameVbSizeCnt].enRemapMode   = VB_REMAP_MODE_CACHED;
        }
        CVI_LOGI("VB pool[%d] BlkSize %d BlkCnt %d\n", i, blk_size, blk_num);
    }
    stVbConf.u32MaxPoolCnt        = attr->vb_pool_num - sameVbSizeCnt;

    s32Ret = CVI_VB_SetConfig(&stVbConf);
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VB_SetConf failed!\n");
        return s32Ret;
    }

    s32Ret = CVI_VB_Init();
    if (s32Ret != CVI_SUCCESS) {
        CVI_LOGE("VB_Init failed!\n");
        return s32Ret;
    }
    return s32Ret;
}

//
// VB Frame helper functions
//
int MAPI_ReleaseFrame(VIDEO_FRAME_INFO_S *frm)
{
    VB_BLK blk = CVI_VB_PhysAddr2Handle(frm->stVFrame.u64PhyAddr[0]);
    if (blk == VB_INVALID_HANDLE) {
		if (frm->stVFrame.pPrivateData == 0) {
			CVI_LOGE("phy-address invalid to locate.\n");
			return MAPI_ERR_FAILURE;
		}
		// blk = (VB_BLK)frm->stVFrame.pPrivateData;
	}

    if (CVI_VB_ReleaseBlock(blk) != MAPI_SUCCESS)
		return MAPI_ERR_FAILURE;
    return MAPI_SUCCESS;
}

int MAPI_GetFrameFromMemory_YUV(VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt, void *data)
{
    CVI_LOG_ASSERT(fmt == PIXEL_FORMAT_YUV_PLANAR_420,
        "Not support fmt %d yet\n", fmt);
    CVI_LOG_ASSERT(width % 2 == 0, "width not align\n");
    CVI_LOG_ASSERT(height % 2 == 0, "height not align\n");

    VB_BLK blk;
    VB_CAL_CONFIG_S stVbCalConfig;

    COMMON_GetPicBufferConfig(width, height, fmt, DATA_BITWIDTH_8,
            COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

    frm->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    frm->stVFrame.enPixelFormat = fmt;
    frm->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    frm->stVFrame.enColorGamut = COLOR_GAMUT_BT709;
    frm->stVFrame.u32Width = width;
    frm->stVFrame.u32Height = height;
    frm->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
    frm->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
    frm->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
    frm->stVFrame.u32TimeRef = 0;
    frm->stVFrame.u64PTS = 0;
    frm->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

    blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
    if (blk == VB_INVALID_HANDLE) {
        CVI_LOGE("Can't acquire vb block\n");
        return MAPI_ERR_FAILURE;
    }

    frm->u32PoolId = CVI_VB_Handle2PoolId(blk);
    frm->stVFrame.u32Length[0] = ALIGN(stVbCalConfig.u32MainYSize,
                                       stVbCalConfig.u16AddrAlign);
    frm->stVFrame.u32Length[1] = frm->stVFrame.u32Length[2]
                               = ALIGN(stVbCalConfig.u32MainCSize,
                                       stVbCalConfig.u16AddrAlign);

    frm->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
    frm->stVFrame.u64PhyAddr[1] = frm->stVFrame.u64PhyAddr[0]
                                + frm->stVFrame.u32Length[0];
    frm->stVFrame.u64PhyAddr[2] = frm->stVFrame.u64PhyAddr[1]
                                + frm->stVFrame.u32Length[1];
    uint32_t image_size = frm->stVFrame.u32Length[0]
                        + frm->stVFrame.u32Length[1]
                        + frm->stVFrame.u32Length[2];
    frm->stVFrame.pu8VirAddr[0] = (uint8_t *)CVI_SYS_MmapCache(
            frm->stVFrame.u64PhyAddr[0], image_size);
    frm->stVFrame.pu8VirAddr[1] = frm->stVFrame.pu8VirAddr[0]
                                + frm->stVFrame.u32Length[0];
    frm->stVFrame.pu8VirAddr[2] = frm->stVFrame.pu8VirAddr[1]
                                + frm->stVFrame.u32Length[1];

    //uint32_t u32LumaSize, u32ChromaSize;
    //
    // CAUTION: this has issue when U V size is not 32 pixel aligned
    //          they don't have same shape as Y
    //
    // We only handle shapes that stride == width here
    //
    CVI_LOGD("stride0:%d  stride1:%d\n", frm->stVFrame.u32Stride[0], frm->stVFrame.u32Stride[1]);
    //CVI_LOG_ASSERT(frm->stVFrame.u32Stride[0] == frm->stVFrame.u32Width, "not align\n");
    //CVI_LOG_ASSERT(frm->stVFrame.u32Stride[1] == frm->stVFrame.u32Width / 2, "not align\n");
    //u32LumaSize   =  frm->stVFrame.u32Stride[0] * frm->stVFrame.u32Height;
    //u32ChromaSize =  frm->stVFrame.u32Stride[1] * frm->stVFrame.u32Height / 2;

    uint8_t *data_ptr = (uint8_t *)data;
    for (int i = 0; i < 3; ++i) {
        if (frm->stVFrame.u32Length[i] == 0)
            continue;

        uint32_t height_step = (i == 0) ?
                        frm->stVFrame.u32Height : frm->stVFrame.u32Height / 2;
        uint32_t width_step = (i == 0) ?
                        frm->stVFrame.u32Width : frm->stVFrame.u32Width / 2;
        uint8_t *frm_ptr = frm->stVFrame.pu8VirAddr[i];
        for (uint32_t j = 0; j < height_step; ++j) {
            memcpy(frm_ptr, data_ptr, width_step);
            frm_ptr += frm->stVFrame.u32Stride[i];
            data_ptr += width_step;
        }
        CVI_SYS_IonFlushCache(frm->stVFrame.u64PhyAddr[i],
                       frm->stVFrame.pu8VirAddr[i],
                       frm->stVFrame.u32Length[i]);
    }
    CVI_SYS_Munmap(frm->stVFrame.pu8VirAddr[0], image_size);
    for (int i = 0; i < 3; ++i) {
        frm->stVFrame.pu8VirAddr[i] = NULL;
    }

    return MAPI_SUCCESS;
}

static uint32_t getFrameSize_YUV(uint32_t w, uint32_t h, PIXEL_FORMAT_E fmt)
{
    if (fmt == PIXEL_FORMAT_YUV_PLANAR_420) {
        return (w * h * 3) >> 1;
    } else if (fmt == PIXEL_FORMAT_YUV_PLANAR_422) {
        return (w * h * 2);
    } else {
        CVI_LOG_ASSERT(0, "Unsupported fmt %d\n", fmt);
    }
    return 0;
}

int MAPI_GetFrameFromFile_YUV(VIDEO_FRAME_INFO_S *frame,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt,
        const char *filaneme, uint32_t frame_no)
{
    FILE *fp = fopen(filaneme, "rb");
    if (fp == NULL) {
        CVI_LOGE("Input file %s open failed !\n", filaneme);
        return MAPI_ERR_FAILURE;
    }

    uint32_t frame_size = getFrameSize_YUV(width, height, fmt);
    char *data = (char *)malloc(frame_size);
    if (!data) {
        CVI_LOGE("malloc frame buffer failed\n");
        fclose(fp);
        return MAPI_ERR_FAILURE;
    }

    fseek(fp, 0, SEEK_END);
    unsigned int file_size = ftell(fp);
    unsigned int num_frames = file_size / frame_size;
    if (num_frames < (frame_no +1)) {
        CVI_LOGE("file %s size %d to small, frame_size %d, no. %d\n",
                filaneme, file_size, frame_size, frame_no);
        free(data);
        fclose(fp);
        return MAPI_ERR_FAILURE;
    }
    rewind(fp);

    fseek(fp, frame_size * frame_no, SEEK_SET);
    fread((void *)data, 1, frame_size, fp);
    int ret = MAPI_GetFrameFromMemory_YUV(frame,
                width, height, fmt, data);

    free(data);
    fclose(fp);
    return ret;
}

/// utils function
int MAPI_SaveFramePixelData(VIDEO_FRAME_INFO_S *frm, const char *name)
{
	FILE *fp;
	uint32_t u32len, u32DataLen;

	fp = fopen(name, "w");
	if (fp == NULL) {
		CVI_LOGE("open data file error\n");
		return MAPI_ERR_FAILURE;
	}
	for (int i = 0; i < 3; ++i) {
		u32DataLen = frm->stVFrame.u32Stride[i] * frm->stVFrame.u32Height;
		if (u32DataLen == 0)
			continue;
		if (i > 0 && ((frm->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
			(frm->stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
			(frm->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
			u32DataLen >>= 1;

		frm->stVFrame.pu8VirAddr[i]
			= CVI_SYS_Mmap(frm->stVFrame.u64PhyAddr[i], frm->stVFrame.u32Length[i]);

		u32len = fwrite(frm->stVFrame.pu8VirAddr[i], u32DataLen, 1, fp);
		if (u32len <= 0) {
			CVI_LOGE("fwrite data(%d) error\n", i);
			break;
		}
		CVI_SYS_Munmap(frm->stVFrame.pu8VirAddr[i], frm->stVFrame.u32Length[i]);
	}

	fclose(fp);
	return MAPI_SUCCESS;
}

int MAPI_AllocateFrame_ByPoolID(VB_POOL pool,VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt) {
    VB_BLK blk;
    VB_CAL_CONFIG_S stVbCalConfig;
    COMMON_GetPicBufferConfig(width, height, fmt, DATA_BITWIDTH_8,
                              COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

    frm->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    frm->stVFrame.enPixelFormat = fmt;
    frm->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    frm->stVFrame.enColorGamut = COLOR_GAMUT_BT709;
    frm->stVFrame.u32Width = width;
    frm->stVFrame.u32Height = height;
    frm->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
    frm->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
    frm->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
    frm->stVFrame.u32TimeRef = 0;
    frm->stVFrame.u64PTS = 0;
    frm->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

    CVI_LOGI("Allocate VB block with size %d\n", stVbCalConfig.u32VBSize);

    blk = CVI_VB_GetBlock(pool, stVbCalConfig.u32VBSize);
    if (blk == (unsigned long)CVI_INVALID_HANDLE) {
        CVI_LOGE("Can't acquire VB block for size %d\n",
            stVbCalConfig.u32VBSize);
        return MAPI_ERR_FAILURE;
    }

    frm->u32PoolId = CVI_VB_Handle2PoolId(blk);
    frm->stVFrame.u32Length[0] = ALIGN(stVbCalConfig.u32MainYSize,
                                       stVbCalConfig.u16AddrAlign);
    frm->stVFrame.u32Length[1] = frm->stVFrame.u32Length[2]
                               = ALIGN(stVbCalConfig.u32MainCSize,
                                       stVbCalConfig.u16AddrAlign);

    frm->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
    frm->stVFrame.u64PhyAddr[1] = frm->stVFrame.u64PhyAddr[0]
                                  + frm->stVFrame.u32Length[0];
    frm->stVFrame.u64PhyAddr[2] = frm->stVFrame.u64PhyAddr[1]
                                  + frm->stVFrame.u32Length[1];
    for (int i = 0; i < 3; ++i) {
        frm->stVFrame.pu8VirAddr[i] = NULL;
    }

    return MAPI_SUCCESS;
}

int MAPI_AllocateFrame(VIDEO_FRAME_INFO_S *frm,
        uint32_t width, uint32_t height, PIXEL_FORMAT_E fmt) {
    VB_BLK blk;
    VB_CAL_CONFIG_S stVbCalConfig;
    COMMON_GetPicBufferConfig(width, height, fmt, DATA_BITWIDTH_8,
                              COMPRESS_MODE_NONE, DEFAULT_ALIGN, &stVbCalConfig);

    frm->stVFrame.enCompressMode = COMPRESS_MODE_NONE;
    frm->stVFrame.enPixelFormat = fmt;
    frm->stVFrame.enVideoFormat = VIDEO_FORMAT_LINEAR;
    frm->stVFrame.enColorGamut = COLOR_GAMUT_BT709;
    frm->stVFrame.u32Width = width;
    frm->stVFrame.u32Height = height;
    frm->stVFrame.u32Stride[0] = stVbCalConfig.u32MainStride;
    frm->stVFrame.u32Stride[1] = stVbCalConfig.u32CStride;
    frm->stVFrame.u32Stride[2] = stVbCalConfig.u32CStride;
    frm->stVFrame.u32TimeRef = 0;
    frm->stVFrame.u64PTS = 0;
    frm->stVFrame.enDynamicRange = DYNAMIC_RANGE_SDR8;

    CVI_LOGD("Allocate VB block with size %d\n", stVbCalConfig.u32VBSize);
    blk = CVI_VB_GetBlock(VB_INVALID_POOLID, stVbCalConfig.u32VBSize);
    if (blk == (unsigned long)CVI_INVALID_HANDLE) {
        CVI_LOGE("Can't acquire VB block for size %d\n",
            stVbCalConfig.u32VBSize);
        return MAPI_ERR_FAILURE;
    }

    frm->u32PoolId = CVI_VB_Handle2PoolId(blk);
    frm->stVFrame.u32Length[0] = ALIGN(stVbCalConfig.u32MainYSize,
                                       stVbCalConfig.u16AddrAlign);
    frm->stVFrame.u32Length[1] = frm->stVFrame.u32Length[2]
                               = ALIGN(stVbCalConfig.u32MainCSize,
                                       stVbCalConfig.u16AddrAlign);

    frm->stVFrame.u64PhyAddr[0] = CVI_VB_Handle2PhysAddr(blk);
    frm->stVFrame.u64PhyAddr[1] = frm->stVFrame.u64PhyAddr[0]
                                  + frm->stVFrame.u32Length[0];
    frm->stVFrame.u64PhyAddr[2] = frm->stVFrame.u64PhyAddr[1]
                                  + frm->stVFrame.u32Length[1];
    for (int i = 0; i < 3; ++i) {
        frm->stVFrame.pu8VirAddr[i] = NULL;
    }

    return MAPI_SUCCESS;
}

VB_POOL MAPI_CreateVbPool(PIXEL_FORMAT_E encode_type, int input_width, int input_height, uint32_t vb_cnt)
{
    VB_POOL VbPool = VB_INVALID_POOLID;
    VB_POOL_CONFIG_S stVbPoolCfg;
    memset(&stVbPoolCfg, 0, sizeof(VB_POOL_CONFIG_S));
    stVbPoolCfg.u32BlkSize	= COMMON_GetPicBufferSize(input_width, input_height, encode_type,
                DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);

    stVbPoolCfg.u32BlkCnt = vb_cnt;
    stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
    VbPool = CVI_VB_CreatePool(&stVbPoolCfg);

    if (VbPool == VB_INVALID_POOLID) {
        CVI_LOGE("CVI_VB_CreatePool Fail\n");
    }

    return VbPool;
}

int MAPI_DestroyVbPool(VB_POOL VbPool)
{
    int s32Ret = MAPI_SUCCESS;

    if(VbPool != VB_INVALID_POOLID){
        s32Ret = CVI_VB_DestroyPool(VbPool);
    }

    return s32Ret;
}

static void get_frame_plane_num_and_mem_size(VIDEO_FRAME_INFO_S *frm,
    int *plane_num, size_t *mem_size)
{
    if (frm->stVFrame.enPixelFormat == PIXEL_FORMAT_RGB_888_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_BGR_888_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_444
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_HSV_888_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_FP32_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_INT32_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_UINT32_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_BF16_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_INT16_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_UINT16_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_INT8_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_UINT8_C3_PLANAR
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_YUYV
            || frm->stVFrame.enPixelFormat == PIXEL_FORMAT_UYVY) {
        *plane_num = 3;
        // check phyaddr
        // CVI_LOG_ASSERT(frm->stVFrame.u64PhyAddr[1] - frm->stVFrame.u64PhyAddr[0]
        //                == frm->stVFrame.u32Length[0],
        //                "phy addr not continue 0, fmt = %d\n",
        //                frm->stVFrame.enPixelFormat);
        // CVI_LOG_ASSERT(frm->stVFrame.u64PhyAddr[2] - frm->stVFrame.u64PhyAddr[1]
        //                == frm->stVFrame.u32Length[1],
        //                "phy addr not continue 1, fmt = %d\n",
        //                frm->stVFrame.enPixelFormat);
    } else if (frm->stVFrame.enPixelFormat == PIXEL_FORMAT_NV21) {
        *plane_num = 2;
    } else {
        *plane_num = 1;
    }

    *mem_size = 0;
    for (int i = 0; i < *plane_num; ++i) {
        *mem_size += frm->stVFrame.u32Length[i];
    }
}

int MAPI_FrameMmap(VIDEO_FRAME_INFO_S *frm, bool enable_cache) {
    int plane_num = 0;
    size_t mem_size = 0;
    get_frame_plane_num_and_mem_size(frm, &plane_num, &mem_size);

    void *vir_addr = NULL;
    if (enable_cache) {
        vir_addr = CVI_SYS_MmapCache(frm->stVFrame.u64PhyAddr[0], mem_size);
    } else {
        vir_addr = CVI_SYS_Mmap(frm->stVFrame.u64PhyAddr[0], mem_size);
    }
    CVI_LOG_ASSERT(vir_addr, "mmap failed\n");

    //CVI_SYS_IonInvalidateCache(frm->stVFrame.u64PhyAddr[0], vir_addr, mem_size);
    CVI_U64 plane_offset = 0;
    for (int i = 0; i < plane_num; ++i) {
        frm->stVFrame.pu8VirAddr[i] = (uint8_t *)vir_addr + plane_offset;
        plane_offset += frm->stVFrame.u32Length[i];
    }

    return MAPI_SUCCESS;
}

int MAPI_FrameMunmap(VIDEO_FRAME_INFO_S *frm)
{
    int plane_num = 0;
    size_t mem_size = 0;
    get_frame_plane_num_and_mem_size(frm, &plane_num, &mem_size);

    void *vir_addr = (void *)frm->stVFrame.pu8VirAddr[0];
    CVI_SYS_Munmap(vir_addr, mem_size);

    for (int i = 0; i < plane_num; ++i) {
        frm->stVFrame.pu8VirAddr[i] = NULL;
    }

    return MAPI_SUCCESS;
}

int MAPI_FrameFlushCache(VIDEO_FRAME_INFO_S *frm)
{
    int plane_num = 0;
    size_t mem_size = 0;
    get_frame_plane_num_and_mem_size(frm, &plane_num, &mem_size);

    void *vir_addr = (void *)frm->stVFrame.pu8VirAddr[0];
    CVI_U64 phy_addr = frm->stVFrame.u64PhyAddr[0];

    CVI_SYS_IonFlushCache(phy_addr, vir_addr, mem_size);
    return MAPI_SUCCESS;
}

int MAPI_FrameInvalidateCache(VIDEO_FRAME_INFO_S *frm)
{
    int plane_num = 0;
    size_t mem_size = 0;
    get_frame_plane_num_and_mem_size(frm, &plane_num, &mem_size);

    void *vir_addr = (void *)frm->stVFrame.pu8VirAddr[0];
    CVI_U64 phy_addr = frm->stVFrame.u64PhyAddr[0];

    CVI_SYS_IonInvalidateCache(phy_addr, vir_addr, mem_size);

    return MAPI_SUCCESS;
}

int MAPI_AllocBuffer(CVI_U64 *pu64PhyAddr, void **ppVirtAddr,
    uint32_t u32Len, bool cached)
{
    int s32Ret;
    CVI_LOG_ASSERT(pu64PhyAddr, "pu64PhyAddr failed\n");
    CVI_LOG_ASSERT(ppVirtAddr, "ppVirtAddr failed\n");

    if(cached)
        s32Ret = CVI_SYS_IonAlloc_Cached(pu64PhyAddr, ppVirtAddr, "cardv_app", u32Len);
    else
        s32Ret = CVI_SYS_IonAlloc(pu64PhyAddr, ppVirtAddr, "cardv_app", u32Len);

    if(s32Ret){
        CVI_LOGE("Ion alloc failed\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

int MAPI_FreeBuffer(CVI_U64 u64PhyAddr, void *pVirtAddr)
{
    int s32Ret;
    CVI_LOG_ASSERT(u64PhyAddr, "pu64PhyAddr failed\n");
    CVI_LOG_ASSERT(pVirtAddr, "ppVirtAddr failed\n");

    s32Ret = CVI_SYS_IonFree(u64PhyAddr, pVirtAddr);
    if(s32Ret){
        CVI_LOGE("SYS_IonFree failed\n");
        return MAPI_ERR_FAILURE;
    }
    return MAPI_SUCCESS;
}

static void meminfo_push(uint64_t phy_addr, void *vir_addr)
{
	MEM_INFO_T *mem = (MEM_INFO_T *)malloc(sizeof(MEM_INFO_T));
	mem->phy_addr = phy_addr;
	mem->vir_addr = vir_addr;
	mem->next = NULL;
	OSAL_MUTEX_Lock(&mem_mutex);
	if (g_mem_info == NULL) {
		g_mem_info = mem;
	} else {
		MEM_INFO_T *head = g_mem_info;
		while (head->next) {
			head = head->next;
		}
		head->next = mem;
	}
	OSAL_MUTEX_Unlock(&mem_mutex);
}

static uint64_t meminfo_pop(void *vir_addr)
{
	OSAL_MUTEX_Lock(&mem_mutex);
	uint64_t phy_addr = 0;
	if (g_mem_info != NULL) {
		MEM_INFO_T *mem = NULL;
		if (g_mem_info->vir_addr == vir_addr) {
			mem = g_mem_info;
			g_mem_info = mem->next;
		} else {
			MEM_INFO_T *tmp = g_mem_info;
			while (tmp->next) {
				if (tmp->next->vir_addr == vir_addr) {
					mem = tmp->next;
					tmp->next = mem->next;
					break;
				}
				tmp = tmp->next;
			}
		}
		if (mem) {
			phy_addr = mem->phy_addr;
			free(mem);
		}
	}

	OSAL_MUTEX_Unlock(&mem_mutex);
	return phy_addr;
}

void *MAPI_MemAllocate(size_t size, const char *name)
{
    void *vir_addr = NULL;
    unsigned long long int phy_addr = 0;
    int32_t ret = CVI_SYS_IonAlloc_Cached(&phy_addr, &vir_addr, name, size);
    if (ret == CVI_SUCCESS) {
        meminfo_push(phy_addr, vir_addr);
        return vir_addr;
    }
    return NULL;
}

void MAPI_MemFree(void *vir_addr)
{
    if (!vir_addr) {
        return;
    }
    unsigned long long int phy_addr = meminfo_pop(vir_addr);
    if (phy_addr > 0) {
        int32_t ret = CVI_SYS_IonFree(phy_addr, vir_addr);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("SYS_IonFree failed, error [%d]", ret);
        }
    }
}

static void MEM_AddVbPoolMapping(void *vir_addr, uint64_t u64PhyAddr, VB_POOL pool_id, size_t size)
{
	VbPoolMapping_T *vb_info = (VbPoolMapping_T *)malloc(sizeof(VbPoolMapping_T));
	vb_info->vir_addr = vir_addr;
	vb_info->u64PhyAddr = u64PhyAddr;
	vb_info->size = size;
	vb_info->pool_id = pool_id;
	vb_info->next = NULL;
	OSAL_MUTEX_Lock(&vb_mutex);
	if (g_vb_info == NULL) {
		g_vb_info = vb_info;
	} else {
		VbPoolMapping_T *head = g_vb_info;
		while (head->next) {
			head = head->next;
		}
		head->next = vb_info;
	}
	OSAL_MUTEX_Unlock(&vb_mutex);

	return;
}

static VbPoolMapping_T MEM_FindVbPoolByMapp(void *vir_addr)
{
	VbPoolMapping_T VbPoolMapping = {0};
	if (g_vb_info != NULL) {
		VbPoolMapping_T *vb_info = NULL;
		if (g_vb_info->vir_addr == vir_addr) {
			vb_info = g_vb_info;
			g_vb_info = vb_info->next;
		} else {
			VbPoolMapping_T *tmp = g_vb_info;
			while (tmp->next) {
				if (tmp->next->vir_addr == vir_addr) {
					vb_info = tmp->next;
					tmp->next = vb_info->next;
					break;
				}
				tmp = tmp->next;
			}
		}
		if (vb_info) {
			memcpy(&VbPoolMapping, vb_info, sizeof(VbPoolMapping_T));
			CVI_SYS_Munmap(vir_addr, VbPoolMapping.size);
			free(vb_info);
		}
	}

	return VbPoolMapping;
}

void *MAPI_MemAllocateVb(size_t size)
{
    VB_POOL_CONFIG_S stVbPoolCfg = {0};
    void *vir_addr = NULL;

    stVbPoolCfg.u32BlkSize = size;
    stVbPoolCfg.u32BlkCnt = 1;
    stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
    VB_BLK vb_blk;
    VB_POOL CreatVbHand = VB_INVALID_POOLID;

    CreatVbHand = CVI_VB_CreatePool(&stVbPoolCfg);

    if (CreatVbHand == VB_INVALID_POOLID) {
        CVI_LOGE("VB_CreatePool Fail");
        return NULL;
    }

    vb_blk = CVI_VB_GetBlock(CreatVbHand, stVbPoolCfg.u32BlkSize);
    if (vb_blk == VB_INVALID_HANDLE) {
        CVI_LOGE("VB_GetBlock Fail");
        CVI_VB_DestroyPool(CreatVbHand);
        return NULL;
    }

    uint64_t u64PhyAddr = CVI_VB_Handle2PhysAddr(vb_blk);
    if (u64PhyAddr == 0) {
        CVI_LOGE("VB_Handle2PhysAddr Fail");
        CVI_VB_ReleaseBlock(vb_blk);
        CVI_VB_DestroyPool(CreatVbHand);
        return NULL;
    }

    vir_addr = CVI_SYS_MmapCache(u64PhyAddr, stVbPoolCfg.u32BlkSize);
    if (vir_addr == NULL) {
        CVI_LOGE("CVI_SYS_Mmap Fail");
        CVI_VB_ReleaseBlock(vb_blk);
        CVI_VB_DestroyPool(CreatVbHand);
        return NULL;
    }

    MEM_AddVbPoolMapping(vir_addr, u64PhyAddr, CreatVbHand, size);

    return vir_addr;
}

void MAPI_MemVbFree(void *vir_addr)
{
	if (!vir_addr) {
		return;
	}
	VB_BLK vb_blk;
	int32_t ret = 0;
	OSAL_MUTEX_Lock(&vb_mutex);
	VbPoolMapping_T V_PooldMapp = MEM_FindVbPoolByMapp(vir_addr);
	OSAL_MUTEX_Unlock(&vb_mutex);

	if (V_PooldMapp.u64PhyAddr > 0) {
		vb_blk = CVI_VB_PhysAddr2Handle(V_PooldMapp.u64PhyAddr);
		if (vb_blk == VB_INVALID_HANDLE) {
			CVI_LOGE("VB_PhysAddr2Handle fail");
			return;
		}

		ret = CVI_VB_ReleaseBlock(vb_blk);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VB_ReleaseBlock fail");
			return;
		}

		ret = CVI_VB_DestroyPool(V_PooldMapp.pool_id);
		if (ret != CVI_SUCCESS) {
			CVI_LOGE("VB_DestroyPool fail");
			return;
		}
	} else {
		CVI_LOGE("Invalid virtual address\n");
	}

	return;
}



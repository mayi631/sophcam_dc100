
#include "mapi.h"
#include "mapi_vcap_internal.h"

MAPI_VCAP_ATTR_T vcap_attr_base = {
	.u8DevNum = 0,
	.attr_sns[0] = {
		.u8SnsId = 0,
		.u8WdrMode = 0,
		.u8I2cBusId = 0x3,
		.u8I2cSlaveAddr = 0xFF,
		.u8HwSync = 0,
		.u8MipiDev = 0xFF,
		.as8LaneId = {
			-1, -1, -1, -1, -1
		},
		.as8PNSwap = {
			0, 0, 0, 0, 0
		},
	},
	.attr_sns[1] = {
		.u8SnsId = 0,
		.u8WdrMode = 0,
		.u8I2cBusId = 0x0,
		.u8I2cSlaveAddr = 0xFF,
		.u8HwSync = 0,
		.u8MipiDev = 0xFF,
		.as8LaneId = {
			-1, -1, -1, -1, -1
		},
		.as8PNSwap = {
			0, 0, 0, 0, 0
		},
	},
	.attr_chn[0] = {
		.u32Width = 1920,
		.u32Height = 1080,
		.enPixelFmt = PIXEL_FORMAT_YUV_PLANAR_420,
		.enCompressMode = COMPRESS_MODE_NONE,
		.bMirror = 0,
		.bFlip = 0,
		.f32Fps = 25.0f,
	},
	.attr_chn[1] = {
		.u32Width = 1920,
		.u32Height = 1080,
		.enPixelFmt = PIXEL_FORMAT_YUV_PLANAR_420,
		.enCompressMode = COMPRESS_MODE_NONE,
		.bMirror = 0,
		.bFlip = 0,
		.f32Fps = 25.0f,
	},
};

CVI_S32 getVcapAttr(MAPI_VCAP_ATTR_T *pstVcapAttr)
{
	memcpy(pstVcapAttr, &vcap_attr_base, sizeof(MAPI_VCAP_ATTR_T));

	return CVI_SUCCESS;
}


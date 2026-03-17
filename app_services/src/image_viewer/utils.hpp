#pragma once

#include "mapi.h"
#include "cvi_log.h"

namespace image_viewer {
namespace utils {

int32_t cloneFrame(VIDEO_FRAME_INFO_S &target_frame, VIDEO_FRAME_INFO_S &source_frame)
{
    constexpr int32_t number_of_planar = 3;
    if (MAPI_AllocateFrame(&target_frame, source_frame.stVFrame.u32Width,
            source_frame.stVFrame.u32Height, PIXEL_FORMAT_YUV_PLANAR_420) != 0) {
        CVI_LOGE("MAPI allocate frame failed");
        return -1;
    }
    if (MAPI_FrameMmap(&target_frame, true) != 0) {
        CVI_LOGE("MAPI frame mmap failed");
        MAPI_ReleaseFrame(&target_frame);
        return -1;
    }
    // copy frame data to buffer frame
    for (int32_t i = 0; i < number_of_planar; ++i) {
        memcpy(static_cast<void *>(target_frame.stVFrame.pu8VirAddr[i]),
            static_cast<const void *>(source_frame.stVFrame.pu8VirAddr[i]),
            target_frame.stVFrame.u32Length[i]);
    }
    if (MAPI_FrameFlushCache(&target_frame) != 0) {
        CVI_LOGE("MAPI flush cache failed");
        MAPI_ReleaseFrame(&target_frame);
        return -1;
    }
    if (MAPI_FrameMunmap(&target_frame) != 0) {
        CVI_LOGE("MAPI frame munmap failed");
        MAPI_ReleaseFrame(&target_frame);
        return -1;
    }

    return 0;
}

} // namespace utils
} // namespace image_viewer

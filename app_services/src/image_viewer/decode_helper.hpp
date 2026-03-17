#pragma once

#include "demuxer.h"
#include "mapi_vdec.h"

namespace image_viewer {

class DecodeHelper final
{
public:
    static inline int32_t send(MAPI_VDEC_HANDLE_T handle, const DemuxerPacket &packet)
    {
        VDEC_STREAM_S stream = {};
        stream.u64PTS = packet.pts;
        stream.pu8Addr = packet.data;
        stream.u32Len = packet.size;
        stream.bEndOfFrame = true;
        stream.bEndOfStream = true;

        return MAPI_VDEC_SendStream(handle, &stream);
    }

    static inline int32_t get(MAPI_VDEC_HANDLE_T handle, VIDEO_FRAME_INFO_S &frame)
    {
        return MAPI_VDEC_GetFrame(handle, &frame);
    }

    static inline int32_t release(MAPI_VDEC_HANDLE_T handle, VIDEO_FRAME_INFO_S &frame)
    {
        return MAPI_VDEC_ReleaseFrame(handle, &frame);
    }
};

} // namespace image_viewer

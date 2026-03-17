#pragma once

#include <string>
#include <functional>
#include <memory>
#include "demuxer.h"
#include "thumbnail_extractor.h"
#include "mapi.h"
#include "display_window.hpp"

namespace image_viewer {

class ImageViewer final
{
public:
    using OutputHandler = std::function<void(VIDEO_FRAME_INFO_S *)>;

    ~ImageViewer();

    void setDecodeHandle(MAPI_VDEC_HANDLE_T handle);
    void setDisplayHandle(MAPI_DISP_HANDLE_T handle);
    int32_t displayThumbnail(const std::string &input, const POINT_S &pos, const SIZE_S &size);
    void setOutputHandler(const OutputHandler &handler);
    void setOutputHandler(OutputHandler &&handler);

private:
    int32_t getThumbnailPacket(const std::string &input, THUMBNAIL_PACKET_S &packet);
    int32_t getJpegThumbnail(const std::string &input, THUMBNAIL_PACKET_S &thumbnail);
    int32_t decodePacketToYUVFrame(DemuxerPacket &src_packet, VIDEO_FRAME_INFO_S &dst_frame);
    int32_t resizeFrameToDisplaySize(VIDEO_FRAME_INFO_S &src_frame, VIDEO_FRAME_INFO_S &dst_frame);
    int32_t displayFrame(VIDEO_FRAME_INFO_S &frame, const POINT_S &pos, const SIZE_S &size);

private:
    THUMBNAIL_EXTRACTOR_HANDLE_T extractor_handle{nullptr};
    MAPI_VDEC_HANDLE_T vdec_handle{nullptr};
    MAPI_DISP_HANDLE_T display_handle{nullptr};
    std::unique_ptr<DisplayWindow> display_window;
    OutputHandler output_handler;
};

} // namespace image_viewer

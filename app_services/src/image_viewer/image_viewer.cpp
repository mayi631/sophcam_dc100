#include "image_viewer.hpp"
#include "cvi_comm_video.h"
#include "cvi_log.h"
#include "decode_singleton.hpp"
#include "decode_helper.hpp"
#include "display_helper.hpp"
#include "utils.hpp"

namespace image_viewer {

ImageViewer::~ImageViewer()
{
    if (extractor_handle != nullptr) {
        THUMBNAIL_EXTRACTOR_Destroy(&extractor_handle);
    }
}

void ImageViewer::setDecodeHandle(MAPI_VDEC_HANDLE_T handle)
{
    vdec_handle = handle;
}

void ImageViewer::setDisplayHandle(MAPI_DISP_HANDLE_T handle)
{
    display_handle = handle;
}

int32_t ImageViewer::displayThumbnail(const std::string &input, const POINT_S &pos, const SIZE_S &size)
{
    THUMBNAIL_PACKET_S thumbnail_packet = {};
    VIDEO_FRAME_INFO_S yuv_frame = {};

    if (getThumbnailPacket(input, thumbnail_packet) != 0) {
        THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail_packet);
        return -1;
    }

    if (decodePacketToYUVFrame(thumbnail_packet, yuv_frame) != 0) {
        THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail_packet);
        return -1;
    }
    if (output_handler) {
        output_handler(&yuv_frame);
        MAPI_ReleaseFrame(&yuv_frame);
    } else {
        if (displayFrame(yuv_frame, pos, size) != 0) {
            THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail_packet);
            return -1;
        }
    }
    THUMBNAIL_EXTRACTOR_ClearPacket(&thumbnail_packet);

    return 0;
}

void ImageViewer::setOutputHandler(const OutputHandler &handler)
{
    output_handler = handler;
}

void ImageViewer::setOutputHandler(OutputHandler &&handler)
{
    output_handler = std::move(handler);
}

int32_t ImageViewer::getThumbnailPacket(const std::string &input, THUMBNAIL_PACKET_S &packet)
{
    if (extractor_handle == nullptr) {
        THUMBNAIL_EXTRACTOR_Create(&extractor_handle);
    }
    // get thumbnail packet from input file
    if (THUMBNAIL_EXTRACTOR_GetThumbnail(extractor_handle, input.c_str(),
            &packet) != 0) {
        CVI_LOGE("Extractor get thumbnail failed");
        return -1;
    }

    return 0;
}

int32_t ImageViewer::decodePacketToYUVFrame(DemuxerPacket &src_packet, VIDEO_FRAME_INFO_S &dst_frame)
{
    if (vdec_handle == nullptr) {
        vdec_handle = DecodeSingleton::getHandle();
    }
    // decode packet (jpg or png) to yuv
    if (DecodeHelper::send(vdec_handle, src_packet) != 0) {
        CVI_LOGE("Decoder helper send failed");
        return -1;
    }
    VIDEO_FRAME_INFO_S yuv_frame = {};
    if (DecodeHelper::get(vdec_handle, yuv_frame) != 0) {
        CVI_LOGE("Decoder helper get failed");
        return -1;
    }
    // decoded yuv frame can't send to vproc directly
    if (utils::cloneFrame(dst_frame, yuv_frame) != 0) {
        CVI_LOGE("Clone frame failed");
        DecodeHelper::release(vdec_handle, yuv_frame);
        return -1;
    }
    DecodeHelper::release(vdec_handle, yuv_frame);

    return 0;
}

int32_t ImageViewer::displayFrame(VIDEO_FRAME_INFO_S &frame, const POINT_S &pos, const SIZE_S &size)
{
    if (display_handle == nullptr) {
        CVI_LOGE("Screen no init\n");
        return -1;
    }
    if (!display_window) {
        display_window = std::make_unique<DisplayWindow>(pos, size);
    }

    if (!DisplayHelper::setAttributeIfNotEqual(display_window->getHandle(), pos, size)) {
        CVI_LOGE("Display helper set attribute failed");
        return -1;
    }
    if (DisplayHelper::send(display_window->getHandle(), frame) != 0) {
        CVI_LOGE("Display helper send failed");
        return -1;
    }

    return 0;
}

} // namespace image_viewer

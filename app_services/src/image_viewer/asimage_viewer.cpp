#include "image_viewer.h"
#include "check.hpp"
#include "cvi_log.h"
#include "image_viewer.hpp"

using namespace image_viewer;

int32_t IMAGE_VIEWER_Create(IMAGE_VIEWER_HANDLE_T *handle)
{
    if (!demuxer::utils::hasNullptr(*handle)) {
        CVI_LOGE("Viewer is not null");
        return -1;
    }

    *handle = new ImageViewer();

    return 0;
}

int32_t IMAGE_VIEWER_Destroy(IMAGE_VIEWER_HANDLE_T *handle)
{
    if (demuxer::utils::hasNullptr(handle, *handle)) {
        CVI_LOGF("Viewer is null");
        return -1;
    }

    ImageViewer *viewer = static_cast<ImageViewer *>(*handle);
    delete viewer;
    *handle = nullptr;

    return 0;
}

int32_t IMAGE_VIEWER_SetDecodeHandle(IMAGE_VIEWER_HANDLE_T handle,
        MAPI_VDEC_HANDLE_T decode_handle)
{
    if (demuxer::utils::hasNullptr(handle, decode_handle)) {
        CVI_LOGF("Viewer or decode handle is null");
        return -1;
    }

    ImageViewer *viewer = static_cast<ImageViewer *>(handle);
    viewer->setDecodeHandle(decode_handle);

    return 0;
}

int32_t IMAGE_VIEWER_SetDisplayHandle(IMAGE_VIEWER_HANDLE_T handle,
        MAPI_DISP_HANDLE_T display_handle)
{
    if (demuxer::utils::hasNullptr(handle, display_handle)) {
        CVI_LOGF("Viewer or display handle is null");
        return -1;
    }

    ImageViewer *viewer = static_cast<ImageViewer *>(handle);
    viewer->setDisplayHandle(display_handle);

    return 0;
}

int32_t IMAGE_VIEWER_DisplayThumbnail(IMAGE_VIEWER_HANDLE_T handle, const char *input,
    POINT_S pos, SIZE_S size)
{
    if (demuxer::utils::hasNullptr(handle, input)) {
        CVI_LOGF("Viewer or input is null");
        return -1;
    }

    ImageViewer *viewer = static_cast<ImageViewer *>(handle);
    if (viewer->displayThumbnail(input, pos, size) != 0) {
        return -1;
    }

    return 0;
}

int32_t IMAGE_VIEWER_SetOutputHandler(IMAGE_VIEWER_HANDLE_T handle,
        IMAGE_VIEWER_OUTPUT_HANDLER handler)
{
    if (demuxer::utils::hasNullptr(handle)) {
        CVI_LOGF("Viewer is null");
        return -1;
    }

    ImageViewer *viewer = static_cast<ImageViewer *>(handle);
    viewer->setOutputHandler(std::move(handler));

    return 0;
}

int32_t IMAGE_VIEWER_SetCustomArgOutputHandler(IMAGE_VIEWER_HANDLE_T handle,
        IMAGE_VIEWER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg)
{
    if (demuxer::utils::hasNullptr(handle)) {
        CVI_LOGF("Viewer is null");
        return -1;
    }

    ImageViewer *viewer = static_cast<ImageViewer *>(handle);
    auto &&wrapper_handler = [custom_arg, handler] (VIDEO_FRAME_INFO_S *frame) {
        handler(custom_arg, frame);
    };
    viewer->setOutputHandler(wrapper_handler);

    return 0;
}

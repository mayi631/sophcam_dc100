#pragma once

#include <stdexcept>
#include "cvi_comm_video.h"
#include "mapi.h"

namespace image_viewer {

class DisplayWindow final
{
public:
    DisplayWindow(const POINT_S &pos, const SIZE_S &size, int32_t display_id = 0)
    {
        // MAPI_WND_ATTR_T window_attr = {
        //     .wnd_x = static_cast<uint32_t>(pos.s32X),
        //     .wnd_y = static_cast<uint32_t>(pos.s32Y),
        //     .wnd_w = size.u32Width,
        //     .wnd_h = size.u32Height,
        //     .u32Priority = 0
        // };

        // if (MAPI_DISP_CreateWindow(&handle, display_id, &window_attr) != 0) {
        //     throw std::runtime_error("MAPI disp create window failed");
        // }
    }

    ~DisplayWindow()
    {
        // MAPI_DISP_DestroyWindow(handle);
    }

    MAPI_WND_HANDLE_T getHandle() const
    {
        return handle;
    }

private:
    MAPI_WND_HANDLE_T handle{nullptr};
};

} // namespace image_viewer

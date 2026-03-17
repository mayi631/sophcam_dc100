#pragma once

#include <stdexcept>
#include "singleton.hpp"
#include "mapi_vdec.h"

namespace image_viewer {

class DecodeSingleton final : private Singleton<DecodeSingleton>
{
public:
    friend class Singleton<DecodeSingleton>;

    static const MAPI_VDEC_HANDLE_T getHandle()
    {
        return getInstance().vdec_handle;
    }

private:
    DecodeSingleton()
    {
        MAPI_VDEC_CHN_ATTR_T vdec_attr;
        vdec_attr.codec = MAPI_VCODEC_JPG;
        vdec_attr.max_width = JPEGD_MAX_WIDTH;
        vdec_attr.max_height = JPEGD_MAX_HEIGHT;
        if (MAPI_VDEC_InitChn(&vdec_handle, &vdec_attr) != 0) {
            throw std::runtime_error("MAPI vdec init failed");
        }
    }

    virtual ~DecodeSingleton()
    {
        /// TODO: call MAPI_VDEC_DeinitChn will segmentation fault
        // MAPI_VDEC_DeinitChn(vdec_handle);
    }

private:
    MAPI_VDEC_HANDLE_T vdec_handle{nullptr};
};

} // namespace image_viewer

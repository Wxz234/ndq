#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_format.h"
#include "ndq/rhi_resource.h"

namespace ndq
{
    struct NDQ_TEX2D_RTV
    {
        uint32 MipSlice;
        uint32 PlaneSlice;
    };

    struct NDQ_RENDER_TARGET_VIEW_DESC
    {
        NDQ_RESOURCE_FORMAT Format;
        NDQ_RESOURCE_DIMENSION ViewDimension;
        NDQ_TEX2D_RTV Texture2D;
    };

    class IRenderTargetView
    {
    public:
        virtual NDQ_RENDER_TARGET_VIEW_DESC GetDesc() const = 0;
    };
}
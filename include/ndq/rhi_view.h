#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_format.h"
#include "ndq/rhi_resource.h"

namespace ndq
{
    class IView
    {
    public:
        virtual size_type GetHandle() const = 0;
    };

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

    class IRenderTargetView : public IView
    {
    public:
        virtual NDQ_RENDER_TARGET_VIEW_DESC GetDesc() const = 0;
    };

    struct NDQ_TEX2D_DSV
    {
        uint32 MipSlice;
    };

    enum NDQ_DSV_FLAGS
    {
        NDQ_DSV_FLAG_NONE = 0,
        NDQ_DSV_FLAG_READ_ONLY_DEPTH = 0x1,
        NDQ_DSV_FLAG_READ_ONLY_STENCIL = 0x2
    };

    struct NDQ_DEPTH_STENCIL_VIEW_DESC
    {
        NDQ_RESOURCE_FORMAT Format;
        NDQ_RESOURCE_DIMENSION ViewDimension;
        NDQ_DSV_FLAGS Flags;
        NDQ_TEX2D_DSV Texture2D;
    };

    class IDepthStencilView : public IView
    {
    public:
        virtual NDQ_DEPTH_STENCIL_VIEW_DESC GetDesc() const = 0;
    };

    struct NDQ_VERTEX_BUFFER_VIEW
    {
        uint64 BufferLocation;
        uint32 SizeInBytes;
        uint32 StrideInBytes;
    };
}
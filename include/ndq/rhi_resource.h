#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_format.h"

namespace ndq
{
    enum class NDQ_RESOURCE_DIMENSION
    {
        UNKNOWN = 0,
        BUFFER = 1,
        TEXTURE2D = 2,
    };

    enum class NDQ_RESOURCE_STATE
    {
        COMMON,
        RENDER_TARGET,
        READ,
        COPY_DEST,
    };

    enum class NDQ_RESOURCE_HEAP_TYPE
    {
        DEFAULT,
        UPLOAD,
        READBACK,
    };

    enum NDQ_RESOURCE_FLAGS
    {
        NDQ_RESOURCE_FLAG_NONE = 0,
        NDQ_RESOURCE_FLAG_ALLOW_RENDER_TARGET = 0x1,
        NDQ_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL = 0x2,
    };

    struct NDQ_BUFFER_DESC
    {
        NDQ_RESOURCE_FLAGS Flags;
        size_type SizeInBytes;
    };

    struct NDQ_TEXTURE2D_DESC
    {
        NDQ_RESOURCE_FLAGS Flags;
        NDQ_RESOURCE_FORMAT Format;
        uint64 Width;
        uint32 Height;
        uint16 MipLevels;
    };

    class IGraphicsResource
    {
    public:
        virtual NDQ_RESOURCE_DIMENSION GetType() const = 0;
    };

    class IGraphicsBuffer : public IGraphicsResource
    {
    public:
        virtual void Map(void** ppData) = 0;
        virtual void Unmap() = 0;
        virtual NDQ_BUFFER_DESC GetDesc() const = 0;
    };

    class IGraphicsTexture2D : public IGraphicsResource
    {
    public:
        virtual NDQ_TEXTURE2D_DESC GetDesc() const = 0;
    };
}
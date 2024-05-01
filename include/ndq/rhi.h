#pragma once

#include "ndq/platform.h"

namespace ndq
{
    enum class RESOURCE_STATE
    {
        COMMON,
        RENDER_TARGET,
        READ,
    };

    enum class RESOURCE_FORMAT
    {
        UNKNOWN,
        R8G8B8A8_UNORM,
    };

    enum class RESOURCE_HEAP_TYPE
    {
        DEFAULT,
        UPLOAD,
        READBACK,
    };

    enum RESOURCE_FLAGS
    {
        NONE = 0,
        ALLOW_RENDER_TARGET = 0x1,
    };

    struct GRAPHICS_BUFFER_DESC
    {
        RESOURCE_STATE State;
        RESOURCE_HEAP_TYPE Type;
        RESOURCE_FLAGS Flags;
        size_type SizeInBytes;
    };

    struct GRAPHICS_TEXTURE_DESC
    {
        RESOURCE_STATE State;
        RESOURCE_FLAGS Flags;
        RESOURCE_FORMAT Format;
        uint32 Width;
        uint32 Height;
        uint16 MipLevels;
    };

    enum class COMMAND_LIST_TYPE
    {
        GRAPHICS,
        COPY,
        COMPUTE,
    };

    enum class PRIMITIVE_TOPOLOGY
    {
        TRIANGLELIST = 4,
    };

    class IGraphicsResource
    {
    public:
        virtual void* GetRawResource() const = 0;
    };

    class IGraphicsBuffer : public IGraphicsResource
    {
    public:
        virtual void Map(void** ppData) = 0;
        virtual void Unmap() = 0;
    };

    class IGraphicsTexture2D : public IGraphicsResource
    {
    public:
        virtual uint32 GetWidth() const = 0;
        virtual uint32 GetHeight() const = 0;
        virtual RESOURCE_FORMAT GetFormat() const = 0;
    };

    class ICommandList
    {
    public:
        virtual void Open() = 0;
        virtual void SetPrimitiveTopology(PRIMITIVE_TOPOLOGY topology) = 0;
        virtual void Close() = 0;
        virtual COMMAND_LIST_TYPE GetType() const = 0;
    };

    class IGraphicsDevice
    {
    public:
        virtual void ExecuteCommandList(ICommandList* pList) = 0;
        virtual void Wait(COMMAND_LIST_TYPE type) = 0;
        virtual ICommandList* GetCommandList(COMMAND_LIST_TYPE type) = 0;
        virtual IGraphicsBuffer* AllocateBuffer(const GRAPHICS_BUFFER_DESC* pDesc) = 0;
        virtual IGraphicsTexture2D* AllocateTexture2D(const GRAPHICS_TEXTURE_DESC* pDesc) = 0;
        virtual void CollectResource(IGraphicsResource* pResource) = 0;
    };

    IGraphicsDevice* GetGraphicsDevice();
}
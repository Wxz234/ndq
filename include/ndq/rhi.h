#pragma once

#include "ndq/platform.h"

#include <memory>

namespace ndq
{
    enum class NDQ_RESOURCE_STATE
    {
        COMMON,
        RENDER_TARGET,
        READ,
    };

    enum class NDQ_RESOURCE_FORMAT
    {
        UNKNOWN,
        R8G8B8A8_UNORM,
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
    };

    enum class NDQ_COMMAND_LIST_TYPE
    {
        GRAPHICS,
        COPY,
        COMPUTE,
    };

    enum class NDQ_PRIMITIVE_TOPOLOGY
    {
        UNDEFINED = 0,
        POINTLIST = 1,
        LINELIST = 2,
        LINESTRIP = 3,
        TRIANGLELIST = 4,
    };

    enum class NDQ_SHADER_TYPE
    {
        VERTEX,
        PIXEL,
    };

    struct NDQ_GRAPHICS_BUFFER_DESC
    {
        NDQ_RESOURCE_STATE State;
        NDQ_RESOURCE_HEAP_TYPE Type;
        NDQ_RESOURCE_FLAGS Flags;
        size_type SizeInBytes;
    };

    struct NDQ_GRAPHICS_TEXTURE_DESC
    {
        NDQ_RESOURCE_STATE State;
        NDQ_RESOURCE_FLAGS Flags;
        NDQ_RESOURCE_FORMAT Format;
        uint32 Width;
        uint32 Height;
        uint16 MipLevels;
    };

    struct NDQ_SHADER_DEFINE
    {
        const wchar_t* Name;
        const wchar_t* Value;
    };

    struct NDQ_BUFFER_RTV
    {
        uint64 FirstElement;
        uint32 NumElements;
    };

    struct NDQ_TEX2D_RTV
    {
        uint32 MipSlice;
        uint32 PlaneSlice;
    };

    enum class NDQ_RTV_DIMENSION
    {
        UNKNOWN = 0,
        BUFFER = 1,
        TEXTURE2D = 4,
    };

    struct RENDER_TARGET_VIEW_DESC
    {
        NDQ_RESOURCE_FORMAT Format;
        NDQ_RTV_DIMENSION ViewDimension;
        union
        {
            NDQ_BUFFER_RTV Buffer;
            NDQ_TEX2D_RTV  Texture2D;
        };
    };

    class IShader
    {
    public:
        virtual NDQ_SHADER_TYPE GetShaderType() const = 0;
        virtual void* GetBlobPointer() const = 0;
        virtual size_type GetBlobSize() const = 0;
    };

    std::shared_ptr<IShader> CompileShaderFromFile(const wchar_t* filePath, const NDQ_SHADER_DEFINE* pDefines, uint32 defineCount, const wchar_t* entryPoint, NDQ_SHADER_TYPE shaderType);

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
        virtual NDQ_RESOURCE_FORMAT GetFormat() const = 0;
    };

    class ICommandList
    {
    public:
        virtual void Open() = 0;
        virtual void SetPrimitiveTopology(NDQ_PRIMITIVE_TOPOLOGY topology) = 0;
        virtual void SetVertexShader(IShader* pShader) = 0;
        virtual void SetPixelShader(IShader* pShader) = 0;
        virtual void Close() = 0;
        virtual NDQ_COMMAND_LIST_TYPE GetType() const = 0;
    };

    class IGraphicsDevice
    {
    public:
        virtual void ExecuteCommandList(ICommandList* pList) = 0;
        virtual void Wait(NDQ_COMMAND_LIST_TYPE type) = 0;
        virtual std::shared_ptr<ICommandList> GetCommandList(NDQ_COMMAND_LIST_TYPE type) = 0;
        virtual std::shared_ptr<IGraphicsBuffer> AllocateBuffer(const NDQ_GRAPHICS_BUFFER_DESC* pDesc) = 0;
        virtual std::shared_ptr<IGraphicsTexture2D> AllocateTexture2D(const NDQ_GRAPHICS_TEXTURE_DESC* pDesc) = 0;
    };

    std::shared_ptr<IGraphicsDevice> GetGraphicsDevice();
}
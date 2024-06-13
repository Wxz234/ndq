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
        COPY_DEST,
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

    struct NDQ_SHADER_DEFINE
    {
        const wchar_t* Name;
        const wchar_t* Value;
    };

    enum class NDQ_RESOURCE_DIMENSION
    {
        UNKNOWN = 0,
        BUFFER = 1,
        TEXTURE2D = 2,
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

    class IRenderTargetView
    {
    public:
        virtual NDQ_RENDER_TARGET_VIEW_DESC GetDesc() const = 0;
    };

    class IShader
    {
    public:
        virtual NDQ_SHADER_TYPE GetShaderType() const = 0;
        virtual void* GetBlobPointer() const = 0;
        virtual size_type GetBlobSize() const = 0;
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

    class ICommandList
    {
    public:
        virtual void Open() = 0;
        virtual void SetPrimitiveTopology(NDQ_PRIMITIVE_TOPOLOGY topology) = 0;
        virtual void SetVertexShader(IShader* pShader) = 0;
        virtual void SetPixelShader(IShader* pShader) = 0;
        virtual void DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation) = 0;
        virtual void DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation) = 0;
        virtual void Close() = 0;
        virtual NDQ_COMMAND_LIST_TYPE GetType() const = 0;
    };

    class IGraphicsDevice
    {
    public:
        virtual void ExecuteCommandList(ICommandList* pList) = 0;
        virtual void Wait(NDQ_COMMAND_LIST_TYPE type) = 0;
        virtual std::shared_ptr<ICommandList> GetCommandList(NDQ_COMMAND_LIST_TYPE type) const = 0;
        virtual std::shared_ptr<IGraphicsBuffer> AllocateUploadBuffer(const NDQ_BUFFER_DESC* pDesc) = 0;
        virtual std::shared_ptr<IGraphicsBuffer> AllocateDefaultBuffer(const NDQ_BUFFER_DESC* pDesc) = 0;
        virtual std::shared_ptr<IGraphicsBuffer> AllocateReadbackBuffer(const NDQ_BUFFER_DESC* pDesc) = 0;
        virtual std::shared_ptr<IGraphicsTexture2D> AllocateTexture2D(const NDQ_TEXTURE2D_DESC* pDesc) = 0;
        virtual std::shared_ptr<IRenderTargetView> GetInternalCurrentRenderTargetView() const = 0;
    };

    std::shared_ptr<IGraphicsDevice> GetGraphicsDevice();
    std::shared_ptr<IShader> CompileShaderFromFile(const wchar_t* filePath, NDQ_SHADER_TYPE shaderType, const wchar_t* entryPoint, const NDQ_SHADER_DEFINE* pDefines, uint32 defineCount);
}
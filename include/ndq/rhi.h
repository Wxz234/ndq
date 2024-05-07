#pragma once

#include "ndq/platform.h"

#include <memory>

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

    enum class COMMAND_LIST_TYPE
    {
        GRAPHICS,
        COPY,
        COMPUTE,
    };

    enum class PRIMITIVE_TOPOLOGY
    {
        UNDEFINED = 0,
        POINTLIST = 1,
        LINELIST = 2,
        LINESTRIP = 3,
        TRIANGLELIST = 4,
    };

    enum class SHADER_TYPE
    {
        VERTEX,
        PIXEL,
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

    struct SHADER_DEFINE {
        const wchar_t* Name;
        const wchar_t* Value;
    };

    class IShader
    {
    public:
        virtual SHADER_TYPE GetShaderType() const = 0;
        virtual void* GetBlobPointer() const = 0;
        virtual size_type GetBlobSize() const = 0;
    };

    std::shared_ptr<IShader> CompileShaderFromFile(const wchar_t* filePath, const SHADER_DEFINE* pDefines, uint32 defineCount, const wchar_t* entryPoint, SHADER_TYPE shaderType);

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
        virtual void SetVertexShader(IShader* pShader) = 0;
        virtual void SetPixelShader(IShader* pShader) = 0;
        virtual void Close() = 0;
        virtual COMMAND_LIST_TYPE GetType() const = 0;
    };

    class IGraphicsDevice
    {
    public:
        virtual void ExecuteCommandList(ICommandList* pList) = 0;
        virtual void Wait(COMMAND_LIST_TYPE type) = 0;
        virtual std::shared_ptr<ICommandList> GetCommandList(COMMAND_LIST_TYPE type) = 0;
        virtual std::shared_ptr<IGraphicsBuffer> AllocateBuffer(const GRAPHICS_BUFFER_DESC* pDesc) = 0;
        virtual std::shared_ptr<IGraphicsTexture2D> AllocateTexture2D(const GRAPHICS_TEXTURE_DESC* pDesc) = 0;
    };

    std::shared_ptr<IGraphicsDevice> GetGraphicsDevice();
}
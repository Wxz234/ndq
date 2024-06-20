#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_command.h"
#include "ndq/rhi_format.h"
#include "ndq/rhi_input_layout.h"
#include "ndq/rhi_resource.h"
#include "ndq/rhi_shader.h"
#include "ndq/rhi_view.h"

#include <memory>

namespace ndq
{
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
        virtual std::shared_ptr<IRenderTargetView> GetInternalRenderTargetView(uint32 index) const = 0;
        virtual std::shared_ptr<IGraphicsTexture2D> GetInternalSwapchainTexture2D(uint32 index) const = 0;
        virtual uint32 GetCurrentFrameIndex() const = 0;
        virtual std::shared_ptr<IShader> CreateShaderFromFile(const wchar_t* filePath, const wchar_t* entryPoint, NDQ_SHADER_TYPE shaderType, const NDQ_SHADER_DEFINE* pDefines, uint32 defineCount) = 0;
    };

    std::shared_ptr<IGraphicsDevice> GetGraphicsDevice();
}
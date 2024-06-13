#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_shader.h"

namespace ndq
{
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

    class ICommandList
    {
    public:
        virtual void Open() = 0;
        virtual void SetRenderTargets(uint32 numRenderTargetDescriptors, const size_type* pRenderTargetDescriptors, const size_type* pDepthStencilDescriptor) = 0;
        virtual void SetPrimitiveTopology(NDQ_PRIMITIVE_TOPOLOGY topology) = 0;
        virtual void SetVertexShader(IShader* pShader) = 0;
        virtual void SetPixelShader(IShader* pShader) = 0;
        virtual void DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation) = 0;
        virtual void DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation) = 0;
        virtual void Close() = 0;
        virtual NDQ_COMMAND_LIST_TYPE GetType() const = 0;
    };
}
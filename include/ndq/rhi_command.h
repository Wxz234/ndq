#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_input_layout.h"
#include "ndq/rhi_resource.h"
#include "ndq/rhi_shader.h"
#include "ndq/rhi_view.h"

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

    // IA VS HS TS DS GS SO RS PS OM

    class ICommandList
    {
    public:
        virtual void Open() = 0;
        virtual void ResourceBarrier(IGraphicsResource* pRes, NDQ_RESOURCE_STATE brfore, NDQ_RESOURCE_STATE after) = 0;
        virtual void ClearRenderTargetView(IRenderTargetView* pRTV, const float colorRGBA[4]) = 0;
        virtual void DrawInstanced(uint32 VertexCountPerInstance, uint32 InstanceCount, uint32 StartVertexLocation, uint32 StartInstanceLocation) = 0;
        virtual void DrawIndexedInstanced(uint32 IndexCountPerInstance, uint32 InstanceCount, uint32 StartIndexLocation, int32 BaseVertexLocation, uint32 StartInstanceLocation) = 0;
        virtual void Close() = 0;
        virtual NDQ_COMMAND_LIST_TYPE GetType() const = 0;

        virtual void IASetInputLayout(IInputLayout* pInputLayout) = 0;
        virtual void IASetPrimitiveTopology(NDQ_PRIMITIVE_TOPOLOGY topology) = 0;
        virtual void IASetVertexBuffers(uint32 startSlot, uint32 numViews, const NDQ_VERTEX_BUFFER_VIEW* pViews) = 0;
        virtual void VSSetVertexShader(IShader* pShader) = 0;
        virtual void PSSetPixelShader(IShader* pShader) = 0;
        virtual void OMSetRenderTargets(uint32 numViews, IRenderTargetView* const* ppRenderTargetViews, IDepthStencilView* pDepthStencilView) = 0;
    };
}
#include "ndq/ApplicationWindow.h"
#include "ndq/CommandList.h"
#include "ndq/GraphicsDevice.h"

#include "Pixel.h"
#include "Vertex.h"

#include <combaseapi.h>
#include <d3d12.h>
#include <d3dcommon.h>
#include <d3dx12_barriers.h>
#include <d3dx12_core.h>
#include <d3dx12_default.h>
#include <dxgiformat.h>

using namespace ndq;

struct MainWindow : ApplicationWindow
{
    MainWindow() : ApplicationWindow(800, 600, L"Triangle"), mPipelineSatae(nullptr), mRootSignature(nullptr), mCmdList(nullptr) {}

    void initialize()
    {
        auto rawDevice = (ID3D12Device*)GraphicsDevice::getGraphicsDevice()->getRawGraphicsDevice();
        rawDevice->CreateRootSignature(1, vertexBlob, sizeof(vertexBlob), IID_PPV_ARGS(&mRootSignature));

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.InputLayout = { nullptr, 0 };
        psoDesc.pRootSignature = mRootSignature;
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexBlob, sizeof(vertexBlob));
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelBlob, sizeof(pixelBlob));
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.SampleMask = 0xffffffff;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        
        rawDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPipelineSatae));

        mCmdList = GraphicsDevice::getGraphicsDevice()->createCommandList(CommandList::CLT_GRAPHICS);
    }

    void update(float)
    {
        mCmdList->open();
        auto rawCmdList = (ID3D12GraphicsCommandList*)mCmdList->getRawCommandList();
        rawCmdList->SetGraphicsRootSignature(mRootSignature);
        rawCmdList->SetPipelineState(mPipelineSatae);

        CD3DX12_VIEWPORT viewport(0.f, 0.f, mWidth, mHeight);
        rawCmdList->RSSetViewports(1, &viewport);
        CD3DX12_RECT scissorRect(0, 0, mWidth, mHeight);
        rawCmdList->RSSetScissorRects(1, &scissorRect);

        D3D12_VERTEX_BUFFER_VIEW emptyVertexBuffer{};
        rawCmdList->IASetVertexBuffers(0, 1, &emptyVertexBuffer);
        rawCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        auto currentResource = (ID3D12Resource*)GraphicsDevice::getGraphicsDevice()->getCurrentResource();
        {
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            rawCmdList->ResourceBarrier(1, &barrier);
        }

        GraphicsDevice::getGraphicsDevice()->setCurrentResourceRenderTarget(mCmdList);
        const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        GraphicsDevice::getGraphicsDevice()->clearCurrentResourceRenderTargetView(mCmdList, clearColor);
        rawCmdList->DrawInstanced(3, 1, 0, 0);

        {
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            rawCmdList->ResourceBarrier(1, &barrier);
        }
        
        mCmdList->close();
        CommandList* lists[1] = { mCmdList };
        GraphicsDevice::getGraphicsDevice()->executeCommandList(CommandList::CLT_GRAPHICS, 1, lists);
        GraphicsDevice::getGraphicsDevice()->wait(CommandList::CLT_GRAPHICS);
    }

    void finalize()
    {
        mPipelineSatae->Release();
        mRootSignature->Release();
        GraphicsDevice::getGraphicsDevice()->destroyCommandList(mCmdList);
    }

    ID3D12PipelineState* mPipelineSatae;
    ID3D12RootSignature* mRootSignature;
    CommandList* mCmdList;
};

WIN_MAIN_MACRO(MainWindow)
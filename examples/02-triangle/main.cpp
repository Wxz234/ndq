#include <Windows.h>

#include "d3dx12.h"

#include <combaseapi.h>
#include <dxgiformat.h>
#include <wrl/client.h>

#include "ndq/core/blob.h"
#include "ndq/core/resource.h"
#include "ndq/platform/window.h"
#include "ndq/rhi/command_list.h"
#include "ndq/rhi/device.h"
#include "ndq/rhi/shader.h"

using namespace ndq;

struct Window : IWindow
{
    Window()
    {
        Title = L"Triangle";
    }

    void Initialize()
    {
        const wchar_t* VertexArgs[] =
        {
            L"-T", L"vs_6_0",
            L"-E", L"main"
        };
        const wchar_t* PixelArgs[] =
        {
            L"-T", L"ps_6_0",
            L"-E", L"main"
        };

        auto pVertexBlob = LoadShaderFromPath(L"vertex.hlsl", VertexArgs, 4);
        auto pPixelBlob = LoadShaderFromPath(L"pixel.hlsl", PixelArgs, 4);
       
        auto pRawDevice = GetGraphicsDevice()->GetRawDevice();
        pRawDevice->CreateRootSignature(1, pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature));

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = {};
        PsoDesc.InputLayout = { nullptr, 0 };
        PsoDesc.pRootSignature = pRootSignature.Get();
        PsoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexBlob->GetBufferPointer(), pVertexBlob->GetBufferSize());
        PsoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelBlob->GetBufferPointer(), pPixelBlob->GetBufferSize());
        PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        PsoDesc.DepthStencilState.DepthEnable = FALSE;
        PsoDesc.DepthStencilState.StencilEnable = FALSE;
        PsoDesc.SampleMask = 0xffffffff;
        PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        PsoDesc.NumRenderTargets = 1;
        PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        PsoDesc.SampleDesc.Count = 1;
        
        pRawDevice->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&pPipelineSatae));

        pCmdList = GetGraphicsDevice()->CreateCommandList(NDQ_COMMAND_LIST_TYPE::GRAPHICS);
    }

    void Update(float)
    {
        pCmdList->Open(pPipelineSatae.Get());
        auto pRawCmdList = pCmdList->GetRawCommandList();
        pRawCmdList->SetGraphicsRootSignature(pRootSignature.Get());

        CD3DX12_VIEWPORT viewport(0.f, 0.f, Width, Height);
        pRawCmdList->RSSetViewports(1, &viewport);
        CD3DX12_RECT scissorRect(0, 0, Width, Height);
        pRawCmdList->RSSetScissorRects(1, &scissorRect);

        D3D12_VERTEX_BUFFER_VIEW emptyVertexBuffer{};
        pRawCmdList->IASetVertexBuffers(0, 1, &emptyVertexBuffer);
        pRawCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        
        auto CurrentResource = GetGraphicsDevice()->GetCurrentResource();
        {
            auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentResource, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
            pRawCmdList->ResourceBarrier(1, &Barrier);
        }

        auto RtvHandle = GetGraphicsDevice()->GetCurrentRenderTargetView();
        pRawCmdList->OMSetRenderTargets(1, &RtvHandle, FALSE, nullptr);
        const float ClearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        pRawCmdList->ClearRenderTargetView(RtvHandle, ClearColor, 0, nullptr);
        pRawCmdList->DrawInstanced(3, 1, 0, 0);

        {
            auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(CurrentResource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
            pRawCmdList->ResourceBarrier(1, &Barrier);
        }
        
        pCmdList->Close();

        GetGraphicsDevice()->ExecuteCommandList(pCmdList.Get());

        GetGraphicsDevice()->Wait(NDQ_COMMAND_LIST_TYPE::GRAPHICS);
    }

    void Finalize()
    {
        pCmdList.Reset();
    }

    Microsoft::WRL::ComPtr<ID3D12PipelineState> pPipelineSatae;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
    TRefCountPtr<ICommandList> pCmdList;
};

WIN_MAIN_MACRO(Window)
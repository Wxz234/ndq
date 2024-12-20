#include "directx/d3dx12.h"

#include "ndq/rhi/core.h"
#include "ndq/rhi/device.h"
#include "ndq/window.h"

using namespace ndq;

struct Window : public IWindow
{
    Window()
    {
        Title = L"Triangle";
    }

    void Initialize()
    {
        auto pDevice = GetGraphicsDevice();

        auto VertexBlob = LoadShader(L"vertex.cso");
        auto PixelBlob = LoadShader(L"pixel.cso");

        auto pRawDevice = pDevice->GetRawDevice();

        D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = {};
        //PsoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        //PsoDesc.pRootSignature = m_rootSignature.Get();
        //PsoDesc.VS = CD3DX12_SHADER_BYTECODE(VertexBlob.Get());
        //PsoDesc.PS = CD3DX12_SHADER_BYTECODE(PixelBlob.Get());
        PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        PsoDesc.DepthStencilState.DepthEnable = FALSE;
        PsoDesc.DepthStencilState.StencilEnable = FALSE;
        PsoDesc.SampleMask = 0xffffffff;
        PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        PsoDesc.NumRenderTargets = 1;
        PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        PsoDesc.SampleDesc.Count = 1;
        //pRawDevice->CreateGraphicsPipelineState()

        DeleteShader(VertexBlob);
        DeleteShader(PixelBlob);
    }

    void Update(float t) {}
    void Finalize() {}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    Window MyWindow;
    return MyWindow.Run();
}
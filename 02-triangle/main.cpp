#include <Windows.h>

#include "ndq/rhi.h"
#include "ndq/window.h"

using namespace ndq;

struct App : public IApplication
{
    App()
    {
        Title = L"triangle";
    }

    void Initialize()
    {
        pGraphicsDevice = GetGraphicsDevice();
        pCmdList = pGraphicsDevice->GetCommandList(NDQ_COMMAND_LIST_TYPE::GRAPHICS);
        pVertexShader = pGraphicsDevice->CreateShaderFromFile(L"vertex.hlsl", L"main", NDQ_SHADER_TYPE::VERTEX, nullptr, 0);
        pPixelShader = pGraphicsDevice->CreateShaderFromFile(L"pixel.hlsl", L"main", NDQ_SHADER_TYPE::PIXEL, nullptr, 0);

        NDQ_INPUT_ELEMENT_DESC InputDesc[] =
        {
            { "POSITION", NDQ_RESOURCE_FORMAT::R32G32B32A32_FLOAT, 0 }
        };
        pInputLayout = pGraphicsDevice->CreateInputLayout(InputDesc, 1);

        NDQ_BUFFER_DESC BufferDesc{};
        BufferDesc.SizeInBytes = sizeof(float) * 4 * 3;
        pVertex = pGraphicsDevice->AllocateBuffer(&BufferDesc, NDQ_RESOURCE_HEAP_TYPE::UPLOAD, NDQ_RESOURCE_STATE::UNIVERSAL_READ);
    }

    void Update(float t)
    {
        auto CurrentIndex = pGraphicsDevice->GetCurrentFrameIndex();
        auto CurrentRTV = pGraphicsDevice->GetInternalRenderTargetView(CurrentIndex);
        auto CurrentTexture = pGraphicsDevice->GetInternalSwapchainTexture2D(CurrentIndex);
        IRenderTargetView* CurrentRTVArray[] = { CurrentRTV.get() };

        pCmdList->Open();
        pCmdList->IASetInputLayout(pInputLayout.get());
        pCmdList->IASetPrimitiveTopology(NDQ_PRIMITIVE_TOPOLOGY::TRIANGLELIST);
        pCmdList->OMSetRenderTargets(1, CurrentRTVArray, nullptr);
        pCmdList->VSSetVertexShader(pVertexShader.get());
        pCmdList->PSSetPixelShader(pPixelShader.get());
        pCmdList->ResourceBarrier(CurrentTexture.get(), NDQ_RESOURCE_STATE::PRESENT, NDQ_RESOURCE_STATE::RENDER_TARGET);
        float Color[4] = { 1.0f, 0.3f, 0.6f, 1.0f };
        pCmdList->ClearRenderTargetView(CurrentRTV.get(), Color);
        pCmdList->ResourceBarrier(CurrentTexture.get(), NDQ_RESOURCE_STATE::RENDER_TARGET, NDQ_RESOURCE_STATE::PRESENT);
        pCmdList->Close();
        pGraphicsDevice->ExecuteCommandList(pCmdList.get());
    }

    std::shared_ptr<IShader> pVertexShader;
    std::shared_ptr<IShader> pPixelShader;
    std::shared_ptr<IGraphicsDevice> pGraphicsDevice;
    std::shared_ptr<ICommandList> pCmdList;
    std::shared_ptr<IInputLayout> pInputLayout;
    std::shared_ptr<IGraphicsBuffer> pVertex;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    App MyApp;
    return MyApp.Run();
}
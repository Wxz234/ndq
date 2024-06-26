#include <Windows.h>

#include "ndq/rhi.h"
#include "ndq/window.h"

using namespace ndq;

struct App : public IApplication
{
    App()
    {
        Title = L"window";
    }

    void Initialize()
    {
        pGraphicsDevice = GetGraphicsDevice();
        pCmdList = pGraphicsDevice->GetCommandList(NDQ_COMMAND_LIST_TYPE::GRAPHICS);
    }

    void Update(float t)
    {
        auto CurrentIndex = pGraphicsDevice->GetCurrentFrameIndex();
        auto CurrentRTV = pGraphicsDevice->GetInternalRenderTargetView(CurrentIndex);
        auto CurrentTexture = pGraphicsDevice->GetInternalSwapchainTexture2D(CurrentIndex);
        IRenderTargetView* CurrentRTVArray[] = { CurrentRTV.get() };

        pCmdList->Open();
        pCmdList->OMSetRenderTargets(1, CurrentRTVArray, nullptr);
        pCmdList->ResourceBarrier(CurrentTexture.get(), NDQ_RESOURCE_STATE::PRESENT, NDQ_RESOURCE_STATE::RENDER_TARGET);
        float Color[4] = { 1.0f, 0.3f, 0.6f, 1.0f };
        pCmdList->ClearRenderTargetView(CurrentRTV.get(), Color);
        pCmdList->ResourceBarrier(CurrentTexture.get(), NDQ_RESOURCE_STATE::RENDER_TARGET, NDQ_RESOURCE_STATE::PRESENT);
        pCmdList->Close();
        pGraphicsDevice->ExecuteCommandList(pCmdList.get());
    }

    std::shared_ptr<IGraphicsDevice> pGraphicsDevice;
    std::shared_ptr<ICommandList> pCmdList;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    App MyApp;
    return MyApp.Run();
}
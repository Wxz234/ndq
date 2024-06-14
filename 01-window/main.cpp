#include <Windows.h>

#include "ndq/rhi.h"
#include "ndq/window.h"

struct App : public ndq::IApplication
{
    App()
    {
        Title = L"window";
    }

    void Initialize()
    {
        pGraphicsDevice = ndq::GetGraphicsDevice();
        pCmdList = pGraphicsDevice->GetCommandList(ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS);
    }

    void Update(float t)
    {
        auto CurrentIndex = pGraphicsDevice->GetCurrentFrameIndex();
        auto CurrentRTV = pGraphicsDevice->GetInternalRenderTargetView(CurrentIndex);
        auto rtvhandle = CurrentRTV->GetHandle();

        auto CurrentTexture = pGraphicsDevice->GetInternalSwapchainTexture2D(CurrentIndex);

        pCmdList->Open();
        //pCmdList->ResourceBarrier(CurrentTexture.get(), ndq::NDQ_RESOURCE_STATE::COMMON, ndq::NDQ_RESOURCE_STATE::RENDER_TARGET);
        pCmdList->SetRenderTargets(1, &rtvhandle, nullptr);
        //pCmdList->ResourceBarrier(CurrentTexture.get(), ndq::NDQ_RESOURCE_STATE::RENDER_TARGET, ndq::NDQ_RESOURCE_STATE::COMMON);
        pCmdList->Close();
        pGraphicsDevice->ExecuteCommandList(pCmdList.get());
    }

    std::shared_ptr<ndq::IGraphicsDevice> pGraphicsDevice;
    std::shared_ptr<ndq::ICommandList> pCmdList;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    App MyApp;
    return MyApp.Run();
}
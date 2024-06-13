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
        auto CurrentRTV = pGraphicsDevice->GetInternalRenderTargetView(pGraphicsDevice->GetCurrentFrameIndex());
        ndq::size_type rtvhandle = CurrentRTV->GetHandle();

        pCmdList->Open();
        //pCmdList->SetRenderTargets(1, &rtvhandle, nullptr);
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
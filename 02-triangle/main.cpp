#include <Windows.h>

#include "ndq/rhi.h"
#include "ndq/window.h"

struct App : public ndq::IApplication
{
    App()
    {
        Title = L"triangle";
    }

    void Initialize()
    {
        auto VertexShaderBlob = ndq::CompileShaderFromFile(L"vertex.hlsl", nullptr, 0, L"main", ndq::NDQ_SHADER_TYPE::VERTEX);
        auto PixelShaderBlob = ndq::CompileShaderFromFile(L"pixel.hlsl", nullptr, 0, L"main", ndq::NDQ_SHADER_TYPE::PIXEL);
        pGraphicsDevice = ndq::GetGraphicsDevice();
        pCmdList = pGraphicsDevice->GetCommandList(ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS);
        pCmdList->SetVertexShader(VertexShaderBlob.get());
        pCmdList->SetPixelShader(PixelShaderBlob.get());
    }

    void Finalize() {}
    void Update(float t)
    {
        pCmdList->Open();
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
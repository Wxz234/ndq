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
        pVertexShaderBlob = ndq::CompileShaderFromFile(L"vertex.hlsl", ndq::NDQ_SHADER_TYPE::VERTEX, L"main", nullptr, 0);
        pPixelShaderBlob = ndq::CompileShaderFromFile(L"pixel.hlsl", ndq::NDQ_SHADER_TYPE::PIXEL, L"main", nullptr, 0);
        pGraphicsDevice = ndq::GetGraphicsDevice();
        pCmdList = pGraphicsDevice->GetCommandList(ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS);
    }

    void Finalize() {}
    void Update(float t)
    {
        pCmdList->Open();
        pCmdList->SetVertexShader(pVertexShaderBlob.get());
        pCmdList->SetPixelShader(pPixelShaderBlob.get());
        pCmdList->SetPrimitiveTopology(ndq::NDQ_PRIMITIVE_TOPOLOGY::TRIANGLELIST);
        pCmdList->DrawInstanced(3, 1, 0, 0);
        pCmdList->Close();
        pGraphicsDevice->ExecuteCommandList(pCmdList.get());
    }

    std::shared_ptr<ndq::IGraphicsDevice> pGraphicsDevice;
    std::shared_ptr<ndq::ICommandList> pCmdList;
    std::shared_ptr<ndq::IShader> pVertexShaderBlob;
    std::shared_ptr<ndq::IShader> pPixelShaderBlob;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    App MyApp;
    return MyApp.Run();
}
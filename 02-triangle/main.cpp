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
        auto VertexShaderBlob = ndq::CompileShaderFromFile(L"vertex.hlsl", nullptr, 0, L"main", ndq::SHADER_TYPE::VERTEX);
        auto PixelShaderBlob = ndq::CompileShaderFromFile(L"pixel.hlsl", nullptr, 0, L"main", ndq::SHADER_TYPE::PIXEL);
    }

    void Finalize() {}
    void Update(float t) {}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    App MyApp;
    return MyApp.Run();
}
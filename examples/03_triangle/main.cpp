#include <Windows.h>

import ndq;

struct App : public ndq::Application
{
    App(ndq::ApplicationDesc* pDesc) : ndq::Application(pDesc) {}

    void Initialize()
    {
        const wchar_t* path = L"C:\\Users\\42937\\source\\repos\\ConsoleApplication2\\ConsoleApplication2\\VertexShader.hlsl";
        const wchar_t* _1 = L"-E";
        const wchar_t* _2 = L"main";
        const wchar_t* _3 = L"-T";
        const wchar_t* _4 = L"vs_6_6";
        const wchar_t* Args[] = { path, _1, _2, _3, _4 };
        ndq::CompileShader(Args, 5);
    }
    void Finalize() {}

    void Update(float t)
    {
        ndq::Gui::NewFrame();
        ndq::Gui::SetNextWindowSize(200, 400, ndq::GuiCond::FirstUseEver);
        ndq::Gui::Begin("Triangle");
        ndq::Gui::End();
        ndq::Gui::Render();

        auto CommandList = ndq::CommandListPool::GetPool()->GetCommandList(ndq::CommandListType::Graphics);
        CommandList->Open();
        ndq::GraphicsDevice::GetDevice()->SetCurrentRenderTargetState(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const float Colors[4] = { 0.2f, 0.8f, 1.0f, 1.0f };
        ndq::GraphicsDevice::GetDevice()->ClearCurrentRTV(CommandList, Colors);
        ndq::GraphicsDevice::GetDevice()->BindCurrentRTV(CommandList);
        ndq::Gui::Submit(CommandList);
        ndq::GraphicsDevice::GetDevice()->SetCurrentRenderTargetState(CommandList, D3D12_RESOURCE_STATE_PRESENT);
        CommandList->Close();

        ndq::GraphicsDevice::GetDevice()->ExecuteCommandList(CommandList);
        ndq::GraphicsDevice::GetDevice()->Present();

        ndq::CommandListPool::GetPool()->CollectCommandList();
    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    ndq::ApplicationDesc AppDesc;
    AppDesc.Instance = hInstance;
    AppDesc.Title = L"Triangle";
    AppDesc.Width = 800;
    AppDesc.Height = 600;
    App MyApp(&AppDesc);
    return MyApp.Run();
}
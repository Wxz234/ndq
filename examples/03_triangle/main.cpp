#include <Windows.h>

import ndq;

struct App : public ndq::Application
{
    App(ndq::ApplicationDesc* pDesc) : ndq::Application(pDesc) {}

    void Initialize() {}
    void Finalize() {}

    void Update(float t)
    {
        ndq::Gui::NewFrame();
        ndq::Gui::SetNextWindowSize(200, 400, ndq::GuiCond::FirstUseEver);
        ndq::Gui::Begin("Triangle");
        ndq::Gui::End();
        ndq::Gui::Render();

        auto CommandList = ndq::GetCommandListPool()->GetCommandList(ndq::CommandListType::Graphics);
        CommandList->Open();
        ndq::GetGraphicsDevice()->SetCurrentRenderTargetState(CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        const float Colors[4] = { 0.2f, 0.8f, 1.0f, 1.0f };
        ndq::GetGraphicsDevice()->ClearCurrentRTV(CommandList, Colors);
        ndq::GetGraphicsDevice()->BindCurrentRTV(CommandList);
        ndq::Gui::Submit(CommandList);
        ndq::GetGraphicsDevice()->SetCurrentRenderTargetState(CommandList, D3D12_RESOURCE_STATE_PRESENT);
        CommandList->Close();

        ndq::GetGraphicsDevice()->ExecuteCommandList(CommandList);
        ndq::GetGraphicsDevice()->Present();

        ndq::CompileShaderFromPath("");
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
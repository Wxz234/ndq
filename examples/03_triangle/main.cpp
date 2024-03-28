#include <Windows.h>

import ndq;

using namespace ndq;

struct App : public IApplication
{
    App(ApplicationDesc* pDesc) : IApplication(pDesc) {}

    void Initialize()
    {
        pScene = CreateScene(SCENE_TYPE::DEFAULT);
        pRenderer = CreateRenderer(RENDERER_TYPE::DEFAULT);
        LoadTextureFromFile(L"");
    }

    void Finalize()
    {
        RemoveScene(pScene);
        RemoveRenderer(pRenderer);
    }

    void Update(float t)
    {
        pRenderer->BeginGuiFrame();
        pRenderer->SetGuiNextWindowPos(10.0f, 10.0f);
        pRenderer->SetGuiNextWindowSize(200.0f, 400.0f);
        pRenderer->BeginGuiWindow("Triangle");
        pRenderer->EndGuiWindow();
        pRenderer->EndGuiFrame();
        pRenderer->Draw(pScene->GetRenderData());
    }

    IScene* pScene = nullptr;
    IRenderer* pRenderer = nullptr;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    ApplicationDesc AppDesc;
    AppDesc.SetTitle(L"Triangle")
           .SetWidth(800)
           .SetHeight(600);
    App MyApp(&AppDesc);
    return MyApp.Run();
}
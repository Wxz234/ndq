#include <Windows.h>

import ndq;

using namespace ndq;

struct App : public IApplication
{
    App(ApplicationDesc* pDesc) : IApplication(pDesc) {}

    void Initialize()
    {
        pRenderer = CreateRenderer(RENDERER_TYPE::DEFAULT);
    }

    void Finalize() {}

    void Update(float t)
    {
        pRenderer->Draw();
    }

    shared_ptr<IRenderer> pRenderer;
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    ApplicationDesc AppDesc;
    AppDesc.SetTitle(L"Window")
           .SetWidth(800)
           .SetHeight(600);
    App MyApp(&AppDesc);
    return MyApp.Run();
}
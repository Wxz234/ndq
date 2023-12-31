#include <Windows.h>

import ndq;

struct App : public ndq::Application
{
    App(ndq::ApplicationDesc* pDesc) : ndq::Application(pDesc) {}

    void Initialize() {}
    void Finalize() {}

    void Update(float t)
    {
        ndq::GetGraphicsDevice()->Present();
    }
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    ndq::ApplicationDesc AppDesc;
    AppDesc.Instance = hInstance;
    AppDesc.Title = L"Window";
    AppDesc.Width = 800;
    AppDesc.Height = 600;
    App MyApp(&AppDesc);
    return MyApp.Run();
}
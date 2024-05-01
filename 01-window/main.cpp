#include <Windows.h>

#include "ndq/window.h"

struct App : public ndq::IApplication
{
    App()
    {
        Title = L"window";
    }

    void Initialize() {}
    void Finalize() {}
    void Update(float t) {}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    App MyApp;
    return MyApp.Run();
}
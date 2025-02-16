#include "ndq/platform/window.h"
#include "ndq/render/renderer.h"
#include "ndq/core/resource.h"

using namespace ndq;

struct Window : IWindow
{
    Window()
    {
        Title = L"Model";
    }

    void Initialize()
    {
        //CreateRenderer(&pRenderer);
    }

    void Update(float)
    {
        TRefCountPtr<IRefCounted> SS;
    }

    void Finalize()
    {
        //pRenderer->Release();
    }

    //IRenderer* pRenderer = nullptr;
};

WIN_MAIN_MACRO(Window)
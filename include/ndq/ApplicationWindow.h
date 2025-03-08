#pragma once

#include "ndq/Type.h"

namespace ndq
{
    class ApplicationWindow
    {
    public:
        virtual ~ApplicationWindow() = default;

        int run();
        void createRenderWindow(void* instance);
        virtual void initialize() = 0;
        virtual void finalize() = 0;
        virtual void update(float t) = 0;
    protected:
        ApplicationWindow(uint32 width, uint32 height, const WString& title) : mWidth(width), mHeight(height), mTitle(title) {}

        uint32 mWidth;
        uint32 mHeight;
        WString mTitle;
    };
}

#define NDQ_WIN_MAIN_MACRO(WindowClass)        \
    struct HINSTANCE__;                        \
    int __stdcall wWinMain(                    \
        HINSTANCE__* instance,                 \
        HINSTANCE__* prevInstance,             \
        wchar_t* cmdLine,                      \
        int cmdShow                            \
    )                                          \
    {                                          \
        WindowClass myWindow;                  \
        myWindow.createRenderWindow(instance); \
        return myWindow.run();                 \
    }

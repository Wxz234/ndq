#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "ndq/ApplicationWindow.h"

#include "GraphicsDeviceInternal.h"

#include <profileapi.h>

namespace ndq
{
    LRESULT CALLBACK wndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        LRESULT result;
        switch (message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            result = 0;
            break;
        default:
            result = DefWindowProcW(hwnd, message, wparam, lparam);
            break;
        }
        return result;
    }

    float getElapsedTime(LARGE_INTEGER& frequency, LARGE_INTEGER& lastTime)
    {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        float elapsedTime = static_cast<float>(currentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = currentTime;
        return elapsedTime;
    }

    int ApplicationWindow::run()
    {
        initialize();

        LARGE_INTEGER frequency;
        LARGE_INTEGER lastTime;
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastTime);

        MSG msg = {};
        while (WM_QUIT != msg.message)
        {
            if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            else
            {
                auto ElapsedTime = getElapsedTime(frequency, lastTime);
                update(ElapsedTime);

                devicePresent();
            }
        }

        finalize();
        deviceFinalize();

        return static_cast<int>(msg.wParam);
    }

    void ApplicationWindow::createRenderWindow(void* instance)
    {
        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(WNDCLASSEXW);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = wndProc;
        wcex.hInstance = (HINSTANCE)instance;
        wcex.hIcon = LoadIconW(wcex.hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wcex.lpszClassName = L"ndq";
        wcex.hIconSm = LoadIconW(wcex.hInstance, L"IDI_ICON");
        RegisterClassExW(&wcex);

        auto stype = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
        RECT rc = { 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
        AdjustWindowRect(&rc, stype, FALSE);
        auto hwnd = CreateWindowExW(0, L"ndq", mTitle.c_str(), stype, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, wcex.hInstance, nullptr);
        ShowWindow(hwnd, SW_SHOWDEFAULT);

        setDeviceHwndAndSize(&hwnd, mWidth, mHeight);
    }
}
module;

#include "predef.h"

export module ndq:window;

import :platform;
import :gui;
import :rhi;

namespace ndq
{
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        {
            return true;
        }

        LRESULT Result;

        switch (message)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            Result = 0;
            break;
        default:
            Result = DefWindowProcW(hWnd, message, wParam, lParam);
            break;
        }

        return Result;
    }

    float GetElapsedTime(LARGE_INTEGER& frequency, LARGE_INTEGER& lastTime)
    {
        LARGE_INTEGER CurrentTime;
        QueryPerformanceCounter(&CurrentTime);
        float ElapsedTime = static_cast<float>(CurrentTime.QuadPart - lastTime.QuadPart) / frequency.QuadPart;
        lastTime = CurrentTime;
        return ElapsedTime;
    }

    export struct ApplicationDesc
    {
        HINSTANCE Instance;
        const wchar_t* Title;
        uint32 Width;
        uint32 Height;
    };

    export class Application
    {
    public:
        Application(ApplicationDesc* pDesc)
        {
            mWidth = pDesc->Width;
            mHeight = pDesc->Height;
            WNDCLASSEXW Wcex{};
            Wcex.cbSize = sizeof(WNDCLASSEXW);
            Wcex.style = CS_HREDRAW | CS_VREDRAW;
            Wcex.lpfnWndProc = WndProc;
            Wcex.hInstance = pDesc->Instance;
            Wcex.hIcon = LoadIconW(Wcex.hInstance, L"IDI_ICON");
            Wcex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
            Wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            Wcex.lpszClassName = L"ndq";
            Wcex.hIconSm = LoadIconW(Wcex.hInstance, L"IDI_ICON");
            RegisterClassExW(&Wcex);

            auto Stype = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
            RECT RC = { 0, 0, static_cast<LONG>(pDesc->Width), static_cast<LONG>(pDesc->Height) };
            AdjustWindowRect(&RC, Stype, FALSE);
            mHwnd = CreateWindowExW(0, L"ndq", pDesc->Title, Stype, CW_USEDEFAULT, CW_USEDEFAULT, RC.right - RC.left, RC.bottom - RC.top, nullptr, nullptr, Wcex.hInstance, nullptr);
            ShowWindow(mHwnd, SW_SHOWDEFAULT);
            UpdateWindow(mHwnd);
        }

        virtual ~Application() {}

        int Run()
        {
            PreInitialize();
            Initialize();

            LARGE_INTEGER Frequency;
            LARGE_INTEGER LastTime;
            QueryPerformanceFrequency(&Frequency);
            QueryPerformanceCounter(&LastTime);

            MSG Msg = {};
            while (WM_QUIT != Msg.message)
            {
                if (PeekMessageW(&Msg, nullptr, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&Msg);
                    DispatchMessageW(&Msg);
                }
                else
                {
                    auto ElapsedTime = GetElapsedTime(Frequency, LastTime);
                    Update(ElapsedTime);
                    PostUpdate();
                }
            }

            Finalize();
            PostFinalize();
            return static_cast<int>(Msg.wParam);
        }

        virtual void Initialize() = 0;
        virtual void Finalize() = 0;
        virtual void Update(float t) = 0;

    private:
        HWND mHwnd;
        uint32 mWidth;
        uint32 mHeight;

        void PreInitialize()
        {
            InitializeRHI(mHwnd, mWidth, mHeight);
            InitializeGui(mHwnd);
        }

        void PostFinalize()
        {
            FinalizeGui();
            FinalizeRHI();

            RemoveAllDll();
        }

        void PostUpdate()
        {
            CommandListPool::GetPool()->CollectCommandList();
        }
    };
}

module;

#include "predef.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "window.h"

export module ndq:window;

import :platform;
import :gui;
import :rhi;

namespace Internal
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
}

export namespace ndq
{
    struct ApplicationDesc
    {
    public:
        ApplicationDesc& SetTitle(const wchar_t* title)
        {
            mTitle = title;
            return *this;
        }

        ApplicationDesc& SetWidth(uint32 w)
        {
            mWidth = w;
            return *this;
        }

        ApplicationDesc& SetHeight(uint32 h)
        {
            mHeight = h;
            return *this;
        }

        const wchar_t* GetTitle() const
        {
            return mTitle.c_str();
        }

        uint32 GetWidth() const
        {
            return mWidth;
        }

        uint32 GetHeight() const
        {
            return mHeight;
        }

    private:
        std::wstring mTitle = L"ndq";
        uint32 mWidth = 800;
        uint32 mHeight = 600;
    };

    class IApplication
    {
    public:
        IApplication(ApplicationDesc* pDesc)
        {
            winrt::hresult const result = WINRT_IMPL_CoInitializeEx(nullptr, static_cast<uint32_t>(winrt::apartment_type::multi_threaded));

            if (result < 0)
            {
                std::terminate();
            }

            mWidth = pDesc->GetWidth();
            mHeight = pDesc->GetHeight();
            WNDCLASSEXW Wcex{};
            Wcex.cbSize = sizeof(WNDCLASSEXW);
            Wcex.style = CS_HREDRAW | CS_VREDRAW;
            Wcex.lpfnWndProc = Internal::WndProc;
            Wcex.hInstance = GetModuleHandleW(nullptr);
            Wcex.hIcon = LoadIconW(Wcex.hInstance, L"IDI_ICON");
            Wcex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
            Wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
            Wcex.lpszClassName = L"ndq";
            Wcex.hIconSm = LoadIconW(Wcex.hInstance, L"IDI_ICON");
            RegisterClassExW(&Wcex);

            auto Stype = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;
            RECT RC = { 0, 0, static_cast<LONG>(mWidth), static_cast<LONG>(mHeight) };
            AdjustWindowRect(&RC, Stype, FALSE);
            mHwnd = CreateWindowExW(0, L"ndq", pDesc->GetTitle(), Stype, CW_USEDEFAULT, CW_USEDEFAULT, RC.right - RC.left, RC.bottom - RC.top, nullptr, nullptr, Wcex.hInstance, nullptr);
            ShowWindow(mHwnd, SW_SHOWDEFAULT);
            UpdateWindow(mHwnd);
        }

        virtual ~IApplication() {}

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
                    auto ElapsedTime = Internal::GetElapsedTime(Frequency, LastTime);
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
            Internal::CreateAllDll();
            Internal::InitializeRHI(mHwnd, mWidth, mHeight);
            Internal::InitializeGui(mHwnd);
        }

        void PostFinalize()
        {
            auto TempDevice = ndq::GetGraphicsDevice();
            TempDevice->Wait(ndq::COMMAND_LIST_TYPE::GRAPHICS);
            TempDevice->Wait(ndq::COMMAND_LIST_TYPE::COPY);
            TempDevice->Wait(ndq::COMMAND_LIST_TYPE::COMPUTE);

            Internal::FinalizeGui();
            Internal::FinalizeRHI();
            Internal::RemoveAllDll();
        }

        void PostUpdate()
        {
            auto GraphicsDevice = GetGraphicsDevice();
            if (GraphicsDevice->NeedGarbageCollection())
            {
                GraphicsDevice->CollectCommandList();
            }
        }
    };
}

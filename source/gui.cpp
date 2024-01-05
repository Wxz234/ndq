module;

#include "predef.h"

export module ndq:gui;

import :rhi;

namespace ndq
{
    export enum class GuiCond
    {
        None = 0,             // No condition (always set the variable), same as _Always
        Always = 1 << 0,      // No condition (always set the variable), same as _None
        Once = 1 << 1,        // Set the variable once per runtime session (only the first call will succeed)
        FirstUseEver = 1 << 2,// Set the variable if the object/window has no persistently saved data (no entry in .ini file)
        Appearing = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
    };

    export class Gui
    {
    public:

        static void NewFrame()
        {
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
        }

        static void SetNextWindowSize(float x, float y, GuiCond cond = GuiCond::None)
        {
            ImGui::SetNextWindowSize(ImVec2(x, y), static_cast<ImGuiCond>(cond));
        }

        static void ShowDemoWindow()
        {
            ImGui::ShowDemoWindow();
        }

        static void Begin(const char* name)
        {
            ImGui::Begin(name);
        }

        static void End()
        {
            ImGui::End();
        }

        static void Render()
        {
            ImGui::Render();
        }

        static void Submit(CommandList* pList)
        {
            ID3D12DescriptorHeap* Lists[1] = { GetContext()->ImguiHeap };
            pList->GetRawList()->SetDescriptorHeaps(1, Lists);
            ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pList->GetRawList());
        }
    private:

        struct GuiContext
        {
            ID3D12DescriptorHeap* ImguiHeap = nullptr;
        };

        static GuiContext* GetContext()
        {
            static GuiContext Context;
            return &Context;
        }

        static void Initialize(HWND hwnd)
        {
            D3D12_DESCRIPTOR_HEAP_DESC HeapDesc{};
            HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            HeapDesc.NumDescriptors = 1;
            HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            auto Context = GetContext();

            GraphicsDevice::GetDevice()->GetRawDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Context->ImguiHeap));

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            // Disable ini
            io.IniFilename = nullptr;
            io.LogFilename = nullptr;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            ImGui::StyleColorsDark();
            ImGui_ImplWin32_Init(hwnd);

            ImGui_ImplDX12_Init(GraphicsDevice::GetDevice()->GetRawDevice(), GraphicsDevice::GetDevice()->GetSwapChainBufferCount(),
                GraphicsDevice::GetDevice()->GetSwapChainFormat(),
                Context->ImguiHeap,
                Context->ImguiHeap->GetCPUDescriptorHandleForHeapStart(),
                Context->ImguiHeap->GetGPUDescriptorHandleForHeapStart());
        }

        static void Finalize()
        {
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            auto Context = GetContext();
            Context->ImguiHeap->Release();
        }

        friend void InitializeGui(HWND hwnd);
        friend void FinalizeGui();
    };

    void InitializeGui(HWND hwnd)
    {
        Gui::Initialize(hwnd);
    }

    void FinalizeGui()
    {
        Gui::Finalize();
    }
}
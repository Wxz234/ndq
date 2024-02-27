module;

#include "predef.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

export module ndq:gui;

import :rhi;
import :smart_ptr;

namespace ndq
{
    class IGui
    {
    public:
        virtual void Submit(ICommandList* pList) = 0;
    };

    shared_ptr<IGui> GetGui();
}

namespace Internal
{
    class Gui : public ndq::IGui
    {
    public:
        ~Gui()
        {
            Release();
        }

        void Release()
        {
            if (!bIsReleased)
            {
                ImGui_ImplDX12_Shutdown();
                ImGui_ImplWin32_Shutdown();
                ImGui::DestroyContext();
                ImguiHeap->Release();
            }
            bIsReleased = true;
        }

        void Submit(ndq::ICommandList* pList)
        {
            if (ImGui::GetDrawData())
            {
                ID3D12DescriptorHeap* Heap[1] = { ImguiHeap };
                auto TempList = reinterpret_cast<ID3D12GraphicsCommandList*>(pList->GetRawList());
                TempList->SetDescriptorHeaps(1, Heap);
                ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), TempList);
            }
        }

        void Initialize(HWND hwnd)
        {
            auto TempPtr = ndq::GetGraphicsDevice();

            D3D12_DESCRIPTOR_HEAP_DESC HeapDesc{};
            HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            HeapDesc.NumDescriptors = 1;
            HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

            auto TempDevice = reinterpret_cast<ID3D12Device4*>(TempPtr->GetRawDevice());
            TempDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&ImguiHeap));

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
            ImGui_ImplDX12_Init
            (
                TempDevice,
                TempPtr->GetSwapChainBufferCount(),
                Internal::GetRawResourceFormat(TempPtr->GetSwapChainFormat()),
                ImguiHeap,
                ImguiHeap->GetCPUDescriptorHandleForHeapStart(),
                ImguiHeap->GetGPUDescriptorHandleForHeapStart()
            );
        }

        ID3D12DescriptorHeap* ImguiHeap = nullptr;
        bool bIsReleased = false;
    };

    void InitializeGui(HWND hwnd)
    {
        auto TempGui = ndq::GetGui();
        auto TempPtr = dynamic_cast<Gui*>(TempGui.get());
        TempPtr->Initialize(hwnd);
    }

    void FinalizeGui()
    {
        auto TempGui = ndq::GetGui();
        auto TempPtr = dynamic_cast<Gui*>(TempGui.get());
        TempPtr->Release();
    }
}

namespace ndq
{
    shared_ptr<IGui> GetGui()
    {
        static shared_ptr<IGui> Gui(new Internal::Gui);
        return Gui;
    }
}
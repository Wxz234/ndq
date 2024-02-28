module;

#include "predef.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

export module ndq:renderer;

import :gui;
import :rhi;
import :smart_ptr;

export namespace ndq
{
    enum class RENDERER_TYPE
    {
        DEFAULT
    };

    class IRenderer
    {
    public:
        virtual void BeginGuiFrame() = 0;
        virtual void SetGuiNextWindowSize(float x, float y) = 0;
        virtual void SetGuiNextWindowPos(float x, float y) = 0;
        virtual void ShowGuiDemoWindow() = 0;
        virtual void BeginGuiWindow(const char* name) = 0;
        virtual void EndGuiWindow() = 0;
        virtual void EndGuiFrame() = 0;
        virtual void Draw() = 0;
    };

    shared_ptr<IRenderer> CreateRenderer(RENDERER_TYPE type);
}

namespace Internal
{
    class Renderer : public ndq::IRenderer
    {
    public:
        void BeginGuiFrame()
        {
            ImGui_ImplDX12_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
        }

        void SetGuiNextWindowSize(float x, float y)
        {
            ImGui::SetNextWindowSize(ImVec2(x, y), ImGuiCond_FirstUseEver);
        }

        void SetGuiNextWindowPos(float x, float y)
        {
            ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_FirstUseEver);
        }

        void ShowGuiDemoWindow()
        {
            ImGui::ShowDemoWindow();
        }

        void BeginGuiWindow(const char* name)
        {
            ImGui::Begin(name);
        }

        void EndGuiWindow()
        {
            ImGui::End();
        }

        void EndGuiFrame()
        {
            ImGui::Render();
        }

        void Draw()
        {
            auto GraphicsDevice = ndq::GetGraphicsDevice();
            auto CommandList = GraphicsDevice->GetCommandList(ndq::COMMAND_LIST_TYPE::GRAPHICS);
            CommandList->Open();
            GraphicsDevice->SetCurrentRenderTargetState(CommandList.get(), ndq::RESOURCE_STATE::RENDER_TARGET);
            GraphicsDevice->BindCurrentRTV(CommandList.get());
            ndq::GetGui()->Submit(CommandList.get());
            GraphicsDevice->SetCurrentRenderTargetState(CommandList.get(), ndq::RESOURCE_STATE::COMMON);
            CommandList->Close();

            GraphicsDevice->ExecuteCommandList(CommandList.get());
            GraphicsDevice->Present();
        }
    };
}

namespace ndq
{
    shared_ptr<IRenderer> CreateRenderer(RENDERER_TYPE type)
    {
        shared_ptr<IRenderer> temp;
        if (type == RENDERER_TYPE::DEFAULT)
        {
            temp = shared_ptr<IRenderer>(new Internal::Renderer);
        }
        return temp;
    }
}
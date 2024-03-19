module;

#include "predef.h"
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

export module ndq:renderer;

import :gui;
import :rhi;
import :render_data;
import :gltf;

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
        virtual void Draw(const RenderData* data = nullptr) = 0;
    };

    IRenderer* CreateRenderer(RENDERER_TYPE type);
    void RemoveRenderer(IRenderer* pRenderer);
}

namespace Internal
{
    class Renderer : public ndq::IRenderer
    {
    public:
        virtual ~Renderer() {}
    };

    class Renderer_Default : public Renderer
    {
    public:
        Renderer_Default()
        {
            auto pDevice = (ID3D12Device4*)ndq::GetGraphicsDevice()->GetRawDevice();

            D3D12_ROOT_SIGNATURE_DESC1 rootSigDesc{};
            rootSigDesc.NumParameters = 0;
            rootSigDesc.pParameters = nullptr;
            rootSigDesc.NumStaticSamplers = 0;
            rootSigDesc.pStaticSamplers = nullptr;
            rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
                D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

            Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig;

            D3D12_VERSIONED_ROOT_SIGNATURE_DESC desc{};
            desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
            desc.Desc_1_1 = rootSigDesc;
            D3D12SerializeVersionedRootSignature(&desc, &serializedRootSig, nullptr);

            //pDevice->CreateRootSignature(NDQ_NODEMASK, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
        }

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

        void Draw(const ndq::RenderData* data)
        {
            if (data)
            {

            }

            auto GraphicsDevice = ndq::GetGraphicsDevice();
            auto CommandList = GraphicsDevice->GetCommandList(ndq::COMMAND_LIST_TYPE::GRAPHICS);
            CommandList->Open();
            GraphicsDevice->SetCurrentRenderTargetState(CommandList, ndq::RESOURCE_STATE::RENDER_TARGET);
            GraphicsDevice->BindCurrentRTV(CommandList);

            float black[4] = { 0.f,0.f,0.f,1.f };
            GraphicsDevice->ClearCurrentRTV(CommandList, black);

            ndq::GetGui()->Submit(CommandList);
            GraphicsDevice->SetCurrentRenderTargetState(CommandList, ndq::RESOURCE_STATE::COMMON);
            CommandList->Close();

            GraphicsDevice->ExecuteCommandList(CommandList);
            GraphicsDevice->Present();
        }

        Microsoft::WRL::ComPtr<ID3D12PipelineState> mDefaultPipeline;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
    };
}

namespace ndq
{
    IRenderer* CreateRenderer(RENDERER_TYPE type)
    {
        IRenderer* temp = nullptr;
        if (type == RENDERER_TYPE::DEFAULT)
        {
            temp = new Internal::Renderer_Default;
        }
        return temp;
    }

    void RemoveRenderer(IRenderer* pRenderer)
    {
        auto TempRendererPtr = dynamic_cast<Internal::Renderer*>(pRenderer);
        delete TempRendererPtr;
    }
}
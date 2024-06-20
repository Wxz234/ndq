#include "ndq/platform.h"
#include "ndq/rhi.h"

#include <concurrent_vector.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcommon.h>
#include <d3dx12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "ndq_internal.h"

#define NDQ_NODE_MASK 1

#define NDQ_SWAPCHAIN_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define NDQ_SWAPCHAIN_COUNT 3

typedef HRESULT(WINAPI* PfnCreateFactory2)(UINT Flags, REFIID riid, void** ppFactory);

std::string RemoveTrailingNumbers(const std::string& input);
ndq::uint32 ExtractTrailingNumbers(const std::string& input);

namespace ndq
{
    std::shared_ptr<IShader> CompileShaderFromFile(const wchar_t* filePath, const wchar_t* entryPoint, NDQ_SHADER_TYPE shaderType, const NDQ_SHADER_DEFINE* pDefines, uint32 defineCount);
}

namespace Internal
{
    DXGI_FORMAT GetRawResourceFormat(ndq::NDQ_RESOURCE_FORMAT format)
    {
        DXGI_FORMAT RawFormat;
        switch (format)
        {
        case ndq::NDQ_RESOURCE_FORMAT::R8G8B8A8_UNORM:
            RawFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        case ndq::NDQ_RESOURCE_FORMAT::D24_UNORM_S8_UINT:
            RawFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
            break;
        default:
            RawFormat = DXGI_FORMAT_UNKNOWN;
            break;
        }

        return RawFormat;
    }

    ndq::NDQ_RESOURCE_FORMAT GetResourceFormat(DXGI_FORMAT format)
    {
        ndq::NDQ_RESOURCE_FORMAT Format;
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            Format = ndq::NDQ_RESOURCE_FORMAT::R8G8B8A8_UNORM;
            break;
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            Format = ndq::NDQ_RESOURCE_FORMAT::D24_UNORM_S8_UINT;
            break;
        default:
            Format = ndq::NDQ_RESOURCE_FORMAT::UNKNOWN;
            break;
        }
        return Format;
    }

    D3D12_RESOURCE_STATES GetRawResourceState(ndq::NDQ_RESOURCE_STATE state)
    {
        D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATE_COMMON;
        switch (state)
        {
        case ndq::NDQ_RESOURCE_STATE::COMMON:
            State = D3D12_RESOURCE_STATE_COMMON;
            break;
        case ndq::NDQ_RESOURCE_STATE::PRESENT:
            State = D3D12_RESOURCE_STATE_PRESENT;
            break;
        case ndq::NDQ_RESOURCE_STATE::RENDER_TARGET:
            State = D3D12_RESOURCE_STATE_RENDER_TARGET;
            break;
        case ndq::NDQ_RESOURCE_STATE::UNIVERSAL_READ:
            State = D3D12_RESOURCE_STATE_GENERIC_READ;
            break;
        case ndq::NDQ_RESOURCE_STATE::COPY_DEST:
            State = D3D12_RESOURCE_STATE_COPY_DEST;
            break;
        case ndq::NDQ_RESOURCE_STATE::COPY_SOURCE:
            State = D3D12_RESOURCE_STATE_COPY_SOURCE;
            break;
        }
        return State;
    }

    D3D12_HEAP_TYPE GetRawHeapType(ndq::NDQ_RESOURCE_HEAP_TYPE type)
    {
        D3D12_HEAP_TYPE Type = D3D12_HEAP_TYPE_DEFAULT;
        switch (type)
        {
        case ndq::NDQ_RESOURCE_HEAP_TYPE::DEFAULT:
            Type = D3D12_HEAP_TYPE_DEFAULT;
            break;
        case ndq::NDQ_RESOURCE_HEAP_TYPE::UPLOAD:
            Type = D3D12_HEAP_TYPE_UPLOAD;
            break;
        case ndq::NDQ_RESOURCE_HEAP_TYPE::READBACK:
            Type = D3D12_HEAP_TYPE_READBACK;
            break;
        }
        return Type;
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType(ndq::NDQ_PRIMITIVE_TOPOLOGY topology)
    {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE Type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        switch (topology)
        {
        case ndq::NDQ_PRIMITIVE_TOPOLOGY::UNDEFINED:
            Type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
            break;
        case ndq::NDQ_PRIMITIVE_TOPOLOGY::POINTLIST:
            Type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            break;
        case ndq::NDQ_PRIMITIVE_TOPOLOGY::LINELIST:
            Type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            break;
        case ndq::NDQ_PRIMITIVE_TOPOLOGY::LINESTRIP:
            Type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            break;
        case ndq::NDQ_PRIMITIVE_TOPOLOGY::TRIANGLELIST:
            Type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            break;
        }
        return Type;
    }

    class InputLayout : public ndq::IInputLayout
    {
    public:
        InputLayout(const ndq::NDQ_INPUT_ELEMENT_DESC* pInputElementDescs, ndq::uint32 numElements) : mDesc(pInputElementDescs, pInputElementDescs + numElements)
        {
            for (ndq::uint32 i = 0; i < numElements; ++i)
            {
                auto RealName = RemoveTrailingNumbers(mDesc[i].SemanticName);
                auto RealIndex = ExtractTrailingNumbers(mDesc[i].SemanticName);

                D3D12_INPUT_ELEMENT_DESC Desc;
                Desc.SemanticName = RealName.c_str();
                Desc.SemanticIndex = RealIndex;
                Desc.Format = GetRawResourceFormat(mDesc[i].Format);
                Desc.InputSlot = mDesc[i].InputSlot;
                Desc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
                Desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
                Desc.InstanceDataStepRate = 0;

                mRawInputElementDescs.emplace_back(Desc);
            }
        }

        ndq::NDQ_INPUT_ELEMENT_DESC GetDesc(ndq::uint32 index) const { return mDesc[index]; }

        std::vector<ndq::NDQ_INPUT_ELEMENT_DESC> mDesc;
        std::vector<D3D12_INPUT_ELEMENT_DESC> mRawInputElementDescs;
    };

    class RenderTargetView : public ndq::IRenderTargetView
    {
    public:
        RenderTargetView(const ndq::NDQ_RENDER_TARGET_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE handle) : mDesc(*pDesc), mHandle(handle) {}

        ndq::NDQ_RENDER_TARGET_VIEW_DESC GetDesc() const
        {
            return mDesc;
        }

        ndq::size_type GetHandle() const
        {
            return mHandle.ptr;
        }

        ndq::NDQ_RENDER_TARGET_VIEW_DESC mDesc;
        D3D12_CPU_DESCRIPTOR_HANDLE mHandle;
    };

    class DepthStencilView : public ndq::IDepthStencilView
    {
    public:
        DepthStencilView(const ndq::NDQ_DEPTH_STENCIL_VIEW_DESC* pDesc, D3D12_CPU_DESCRIPTOR_HANDLE handle) : mDesc(*pDesc), mHandle(handle) {}

        ndq::NDQ_DEPTH_STENCIL_VIEW_DESC GetDesc() const
        {
            return mDesc;
        }

        ndq::size_type GetHandle() const
        {
            return mHandle.ptr;
        }

        ndq::NDQ_DEPTH_STENCIL_VIEW_DESC mDesc;
        D3D12_CPU_DESCRIPTOR_HANDLE mHandle;
    };

    class Shader : public ndq::IShader
    {
    public:
        Shader(ndq::NDQ_SHADER_TYPE type, Microsoft::WRL::ComPtr<IDxcBlob> pBlob, Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflection)
            : mType(type), pBlob(pBlob), pReflection(pReflection) {}

        ndq::NDQ_SHADER_TYPE GetShaderType() const
        {
            return mType;
        }

        void* GetBlobPointer() const
        {
            return pBlob->GetBufferPointer();
        }

        ndq::size_type GetBlobSize() const
        {
            return pBlob->GetBufferSize();
        }
        
        ndq::NDQ_SHADER_TYPE mType;
        Microsoft::WRL::ComPtr<IDxcBlob> pBlob;
        Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflection;
    };

    std::wstring GetShaderTypeString(ndq::NDQ_SHADER_TYPE shaderType)
    {
        std::wstring Temp;
        switch (shaderType)
        {
        case ndq::NDQ_SHADER_TYPE::VERTEX:
            Temp = L"vs_6_6";
            break;
        case ndq::NDQ_SHADER_TYPE::PIXEL:
            Temp = L"ps_6_6";
            break;
        }
        return Temp;
    }

    class GraphicsBuffer : public ndq::IGraphicsBuffer
    {
    public:
        GraphicsBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> pResource,const ndq::NDQ_BUFFER_DESC *pDesc) : pResource(pResource), mDesc(*pDesc) {}


        void Map(void** ppData)
        {
            pResource->Map(0, nullptr, ppData);
        }

        void Unmap()
        {
            pResource->Unmap(0, nullptr);
        }

        ndq::NDQ_BUFFER_DESC GetDesc() const
        {
            return mDesc;
        }

        void* GetRawPtr() const
        {
            return pResource.Get();
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
        ndq::NDQ_BUFFER_DESC mDesc;
    };

    class GraphicsTexture2D : public ndq::IGraphicsTexture2D
    {
    public:
        GraphicsTexture2D(Microsoft::WRL::ComPtr<ID3D12Resource> pResource,const ndq::NDQ_TEXTURE2D_DESC* pDesc) : pResource(pResource), mDesc(*pDesc) {}
        ndq::NDQ_TEXTURE2D_DESC GetDesc() const { return mDesc; }
        void* GetRawPtr() const { return pResource.Get(); }
        Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
        ndq::NDQ_TEXTURE2D_DESC mDesc;
    };

    class CommandList : public ndq::ICommandList
    {
        struct RTCache
        {
            ndq::uint32 NumViews;
            ndq::IRenderTargetView* RenderTargetViews[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
            ndq::IDepthStencilView* DepthStencilView;
            std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RawRTs;
            D3D12_CPU_DESCRIPTOR_HANDLE RawDS;

            RTCache() :NumViews(0), RenderTargetViews{ nullptr }, DepthStencilView(nullptr), RawRTs(), RawDS{} {}

            RTCache(ndq::uint32 numViews, ndq::IRenderTargetView* const* ppRenderTargetViews, ndq::IDepthStencilView* pDepthStencilView)
            {
                NumViews = numViews;
                for (ndq::uint32 i = 0; i < NumViews; ++i)
                {
                    RenderTargetViews[i] = ppRenderTargetViews[i];
                    D3D12_CPU_DESCRIPTOR_HANDLE Handle;
                    Handle.ptr = RenderTargetViews[i]->GetHandle();
                    RawRTs.emplace_back(Handle);
                }
                DepthStencilView = pDepthStencilView;
                if (DepthStencilView)
                {
                    RawDS.ptr = DepthStencilView->GetHandle();
                }
            }

            bool operator==(const RTCache& other) const
            {
                if (NumViews != other.NumViews || DepthStencilView != other.DepthStencilView)
                {
                    return false;
                }

                for (ndq::uint32 i = 0; i < NumViews; ++i)
                {
                    if (RenderTargetViews[i] != other.RenderTargetViews[i])
                    {
                        return false;
                    }
                }

                return true;
            }

            void Update(const RTCache& other)
            {
                NumViews = other.NumViews;
                for (ndq::uint32 i = 0; i < NumViews; ++i)
                {
                    RenderTargetViews[i] = other.RenderTargetViews[i];
                }
                DepthStencilView = other.DepthStencilView;
                RawRTs = other.RawRTs;
                RawDS = other.RawDS;
            }

            const D3D12_CPU_DESCRIPTOR_HANDLE* GetRawRTs() const
            {
                if (NumViews)
                {
                    return RawRTs.data();
                }
                return nullptr;
            }

            const D3D12_CPU_DESCRIPTOR_HANDLE* GetRawDS() const
            {
                if (DepthStencilView)
                {
                    return &RawDS;
                }
                return nullptr;
            }
        };

        struct PIPELINE_DESC
        {
            PIPELINE_DESC()
            {
                mCacheGraphicsPSO = {};
                mCacheGraphicsPSO.pRootSignature = nullptr;
                mCacheGraphicsPSO.VS = {};
                mCacheGraphicsPSO.PS = {};
                mCacheGraphicsPSO.DS = {};
                mCacheGraphicsPSO.HS = {};
                mCacheGraphicsPSO.GS = {};
                mCacheGraphicsPSO.StreamOutput = {};
                mCacheGraphicsPSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
                mCacheGraphicsPSO.SampleMask = 0xffffffff;
                mCacheGraphicsPSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
                mCacheGraphicsPSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
                mCacheGraphicsPSO.InputLayout = { nullptr, 0 };
                mCacheGraphicsPSO.IBStripCutValue = {};
                mCacheGraphicsPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
                mCacheGraphicsPSO.NumRenderTargets = 0;
                for (UINT i = 0; i < 8; ++i)
                {
                    mCacheGraphicsPSO.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
                }
                mCacheGraphicsPSO.DSVFormat = DXGI_FORMAT_UNKNOWN;
                mCacheGraphicsPSO.SampleDesc.Count = 1;
                mCacheGraphicsPSO.SampleDesc.Quality = 0;
                mCacheGraphicsPSO.NodeMask = NDQ_NODE_MASK;
                mCacheGraphicsPSO.CachedPSO = {};
                mCacheGraphicsPSO.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
            }

            D3D12_GRAPHICS_PIPELINE_STATE_DESC mCacheGraphicsPSO;

            Microsoft::WRL::ComPtr<IDxcBlob> pVertexBlob;
            Microsoft::WRL::ComPtr<IDxcBlob> pPixelBlob;

            Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pVertexReflection;
            Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pPixelReflection;
        };

    public:
        CommandList(ndq::NDQ_COMMAND_LIST_TYPE type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pAllocator, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pList)
        {
            bIsBusy.store(true);
            mValue = 0;
            mType = type;
            this->pAllocator = pAllocator;
            this->pList = pList;
            bPSODirty = false;
            pInputLayoutCache = nullptr;
        }

        void Open()
        {
            if (!_CanUse())
            {
                ndq::GetGraphicsDevice()->Wait(mType);
            }

            pAllocator->Reset();
            pList->Reset(pAllocator.Get(), nullptr);
        }

        void ResourceBarrier(ndq::IGraphicsResource* pRes, ndq::NDQ_RESOURCE_STATE brfore, ndq::NDQ_RESOURCE_STATE after)
        {
            auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(reinterpret_cast<ID3D12Resource*> (pRes->GetRawPtr()), GetRawResourceState(brfore), GetRawResourceState(after));
            pList->ResourceBarrier(1, &barrier);
        }

        void ClearRenderTargetView(ndq::IRenderTargetView *pRTV, const float colorRGBA[4])
        {
            D3D12_CPU_DESCRIPTOR_HANDLE Handle;
            Handle.ptr = pRTV->GetHandle();
            pList->ClearRenderTargetView(Handle, colorRGBA, 0, nullptr);
        }

        void OMSetRenderTargets(ndq::uint32 numViews, ndq::IRenderTargetView* const* ppRenderTargetViews, ndq::IDepthStencilView* pDepthStencilView)
        {
            RTCache TempCache(numViews, ppRenderTargetViews, pDepthStencilView);

            if (mRTCahce != TempCache)
            {
                mPipelineDesc.mCacheGraphicsPSO.NumRenderTargets = numViews;
                for (ndq::size_type i = 0; i < numViews; ++i)
                {
                    auto Desc = ppRenderTargetViews[i]->GetDesc();
                    mPipelineDesc.mCacheGraphicsPSO.RTVFormats[i] = GetRawResourceFormat(Desc.Format);
                }
                if (pDepthStencilView)
                {
                    auto Desc = pDepthStencilView->GetDesc();
                    mPipelineDesc.mCacheGraphicsPSO.DSVFormat = GetRawResourceFormat(Desc.Format);
                }
                mRTCahce.Update(TempCache);
                bPSODirty = true;
            }

            auto TempRTHandles = mRTCahce.GetRawRTs();
            auto TempDSHandle = mRTCahce.GetRawDS();

            pList->OMSetRenderTargets(numViews, TempRTHandles, FALSE, TempDSHandle);
        }

        void IASetInputLayout(ndq::IInputLayout* pInputLayout)
        {
            if (pInputLayoutCache != pInputLayout)
            {
                pInputLayoutCache = pInputLayout;
                auto pTemp = dynamic_cast<InputLayout*>(pInputLayoutCache);
                mPipelineDesc.mCacheGraphicsPSO.InputLayout = { pTemp->mRawInputElementDescs.data(), static_cast<UINT>(pTemp->mRawInputElementDescs.size()) };
                bPSODirty = true;
            }
        }

        void IASetPrimitiveTopology(ndq::NDQ_PRIMITIVE_TOPOLOGY topology)
        {
            if (auto Temp = GetPrimitiveTopologyType(topology); Temp != mPipelineDesc.mCacheGraphicsPSO.PrimitiveTopologyType)
            {
                mPipelineDesc.mCacheGraphicsPSO.PrimitiveTopologyType = Temp;
                bPSODirty = true;
            }
            pList->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(topology));
        }

        void VSSetVertexShader(ndq::IShader* pShader)
        {
            if (auto TempShader = dynamic_cast<Shader*>(pShader); mPipelineDesc.pVertexBlob != TempShader->pBlob)
            {
                mPipelineDesc.pVertexBlob = TempShader->pBlob;
                mPipelineDesc.pVertexReflection = TempShader->pReflection;
                mPipelineDesc.mCacheGraphicsPSO.VS = CD3DX12_SHADER_BYTECODE(mPipelineDesc.pVertexBlob->GetBufferPointer(), mPipelineDesc.pVertexBlob->GetBufferSize());
                bPSODirty = true;
            }
        }

        void PSSetPixelShader(ndq::IShader* pShader)
        {
            if (auto TempShader = dynamic_cast<Shader*>(pShader); mPipelineDesc.pPixelBlob != TempShader->pBlob)
            {
                mPipelineDesc.pPixelBlob = TempShader->pBlob;
                mPipelineDesc.pPixelReflection = TempShader->pReflection;
                mPipelineDesc.mCacheGraphicsPSO.PS = CD3DX12_SHADER_BYTECODE(mPipelineDesc.pPixelBlob->GetBufferPointer(), mPipelineDesc.pPixelBlob->GetBufferSize());
                bPSODirty = true;
            }
        }

        void Close()
        {
            pList->Close();
        }

        void DrawInstanced(ndq::uint32 VertexCountPerInstance, ndq::uint32 InstanceCount, ndq::uint32 StartVertexLocation, ndq::uint32 StartInstanceLocation)
        {
            _MakePipeline();
            pList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
        }

        void DrawIndexedInstanced(ndq::uint32 IndexCountPerInstance, ndq::uint32 InstanceCount, ndq::uint32 StartIndexLocation, ndq::int32 BaseVertexLocation, ndq::uint32 StartInstanceLocation)
        {
            _MakePipeline();
            pList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        }

        ndq::NDQ_COMMAND_LIST_TYPE GetType() const
        {
            return mType;
        }

        void* GetRawList() const
        {
            return pList.Get();
        }

        void _MakePipeline()
        {
            _BuildGraphicsRootSignature();
        }

        bool _CanUse()
        {
            auto TempPtr = dynamic_cast<GraphicsDeviceInterface*>(ndq::GetGraphicsDevice().get());
            auto CompletedFenceValue = TempPtr->GetCompletedFenceValue(mType);
            return CompletedFenceValue >= mValue;
        }

        void _BuildGraphicsRootSignature()
        {
            _ParseShaderDesc(mPipelineDesc.pVertexReflection);
            _ParseShaderDesc(mPipelineDesc.pPixelReflection);
        }

        void _ParseShaderDesc(Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflection)
        {
            D3D12_SHADER_DESC ShaderDesc;
            pReflection->GetDesc(&ShaderDesc);
            for (ndq::uint32 i = 0; i < ShaderDesc.BoundResources; ++i)
            {
                D3D12_SHADER_INPUT_BIND_DESC BindDesc;
                pReflection->GetResourceBindingDesc(i, &BindDesc);
            }
        }

        std::atomic_bool bIsBusy;
        ndq::uint64 mValue;
        ndq::NDQ_COMMAND_LIST_TYPE mType;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pList;

        PIPELINE_DESC mPipelineDesc;
        RTCache mRTCahce;
        ndq::IInputLayout* pInputLayoutCache;

        bool bPSODirty;
    };


    class GraphicsDevice : public ndq::IGraphicsDevice, public Internal::GraphicsDeviceInterface
    {
    public:
        void Initialize(HWND hwnd, UINT width, UINT height)
        {
            mHwnd = hwnd;

            Microsoft::WRL::ComPtr<IDXGIFactory7> Factory;
            UINT FactoryFlag = 0;

            mD3D12 = LoadLibraryW(L"d3d12.dll");
#ifdef _DEBUG
            Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
            auto _D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetProcAddress(mD3D12, "D3D12GetDebugInterface");
            if (SUCCEEDED(_D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
            {
                DebugController->EnableDebugLayer();
            }
            FactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
            mDXGI = LoadLibraryW(L"dxgi.dll");
            auto _CreateDXGIFactory2 = (PfnCreateFactory2)GetProcAddress(mDXGI, "CreateDXGIFactory2");
            _CreateDXGIFactory2(FactoryFlag, IID_PPV_ARGS(&Factory));
            Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter;
            Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter));

            auto _D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetProcAddress(mD3D12, "D3D12CreateDevice");
            _D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice));

            D3D12_COMMAND_QUEUE_DESC QueueDesc{};
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            pDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pGraphicsQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            pDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pCopyQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            pDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&pComputeQueue));

            DXGI_SWAP_CHAIN_DESC1 ScDesc{};
            ScDesc.BufferCount = NDQ_SWAPCHAIN_COUNT;
            ScDesc.Width = width;
            ScDesc.Height = height;
            ScDesc.Format = NDQ_SWAPCHAIN_FORMAT;
            ScDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            ScDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            ScDesc.SampleDesc.Count = 1;
            ScDesc.SampleDesc.Quality = 0;
            ScDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            ScDesc.Scaling = DXGI_SCALING_STRETCH;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC FsSwapChainDesc{};
            FsSwapChainDesc.Windowed = TRUE;
            Microsoft::WRL::ComPtr<IDXGISwapChain1> SwapChain;

            Factory->CreateSwapChainForHwnd(pGraphicsQueue.Get(), hwnd, &ScDesc, &FsSwapChainDesc, nullptr, &SwapChain);
            Factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

            SwapChain.As(&pSwapChain);

            D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc{};
            RTVDescriptorHeapDesc.NumDescriptors = NDQ_SWAPCHAIN_COUNT;
            RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            pDevice->CreateDescriptorHeap(&RTVDescriptorHeapDesc, IID_PPV_ARGS(&pRtvDescriptorHeap));

            BuildRT();

            mFrameIndex = pSwapChain->GetCurrentBackBufferIndex();

            pDevice->CreateFence(mFenceValue[mFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
            ++mFenceValue[mFrameIndex];
            mEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

            {
                pDevice->CreateFence(mGraphicsFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pGraphicsFence));
                mGraphicsEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                pDevice->CreateFence(mCopyFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pCopyFence));
                mCopyEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                pDevice->CreateFence(mComputeFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pComputeFence));
                mComputeEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));
            }

            mStates.resize(NDQ_SWAPCHAIN_COUNT, D3D12_RESOURCE_STATE_PRESENT);
        }

        ~GraphicsDevice()
        {
            Release();
        }

        void Release()
        {
            if (!bIsReleased)
            {
                Wait(ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS);
                Wait(ndq::NDQ_COMMAND_LIST_TYPE::COPY);
                Wait(ndq::NDQ_COMMAND_LIST_TYPE::COMPUTE);

                pDevice.Reset();
                pSwapChain.Reset();
                pGraphicsQueue.Reset();
                pCopyQueue.Reset();
                pComputeQueue.Reset();
                pRtvDescriptorHeap.Reset();
                for (ndq::int32 i = 0; i < NDQ_SWAPCHAIN_COUNT; ++i)
                {
                    pRT[i].Reset();
                    pRTObject[i].reset();
                }

                pFence.Reset();
                mEvent.Close();
                pGraphicsFence.Reset();
                mGraphicsEvent.Close();
                pCopyFence.Reset();
                mCopyEvent.Close();
                pComputeFence.Reset();
                mComputeEvent.Close();

                mGraphicsLists.clear();
                mCopyLists.clear();
                mComputeLists.clear();

                mGPURes.clear();

                mInternalRTV.clear();

                FreeLibrary(mD3D12);
                FreeLibrary(mDXGI);
            }
            bIsReleased = true;
        }

        void Present()
        {
            pSwapChain->Present(1, 0);
            QueueSignal();
            MoveToNextFrame();
        }

        ndq::uint64 GetCompletedFenceValue(ndq::NDQ_COMMAND_LIST_TYPE type) const
        {
            ndq::uint64 Val = 0;
            switch (type)
            {
            case ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                Val = pGraphicsFence->GetCompletedValue();
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COPY:
                Val = pCopyFence->GetCompletedValue();
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COMPUTE:
                Val = pComputeFence->GetCompletedValue();
                break;
            }
            return Val;
        }

        void ExecuteCommandList(ndq::ICommandList* pList)
        {
            auto Type = pList->GetType();
            auto TempList = dynamic_cast<CommandList*>(pList);
            ID3D12CommandList* Lists[1] = { reinterpret_cast<ID3D12CommandList*> (TempList->GetRawList()) };
            switch (Type)
            {
            case ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                TempList->mValue = mGraphicsFenceValue.fetch_add(1);
                pGraphicsQueue->ExecuteCommandLists(1, Lists);
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COPY:
                TempList->mValue = mCopyFenceValue.fetch_add(1);
                pCopyQueue->ExecuteCommandLists(1, Lists);
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COMPUTE:
                TempList->mValue = mComputeFenceValue.fetch_add(1);
                pComputeQueue->ExecuteCommandLists(1, Lists);
                break;
            }

            TempList->bIsBusy.store(false);
        }

        void* GetRawDevice() const
        {
            return pDevice.Get();
        }

        void Wait(ndq::NDQ_COMMAND_LIST_TYPE type)
        {
            switch (type)
            {
            case ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                WaitForQueue(pGraphicsQueue.Get());
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COPY:
                WaitForQueue(pCopyQueue.Get());
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COMPUTE:
                WaitForQueue(pComputeQueue.Get());
                break;
            }
        }

        void WaitForQueue(ID3D12CommandQueue* pCommandQueue)
        {
            Microsoft::WRL::ComPtr<ID3D12Fence> pFence;
            HANDLE EventHandle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            ndq::uint64 FenceValue = 0;
            pDevice->CreateFence(FenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
            pCommandQueue->Signal(pFence.Get(), FenceValue);
            pFence->SetEventOnCompletion(FenceValue, EventHandle);
            WaitForSingleObject(EventHandle, INFINITE);
            CloseHandle(EventHandle);
        }

        void MoveToNextFrame()
        {
            const ndq::uint64 CurrentFenceValue = mFenceValue[mFrameIndex];
            pGraphicsQueue->Signal(pFence.Get(), CurrentFenceValue);
            mFrameIndex = pSwapChain->GetCurrentBackBufferIndex();
            if (pFence->GetCompletedValue() < mFenceValue[mFrameIndex])
            {
                pFence->SetEventOnCompletion(mFenceValue[mFrameIndex], mEvent.Get());
                WaitForSingleObjectEx(mEvent.Get(), INFINITE, FALSE);
            }
            mFenceValue[mFrameIndex] = CurrentFenceValue + 1;
        }

        void QueueSignal()
        {
            pGraphicsQueue->Signal(pGraphicsFence.Get(), mGraphicsFenceValue++);
            pCopyQueue->Signal(pCopyFence.Get(), mCopyFenceValue++);
            pComputeQueue->Signal(pComputeFence.Get(), mComputeFenceValue++);
        }

        void BuildRT()
        {
            auto CpuHandle = pRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            auto RTVDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            for (ndq::uint32 n = 0; n < NDQ_SWAPCHAIN_COUNT; ++n)
            {
                D3D12_RENDER_TARGET_VIEW_DESC desc{};
                desc.Format = NDQ_SWAPCHAIN_FORMAT;
                desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
                desc.Texture2D.MipSlice = 0;
                desc.Texture2D.PlaneSlice = 0;
                pSwapChain->GetBuffer(n, IID_PPV_ARGS(pRT[n].ReleaseAndGetAddressOf()));
                pDevice->CreateRenderTargetView(pRT[n].Get(), &desc, CpuHandle);

                ndq::NDQ_RENDER_TARGET_VIEW_DESC ndqDesc{};
                ndqDesc.Format = GetResourceFormat(NDQ_SWAPCHAIN_FORMAT);
                ndqDesc.ViewDimension = ndq::NDQ_RESOURCE_DIMENSION::TEXTURE2D;
                ndqDesc.Texture2D.MipSlice = 0;
                ndqDesc.Texture2D.PlaneSlice = 0;
                std::shared_ptr<RenderTargetView> rtvPrt(new RenderTargetView(&ndqDesc, CpuHandle));
                mInternalRTV.emplace_back(rtvPrt);

                CpuHandle.ptr += RTVDescriptorSize;

                _CreateInternalGraphicsTexture2D(pRT[n], n);
            }
        }

        ID3D12CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
        {
            ID3D12CommandQueue* Queue = nullptr;
            if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
            {
                Queue = pGraphicsQueue.Get();
            }
            else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
            {
                Queue = pCopyQueue.Get();
            }
            else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
            {
                Queue = pComputeQueue.Get();
            }
            return Queue;
        }

        std::shared_ptr<ndq::ICommandList> GetCommandList(ndq::NDQ_COMMAND_LIST_TYPE type) const
        {
            ndq::uint64 CurrentValue;
            ndq::size_type ListCount;
            bool Expected = false;
            switch (type)
            {
            case ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                CurrentValue = pGraphicsFence->GetCompletedValue();
                ListCount = mGraphicsLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mGraphicsLists[i]->bIsBusy.compare_exchange_strong(Expected, true))
                    {
                        if (mGraphicsLists[i]->mValue <= CurrentValue)
                        {
                            return mGraphicsLists[i];
                        }
                        mGraphicsLists[i]->bIsBusy.store(false);
                    }
                }
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COPY:
                CurrentValue = pCopyFence->GetCompletedValue();
                ListCount = mCopyLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mCopyLists[i]->bIsBusy.compare_exchange_strong(Expected, true))
                    {
                        if (mCopyLists[i]->mValue <= CurrentValue)
                        {
                            return mCopyLists[i];
                        }
                        mCopyLists[i]->bIsBusy.store(false);
                    }
                }
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COMPUTE:
                CurrentValue = pComputeFence->GetCompletedValue();
                ListCount = mComputeLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mComputeLists[i]->bIsBusy.compare_exchange_strong(Expected, true))
                    {
                        if (mComputeLists[i]->mValue <= CurrentValue)
                        {
                            return mComputeLists[i];
                        }
                        mComputeLists[i]->bIsBusy.store(false);
                    }
                }
                break;
            }

            auto TempPtr = CreateList(type);
            return TempPtr;
        }

        std::shared_ptr<ndq::ICommandList> CreateList(ndq::NDQ_COMMAND_LIST_TYPE type) const
        {
            ID3D12Device4* TempDevice = reinterpret_cast<ID3D12Device4*>(GetRawDevice());
            std::shared_ptr<CommandList> TempPtr;
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
            switch (type)
            {
            case ndq::NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mGraphicsLists.push_back(TempPtr);
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COPY:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mCopyLists.push_back(TempPtr);
                break;
            case ndq::NDQ_COMMAND_LIST_TYPE::COMPUTE:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mComputeLists.push_back(TempPtr);
                break;
            }
            return TempPtr;
        }

        std::shared_ptr<ndq::IGraphicsBuffer> AllocateUploadBuffer(const ndq::NDQ_BUFFER_DESC* pDesc)
        {
            D3D12_RESOURCE_DESC BufferResDesc{};
            BufferResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            BufferResDesc.Alignment = 0;
            BufferResDesc.Width = pDesc->SizeInBytes;
            BufferResDesc.Height = 1;
            BufferResDesc.DepthOrArraySize = 1;
            BufferResDesc.MipLevels = 1;
            BufferResDesc.Format = DXGI_FORMAT_UNKNOWN;
            BufferResDesc.SampleDesc.Count = 1;
            BufferResDesc.SampleDesc.Quality = 0;
            BufferResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            BufferResDesc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(pDesc->Flags);

            D3D12_HEAP_PROPERTIES Prop{};
            Prop.Type = GetRawHeapType(ndq::NDQ_RESOURCE_HEAP_TYPE::UPLOAD);
            Prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            Prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            Prop.CreationNodeMask = 1;
            Prop.VisibleNodeMask = 1;
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
            pDevice->CreateCommittedResource(&Prop, D3D12_HEAP_FLAG_NONE, &BufferResDesc, GetRawResourceState(ndq::NDQ_RESOURCE_STATE::UNIVERSAL_READ), nullptr, IID_PPV_ARGS(&pResource));

            auto* RawPtr = new GraphicsBuffer(pResource, pDesc);
            std::shared_ptr<ndq::IGraphicsBuffer> retVal(RawPtr);
            std::shared_ptr<ndq::IGraphicsResource> TempPtr = retVal;
            mGPURes.push_back(TempPtr);
            return retVal;
        }

        std::shared_ptr<ndq::IGraphicsBuffer> AllocateDefaultBuffer(const ndq::NDQ_BUFFER_DESC* pDesc)
        {
            D3D12_RESOURCE_DESC BufferResDesc{};
            BufferResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            BufferResDesc.Alignment = 0;
            BufferResDesc.Width = pDesc->SizeInBytes;
            BufferResDesc.Height = 1;
            BufferResDesc.DepthOrArraySize = 1;
            BufferResDesc.MipLevels = 1;
            BufferResDesc.Format = DXGI_FORMAT_UNKNOWN;
            BufferResDesc.SampleDesc.Count = 1;
            BufferResDesc.SampleDesc.Quality = 0;
            BufferResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            BufferResDesc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(pDesc->Flags);

            D3D12_HEAP_PROPERTIES Prop{};
            Prop.Type = GetRawHeapType(ndq::NDQ_RESOURCE_HEAP_TYPE::DEFAULT);
            Prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            Prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            Prop.CreationNodeMask = 1;
            Prop.VisibleNodeMask = 1;
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
            pDevice->CreateCommittedResource(&Prop, D3D12_HEAP_FLAG_NONE, &BufferResDesc, GetRawResourceState(ndq::NDQ_RESOURCE_STATE::COMMON), nullptr, IID_PPV_ARGS(&pResource));

            auto* RawPtr = new GraphicsBuffer(pResource, pDesc);
            std::shared_ptr<ndq::IGraphicsBuffer> retVal(RawPtr);
            std::shared_ptr<ndq::IGraphicsResource> TempPtr = retVal;
            mGPURes.push_back(TempPtr);
            return retVal;
        }

        std::shared_ptr<ndq::IGraphicsBuffer> AllocateReadbackBuffer(const ndq::NDQ_BUFFER_DESC* pDesc)
        {
            D3D12_RESOURCE_DESC BufferResDesc{};
            BufferResDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            BufferResDesc.Alignment = 0;
            BufferResDesc.Width = pDesc->SizeInBytes;
            BufferResDesc.Height = 1;
            BufferResDesc.DepthOrArraySize = 1;
            BufferResDesc.MipLevels = 1;
            BufferResDesc.Format = DXGI_FORMAT_UNKNOWN;
            BufferResDesc.SampleDesc.Count = 1;
            BufferResDesc.SampleDesc.Quality = 0;
            BufferResDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            BufferResDesc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(pDesc->Flags);

            D3D12_HEAP_PROPERTIES Prop{};
            Prop.Type = GetRawHeapType(ndq::NDQ_RESOURCE_HEAP_TYPE::READBACK);
            Prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            Prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            Prop.CreationNodeMask = 1;
            Prop.VisibleNodeMask = 1;
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
            pDevice->CreateCommittedResource(&Prop, D3D12_HEAP_FLAG_NONE, &BufferResDesc, GetRawResourceState(ndq::NDQ_RESOURCE_STATE::COPY_DEST), nullptr, IID_PPV_ARGS(&pResource));

            auto* RawPtr = new GraphicsBuffer(pResource, pDesc);
            std::shared_ptr<ndq::IGraphicsBuffer> retVal(RawPtr);
            std::shared_ptr<ndq::IGraphicsResource> TempPtr = retVal;
            mGPURes.push_back(TempPtr);
            return retVal;
        }

        std::shared_ptr<ndq::IGraphicsTexture2D> AllocateTexture2D(const ndq::NDQ_TEXTURE2D_DESC* pDesc)
        {
            D3D12_RESOURCE_DESC TextureResDesc{};
            TextureResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            TextureResDesc.Alignment = 0;
            TextureResDesc.Width = pDesc->Width;
            TextureResDesc.Height = pDesc->Height;
            TextureResDesc.DepthOrArraySize = 1;
            TextureResDesc.MipLevels = pDesc->MipLevels;
            TextureResDesc.Format = GetRawResourceFormat(pDesc->Format);
            TextureResDesc.SampleDesc.Count = 1;
            TextureResDesc.SampleDesc.Quality = 0;
            TextureResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            TextureResDesc.Flags = static_cast<D3D12_RESOURCE_FLAGS>(pDesc->Flags);

            D3D12_HEAP_PROPERTIES Prop{};
            Prop.Type = D3D12_HEAP_TYPE_DEFAULT;
            Prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            Prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            Prop.CreationNodeMask = 1;
            Prop.VisibleNodeMask = 1;
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
            pDevice->CreateCommittedResource(&Prop, D3D12_HEAP_FLAG_NONE, &TextureResDesc, GetRawResourceState(ndq::NDQ_RESOURCE_STATE::COMMON), nullptr, IID_PPV_ARGS(&pResource));

            auto* RawPtr = new GraphicsTexture2D(pResource, pDesc);
            std::shared_ptr<ndq::IGraphicsTexture2D> retVal(RawPtr);
            std::shared_ptr<ndq::IGraphicsResource> TempPtr = retVal;
            mGPURes.push_back(TempPtr);
            return retVal;
        }

        ndq::uint32 GetCurrentFrameIndex() const
        {
            return mFrameIndex;
        }

        std::shared_ptr<ndq::IRenderTargetView> GetInternalRenderTargetView(ndq::uint32 index) const
        {
            return mInternalRTV[index];
        }

        std::shared_ptr<ndq::IGraphicsTexture2D> GetInternalSwapchainTexture2D(ndq::uint32 index) const
        {
            return pRTObject[index];
        }

        std::shared_ptr<ndq::IInputLayout> CreateInputLayout(const ndq::NDQ_INPUT_ELEMENT_DESC* pInputElementDescs, ndq::uint32 numElements)
        {
            return std::shared_ptr<ndq::IInputLayout>(new InputLayout(pInputElementDescs, numElements));
        }

        void RunGarbageCollection()
        {
            static ndq::uint32 flushCount = 0;
            ++flushCount;

            if(flushCount >= 500)
            {
                ndq::size_type count = mGPURes.size();
                for (ndq::size_type i = 0; i < count; ++i)
                {
                    if (mGPURes[i].use_count() == 1)
                    {
                        mGPURes[i].reset();
                    }
                }
                ++mNeedReleaseGPUResCount;

                flushCount = 0;
            }

            if (mNeedReleaseGPUResCount >= 3000)
            {
                concurrency::concurrent_vector<std::shared_ptr<ndq::IGraphicsResource>> TempGPURes;
                ndq::size_type count = mGPURes.size();
                for (ndq::size_type i = 0; i < count; ++i)
                {
                    if (mGPURes[i] != nullptr)
                    {
                        TempGPURes.push_back(mGPURes[i]);
                    }
                }

                mGPURes.swap(TempGPURes);
                TempGPURes.clear();
                mNeedReleaseGPUResCount = 0;
            }
        }

        std::shared_ptr<ndq::IShader> CreateShaderFromFile(const wchar_t* filePath, const wchar_t* entryPoint, ndq::NDQ_SHADER_TYPE shaderType, const ndq::NDQ_SHADER_DEFINE* pDefines, ndq::uint32 defineCount)
        {
            return ndq::CompileShaderFromFile(filePath, entryPoint, shaderType, pDefines, defineCount);
        }

        void _CreateInternalGraphicsTexture2D(Microsoft::WRL::ComPtr<ID3D12Resource> pRes, ndq::uint32 index)
        {
            auto RawDesc = pRes->GetDesc();
            ndq::NDQ_TEXTURE2D_DESC desc{};
            desc.Width = RawDesc.Width;
            desc.Height = RawDesc.Height;
            desc.MipLevels = RawDesc.MipLevels;
            desc.Format = GetResourceFormat(RawDesc.Format);
            desc.Flags = static_cast<ndq::NDQ_RESOURCE_FLAGS>(RawDesc.Flags);
            pRTObject[index].reset(new GraphicsTexture2D(pRes, &desc));
        }

        HWND mHwnd = NULL;

        Microsoft::WRL::ComPtr<ID3D12Device4> pDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> pGraphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCopyQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> pComputeQueue;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pRtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> pRT[NDQ_SWAPCHAIN_COUNT];
        std::vector<D3D12_RESOURCE_STATES> mStates;

        std::shared_ptr<ndq::IGraphicsTexture2D> pRTObject[NDQ_SWAPCHAIN_COUNT];

        ndq::uint32 mFrameIndex = 0;

        ndq::uint64 mFenceValue[NDQ_SWAPCHAIN_COUNT]{};
        Microsoft::WRL::ComPtr<ID3D12Fence1> pFence;
        Microsoft::WRL::Wrappers::Event mEvent;

        std::atomic_uint64_t mGraphicsFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> pGraphicsFence;
        Microsoft::WRL::Wrappers::Event mGraphicsEvent;
        std::atomic_uint64_t mCopyFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> pCopyFence;
        Microsoft::WRL::Wrappers::Event mCopyEvent;
        std::atomic_uint64_t mComputeFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> pComputeFence;
        Microsoft::WRL::Wrappers::Event mComputeEvent;

        bool bIsReleased = false;

        mutable concurrency::concurrent_vector<std::shared_ptr<CommandList>> mGraphicsLists;
        mutable concurrency::concurrent_vector<std::shared_ptr<CommandList>> mCopyLists;
        mutable concurrency::concurrent_vector<std::shared_ptr<CommandList>> mComputeLists;

        concurrency::concurrent_vector<std::shared_ptr<ndq::IGraphicsResource>> mGPURes;
        std::atomic_size_t mNeedReleaseGPUResCount = 0;

        std::vector<std::shared_ptr<RenderTargetView>> mInternalRTV;

        HMODULE mD3D12{};
        HMODULE mDXGI{};
    };
}

namespace ndq
{
    std::shared_ptr<IGraphicsDevice> GetGraphicsDevice()
    {
        static std::shared_ptr<IGraphicsDevice> Device(new Internal::GraphicsDevice);
        return Device;
    }

    std::shared_ptr<IShader> CompileShaderFromFile(const wchar_t* filePath, const wchar_t* entryPoint, NDQ_SHADER_TYPE shaderType, const NDQ_SHADER_DEFINE* pDefines, uint32 defineCount)
    {
        static HMODULE DXCLIB = LoadLibraryW(L"dxcompiler.dll");
        auto _DxcCreateInstance = (DxcCreateInstanceProc)GetProcAddress(DXCLIB, "DxcCreateInstance");

        Microsoft::WRL::ComPtr<IDxcUtils> pUtils;
        Microsoft::WRL::ComPtr<IDxcCompiler3> pCompiler;
        _DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
        _DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

        Microsoft::WRL::ComPtr<IDxcIncludeHandler> pIncludeHandler;
        pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);

        Microsoft::WRL::ComPtr<IDxcBlobEncoding> pSource = nullptr;
        pUtils->LoadFile(filePath, nullptr, &pSource);

        DxcBuffer Source{};
        Source.Ptr = pSource->GetBufferPointer();
        Source.Size = pSource->GetBufferSize();

        auto ShaderTypeString = Internal::GetShaderTypeString(shaderType);

        std::vector<LPCWSTR> pszArgs;
        pszArgs.push_back(filePath);
        pszArgs.push_back(L"-E");
        pszArgs.push_back(entryPoint);
        pszArgs.push_back(L"-T");
        pszArgs.push_back(ShaderTypeString.c_str());
        if (defineCount != 0)
        {
            pszArgs.push_back(L"-D");
        }
        std::vector<std::wstring> TempDefineStringArgs;
        for (uint32 i = 0; i < defineCount; ++i)
        {
            TempDefineStringArgs.push_back(std::wstring(L""));
            TempDefineStringArgs[i] += pDefines[i].Name;
            TempDefineStringArgs[i] += L"=";
            TempDefineStringArgs[i] += pDefines[i].Value;
            pszArgs.push_back(TempDefineStringArgs[i].c_str());
        }

        Microsoft::WRL::ComPtr<IDxcResult> pResults;
        pCompiler->Compile
        (
            &Source,
            pszArgs.data(),
            static_cast<UINT32>(pszArgs.size()),
            pIncludeHandler.Get(),
            IID_PPV_ARGS(&pResults)
        );

        Microsoft::WRL::ComPtr<IDxcBlob> pShader;
        Microsoft::WRL::ComPtr<IDxcBlob> pReflectionData;
        Microsoft::WRL::ComPtr<IDxcBlobUtf16> pShaderName;
        pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);
        pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
        DxcBuffer ReflectionData{};
        ReflectionData.Ptr = pReflectionData->GetBufferPointer();
        ReflectionData.Size = pReflectionData->GetBufferSize();

        Microsoft::WRL::ComPtr<ID3D12ShaderReflection> pReflection;
        pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection));

        return std::shared_ptr<IShader>(new Internal::Shader(shaderType, pShader, pReflection));
    }
}
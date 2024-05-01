#include "ndq/platform.h"
#include "ndq/rhi.h"

#include <concurrent_vector.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>

#include <atomic>
#include <memory>
#include <vector>

#include "ndq_internal.h"

#define NDQ_NODE_MASK 1

#define NDQ_SWAPCHAIN_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define NDQ_SWAPCHAIN_COUNT 3

typedef HRESULT(WINAPI* PfnCreateFactory2)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

namespace Internal
{
    DXGI_FORMAT GetRawResourceFormat(ndq::RESOURCE_FORMAT format)
    {
        DXGI_FORMAT RawFormat;
        switch (format)
        {
        case ndq::RESOURCE_FORMAT::R8G8B8A8_UNORM:
            RawFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            RawFormat = DXGI_FORMAT_UNKNOWN;
            break;
        }

        return RawFormat;
    }

    ndq::RESOURCE_FORMAT GetResourceFormat(DXGI_FORMAT format)
    {
        ndq::RESOURCE_FORMAT Format;
        switch (format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            Format = ndq::RESOURCE_FORMAT::R8G8B8A8_UNORM;
            break;
        default:
            Format = ndq::RESOURCE_FORMAT::UNKNOWN;
            break;
        }
        return Format;
    }

    D3D12_RESOURCE_STATES GetRawResourceState(ndq::RESOURCE_STATE state)
    {
        D3D12_RESOURCE_STATES State;
        switch (state)
        {
        case ndq::RESOURCE_STATE::COMMON:
            State = D3D12_RESOURCE_STATE_COMMON;
            break;
        case ndq::RESOURCE_STATE::RENDER_TARGET:
            State = D3D12_RESOURCE_STATE_RENDER_TARGET;
            break;
        case ndq::RESOURCE_STATE::READ:
            State = D3D12_RESOURCE_STATE_GENERIC_READ;
            break;
        }
        return State;
    }

    D3D12_HEAP_TYPE GetRawHeapType(ndq::RESOURCE_HEAP_TYPE type)
    {
        D3D12_HEAP_TYPE Type;
        switch (type)
        {
        case ndq::RESOURCE_HEAP_TYPE::DEFAULT:
            Type = D3D12_HEAP_TYPE_DEFAULT;
            break;
        case ndq::RESOURCE_HEAP_TYPE::UPLOAD:
            Type = D3D12_HEAP_TYPE_UPLOAD;
            break;
        case ndq::RESOURCE_HEAP_TYPE::READBACK:
            Type = D3D12_HEAP_TYPE_READBACK;
            break;
        }
        return Type;
    }

    class GraphicsBuffer : public ndq::IGraphicsBuffer
    {
    public:
        GraphicsBuffer(Microsoft::WRL::ComPtr<ID3D12Resource> pResource) : mResource(pResource) {}

        void* GetRawResource() const
        {
            return mResource.Get();
        }

        void Map(void** ppData)
        {
            mResource->Map(0, nullptr, ppData);
        }

        void Unmap()
        {
            mResource->Unmap(0, nullptr);
        }

        Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
    };

    class GraphicsTexture2D : public ndq::IGraphicsTexture2D
    {
    public:
        GraphicsTexture2D(
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource,
            ndq::uint32 width,
            ndq::uint32 height,
            ndq::RESOURCE_FORMAT format
        ) : mResource(pResource), mWidth(width), mHeight(height), mFormat(format) {}

        void* GetRawResource() const { return mResource.Get(); }
        ndq::uint32 GetWidth() const { return mWidth; }
        ndq::uint32 GetHeight() const { return mHeight; }
        ndq::RESOURCE_FORMAT GetFormat() const { return mFormat; }

        Microsoft::WRL::ComPtr<ID3D12Resource> mResource;
        ndq::uint32 mWidth;
        ndq::uint32 mHeight;
        ndq::RESOURCE_FORMAT mFormat;
    };

    class CommandList : public ndq::ICommandList
    {
    public:
        CommandList(ndq::COMMAND_LIST_TYPE type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pAllocator, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pList)
        {
            bIsBusy.store(true);
            mValue = 0;
            mType = type;
            mAllocator = pAllocator;
            mList = pList;
        }

        void Open()
        {
            mAllocator->Reset();
            mList->Reset(mAllocator.Get(), nullptr);
        }

        void SetPrimitiveTopology(ndq::PRIMITIVE_TOPOLOGY topology)
        {
            mList->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(topology));
        }

        void Close()
        {
            mList->Close();
        }

        ndq::COMMAND_LIST_TYPE GetType() const
        {
            return mType;
        }

        void* GetRawList() const
        {
            return mList.Get();
        }

        std::atomic_bool bIsBusy;
        ndq::uint64 mValue;
        ndq::COMMAND_LIST_TYPE mType;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mList;
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
            _D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));

            D3D12_COMMAND_QUEUE_DESC QueueDesc{};
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mGraphicsQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mCopyQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mComputeQueue));

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

            Factory->CreateSwapChainForHwnd(mGraphicsQueue.Get(), hwnd, &ScDesc, &FsSwapChainDesc, nullptr, &SwapChain);
            Factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

            SwapChain.As(&mSwapChain);

            D3D12_DESCRIPTOR_HEAP_DESC RTVDescriptorHeapDesc{};
            RTVDescriptorHeapDesc.NumDescriptors = NDQ_SWAPCHAIN_COUNT;
            RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            mDevice->CreateDescriptorHeap(&RTVDescriptorHeapDesc, IID_PPV_ARGS(&mRtvDescriptorHeap));

            BuildRT();

            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

            mDevice->CreateFence(mFenceValue[mFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
            ++mFenceValue[mFrameIndex];
            mEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

            {
                mDevice->CreateFence(mGraphicsFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mGraphicsFence));
                mGraphicsEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                mDevice->CreateFence(mCopyFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mCopyFence));
                mCopyEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                mDevice->CreateFence(mComputeFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mComputeFence));
                mComputeEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                mRTVHandle = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            }

            mStates.resize(NDQ_SWAPCHAIN_COUNT, D3D12_RESOURCE_STATE_PRESENT);

            CreateDefaultDescriptorHeap();
        }

        ~GraphicsDevice()
        {
            Release();
        }

        void Release()
        {
            if (!bIsReleased)
            {
                Wait(ndq::COMMAND_LIST_TYPE::GRAPHICS);
                Wait(ndq::COMMAND_LIST_TYPE::COPY);
                Wait(ndq::COMMAND_LIST_TYPE::COMPUTE);

                mDevice.Reset();
                mSwapChain.Reset();
                mGraphicsQueue.Reset();
                mCopyQueue.Reset();
                mComputeQueue.Reset();
                mRtvDescriptorHeap.Reset();
                for (ndq::int32 i = 0; i < NDQ_SWAPCHAIN_COUNT; ++i)
                {
                    mRT[i].Reset();
                    mRTObject[i].reset();
                }

                mFence.Reset();
                mEvent.Close();
                mGraphicsFence.Reset();
                mGraphicsEvent.Close();
                mCopyFence.Reset();
                mCopyEvent.Close();
                mComputeFence.Reset();
                mComputeEvent.Close();

                mGraphicsLists.clear();
                mCopyLists.clear();
                mComputeLists.clear();

                mGPURes.clear();
                mDefaultDescriptorHeap.Reset();

                FreeLibrary(mD3D12);
                FreeLibrary(mDXGI);
            }
            bIsReleased = true;
        }

        void Present()
        {
            mSwapChain->Present(1, 0);
            QueueSignal();
            MoveToNextFrame();
        }

        void ExecuteCommandList(ndq::ICommandList* pList)
        {
            auto Type = pList->GetType();
            auto tempList = dynamic_cast<CommandList*>(pList);
            ID3D12CommandList* Lists[1] = { reinterpret_cast<ID3D12CommandList*> (tempList->GetRawList()) };
            CommandList* pRealList = dynamic_cast<CommandList*>(pList);
            switch (Type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                pRealList->mValue = mGraphicsFenceValue.fetch_add(1);
                mGraphicsQueue->ExecuteCommandLists(1, Lists);
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                pRealList->mValue = mCopyFenceValue.fetch_add(1);
                mCopyQueue->ExecuteCommandLists(1, Lists);
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                pRealList->mValue = mComputeFenceValue.fetch_add(1);
                mComputeQueue->ExecuteCommandLists(1, Lists);
                break;
            }

            pRealList->bIsBusy.store(false);
        }

        void* GetRawDevice() const
        {
            return mDevice.Get();
        }

        void Wait(ndq::COMMAND_LIST_TYPE type)
        {
            switch (type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                WaitForQueue(mGraphicsQueue.Get());
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                WaitForQueue(mCopyQueue.Get());
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                WaitForQueue(mComputeQueue.Get());
                break;
            }
        }

        void WaitForQueue(ID3D12CommandQueue* pCommandQueue)
        {
            Microsoft::WRL::ComPtr<ID3D12Fence> pFence;
            HANDLE EventHandle = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            ndq::uint64 FenceValue = 0;
            mDevice->CreateFence(FenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
            pCommandQueue->Signal(pFence.Get(), FenceValue);
            pFence->SetEventOnCompletion(FenceValue, EventHandle);
            WaitForSingleObject(EventHandle, INFINITE);
            CloseHandle(EventHandle);
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const
        {
            auto handle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += (mRTVHandle * mFrameIndex);
            return handle;
        }

        void MoveToNextFrame()
        {
            const ndq::uint64 CurrentFenceValue = mFenceValue[mFrameIndex];
            mGraphicsQueue->Signal(mFence.Get(), CurrentFenceValue);

            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

            if (mFence->GetCompletedValue() < mFenceValue[mFrameIndex])
            {
                mFence->SetEventOnCompletion(mFenceValue[mFrameIndex], mEvent.Get());
                WaitForSingleObjectEx(mEvent.Get(), INFINITE, FALSE);
            }

            mFenceValue[mFrameIndex] = CurrentFenceValue + 1;
        }

        void QueueSignal()
        {
            mGraphicsQueue->Signal(mGraphicsFence.Get(), mGraphicsFenceValue++);
            mCopyQueue->Signal(mCopyFence.Get(), mCopyFenceValue++);
            mComputeQueue->Signal(mComputeFence.Get(), mComputeFenceValue++);
        }

        void BuildRT()
        {
            auto CpuHandle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            auto RTVDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            for (ndq::uint32 n = 0; n < NDQ_SWAPCHAIN_COUNT; ++n)
            {
                mSwapChain->GetBuffer(n, IID_PPV_ARGS(mRT[n].ReleaseAndGetAddressOf()));
                mDevice->CreateRenderTargetView(mRT[n].Get(), nullptr, CpuHandle);
                CpuHandle.ptr += RTVDescriptorSize;

                _CreateInternalGraphicsTexture2D(mRT[n], n);
            }
        }

        ID3D12CommandQueue* GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
        {
            ID3D12CommandQueue* Queue = nullptr;
            if (type == D3D12_COMMAND_LIST_TYPE_DIRECT)
            {
                Queue = mGraphicsQueue.Get();
            }
            else if (type == D3D12_COMMAND_LIST_TYPE_COPY)
            {
                Queue = mCopyQueue.Get();
            }
            else if (type == D3D12_COMMAND_LIST_TYPE_COMPUTE)
            {
                Queue = mComputeQueue.Get();
            }
            return Queue;
        }

        ndq::ICommandList* GetCommandList(ndq::COMMAND_LIST_TYPE type)
        {
            ndq::uint64 CurrentValue;
            ndq::size_type ListCount;
            bool Expected = false;
            switch (type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                CurrentValue = mGraphicsFence->GetCompletedValue();
                ListCount = mGraphicsLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mGraphicsLists[i]->bIsBusy.compare_exchange_strong(Expected, true))
                    {
                        if (mGraphicsLists[i]->mValue <= CurrentValue)
                        {
                            return mGraphicsLists[i].get();
                        }
                        mGraphicsLists[i]->bIsBusy.store(false);
                    }
                }
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                CurrentValue = mCopyFence->GetCompletedValue();
                ListCount = mCopyLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mCopyLists[i]->bIsBusy.compare_exchange_strong(Expected, true))
                    {
                        if (mCopyLists[i]->mValue <= CurrentValue)
                        {
                            return mCopyLists[i].get();
                        }
                        mCopyLists[i]->bIsBusy.store(false);
                    }
                }
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                CurrentValue = mComputeFence->GetCompletedValue();
                ListCount = mComputeLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mComputeLists[i]->bIsBusy.compare_exchange_strong(Expected, true))
                    {
                        if (mComputeLists[i]->mValue <= CurrentValue)
                        {
                            return mComputeLists[i].get();
                        }
                        mComputeLists[i]->bIsBusy.store(false);
                    }
                }
                break;
            }

            auto TempPtr = CreateList(type);
            return TempPtr.get();
        }

        std::shared_ptr<ndq::ICommandList> CreateList(ndq::COMMAND_LIST_TYPE type)
        {
            ID3D12Device4* TempDevice = reinterpret_cast<ID3D12Device4*>(GetRawDevice());
            std::shared_ptr<CommandList> TempPtr;
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
            switch (type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mGraphicsLists.push_back(TempPtr);
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mCopyLists.push_back(TempPtr);
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mComputeLists.push_back(TempPtr);
                break;
            }
            return TempPtr;
        }

        ndq::IGraphicsBuffer* AllocateBuffer(const ndq::GRAPHICS_BUFFER_DESC* pDesc)
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
            Prop.Type = GetRawHeapType(pDesc->Type);
            Prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            Prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            Prop.CreationNodeMask = 1;
            Prop.VisibleNodeMask = 1;
            Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
            mDevice->CreateCommittedResource(&Prop, D3D12_HEAP_FLAG_NONE, &BufferResDesc, GetRawResourceState(pDesc->State), nullptr, IID_PPV_ARGS(&pResource));

            auto* RawPtr = new GraphicsBuffer(pResource);
            std::shared_ptr<ndq::IGraphicsResource> TempPtr(RawPtr);
            mGPURes.push_back(TempPtr);
            return RawPtr;
        }
        ndq::IGraphicsTexture2D* AllocateTexture2D(const ndq::GRAPHICS_TEXTURE_DESC* pDesc)
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
            mDevice->CreateCommittedResource(&Prop, D3D12_HEAP_FLAG_NONE, &TextureResDesc, GetRawResourceState(pDesc->State), nullptr, IID_PPV_ARGS(&pResource));

            auto* RawPtr = new GraphicsTexture2D(pResource, pDesc->Width, pDesc->Height, pDesc->Format);
            std::shared_ptr<ndq::IGraphicsResource> TempPtr(RawPtr);
            mGPURes.push_back(TempPtr);
            return RawPtr;
        }

        void CollectResource(ndq::IGraphicsResource* pResource)
        {
            ndq::size_type count = mGPURes.size();
            for (ndq::size_type i = 0; i < count; ++i)
            {
                if (pResource == mGPURes[i].get())
                {
                    mGPURes[i].reset();
                }
            }
            ++mNeedReleaseGPUResCount;
        }

        void RunGarbageCollection()
        {
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

        void CreateDefaultDescriptorHeap()
        {
            D3D12_DESCRIPTOR_HEAP_DESC Desc{};
            Desc.NodeMask = NDQ_NODE_MASK;
            Desc.NumDescriptors = 8;
            Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            mDevice->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&mDefaultDescriptorHeap));
        }

        void _CreateInternalGraphicsTexture2D(Microsoft::WRL::ComPtr<ID3D12Resource> pRes, ndq::uint32 index)
        {
            auto RawDesc = pRes->GetDesc();
            mRTObject[index].reset(new GraphicsTexture2D(pRes, static_cast<ndq::uint32>(RawDesc.Width), RawDesc.Height, GetResourceFormat(RawDesc.Format)));
        }

        HWND mHwnd = NULL;

        ndq::uint32 mRTVHandle = 0;
        Microsoft::WRL::ComPtr<ID3D12Device4> mDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mGraphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCopyQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mComputeQueue;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>  mRtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> mRT[NDQ_SWAPCHAIN_COUNT];
        std::vector<D3D12_RESOURCE_STATES> mStates;

        std::unique_ptr<GraphicsTexture2D> mRTObject[NDQ_SWAPCHAIN_COUNT];

        ndq::uint32 mFrameIndex = 0;

        ndq::uint64 mFenceValue[NDQ_SWAPCHAIN_COUNT]{};
        Microsoft::WRL::ComPtr<ID3D12Fence1> mFence;
        Microsoft::WRL::Wrappers::Event mEvent;

        std::atomic_uint64_t mGraphicsFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mGraphicsFence;
        Microsoft::WRL::Wrappers::Event mGraphicsEvent;
        std::atomic_uint64_t mCopyFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mCopyFence;
        Microsoft::WRL::Wrappers::Event mCopyEvent;
        std::atomic_uint64_t mComputeFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mComputeFence;
        Microsoft::WRL::Wrappers::Event mComputeEvent;

        bool bIsReleased = false;

        concurrency::concurrent_vector<std::shared_ptr<CommandList>> mGraphicsLists;
        concurrency::concurrent_vector<std::shared_ptr<CommandList>> mCopyLists;
        concurrency::concurrent_vector<std::shared_ptr<CommandList>> mComputeLists;

        concurrency::concurrent_vector<std::shared_ptr<ndq::IGraphicsResource>> mGPURes;
        std::atomic_size_t mNeedReleaseGPUResCount = 0;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> mDefaultDescriptorHeap;

        HMODULE mD3D12{};
        HMODULE mDXGI{};
    };

}

namespace ndq
{
    IGraphicsDevice* GetGraphicsDevice()
    {
        static std::shared_ptr<IGraphicsDevice> Device(new Internal::GraphicsDevice);
        return Device.get();
    }
}
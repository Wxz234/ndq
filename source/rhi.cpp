module;

#include "predef.h"

export module ndq:rhi;

import :platform;

#define SWAP_CHAIN_BUFFER_COUNT 3
#define SWAP_CHAIN_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM

typedef HRESULT(WINAPI* PfnCreateFactory2)(UINT Flags, REFIID riid, _COM_Outptr_ void** ppFactory);

export namespace ndq
{
    enum class RESOURCE_STATE
    {
        UNKNOWN,
        COMMON,
        RENDER_TARGET,
    };

    enum class RESOURCE_FORMAT
    {
        UNKNOWN,
        R8G8B8A8_UNORM,
    };

    enum class COMMAND_LIST_TYPE
    {
        GRAPHICS,
        COPY,
        COMPUTE,
    };

    class ICommandList
    {
    public:
        virtual void Open() = 0;
        virtual void Close() = 0;
        virtual COMMAND_LIST_TYPE GetType() const = 0;
        virtual void* GetRawList() const = 0;
    };

    class IGraphicsDevice
    {
    public:
        virtual void Present() = 0;
        virtual void ExecuteCommandList(ICommandList* pList) = 0;
        virtual void ClearCurrentRTV(ICommandList* pList, const float* colors) = 0;
        virtual void SetCurrentRenderTargetState(ICommandList* pList, RESOURCE_STATE state) = 0;
        virtual void BindCurrentRTV(ICommandList* pList) = 0;
        virtual uint32 GetSwapChainBufferCount() const = 0;
        virtual RESOURCE_FORMAT GetSwapChainFormat() const = 0;
        virtual void Wait(COMMAND_LIST_TYPE type) = 0;
        virtual ICommandList* GetCommandList(COMMAND_LIST_TYPE type) = 0;
        virtual void* GetRawDevice() const = 0;
    };

    IGraphicsDevice* GetGraphicsDevice();
}

namespace Internal
{
    void InitializeRHI(HWND hwnd, UINT width, UINT height);

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

    ndq::RESOURCE_STATE GetResourceState(D3D12_RESOURCE_STATES state)
    {
        ndq::RESOURCE_STATE State;
        switch (state)
        {
        case D3D12_RESOURCE_STATE_COMMON:
            State = ndq::RESOURCE_STATE::COMMON;
            break;
        case D3D12_RESOURCE_STATE_RENDER_TARGET:
            State = ndq::RESOURCE_STATE::RENDER_TARGET;
            break;
        default:
            State = ndq::RESOURCE_STATE::UNKNOWN;
            break;
        }
        return State;
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
        default:
            State = D3D12_RESOURCE_STATE_COMMON;
            break;
        }
        return State;
    }

    class CommandList : public ndq::ICommandList
    {
    public:
        CommandList(ndq::COMMAND_LIST_TYPE type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pAllocator, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pList)
        {
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

        ndq::uint64 mValue;
        ndq::COMMAND_LIST_TYPE mType;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mList;
    };

    class GraphicsDevice : public ndq::IGraphicsDevice
    {
    public:
        GraphicsDevice() {}

        void Initialize(HWND hwnd, UINT width, UINT height)
        {
            mHwnd = hwnd;

            Microsoft::WRL::ComPtr<IDXGIFactory7> Factory;
            UINT FactoryFlag = 0;
#ifdef _DEBUG
            Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;

            auto _D3D12GetDebugInterface = (PFN_D3D12_GET_DEBUG_INTERFACE)GetDllExport(GetDll(DllType::D3D12), "D3D12GetDebugInterface");
            if (SUCCEEDED(_D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
            {
                DebugController->EnableDebugLayer();
            }
            FactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
            auto _CreateDXGIFactory2 = (PfnCreateFactory2)GetDllExport(GetDll(DllType::DXGI), "CreateDXGIFactory2");
            _CreateDXGIFactory2(FactoryFlag, IID_PPV_ARGS(&Factory));
            Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter;
            Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter));

            auto _D3D12CreateDevice = (PFN_D3D12_CREATE_DEVICE)GetDllExport(GetDll(DllType::D3D12), "D3D12CreateDevice");
            _D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));

            D3D12_COMMAND_QUEUE_DESC QueueDesc{};
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mGraphicsQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mCopyQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mComputeQueue));

            DXGI_SWAP_CHAIN_DESC1 ScDesc{};
            ScDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
            ScDesc.Width = width;
            ScDesc.Height = height;
            ScDesc.Format = SWAP_CHAIN_FORMAT;
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
            RTVDescriptorHeapDesc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
            RTVDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            mDevice->CreateDescriptorHeap(&RTVDescriptorHeapDesc, IID_PPV_ARGS(&mRtvDescriptorHeap));

            BuildRT();

            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

            mDevice->CreateFence(mFenceValue[mFrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
            ++mFenceValue[mFrameIndex];
            mEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

            {
                mDevice->CreateFence(mGraphicsFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mGraphicsFence));
                ++mGraphicsFenceValue;
                mGraphicsEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                mDevice->CreateFence(mCopyFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mCopyFence));
                ++mCopyFenceValue;
                mCopyEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                mDevice->CreateFence(mComputeFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mComputeFence));
                ++mComputeFenceValue;
                mComputeEvent.Attach(CreateEventW(nullptr, FALSE, FALSE, nullptr));

                mRTVHandle = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            }

            mStates.resize(SWAP_CHAIN_BUFFER_COUNT, D3D12_RESOURCE_STATE_PRESENT);
        }

        ~GraphicsDevice()
        {
            Release();
        }

        void Release()
        {
            if (!bIsReleased)
            {
                mDevice.Reset();
                mSwapChain.Reset();
                mGraphicsQueue.Reset();
                mCopyQueue.Reset();
                mComputeQueue.Reset();
                mRtvDescriptorHeap.Reset();
                std::for_each_n(mRT, SWAP_CHAIN_BUFFER_COUNT, [](auto& comPtr) { comPtr.Reset(); });

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
            }
            bIsReleased = true;
        }

        void Present()
        {
            mSwapChain->Present(1, 0);
            MoveToNextFrame();
        }

        void ExecuteCommandList(ndq::ICommandList* pList)
        {
            auto Type = pList->GetType();
            ID3D12CommandList* Lists[1] = { reinterpret_cast<ID3D12CommandList*> (pList->GetRawList()) };
            CommandList* pRealList = dynamic_cast<CommandList*>(pList);
            std::lock_guard<std::mutex> lock(mutex_);
            switch (Type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                pRealList->mValue = mGraphicsFenceValue;
                mGraphicsQueue->ExecuteCommandLists(1, Lists);
                mGraphicsQueue->Signal(mGraphicsFence.Get(), mGraphicsFenceValue++);
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                pRealList->mValue = mCopyFenceValue;
                mCopyQueue->ExecuteCommandLists(1, Lists);
                mCopyQueue->Signal(mCopyFence.Get(), mCopyFenceValue++);
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                pRealList->mValue = mComputeFenceValue;
                mComputeQueue->ExecuteCommandLists(1, Lists);
                mComputeQueue->Signal(mComputeFence.Get(), mComputeFenceValue++);
                break;
            }
        }

        void* GetRawDevice() const
        {
            return mDevice.Get();
        }

        void ClearCurrentRTV(ndq::ICommandList * pList, const float* colors)
        {
            auto TempList = reinterpret_cast<ID3D12GraphicsCommandList*>(pList->GetRawList());
            TempList->ClearRenderTargetView(GetCurrentRenderTargetView(), colors, 0, nullptr);
        }

        void SetCurrentRenderTargetState(ndq::ICommandList* pList, ndq::RESOURCE_STATE state)
        {
            D3D12_RESOURCE_BARRIER Barrier{};
            Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            Barrier.Transition.pResource = mRT[mFrameIndex].Get();
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            Barrier.Transition.StateBefore = mStates[mFrameIndex];
            Barrier.Transition.StateAfter = Internal::GetRawResourceState(state);
            auto TempList = reinterpret_cast<ID3D12GraphicsCommandList*>(pList->GetRawList());
            TempList->ResourceBarrier(1, &Barrier);

            mStates[mFrameIndex] = Internal::GetRawResourceState(state);
        }

        void BindCurrentRTV(ndq::ICommandList* pList)
        {
            auto RTVHandle = GetCurrentRenderTargetView();
            auto TempList = reinterpret_cast<ID3D12GraphicsCommandList*>(pList->GetRawList());
            TempList->OMSetRenderTargets(1, &RTVHandle, FALSE, nullptr);
        }

        ndq::uint32 GetSwapChainBufferCount() const
        {
            return SWAP_CHAIN_BUFFER_COUNT;
        }

        ndq::RESOURCE_FORMAT GetSwapChainFormat() const
        {
            return Internal::GetResourceFormat(SWAP_CHAIN_FORMAT);
        }

        void Wait(ndq::COMMAND_LIST_TYPE type)
        {
            switch (type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                mGraphicsQueue->Signal(mGraphicsFence.Get(), mGraphicsFenceValue);
                mGraphicsFence->SetEventOnCompletion(mGraphicsFenceValue, mGraphicsEvent.Get());
                WaitForSingleObjectEx(mGraphicsEvent.Get(), INFINITE, FALSE);
                ++mGraphicsFenceValue;
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                mCopyQueue->Signal(mCopyFence.Get(), mCopyFenceValue);
                mCopyFence->SetEventOnCompletion(mCopyFenceValue, mCopyEvent.Get());
                WaitForSingleObjectEx(mCopyEvent.Get(), INFINITE, FALSE);
                ++mCopyFenceValue;
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                mComputeQueue->Signal(mComputeFence.Get(), mComputeFenceValue);
                mComputeFence->SetEventOnCompletion(mComputeFenceValue, mComputeEvent.Get());
                WaitForSingleObjectEx(mComputeEvent.Get(), INFINITE, FALSE);
                ++mComputeFenceValue;
                break;
            }
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

        void BuildRT()
        {
            auto CpuHandle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            auto RTVDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            for (UINT n = 0; n < SWAP_CHAIN_BUFFER_COUNT; ++n)
            {
                mSwapChain->GetBuffer(n, IID_PPV_ARGS(mRT[n].ReleaseAndGetAddressOf()));
                mDevice->CreateRenderTargetView(mRT[n].Get(), nullptr, CpuHandle);
                CpuHandle.ptr += RTVDescriptorSize;
            }
        }

        void Wait()
        {
            mGraphicsQueue->Signal(mFence.Get(), mFenceValue[mFrameIndex]);
            mFence->SetEventOnCompletion(mFenceValue[mFrameIndex], mEvent.Get());
            WaitForSingleObjectEx(mEvent.Get(), INFINITE, FALSE);
            ++mFenceValue[mFrameIndex];
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
            switch (type)
            {
            case ndq::COMMAND_LIST_TYPE::GRAPHICS:
                CurrentValue = mGraphicsFence->GetCompletedValue();
                ListCount = mGraphicsLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mGraphicsLists[i]->mValue <= CurrentValue)
                    {
                        return mGraphicsLists[i].get();
                    }
                }
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                CurrentValue = mCopyFence->GetCompletedValue();
                ListCount = mCopyLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mCopyLists[i]->mValue <= CurrentValue)
                    {
                        return mCopyLists[i].get();
                    }
                }
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                CurrentValue = mComputeFence->GetCompletedValue();
                ListCount = mComputeLists.size();
                for (ndq::size_type i = 0; i < ListCount; ++i)
                {
                    if (mComputeLists[i]->mValue <= CurrentValue)
                    {
                        return mComputeLists[i].get();
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
                TempDevice->CreateCommandList1(NDQ_NODEMASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mGraphicsLists.push_back(TempPtr);
                break;
            case ndq::COMMAND_LIST_TYPE::COPY:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODEMASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mCopyLists.push_back(TempPtr);
                break;
            case ndq::COMMAND_LIST_TYPE::COMPUTE:
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                TempDevice->CreateCommandList1(NDQ_NODEMASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                TempPtr = std::shared_ptr<CommandList>(new CommandList(type, Allocator, List));
                mComputeLists.push_back(TempPtr);
                break;
            }
            return TempPtr;
        }

        HWND mHwnd = NULL;

        ndq::uint32 mRTVHandle = 0;
        Microsoft::WRL::ComPtr<ID3D12Device4> mDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mGraphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCopyQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mComputeQueue;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>  mRtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> mRT[SWAP_CHAIN_BUFFER_COUNT];
        std::vector<D3D12_RESOURCE_STATES> mStates;
        //Sync object
        ndq::uint32 mFrameIndex = 0;

        ndq::uint64 mFenceValue[SWAP_CHAIN_BUFFER_COUNT]{};
        Microsoft::WRL::ComPtr<ID3D12Fence1> mFence;
        Microsoft::WRL::Wrappers::Event mEvent;

        ndq::uint64 mGraphicsFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mGraphicsFence;
        Microsoft::WRL::Wrappers::Event mGraphicsEvent;
        ndq::uint64 mCopyFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mCopyFence;
        Microsoft::WRL::Wrappers::Event mCopyEvent;
        ndq::uint64 mComputeFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mComputeFence;
        Microsoft::WRL::Wrappers::Event mComputeEvent;

        bool bIsReleased = false;

        concurrency::concurrent_vector<std::shared_ptr<CommandList>> mGraphicsLists;
        concurrency::concurrent_vector<std::shared_ptr<CommandList>> mCopyLists;
        concurrency::concurrent_vector<std::shared_ptr<CommandList>> mComputeLists;

        std::mutex mutex_;
    };

    void InitializeRHI(HWND hwnd, UINT width, UINT height)
    {
        auto TempPtr = dynamic_cast<GraphicsDevice*>(ndq::GetGraphicsDevice());
        TempPtr->Initialize(hwnd, width, height);
    }

    void FinalizeRHI()
    {
        auto TempPtr = dynamic_cast<GraphicsDevice*>(ndq::GetGraphicsDevice());
        TempPtr->Release();
    }
}

namespace ndq
{
    IGraphicsDevice* GetGraphicsDevice()
    {
        static std::shared_ptr<IGraphicsDevice> Device(new Internal::GraphicsDevice);
        return Device.get();
    }
}
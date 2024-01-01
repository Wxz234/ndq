module;

#include "predef.h"

export module ndq:rhi;

import :platform;

#define SWAP_CHAIN_BUFFER_COUNT 3
#define SWAP_CHAIN_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define NODEMASK 1
namespace ndq
{
    export enum class CommandListType
    {
        Graphics,
        Copy,
        Compute,
    };

    export class CommandList
    {
    public:
        void Open()
        {
            mAllocator->Reset();
            mList->Reset(mAllocator.Get(), nullptr);
        }
        void Close()
        {
            mList->Close();
        }

        CommandListType GetType() const
        {
            return mType;
        }

        ID3D12GraphicsCommandList* GetRawList() const
        {
            return mList.Get();
        }
    private:

        CommandList(CommandListType type, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pAllocator, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pList)
        {
            mType = type;
            mAllocator = pAllocator;
            mList = pList;
        }

        CommandListType mType;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mList;

        friend class CommandListPool;
    };

    export class GraphicsDevice
    {
    public:
        void Present()
        {
            mSwapChain->Present(1, 0);
            MoveToNextFrame();
        }

        void ExecuteCommandList(CommandList* pList)
        {
            auto Type = pList->GetType();
            ID3D12CommandList* Lists[1] = { pList->GetRawList() };
            if (Type == CommandListType::Graphics)
            {
                mGraphicsUsedCommandLists.emplace_back(pList);
                mGraphicsUsedCommandListsSignal.emplace_back(mGraphicsFenceValue);
                ++mGraphicsUsedCommandListsCount;
                mGraphicsQueue->ExecuteCommandLists(1, Lists);
                mGraphicsQueue->Signal(mGraphicsFence.Get(), mGraphicsFenceValue++);
            }
            else if (Type == CommandListType::Copy)
            {
                mCopyUsedCommandLists.emplace_back(pList);
                mCopyUsedCommandListsSignal.emplace_back(mCopyFenceValue);
                ++mCopyUsedCommandListsCount;
                mCopyQueue->ExecuteCommandLists(1, Lists);
                mCopyQueue->Signal(mCopyFence.Get(), mCopyFenceValue++);
            }
            else
            {
                mComputeUsedCommandLists.emplace_back(pList);
                mComputeUsedCommandListsSignal.emplace_back(mComputeFenceValue);
                ++mComputeUsedCommandListsCount;
                mComputeQueue->ExecuteCommandLists(1, Lists);
                mComputeQueue->Signal(mComputeFence.Get(), mComputeFenceValue++);
            }
        }

        ID3D12Device4* GetRawDevice() const
        {
            return mDevice.Get();
        }

        void ClearCurrentRTV(CommandList* pList, const float* colors)
        {
            pList->GetRawList()->ClearRenderTargetView(GetCurrentRenderTargetView(), colors, 0, nullptr);
        }

        void SetCurrentRenderTargetState(CommandList* pList, D3D12_RESOURCE_STATES state)
        {
            D3D12_RESOURCE_BARRIER Barrier{};
            Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
            Barrier.Transition.pResource = mRT[mFrameIndex].Get();
            Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            Barrier.Transition.StateBefore = mStates[mFrameIndex];
            Barrier.Transition.StateAfter = state;
            pList->GetRawList()->ResourceBarrier(1, &Barrier);

            mStates[mFrameIndex] = state;
        }

        void BindCurrentRTV(CommandList* pList)
        {
            auto RTVHandle = GetCurrentRenderTargetView();
            pList->GetRawList()->OMSetRenderTargets(1, &RTVHandle, FALSE, nullptr);
        }

        uint32 GetSwapChainBufferCount() const
        {
            return SWAP_CHAIN_BUFFER_COUNT;
        }

        DXGI_FORMAT GetSwapChainFormat() const
        {
            return SWAP_CHAIN_FORMAT;
        }

        void Wait(CommandListType type)
        {
            if (type == CommandListType::Graphics)
            {
                mGraphicsQueue->Signal(mGraphicsFence.Get(), mGraphicsFenceValue);
                mGraphicsFence->SetEventOnCompletion(mGraphicsFenceValue, mGraphicsEvent.Get());
                WaitForSingleObjectEx(mGraphicsEvent.Get(), INFINITE, FALSE);
                ++mGraphicsFenceValue;
            }
            else if (type == CommandListType::Copy)
            {
                mCopyQueue->Signal(mCopyFence.Get(), mCopyFenceValue);
                mCopyFence->SetEventOnCompletion(mCopyFenceValue, mCopyEvent.Get());
                WaitForSingleObjectEx(mCopyEvent.Get(), INFINITE, FALSE);
                ++mCopyFenceValue;
            }
            else
            {
                mComputeQueue->Signal(mComputeFence.Get(), mComputeFenceValue);
                mComputeFence->SetEventOnCompletion(mComputeFenceValue, mComputeEvent.Get());
                WaitForSingleObjectEx(mComputeEvent.Get(), INFINITE, FALSE);
                ++mComputeFenceValue;
            }
        }

    private:
        uint32 mRTVHandle = 0;
        Microsoft::WRL::ComPtr<ID3D12Device4> mDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mGraphicsQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCopyQueue;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> mComputeQueue;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>  mRtvDescriptorHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> mRT[SWAP_CHAIN_BUFFER_COUNT];
        std::vector<D3D12_RESOURCE_STATES> mStates;
        //Sync object
        uint32 mFrameIndex = 0;

        uint64 mFenceValue[SWAP_CHAIN_BUFFER_COUNT]{};
        Microsoft::WRL::ComPtr<ID3D12Fence1> mFence;
        Microsoft::WRL::Wrappers::Event mEvent;

        uint64 mGraphicsFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mGraphicsFence;
        Microsoft::WRL::Wrappers::Event mGraphicsEvent;
        uint64 mCopyFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mCopyFence;
        Microsoft::WRL::Wrappers::Event mCopyEvent;
        uint64 mComputeFenceValue = 0;
        Microsoft::WRL::ComPtr<ID3D12Fence1> mComputeFence;
        Microsoft::WRL::Wrappers::Event mComputeEvent;

        HWND mHwnd = NULL;

        bool bIsInit = false;

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const
        {
            auto handle = mRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += (mRTVHandle * mFrameIndex);
            return handle;
        }

        void MoveToNextFrame()
        {
            const uint64 CurrentFenceValue = mFenceValue[mFrameIndex];
            mGraphicsQueue->Signal(mFence.Get(), CurrentFenceValue);

            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();

            if (mFence->GetCompletedValue() < mFenceValue[mFrameIndex])
            {
                mFence->SetEventOnCompletion(mFenceValue[mFrameIndex], mEvent.Get());
                WaitForSingleObjectEx(mEvent.Get(), INFINITE, FALSE);
            }

            mFenceValue[mFrameIndex] = CurrentFenceValue + 1;
        }

        void CreateInternalCMDQueue()
        {
            D3D12_COMMAND_QUEUE_DESC QueueDesc{};
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mGraphicsQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mCopyQueue));
            QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            mDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mComputeQueue));
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

        void Initialize(HWND hwnd, UINT width, UINT height)
        {
            mHwnd = hwnd;

            if (bIsInit)
            {
                return;
            }

            bIsInit = true;
            Microsoft::WRL::ComPtr<IDXGIFactory7> Factory;
            UINT FactoryFlag = 0;
#ifdef _DEBUG
            Microsoft::WRL::ComPtr<ID3D12Debug> DebugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
            {
                DebugController->EnableDebugLayer();
            }
            FactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
            CreateDXGIFactory2(FactoryFlag, IID_PPV_ARGS(&Factory));
            Microsoft::WRL::ComPtr<IDXGIAdapter4> Adapter;
            Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter));
            D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));
            CreateInternalCMDQueue();

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

        std::vector<CommandList*> mGraphicsUsedCommandLists;
        std::vector<uint64> mGraphicsUsedCommandListsSignal;
        size_t mGraphicsUsedCommandListsCount = 0;
        std::vector<CommandList*> mCopyUsedCommandLists;
        std::vector<uint64> mCopyUsedCommandListsSignal;
        size_t mCopyUsedCommandListsCount = 0;
        std::vector<CommandList*> mComputeUsedCommandLists;
        std::vector<uint64> mComputeUsedCommandListsSignal;
        size_t mComputeUsedCommandListsCount = 0;

        friend void InitializeRHI(HWND hwnd, UINT width, UINT height);
        friend class CommandListPool;
    };

    export GraphicsDevice* GetGraphicsDevice()
    {
        static GraphicsDevice* Device = new GraphicsDevice();
        return Device;
    }

    export class CommandListPool
    {
    public:
        CommandList *GetCommandList(CommandListType type)
        {
            if (type == CommandListType::Graphics)
            {
                size_t Count = mGraphicsListsAndStatus.size();
                for (size_t i = 0; i < Count; ++i)
                {
                    bool Status = true;
                    bool Result = mGraphicsListsAndStatus[i]->mStatus->compare_exchange_strong(Status, false);
                    if (Result)
                    {
                        return mGraphicsListsAndStatus[i]->mCommandList;
                    }
                }
            }
            else if (type == CommandListType::Copy)
            {
                size_t Count = mCopyListsAndStatus.size();
                for (size_t i = 0; i < Count; ++i)
                {
                    bool Status = true;
                    bool Result = mCopyListsAndStatus[i]->mStatus->compare_exchange_strong(Status, false);
                    if (Result)
                    {
                        return mCopyListsAndStatus[i]->mCommandList;
                    }
                }
            }
            else
            {
                size_t Count = mComputeListsAndStatus.size();
                for (size_t i = 0; i < Count; ++i)
                {
                    bool Status = true;
                    bool Result = mComputeListsAndStatus[i]->mStatus->compare_exchange_strong(Status, false);
                    if (Result)
                    {
                        return mComputeListsAndStatus[i]->mCommandList;
                    }
                }
            }

            return _CreateList(type);
        }
    private:
        CommandList* _CreateList(CommandListType type)
        {
            concurrency::concurrent_vector<std::unique_ptr<CommandListAndStatus>>* ListAndStatus;
            D3D12_COMMAND_LIST_TYPE RawType;
            if (type == CommandListType::Graphics)
            {
                ListAndStatus = &mGraphicsListsAndStatus;
                RawType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            }
            else if (type == CommandListType::Copy)
            {
                ListAndStatus = &mCopyListsAndStatus;
                RawType = D3D12_COMMAND_LIST_TYPE_COPY;
            }
            else
            {
                ListAndStatus = &mComputeListsAndStatus;
                RawType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            }
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
            GetGraphicsDevice()->mDevice->CreateCommandAllocator(RawType, IID_PPV_ARGS(&Allocator));
            GetGraphicsDevice()->mDevice->CreateCommandList1(NODEMASK, RawType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));

            std::unique_ptr<CommandListAndStatus> TempPtr(new CommandListAndStatus(type, Allocator, List));
            TempPtr->mStatus->store(false);
            CommandList* TempCmdListPtr = TempPtr->mCommandList;
            ListAndStatus->push_back(std::move(TempPtr));
            return TempCmdListPtr;
        }

        CommandListPool()
        {
            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;

            for (uint32 i = 0; i < 2; ++i)
            {
                GetGraphicsDevice()->GetRawDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                GetGraphicsDevice()->GetRawDevice()->CreateCommandList1(NODEMASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                mGraphicsListsAndStatus.push_back(std::make_unique<CommandListAndStatus>(CommandListType::Graphics, Allocator, List));

                GetGraphicsDevice()->GetRawDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                GetGraphicsDevice()->GetRawDevice()->CreateCommandList1(NODEMASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                mCopyListsAndStatus.push_back(std::make_unique<CommandListAndStatus>(CommandListType::Copy, Allocator, List));

                GetGraphicsDevice()->GetRawDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(Allocator.ReleaseAndGetAddressOf()));
                GetGraphicsDevice()->GetRawDevice()->CreateCommandList1(NODEMASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(List.ReleaseAndGetAddressOf()));
                mComputeListsAndStatus.push_back(std::make_unique<CommandListAndStatus>(CommandListType::Compute, Allocator, List));
            }
        }

        void CollectInternalCommandList()
        {
            std::vector<size_t> GraphicsRemoveIndices;
            std::vector<size_t> CopyRemoveIndices;
            std::vector<size_t> ComputeRemoveIndices;

            for (size_t i = 0; i < GetGraphicsDevice()->mGraphicsUsedCommandListsCount; ++i)
            {
                for (size_t j = 0; j < mGraphicsListsAndStatus.size(); ++j)
                {
                    if(
                        GetGraphicsDevice()->mGraphicsUsedCommandLists[i] == mGraphicsListsAndStatus[j]->mCommandList && 
                        GetGraphicsDevice()->mGraphicsUsedCommandListsSignal[i] < GetGraphicsDevice()->mGraphicsFence->GetCompletedValue()
                    )
                    {
                        mGraphicsListsAndStatus[j]->mStatus->store(true);
                        GraphicsRemoveIndices.emplace_back(i);
                    }
                }
            }

            for (size_t i = 0; i < GetGraphicsDevice()->mCopyUsedCommandListsCount; ++i)
            {
                for (size_t j = 0; j < mCopyListsAndStatus.size(); ++j)
                {
                    if(
                        GetGraphicsDevice()->mCopyUsedCommandLists[i] == mCopyListsAndStatus[j]->mCommandList &&
                        GetGraphicsDevice()->mCopyUsedCommandListsSignal[i] < GetGraphicsDevice()->mCopyFence->GetCompletedValue()
                    )
                    {
                        mCopyListsAndStatus[j]->mStatus->store(true);
                        CopyRemoveIndices.emplace_back(i);
                    }
                }
            }

            for (size_t i = 0; i < GetGraphicsDevice()->mComputeUsedCommandListsCount; ++i)
            {
                for (size_t j = 0; j < mComputeListsAndStatus.size(); ++j)
                {
                    if (
                        GetGraphicsDevice()->mComputeUsedCommandLists[i] == mComputeListsAndStatus[j]->mCommandList &&
                        GetGraphicsDevice()->mComputeUsedCommandListsSignal[i] < GetGraphicsDevice()->mComputeFence->GetCompletedValue()
                    )
                    {
                        mComputeListsAndStatus[j]->mStatus->store(true);
                        ComputeRemoveIndices.emplace_back(i);
                    }
                }
            }

            for (auto it = GraphicsRemoveIndices.rbegin(); it != GraphicsRemoveIndices.rend(); ++it)
            {
                GetGraphicsDevice()->mGraphicsUsedCommandLists.erase(GetGraphicsDevice()->mGraphicsUsedCommandLists.begin() + *it);
                GetGraphicsDevice()->mGraphicsUsedCommandListsSignal.erase(GetGraphicsDevice()->mGraphicsUsedCommandListsSignal.begin() + *it);
            }
            for (auto it = CopyRemoveIndices.rbegin(); it != CopyRemoveIndices.rend(); ++it)
            {
                GetGraphicsDevice()->mCopyUsedCommandLists.erase(GetGraphicsDevice()->mCopyUsedCommandLists.begin() + *it);
                GetGraphicsDevice()->mCopyUsedCommandListsSignal.erase(GetGraphicsDevice()->mCopyUsedCommandListsSignal.begin() + *it);
            }
            for (auto it = ComputeRemoveIndices.rbegin(); it != ComputeRemoveIndices.rend(); ++it)
            {
                GetGraphicsDevice()->mComputeUsedCommandLists.erase(GetGraphicsDevice()->mComputeUsedCommandLists.begin() + *it);
                GetGraphicsDevice()->mComputeUsedCommandListsSignal.erase(GetGraphicsDevice()->mComputeUsedCommandListsSignal.begin() + *it);
            }

            GetGraphicsDevice()->mGraphicsUsedCommandListsCount -= GraphicsRemoveIndices.size();
            GetGraphicsDevice()->mCopyUsedCommandListsCount -= CopyRemoveIndices.size();
            GetGraphicsDevice()->mComputeUsedCommandListsCount -= ComputeRemoveIndices.size();
        }

        struct CommandListAndStatus
        {
            CommandListAndStatus
            (
                CommandListType type, 
                Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pAllocator, 
                Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pList
            ) 
            {
                mCommandList = new CommandList(type, pAllocator, pList);
                mStatus = new std::atomic_bool; 
                mStatus->store(true);
            }

            ~CommandListAndStatus()
            {
                delete mCommandList;
                delete mStatus;
            }

            CommandListAndStatus(const CommandListAndStatus&) = delete;
            CommandListAndStatus(CommandListAndStatus&&) = delete;
            CommandListAndStatus& operator=(const CommandListAndStatus&) = delete;
            CommandListAndStatus& operator=(CommandListAndStatus&&) = delete;

            CommandList* mCommandList;
            std::atomic_bool* mStatus;
        };

        concurrency::concurrent_vector<std::unique_ptr<CommandListAndStatus>> mGraphicsListsAndStatus;
        concurrency::concurrent_vector<std::unique_ptr<CommandListAndStatus>> mCopyListsAndStatus;
        concurrency::concurrent_vector<std::unique_ptr<CommandListAndStatus>> mComputeListsAndStatus;

        friend CommandListPool* GetCommandListPool();
        friend void CollectCommandList();
    };

    export CommandListPool* GetCommandListPool()
    {
        static CommandListPool* Pool = new CommandListPool;
        return Pool;
    }

    void CollectCommandList()
    {
        GetCommandListPool()->CollectInternalCommandList();
    }

    void InitializeRHI(HWND hwnd, UINT width, UINT height)
    {
        GetGraphicsDevice()->Initialize(hwnd, width, height);
        GetCommandListPool();
    }

    void FinalizeRHI()
    {
        delete GetGraphicsDevice();
        delete GetCommandListPool();
    }
}
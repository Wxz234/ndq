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
                mUsedCommandListsSignal.push_back(mGraphicsFenceValue);
                mGraphicsQueue->ExecuteCommandLists(1, Lists);
                mGraphicsQueue->Signal(mGraphicsFence.Get(), mGraphicsFenceValue++);
            }
            else if (Type == CommandListType::Copy)
            {
                mUsedCommandListsSignal.push_back(mCopyFenceValue);
                mCopyQueue->ExecuteCommandLists(1, Lists);
                mCopyQueue->Signal(mCopyFence.Get(), mCopyFenceValue++);
            }
            else
            {
                mUsedCommandListsSignal.push_back(mComputeFenceValue);
                mComputeQueue->ExecuteCommandLists(1, Lists);
                mComputeQueue->Signal(mComputeFence.Get(), mComputeFenceValue++);
            }

            mUsedCommandLists.push_back(pList);
            ++mUsedCommandListsCount;
        }

        ID3D12Device* GetRawDevice() const
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
#ifdef _DEBUG
#endif
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

        std::vector<CommandList*> mUsedCommandLists;
        std::vector<uint64> mUsedCommandListsSignal;
        size_t mUsedCommandListsCount = 0;

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
            const std::lock_guard<std::mutex> Lock(mMutex);

            std::vector<std::shared_ptr<CommandList>>* Lists;
            std::vector<int>* Status;
            uint32* ListsCount;

            if (type == CommandListType::Graphics)
            {
                Lists = &mGraphicsLists;
                Status = &mGraphicsListsStatus;
                ListsCount = &mGraphicsListsCount;
            }
            else if (type == CommandListType::Copy)
            {
                Lists = &mCopyLists;
                Status = &mCopyListsStatus;
                ListsCount = &mCopyListsCount;
            }
            else
            {
                Lists = &mComputeLists;
                Status = &mComputeListsStatus;
                ListsCount = &mComputeListsCount;
            }

            for (uint32 i = 0;i < *ListsCount; ++i)
            {
                if (Status->operator[](i))
                {
                    Status->operator[](i) = 0;
                    return Lists->operator[](i).get();
                }
            }

            CreateList(type);

            Status->operator[](*ListsCount - 1) = 0;
            return Lists->operator[](*ListsCount - 1).get();
        }
    private:
        CommandListPool(GraphicsDevice* pDevice)
        {
            ID3D12Device4* TempDevice;
            pDevice->GetRawDevice()->QueryInterface(&TempDevice);

            for (uint32 i = 0; i < mGraphicsListsCount; ++i)
            {
                Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
                Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator));
                TempDevice->CreateCommandList1(NODEMASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));
                mGraphicsLists.push_back(std::shared_ptr<CommandList>(new CommandList(CommandListType::Graphics, Allocator, List)));
                mGraphicsListsStatus.push_back(1);
            }

            for (uint32 i = 0; i < mCopyListsCount; ++i)
            {
                Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
                Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&Allocator));
                TempDevice->CreateCommandList1(NODEMASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));
                mCopyLists.push_back(std::shared_ptr<CommandList>(new CommandList(CommandListType::Copy, Allocator, List)));
                mCopyListsStatus.push_back(1);
            }

            for (uint32 i = 0; i < mComputeListsCount; ++i)
            {
                Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
                Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
                TempDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&Allocator));
                TempDevice->CreateCommandList1(NODEMASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));
                mComputeLists.push_back(std::shared_ptr<CommandList>(new CommandList(CommandListType::Compute, Allocator, List)));
                mComputeListsStatus.push_back(1);
            }
            TempDevice->Release();
        }

        void CreateList(CommandListType type)
        {
            std::vector<std::shared_ptr<CommandList>>* Lists;
            std::vector<int>* Status;
            uint32* ListsCount;
            D3D12_COMMAND_LIST_TYPE RawType;

            if (type == CommandListType::Graphics)
            {
                Lists = &mGraphicsLists;
                Status = &mGraphicsListsStatus;
                ListsCount = &mGraphicsListsCount;
                RawType = D3D12_COMMAND_LIST_TYPE_DIRECT;
            }
            else if (type == CommandListType::Copy)
            {
                Lists = &mCopyLists;
                Status = &mCopyListsStatus;
                ListsCount = &mCopyListsCount;
                RawType = D3D12_COMMAND_LIST_TYPE_COPY;
            }
            else
            {
                Lists = &mComputeLists;
                Status = &mComputeListsStatus;
                ListsCount = &mComputeListsCount;
                RawType = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            }

            Microsoft::WRL::ComPtr<ID3D12CommandAllocator> Allocator;
            Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> List;
            GetGraphicsDevice()->mDevice->CreateCommandAllocator(RawType, IID_PPV_ARGS(&Allocator));
            GetGraphicsDevice()->mDevice->CreateCommandList1(NODEMASK, RawType, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));

            std::shared_ptr<CommandList> CMDList(new CommandList(type, Allocator, List));
            Lists->push_back(CMDList);
            Status->push_back(1);
            ++(*ListsCount);
        }

        void CollectInternalCommandList()
        {
            std::vector<size_t> IndicesToRemove;

            for (size_t i = 0; i < GetGraphicsDevice()->mUsedCommandListsCount; ++i)
            {
                auto ListType = GetGraphicsDevice()->mUsedCommandLists[i]->GetType();

                if (ListType == CommandListType::Graphics)
                {
                    auto CompletedValue = GetGraphicsDevice()->mGraphicsFence->GetCompletedValue();
                    for (size_t j = 0; j < mGraphicsListsCount; ++j)
                    {
                        if (mGraphicsLists[j].get() == GetGraphicsDevice()->mUsedCommandLists[i] && GetGraphicsDevice()->mUsedCommandListsSignal[i] < CompletedValue)
                        {
                            mGraphicsListsStatus[j] = 1;
                            IndicesToRemove.push_back(i);
                        }
                    }
                }
                else if (ListType == CommandListType::Copy)
                {
                    auto CompletedValue = GetGraphicsDevice()->mCopyFence->GetCompletedValue();
                    for (size_t j = 0; j < mCopyListsCount; ++j)
                    {
                        if (mCopyLists[j].get() == GetGraphicsDevice()->mUsedCommandLists[i] && GetGraphicsDevice()->mUsedCommandListsSignal[i] < CompletedValue)
                        {
                            mCopyListsStatus[j] = 1;
                            IndicesToRemove.push_back(i);
                        }
                    }
                }
                else
                {
                    auto CompletedValue = GetGraphicsDevice()->mComputeFence->GetCompletedValue();
                    for (size_t j = 0; j < mComputeListsCount; ++j)
                    {
                        if (mComputeLists[j].get() == GetGraphicsDevice()->mUsedCommandLists[i] && GetGraphicsDevice()->mUsedCommandListsSignal[i] < CompletedValue)
                        {
                            mComputeListsStatus[j] = 1;
                            IndicesToRemove.push_back(i);
                        }
                    }
                }
            }

            std::sort(IndicesToRemove.begin(), IndicesToRemove.end(), std::greater<size_t>());
            for (size_t index : IndicesToRemove)
            {
                GetGraphicsDevice()->mUsedCommandLists.erase(GetGraphicsDevice()->mUsedCommandLists.begin() + index);
                GetGraphicsDevice()->mUsedCommandListsSignal.erase(GetGraphicsDevice()->mUsedCommandListsSignal.begin() + index);
                --(GetGraphicsDevice()->mUsedCommandListsCount);
            }
        }

        std::vector<std::shared_ptr<CommandList>> mGraphicsLists;
        std::vector<std::shared_ptr<CommandList>> mCopyLists;
        std::vector<std::shared_ptr<CommandList>> mComputeLists;

        std::vector<int32> mGraphicsListsStatus;
        std::vector<int32> mCopyListsStatus;
        std::vector<int32> mComputeListsStatus;

        uint32 mGraphicsListsCount = 8;
        uint32 mCopyListsCount = 2;
        uint32 mComputeListsCount = 2;

        std::mutex mMutex;

        friend CommandListPool* GetCommandListPool();
        friend void CollectCommandList();
    };

    export CommandListPool* GetCommandListPool()
    {
        static CommandListPool* Pool = new CommandListPool(GetGraphicsDevice());
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
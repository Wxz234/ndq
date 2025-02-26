#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "ndq/core/resource.h"
#include "ndq/rhi/command_list.h"
#include "ndq/rhi/device.h"

#include "command_list_internal.h"
#include "device_internal.h"

#include <basetsd.h>
#include <combaseapi.h>
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>
#include <handleapi.h>
#include <synchapi.h>

#include <vector>

#define NDQ_SWAPCHAIN_FORMAT DXGI_FORMAT_R8G8B8A8_UNORM
#define NDQ_SWAPCHAIN_COUNT 3
#define NDQ_NODE_MASK 1

namespace ndq
{
    class Device : public IDevice
    {
    public:
        Device() {}

        ~Device()
        {
            Finalize();

            pDevice->Release();
            pSwapChain->Release();
            pGraphicsQueue->Release();
            pCopyQueue->Release();
            pComputeQueue->Release();
            pFence->Release();
            pCopyFence->Release();
            pComputeFence->Release();

            CloseHandle(mEvent);
            CloseHandle(mCopyEvent);
            CloseHandle(mComputeEvent);

            pRtvHeap->Release();

            for (ID3D12Resource* resource : mRenderTargets)
            {
                resource->Release();
            }
        }

        void Initialize(HWND hwnd, UINT width, UINT height)
        {
            IDXGIFactory7* Factory = nullptr;
            UINT FactoryFlag = 0;
            ID3D12Debug* DebugController = nullptr;
#ifdef _DEBUG
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
            {
                DebugController->EnableDebugLayer();
            }
            FactoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
            CreateDXGIFactory2(FactoryFlag, IID_PPV_ARGS(&Factory));
            IDXGIAdapter4* Adapter;
            Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Adapter));
            D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice));

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
            IDXGISwapChain1* SwapChain;

            Factory->CreateSwapChainForHwnd(pGraphicsQueue, hwnd, &ScDesc, &FsSwapChainDesc, nullptr, &SwapChain);
            Factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

            SwapChain->QueryInterface(&pSwapChain);

            mFrameIndex = pSwapChain->GetCurrentBackBufferIndex();
            pDevice->CreateFence(mFenceValue[mFrameIndex]++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence));
            mEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            pDevice->CreateFence(mCopyFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pCopyFence));
            mCopyEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            pDevice->CreateFence(mComputeFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pComputeFence));
            mComputeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            _CreateRTV();

            Factory->Release();
            if (DebugController)
            {
                DebugController->Release();
            }
            Adapter->Release();
            SwapChain->Release();
        }

        void Finalize()
        {
            Wait(NDQ_COMMAND_LIST_TYPE::GRAPHICS);
            Wait(NDQ_COMMAND_LIST_TYPE::COPY);
            Wait(NDQ_COMMAND_LIST_TYPE::COMPUTE);
        }

        void Present()
        {
            pSwapChain->Present(1, 0);
            MoveToNextFrame();
        }

        ID3D12Device* GetRawDevice() const
        {
            return pDevice;
        }

        void Wait(NDQ_COMMAND_LIST_TYPE type)
        {
            _Wait(type);
        }

        void ExecuteCommandList(ICommandList* pList)
        {
            auto Type = pList->GetType();
            auto TempList = pList->GetRawCommandList();
            ID3D12CommandList* Lists[1] = { reinterpret_cast<ID3D12CommandList*> (TempList) };
            switch (Type)
            {
            case NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                pGraphicsQueue->ExecuteCommandLists(1, Lists);
                break;
            case NDQ_COMMAND_LIST_TYPE::COPY:
                pCopyQueue->ExecuteCommandLists(1, Lists);
                break;
            case NDQ_COMMAND_LIST_TYPE::COMPUTE:
                pComputeQueue->ExecuteCommandLists(1, Lists);
                break;
            }
        }

        TRefCountPtr<ICommandList> CreateCommandList(NDQ_COMMAND_LIST_TYPE type)
        {
            ID3D12CommandAllocator* Allocator = nullptr;
            ID3D12GraphicsCommandList4* List = nullptr;
            switch (type)
            {
            case NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Allocator));
                pDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));
                break;
            case NDQ_COMMAND_LIST_TYPE::COPY:
                pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&Allocator));
                pDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));
                break;
            case NDQ_COMMAND_LIST_TYPE::COMPUTE:
                pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&Allocator));
                pDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&List));
                break;
            }

            TRefCountPtr<ICommandList> pRet;
            CreateCommandListFunction(type, List, Allocator, pRet.ReleaseAndGetAddressOf());
            return pRet;
        }

        void MoveToNextFrame()
        {
            const UINT64 CurrentFenceValue = mFenceValue[mFrameIndex];
            pGraphicsQueue->Signal(pFence, CurrentFenceValue);
            mFrameIndex = pSwapChain->GetCurrentBackBufferIndex();
            if (pFence->GetCompletedValue() < mFenceValue[mFrameIndex])
            {
                pFence->SetEventOnCompletion(mFenceValue[mFrameIndex], mEvent);
                WaitForSingleObjectEx(mEvent, INFINITE, FALSE);
            }
            mFenceValue[mFrameIndex] = CurrentFenceValue + 1;
        }

        void _Wait(NDQ_COMMAND_LIST_TYPE type)
        {
            switch (type)
            {
            case NDQ_COMMAND_LIST_TYPE::GRAPHICS:
                pGraphicsQueue->Signal(pFence, mFenceValue[mFrameIndex]);
                pFence->SetEventOnCompletion(mFenceValue[mFrameIndex]++, mEvent);
                WaitForSingleObjectEx(mEvent, INFINITE, FALSE);
                break;
            case NDQ_COMMAND_LIST_TYPE::COPY:
                pCopyQueue->Signal(pCopyFence, mCopyFenceValue);
                pCopyFence->SetEventOnCompletion(mCopyFenceValue++, mCopyEvent);
                WaitForSingleObjectEx(mCopyEvent, INFINITE, FALSE);
                break;
            case NDQ_COMMAND_LIST_TYPE::COMPUTE:
                pComputeQueue->Signal(pComputeFence, mComputeFenceValue);
                pComputeFence->SetEventOnCompletion(mComputeFenceValue++, mComputeEvent);
                WaitForSingleObjectEx(mComputeEvent, INFINITE, FALSE);
                break;
            }
        }

        D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += mFrameIndex * mRtvDescriptorSize;
            return handle;
        }

        void _CreateRTV()
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = NDQ_SWAPCHAIN_COUNT;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&pRtvHeap));

            mRenderTargets.resize(NDQ_SWAPCHAIN_COUNT);

            D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = pRtvHeap->GetCPUDescriptorHandleForHeapStart();
            mRtvDescriptorSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            for (UINT i = 0; i < NDQ_SWAPCHAIN_COUNT; i++)
            {
                pSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i]));
                pDevice->CreateRenderTargetView(mRenderTargets[i], nullptr, RtvHandle);
                RtvHandle.ptr += mRtvDescriptorSize;
            }
        }

        ID3D12Resource* GetCurrentResource() const
        {
            return mRenderTargets[mFrameIndex];
        }

        ID3D12Device4* pDevice = nullptr;
        IDXGISwapChain4* pSwapChain = nullptr;

        ID3D12CommandQueue* pGraphicsQueue = nullptr;
        ID3D12CommandQueue* pCopyQueue = nullptr;
        ID3D12CommandQueue* pComputeQueue = nullptr;

        UINT32 mFrameIndex = 0;

        UINT64 mFenceValue[NDQ_SWAPCHAIN_COUNT]{};
        ID3D12Fence1* pFence = nullptr;
        HANDLE mEvent = INVALID_HANDLE_VALUE;

        UINT64 mCopyFenceValue = 0;
        ID3D12Fence1* pCopyFence = nullptr;
        HANDLE mCopyEvent = INVALID_HANDLE_VALUE;
        UINT64 mComputeFenceValue = 0;
        ID3D12Fence1* pComputeFence = nullptr;
        HANDLE mComputeEvent = INVALID_HANDLE_VALUE;

        ID3D12DescriptorHeap* pRtvHeap = nullptr;

        std::vector<ID3D12Resource*> mRenderTargets;

        UINT mRtvDescriptorSize = 0;
    };

    IDevice* IDevice::GetGraphicsDevice()
    {
        static IDevice* pDevice = new Device;
        return pDevice;
    }

    void SetDeviceHwndAndSize(void* hwnd, unsigned width, unsigned height)
    {
        auto TempPtr = dynamic_cast<Device*>(IDevice::GetGraphicsDevice());
        HWND* pHwnd = reinterpret_cast<HWND*>(hwnd);
        TempPtr->Initialize(*pHwnd, width, height);
    }

    void DevicePresent()
    {
        auto TempPtr = dynamic_cast<Device*>(IDevice::GetGraphicsDevice());
        TempPtr->Present();
    }

    void DeviceFinalize()
    {
        auto TempPtr = dynamic_cast<Device*>(IDevice::GetGraphicsDevice());
        TempPtr->Finalize();
        delete TempPtr;
    }
}
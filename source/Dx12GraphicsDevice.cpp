#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "ndq/CommandList.h"
#include "ndq/GraphicsDevice.h"
#include "ndq/Type.h"

#include "CommandListInternal.h"
#include "GraphicsDeviceInternal.h"

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
    class Dx12Device : public GraphicsDevice
    {
    public:
        Dx12Device() {}

        ~Dx12Device()
        {
            finalize();

            mDevice->Release();
            mSwapChain->Release();
            mGraphicsQueue->Release();
            mCopyQueue->Release();
            mComputeQueue->Release();
            mFence->Release();
            mCopyFence->Release();
            mComputeFence->Release();

            CloseHandle(mEvent);
            CloseHandle(mCopyEvent);
            CloseHandle(mComputeEvent);

            mRtvHeap->Release();

            for (ID3D12Resource* resource : mRenderTargets)
            {
                resource->Release();
            }
        }

        void initialize(HWND hwnd, UINT width, UINT height)
        {
            IDXGIFactory7* factory = nullptr;
            uint32 factoryFlag = 0;
            ID3D12Debug* debugController = nullptr;
#ifdef _DEBUG
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();
            }
            factoryFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif
            CreateDXGIFactory2(factoryFlag, IID_PPV_ARGS(&factory));
            IDXGIAdapter4* adapter;
            factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
            D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice));

            D3D12_COMMAND_QUEUE_DESC queueDesc{};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mGraphicsQueue));
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCopyQueue));
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
            mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mComputeQueue));

            DXGI_SWAP_CHAIN_DESC1 scDesc{};
            scDesc.BufferCount = NDQ_SWAPCHAIN_COUNT;
            scDesc.Width = width;
            scDesc.Height = height;
            scDesc.Format = NDQ_SWAPCHAIN_FORMAT;
            scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            scDesc.SampleDesc.Count = 1;
            scDesc.SampleDesc.Quality = 0;
            scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            scDesc.Scaling = DXGI_SCALING_STRETCH;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc{};
            fsSwapChainDesc.Windowed = TRUE;
            IDXGISwapChain1* swapChain;

            factory->CreateSwapChainForHwnd(mGraphicsQueue, hwnd, &scDesc, &fsSwapChainDesc, nullptr, &swapChain);
            factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

            swapChain->QueryInterface(&mSwapChain);

            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
            mDevice->CreateFence(mFenceValue[mFrameIndex]++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
            mEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            mDevice->CreateFence(mCopyFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mCopyFence));
            mCopyEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            mDevice->CreateFence(mComputeFenceValue++, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mComputeFence));
            mComputeEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

            _createRTV();

            factory->Release();
            if (debugController)
            {
                debugController->Release();
            }
            adapter->Release();
            swapChain->Release();
        }

        void finalize()
        {
            wait(CommandList::CLT_GRAPHICS);
            wait(CommandList::CLT_COPY);
            wait(CommandList::CLT_COMPUTE);
        }

        void present()
        {
            mSwapChain->Present(1, 0);
            moveToNextFrame();
        }

        void* getRawGraphicsDevice() const
        {
            return mDevice;
        }

        void wait(CommandList::CommandListTypes type)
        {
            _wait(type);
        }

        void executeCommandList(CommandList::CommandListTypes type, uint32 numLists, CommandList* const* lists)
        {
            std::vector<ID3D12CommandList*> tempLists;
            for (uint32 i = 0; i < numLists; ++i)
            {
                auto rawList = lists[i]->getRawCommandList();
                tempLists.push_back(reinterpret_cast<ID3D12CommandList*>(rawList));
            }

            switch (type)
            {
            case CommandList::CLT_GRAPHICS:
                mGraphicsQueue->ExecuteCommandLists(numLists, tempLists.data());
                break;
            case CommandList::CLT_COPY:
                mCopyQueue->ExecuteCommandLists(numLists, tempLists.data());
                break;
            case CommandList::CLT_COMPUTE:
                mComputeQueue->ExecuteCommandLists(numLists, tempLists.data());
                break;
            }
        }

        CommandList* createCommandList(CommandList::CommandListTypes type)
        {
            ID3D12CommandAllocator* allocator = nullptr;
            ID3D12GraphicsCommandList4* list = nullptr;
            switch (type)
            {
            case CommandList::CLT_GRAPHICS:
                mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator));
                mDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list));
                break;
            case CommandList::CLT_COPY:
                mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&allocator));
                mDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COPY, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list));
                break;
            case CommandList::CLT_COMPUTE:
                mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(&allocator));
                mDevice->CreateCommandList1(NDQ_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COMPUTE, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&list));
                break;
            }

            CommandList* ret;
            createCommandListFunction(type, list, allocator, &ret);
            return ret;
        }

        void destroyCommandList(CommandList* list)
        {
            destroyCommandListFunction(list);
        }

        void moveToNextFrame()
        {
            const uint64 currentFenceValue = mFenceValue[mFrameIndex];
            mGraphicsQueue->Signal(mFence, currentFenceValue);
            mFrameIndex = mSwapChain->GetCurrentBackBufferIndex();
            if (mFence->GetCompletedValue() < mFenceValue[mFrameIndex])
            {
                mFence->SetEventOnCompletion(mFenceValue[mFrameIndex], mEvent);
                WaitForSingleObjectEx(mEvent, INFINITE, FALSE);
            }
            mFenceValue[mFrameIndex] = currentFenceValue + 1;
        }

        void _wait(CommandList::CommandListTypes type)
        {
            switch (type)
            {
            case CommandList::CLT_GRAPHICS:
                mGraphicsQueue->Signal(mFence, mFenceValue[mFrameIndex]);
                mFence->SetEventOnCompletion(mFenceValue[mFrameIndex]++, mEvent);
                WaitForSingleObjectEx(mEvent, INFINITE, FALSE);
                break;
            case CommandList::CLT_COPY:
                mCopyQueue->Signal(mCopyFence, mCopyFenceValue);
                mCopyFence->SetEventOnCompletion(mCopyFenceValue++, mCopyEvent);
                WaitForSingleObjectEx(mCopyEvent, INFINITE, FALSE);
                break;
            case CommandList::CLT_COMPUTE:
                mComputeQueue->Signal(mComputeFence, mComputeFenceValue);
                mComputeFence->SetEventOnCompletion(mComputeFenceValue++, mComputeEvent);
                WaitForSingleObjectEx(mComputeEvent, INFINITE, FALSE);
                break;
            }
        }

        D3D12_CPU_DESCRIPTOR_HANDLE getCurrentRenderTargetView() const
        {
            D3D12_CPU_DESCRIPTOR_HANDLE handle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += mFrameIndex * mRtvDescriptorSize;
            return handle;
        }

        void _createRTV()
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = NDQ_SWAPCHAIN_COUNT;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));

            mRenderTargets.resize(NDQ_SWAPCHAIN_COUNT);

            D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRtvHeap->GetCPUDescriptorHandleForHeapStart();
            mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            for (uint32 i = 0; i < NDQ_SWAPCHAIN_COUNT; ++i)
            {
                mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i]));
                mDevice->CreateRenderTargetView(mRenderTargets[i], nullptr, rtvHandle);
                rtvHandle.ptr += mRtvDescriptorSize;
            }
        }

        void* getCurrentResource() const
        {
            return mRenderTargets[mFrameIndex];
        }

        void setCurrentResourceRenderTarget(CommandList* list)
        {
            auto rawList = (ID3D12GraphicsCommandList*)list->getRawCommandList();
            auto rtvHandle = getCurrentRenderTargetView();
            rawList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        }

        void clearCurrentResourceRenderTargetView(CommandList* list, const float colorRGBA[4])
        {
            auto rawList = (ID3D12GraphicsCommandList*)list->getRawCommandList();
            auto rtvHandle = getCurrentRenderTargetView();
            rawList->ClearRenderTargetView(rtvHandle, colorRGBA, 0, nullptr);
        }

        ID3D12Device4* mDevice = nullptr;
        IDXGISwapChain4* mSwapChain = nullptr;

        ID3D12CommandQueue* mGraphicsQueue = nullptr;
        ID3D12CommandQueue* mCopyQueue = nullptr;
        ID3D12CommandQueue* mComputeQueue = nullptr;

        uint32 mFrameIndex = 0;

        uint64 mFenceValue[NDQ_SWAPCHAIN_COUNT]{};
        ID3D12Fence1* mFence = nullptr;
        HANDLE mEvent = INVALID_HANDLE_VALUE;

        uint64 mCopyFenceValue = 0;
        ID3D12Fence1* mCopyFence = nullptr;
        HANDLE mCopyEvent = INVALID_HANDLE_VALUE;
        uint64 mComputeFenceValue = 0;
        ID3D12Fence1* mComputeFence = nullptr;
        HANDLE mComputeEvent = INVALID_HANDLE_VALUE;

        ID3D12DescriptorHeap* mRtvHeap = nullptr;

        std::vector<ID3D12Resource*> mRenderTargets;

        uint32 mRtvDescriptorSize = 0;
    };

    GraphicsDevice* GraphicsDevice::getGraphicsDevice()
    {
        static GraphicsDevice* device = new Dx12Device;
        return device;
    }

    void setDeviceHwndAndSize(void* hwnd, unsigned width, unsigned height)
    {
        auto tempPtr = dynamic_cast<Dx12Device*>(GraphicsDevice::getGraphicsDevice());
        HWND* hwndPtr = reinterpret_cast<HWND*>(hwnd);
        tempPtr->initialize(*hwndPtr, width, height);
    }

    void devicePresent()
    {
        auto tempPtr = dynamic_cast<Dx12Device*>(GraphicsDevice::getGraphicsDevice());
        tempPtr->present();
    }

    void deviceFinalize()
    {
        auto tempPtr = dynamic_cast<Dx12Device*>(GraphicsDevice::getGraphicsDevice());
        tempPtr->finalize();
        delete tempPtr;
    }
}
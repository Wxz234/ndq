#pragma once

#include "ndq/core/resource.h"
#include "ndq/rhi/command_list.h"

#include <d3d12.h>

namespace ndq
{
    class IDevice
    {
    public:
        virtual TRefCountPtr<ID3D12Device> GetRawDevice() const = 0;
        virtual TRefCountPtr<ICommandList> CreateCommandList(NDQ_COMMAND_LIST_TYPE type) = 0;
        virtual void ExecuteCommandList(ICommandList* pList) = 0;
        virtual void Wait(NDQ_COMMAND_LIST_TYPE type) = 0;
        virtual D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const = 0;
        virtual TRefCountPtr<ID3D12Resource> GetCurrentResource() const = 0;

        static IDevice* GetGraphicsDevice();
    };

}

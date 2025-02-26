#pragma once

#include "ndq/rhi/command_list.h"

#include <d3d12.h>

namespace ndq
{
    void CreateCommandListFunction(NDQ_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList4* pList, ID3D12CommandAllocator* pAllocator, ICommandList** ppCmdList);
}
#pragma once

#include "ndq/CommandList.h"

#include <d3d12.h>

namespace ndq
{
    void createCommandListFunction(CommandList::CommandListTypes type, ID3D12GraphicsCommandList4* list, ID3D12CommandAllocator* allocator, CommandList** cmdLists);
    void destroyCommandListFunction(CommandList* list);
}
#include "ndq/CommandList.h"

#include "CommandListInternal.h"

#include <d3d12.h>

namespace ndq
{
    class Dx12CommandList : public CommandList
    {
    public:
        Dx12CommandList(CommandListTypes type,
            ID3D12GraphicsCommandList4* list,
            ID3D12CommandAllocator* allocator) :
            mType(type), mList(list), mAllocator(allocator) {
        }

        ~Dx12CommandList()
        {
            mAllocator->Release();
            mList->Release();
        }

        void* getRawCommandList() const
        {
            return mList;
        }

        void open()
        {
            mAllocator->Reset();
            mList->Reset(mAllocator, nullptr);
        }

        void close()
        {
            mList->Close();
        }

        CommandListTypes getType() const
        {
            return mType;
        }

        CommandListTypes mType;
        ID3D12GraphicsCommandList4* mList;
        ID3D12CommandAllocator* mAllocator;
    };

    void createCommandListFunction(CommandList::CommandListTypes type, ID3D12GraphicsCommandList4* list, ID3D12CommandAllocator* allocator, CommandList** cmdLists)
    {
        if (cmdLists == nullptr)
        {
            return;
        }
        *cmdLists = new Dx12CommandList(type, list, allocator);
    }

    void destroyCommandListFunction(CommandList* list)
    {
        auto tempPtr = (Dx12CommandList*)list;
        delete tempPtr;
    }
}
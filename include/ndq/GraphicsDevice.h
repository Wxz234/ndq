#pragma once

#include "ndq/CommandList.h"
#include "ndq/Defs.h"
#include "ndq/Type.h"

namespace ndq
{
    class GraphicsDevice
    {
    public:
        virtual ~GraphicsDevice() = default;

        virtual void* getRawGraphicsDevice() const = 0;
        virtual CommandList* createCommandList(CommandList::CommandListTypes type) = 0;
        virtual void destroyCommandList(CommandList* list) = 0;
        virtual void executeCommandList(CommandList::CommandListTypes type, uint32 numLists, CommandList* const* lists) = 0;
        virtual void wait(CommandList::CommandListTypes type) = 0;
        virtual void* getCurrentResource() const = 0;
        virtual void setCurrentResourceRenderTarget(CommandList* list) = 0;
        virtual void clearCurrentResourceRenderTargetView(CommandList* list, const float colorRGBA[4]) = 0;

        static GraphicsDevice* getGraphicsDevice();
    };
}
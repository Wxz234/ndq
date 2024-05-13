#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include "ndq/platform.h"
#include "ndq/rhi.h"

namespace Internal
{
    struct GraphicsDeviceInterface
    {
        virtual void Present() = 0;
        virtual void Initialize(HWND hwnd, UINT width, UINT height) = 0;
        virtual void Release() = 0;
        virtual void RunGarbageCollection() = 0;
        virtual ndq::uint64 GetCompletedFenceValue(ndq::NDQ_COMMAND_LIST_TYPE type) const = 0;
    };
}

module;

#include "predef.h"

export module ndq:render_data;

import :platform;

export namespace ndq
{
    struct RenderData
    {
        uint32 *Width = nullptr;
        uint32 *Height = nullptr;
        void* MainCamera = nullptr;
        void* StaticModel = nullptr;
        void* DefaultSkyLight = nullptr;
    };
}
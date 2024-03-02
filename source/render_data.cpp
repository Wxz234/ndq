module;

#include "predef.h"

export module ndq:render_data;

export namespace ndq
{
    struct RenderData
    {
        void* MainCamera = nullptr;
        void* StaticModel = nullptr;
        void* DefaultSkyLight = nullptr;
    };
}
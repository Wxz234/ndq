#pragma once

#include "ndq/platform.h"

namespace ndq
{
    class IApplication
    {
    public:
        int Run();

        virtual void Initialize() {}
        virtual void Finalize() {}
        virtual void Update(float t) {}

        uint32 Width = 800;
        uint32 Height = 600;
        const wchar_t* Title = L"ndq";
    };
}
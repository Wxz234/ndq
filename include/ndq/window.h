#pragma once

#include "ndq/platform.h"

namespace ndq
{
    class IApplication
    {
    public:
        int Run();

        virtual void Initialize() = 0;
        virtual void Finalize() = 0;
        virtual void Update(float t) = 0;

        uint32 Width = 800;
        uint32 Height = 600;
        const wchar_t* Title = L"ndq";
    };
}
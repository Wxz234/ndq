#pragma once

#include "ndq/platform.h"

namespace ndq
{
    enum class NDQ_SHADER_TYPE
    {
        VERTEX,
        PIXEL,
    };

    struct NDQ_SHADER_DEFINE
    {
        const wchar_t* Name;
        const wchar_t* Value;
    };

    class IShader
    {
    public:
        virtual NDQ_SHADER_TYPE GetShaderType() const = 0;
        virtual void* GetBlobPointer() const = 0;
        virtual size_type GetBlobSize() const = 0;
    };
}
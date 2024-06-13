#pragma once

#include "ndq/platform.h"

#include <memory>

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

    std::shared_ptr<IShader> CompileShaderFromFile(const wchar_t* filePath, NDQ_SHADER_TYPE shaderType, const wchar_t* entryPoint, const NDQ_SHADER_DEFINE* pDefines, uint32 defineCount);
}
#pragma once

#include "ndq/core/blob.h"
#include "ndq/core/resource.h"

namespace ndq
{
    TRefCountPtr<IBlob> LoadShaderFromPath(const wchar_t* path, const wchar_t** pArguments, unsigned argCount);
}
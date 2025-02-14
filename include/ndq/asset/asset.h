#pragma once

#include "ndq/core/resource.h"

namespace ndq
{
    class IAsset : public IRefCounted
    {
    public:
        virtual bool HasError() const = 0;
    };
}
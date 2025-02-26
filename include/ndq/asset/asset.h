#pragma once

#include "ndq/core/resource.h"

#include <string>

namespace ndq
{
    class IAsset : public IRefCounted
    {
    public:
        virtual std::string GetUUID() const = 0;
        virtual std::string GetAssetType() const = 0;
    };
}
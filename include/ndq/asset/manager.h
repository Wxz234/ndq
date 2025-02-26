#pragma once

#include "ndq/asset/asset.h"
#include "ndq/core/resource.h"

namespace ndq
{
    class AssetManager
    {
    public:
        static AssetManager* GetAssetManager();

        TRefCountPtr<IAsset> LoadAssetFromPath(const wchar_t* path);
    private:
        AssetManager() = default;
        AssetManager(const AssetManager&) = default;
        AssetManager(AssetManager&&) noexcept = default;
        AssetManager& operator=(const AssetManager&) = default;
        AssetManager& operator=(AssetManager&&) noexcept = default;
    };
}
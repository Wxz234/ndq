#include "ndq/asset/asset.h"
#include "ndq/asset/manager.h"
#include "ndq/core/resource.h"

namespace ndq
{
    AssetManager* AssetManager::GetAssetManager()
    {
        return nullptr;
    }

    TRefCountPtr<IAsset> AssetManager::LoadAssetFromPath(const wchar_t* path)
    {
        return TRefCountPtr<IAsset>(nullptr);
    }
}
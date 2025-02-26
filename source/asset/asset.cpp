#include "ndq/asset/asset.h"
#include "ndq/core/uuid.h"

#include <atomic>
#include <filesystem>
#include <string>

namespace ndq
{
    class Asset : public IAsset
    {
    public:
        Asset(const wchar_t* path)
        {
            mRefCount = 1;
            mPath = path;
            mUuidStr = GenerateUUID();
        }

        unsigned long AddRef()
        {
            return ++mRefCount;
        }

        unsigned long Release()
        {
            unsigned long result = --mRefCount;
            if (result == 0)
            {
                delete this;
            }
            return result;
        }

        std::string GetUUID() const
        {
            return mUuidStr;
        }

        std::string GetAssetType() const
        {
            return mPath.extension().string();
        }

        std::atomic<unsigned long> mRefCount;
        std::filesystem::path mPath;
        std::string mUuidStr;
    };

    void CreateAssetFromPath(const wchar_t* path, IAsset** ppAsset)
    {
        if (!ppAsset)
        {
            return;
        }
        *ppAsset = new Asset(path);
    }
}
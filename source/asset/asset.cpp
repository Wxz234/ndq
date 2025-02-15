#include "ndq/asset/asset.h"
#include "ndq/core/uuid.h"

#include <atomic>

namespace ndq
{
    class Asset : public IAsset
    {
    public:

        Asset()
        {
            mRefCount = 1;
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
            return mUuid.ToString();
        }

        UUID mUuid;
        std::atomic<unsigned long> mRefCount;
    };
}
#include "ndq/asset/asset.h"

#include <cstddef>
#include <vector>

namespace ndq
{
    class Asset : public IAsset
    {
    public:


        std::vector<std::byte> mByte;
    };
}
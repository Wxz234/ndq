module;

#include "predef.h"

export module ndq:asset;

namespace ndq
{
    enum class ASSET_TYPE
    {
        STATIC_MODEL
    };

    class Asset
    {
    public:
        virtual ASSET_TYPE GetType() const = 0;
    };
}
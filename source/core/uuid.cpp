#include "ndq/core/uuid.h"

#include <rpc.h>

namespace ndq
{
    class UUID::DATA
    {
    public:
        DATA()
        {
            UuidCreate(&uuid);
            UuidToStringA(&uuid, &uuidStr);
        }

        ~DATA()
        {
            RpcStringFreeA(&uuidStr);
        }

        ::UUID uuid;
        RPC_CSTR uuidStr;
    };

    UUID::UUID()
    {
        pimpl = new UUID::DATA;
    }

    UUID::UUID(const UUID& u)
    {
        pimpl = new UUID::DATA;
        pimpl->uuid = u.pimpl->uuid;
    }

    UUID::UUID(UUID&& u) noexcept
    {
        pimpl = new UUID::DATA;
        pimpl->uuid = u.pimpl->uuid;
    }

    UUID& UUID::operator=(const UUID& u)
    {
        pimpl->uuid = u.pimpl->uuid;
        return *this;
    }

    UUID& UUID::operator= (UUID&& u) noexcept
    {
        pimpl->uuid = u.pimpl->uuid;
        return *this;
    }

    UUID::~UUID()
    {
        delete pimpl;
    }

    std::strong_ordering UUID::operator<=>(const UUID& other) const
    {
        RPC_STATUS status;
        return UuidCompare(&pimpl->uuid, &other.pimpl->uuid, &status) <=> 0;
    }

    bool UUID::operator==(const UUID& other) const
    {
        RPC_STATUS status;
        return UuidCompare(&pimpl->uuid, &other.pimpl->uuid, &status) == 0;
    }

    std::string UUID::ToString() const
    {
        return std::string(reinterpret_cast<char*>(pimpl->uuidStr));
    }
}
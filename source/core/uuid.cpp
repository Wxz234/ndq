#include "ndq/core/uuid.h"

#include <rpc.h>

#include <memory>

namespace ndq
{
    void UUIDStrDeleter(RPC_CSTR* str)
    {
        RpcStringFreeA(str);
        delete str;
    }

    class UUID::DATA
    {
    public:
        DATA() : uuid{}, uuidStr(new RPC_CSTR, UUIDStrDeleter)
        {
            UuidCreate(&uuid);
            UuidToStringA(&uuid, uuidStr.get());
        }

        ::UUID uuid;
        std::shared_ptr<RPC_CSTR> uuidStr;
    };

    UUID::UUID()
    {
        pimpl = new UUID::DATA;
    }

    UUID::UUID(const UUID& u)
    {
        pimpl = new UUID::DATA;
        pimpl->uuid = u.pimpl->uuid;
        pimpl->uuidStr = u.pimpl->uuidStr;
    }

    UUID::UUID(UUID&& u) noexcept
    {
        pimpl = new UUID::DATA;
        pimpl->uuid = u.pimpl->uuid;
        pimpl->uuidStr = u.pimpl->uuidStr;
    }

    UUID& UUID::operator=(const UUID& u)
    {
        pimpl->uuid = u.pimpl->uuid;
        pimpl->uuidStr = u.pimpl->uuidStr;
        return *this;
    }

    UUID& UUID::operator= (UUID&& u) noexcept
    {
        pimpl->uuid = u.pimpl->uuid;
        pimpl->uuidStr = u.pimpl->uuidStr;
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
        return std::string(reinterpret_cast<char*>(*pimpl->uuidStr));
    }
}
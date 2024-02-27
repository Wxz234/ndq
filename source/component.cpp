module;

#include "predef.h"
#include "entt/entt.hpp"

export module ndq:component;

import :entity;
import :platform;

namespace ndq
{
    template<typename T, typename... Args>
    T& EmplaceComponent(uint32 entity,Args &&...args)
    {
        auto Reg = Internal::GetRegistry();
        return Reg->emplace<T>(static_cast<entt::entity>(entity), std::forward<Args>(args)...);
    }

    template<typename... Types>
    size_type RemoveComponent(uint32 entity)
    {
        auto Reg = Internal::GetRegistry();
        return Reg->remove<Types...>(static_cast<entt::entity>(entity));
    }

    template<typename T>
    T& GetComponent(uint32 entity)
    {
        auto Reg = Internal::GetRegistry();
        return Reg->get<T>(static_cast<entt::entity>(entity));
    }
}
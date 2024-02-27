module;

#include "predef.h"
#include "entt/entt.hpp"

export module ndq:entity;

import :platform;

namespace Internal
{
    entt::registry* GetRegistry()
    {
        static entt::registry Reg;
        return &Reg;
    }
}

export namespace ndq
{
    uint32 CreateEntity()
    {
        auto Reg = Internal::GetRegistry();
        return static_cast<uint32>(Reg->create());
    }
}
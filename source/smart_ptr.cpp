module;

#include "predef.h"

export module ndq:smart_ptr;

export namespace ndq
{
    template <typename T>
    using shared_ptr = std::shared_ptr<T>;

    template <typename T, typename D = std::default_delete<T>>
    using unique_ptr = std::unique_ptr<T, D>;

    template <typename T>
    using weak_ptr = std::weak_ptr<T>;
}
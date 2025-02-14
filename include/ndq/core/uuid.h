#pragma once

#include <compare>
#include <string>

namespace ndq
{
    class UUID
    {
    public:
        UUID();
        UUID(const UUID& u);
        UUID(UUID&& u) noexcept;
        UUID& operator=(const UUID& u);
        UUID& operator=(UUID&& u) noexcept;
        ~UUID();

        std::strong_ordering operator<=>(const UUID& other) const;
        bool operator==(const UUID& other) const;

        std::string ToString() const;
    private:
        class DATA;
        DATA* pimpl;
    };
}
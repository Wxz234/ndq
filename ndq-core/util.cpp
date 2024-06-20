#include "ndq/platform.h"

#include <cctype>
#include <charconv>
#include <string>

std::string RemoveTrailingNumbers(const std::string& input)
{
    auto pos = input.size();
    while (pos > 0 && std::isdigit(input[pos - 1]))
    {
        --pos;
    }
    return input.substr(0, pos);
}

ndq::uint32 ExtractTrailingNumbers(const std::string& input)
{
    auto pos = input.size();
    while (pos > 0 && std::isdigit(input[pos - 1]))
    {
        --pos;
    }
    std::string trailingNumbersStr = input.substr(pos);
    if (trailingNumbersStr.empty())
    {
        return 0;
    }

    ndq::uint32 result = 0xffffffff;
    std::from_chars(trailingNumbersStr.data(), trailingNumbersStr.data() + trailingNumbersStr.size(), result);

    return result;
}
#include "ndq/platform.h"

#include <cctype>
#include <charconv>
#include <limits>
#include <string>
#include <system_error>

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

    ndq::uint32 result = 0;
    auto [ptr, ec] = std::from_chars(trailingNumbersStr.data(), trailingNumbersStr.data() + trailingNumbersStr.size(), result);
    if (ec == std::errc::invalid_argument)
    {
        return std::numeric_limits<ndq::uint32>::max();
    }
    else if (ec == std::errc::result_out_of_range)
    {
        return std::numeric_limits<ndq::uint32>::max();
    }

    return result;
}
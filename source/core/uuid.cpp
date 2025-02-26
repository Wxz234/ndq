#include "ndq/core/uuid.h"

#include <cstdint>
#include <random>
#include <string>

namespace ndq
{
    std::string GenerateUUID()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);

        uint8_t bytes[16];
        for (auto& byte : bytes)
        {
            byte = dis(gen);
        }

        bytes[8] &= 0xBF;
        bytes[8] |= 0x80;
        bytes[6] &= 0x4F;
        bytes[6] |= 0x40;

        std::string uuidStr = "00000000-0000-0000-0000-000000000000";
        constexpr char guid_encoder[17] = "0123456789abcdef";
        for (size_t i = 0, index = 0; i < 36; ++i)
        {
            if (i == 8 || i == 13 || i == 18 || i == 23)
            {
                continue;
            }

            uuidStr[i] = guid_encoder[bytes[index] >> 4 & 0x0f];
            uuidStr[++i] = guid_encoder[bytes[index] & 0x0f];
            ++index;
        }

        return uuidStr;
    }
}
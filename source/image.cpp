module;

#include "predef.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

export module ndq:image;

import :platform;

namespace ndq
{
    enum class IMAGE_FORMAT
    {
        UNKNOWN,
        RGBA_U8
    };

    struct Image
    {
        std::vector<uint8> RawData;
        uint32 Width;
        uint32 Height;
        IMAGE_FORMAT Format;
    };

    Image LoadImageFromPath(const char* path)
    {
        Image image;
        int width, height, channels;
        unsigned char* data = stbi_load(path, &width, &height, &channels, STBI_rgb_alpha);
        if (data)
        {
            image.Width = static_cast<uint32_t>(width);
            image.Height = static_cast<uint32_t>(height);
            image.Format = IMAGE_FORMAT::RGBA_U8;
            image.RawData = std::vector<uint8_t>(data, data + width * height * 4);

            stbi_image_free(data);
        }
        else
        {
            image.Width = 0;
            image.Height = 0;
            image.Format = IMAGE_FORMAT::UNKNOWN;
        }
        return image;
    }
}
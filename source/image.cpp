module;

#include "predef.h"

#include <combaseapi.h>
#include <intsafe.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <filesystem>
#include <utility>
#include <vector>

export module ndq:image;

import :platform;

namespace ndq
{
    export enum class IMAGE_FORMAT
    {
        UNKNOWN,
        R8G8B8A8_UNORM,
        NUM
    };

    IMAGE_FORMAT GetImageFormat(WICPixelFormatGUID format)
    {
        if (format == GUID_WICPixelFormat32bppRGBA)
        {
            return IMAGE_FORMAT::R8G8B8A8_UNORM;
        }
        return IMAGE_FORMAT::UNKNOWN;
    }

    WICPixelFormatGUID GetWICFormat(IMAGE_FORMAT format)
    {
        if (format == IMAGE_FORMAT::R8G8B8A8_UNORM)
        {
            return GUID_WICPixelFormat32bppRGBA;
        }
        return GUID_WICPixelFormatUndefined;
    }

    uint32 GetBPP(IMAGE_FORMAT format)
    {
        if (format == IMAGE_FORMAT::R8G8B8A8_UNORM)
        {
            return 32;
        }
        return 0;
    }
}

export namespace ndq
{
    class Image
    {
    public:
        Image() { Clear(); }
        Image(const void* data, uint32 count, uint32 width, uint32 height, IMAGE_FORMAT type) :
            mRawData(reinterpret_cast<const uint8*>(data), reinterpret_cast<const uint8*>(data) + count),
            mWidth(width), mHeight(height), mFormat(type) {}

        Image(const Image& r)
        {
            mRawData = r.mRawData;
            mWidth = r.mWidth;
            mHeight = r.mHeight;
            mFormat = r.mFormat;
        }

        Image(Image&& r) noexcept
        {
            mRawData = std::move(r.mRawData);
            mWidth = r.mWidth;
            mHeight = r.mHeight;
            mFormat = r.mFormat;

            r.Clear();
        }

        Image& operator=(const Image& r)
        {
            if (this != &r)
            {
                mRawData = r.mRawData;
                mWidth = r.mWidth;
                mHeight = r.mHeight;
                mFormat = r.mFormat;
            }
            return *this;
        }

        Image& operator=(Image&& r) noexcept
        {
            if (this != &r)
            {
                mRawData = std::move(r.mRawData);
                mWidth = r.mWidth;
                mHeight = r.mHeight;
                mFormat = r.mFormat;

                r.Clear();
            }
            return *this;
        }

        void Clear()
        {
            mRawData.clear();
            mWidth = 0;
            mHeight = 0;
            mFormat = IMAGE_FORMAT::UNKNOWN;
        }

        uint32 GetWidth() const
        {
            return mWidth;
        }

        uint32 GetHeight() const
        {
            return mHeight;
        }

        IMAGE_FORMAT GetFormat() const
        {
            return mFormat;
        }

    private:
        std::vector<uint8> mRawData;
        uint32 mWidth;
        uint32 mHeight;
        IMAGE_FORMAT mFormat;
    };

    Image LoadTextureFromFile(const char* path)
    {
        Microsoft::WRL::ComPtr<IWICImagingFactory2> pWIC;
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWIC))))
        {
            return Image();
        }

        std::filesystem::path MyPath(path);
        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        if (FAILED(pWIC->CreateDecoderFromFilename(MyPath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder)))
        {
            return Image();
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        if (FAILED(decoder->GetFrame(0, &frame)))
        {
            return Image();
        }

        uint32 width, height;
        if (FAILED(frame->GetSize(&width, &height)))
        {
            return Image();
        }
        if (width == 0 || height == 0)
        {
            return Image();
        }

        // Determine format
        WICPixelFormatGUID pixelFormat;
        if (FAILED(frame->GetPixelFormat(&pixelFormat)))
        {
            return Image();
        }

        Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
        if (FAILED(pWIC->CreateFormatConverter(&converter)))
        {
            return Image();
        }

        if (auto format = GetImageFormat(pixelFormat); format == IMAGE_FORMAT::UNKNOWN)
        {
            for (int32 i = 1; i < static_cast<int32>(IMAGE_FORMAT::NUM); ++i)
            {
                auto Format = static_cast<IMAGE_FORMAT>(i);
                auto DestFormat = GetWICFormat(Format);
                BOOL CanConvert;
                if (FAILED(converter->CanConvert(pixelFormat, DestFormat, &CanConvert)))
                {
                    return Image();
                }

                if (CanConvert)
                {
                    if (FAILED(converter->Initialize(frame.Get(), DestFormat, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom)))
                    {
                        return Image();
                    }
                    else
                    {
                        auto bpp = GetBPP(Format);
                        if (bpp == 0)
                        {
                            return Image();
                        }

                        size_type imageSize = width * height * bpp / 8;
                        std::vector<uint8> buffer(imageSize);
                        if (FAILED(converter->CopyPixels(nullptr, width * (bpp / 8), buffer.size(), buffer.data())))
                        {
                            return Image();
                        }

                        return Image(buffer.data(), buffer.size(), width, height, Format);

                    }
                }
            }
        }
        else
        {
            auto bpp = GetBPP(format);
            if (bpp == 0)
            {
                return Image();
            }

            size_type imageSize = width * height * bpp / 8;
            std::vector<uint8> buffer(imageSize);
            if (FAILED(converter->CopyPixels(nullptr, width * (bpp / 8), buffer.size(), buffer.data())))
            {
                return Image();
            }

            return Image(buffer.data(), buffer.size(), width, height, format);
        }

        return Image();
    }
}
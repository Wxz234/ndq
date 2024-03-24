module;

#include "predef.h"

export module ndq:image;

import :platform;

namespace ndq
{
    struct WICTranslate
    {
        const GUID& wic;
        DXGI_FORMAT format;

        constexpr WICTranslate(const GUID& wg, DXGI_FORMAT fmt) noexcept :
            wic(wg),
            format(fmt) {}
    };

    DXGI_FORMAT WICToDXGI(const GUID& guid) noexcept
    {
        constexpr WICTranslate g_WICFormats[] =
        {
            { GUID_WICPixelFormat128bppRGBAFloat,       DXGI_FORMAT_R32G32B32A32_FLOAT },

            { GUID_WICPixelFormat64bppRGBAHalf,         DXGI_FORMAT_R16G16B16A16_FLOAT },
            { GUID_WICPixelFormat64bppRGBA,             DXGI_FORMAT_R16G16B16A16_UNORM },

            { GUID_WICPixelFormat32bppRGBA,             DXGI_FORMAT_R8G8B8A8_UNORM },
            { GUID_WICPixelFormat32bppBGRA,             DXGI_FORMAT_B8G8R8A8_UNORM },
            { GUID_WICPixelFormat32bppBGR,              DXGI_FORMAT_B8G8R8X8_UNORM },

            { GUID_WICPixelFormat32bppRGBA1010102XR,    DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
            { GUID_WICPixelFormat32bppRGBA1010102,      DXGI_FORMAT_R10G10B10A2_UNORM },

            { GUID_WICPixelFormat16bppBGRA5551,         DXGI_FORMAT_B5G5R5A1_UNORM },
            { GUID_WICPixelFormat16bppBGR565,           DXGI_FORMAT_B5G6R5_UNORM },

            { GUID_WICPixelFormat32bppGrayFloat,        DXGI_FORMAT_R32_FLOAT },
            { GUID_WICPixelFormat16bppGrayHalf,         DXGI_FORMAT_R16_FLOAT },
            { GUID_WICPixelFormat16bppGray,             DXGI_FORMAT_R16_UNORM },
            { GUID_WICPixelFormat8bppGray,              DXGI_FORMAT_R8_UNORM },

            { GUID_WICPixelFormat8bppAlpha,             DXGI_FORMAT_A8_UNORM },

            { GUID_WICPixelFormat96bppRGBFloat,         DXGI_FORMAT_R32G32B32_FLOAT },
        };

        for (size_t i = 0; i < std::size(g_WICFormats); ++i)
        {
            if (memcmp(&g_WICFormats[i].wic, &guid, sizeof(GUID)) == 0)
                return g_WICFormats[i].format;
        }

        return DXGI_FORMAT_UNKNOWN;
    }
}

export namespace ndq
{
    enum class IMAGE_FORMAT
    {
        UNKNOWN,
        R8G8B8A8_UNORM
    };

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
        if (auto hr = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWIC)); FAILED(hr))
        {
            return Image();
        }

        std::filesystem::path MyPath(path);
        Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
        if (auto hr = pWIC->CreateDecoderFromFilename(MyPath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder); FAILED(hr))
        {
            return Image();
        }

        Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
        if (auto hr = decoder->GetFrame(0, &frame); FAILED(hr))
        {
            return Image();
        }

        uint32 width, height;
        if (auto hr = frame->GetSize(&width, &height); FAILED(hr))
        {
            return Image();
        }
        if (width == 0 || height == 0)
        {
            return Image();
        }
        
        size_type maxsize = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
        uint32 twidth = width;
        uint32 theight = height;
        //todo POW2
        if (width > maxsize || height > maxsize)
        {
            const float ar = static_cast<float>(height) / static_cast<float>(width);
            if (width > height)
            {
                twidth = static_cast<uint32>(maxsize);
                theight = std::max<uint32>(1, static_cast<uint32>(static_cast<float>(maxsize) * ar));
            }
            else
            {
                theight = static_cast<uint32>(maxsize);
                twidth = std::max<uint32>(1, static_cast<uint32>(static_cast<float>(maxsize) / ar));
            }

            if (twidth > maxsize || theight > maxsize)
            {
                return Image();
            }
        }

        // Determine format
        WICPixelFormatGUID pixelFormat;
        if (auto hr = frame->GetPixelFormat(&pixelFormat); FAILED(hr))
        {
            return Image();
        }

        WICPixelFormatGUID convertGUID;
        memcpy_s(&convertGUID, sizeof(WICPixelFormatGUID), &pixelFormat, sizeof(GUID));

        size_type bpp = 0;

        return Image();
    }
}
export module ndq:asset;

namespace ndq
{
    enum class ASSET_TYPE
    {
        STATIC_MODEL,
    };

    class IAsset
    {
    public:
        virtual ASSET_TYPE GetType() const = 0;
    };

    IAsset* LoadStaticModel(const char* path);

    void RemoveAsset(IAsset* pAsset);
}

namespace ndq
{
    class Asset : public IAsset
    {
    public:
        virtual ~Asset() {}
    };
}

namespace ndq
{
    IAsset* LoadStaticModel(const char* path)
    {
        return nullptr;
    }

    void RemoveAsset(IAsset* pAsset)
    {

    }
}
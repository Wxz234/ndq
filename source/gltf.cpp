module;

#include "asset_proxy.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>

export module ndq:gltf;

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::Storage;

export namespace ndq
{
    IAsyncAction ReadTextFileAsync(const wchar_t* path)
    {
        auto localFolder = ApplicationData::Current().LocalFolder();
        StorageFile file = co_await localFolder.GetFileAsync(path);
        hstring text = co_await FileIO::ReadTextAsync(file);
        co_return;
    }

    AssetProxy LoadGLTF(const char* path)
    {



        return AssetProxy();
    }
}
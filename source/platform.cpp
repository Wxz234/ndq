module;

#include "predef.h"

export module ndq:platform;

export namespace ndq
{
    using int8   = int8_t;
    using int16  = int16_t;
    using int32  = int32_t;
    using int64  = int64_t;
    using uint8  = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;
    using size_type = size_t;
}

void* GetDllExport(HMODULE module, const char* name)
{
    return (void*)GetProcAddress(module, name);
}

enum class DllType
{
    D3D12,
    DXGI,
};

HMODULE GetDll(DllType type)
{
    static auto D3d12 = LoadLibraryA("d3d12.dll");
    static auto Dxgi = LoadLibraryA("dxgi.dll");

    HMODULE Module{};
    switch (type)
    {
    case DllType::D3D12:
        Module = D3d12;
        break;
    case DllType::DXGI:
        Module = Dxgi;
        break;
    default:
        break;
    }

    return Module;
}

namespace Internal
{
    void CreateAllDll()
    {
        GetDll(DllType::D3D12);
        GetDll(DllType::DXGI);
    }

    void RemoveAllDll()
    {
        FreeLibrary(GetDll(DllType::D3D12));
        FreeLibrary(GetDll(DllType::DXGI));
    }
}
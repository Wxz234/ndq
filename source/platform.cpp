module;

#include "predef.h"

export module ndq:platform;

namespace ndq
{
    export using int8   = int8_t;
    export using int16  = int16_t;
    export using int32  = int32_t;
    export using int64  = int64_t;
    export using uint8  = uint8_t;
    export using uint16 = uint16_t;
    export using uint32 = uint32_t;
    export using uint64 = uint64_t;

    HMODULE GetDllHandleFromPath(const char* name)
    {
        return LoadLibraryA(name);
    }

    void* GetDllExport(HMODULE module, const char* name)
    {
        return (void*)GetProcAddress(module, name);
    }

    void FreeDllHandle(HMODULE module)
    {
        FreeLibrary(module);
    }

    enum class DllType
    {
        DXCOMPILER,
        D3D12,
        DXGI,
    };

    HMODULE GetDll(DllType type)
    {
        static auto Dxcompiler = GetDllHandleFromPath(NDQ_DXCOMPILER_DLL);
        static auto D3d12 = GetDllHandleFromPath("d3d12.dll");
        static auto Dxgi = GetDllHandleFromPath("dxgi.dll");

        HMODULE Module{};
        switch (type)
        {
        case DllType::DXCOMPILER:
            Module = Dxcompiler;
            break;
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

    void RemoveAllDll()
    {
        FreeDllHandle(GetDll(DllType::DXCOMPILER));
        FreeDllHandle(GetDll(DllType::D3D12));
        FreeDllHandle(GetDll(DllType::DXGI));
    }
}
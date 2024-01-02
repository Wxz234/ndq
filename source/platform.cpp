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

    HMODULE GetDllHandleFromWindowsSDK(const char* name)
    {
        std::string DllPath(NDQ_WINDOWS_SDK_DLL_PATH);
        std::string Path = DllPath + name;
        return LoadLibraryA(Path.c_str());
    }

    void* GetDllExport(HMODULE module, const char* name)
    {
        return (void*)GetProcAddress(module, name);
    }

    void FreeDllHandle(HMODULE module)
    {
        FreeLibrary(module);
    }
}
set(NDQ_IMGUI_CODE
    "imgui/imconfig.h"
    "imgui/imgui.h"
    "imgui/imgui_impl_dx12.h"
    "imgui/imgui_impl_win32.h"
    "imgui/imgui_internal.h"
    "imgui/imstb_rectpack.h"
    "imgui/imstb_textedit.h"
    "imgui/imstb_truetype.h"
    "imgui/imgui.cpp"
    "imgui/imgui_demo.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_impl_dx12.cpp"
    "imgui/imgui_impl_win32.cpp"
    "imgui/imgui_tables.cpp"
    "imgui/imgui_widgets.cpp"
)
add_library(imgui STATIC ${NDQ_IMGUI_CODE})
target_include_directories(imgui PUBLIC ${PROJECT_SOURCE_DIR}/thirdparty/imgui)
set_target_properties(imgui PROPERTIES FOLDER "libraries")

include(FetchContent)
FetchContent_Declare(dxc_bin
    URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2308/dxc_2023_08_14.zip
    URL_HASH SHA256=01d4c4dfa37dee21afe70cac510d63001b6b611a128e3760f168765eead1e625
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dxc
)
FetchContent_GetProperties(dxc_bin)
if(NOT dxc_bin_POPULATED)
    FetchContent_Populate(dxc_bin)
endif()

set(NDQ_DXCOMPILER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/dxc/bin/x64/dxcompiler.dll" CACHE STRING "dxcompiler dll")
set(NDQ_DXCOMPILER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dxc/inc" CACHE STRING "dxcompiler header")
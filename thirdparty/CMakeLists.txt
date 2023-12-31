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
add_library(ndq STATIC)
target_sources(ndq PUBLIC FILE_SET HEADERS FILES
    "predef.h"
)
target_sources(ndq PUBLIC FILE_SET CXX_MODULES FILES
    "ndq.ixx"
    "window.cpp"
    "rhi.cpp"
    "platform.cpp"
    "gui.cpp"
)

target_compile_options(ndq PUBLIC /wd4005 /wd5105)
target_link_libraries(ndq PRIVATE d3d12.lib dxgi.lib dxguid.lib imgui)
set_target_properties(ndq PROPERTIES FOLDER "libraries")
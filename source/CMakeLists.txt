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
    "shader.cpp"
)

# dxcompiler
target_compile_definitions(ndq PRIVATE NDQ_DXCOMPILER_DLL="${NDQ_DXCOMPILER_DLL}")
target_include_directories(ndq PRIVATE NDQ_DXCOMPILER_INCLUDE_DIR)
# imgui
target_link_libraries(ndq PRIVATE imgui)

target_compile_options(ndq PUBLIC /wd4005 /wd5105)

set_target_properties(ndq PROPERTIES FOLDER "libraries")
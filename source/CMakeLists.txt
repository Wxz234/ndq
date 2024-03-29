file(GLOB SHADER_FILE "shaders/*.hlsl")
foreach(SHADER IN LISTS SHADER_FILE)
    set_source_files_properties(${SHADER} PROPERTIES VS_TOOL_OVERRIDE "None")
    source_group("Shader Files" FILES ${SHADER})
endforeach()

add_library(ndq STATIC ${SHADER_FILE})
target_sources(ndq PUBLIC FILE_SET HEADERS FILES
    "predef.h"
    "window.h"
    "asset_proxy.h"
)
target_sources(ndq PUBLIC FILE_SET CXX_MODULES FILES
    "ndq.ixx"
    "window.cpp"
    "rhi.cpp"
    "platform.cpp"
    "gui.cpp"
    "renderer.cpp"
    "scene.cpp"
    "mesh.cpp"
    "image.cpp"
    "render_data.cpp"
    "gltf.cpp"
    "camera.cpp"
    "asset.cpp"
)

target_link_libraries(ndq PRIVATE WindowsApp.lib windowscodecs.lib)
target_link_libraries(ndq PRIVATE imgui)
set_target_properties(ndq PROPERTIES FOLDER "libraries")
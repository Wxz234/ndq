file(GLOB SHADER_FILE "shaders/*.hlsl")
foreach(SHADER IN LISTS SHADER_FILE)
    set_source_files_properties(${SHADER} PROPERTIES VS_TOOL_OVERRIDE "None")
    source_group("Shader Files" FILES ${SHADER})
endforeach()

add_library(ndq STATIC ${SHADER_FILE})
target_sources(ndq PUBLIC FILE_SET HEADERS FILES
    "predef.h"
    "window.h"
)
target_sources(ndq PUBLIC FILE_SET CXX_MODULES FILES
    "ndq.ixx"
    "window.cpp"
    "rhi.cpp"
    "platform.cpp"
    "gui.cpp"
    "renderer.cpp"
    "scene.cpp"
    "entity.cpp"
    "component.cpp"
    "system.cpp"
    "mesh.cpp"
    "image.cpp"
    "smart_ptr.cpp"
    "render_data.cpp"
    "gltf.cpp"
    "camera.cpp"
    "asset.cpp"
)

target_compile_options(ndq PUBLIC /wd4005 /wd5105 /wd26495)
target_link_libraries(ndq PRIVATE WindowsApp.lib)
target_link_libraries(ndq PRIVATE imgui entt stb json tinygltf mikktspace)
set_target_properties(ndq PROPERTIES FOLDER "libraries")
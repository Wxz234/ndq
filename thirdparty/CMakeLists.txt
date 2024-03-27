file(GLOB IMGUI_FILE "imgui/*.cpp" "imgui/*.h")
add_library(imgui STATIC ${IMGUI_FILE})
target_include_directories(imgui PUBLIC "imgui")
set_target_properties(imgui PROPERTIES FOLDER "libraries")
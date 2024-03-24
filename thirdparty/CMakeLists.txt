file(GLOB IMGUI_FILE "imgui/*.cpp" "imgui/*.h")
add_library(imgui STATIC ${IMGUI_FILE})
target_include_directories(imgui PUBLIC "imgui")
set_target_properties(imgui PROPERTIES FOLDER "libraries")

add_library(entt INTERFACE "entt/entt/entt.hpp")
target_include_directories(entt INTERFACE "entt")
set_target_properties(entt PROPERTIES FOLDER "libraries")

add_library(tinygltf INTERFACE "tinygltf/tiny_gltf.h")
target_include_directories(tinygltf INTERFACE "tinygltf")
set_target_properties(tinygltf PROPERTIES FOLDER "libraries")

add_library(json INTERFACE "json/json.hpp")
target_include_directories(json INTERFACE "json")
set_target_properties(json PROPERTIES FOLDER "libraries")

add_library(mikktspace STATIC "mikktspace/mikktspace.h" "mikktspace/mikktspace.c")
target_include_directories(mikktspace PUBLIC "mikktspace")
set_target_properties(mikktspace PROPERTIES FOLDER "libraries")
set_source_files_properties(Vertex.hlsl Pixel.hlsl PROPERTIES VS_TOOL_OVERRIDE "None")

add_executable(Triangle WIN32 "Triangle.cpp" "Vertex.h" "Pixel.h")
target_sources(Triangle PRIVATE 	vertex.hlsl pixel.hlsl)

target_link_libraries(Triangle PRIVATE ndq)

set_target_properties(Triangle PROPERTIES FOLDER "examples")
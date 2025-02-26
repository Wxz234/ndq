set_source_files_properties(vertex.hlsl pixel.hlsl PROPERTIES VS_TOOL_OVERRIDE "None")

add_executable(02-triangle WIN32 "main.cpp" "vertex.h" "pixel.h")
target_sources(02-triangle PRIVATE 	vertex.hlsl pixel.hlsl)

target_link_libraries(02-triangle PRIVATE ndq)

set_target_properties(02-triangle PROPERTIES FOLDER "examples")
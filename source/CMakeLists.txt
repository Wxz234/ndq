file(GLOB_RECURSE NDQ_HEADER CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE NDQ_SOURCE CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/source/*.cpp")
file(GLOB_RECURSE NDQ_INTERNAL_HEADER CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/source/*.h")
source_group("Internal Header Files" FILES ${NDQ_INTERNAL_HEADER})

add_library(ndq STATIC ${NDQ_HEADER} ${NDQ_SOURCE} ${NDQ_INTERNAL_HEADER})
target_include_directories(ndq PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(ndq PUBLIC d3d12.lib dxgi.lib dxguid.lib)
set_target_properties(ndq PROPERTIES FOLDER "libraries")

target_link_libraries(ndq PUBLIC "${PROJECT_SOURCE_DIR}/nuget/packages/Microsoft.Direct3D.D3D12/build/native/Microsoft.Direct3D.D3D12.targets")
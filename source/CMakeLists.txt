file(GLOB_RECURSE NDQ_HEADER "${PROJECT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE NDQ_SOURCE "${PROJECT_SOURCE_DIR}/source/*.cpp")

add_library(ndq STATIC ${NDQ_HEADER} ${NDQ_SOURCE})
target_include_directories(ndq PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(ndq PUBLIC d3d12.lib dxgi.lib dxguid.lib Rpcrt4.lib)
set_target_properties(ndq PROPERTIES FOLDER "libraries")

target_link_libraries(ndq PUBLIC "${PROJECT_SOURCE_DIR}/nuget/packages/Microsoft.Direct3D.DXC/build/native/Microsoft.Direct3D.DXC.targets")
target_link_libraries(ndq PUBLIC "${PROJECT_SOURCE_DIR}/nuget/packages/Microsoft.Direct3D.D3D12/build/native/Microsoft.Direct3D.D3D12.targets")
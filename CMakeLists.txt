cmake_minimum_required(VERSION 3.28)
project(ndq)

if (NOT MSVC)
    message(FATAL_ERROR "Unsupported compiler.Please choose Visual Studio.")
endif()
if(NOT CMAKE_VS_PLATFORM_NAME STREQUAL "x64")
    message("Unsupported architecture.Please choose x64.")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory("thirdparty")
add_subdirectory("source")
add_subdirectory("examples")
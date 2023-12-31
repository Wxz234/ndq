cmake_minimum_required(VERSION 3.28)
project(ndq)

if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Compiler does not support 64-bit builds.")
endif()

if (NOT MSVC)
    message(FATAL_ERROR "Unsupported compiler.")
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
add_subdirectory("thirdparty")
add_subdirectory("source")
add_subdirectory("examples")
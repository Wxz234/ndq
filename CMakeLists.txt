cmake_minimum_required(VERSION 3.28)
project(ndq)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

execute_process(
    COMMAND "prebuild.bat"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

add_subdirectory("source")
add_subdirectory("examples/01-window")
add_subdirectory("examples/02-triangle")
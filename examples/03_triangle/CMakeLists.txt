add_executable(03_triangle WIN32 "main.cpp")
target_link_libraries(03_triangle PRIVATE ndq)
set_target_properties(03_triangle PROPERTIES FOLDER "examples")
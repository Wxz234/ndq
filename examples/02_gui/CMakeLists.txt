add_executable(02_gui WIN32 main.cpp)
target_link_libraries(02_gui PRIVATE ndq)
set_target_properties(02_gui PROPERTIES FOLDER "examples")
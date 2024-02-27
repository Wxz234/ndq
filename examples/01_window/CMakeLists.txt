add_executable(01_window WIN32 main.cpp)
target_link_libraries(01_window PRIVATE ndq)
set_target_properties(01_window PROPERTIES FOLDER "examples")
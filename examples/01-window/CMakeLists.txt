add_executable(01-window WIN32 "main.cpp")
target_link_libraries(01-window PUBLIC ndq)
set_target_properties(01-window PROPERTIES FOLDER "examples")
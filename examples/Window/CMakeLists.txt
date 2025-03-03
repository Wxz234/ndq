add_executable(Window WIN32 "Window.cpp")
target_link_libraries(Window PUBLIC ndq)
set_target_properties(Window PROPERTIES FOLDER "examples")
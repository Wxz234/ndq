add_executable(Mesh WIN32 "Mesh.cpp")
target_link_libraries(Mesh PUBLIC ndq)
set_target_properties(Mesh PROPERTIES FOLDER "examples")
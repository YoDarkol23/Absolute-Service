add_executable(server_app
    src/main.cpp
    src/server_core.cpp
)
target_include_directories(server_app PRIVATE include)
target_link_libraries(server_app PRIVATE common_lib)

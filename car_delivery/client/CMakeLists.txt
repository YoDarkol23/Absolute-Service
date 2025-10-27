add_executable(client_app
    src/main.cpp
    src/client_menu.cpp
)
target_include_directories(client_app PRIVATE include)
target_link_libraries(client_app PRIVATE common_lib)

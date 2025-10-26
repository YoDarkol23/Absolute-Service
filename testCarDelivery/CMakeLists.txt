cmake_minimum_required(VERSION 3.14)
project(car_delivery CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# === Общая библиотека (с реализацией!) ===
add_library(common
    common/utils.cpp
)
target_include_directories(common PUBLIC ${CMAKE_SOURCE_DIR}/common)

# === HTTP Parser библиотека ===
add_library(http_parser
    server/http_parser.cpp
)
target_include_directories(http_parser PUBLIC ${CMAKE_SOURCE_DIR}/server)

# === Сервер ===
add_executable(server
    server/main.cpp
    server/server.cpp
    server/handlers.cpp
)
target_link_libraries(server 
    common 
    http_parser 
    ${Boost_LIBRARIES} 
    pthread
)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/server)

# === Клиент ===
add_executable(client
    client/main.cpp
    client/client.cpp
)
target_link_libraries(client common ${Boost_LIBRARIES} pthread)

# Копируем данные в build/
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)
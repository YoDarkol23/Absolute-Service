cmake_minimum_required(VERSION 3.14)
project(car_delivery CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск Boost
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# === Общая библиотека ===
add_library(common
    common/utils.cpp
)
target_include_directories(common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# === HTTP Parser (отдельная библиотека) ===
add_library(http_parser
    server/http_parser.cpp
)
target_include_directories(http_parser PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/server
)

# === Сервер ===
add_executable(server
    server/main.cpp
    server/server.cpp
    server/server.hpp
    server/handlers.cpp
    server/handlers.hpp
    server/http_parser.cpp
    server/http_parser.hpp
)
target_link_libraries(server 
    common
    http_parser
    ${Boost_LIBRARIES} 
    pthread
)
target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

# === Клиент ===
add_executable(client
    client/main.cpp
    client/client.cpp
)
target_link_libraries(client 
    common 
    ${Boost_LIBRARIES} 
    pthread
)

# Копируем данные в build/
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)

# Для отладки - показываем найденные файлы
message(STATUS "Boost found: ${Boost_FOUND}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
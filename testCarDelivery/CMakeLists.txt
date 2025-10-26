cmake_minimum_required(VERSION 3.14)
project(car_delivery CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

# Все включаем напрямую в исполняемые файлы
add_executable(server
    server/main.cpp
    server/server.cpp
    server/handlers.cpp 
    server/http_parser.cpp
    common/utils.cpp
)

target_link_libraries(server 
    ${Boost_LIBRARIES} 
    pthread
)

target_include_directories(server PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

add_executable(client
    client/main.cpp
    client/client.cpp
    common/utils.cpp
)

target_link_libraries(client 
    ${Boost_LIBRARIES} 
    pthread
)

target_include_directories(client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIRS}
)

# Копируем данные
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)
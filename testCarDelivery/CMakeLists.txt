cmake_minimum_required(VERSION 3.14)
project(car_delivery CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Загрузка nlohmann/json
include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(Boost REQUIRED COMPONENTS system)

# Общая библиотека
add_library(common
    common/utils.cpp
)
target_include_directories(common PUBLIC ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(common nlohmann_json::nlohmann_json ${Boost_LIBRARIES})

# Сервер
add_executable(server
    server/main.cpp
    server/server.cpp
    server/handlers.cpp
)
target_include_directories(server PUBLIC ${CMAKE_SOURCE_DIR}/server ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(server common nlohmann_json::nlohmann_json ${Boost_LIBRARIES} pthread)

# Клиент
add_executable(client
    client/main.cpp
    client/client.cpp
)
target_include_directories(client PUBLIC ${CMAKE_SOURCE_DIR}/client ${CMAKE_SOURCE_DIR}/common)
target_link_libraries(client common nlohmann_json::nlohmann_json ${Boost_LIBRARIES} pthread)

# Копируем данные
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)

message("✅ Проект настроен с nlohmann/json")
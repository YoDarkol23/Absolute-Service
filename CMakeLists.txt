cmake_minimum_required(VERSION 3.16)
project(testCarDelivery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти пакет Boost
find_package(Boost REQUIRED COMPONENTS system thread unit_test_framework)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost libraries not found")
endif()

# --- Определение исходных файлов ---
# Включаем json.hpp в COMMON_SOURCES, если он там находится
set(COMMON_SOURCES
    common/utils.cpp
    # common/json.hpp не нужно добавлять сюда, это заголовочный файл
)

set(SERVER_SOURCES
    server/server.cpp
    server/handlers.cpp
    # Не включаем server/main.cpp, так как он содержит main()
)

set(CLIENT_SOURCES
    client/client.cpp
    # Не включаем client/main.cpp, так как он содержит main()
)

# --- Установка общего пути к заголовкам ---
# Все цели (исполняемые файлы и тесты) будут знать, где искать заголовки из common/
# Это обеспечит доступ к common/utils.hpp и common/json.hpp
set(COMMON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/common)

# --- Определение исполняемых файлов ---
add_executable(server
    server/main.cpp # Только точка входа
    ${SERVER_SOURCES}
    ${COMMON_SOURCES}
)
# Установка путей к заголовкам для сервера
target_include_directories(server PRIVATE ${COMMON_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/server)
target_link_libraries(server
    ${Boost_LIBRARIES}
    pthread
)

add_executable(client
    client/main.cpp # Только точка входа
    ${CLIENT_SOURCES}
    ${COMMON_SOURCES}
)
# Установка путей к заголовкам для клиента
target_include_directories(client PRIVATE ${COMMON_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/client)
target_link_libraries(client
    ${Boost_LIBRARIES}
)

# --- Копирование директории data ---
file(COPY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})

# --- Условная сборка тестов ---
option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing() # Включить CTest

    # Цель для тестов utils
    add_executable(test_utils tests/test_utils.cpp ${COMMON_SOURCES})
    target_include_directories(test_utils PRIVATE ${COMMON_INCLUDE_DIR})
    target_link_libraries(test_utils Boost::unit_test_framework)
    add_test(NAME UtilsTests COMMAND test_utils)

    # Цель для тестов handlers
    add_executable(test_handlers tests/test_handlers.cpp ${SERVER_SOURCES} ${COMMON_SOURCES})
    target_include_directories(test_handlers PRIVATE ${COMMON_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/server)
    target_link_libraries(test_handlers Boost::unit_test_framework)
    add_test(NAME HandlersTests COMMAND test_handlers)

    # Цель для тестов client
    add_executable(test_client tests/test_client.cpp ${CLIENT_SOURCES} ${COMMON_SOURCES})
    target_include_directories(test_client PRIVATE ${COMMON_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/client)
    target_link_libraries(test_client Boost::unit_test_framework)
    add_test(NAME ClientTests COMMAND test_client)

endif()

# --- Управление неиспользуемыми переменными ---
if(BUILD_TESTS)
    # Используем переменную внутри блока, где она применяется
endif()

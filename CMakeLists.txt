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
set(SERVER_SOURCES
    server/server.cpp
    server/handlers.cpp
    # Не включаем server/main.cpp, так как он содержит main()
)

set(COMMON_SOURCES
    common/utils.cpp
)

set(CLIENT_SOURCES
    client/client.cpp
    # Не включаем client/main.cpp, так как он содержит main()
)

# --- Определение исполняемых файлов ---
add_executable(server
    server/main.cpp # Только точка входа
    ${SERVER_SOURCES}
    ${COMMON_SOURCES} # Server может использовать utils
)

add_executable(client
    client/main.cpp # Только точка входа
    ${CLIENT_SOURCES}
    ${COMMON_SOURCES} # Client может использовать utils
)

# --- Настройка линковки ---
target_link_libraries(server
    ${Boost_LIBRARIES}
    pthread # Требуется для сервера
)

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
    target_link_libraries(test_utils Boost::unit_test_framework)
    target_include_directories(test_utils PRIVATE common)
    add_test(NAME UtilsTests COMMAND test_utils)

    # Цель для тестов handlers
    add_executable(test_handlers tests/test_handlers.cpp ${SERVER_SOURCES} ${COMMON_SOURCES})
    target_link_libraries(test_handlers Boost::unit_test_framework)
    target_include_directories(test_handlers PRIVATE common server)
    add_test(NAME HandlersTests COMMAND test_handlers)

    # Цель для тестов client
    add_executable(test_client tests/test_client.cpp ${CLIENT_SOURCES} ${COMMON_SOURCES})
    target_link_libraries(test_client Boost::unit_test_framework)
    target_include_directories(test_client PRIVATE common client)
    add_test(NAME ClientTests COMMAND test_client)

endif()

# --- Управление неиспользуемыми переменными ---
# Убираем предупреждение CMake о неиспользуемой переменной BUILD_TESTS
if(BUILD_TESTS)
    # Используем переменную внутри блока, где она применяется
endif()

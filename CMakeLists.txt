cmake_minimum_required(VERSION 3.14)
project(testCarDelivery CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# Общая библиотека
add_library(common
    common/utils.cpp
)
target_include_directories(common PUBLIC ${CMAKE_SOURCE_DIR}/common)

# Сервер (без admin_menu.cpp)
add_executable(server
    server/main.cpp
    server/server.cpp
    server/handlers.cpp
)
target_link_libraries(server common ${Boost_LIBRARIES} pthread)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR}/common)

# Клиент (с админским меню)
add_executable(client
    client/main.cpp
    client/client.cpp
    client/admin_menu.cpp
)
target_link_libraries(client common ${Boost_LIBRARIES} pthread)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/common)

# Копируем данные
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)

option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
    enable_testing()

    # Найдем Google Test
    find_package(GTest REQUIRED)

    # Тесты для common/utils.*
    add_executable(test_utils
        tests/test_utils.cpp
        common/utils.cpp # Компилируем тестируемый .cpp файл
    )
    target_link_libraries(test_utils GTest::gtest GTest::gtest_main pthread)
    target_include_directories(test_utils PRIVATE common server) # Указываем пути к заголовкам
    add_test(NAME UtilsTests COMMAND test_utils)

    # Тесты для server/handlers.*
    add_executable(test_handlers
        tests/test_handlers.cpp
        server/handlers.cpp
        server/server.cpp # Возможно, нужен для Mock объектов или утилит
        common/utils.cpp  # Зависимость от utils
    )
    target_link_libraries(test_handlers GTest::gtest GTest::gtest_main pthread boost_system)
    target_include_directories(test_handlers PRIVATE common server)
    add_test(NAME HandlerTests COMMAND test_handlers)

    # Тесты для server/server.* (если нужно тестировать логику сервера напрямую)
    # add_executable(test_server_logic tests/test_server_logic.cpp server/server.cpp common/utils.cpp)
    # target_link_libraries(test_server_logic GTest::gtest GTest::gtest_main pthread boost_system)
    # target_include_directories(test_server_logic PRIVATE common server)
    # add_test(NAME ServerLogicTests COMMAND test_server_logic)

endif()

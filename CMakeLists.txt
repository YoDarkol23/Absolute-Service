cmake_minimum_required(VERSION 3.14)
project(testCarDelivery CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск зависимостей
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)

# Поиск GTest
find_package(GTest REQUIRED)

# Общие настройки
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/server
    ${CMAKE_SOURCE_DIR}/client
    ${CMAKE_SOURCE_DIR}/tests
)

# Общая библиотека
add_library(common_lib
    common/utils.cpp
)
target_link_libraries(common_lib ${Boost_LIBRARIES} Threads::Threads)

# Сервер
add_executable(server
    server/main.cpp
    server/server.cpp
    server/handlers.cpp
)
target_link_libraries(server common_lib ${Boost_LIBRARIES} Threads::Threads)

# Клиент
add_executable(client
    client/main.cpp
    client/client.cpp
    client/admin_menu.cpp
)
target_link_libraries(client common_lib ${Boost_LIBRARIES} Threads::Threads)

# Копируем данные
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/data)
file(COPY data/cars.json DESTINATION ${CMAKE_BINARY_DIR}/data)
file(COPY data/cities.json DESTINATION ${CMAKE_BINARY_DIR}/data)
file(COPY data/documents.json DESTINATION ${CMAKE_BINARY_DIR}/data)

# Тесты
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    
    # Тесты для handlers
    add_executable(test_handlers
        tests/test_handlers.cpp
        server/handlers.cpp
        common/utils.cpp
    )
    target_link_libraries(test_handlers
        GTest::gtest
        GTest::gtest_main
        common_lib
        ${Boost_LIBRARIES}
        Threads::Threads
    )
    
    add_test(NAME HandlersTest 
        COMMAND test_handlers
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    # Тесты для utils
    add_executable(test_utils
        tests/test_utils.cpp
        common/utils.cpp
    )
    target_link_libraries(test_utils
        GTest::gtest
        GTest::gtest_main
        common_lib
        ${Boost_LIBRARIES}
        Threads::Threads
    )
    
    add_test(NAME UtilsTest 
        COMMAND test_utils
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
endif()

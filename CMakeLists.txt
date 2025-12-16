cmake_minimum_required(VERSION 3.16)
project(testCarDelivery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Зависимости ---
find_package(Boost REQUIRED COMPONENTS system thread log)
find_package(nlohmann_json REQUIRED) # Убедитесь, что nlohmann_json установлен и найден

# --- Опциональная настройка: включать/выключать тесты ---
option(BUILD_TESTS "Build tests" ON)

# --- Общая библиотека (для клиента, сервера и тестов) ---
add_library(common_lib
    common/utils.hpp
    common/utils.cpp
)
target_include_directories(common_lib PUBLIC common)
target_link_libraries(common_lib PUBLIC ${Boost_LIBRARIES} nlohmann_json::nlohmann_json) # Линкуем json к библиотеке


# --- Сервер ---
add_executable(server
    server/main.cpp
    server/server.hpp
    server/server.cpp
    server/handlers.hpp
    server/handlers.cpp
)
target_link_libraries(server PRIVATE common_lib ${Boost_LIBRARIES} pthread)


# --- Клиент ---
add_executable(client
    client/main.cpp
    client/client.hpp
    client/client.cpp
)
target_link_libraries(client PRIVATE common_lib ${Boost_LIBRARIES} pthread)


# --- Тестирование ---
if(BUILD_TESTS)
    enable_testing() # Включаем тестирование через CTest

    # Загрузка GoogleTest через FetchContent (рекомендуется для автономности)
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9fd7188aa79ea69d93b9d99.zip # Используйте последнюю стабильную версию
    )
    # Настройка для Windows или совместимости (рекомендуется для сборки внутри проекта)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE) # Отключаем GMock, если не используется
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE) # Не устанавливаем GTest глобально
    FetchContent_MakeAvailable(googletest)


    # Цель для исполняемого файла тестов utils
    add_executable(runUtilsTests
        tests/test_utils.cpp
    )
    target_link_libraries(runUtilsTests
        gtest_main
        common_lib # Линкуем тестируемую библиотеку
    )
    add_test(
        NAME RunUtilsTests
        COMMAND runUtilsTests
    )

    # Цель для исполняемого файла тестов handlers
    add_executable(runHandlerTests
        tests/test_handlers.cpp
    )
    target_link_libraries(runHandlerTests
        gtest_main
        common_lib # handlers.cpp использует Car из common
    )
    add_test(
        NAME RunHandlerTests
        COMMAND runHandlerTests
    )

    # Опционально: общая цель для запуска всех тестов
    add_custom_target(test_all
        COMMAND ctest --output-on-failure
        DEPENDS runUtilsTests runHandlerTests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

endif()

# --- Копирование данных ---
configure_file(data/cars.json data/cars.json COPYONLY)
# Добавьте другие файлы из data/, если нужно

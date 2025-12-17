cmake_minimum_required(VERSION 3.16)
project(testCarDelivery)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Укажите путь к заголовкам nlohmann/json
# Этот путь часто бывает /usr/include, где ищет find_package по умолчанию.
# Если json.hpp установлен в стандартное место (/usr/include/nlohmann/json.hpp),
# find_package(nlohmann_json ...) должен найти его.
# Если нет, можно явно указать путь через CMAKE_INCLUDE_PATH или PATHS в find_package.
# find_package(nlohmann_json REQUIRED) # Попробуем сначала без указания пути

# Найти пакет Boost
find_package(Boost REQUIRED COMPONENTS system thread unit_test_framework)
if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost libraries not found")
endif()

# Попробуем найти nlohmann/json
# find_package(nlohmann_json REQUIRED NO_MODULE)
# find_package(nlohmann_json REQUIRED PATHS /usr/include/nlohmann NO_DEFAULT_PATH)
# Иногда работает просто find_package с NO_MODULE, если заголовок в стандартном месте.
# find_package(nlohmann_json REQUIRED NO_MODULE HINTS /usr/include/nlohmann)

# Альтернатива: использовать FetchContent для загрузки nlohmann/json во время сборки
# include(FetchContent)
# FetchContent_Declare(
#   nlohmann_json
#   GIT_REPOSITORY https://github.com/nlohmann/json.git
#   GIT_TAG v3.11.3 # Укажите предпочитаемую версию
# )
# FetchContent_MakeAvailable(nlohmann_json)

# Наиболее надежный способ, если nlohmann/json установлен системно:
# Установите CMAKE_INCLUDE_PATH перед find_package, если он в нестандартном месте.
# find_package с NO_MODULE ищет только заголовочный файл.
find_package(nlohmann_json 3.2.0 REQUIRED NO_MODULE)
# Если find_package выше не работает, попробуйте установить путь:
# set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/include")
# find_package(nlohmann_json 3.2.0 REQUIRED NO_MODULE)

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

# Клиент использует Boost и nlohmann::json
target_link_libraries(client
    ${Boost_LIBRARIES}
    nlohmann_json::nlohmann_json # Линкуем целевой объект nlohmann_json
)

# --- Установка путей к заголовкам ---
# Убедимся, что исполняемые файлы знают, где искать заголовки nlohmann_json
target_include_directories(server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common ${CMAKE_CURRENT_SOURCE_DIR}/server)
target_include_directories(client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/common ${CMAKE_CURRENT_SOURCE_DIR}/client)

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
    target_link_libraries(test_handlers Boost::unit_test_framework nlohmann_json::nlohmann_json) # handlers может использовать json
    target_include_directories(test_handlers PRIVATE common server)
    add_test(NAME HandlersTests COMMAND test_handlers)

    # Цель для тестов client
    add_executable(test_client tests/test_client.cpp ${CLIENT_SOURCES} ${COMMON_SOURCES})
    target_link_libraries(test_client Boost::unit_test_framework nlohmann_json::nlohmann_json) # client тесты могут использовать json
    target_include_directories(test_client PRIVATE common client)
    add_test(NAME ClientTests COMMAND test_client)

endif()

# --- Управление неиспользуемыми переменными ---
# Убираем предупреждение CMake о неиспользуемой переменной BUILD_TESTS
if(BUILD_TESTS)
    # Используем переменную внутри блока, где она применяется
endif()

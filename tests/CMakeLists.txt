# САМЫЙ ПРОСТОЙ РАБОЧИЙ CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(CarDeliveryTests)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Тест для utils
add_executable(test_utils 
    common/test_utils.cpp
)

# 2. Тест для client  
add_executable(test_client
    client/test_client.cpp
)

# Больше ничего не нужно! Никаких find_package, никаких библиотек!
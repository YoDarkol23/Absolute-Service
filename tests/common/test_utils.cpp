#include "catch_amalgamated.hpp"
#include <string>
#include <fstream>

// Простая функция для теста (упрощенная версия)
std::string read_test_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "ERROR";
    }
    return std::string(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

TEST_CASE("Тест 1: Чтение существующего файла") {
    // Создаем временный файл
    std::string filename = "test_file_1.txt";
    std::string content = "Hello, World!";
    
    // Записываем в файл
    std::ofstream out(filename);
    out << content;
    out.close();
    
    // Читаем файл
    std::string result = read_test_file(filename);
    
    // Проверяем
    REQUIRE(result == content);
    
    // Удаляем временный файл
    std::remove(filename.c_str());
}

TEST_CASE("Тест 2: Чтение несуществующего файла") {
    std::string result = read_test_file("non_existent_file_12345.txt");
    REQUIRE(result == "ERROR");
}

TEST_CASE("Тест 3: Простые вычисления") {
    REQUIRE(2 + 2 == 4);
    REQUIRE(10 > 5);
    REQUIRE_FALSE(1 == 2);
}

TEST_CASE("Тест 4: Работа со строками") {
    std::string str = "Car";
    REQUIRE(str.length() == 3);
    REQUIRE(str + " Brand" == "Car Brand");
}

// Главная функция для Catch2
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

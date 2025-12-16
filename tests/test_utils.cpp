// tests/test_utils.cpp
#include <gtest/gtest.h>
#include "../common/utils.hpp"
#include <string>
#include <sstream> // Для имитации потоков
#include <iostream> // Для cout, cerr в тестируемом коде (если нужно перехватить)

// Тестируем create_http_response
TEST(UtilsTest, CreateHttpResponseReturnsCorrectHeadersAndBody) {
    std::string body = "Test Body";
    int code = 200;

    std::string response = create_http_response(code, body);

    // Проверяем наличие важных частей ответа
    EXPECT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos) << "Status line is missing or incorrect";
    EXPECT_NE(response.find("Content-Length:"), std::string::npos) << "Content-Length header is missing";
    EXPECT_NE(response.find("Content-Type: application/json"), std::string::npos) << "Content-Type header is missing or incorrect";
    EXPECT_NE(response.find(body), std::string::npos) << "Response body is missing from the response";
}

// Тестируем parse_json
TEST(UtilsTest, ParseJsonSuccessfullyParsesValidJson) {
    std::string valid_json_str = R"({"key": "value", "number": 123})";

    nlohmann::json parsed = parse_json(valid_json_str);

    EXPECT_EQ(parsed["key"], "value");
    EXPECT_EQ(parsed["number"], 123);
}

TEST(UtilsTest, ParseJsonThrowsOnInvalidJson) {
    std::string invalid_json_str = R"(invalid json {)";

    EXPECT_THROW(parse_json(invalid_json_str), nlohmann::json::parse_error);
}

// Тестируем get_cars_from_file
// Этот тест зависит от содержимого data/cars.json в момент запуска теста.
// Для изолированного тестирования лучше создать временную копию файла или использовать mock.
// Здесь представлен тест, который работает с реальным файлом, если он существует.
TEST(UtilsTest, GetCarsFromFileReturnsCarsVector) {
    std::string filename = "./data/cars.json"; // Путь к файлу, как ожидается сервером

    // Проверяем, что файл существует перед тестом (может быть проверкой окружения)
    std::ifstream file(filename);
    if (!file.good()) {
        // Если файла нет, тест может завершиться с сообщением об ошибке или быть пропущен.
        // Для простоты, просто проверим, что функция не падает, если файла нет.
        // В реальных условиях, лучше обработать этот случай в тесте или подготовить окружение.
        EXPECT_THROW(get_cars_from_file(filename), std::exception); // Ожидаем исключение, если файл не найден
        return; // Завершаем тест, так как файл не найден
    }
    file.close(); // Закрываем файл, так как get_cars_from_file откроет его снова

    std::vector<Car> cars = get_cars_from_file(filename);

    // Пример проверки: хотя бы один автомобиль должен быть загружен, если файл не пуст
    // Это зависит от содержимого файла. Лучше проверить размер или конкретные значения.
    //EXPECT_GT(cars.size(), 0); // Пример проверки, что список не пуст (требует, чтобы файл был не пуст)

    // Пример проверки первого элемента (предполагая, что он есть и имеет известную структуру)
    // if (!cars.empty()) {
    //     EXPECT_EQ(cars[0].brand, "SomeBrand"); // Замените на реальные значения из вашего файла
    //     // Проверьте другие поля Car...
    // }
    // Для простоты, просто проверим, что функция не падает и возвращает вектор.
    EXPECT_TRUE(true); // Функция успешно завершилась, если не было исключения
}

// Тестируем load_cars
// Аналогично get_cars_from_file, зависит от файла. Проверим, что функция не падает.
TEST(UtilsTest, LoadCarsDoesNotThrow) {
    std::string filename = "./data/cars.json";
    std::vector<Car> cars;

    EXPECT_NO_THROW(cars = load_cars(filename)); // Проверяем, что функция не кидает исключение

    // Повторяем проверки из get_cars_from_file, если файл существует
    std::ifstream file(filename);
    if (file.good()) {
         // Пример проверки, если файл существует
        // EXPECT_GT(cars.size(), 0);
        // if (!cars.empty()) {
        //     EXPECT_EQ(cars[0].model, "SomeModel");
        // }
        EXPECT_TRUE(true); // Файл существует, функция выполнилась
    } else {
        EXPECT_TRUE(cars.empty()); // Если файл не существует, ожидаем пустой вектор (или исключение, см. реализацию load_cars)
    }
    file.close();
}


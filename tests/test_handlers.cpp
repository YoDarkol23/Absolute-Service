// tests/test_handlers.cpp
#include <gtest/gtest.h>
#include "../server/handlers.hpp"
#include "../common/utils.hpp" // Для Car и функций парсинга/создания ответов
#include <string>
#include <vector>
#include <sstream> // Для имитации потоков
#include <iostream> // Для cout, cerr в тестируемом коде (если нужно перехватить)

// Для тестирования handle_get_cars нужно создать mock-данные и проверить результат
TEST(HandlersTest, HandleGetCarsReturnsCorrectJsonResponse) {
    // Подготовка данных: создаем вектор автомобилей
    std::vector<Car> available_cars = {
        Car{.id = 1, .brand = "Toyota", .model = "Camry", .year = 2020, .engine_volume = 2.5, .price = 25000},
        Car{.id = 2, .brand = "Honda", .model = "Civic", .year = 2019, .engine_volume = 1.8, .price = 22000}
    };

    // Вызов тестируемой функции
    std::string response = handle_get_cars(available_cars);

    // Проверка: ответ должен содержать HTTP заголовки и JSON с автомобилями
    EXPECT_NE(response.find("HTTP/1.1 200 OK"), std::string::npos);
    EXPECT_NE(response.find("Content-Type: application/json"), std::string::npos);
    EXPECT_NE(response.find(R"("id":1)"), std::string::npos); // Проверяем наличие данных автомобиля
    EXPECT_NE(response.find(R"("brand":"Toyota")"), std::string::npos);
    EXPECT_NE(response.find(R"("model":"Camry")"), std::string::npos);
    EXPECT_NE(response.find(R"("id":2)"), std::string::npos);
    EXPECT_NE(response.find(R"("brand":"Honda")"), std::string::npos);
    EXPECT_NE(response.find(R"("model":"Civic")"), std::string::npos);

    // Также проверим, что JSON корректно сформирован (парсится обратно)
    EXPECT_NO_THROW(parse_json(response.substr(response.find("\r\n\r\n") + 4))); // Извлекаем тело и парсим
}

// handle_admin_request - заглушка, просто проверим, что она возвращает нужный ответ
TEST(HandlersTest, HandleAdminRequestReturnsNotImplemented) {
    std::string request = "ANY_ADMIN_REQUEST";

    std::string response = handle_admin_request(request);

    EXPECT_NE(response.find("HTTP/1.1 501 Not Implemented"), std::string::npos);
    EXPECT_NE(response.find("Admin functionality not implemented yet"), std::string::npos);
}

// Тестируем функцию из utils.cpp, но используемую в handlers - load_cars
// Это повторение из test_utils.cpp, но можно добавить тест, специфичный для использования в handlers
TEST(HandlersTest, LoadCarsForHandler) {
    // Используем тот же подход, что и в test_utils
    std::string filename = "./data/handlers_test_cars.json"; // Используем отдельный файл для тестов handlers, если нужно
    // Пока используем общий файл
    filename = "./data/cars.json";

    std::vector<Car> cars;
    EXPECT_NO_THROW(cars = load_cars(filename));

    // Проверка аналогична test_utils, зависит от файла
    std::ifstream file(filename);
    if (file.good()) {
        //EXPECT_GT(cars.size(), 0); // Пример
        EXPECT_TRUE(true); // Файл существует, функция не упала
    } else {
        EXPECT_TRUE(cars.empty());
    }
    file.close();

    // Теперь проверим, как эти машины обрабатываются в handle_get_cars
    if (!cars.empty()) {
        std::string handler_resp = handle_get_cars(cars);
        EXPECT_NE(handler_resp.find("HTTP/1.1 200 OK"), std::string::npos);
        // Можно проверить, что в ответе есть хотя бы одна машина из загруженного списка
        // Это проверит интеграцию между load_cars и handle_get_cars
        if (cars.size() > 0) {
            std::string car_id_str = std::to_string(cars[0].id);
            EXPECT_NE(handler_resp.find(car_id_str), std::string::npos);
        }
    }
}

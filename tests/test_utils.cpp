#include <gtest/gtest.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "../common/utils.hpp" // Подключаем тестируемые функции
#include <sstream>
#include <string>
#include <stdexcept> // Для обработки исключений

// Вспомогательная функция для симуляции parseConfig, если она не объявлена в utils.hpp
// Эта функция демонстрирует логику, которую можно протестировать.
// В реальности, если parseConfig объявлена в utils.hpp, используйте её напрямую.
// boost::property_tree::ptree parseConfigFromString(const std::string& json_str) {
//     std::istringstream iss(json_str);
//     boost::property_tree::ptree pt;
//     boost::property_tree::read_json(iss, pt); // Может выбросить исключение
//     return pt;
// }

// Тест: Успешный парсинг корректного JSON
TEST(UtilsTest, ParseJsonValidStructure) {
    std::string json_str = R"({
        "cars": [
            {"id": 1, "make": "Toyota", "model": "Camry", "year": 2020},
            {"id": 2, "make": "Honda", "model": "Civic", "year": 2021}
        ]
    })";

    boost::property_tree::ptree pt;
    try {
        std::istringstream iss(json_str);
        boost::property_tree::read_json(iss, pt); // Используем Boost напрямую для демонстрации
    } catch (const boost::property_tree::json_parser::json_parser_error& e) {
        FAIL() << "Failed to parse valid JSON: " << e.what(); // Если парсинг не удался, тест падает
    }

    // Проверяем структуру результата
    EXPECT_TRUE(pt.count("cars") > 0); // Проверяем наличие массива "cars"
    auto cars_array = pt.get_child("cars");
    size_t count = 0;
    for (auto& item : cars_array) {
        ++count;
        EXPECT_TRUE(item.second.count("id") > 0);
        EXPECT_TRUE(item.second.count("make") > 0);
        EXPECT_TRUE(item.second.count("model") > 0);
        EXPECT_TRUE(item.second.count("year") > 0);
    }
    EXPECT_EQ(count, 2); // Проверяем количество элементов
}

// Тест: Парсинг неверного JSON должен выбрасывать исключение
TEST(UtilsTest, ParseJsonInvalidThrowsException) {
    std::string invalid_json_str = R"({ "key": "value", "invalid": } )"; // Неверный JSON

    boost::property_tree::ptree pt;
    EXPECT_THROW(
        {
            std::istringstream iss(invalid_json_str);
            boost::property_tree::read_json(iss, pt); // Эта строка должна выбросить исключение
        },
        boost::property_tree::json_parser::json_parser_error
    );
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

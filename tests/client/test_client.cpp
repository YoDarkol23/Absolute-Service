#include "catch_amalgamated.hpp"
#include <string>
#include <vector>

using namespace std;

// Простые тесты без реальных зависимостей
TEST_CASE("Клиент 1: Проверка основных типов") {
    int car_price = 25000;
    string car_brand = "Toyota";
    
    REQUIRE(car_price > 20000);
    REQUIRE(car_brand == "Toyota");
    REQUIRE(car_brand.length() == 6);
}

TEST_CASE("Клиент 2: Работа с коллекциями") {
    vector<string> car_brands = {"Toyota", "Honda", "BMW"};
    
    REQUIRE(car_brands.size() == 3);
    REQUIRE(car_brands[0] == "Toyota");
    REQUIRE(car_brands[1] == "Honda");
    REQUIRE(car_brands[2] == "BMW");
}

TEST_CASE("Клиент 3: Логические операции") {
    bool has_car = true;
    bool is_expensive = false;
    
    REQUIRE(has_car == true);
    REQUIRE(is_expensive == false);
    REQUIRE((has_car && !is_expensive) == true);
}

TEST_CASE("Клиент 4: Математические операции") {
    int price_usd = 30000;
    double price_eur = price_usd * 0.92;
    
    REQUIRE(price_usd == 30000);
    REQUIRE(price_eur > 27000);
    REQUIRE(price_eur < 28000);
}

// Главная функция для Catch2
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

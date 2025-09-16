#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <optional>
#include <iomanip>
#include <algorithm>
#include <filesystem>
#include "json.hpp"  // nlohmann/json

using json = nlohmann::json;

// Структура автомобиля
struct Car {
    int id;
    std::string brand;
    std::string model;
    int year;
    double engine_volume;
    int auction_price_jpy;
    int shipping_to_vladivostok_jpy;
};

// Глобальные константы
constexpr double JPY_TO_RUB = 0.63;  // курс йены к рублю (пример)
constexpr double EUR_TO_RUB = 90.0;  // курс евро к рублю
constexpr int CURRENT_YEAR = 2024;

// Города: {название, {стоимость_доставки_₽, срок_дней}}
const std::map<std::string, std::pair<int, int>> CITY_DELIVERY = {
    {"Москва", {50000, 15}},
    {"Санкт-Петербург", {55000, 18}},
    {"Екатеринбург", {45000, 12}},
    {"Новосибирск", {48000, 14}},
    {"Казань", {47000, 13}}
};

// Функция чтения автомобилей из JSON
std::optional<std::vector<Car>> loadCars(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        std::cerr << "❌ Файл " << filename << " не найден.\n";
        return std::nullopt;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "❌ Не удалось открыть файл " << filename << ".\n";
        return std::nullopt;
    }

    try {
        json j;
        file >> j;

        std::vector<Car> cars;
        for (const auto& item : j) {
            Car car;
            car.id = item.at("id");
            car.brand = item.at("brand");
            car.model = item.at("model");
            car.year = item.at("year");
            car.engine_volume = item.at("engine_volume");
            car.auction_price_jpy = item.at("auction_price_jpy");
            car.shipping_to_vladivostok_jpy = item.at("shipping_to_vladivostok_jpy");
            cars.push_back(car);
        }
        return cars;
    } catch (const std::exception& e) {
        std::cerr << "❌ Ошибка парсинга JSON: " << e.what() << "\n";
        return std::nullopt;
    }
}

// Расчет таможенной пошлины в рублях
int calculateCustomsDuty(int year, double engine_volume, int invoice_rub) {
    int age = CURRENT_YEAR - year;
    double rate_per_cc = 0.0;
    double min_duty_eur = 0.0;

    if (age > 3) {
        // Старше 3 лет — фиксированная ставка за см³
        if (engine_volume <= 1000) rate_per_cc = 1.5;
        else if (engine_volume <= 1500) rate_per_cc = 1.7;
        else if (engine_volume <= 1800) rate_per_cc = 2.5;
        else if (engine_volume <= 2300) rate_per_cc = 2.7;
        else if (engine_volume <= 3000) rate_per_cc = 3.0;
        else rate_per_cc = 3.6;
        return static_cast<int>(engine_volume * rate_per_cc * EUR_TO_RUB);
    } else {
        // 3 года и младше — 48% от инвойса, но не менее фиксированной суммы
        double duty_percent = invoice_rub * 0.48;
        if (engine_volume <= 1000) min_duty_eur = 2.5;
        else if (engine_volume <= 1500) min_duty_eur = 3.0;
        else if (engine_volume <= 1800) min_duty_eur = 3.2;
        else if (engine_volume <= 2300) min_duty_eur = 3.5;
        else if (engine_volume <= 3000) min_duty_eur = 4.8;
        else min_duty_eur = 5.0;

        double min_duty_rub = engine_volume * min_duty_eur * EUR_TO_RUB;
        return static_cast<int>(std::max(duty_percent, min_duty_rub));
    }
}

// Расчет утилизационного сбора
int calculateUtilizationFee(int year) {
    int age = CURRENT_YEAR - year;
    return (age <= 3) ? 3400 : 5200;
}

// Выбор города
std::optional<std::string> selectCity() {
    std::cout << "\n🚚 Выберите город доставки:\n";
    int i = 1;
    std::vector<std::string> cityList;
    for (const auto& [city, _] : CITY_DELIVERY) {
        std::cout << i++ << ". " << city << "\n";
        cityList.push_back(city);
    }

    int choice;
    std::cout << "\nВведите номер города: ";
    std::cin >> choice;

    if (choice < 1 || choice > static_cast<int>(cityList.size())) {
        std::cerr << "❌ Неверный выбор города.\n";
        return std::nullopt;
    }
    return cityList[choice - 1];
}

// Основная функция
int main() {
    setlocale(LC_ALL, "Russian");  // для Windows, если нужна кириллица

    auto cars = loadCars("cars.json");
    if (!cars.has_value()) {
        return 1;
    }

    if (cars->empty()) {
        std::cout << "📭 Нет доступных автомобилей.\n";
        return 0;
    }

    std::cout << "🚘 Доступные автомобили:\n";
    for (const auto& car : *cars) {
        std::cout << car.id << ". " << car.brand << " " << car.model
                  << " (" << car.year << ", " << car.engine_volume << "L)\n";
    }

    int carId;
    std::cout << "\nВведите номер автомобиля: ";
    std::cin >> carId;

    auto it = std::find_if(cars->begin(), cars->end(),
        [carId](const Car& c) { return c.id == carId; });

    if (it == cars->end()) {
        std::cerr << "❌ Автомобиль с ID " << carId << " не найден.\n";
        return 1;
    }

    Car selectedCar = *it;

    auto selectedCity = selectCity();
    if (!selectedCity.has_value()) {
        return 1;
    }

    auto [deliveryCost, deliveryDays] = CITY_DELIVERY.at(*selectedCity);

    // Конвертация в рубли
    int auction_rub = static_cast<int>(selectedCar.auction_price_jpy * JPY_TO_RUB);
    int shipping_rub = static_cast<int>(selectedCar.shipping_to_vladivostok_jpy * JPY_TO_RUB);
    int invoice_rub = auction_rub + shipping_rub;

    // Расчеты
    int customs = calculateCustomsDuty(selectedCar.year, selectedCar.engine_volume, invoice_rub);
    int utilization = calculateUtilizationFee(selectedCar.year);
    int vat = static_cast<int>((invoice_rub + customs) * 0.20);  // НДС 20%
    int total = invoice_rub + customs + vat + utilization + deliveryCost;

    // Вывод отчета
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "📋 ИТОГОВЫЙ РАСЧЕТ ДОСТАВКИ\n";
    std::cout << std::string(50, '=') << "\n";
    std::cout << "Автомобиль: " << selectedCar.brand << " " << selectedCar.model
              << " (" << selectedCar.year << ", " << selectedCar.engine_volume << "L)\n";
    std::cout << "Город доставки: " << *selectedCity << "\n\n";

    std::cout << std::fixed << std::setprecision(0);
    std::cout << "1. Инвойс (аукцион + доставка до Владивостока): " << invoice_rub << " ₽\n";
    std::cout << "   - Аукцион: " << auction_rub << " ₽\n";
    std::cout << "   - Доставка до Владивостока: " << shipping_rub << " ₽\n";
    std::cout << "2. Таможенная пошлина: " << customs << " ₽\n";
    std::cout << "3. НДС (20%): " << vat << " ₽\n";
    std::cout << "4. Утилизационный сбор: " << utilization << " ₽\n";
    std::cout << "5. Доставка до города: " << deliveryCost << " ₽\n";
    std::cout << "---------------------------\n";
    std::cout << "💰 ИТОГО: " << total << " ₽\n";
    std::cout << "⏱️  Срок доставки: " << deliveryDays << " рабочих дней\n";
    std::cout << std::string(50, '=') << "\n";

    return 0;
}

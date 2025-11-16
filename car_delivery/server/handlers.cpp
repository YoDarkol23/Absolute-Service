#include "handlers.hpp"
#include "../common/utils.hpp"
#include "../common/json.hpp"  // nlohmann/json
#include <iostream>
#include <string>

using json = nlohmann::json;

// Вспомогательная функция: загрузка cars.json как JSON-объект
json load_cars_db() {
    std::string content = read_file("data/cars.json");
    try {
        return json::parse(content);
    } catch (...) {
        std::cerr << "Ошибка парсинга data/cars.json\n";
        return json::array(); // пустой массив
    }
}

// GET /cars
std::string handle_get_cars() {
    json cars = load_cars_db();
    return cars.dump();
}

// POST /search — поиск по JSON-фильтрам с поддержкой >= и <=
std::string handle_post_search(const std::string& body) {
    try {
        std::cout << "POST /search body: " << body << std::endl;

        if (body.empty()) {
            return R"({"error": "Empty request body"})";
        }

        json request = json::parse(body);
        json filters = request.value("filters", json::object());
        json cars = load_cars_db();
        json results = json::array();

        std::cout << "Filters: " << filters.dump() << std::endl;

        for (const auto& car : cars) {
            bool match = true;

            for (auto& [key, filter_value] : filters.items()) {
                if (!car.contains(key)) {
                    match = false;
                    break;
                }

                auto car_value = car[key];

                // Если фильтр - объект с операторами (>=, <=)
                if (filter_value.is_object()) {
                    for (auto& [op, value] : filter_value.items()) {
                        if (op == "$gte") {
                            // Больше или равно >=
                            if (!(car_value >= value)) {
                                match = false;
                                break;
                            }
                        }
                        else if (op == "$lte") {
                            // Меньше или равно <=
                            if (!(car_value <= value)) {
                                match = false;
                                break;
                            }
                        }
                        else {
                            // Неизвестный оператор - считаем как равно
                            if (car_value != value) {
                                match = false;
                                break;
                            }
                        }
                    }
                }
                // Простое сравнение (по умолчанию =)
                else {
                    if (car_value != filter_value) {
                        match = false;
                        break;
                    }
                }

                if (!match) break;
            }

            if (match) {
                results.push_back(car);
            }
        }

        json response;
        response["found"] = results.size();
        response["results"] = results;

        std::cout << "Found " << results.size() << " results" << std::endl;
        return response.dump();

    }
    catch (const std::exception& e) {
        std::cerr << "Search error: " << e.what() << std::endl;
        return R"({"error": ")" + std::string(e.what()) + "\"}";
    }
}

// GET /search?brand=...&model=...
std::string handle_get_search(const std::string& query_string) {
    // Простой парсинг query_string: brand=Toyota&model=Camry
    json filters;
    std::istringstream iss(query_string);
    std::string param;
    while (std::getline(iss, param, '&')) {
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            std::string key = param.substr(0, eq);
            std::string value = param.substr(eq + 1);
            filters[key] = value;
        }
    }

    json cars = load_cars_db();
    json results = json::array();
    for (const auto& car : cars) {
        bool match = true;
        for (auto& [key, value] : filters.items()) {
            if (!car.contains(key) || car[key] != value) {
                match = false;
                break;
            }
        }
        if (match) {
            results.push_back(car);
        }
    }

    json response;
    response["found"] = results.size();
    if (!results.empty()) {
        response["results"] = results;
    }
    return response.dump();
}

// GET /cities
std::string handle_get_cities() {
    // Заглушка: список городов доставки
    json cities = {
        {{"id", 1}, {"name", "Moscow"}, {"delivery_days", 5}, {"delivery_cost", 15000}},
        {{"id", 2}, {"name", "Saint-Petersburg"}, {"delivery_days", 7}, {"delivery_cost", 18000}},
        {{"id", 3}, {"name", "Novosibirsk"}, {"delivery_days", 10}, {"delivery_cost", 25000}},
        {{"id", 4}, {"name", "Yekaterinburg"}, {"delivery_days", 8}, {"delivery_cost", 22000}}
    };
    return cities.dump();
}

// GET /documents
std::string handle_get_documents() {
    json response;
    response["documents"] = {
        {{"category", "purchase"}, {"name", "Contract of sale"}},
        {{"category", "purchase"}, {"name", "Payment confirmation"}},
        {{"category", "registration"}, {"name", "Passport"}},
        {{"category", "registration"}, {"name", "Vehicle registration application"}},
        {{"category", "registration"}, {"name", "Customs clearance certificate"}}
    };
    return response.dump();
}

// GET /delivery
std::string handle_get_delivery() {
    json response;
    response["progress"] = 0;
    response["duration"] = "10-14 days";
    response["cost"] = "From 15,000 RUB";
    response["process"] = {
        {{"step", 1}, {"status", "pending"}, {"description", "Vehicle selection and contract signing"}},
        {{"step", 2}, {"status", "pending"}, {"description", "Payment and document preparation"}},
        {{"step", 3}, {"status", "pending"}, {"description", "Customs clearance"}},
        {{"step", 4}, {"status", "pending"}, {"description", "Transportation to destination city"}},
        {{"step", 5}, {"status", "pending"}, {"description", "Final registration and handover"}}
    };
    return response.dump();
}

// POST /admin/login
std::string handle_post_admin_login(const std::string& body) {
    try {
        json creds = json::parse(body);
        std::string username = creds.value("username", "");
        std::string password = creds.value("password", "");

        // Простая проверка (в реальности — сверка с БД)
        if (username == "admin" && password == "123") {
            json response;
            response["status"] = "success";
            response["user"] = {
                {"username", "admin"},
                {"role", "admin"}
            };
            return response.dump();
        }
        return R"({"error": "Invalid username or password"})";
    } catch (...) {
        return R"({"error": "Malformed login request"})";
    }
}
// Расчет утильсбора с учетом льгот до 160 л.с. и объема меньше 3 литров
double calculate_utilization_fee(double engine_volume, int horsepower, int car_age) {
    double utilization_fee_rub = 0.0;

    // Льготный утильсбор для автомобилей до 160 л.с. И объемом до 3 литров
    if (horsepower <= 160 && engine_volume <= 3.0) {
        if (car_age < 3) {
            // Автомобили до 3 лет - льготный сбор
            utilization_fee_rub = 3400.0; // фиксированная ставка
        }
        else {
            // Автомобили старше 3 лет - льготный сбор
            utilization_fee_rub = 5200.0; // фиксированная ставка
        }
    }
    else {
        // Полный утильсбор для автомобилей свыше 160 л.с. ИЛИ объемом свыше 3 литров
        if (car_age < 3) {
            // Автомобили до 3 лет
            if (engine_volume <= 1.0) {
                utilization_fee_rub = 3400.0;
            }
            else if (engine_volume <= 2.0) {
                utilization_fee_rub = 3400.0;
            }
            else if (engine_volume <= 3.0) {
                utilization_fee_rub = 3400.0;
            }
            else if (engine_volume <= 3.5) {
                utilization_fee_rub = 2153400.0; // 2 153 400 руб.
            }
            else {
                utilization_fee_rub = 2742200.0; // 2 742 200 руб.
            }
        }
        else {
            // Автомобили старше 3 лет
            if (engine_volume <= 1.0) {
                utilization_fee_rub = 5200.0;
            }
            else if (engine_volume <= 2.0) {
                utilization_fee_rub = 5200.0;
            }
            else if (engine_volume <= 3.0) {
                utilization_fee_rub = 5200.0;
            }
            else if (engine_volume <= 3.5) {
                utilization_fee_rub = 3296800.0; // 3 296 800 руб.
            }
            else {
                utilization_fee_rub = 3604800.0; // 3 604 800 руб.
            }
        }
    }

    return utilization_fee_rub;
}

// POST /calculate-delivery - расчёт стоимости доставки
std::string handle_post_calculate_delivery(const std::string& body) {
    try {
        json request = json::parse(body);
        int car_id = request.value("car_id", 0);
        int city_id = request.value("city_id", 0);

        if (car_id == 0 || city_id == 0) {
            return R"({"error": "car_id and city_id are required"})";
        }

        // Загружаем данные
        json cars = load_cars_db();
        json cities_json = json::parse(read_file("data/cities.json"));

        // Находим автомобиль
        json car;
        bool car_found = false;
        for (const auto& c : cars) {
            if (c.value("id", 0) == car_id) {
                car = c;
                car_found = true;
                break;
            }
        }

        if (!car_found) {
            return R"({"error": "Car not found"})";
        }

        // Находим город
        json city;
        bool city_found = false;
        for (const auto& ct : cities_json) {
            if (ct.value("id", 0) == city_id) {
                city = ct;
                city_found = true;
                break;
            }
        }

        if (!city_found) {
            return R"({"error": "City not found"})";
        }

        // Курс валют
        const double USD_TO_RUB = 90.0;
        const double EUR_TO_RUB = 100.0;

        // Данные для расчёта
        double car_price_usd = car.value("price_usd", 0);
        double car_price_eur = car_price_usd * (USD_TO_RUB / EUR_TO_RUB);
        double car_price_rub = car_price_usd * USD_TO_RUB;
        double engine_volume = car.value("engine_volume", 0.0);
        int engine_volume_cm3 = engine_volume * 1000;
        int horsepower = car.value("horsepower", 0);
        int car_year = car.value("year", 0);
        int current_year = 2025;
        int car_age = current_year - car_year;

        // Расчет таможенной пошлины
        double customs_duty_eur = 0.0;
        std::string duty_calculation_method = "";

        if (car_age < 3) {
            duty_calculation_method = "По стоимости (возраст < 3 лет)";

            if (car_price_eur <= 8500) {
                customs_duty_eur = std::max(car_price_eur * 0.54, 2.5 * engine_volume_cm3);
            }
            else if (car_price_eur <= 16700) {
                customs_duty_eur = std::max(car_price_eur * 0.48, 3.5 * engine_volume_cm3);
            }
            else if (car_price_eur <= 42300) {
                customs_duty_eur = std::max(car_price_eur * 0.48, 5.5 * engine_volume_cm3);
            }
            else if (car_price_eur <= 84500) {
                customs_duty_eur = std::max(car_price_eur * 0.48, 7.5 * engine_volume_cm3);
            }
            else if (car_price_eur <= 169000) {
                customs_duty_eur = std::max(car_price_eur * 0.48, 15.0 * engine_volume_cm3);
            }
            else {
                customs_duty_eur = std::max(car_price_eur * 0.48, 20.0 * engine_volume_cm3);
            }
        }
        else {
            duty_calculation_method = "По объему двигателя (возраст ≥ 3 лет)";

            if (car_age >= 3 && car_age <= 5) {
                if (engine_volume_cm3 <= 1000) {
                    customs_duty_eur = 1.5 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 1500) {
                    customs_duty_eur = 1.7 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 1800) {
                    customs_duty_eur = 2.5 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 2300) {
                    customs_duty_eur = 2.7 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 3000) {
                    customs_duty_eur = 3.0 * engine_volume_cm3;
                }
                else {
                    customs_duty_eur = 3.6 * engine_volume_cm3;
                }
            }
            else {
                if (engine_volume_cm3 <= 1000) {
                    customs_duty_eur = 3.0 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 1500) {
                    customs_duty_eur = 3.2 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 1800) {
                    customs_duty_eur = 3.5 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 2300) {
                    customs_duty_eur = 4.8 * engine_volume_cm3;
                }
                else if (engine_volume_cm3 <= 3000) {
                    customs_duty_eur = 5.0 * engine_volume_cm3;
                }
                else {
                    customs_duty_eur = 5.7 * engine_volume_cm3;
                }
            }
        }

        double customs_duty_rub = customs_duty_eur * EUR_TO_RUB;

        // Расчет утильсбора
        double utilization_fee_rub = calculate_utilization_fee(engine_volume, horsepower, car_age);
        std::string utilization_fee_type = (horsepower <= 160 && engine_volume <= 3.0) ? "Льготный (≤160 л.с. и ≤3.0 л)" : "Полный";

        // Фиксированные сборы
        double customs_clearance_rub = 70000.0;
        double broker_fee_rub = 60000.0;

        // Стоимость доставки до города
        double city_delivery_cost_usd = city.value("delivery_cost", 0);
        double city_delivery_cost_rub = city_delivery_cost_usd * USD_TO_RUB;
        int delivery_days = city.value("delivery_days", 0);

        // Итоговый расчёт
        double total_cost_rub = car_price_rub + customs_duty_rub + utilization_fee_rub +
            customs_clearance_rub + broker_fee_rub + city_delivery_cost_rub;
        double total_cost_usd = total_cost_rub / USD_TO_RUB;

        // Формируем ответ
        json response;
        response["car"] = {
            {"id", car_id},
            {"brand", car.value("brand", "")},
            {"model", car.value("model", "")},
            {"year", car_year},
            {"age_years", car_age},
            {"horsepower", horsepower},
            {"engine_volume", engine_volume},
            {"engine_volume_cm3", engine_volume_cm3},
            {"price_usd", car_price_usd},
            {"price_eur", car_price_eur},
            {"price_rub", car_price_rub}
        };

        response["city"] = {
            {"id", city_id},
            {"name", city.value("name", "")},
            {"delivery_days", delivery_days},
            {"delivery_cost_usd", city_delivery_cost_usd},
            {"delivery_cost_rub", city_delivery_cost_rub}
        };

        response["customs_calculation"] = {
            {"method", duty_calculation_method},
            {"duty_eur", customs_duty_eur},
            {"duty_rub", customs_duty_rub},
            {"utilization_fee_type", utilization_fee_type},
            {"utilization_fee_rub", utilization_fee_rub},
            {"customs_clearance_rub", customs_clearance_rub},
            {"broker_fee_rub", broker_fee_rub}
        };

        response["calculation"] = {
            {"car_price_rub", car_price_rub},
            {"customs_duty_rub", customs_duty_rub},
            {"utilization_fee_rub", utilization_fee_rub},
            {"customs_clearance_rub", customs_clearance_rub},
            {"broker_fee_rub", broker_fee_rub},
            {"city_delivery_cost_usd", city_delivery_cost_usd},
            {"city_delivery_cost_rub", city_delivery_cost_rub},
            {"total_cost_rub", total_cost_rub},
            {"total_cost_usd", total_cost_usd}
        };

        response["exchange_rates"] = {
            {"USD_TO_RUB", USD_TO_RUB},
            {"EUR_TO_RUB", EUR_TO_RUB}
        };

        response["summary"] = {
            {"total_cost_rub", total_cost_rub},
            {"total_cost_usd", total_cost_usd},
            {"delivery_days", delivery_days}
        };

        return response.dump();

    }
    catch (const std::exception& e) {
        std::cerr << "Delivery calculation error: " << e.what() << std::endl;
        return R"({"error": "Calculation failed: )" + std::string(e.what()) + "\"}";
    }
}
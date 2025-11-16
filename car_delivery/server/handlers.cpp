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

// POST /search — поиск по JSON-фильтрам
std::string handle_post_search(const std::string& body) {
    try {
        json request = json::parse(body);
        json filters = request.value("filters", json::object());
        json cars = load_cars_db();
        json results = json::array();

        for (const auto& car : cars) {
            bool match = true;
            for (auto& [key, value] : filters.items()) {
                if (!car.contains(key)) {
                    match = false;
                    break;
                }

                // Сравнение значений с учетом типов
                if (value.is_string()) {
                    // Строковое сравнение (регистронезависимое)
                    std::string car_value = car[key].get<std::string>();
                    std::string filter_value = value.get<std::string>();
                    std::transform(car_value.begin(), car_value.end(), car_value.begin(), ::tolower);
                    std::transform(filter_value.begin(), filter_value.end(), filter_value.begin(), ::tolower);

                    if (car_value != filter_value) {
                        match = false;
                        break;
                    }
                }
                else if (value.is_number()) {
                    // Числовое сравнение
                    if (car[key].get<double>() != value.get<double>()) {
                        match = false;
                        break;
                    }
                }
                else {
                    // Другие типы
                    if (car[key] != value) {
                        match = false;
                        break;
                    }
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
        else {
            response["message"] = "No vehicles found matching the specified criteria";
        }
        return response.dump();
    }
    catch (...) {
        return R"({"error": "Invalid search request format"})";
    }
}

// GET /search?brand=...&model=...
std::string handle_get_search(const std::string& query_string) {
    // Парсинг query_string: brand=Toyota&model=Camry&horsepower=150
    json filters;
    std::istringstream iss(query_string);
    std::string param;

    while (std::getline(iss, param, '&')) {
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            std::string key = param.substr(0, eq);
            std::string value = param.substr(eq + 1);

            // Преобразование числовых значений
            if (key == "year" || key == "price_usd" || key == "horsepower" || key == "engine_volume") {
                try {
                    if (value.find('.') != std::string::npos) {
                        // Дробное число (engine_volume)
                        filters[key] = std::stod(value);
                    }
                    else {
                        // Целое число
                        filters[key] = std::stoi(value);
                    }
                }
                catch (...) {
                    // Если не удалось преобразовать, оставляем строкой
                    filters[key] = value;
                }
            }
            else {
                // Строковые значения
                filters[key] = value;
            }
        }
    }

    json cars = load_cars_db();
    json results = json::array();

    for (const auto& car : cars) {
        bool match = true;
        for (auto& [key, value] : filters.items()) {
            if (!car.contains(key)) {
                match = false;
                break;
            }

            // Сравнение значений с учетом типов
            if (value.is_string()) {
                // Строковое сравнение (регистронезависимое)
                std::string car_value = car[key].get<std::string>();
                std::string filter_value = value.get<std::string>();
                std::transform(car_value.begin(), car_value.end(), car_value.begin(), ::tolower);
                std::transform(filter_value.begin(), filter_value.end(), filter_value.begin(), ::tolower);

                if (car_value != filter_value) {
                    match = false;
                    break;
                }
            }
            else if (value.is_number()) {
                // Числовое сравнение
                if (car[key].get<double>() != value.get<double>()) {
                    match = false;
                    break;
                }
            }
            else {
                // Другие типы
                if (car[key] != value) {
                    match = false;
                    break;
                }
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
    else {
        response["message"] = "No vehicles found matching the specified criteria";
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
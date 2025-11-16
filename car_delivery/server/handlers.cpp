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

            for (auto& [key, filter_value] : filters.items()) {
                if (!car.contains(key)) {
                    match = false;
                    break;
                }

                std::string filter_str = filter_value.get<std::string>();
                double car_num = 0;
                bool car_is_number = false;

                // Пробуем преобразовать значение автомобиля в число
                try {
                    if (car[key].is_number()) {
                        car_num = car[key].get<double>();
                        car_is_number = true;
                    }
                    else {
                        car_num = std::stod(car[key].get<std::string>());
                        car_is_number = true;
                    }
                }
                catch (...) {
                    car_is_number = false;
                }

                // Проверяем операторы сравнения
                if (car_is_number && (filter_str.find('>') != std::string::npos ||
                    filter_str.find('<') != std::string::npos ||
                    filter_str.find('=') != std::string::npos)) {

                    double filter_num = 0;
                    try {
                        // Извлекаем число из строки с оператором
                        size_t op_pos = filter_str.find_first_of("><=!");
                        if (op_pos != std::string::npos) {
                            filter_num = std::stod(filter_str.substr(op_pos + 1));
                        }
                        else {
                            filter_num = std::stod(filter_str);
                        }
                    }
                    catch (...) {
                        match = false;
                        break;
                    }

                    // Применяем оператор
                    if (filter_str.find(">=") != std::string::npos) {
                        if (car_num < filter_num) { match = false; break; }
                    }
                    else if (filter_str.find("<=") != std::string::npos) {
                        if (car_num > filter_num) { match = false; break; }
                    }
                    else if (filter_str.find(">") != std::string::npos) {
                        if (car_num <= filter_num) { match = false; break; }
                    }
                    else if (filter_str.find("<") != std::string::npos) {
                        if (car_num >= filter_num) { match = false; break; }
                    }
                    else if (filter_str.find("=") != std::string::npos) {
                        if (car_num != filter_num) { match = false; break; }
                    }
                    else {
                        // Просто число без оператора
                        if (car_num != filter_num) { match = false; break; }
                    }

                }
                else {
                    // Строковое сравнение (точное совпадение)
                    std::string car_value = car[key].get<std::string>();
                    std::transform(car_value.begin(), car_value.end(), car_value.begin(), ::tolower);
                    std::transform(filter_str.begin(), filter_str.end(), filter_str.begin(), ::tolower);

                    if (car_value != filter_str) {
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
    // Парсинг query_string: brand=Toyota&horsepower<=160
    json filters;
    std::istringstream iss(query_string);
    std::string param;

    while (std::getline(iss, param, '&')) {
        size_t eq = param.find('=');
        if (eq != std::string::npos) {
            std::string key = param.substr(0, eq);
            std::string value = param.substr(eq + 1);

            // Просто передаем как есть, обработка будет в основном алгоритме
            filters[key] = value;
        }
    }

    json cars = load_cars_db();
    json results = json::array();

    for (const auto& car : cars) {
        bool match = true;

        for (auto& [key, filter_value] : filters.items()) {
            if (!car.contains(key)) {
                match = false;
                break;
            }

            std::string filter_str = filter_value.get<std::string>();
            double car_num = 0;
            bool car_is_number = false;

            // Пробуем преобразовать значение автомобиля в число
            try {
                if (car[key].is_number()) {
                    car_num = car[key].get<double>();
                    car_is_number = true;
                }
                else {
                    car_num = std::stod(car[key].get<std::string>());
                    car_is_number = true;
                }
            }
            catch (...) {
                car_is_number = false;
            }

            // Проверяем операторы сравнения для числовых полей
            if (car_is_number && (filter_str.find('>') != std::string::npos ||
                filter_str.find('<') != std::string::npos ||
                filter_str.find('=') != std::string::npos)) {

                double filter_num = 0;
                try {
                    // Извлекаем число из строки с оператором
                    size_t op_pos = filter_str.find_first_of("><=!");
                    if (op_pos != std::string::npos) {
                        filter_num = std::stod(filter_str.substr(op_pos + 1));
                    }
                    else {
                        filter_num = std::stod(filter_str);
                    }
                }
                catch (...) {
                    match = false;
                    break;
                }

                // Применяем оператор
                if (filter_str.find(">=") != std::string::npos) {
                    if (car_num < filter_num) { match = false; break; }
                }
                else if (filter_str.find("<=") != std::string::npos) {
                    if (car_num > filter_num) { match = false; break; }
                }
                else if (filter_str.find(">") != std::string::npos) {
                    if (car_num <= filter_num) { match = false; break; }
                }
                else if (filter_str.find("<") != std::string::npos) {
                    if (car_num >= filter_num) { match = false; break; }
                }
                else if (filter_str.find("=") != std::string::npos) {
                    if (car_num != filter_num) { match = false; break; }
                }
                else {
                    // Просто число без оператора
                    if (car_num != filter_num) { match = false; break; }
                }

            }
            else {
                // Строковое сравнение (точное совпадение)
                std::string car_value = car[key].get<std::string>();
                std::transform(car_value.begin(), car_value.end(), car_value.begin(), ::tolower);
                std::transform(filter_str.begin(), filter_str.end(), filter_str.begin(), ::tolower);

                if (car_value != filter_str) {
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
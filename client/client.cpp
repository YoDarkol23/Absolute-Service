#include "client.hpp"
#include "utils.hpp"
#include "json.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

using json = nlohmann::json;

std::string fetch_all_cars(const std::string& host, int port) {
    std::string request = 
        "GET /cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_cars_by_specs(const std::string& specs, const std::string& host, int port) {
    // Если строка пустая, возвращаем все автомобили
    if (specs.empty()) {
        return fetch_all_cars(host, port);
    }

    try {
        std::cout << "Поиск параметров: " << specs << std::endl;

        // Парсим параметры и формируем JSON фильтры с поддержкой >= и <=
        json filters = json::object();
        std::istringstream iss(specs);
        std::string pair;

        while (std::getline(iss, pair, ',')) {
            // Ищем операторы >= и <=
            std::string field, op, value_str;

            if (pair.find(">=") != std::string::npos) {
                size_t op_pos = pair.find(">=");
                field = pair.substr(0, op_pos);
                op = "$gte";
                value_str = pair.substr(op_pos + 2);
            }
            else if (pair.find("<=") != std::string::npos) {
                size_t op_pos = pair.find("<=");
                field = pair.substr(0, op_pos);
                op = "$lte";
                value_str = pair.substr(op_pos + 2);
            }
            else if (pair.find('=') != std::string::npos) {
                size_t op_pos = pair.find('=');
                field = pair.substr(0, op_pos);
                op = "$eq";
                value_str = pair.substr(op_pos + 1);
            }
            else {
                // Пропускаем некорректные параметры
                continue;
            }

            // Преобразуем значение в правильный тип
            json value;
            if (field == "year" || field == "horsepower" || field == "price_usd") {
                try {
                    value = std::stoi(value_str);
                }
                catch (...) {
                    value = value_str;
                }
            }
            else if (field == "engine_volume") {
                try {
                    value = std::stod(value_str);
                }
                catch (...) {
                    value = value_str;
                }
            }
            else {
                value = value_str;
            }

            // Если это оператор по умолчанию, используем простое значение
            if (op == "$eq") {
                filters[field] = value;
            }
            else {
                // Иначе создаем объект с оператором
                filters[field] = json::object();
                filters[field][op] = value;
            }
        }

        // Формируем JSON тело запроса
        json request_body;
        request_body["filters"] = filters;
        std::string body = request_body.dump();

        std::cout << "Тело запроса: " << body << std::endl;

        // Формируем POST запрос
        std::string request =
            "POST /search HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        std::cout << "Отправка POST-запроса..." << std::endl;
        return send_http_request(host, port, request);
    }
    catch (const std::exception& e) {
        std::cerr << "Search error: " << e.what() << std::endl;
        return R"({"error": "Invalid request format: )" + std::string(e.what()) + "\"}";
    }
}

std::string fetch_cars_by_brand_model(const std::string& brand, const std::string& model, const std::string& host, int port) {
    std::string query = "brand=" + brand;
    if (!model.empty()) query += "&model=" + model;
    std::string request = 
        "GET /search?" + query + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_delivery_cities(const std::string& host, int port) {
    std::string request = 
        "GET /cities HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_required_documents(const std::string& host, int port) {
    std::string request = 
        "GET /documents HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_delivery_process(const std::string& host, int port) {
    std::string request = 
        "GET /delivery HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_admin_login(const std::string& username, const std::string& password, const std::string& host, int port) {
    try {
        json body_json = {{"username", username}, {"password", password}};
        std::string body = body_json.dump();
        std::string request = 
            "POST /admin/login HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form login request"})";
    }
}

std::string fetch_delivery_calculation(int car_id, int city_id, const std::string& host, int port) {
    try {
        json body_json = { {"car_id", car_id}, {"city_id", city_id} };
        std::string body = body_json.dump();

        std::string request =
            "POST /calculate-delivery HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        return send_http_request(host, port, request);
    }
    catch (const std::exception& e) {
        return R"({"error": "Failed to form delivery calculation request: )" + std::string(e.what()) + "\"}";
    }
}

void print_car_table(const json& cars) {
    if (cars.empty()) {
        std::cout << "No cars available.\n";
        return;
    }
    std::cout << std::setw(5) << "ID"
        << " | " << std::setw(12) << "Brand"
        << " | " << std::setw(16) << "Model"
        << " | " << std::setw(4) << "Year"
        << " | " << std::setw(8) << "Price,$"
        << " | " << std::setw(6) << "HP"
        << " | " << std::setw(8) << "Steering" << '\n';
    std::cout << std::string(80, '-') << '\n'; 
    for (const auto& car : cars) {
        int id = car.value("id", 0);
        std::string brand = car.value("brand", "");
        std::string model = car.value("model", "");
        int year = car.value("year", 0);
        int price = car.value("price_usd", 0);
        int horsepower = car.value("horsepower", 0);  
        std::string steering = (car.value("steering_wheel", "") == "left") ? "Left" : "Right";
        std::cout << std::setw(5) << id
            << " | " << std::setw(12) << brand
            << " | " << std::setw(16) << model
            << " | " << std::setw(4) << year
            << " | " << std::setw(8) << price
            << " | " << std::setw(6) << horsepower
            << " | " << std::setw(8) << steering << '\n';
    }
    std::cout << std::string(80, '-') << '\n';
}

void print_cities_table(const json& cities) {
    if (cities.empty()) {
        std::cout << "Города доставки недоступны.\n";
        return;
    }
    std::cout << std::setw(5) << "ID"
              << " | " << std::setw(16) << "City"
              << " | " << std::setw(8) << "Days"
              << " | " << std::setw(10) << "Cost,$" << '\n';
    std::cout << std::string(50, '-') << '\n';
    for (const auto& city : cities) {
        std::cout << std::setw(5) << city.value("id", 0)
                  << " | " << std::setw(16) << city.value("name", "")
                  << " | " << std::setw(8) << city.value("delivery_days", 0)
                  << " | " << std::setw(10) << city.value("delivery_cost", 0) << '\n';
    }
    std::cout << std::string(50, '-') << '\n';
}

void print_documents_list(const json& documents) {
    if (!documents.contains("documents") || documents["documents"].empty()) {
        std::cout << "Никаких документов не требуется.\n";
        return;
    }

    std::cout << "\nНеобходимые документы для покупки и регистрации транспортного средства:\n";
    std::cout << std::string(55, '-') << '\n';

    // Сначала документы для покупки (purchase)
    std::cout << "\nPurchase documents (для покупки):\n";
    for (const auto& doc : documents["documents"]) {
        if (doc.value("category", "") == "purchase") {
            std::cout << " - " << doc.value("name", "") << '\n';
        }
    }

    // Затем документы для регистрации (registration)
    std::cout << "\nRegistration documents (для регистрации):\n";
    for (const auto& doc : documents["documents"]) {
        if (doc.value("category", "") == "registration") {
            std::cout << " - " << doc.value("name", "") << '\n';
        }
    }
}

void print_delivery_process(const json& process) {
    if (!process.contains("process") || process["process"].empty()) {
        std::cout << "Информация о процессе доставки недоступна.\n";
        return;
    }
    std::cout << "\nПроцесс доставки транспортного средства:\n";
    std::cout << std::string(40, '-') << '\n';
    if (process.contains("progress")) std::cout << "Прогресс: " << process.value("progress", 0) << "%\n";
    if (process.contains("duration")) std::cout << "Предполагаемая продолжительность доставки: " << process.value("duration", "") << '\n';
    if (process.contains("cost")) std::cout << "Стоимость доставки: " << process.value("cost", "") << '\n';
    std::cout << '\n';
    for (const auto& step : process["process"]) {
        std::string status = step.value("status", "");
        std::string label = (status == "completed") ? "[completed]" :
                           (status == "in_progress") ? "[in progress]" : "[pending]";
        std::cout << "Step " << step.value("step", 0) << " " << label
                  << ": " << step.value("description", "") << '\n';
    }
}

void print_search_results(const json& results) {
    if (results.contains("error")) {
        std::cout << "Ошибка: " << results["error"] << '\n';
        return;
    }
    if (results.contains("message")) std::cout << results["message"] << '\n';
    if (results.contains("found")) {
        int n = results["found"];
        std::cout << "Найдено " << n << " vehicle(s).\n";
        if (n > 0 && results.contains("results")) print_car_table(results["results"]);
        else if (n == 0) std::cout << "Ни одно транспортное средство не соответствует указанным критериям.\n";
    }
}

void print_admin_login_result(const json& result) {
    if (result.contains("error")) {
        std::cout << "Аутентификация не удалась: " << result["error"] << '\n';
        return;
    }
    if (result.value("status", "") == "success") {
        std::cout << "Аутентификация прошла успешно.\n";
        if (result.contains("user")) {
            auto& u = result["user"];
            std::cout << "Пользователь: " << u.value("username", "unknown")
                      << " (" << u.value("role", "user") << ")\n";
        }
    }
}

void print_delivery_calculation(const json& calculation) {
    if (calculation.contains("error")) {
        std::cout << "Ошибка: " << calculation["error"] << '\n';
        return;
    }

    std::cout << "\n========================================\n";
    std::cout << "       РАСЧЁТ СТОИМОСТИ ДОСТАВКИ       \n";
    std::cout << "========================================\n";

    // Информация об автомобиле
    if (calculation.contains("car")) {
        auto& car = calculation["car"];
        std::cout << "АВТОМОБИЛЬ:\n";
        std::cout << "  " << car.value("brand", "") << " " << car.value("model", "")
            << " (" << car.value("year", 0) << " год, "
            << car.value("age_years", 0) << " лет)\n";
        std::cout << "  Объём двигателя: " << car.value("engine_volume", 0.0)
            << " л (" << car.value("engine_volume_cm3", 0) << " см³)\n";
        std::cout << "  Мощность: " << car.value("horsepower", 0) << " л.с.\n";
        std::cout << "  Стоимость: $" << car.value("price_usd", 0)
            << " (€" << std::fixed << std::setprecision(0) << car.value("price_eur", 0)
            << ", " << car.value("price_rub", 0) << " руб.)\n";
    }

    // Информация о городе
    if (calculation.contains("city")) {
        auto& city = calculation["city"];
        std::cout << "Город доставки: " << city.value("name", "") << "\n";
        std::cout << "Срок доставки: " << city.value("delivery_days", 0) << " дней\n";
    }

    // Курсы валют
    if (calculation.contains("exchange_rates")) {
        auto& rates = calculation["exchange_rates"];
        std::cout << "Курсы: USD = " << rates.value("USD_TO_RUB", 0.0)
            << " руб., EUR = " << rates.value("EUR_TO_RUB", 0.0) << " руб.\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "РАСЧЁТ ТАМОЖЕННЫХ ПЛАТЕЖЕЙ:\n";

    if (calculation.contains("customs_calculation")) {
        auto& customs = calculation["customs_calculation"];
        std::cout << "  Метод расчёта пошлины: " << customs.value("method", "") << "\n";
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "  Таможенная пошлина: €" << customs.value("duty_eur", 0.0)
            << " (" << customs.value("duty_rub", 0.0) << " руб.)\n";

        // Вывод утильсбора с информацией о типе
        std::cout << "  Утильсбор (" << calculation["car"].value("horsepower", 0)
            << " л.с., " << calculation["car"].value("engine_volume", 0.0)
            << " л, " << customs.value("utilization_fee_type", "") << "): "
            << customs.value("utilization_fee_rub", 0.0) << " руб.\n";

        std::cout << "  Таможенное оформление: " << customs.value("customs_clearance_rub", 0.0) << " руб.\n";
        std::cout << "  Комиссия брокера: " << customs.value("broker_fee_rub", 0.0) << " руб.\n";
    }

    std::cout << "----------------------------------------\n";
    std::cout << "ОБЩИЙ РАСЧЁТ:\n";

    if (calculation.contains("calculation")) {
        auto& calc = calculation["calculation"];
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "  Стоимость авто: " << calc.value("car_price_rub", 0.0) << " руб.\n";
        std::cout << "  Таможенные платежи: " << calc.value("customs_duty_rub", 0.0)
            << " + " << calc.value("utilization_fee_rub", 0.0)
            << " + " << calc.value("customs_clearance_rub", 0.0)
            << " + " << calc.value("broker_fee_rub", 0.0) << " руб.\n";
        std::cout << "  Доставка до города: $" << calc.value("city_delivery_cost_usd", 0.0)
            << " (" << calc.value("city_delivery_cost_rub", 0.0) << " руб.)\n";
    }

    std::cout << "----------------------------------------\n";

    if (calculation.contains("summary")) {
        auto& summary = calculation["summary"];
        std::cout << std::fixed << std::setprecision(0);
        std::cout << "ИТОГО К ОПЛАТЕ:\n";
        std::cout << "  " << summary.value("total_cost_rub", 0.0) << " руб.\n";
        std::cout << "  ($" << std::fixed << std::setprecision(2)
            << summary.value("total_cost_usd", 0.0) << ")\n";
        std::cout << "  Срок: " << summary.value("delivery_days", 0) << " дней\n";
    }

    std::cout << "========================================\n\n";
}

void display_response(const std::string& response) {
    try {
        std::string json_body = extract_json_from_response(response);
        auto j = json::parse(json_body);

        if (j.is_array() && !j.empty() && j[0].contains("brand")) {
            print_car_table(j);
        }
        else if (j.is_array() && !j.empty() && j[0].contains("name") && j[0].contains("delivery_days")) {
            print_cities_table(j);
        }
        else if (j.contains("documents")) {
            print_documents_list(j);
        }
        else if (j.contains("process")) {
            print_delivery_process(j);
        }
        else if (j.contains("results") || j.contains("found")) {
            print_search_results(j);
        }
        else if (j.contains("status") || j.contains("error")) {
            print_admin_login_result(j);
        }
        else if (j.contains("calculation") && j.contains("summary")) {
            print_delivery_calculation(j); // Добавляем обработку расчёта доставки
        }
        else {
            std::cout << j.dump(2) << '\n';
        }
    }
    catch (const json::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << "\nRaw response:\n" << response << '\n';
    }
}
// === Реализация админских функций ===

std::string fetch_admin_cars(const std::string& host, int port) {
    std::string request =
        "GET /admin/cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_admin_cities(const std::string& host, int port) {
    std::string request =
        "GET /admin/cities HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string fetch_admin_documents(const std::string& host, int port) {
    std::string request =
        "GET /admin/documents HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    return send_http_request(host, port, request);
}

std::string add_admin_car(const std::string& brand, const std::string& model, int year, int price_usd, const std::string& host, int port) {
    try {
        json body_json = {
            {"brand", brand},
            {"model", model},
            {"year", year},
            {"price_usd", price_usd}
        };
        std::string body = body_json.dump();
        std::string request =
            "POST /admin/cars HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form add car request"})";
    }
}

std::string add_admin_city(const std::string& name, int delivery_days, int delivery_cost, const std::string& host, int port) {
    try {
        json body_json = {
            {"name", name},
            {"delivery_days", delivery_days},
            {"delivery_cost", delivery_cost}
        };
        std::string body = body_json.dump();
        std::string request =
            "POST /admin/cities HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form add city request"})";
    }
}

std::string add_admin_document(const std::string& category, const std::string& name, const std::string& host, int port) {
    try {
        json body_json = {
            {"category", category},
            {"name", name}
        };
        std::string body = body_json.dump();
        std::string request =
            "POST /admin/documents HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    }
    catch (...) {
        return R"({"error": "Failed to form add document request"})";
    }
}

std::string update_admin_car(int car_id, const std::string& brand, const std::string& model, int year, int price_usd, const std::string& host, int port) {
    try {
        json body_json = {
            {"brand", brand},
            {"model", model},
            {"year", year},
            {"price_usd", price_usd}
        };
        std::string body = body_json.dump();
        std::string request =
            "PUT /admin/cars/" + std::to_string(car_id) + " HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form update car request"})";
    }
}

std::string update_admin_city(int city_id, const std::string& name, int delivery_days, int delivery_cost, const std::string& host, int port) {
    try {
        json body_json = {
            {"name", name},
            {"delivery_days", delivery_days},
            {"delivery_cost", delivery_cost}
        };
        std::string body = body_json.dump();
        std::string request =
            "PUT /admin/cities/" + std::to_string(city_id) + " HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form update city request"})";
    }
}

std::string delete_admin_car(int car_id, const std::string& host, int port) {
    try {
        std::string request =
            "DELETE /admin/cars/" + std::to_string(car_id) + " HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Connection: close\r\n"
            "\r\n";
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form delete car request"})";
    }
}

std::string delete_admin_city(int city_id, const std::string& host, int port) {
    try {
        std::string request =
            "DELETE /admin/cities/" + std::to_string(city_id) + " HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Connection: close\r\n"
            "\r\n";
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form delete city request"})";
    }
}

std::string delete_admin_document(const std::string& category, const std::string& name, const std::string& host, int port) {
    try {
        json body_json = {
            {"category", category},
            {"name", name}
        };
        std::string body = body_json.dump();
        std::string request =
            "DELETE /admin/documents HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;
        return send_http_request(host, port, request);
    } catch (...) {
        return R"({"error": "Failed to form delete document request"})";
    }
}

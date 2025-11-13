#include "client.hpp"
#include "../common/utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

using json = nlohmann::json;

// ANSI коды цветов
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_BOLD    "\033[1m"

std::string extract_json_from_response(const std::string& http_response) {
    size_t body_start = http_response.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        return http_response.substr(body_start + 4);
    }
    
    body_start = http_response.find("\n\n");
    if (body_start != std::string::npos) {
        return http_response.substr(body_start + 2);
    }
    
    return http_response;
}

// === HTTP-ЗАПРОСЫ ===

std::string fetch_all_cars(const std::string& host, int port) {
    std::string request = 
        "GET /cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

std::string fetch_cars_by_specs(const std::string& specs, const std::string& host, int port) {
    try {
        json filters = json::object();
        std::istringstream iss(specs);
        std::string pair;
        
        while (std::getline(iss, pair, ',')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                filters[key] = value;
            }
        }
        
        json request_body = {{"filters", filters}};
        std::string body = request_body.dump();
        
        std::string request = 
            "POST /search HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        std::string response = send_http_request(host, port, request);
        return extract_json_from_response(response);
        
    } catch (const std::exception& e) {
        return R"({"error": "Ошибка формирования запроса"})";
    }
}

std::string fetch_cars_by_brand_model(const std::string& brand, const std::string& model, const std::string& host, int port) {
    std::string query = "brand=" + brand;
    if (!model.empty()) {
        query += "&model=" + model;
    }
    
    std::string request = 
        "GET /cars/brand?" + query + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

std::string fetch_delivery_cities(const std::string& host, int port) {
    std::string request = 
        "GET /cities HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

std::string fetch_required_documents(const std::string& host, int port) {
    std::string request = 
        "GET /documents HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

std::string fetch_delivery_process(const std::string& host, int port) {
    std::string request = 
        "GET /delivery HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

std::string fetch_admin_login(const std::string& username, const std::string& password, const std::string& host, int port) {
    try {
        json request_body = {
            {"username", username},
            {"password", password}
        };
        std::string body = request_body.dump();
        
        std::string request = 
            "POST /admin/login HTTP/1.1\r\n"
            "Host: " + host + "\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" + body;

        std::string response = send_http_request(host, port, request);
        return extract_json_from_response(response);
        
    } catch (const std::exception& e) {
        return R"({"error": "Ошибка входа"})";
    }
}

// === КРАСИВЫЕ ФУНКЦИИ ВЫВОДА ===

void print_car_table(const json& cars) {
    if (cars.empty()) {
        std::cout << COLOR_YELLOW << "🚗 Автомобили не найдены" << COLOR_RESET << std::endl;
        return;
    }

    std::cout << COLOR_CYAN << "-----------------------------------------------------------------" << COLOR_RESET << std::endl;
    std::cout << COLOR_CYAN << "| ID  | Марка          | Модель           | Год  | Цена,$  | Руль |" << COLOR_RESET << std::endl;
    std::cout << COLOR_CYAN << "|-----|----------------|------------------|------|---------|------|" << COLOR_RESET << std::endl;
    
    for (const auto& car : cars) {
        int id = car.value("id", 0);
        std::string brand = car.value("brand", "");
        std::string model = car.value("model", "");
        int year = car.value("year", 0);
        int price = car.value("price_usd", 0);
        std::string steering = car.value("steering_wheel", "");
        std::string steering_display = (steering == "left") ? "Левый" : "Правый";
        
        std::cout << COLOR_CYAN << "| " << COLOR_RESET
                  << std::setw(3) << id << " "
                  << COLOR_CYAN << "| " << COLOR_RESET
                  << std::setw(14) << std::left << brand << " "
                  << COLOR_CYAN << "| " << COLOR_RESET
                  << std::setw(16) << std::left << model << " "
                  << COLOR_CYAN << "| " << COLOR_RESET
                  << std::setw(4) << year << " "
                  << COLOR_CYAN << "| " << COLOR_GREEN
                  << std::setw(7) << std::right << price << " "
                  << COLOR_CYAN << "| " << COLOR_RESET
                  << std::setw(4) << steering_display << " "
                  << COLOR_CYAN << "|" << COLOR_RESET << std::endl;
    }
    
    std::cout << COLOR_CYAN << "-----------------------------------------------------------------" << COLOR_RESET << std::endl;
}

void print_cities_table(const json& cities) {
    if (cities.empty()) {
        std::cout << COLOR_YELLOW << "🏙️  Города не найдены" << COLOR_RESET << std::endl;
        return;
    }

    std::cout << COLOR_MAGENTA << "---------------------------------------------------" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAGENTA << "| ID  | Город            | Срок       | Стоимость |" << COLOR_RESET << std::endl;
    std::cout << COLOR_MAGENTA << "|-----|------------------|------------|-----------|" << COLOR_RESET << std::endl;
    
    for (const auto& city : cities) {
        int id = city.value("id", 0);
        std::string name = city.value("name", "");
        int days = city.value("delivery_days", 0);
        int cost = city.value("delivery_cost", 0);
        
        std::cout << COLOR_MAGENTA << "| " << COLOR_RESET
                  << std::setw(3) << id << " "
                  << COLOR_MAGENTA << "| " << COLOR_RESET
                  << std::setw(16) << std::left << name << " "
                  << COLOR_MAGENTA << "| " << COLOR_YELLOW
                  << std::setw(10) << std::left << (std::to_string(days) + " дн") << " "
                  << COLOR_MAGENTA << "| " << COLOR_GREEN
                  << std::setw(9) << std::right << (std::to_string(cost) + " $") << " "
                  << COLOR_MAGENTA << "|" << COLOR_RESET << std::endl;
    }
    
    std::cout << COLOR_MAGENTA << "---------------------------------------------------" << COLOR_RESET << std::endl;
}

void print_documents_list(const json& documents) {
    if (!documents.contains("documents") || documents["documents"].empty()) {
        std::cout << COLOR_YELLOW << "📄 Документы не найдены" << COLOR_RESET << std::endl;
        return;
    }

    std::cout << COLOR_BLUE << "\n📋 НЕОБХОДИМЫЕ ДОКУМЕНТЫ ДЛЯ ЗАКАЗА АВТОМОБИЛЯ" << COLOR_RESET << std::endl;
    std::cout << COLOR_BLUE << "==============================================" << COLOR_RESET << std::endl;
    
    std::string current_category = "";
    for (const auto& doc : documents["documents"]) {
        std::string category = doc.value("category", "");
        std::string name = doc.value("name", "");
        
        if (category != current_category) {
            current_category = category;
            if (category == "purchase") {
                std::cout << COLOR_CYAN << "\n📝 ДОКУМЕНТЫ ДЛЯ ПОКУПКИ:" << COLOR_RESET << std::endl;
            } else {
                std::cout << COLOR_CYAN << "\n🏛️  ДОКУМЕНТЫ ДЛЯ РЕГИСТРАЦИИ:" << COLOR_RESET << std::endl;
            }
        }
        
        std::cout << "  • " << name << std::endl;
    }
}

void print_delivery_process(const json& process) {
    if (!process.contains("process") || process["process"].empty()) {
        std::cout << COLOR_YELLOW << "🚚 Информация о доставке не найдена" << COLOR_RESET << std::endl;
        return;
    }

    std::cout << COLOR_GREEN << "\n🚚 ПРОЦЕСС ДОСТАВКИ АВТОМОБИЛЯ" << COLOR_RESET << std::endl;
    std::cout << COLOR_GREEN << "================================" << COLOR_RESET << std::endl;
    
    if (process.contains("progress")) {
        std::cout << "📊 Прогресс: " << COLOR_YELLOW << process.value("progress", 0) << "%" << COLOR_RESET << std::endl;
    }
    if (process.contains("duration")) {
        std::cout << "⏱️  Срок доставки: " << COLOR_YELLOW << process.value("duration", "") << COLOR_RESET << std::endl;
    }
    if (process.contains("cost")) {
        std::cout << "💰 Стоимость доставки: " << COLOR_GREEN << process.value("cost", "") << COLOR_RESET << std::endl;
    }
    
    std::cout << std::endl;

    for (const auto& step : process["process"]) {
        std::string status = step.value("status", "");
        std::string status_icon;
        std::string status_color;
        
        if (status == "completed") {
            status_icon = "✅";
            status_color = COLOR_GREEN;
        } else if (status == "in_progress") {
            status_icon = "🔄";
            status_color = COLOR_YELLOW;
        } else {
            status_icon = "⏳";
            status_color = COLOR_BLUE;
        }
        
        std::cout << status_color << status_icon << " Этап " << step.value("step", 0) << ": " 
                  << COLOR_RESET << step.value("description", "") << std::endl;
    }
}

void print_search_results(const json& results) {
    if (results.contains("error")) {
        std::cout << COLOR_RED << "❌ Ошибка: " << results["error"] << COLOR_RESET << std::endl;
        return;
    }
    
    if (results.contains("message")) {
        std::cout << COLOR_BLUE << "\n🔍 " << results["message"] << COLOR_RESET << std::endl;
    }
    
    if (results.contains("found")) {
        int found = results["found"];
        if (found > 0) {
            std::cout << COLOR_GREEN << "✅ Найдено автомобилей: " << found << COLOR_RESET << std::endl;
            if (results.contains("results")) {
                print_car_table(results["results"]);
            }
        } else {
            std::cout << COLOR_YELLOW << "😔 По вашему запросу ничего не найдено" << COLOR_RESET << std::endl;
        }
    }
}

void print_admin_login_result(const json& result) {
    if (result.contains("error")) {
        std::cout << COLOR_RED << "❌ Ошибка входа: " << result["error"] << COLOR_RESET << std::endl;
        return;
    }
    
    if (result.contains("status") && result["status"] == "success") {
        std::cout << COLOR_GREEN << "✅ " << result.value("message", "Вход выполнен успешно") << COLOR_RESET << std::endl;
        if (result.contains("user")) {
            std::cout << COLOR_CYAN << "👤 Пользователь: " << result["user"]["username"] 
                      << " (" << result["user"]["role"] << ")" << COLOR_RESET << std::endl;
        }
    }
}

// === ОСНОВНЫЕ ФУНКЦИИ ИНТЕРФЕЙСА ===

void display_main_menu() {
    std::cout << COLOR_BOLD << COLOR_CYAN << "\n"
        << "========================================\n"
        << "        🚗 СИСТЕМА ДОСТАВКИ АВТО       \n"
        << "        «АБСОЛЮТ СЕРВИС»               \n"
        << "========================================\n"
        << COLOR_RESET << COLOR_WHITE 
        << "1. 📊 Список автомобилей в наличии\n"
        << "2. 🔍 Найти автомобиль по характеристикам\n"
        << "3. 🏷️  Найти автомобиль по марке и модели\n"
        << "4. 🏙️  Города доставки\n"
        << "5. 📄 Необходимые документы\n"
        << "6. 🚚 Процесс доставки\n"
        << "7. 🔐 Вход для администратора\n"
        << "0. 🚪 Выход\n"
        << COLOR_CYAN << "========================================\n"
        << COLOR_RESET;
    std::cout << COLOR_YELLOW << "Выберите опцию: " << COLOR_RESET;
}

void handle_user_choice(int choice) {
    const std::string host = "127.0.0.1";
    const int client_port = 8080;
    const int admin_port = 8081;
    
    std::string response;
    
    switch (choice) {
        case 1:
            std::cout << COLOR_BLUE << "\n📊 ЗАГРУЗКА СПИСКА АВТОМОБИЛЕЙ..." << COLOR_RESET << std::endl;
            response = fetch_all_cars(host, client_port);
            display_response(response);
            break;
            
        case 2: {
            std::cout << COLOR_BLUE << "\n🔍 ПОИСК ПО ХАРАКТЕРИСТИКАМ" << COLOR_RESET << std::endl;
            std::cout << "Примеры фильтров:\n";
            std::cout << "  brand=Toyota - поиск по марке\n";
            std::cout << "  year=2020 - поиск по году\n";
            std::cout << "  price_usd<=30000 - цена до 30000$\n";
            std::cout << "  engine_volume=2.0 - объем двигателя\n\n";
            std::cout << "Введите характеристики: ";
            std::string specs;
            std::getline(std::cin, specs);
            response = fetch_cars_by_specs(specs, host, client_port);
            display_response(response);
            break;
        }
            
        case 3: {
            std::cout << COLOR_BLUE << "\n🏷️  ПОИСК ПО МАРКЕ И МОДЕЛИ" << COLOR_RESET << std::endl;
            std::string brand, model;
            std::cout << "Введите марку: ";
            std::getline(std::cin, brand);
            std::cout << "Введите модель: ";
            std::getline(std::cin, model);
            response = fetch_cars_by_brand_model(brand, model, host, client_port);
            display_response(response);
            break;
        }
            
        case 4:
            std::cout << COLOR_BLUE << "\n🏙️  ЗАГРУЗКА СПИСКА ГОРОДОВ..." << COLOR_RESET << std::endl;
            response = fetch_delivery_cities(host, client_port);
            display_response(response);
            break;
            
        case 5:
            std::cout << COLOR_BLUE << "\n📄 ЗАГРУЗКА ДОКУМЕНТОВ..." << COLOR_RESET << std::endl;
            response = fetch_required_documents(host, client_port);
            display_response(response);
            break;
            
        case 6:
            std::cout << COLOR_BLUE << "\n🚚 ЗАГРУЗКА ИНФОРМАЦИИ О ДОСТАВКЕ..." << COLOR_RESET << std::endl;
            response = fetch_delivery_process(host, client_port);
            display_response(response);
            break;
            
        case 7: {
            std::cout << COLOR_BLUE << "\n🔐 ВХОД В СИСТЕМУ АДМИНИСТРАТОРА" << COLOR_RESET << std::endl;
            std::string username, password;
            std::cout << "Логин: ";
            std::getline(std::cin, username);
            std::cout << "Пароль: ";
            std::getline(std::cin, password);
            response = fetch_admin_login(username, password, host, admin_port);
            display_response(response);
            break;
        }
            
        case 0:
            std::cout << COLOR_GREEN << "\n👋 Спасибо за использование нашей системы!" << COLOR_RESET << std::endl;
            break;
            
        default:
            std::cout << COLOR_RED << "❌ Неверный выбор! Попробуйте снова." << COLOR_RESET << std::endl;
            break;
    }
}

void display_response(const std::string& response) {
    try {
        json j = json::parse(response);
        
        // Определяем тип ответа и используем соответствующий форматтер
        if (j.is_array() && !j.empty() && j[0].contains("brand")) {
            // Это список автомобилей
            print_car_table(j);
        } 
        else if (j.is_array() && !j.empty() && j[0].contains("name")) {
            // Это список городов
            print_cities_table(j);
        }
        else if (j.contains("documents")) {
            // Это документы
            print_documents_list(j);
        }
        else if (j.contains("process")) {
            // Это процесс доставки
            print_delivery_process(j);
        }
        else if (j.contains("results") || j.contains("found")) {
            // Это результаты поиска
            print_search_results(j);
        }
        else if (j.contains("status") || j.contains("error")) {
            // Это результат входа администратора
            print_admin_login_result(j);
        }
        else {
            // По умолчанию - красивый JSON
            std::cout << j.dump(4) << std::endl;
        }
        
    } catch (const std::exception& e) {
        // Если не JSON, выводим как есть
        std::cout << response << std::endl;
    }
}
#include "client.hpp"
#include "../common/utils.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <sstream>

using json = nlohmann::json;

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

// === HTTP-запросы ===
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
        std::cerr << "❌ Ошибка формирования запроса: " << e.what() << std::endl;
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
        std::cerr << "❌ Ошибка входа: " << e.what() << std::endl;
        return R"({"error": "Ошибка входа"})";
    }
}

// === Интерфейс пользователя ===
void display_main_menu() {
    std::cout << "\n=== СИСТЕМА РАСЧЕТА ДОСТАВКИ АВТО ===\n";
    std::cout << "1. Список автомобилей в наличии\n";
    std::cout << "2. Найти автомобиль по характеристикам\n";
    std::cout << "3. Найти автомобиль по марке и модели\n";
    std::cout << "4. Города доставки\n";
    std::cout << "5. Необходимые документы\n";
    std::cout << "6. Процесс доставки\n";
    std::cout << "7. Вход для администратора\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите опцию: ";
}

void handle_user_choice(int choice) {
    const std::string host = "127.0.0.1";
    const int client_port = 8080;
    const int admin_port = 8081;
    
    std::string response;
    
    switch (choice) {
        case 1:
            std::cout << "\n--- Все автомобили ---\n";
            response = fetch_all_cars(host, client_port);
            display_response(response);
            break;
            
        case 2: {
            std::cout << "\n--- Поиск по характеристикам ---\n";
            std::cout << "Введите характеристики (brand=Toyota,year=2020,price_usd<=30000): ";
            std::string specs;
            std::getline(std::cin, specs);
            response = fetch_cars_by_specs(specs, host, client_port);
            display_response(response);
            break;
        }
            
        case 3: {
            std::cout << "\n--- Поиск по марке и модели ---\n";
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
            std::cout << "\n--- Города доставки ---\n";
            response = fetch_delivery_cities(host, client_port);
            display_response(response);
            break;
            
        case 5:
            std::cout << "\n--- Документы для заказа ---\n";
            response = fetch_required_documents(host, client_port);
            display_response(response);
            break;
            
        case 6:
            std::cout << "\n--- Процесс доставки ---\n";
            response = fetch_delivery_process(host, client_port);
            display_response(response);
            break;
            
        case 7: {
            std::cout << "\n--- Вход администратора ---\n";
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
            std::cout << "Выход из программы.\n";
            break;
            
        default:
            std::cout << "Неверный выбор!\n";
            break;
    }
}

void display_response(const std::string& response) {
    try {
        json j = json::parse(response);
        std::cout << j.dump(4) << std::endl; // Красивое форматирование с отступами
    } catch (const std::exception& e) {
        // Если не JSON, выводим как есть
        std::cout << response << std::endl;
    }
}
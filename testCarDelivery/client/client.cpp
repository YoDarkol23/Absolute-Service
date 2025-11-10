#include "client.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include <string>
#include <sstream>

// Вспомогательная функция для извлечения JSON из HTTP-ответа
std::string extract_json_from_response(const std::string& http_response) {
    // Ищем начало тела ответа (после \r\n\r\n)
    size_t body_start = http_response.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        return http_response.substr(body_start + 4);
    }
    
    // Если нет двойного перевода строки, ищем просто после заголовков
    body_start = http_response.find("\n\n");
    if (body_start != std::string::npos) {
        return http_response.substr(body_start + 2);
    }
    
    // Если не нашли разделитель, возвращаем как есть
    return http_response;
}

/**
 * Отправляет запрос на получение списка всех автомобилей.
 */
std::string fetch_all_cars(const std::string& host, int port) {
    std::string request = 
        "GET /cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

/**
 * Отправляет запрос на поиск автомобилей по характеристикам.
 */
std::string fetch_cars_by_specs(const std::string& specs, const std::string& host, int port) {
    std::string request = 
        "GET /cars/specs?" + specs + " HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

/**
 * Отправляет запрос на поиск автомобилей по марке и модели.
 */
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

/**
 * Отправляет запрос на получение списка городов доставки.
 */
std::string fetch_delivery_cities(const std::string& host, int port) {
    std::string request = 
        "GET /delivery/cities HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

/**
 * Отправляет запрос на получение информации о необходимых документах.
 */
std::string fetch_required_documents(const std::string& host, int port) {
    std::string request = 
        "GET /documents HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

/**
 * Отправляет запрос на получение информации о процессе доставки.
 */
std::string fetch_delivery_process(const std::string& host, int port) {
    std::string request = 
        "GET /delivery/process HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

/**
 * Отправляет запрос на авторизацию администратора.
 */
std::string fetch_admin_login(const std::string& username, const std::string& password, const std::string& host, int port) {
    std::string body = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
    
    std::string request = 
        "POST /admin/login HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

// === Реализации функций интерфейса ===

/**
 * Отображает главное меню приложения.
 */
void display_main_menu() {
    std::cout << "\n=== Главное меню ===\n";
    std::cout << "1. Показать все автомобили\n";
    std::cout << "2. Поиск по характеристикам\n";
    std::cout << "3. Поиск по марке и модели\n";
    std::cout << "4. Города доставки\n";
    std::cout << "5. Необходимые документы\n";
    std::cout << "6. Процесс доставки\n";
    std::cout << "7. Вход для администратора\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите опцию: ";
}

/**
 * Обрабатывает выбор пользователя.
 */
void handle_user_choice(int choice) {
    const std::string host = "127.0.0.1";
    const int port = 8080;
    
    std::string response;
    
    switch (choice) {
        case 1:
            std::cout << "\n--- Все автомобили ---\n";
            response = fetch_all_cars(host, port);
            display_response(response);
            break;
            
        case 2: {
            std::cout << "\n--- Поиск по характеристикам ---\n";
            std::string specs;
            std::cout << "Введите характеристики (например: year=2020&fuel_type=petrol): ";
            std::getline(std::cin, specs);
            response = fetch_cars_by_specs(specs, host, port);
            display_response(response);
            break;
        }
            
        case 3: {
            std::cout << "\n--- Поиск по марке и модели ---\n";
            std::string brand, model;
            std::cout << "Введите марку: ";
            std::getline(std::cin, brand);
            std::cout << "Введите модель (опционально): ";
            std::getline(std::cin, model);
            response = fetch_cars_by_brand_model(brand, model, host, port);
            display_response(response);
            break;
        }
            
        case 4:
            std::cout << "\n--- Города доставки ---\n";
            response = fetch_delivery_cities(host, port);
            display_response(response);
            break;
            
        case 5:
            std::cout << "\n--- Необходимые документы ---\n";
            response = fetch_required_documents(host, port);
            display_response(response);
            break;
            
        case 6:
            std::cout << "\n--- Процесс доставки ---\n";
            response = fetch_delivery_process(host, port);
            display_response(response);
            break;
            
        case 7: {
            std::cout << "\n--- Вход для администратора ---\n";
            std::string username, password;
            std::cout << "Логин: ";
            std::getline(std::cin, username);
            std::cout << "Пароль: ";
            std::getline(std::cin, password);
            response = fetch_admin_login(username, password, host, 8081); // Админ порт 8081
            display_response(response);
            break;
        }
            
        case 0:
            std::cout << "Выход из программы...\n";
            break;
            
        default:
            std::cout << "Неверный выбор! Попробуйте снова.\n";
            break;
    }
}

/**
 * Отображает ответ от сервера в удобном формате.
 */
void display_response(const std::string& response) {
    std::cout << "\n--- Ответ сервера ---\n";
    
    // Пытаемся красиво отформатировать JSON
    int indent = 0;
    bool in_string = false;
    
    for (char c : response) {
        if (c == '\"') in_string = !in_string;
        
        if (!in_string) {
            if (c == '{' || c == '[') {
                std::cout << c << '\n';
                indent += 2;
                std::cout << std::string(indent, ' ');
                continue;
            } else if (c == '}' || c == ']') {
                std::cout << '\n';
                indent -= 2;
                std::cout << std::string(indent, ' ') << c;
                continue;
            } else if (c == ',') {
                std::cout << c << '\n';
                std::cout << std::string(indent, ' ');
                continue;
            } else if (c == ':') {
                std::cout << c << ' ';
                continue;
            }
        }
        
        std::cout << c;
    }
    
    std::cout << "\n----------------------\n";
}
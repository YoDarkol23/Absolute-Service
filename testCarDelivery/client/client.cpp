#include "client.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Вспомогательная функция для извлечения JSON из HTTP-ответа
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

// Функции для работы с сервером
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
    // Формируем JSON для поиска по характеристикам
    std::string body = "{\"filters\": {";
    
    std::istringstream iss(specs);
    std::string pair;
    bool first = true;
    
    while (std::getline(iss, pair, ',')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            
            if (!first) body += ",";
            body += "\"" + key + "\": \"" + value + "\"";
            first = false;
        }
    }
    
    body += "}}";
    
    std::string request = 
        "POST /search HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n" + body;

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
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

// Функции администратора
std::string admin_fetch_cars(const std::string& host, int port) {
    std::string request = 
        "GET /admin/cars HTTP/1.1\r\n"
        "Host: " + host + "\r\n"
        "Connection: close\r\n"
        "\r\n";

    std::string response = send_http_request(host, port, request);
    return extract_json_from_response(response);
}

// Вспомогательные функции для интерфейса
void display_car_selection_menu() {
    std::cout << "\n=== Выбор автомобиля и города доставки ===\n";
    std::cout << "1. Выберите номер автомобиля из списка\n";
    std::cout << "2. Выберите город доставки из списка\n";
    std::cout << "3. Рассчитать итоговую стоимость\n";
    std::cout << "0. Вернуться в главное меню\n";
    std::cout << "Выберите опцию: ";
}

void display_admin_menu() {
    std::cout << "\n=== Панель администратора ===\n";
    std::cout << "1. Просмотр полной базы автомобилей\n";
    std::cout << "2. Добавить новый автомобиль\n";
    std::cout << "3. Редактировать автомобиль\n";
    std::cout << "4. Удалить автомобиль\n";
    std::cout << "5. Просмотр статистики\n";
    std::cout << "0. Выйти из режима администратора\n";
    std::cout << "Выберите опцию: ";
}

void handle_admin_choice(int choice, const std::string& host, int port) {
    std::string response;
    
    switch (choice) {
        case 1:
            std::cout << "\n--- Полная база автомобилей ---\n";
            response = admin_fetch_cars(host, port);
            display_response(response);
            break;
            
        case 2:
            std::cout << "\n--- Добавление автомобиля ---\n";
            std::cout << "Функция добавления автомобиля в разработке...\n";
            break;
            
        case 3:
            std::cout << "\n--- Редактирование автомобиля ---\n";
            std::cout << "Функция редактирования автомобиля в разработке...\n";
            break;
            
        case 4:
            std::cout << "\n--- Удаление автомобиля ---\n";
            std::cout << "Функция удаления автомобиля в разработке...\n";
            break;
            
        case 5:
            std::cout << "\n--- Статистика системы ---\n";
            std::cout << "Количество автомобилей: 20\n";
            std::cout << "Города доставки: 10\n";
            std::cout << "Активные заказы: 5\n";
            break;
            
        case 0:
            std::cout << "Выход из режима администратора...\n";
            break;
            
        default:
            std::cout << "Неверный выбор! Попробуйте снова.\n";
            break;
    }
}

// Основные функции интерфейса
void display_main_menu() {
    std::cout << "\n=== Главное меню ===\n";
    std::cout << "1. Список автомобилей в наличии\n";
    std::cout << "2. Найти автомобиль по характеристикам\n";
    std::cout << "3. Найти автомобиль по марке и модели\n";
    std::cout << "4. Города, в которые возможна доставка\n";
    std::cout << "5. Документы, требующиеся для заказа автомобиля\n";
    std::cout << "6. Информация о процессе доставки\n";
    std::cout << "7. Вход администратора\n";
    std::cout << "0. Выход\n";
    std::cout << "Выберите опцию: ";
}

void handle_user_choice(int choice) {
    const std::string host = "127.0.0.1";
    const int client_port = 8080;
    const int admin_port = 8081;
    
    std::string response;
    
    switch (choice) {
        case 1: {
            std::cout << "\n--- Список автомобилей в наличии ---\n";
            response = fetch_all_cars(host, client_port);
            display_response(response);
            
            // Предлагаем выбрать автомобиль для расчета доставки
            std::cout << "\nХотите выбрать автомобиль для расчета доставки? (y/n): ";
            char answer;
            std::cin >> answer;
            std::cin.ignore();
            
            if (answer == 'y' || answer == 'Y') {
                display_car_selection_menu();
                int sub_choice;
                std::cin >> sub_choice;
                std::cin.ignore();
                
                if (sub_choice == 1) {
                    std::cout << "Введите номер автомобиля: ";
                    int car_id;
                    std::cin >> car_id;
                    std::cin.ignore();
                    std::cout << "Вы выбрали автомобиль с ID: " << car_id << std::endl;
                } else if (sub_choice == 2) {
                    std::cout << "Введите номер города доставки: ";
                    int city_id;
                    std::cin >> city_id;
                    std::cin.ignore();
                    std::cout << "Вы выбрали город с ID: " << city_id << std::endl;
                }
            }
            break;
        }
            
        case 2: {
            std::cout << "\n--- Поиск по характеристикам ---\n";
            std::cout << "Введите характеристики в формате: год=2020,цена<=30000,объем>2.0\n";
            std::cout << "Доступные фильтры: год, цена, объем_двигателя, марка, модель, тип_топлива\n";
            std::cout << "Пример: год>=2020,цена<=25000,объем>2.0\n";
            std::string specs;
            std::cout << "Характеристики: ";
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
            std::cout << "Введите модель (опционально): ";
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
            std::cout << "\n--- Вход для администратора ---\n";
            std::string username, password;
            std::cout << "Логин: ";
            std::getline(std::cin, username);
            std::cout << "Пароль: ";
            std::getline(std::cin, password);
            response = fetch_admin_login(username, password, host, admin_port);
            display_response(response);
            
            // Если вход успешен, показываем админ-меню
            if (response.find("\"status\":\"success\"") != std::string::npos) {
                int admin_choice;
                do {
                    display_admin_menu();
                    std::cin >> admin_choice;
                    std::cin.ignore();
                    handle_admin_choice(admin_choice, host, admin_port);
                } while (admin_choice != 0);
            }
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

void display_response(const std::string& response) {
    std::cout << "\n--- Ответ сервера ---\n";
    
    // Простое форматирование JSON
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
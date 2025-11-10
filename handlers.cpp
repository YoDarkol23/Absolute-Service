/**
 * @file server/handlers.cpp
 * @brief Реализация бизнес-логики сервера.
 * 
 * Содержит обработчики для всех эндпоинтов клиента.
 * Все данные читаются из папки ./data/
 */

#include "handlers.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

// Вспомогательная функция для парсинга query-параметров
std::unordered_map<std::string, std::string> parse_query(const std::string& query) {
    std::unordered_map<std::string, std::string> params;
    std::istringstream iss(query);
    std::string pair;
    
    while (std::getline(iss, pair, '&')) {
        size_t pos = pair.find('=');
        if (pos != std::string::npos) {
            std::string key = pair.substr(0, pos);
            std::string value = pair.substr(pos + 1);
            // Декодирование URL (базовое)
            params[key] = value;
        }
    }
    return params;
}

std::string handle_get_cars() {
    std::string content = read_file("data/cars.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        std::cerr << "⚠️  Не удалось загрузить data/cars.json\n";
        return R"([{"error": "No cars available. Check server data directory."}])";
    }
    
    return content;
}

std::string handle_post_search(const std::string& body) {
    // Заглушка для POST поиска
    return R"({"message": "POST search functionality coming soon", "results": []})";
}

std::string handle_get_search(const std::string& query_string) {
    // Заглушка для GET поиска
    return R"({"message": "GET search functionality coming soon", "results": []})";
}

std::string handle_get_cities() {
    // Читаем файл с городами или возвращаем заглушку
    std::string content = read_file("data/cities.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        // Возвращаем заглушку, если файла нет
        return R"([{"id": 1, "name": "Москва"}, {"id": 2, "name": "Санкт-Петербург"}, {"id": 3, "name": "Новосибирск"}])";
    }
    
    return content;
}

std::string handle_get_documents() {
    std::string content = read_file("data/documents.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        std::cerr << "⚠️  Не удалось загрузить data/documents.json\n";
        return R"({"error": "No documents available"})";
    }
    
    return content;
}

std::string handle_get_delivery() {
    // Информация о процессе доставки
    return R"({
        "process": [
            {"step": 1, "description": "Выбор автомобиля и заключение договора"},
            {"step": 2, "description": "Оплата и оформление документов"},
            {"step": 3, "description": "Доставка автомобиля в порт назначения"},
            {"step": 4, "description": "Таможенное оформление"},
            {"step": 5, "description": "Постановка на учет в ГИБДД"}
        ],
        "duration": "30-45 дней",
        "cost": "от 1000 USD"
    })";
}

std::string handle_post_admin_login(const std::string& body) {
    // Простая проверка логина/пароля
    if (body.find("\"username\"") != std::string::npos && body.find("\"password\"") != std::string::npos) {
        // Проверяем наличие admin/admin (для демо)
        if (body.find("admin") != std::string::npos) {
            return R"({"status": "success", "message": "Admin login successful", "token": "demo_token_12345"})";
        }
    }
    return R"({"error": "Invalid admin credentials"})";
}

// Новые обработчики для дополнительных эндпоинтов клиента
std::string handle_get_cars_specs(const std::string& query_string) {
    auto params = parse_query(query_string);
    
    // Здесь должна быть логика фильтрации по характеристикам
    std::string response = R"({
        "message": "Search by specifications",
        "filters_applied": )" + std::to_string(params.size()) + R"(,
        "results": []
    })";
    
    return response;
}

std::string handle_get_cars_brand(const std::string& query_string) {
    auto params = parse_query(query_string);
    std::string brand = params.count("brand") ? params["brand"] : "";
    std::string model = params.count("model") ? params["model"] : "";
    
    std::string response = R"({
        "message": "Search by brand and model",
        "brand": ")" + brand + R"(",
        "model": ")" + model + R"(",
        "results": []
    })";
    
    return response;
}

std::string handle_get_delivery_cities() {
    return handle_get_cities(); // Используем тот же обработчик
}

std::string handle_get_delivery_process() {
    return handle_get_delivery(); // Используем тот же обработчик
}
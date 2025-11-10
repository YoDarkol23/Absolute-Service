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
#include <unordered_map>
#include <algorithm>
#include <regex>

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
            // Простая URL-декодировка
            value = std::regex_replace(value, std::regex("%20"), " ");
            params[key] = value;
        }
    }
    return params;
}

// Вспомогательная функция для парсинга JSON массива автомобилей
std::string filter_cars_by_params(const std::string& cars_json, const std::unordered_map<std::string, std::string>& params) {
    // Простая фильтрация по бренду (для демонстрации)
    if (params.count("brand")) {
        std::string brand = params.at("brand");
        std::transform(brand.begin(), brand.end(), brand.begin(), ::tolower);
        
        // Ищем автомобили с указанным брендом
        std::stringstream result;
        result << "[";
        bool first = true;
        
        // Простой поиск по подстроке (для демо)
        size_t pos = 0;
        while ((pos = cars_json.find("\"brand\"", pos)) != std::string::npos) {
            size_t start = cars_json.find('"', pos + 7) + 1;
            size_t end = cars_json.find('"', start);
            std::string car_brand = cars_json.substr(start, end - start);
            std::transform(car_brand.begin(), car_brand.end(), car_brand.begin(), ::tolower);
            
            if (car_brand.find(brand) != std::string::npos) {
                // Находим начало и конец объекта
                size_t obj_start = cars_json.rfind('{', pos);
                size_t obj_end = cars_json.find("},", pos) + 1;
                if (obj_end == std::string::npos) obj_end = cars_json.find(']', pos);
                
                if (!first) result << ",";
                result << cars_json.substr(obj_start, obj_end - obj_start);
                first = false;
            }
            pos = end;
        }
        result << "]";
        
        return result.str();
    }
    
    return cars_json;
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
    return R"({"message": "POST search functionality coming soon", "results": []})";
}

std::string handle_get_search(const std::string& query_string) {
    return R"({"message": "GET search functionality coming soon", "results": []})";
}

std::string handle_get_cities() {
    std::string content = read_file("data/cities.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        return R"([{"id": 1, "name": "Москва", "delivery_time": "30 дней", "cost": "1000 USD"}, {"id": 2, "name": "Санкт-Петербург", "delivery_time": "35 дней", "cost": "1200 USD"}, {"id": 3, "name": "Новосибирск", "delivery_time": "45 дней", "cost": "1500 USD"}])";
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
    if (body.find("\"username\"") != std::string::npos && body.find("\"password\"") != std::string::npos) {
        // Простая проверка - любой пароль с admin работает
        if (body.find("admin") != std::string::npos) {
            return R"({"status": "success", "message": "Admin login successful", "token": "demo_token_12345"})";
        }
    }
    return R"({"error": "Invalid admin credentials. Use username: admin, password: admin"})";
}

std::string handle_get_cars_specs(const std::string& query_string) {
    auto params = parse_query(query_string);
    
    std::string response = R"({
        "message": "Search by specifications",
        "filters_applied": )" + std::to_string(params.size()) + R"(,
        "results": []
    })";
    
    return response;
}

std::string handle_get_cars_brand(const std::string& query_string) {
    auto params = parse_query(query_string);
    
    // Загружаем все автомобили
    std::string all_cars = read_file("data/cars.json");
    
    if (all_cars.empty() || all_cars.find("error") != std::string::npos) {
        return R"({"error": "Cannot load cars data"})";
    }
    
    // Фильтруем по параметрам
    std::string filtered_cars = filter_cars_by_params(all_cars, params);
    
    return filtered_cars;
}

std::string handle_get_delivery_cities() {
    return handle_get_cities();
}

std::string handle_get_delivery_process() {
    return handle_get_delivery();
}
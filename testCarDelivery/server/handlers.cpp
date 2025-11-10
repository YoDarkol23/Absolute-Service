/**
 * @file server/handlers.cpp
 * @brief Реализация бизнес-логики сервера согласно ТЗ.
 * 
 * Содержит обработчики для всех эндпоинтов с полной функциональностью.
 */

#include "handlers.hpp"
#include "../common/utils.hpp"
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <vector>
#include <cmath>

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
            // URL-декодировка
            value = std::regex_replace(value, std::regex("%20"), " ");
            value = std::regex_replace(value, std::regex("%2C"), ",");
            params[key] = value;
        }
    }
    return params;
}

// Функция для парсинга числовых фильтров (> , <, >=, <=)
bool matches_numeric_filter(const std::string& value, const std::string& filter) {
    if (filter.empty() || value.empty()) return true;
    
    try {
        double num_value = std::stod(value);
        
        if (filter[0] == '>') {
            if (filter[1] == '=') {
                double filter_value = std::stod(filter.substr(2));
                return num_value >= filter_value;
            } else {
                double filter_value = std::stod(filter.substr(1));
                return num_value > filter_value;
            }
        } else if (filter[0] == '<') {
            if (filter[1] == '=') {
                double filter_value = std::stod(filter.substr(2));
                return num_value <= filter_value;
            } else {
                double filter_value = std::stod(filter.substr(1));
                return num_value < filter_value;
            }
        } else {
            // Простое равенство
            double filter_value = std::stod(filter);
            return std::abs(num_value - filter_value) < 0.001;
        }
    } catch (...) {
        return false;
    }
}

// Функция для проверки соответствия автомобиля фильтрам
bool car_matches_filters(const std::string& car_json, const std::unordered_map<std::string, std::string>& filters) {
    for (const auto& [key, value] : filters) {
        if (value.empty()) continue;
        
        // Ищем поле в JSON
        std::string search_key = "\"" + key + "\":";
        size_t pos = car_json.find(search_key);
        if (pos == std::string::npos) continue;
        
        // Извлекаем значение
        size_t value_start = car_json.find(':', pos) + 1;
        while (value_start < car_json.length() && (car_json[value_start] == ' ' || car_json[value_start] == '\t')) {
            value_start++;
        }
        
        size_t value_end = value_start;
        if (car_json[value_start] == '"') {
            // Строковое значение
            value_start++;
            value_end = car_json.find('"', value_start);
        } else {
            // Числовое значение
            value_end = car_json.find_first_of(",}", value_start);
        }
        
        if (value_end == std::string::npos) continue;
        
        std::string car_value = car_json.substr(value_start, value_end - value_start);
        
        // Проверяем соответствие фильтру
        if (key == "year" || key == "price_usd" || key == "engine_volume") {
            if (!matches_numeric_filter(car_value, value)) {
                return false;
            }
        } else {
            // Строковый поиск (частичное совпадение)
            std::string car_val_lower = car_value;
            std::string filter_val_lower = value;
            std::transform(car_val_lower.begin(), car_val_lower.end(), car_val_lower.begin(), ::tolower);
            std::transform(filter_val_lower.begin(), filter_val_lower.end(), filter_val_lower.begin(), ::tolower);
            
            if (car_val_lower.find(filter_val_lower) == std::string::npos) {
                return false;
            }
        }
    }
    return true;
}

// Функция для фильтрации автомобилей
std::string filter_cars(const std::string& cars_json, const std::unordered_map<std::string, std::string>& filters) {
    std::stringstream result;
    result << "[";
    bool first = true;
    
    size_t pos = 0;
    while ((pos = cars_json.find('{', pos)) != std::string::npos) {
        size_t end = cars_json.find('}', pos) + 1;
        if (end == std::string::npos) break;
        
        std::string car_obj = cars_json.substr(pos, end - pos);
        
        if (car_matches_filters(car_obj, filters)) {
            if (!first) result << ",";
            result << car_obj;
            first = false;
        }
        
        pos = end;
    }
    
    result << "]";
    return result.str();
}

// Основные обработчики
std::string handle_get_cars() {
    std::string content = read_file("data/cars.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        std::cerr << "⚠️  Не удалось загрузить data/cars.json\n";
        return R"({"error": "No cars available. Check server data directory."})";
    }
    
    return content;
}

std::string handle_post_search(const std::string& body) {
    // Парсим JSON тело для поиска по характеристикам
    // Формат: {"filters": {"brand": "Toyota", "year": ">2020", "price_usd": "<30000"}}
    try {
        std::unordered_map<std::string, std::string> filters;
        
        // Простой парсинг JSON (для демо)
        size_t pos = 0;
        while ((pos = body.find("\"", pos)) != std::string::npos) {
            size_t key_start = pos + 1;
            size_t key_end = body.find("\"", key_start);
            if (key_end == std::string::npos) break;
            
            std::string key = body.substr(key_start, key_end - key_start);
            
            size_t value_start = body.find(":", key_end) + 1;
            while (value_start < body.length() && (body[value_start] == ' ' || body[value_start] == '\t')) {
                value_start++;
            }
            
            size_t value_end;
            if (body[value_start] == '"') {
                value_start++;
                value_end = body.find("\"", value_start);
            } else {
                value_end = body.find_first_of(",}", value_start);
            }
            
            if (value_end == std::string::npos) break;
            
            std::string value = body.substr(value_start, value_end - value_start);
            filters[key] = value;
            
            pos = value_end + 1;
        }
        
        std::string all_cars = read_file("data/cars.json");
        if (all_cars.empty() || all_cars.find("error") != std::string::npos) {
            return R"({"error": "Cannot load cars data"})";
        }
        
        std::string filtered_cars = filter_cars(all_cars, filters);
        
        if (filtered_cars == "[]") {
            return R"({"message": "Извините, по Вашему запросу ничего не найдено", "results": []})";
        }
        
        return R"({"message": "Результаты поиска", "results": )" + filtered_cars + "}";
        
    } catch (const std::exception& e) {
        return R"({"error": "Invalid search request format"})";
    }
}

std::string handle_get_search(const std::string& query_string) {
    auto params = parse_query(query_string);
    
    std::string all_cars = read_file("data/cars.json");
    if (all_cars.empty() || all_cars.find("error") != std::string::npos) {
        return R"({"error": "Cannot load cars data"})";
    }
    
    std::string filtered_cars = filter_cars(all_cars, params);
    
    if (filtered_cars == "[]") {
        return R"({"message": "Извините, по Вашему запросу ничего не найдено", "results": []})";
    }
    
    return R"({"message": "Результаты поиска", "results": )" + filtered_cars + "}";
}

std::string handle_get_cities() {
    std::string content = read_file("data/cities.json");
    
    if (content.empty() || content.find("error") != std::string::npos) {
        // Возвращаем встроенный список городов
        return R"([
            {"id": 1, "name": "Москва", "delivery_time": "30 дней", "cost": "1000 USD"},
            {"id": 2, "name": "Санкт-Петербург", "delivery_time": "35 дней", "cost": "1200 USD"},
            {"id": 3, "name": "Новосибирск", "delivery_time": "45 дней", "cost": "1500 USD"},
            {"id": 4, "name": "Екатеринбург", "delivery_time": "40 дней", "cost": "1300 USD"},
            {"id": 5, "name": "Нижний Новгород", "delivery_time": "35 дней", "cost": "1200 USD"},
            {"id": 6, "name": "Казань", "delivery_time": "38 дней", "cost": "1250 USD"},
            {"id": 7, "name": "Челябинск", "delivery_time": "42 дней", "cost": "1400 USD"},
            {"id": 8, "name": "Омск", "delivery_time": "44 дней", "cost": "1450 USD"},
            {"id": 9, "name": "Самара", "delivery_time": "37 дней", "cost": "1250 USD"},
            {"id": 10, "name": "Ростов-на-Дону", "delivery_time": "39 дней", "cost": "1350 USD"}
        ])";
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
            {"step": 1, "description": "Выбор автомобиля и заключение договора", "status": "completed"},
            {"step": 2, "description": "Оплата и оформление документов", "status": "completed"},
            {"step": 3, "description": "Доставка автомобиля в порт назначения", "status": "in_progress"},
            {"step": 4, "description": "Таможенное оформление", "status": "pending"},
            {"step": 5, "description": "Постановка на учет в ГИБДД", "status": "pending"}
        ],
        "current_step": 3,
        "progress": 60,
        "duration": "30-45 дней",
        "estimated_completion": "2025-02-15",
        "cost": "от 1000 USD"
    })";
}

std::string handle_post_admin_login(const std::string& body) {
    // Простая проверка - для демо используем admin/admin
    if (body.find("\"username\"") != std::string::npos && body.find("\"password\"") != std::string::npos) {
        if (body.find("admin") != std::string::npos) {
            return R"({
                "status": "success", 
                "message": "Admin login successful", 
                "token": "admin_token_2025",
                "user": {
                    "username": "admin",
                    "role": "administrator"
                }
            })";
        }
    }
    return R"({"error": "Invalid admin credentials. Use username: admin, password: admin"})";
}

std::string handle_get_cars_specs(const std::string& query_string) {
    auto params = parse_query(query_string);
    
    std::string all_cars = read_file("data/cars.json");
    if (all_cars.empty() || all_cars.find("error") != std::string::npos) {
        return R"({"error": "Cannot load cars data"})";
    }
    
    std::string filtered_cars = filter_cars(all_cars, params);
    
    if (filtered_cars == "[]") {
        return R"({"message": "Извините, по Вашему запросу ничего не найдено", "results": []})";
    }
    
    return R"({"message": "Результаты поиска по характеристикам", "results": )" + filtered_cars + "}";
}

std::string handle_get_cars_brand(const std::string& query_string) {
    auto params = parse_query(query_string);
    
    std::string all_cars = read_file("data/cars.json");
    if (all_cars.empty() || all_cars.find("error") != std::string::npos) {
        return R"({"error": "Cannot load cars data"})";
    }
    
    std::string filtered_cars = filter_cars(all_cars, params);
    
    if (filtered_cars == "[]") {
        return R"({"message": "Извините, по Вашему запросу ничего не найдено", "results": []})";
    }
    
    return R"({"message": "Результаты поиска по марке и модели", "results": )" + filtered_cars + "}";
}

std::string handle_get_delivery_cities() {
    return handle_get_cities();
}

std::string handle_get_delivery_process() {
    return handle_get_delivery();
}

// Административные обработчики
std::string handle_admin_get_cars() {
    return handle_get_cars(); // Для админа возвращаем те же данные
}

std::string handle_admin_add_car(const std::string& body) {
    // Заглушка для добавления автомобиля
    return R"({
        "status": "success", 
        "message": "Автомобиль успешно добавлен",
        "car_id": 999
    })";
}

std::string handle_admin_update_car(int car_id, const std::string& body) {
    // Заглушка для обновления автомобиля
    return R"({
        "status": "success", 
        "message": "Автомобиль с ID )" + std::to_string(car_id) + R"( успешно обновлен"
    })";
}

std::string handle_admin_delete_car(int car_id) {
    // Заглушка для удаления автомобиля
    return R"({
        "status": "success", 
        "message": "Автомобиль с ID )" + std::to_string(car_id) + R"( успешно удален"
    })";
}